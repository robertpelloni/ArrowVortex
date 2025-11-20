#include <Editor/Waveform.h>
#include "Spectrogram.h"

#include <Gist.h>

#include <math.h>
#include <stdint.h>
#include <algorithm>
#include <vector>

#include <Core/Utils.h>
#include <Core/Draw.h>
#include <Core/QuadBatch.h>
#include <Core/StringUtils.h>
#include <Core/Xmr.h>

#include <System/System.h>
#include <System/Debug.h>

#include <Editor/Music.h>
#include <Editor/View.h>
#include <Editor/Editor.h>
#include <Managers/TempoMan.h>
#include <Editor/Menubar.h>
#include <Editor/TextOverlay.h>
#include <Editor/Butterworth.h>

namespace Vortex {

static const int TEX_W = 256;
static const int TEX_H = 128;
static const int UNUSED_BLOCK = -1;

struct WaveBlock
{
	int id;
	Texture tex[4];
};

// ================================================================================================
// WaveFilter.

struct WaveFilter {

Waveform::FilterType type;
double strength;

Vector<short> samplesL;
Vector<short> samplesR;

static void lowPassFilter(const short* src, short* dst,
	int numFrames, int samplerate, double strength)
{
	double cutoff = 0.01 + 0.1 * (1.0 - strength);
	LowPassFilter(dst, src, numFrames, cutoff);
}

static void highPassFilter(const short* src, short* dst,
	int numFrames, int samplerate, double strength)
{
	double cutoff = 0.10 + 0.80 * strength;
	HighPassFilter(dst, src, numFrames, cutoff);
}

WaveFilter(Waveform::FilterType type, double strength)
	: type(type), strength(strength)
{
	update();
}

void update()
{
	auto filter = highPassFilter;
	if(type == Waveform::FT_LOW_PASS)
	{
		filter = lowPassFilter;
	}

	samplesL.release();
	samplesR.release();

	auto& music = gMusic->getSamples();
	if(music.isCompleted())
	{
		int numFrames = music.getNumFrames();
		int samplerate = music.getFrequency();

		samplesL.resize(numFrames, 0);
		samplesR.resize(numFrames, 0);

		filter(music.samplesL(), samplesL.begin(), numFrames, samplerate, strength);
		filter(music.samplesR(), samplesR.begin(), numFrames, samplerate, strength);
	}
}

}; // WaveFilter.

// ================================================================================================
// WaveFilterRGB.

struct WaveFilterRGB {

Vector<short> samplesL[3];
Vector<short> samplesR[3]; // 0=Low, 1=Mid, 2=High
bool dirty = true;

void update()
{
	if (!dirty) return;
	auto& music = gMusic->getSamples();
	if (!music.isCompleted()) return;

	int numFrames = music.getNumFrames();
	int samplerate = music.getFrequency();

	for(int i=0; i<3; ++i) {
		 samplesL[i].resize(numFrames);
		 samplesR[i].resize(numFrames);
	}

	// Low: LowPass 250Hz
	LowPassFilter(samplesL[0].begin(), music.samplesL(), numFrames, 250.0 * 2.0 / samplerate);
	LowPassFilter(samplesR[0].begin(), music.samplesR(), numFrames, 250.0 * 2.0 / samplerate);

	// High: HighPass 2500Hz
	HighPassFilter(samplesL[2].begin(), music.samplesL(), numFrames, 2500.0 * 2.0 / samplerate);
	HighPassFilter(samplesR[2].begin(), music.samplesR(), numFrames, 2500.0 * 2.0 / samplerate);

	// Mid: BandPass 250Hz-2500Hz (HighPass 250Hz, then LowPass 2500Hz)
	// Reuse Mid buffer for intermediate
	LowPassFilter(samplesL[1].begin(), music.samplesL(), numFrames, 2500.0 * 2.0 / samplerate);
	HighPassFilter(samplesL[1].begin(), samplesL[1].begin(), numFrames, 250.0 * 2.0 / samplerate);

	LowPassFilter(samplesR[1].begin(), music.samplesR(), numFrames, 2500.0 * 2.0 / samplerate);
	HighPassFilter(samplesR[1].begin(), samplesR[1].begin(), numFrames, 250.0 * 2.0 / samplerate);

	dirty = false;
}

}; // WaveFilterRGB.

// ================================================================================================
// WaveFilterSpectral.

// Uses Gist to calculate the spectral centroid and map it to a color.
struct WaveFilterSpectral {

struct ColorPoint { uchar r, g, b; };
Vector<ColorPoint> colorsL;
Vector<ColorPoint> colorsR;
bool dirty = true;

static void ProcessChannel(const short* src, int numFrames, int samplerate, Vector<ColorPoint>& colors)
{
	// Frame size for FFT analysis. Gist uses powers of 2.
	// 1024 at 44100Hz is about 23ms.
	const int frameSize = 1024;
	const int hopSize = 256; // Overlap

	if (numFrames < frameSize) return;

	colors.resize(numFrames);

	Gist<float> gist(frameSize, samplerate);
	std::vector<float> audioFrame(frameSize);

	// Fill with default color
	for(int i=0; i<numFrames; ++i) colors[i] = {255, 255, 255};

	int numBlocks = (numFrames - frameSize) / hopSize;

	for (int i = 0; i < numBlocks; ++i)
	{
		int startSample = i * hopSize;
		for (int j = 0; j < frameSize; ++j)
		{
			audioFrame[j] = src[startSample + j] / 32768.0f;
		}

		gist.processAudioFrame(audioFrame);
		float centroid = gist.spectralCentroid(); // in Hz

		// Map centroid to color (HSV -> RGB)
		// H: Map 0-8000Hz to 0-360 (or partial range)
		// Low freq = Red (0), High freq = Blue (240) or Purple (280)
		float normalizedFreq = clamp(centroid / 5000.0f, 0.0f, 1.0f);
		// Inverted: Low=Red, High=Blue
		float h = (1.0f - normalizedFreq) * 240.0f;
		// Wait, usually Low=Red, High=Blue is intuitive? No, heat maps use Red=High energy.
		// Let's do Low Freq = Red, High Freq = Blue/Violet.
		// Red is 0, Blue is 240.
		h = normalizedFreq * 280.0f;

		float s = 0.8f;
		float v = 1.0f;

		// Simple HSV to RGB
		float c = v * s;
		float x = c * (1 - fabs(fmod(h / 60.0f, 2) - 1));
		float m = v - c;
		float r=0, g=0, b=0;

		if (h < 60) { r=c; g=x; b=0; }
		else if (h < 120) { r=x; g=c; b=0; }
		else if (h < 180) { r=0; g=c; b=x; }
		else if (h < 240) { r=0; g=x; b=c; }
		else if (h < 300) { r=x; g=0; b=c; }
		else { r=c; g=0; b=x; }

		ColorPoint cp = { (uchar)((r+m)*255), (uchar)((g+m)*255), (uchar)((b+m)*255) };

		// Fill the hop region with this color
		int endSample = min(numFrames, startSample + hopSize);
		for(int k=startSample; k < endSample; ++k) {
			colors[k] = cp;
		}
	}
}

void update()
{
	if (!dirty) return;
	auto& music = gMusic->getSamples();
	if (!music.isCompleted()) return;

	int numFrames = music.getNumFrames();
	int samplerate = music.getFrequency();

	ProcessChannel(music.samplesL(), numFrames, samplerate, colorsL);
	ProcessChannel(music.samplesR(), numFrames, samplerate, colorsR);

	dirty = false;
}

}; // WaveFilterSpectral.

// ================================================================================================
// WaveFilterPitch.

// Uses Gist to calculate the pitch and map it to a color.
struct WaveFilterPitch {

struct ColorPoint { uchar r, g, b; };
Vector<ColorPoint> colorsL;
Vector<ColorPoint> colorsR;
bool dirty = true;

static void ProcessChannel(const short* src, int numFrames, int samplerate, Vector<ColorPoint>& colors)
{
	const int frameSize = 1024;
	const int hopSize = 256;

	if (numFrames < frameSize) return;

	colors.resize(numFrames);

	Gist<float> gist(frameSize, samplerate);
	std::vector<float> audioFrame(frameSize);

	// Fill with default color
	for(int i=0; i<numFrames; ++i) colors[i] = {255, 255, 255};

	int numBlocks = (numFrames - frameSize) / hopSize;

	for (int i = 0; i < numBlocks; ++i)
	{
		int startSample = i * hopSize;
		for (int j = 0; j < frameSize; ++j)
		{
			audioFrame[j] = src[startSample + j] / 32768.0f;
		}

		gist.processAudioFrame(audioFrame);
		float pitch = gist.pitch(); // in Hz

		// Map pitch to color (Chromatic)
		// Midi note = 69 + 12 * log2(freq / 440)
		// C = 0, C# = 1, ... B = 11

		float r=0, g=0, b=0;

		if (pitch > 20.0f) {
			float midiNote = 69.0f + 12.0f * log2(pitch / 440.0f);
			float chroma = fmod(midiNote, 12.0f);
			if (chroma < 0) chroma += 12.0f;

			// Map 0-12 to Hue 0-360
			float h = chroma * 30.0f; // 360 / 12
			float s = 0.8f;
			float v = 1.0f;

			float c = v * s;
			float x = c * (1 - fabs(fmod(h / 60.0f, 2) - 1));
			float m = v - c;

			if (h < 60) { r=c; g=x; b=0; }
			else if (h < 120) { r=x; g=c; b=0; }
			else if (h < 180) { r=0; g=c; b=x; }
			else if (h < 240) { r=0; g=x; b=c; }
			else if (h < 300) { r=x; g=0; b=c; }
			else { r=c; g=0; b=x; }
		} else {
			// Unpitched / Silence -> White/Gray
			r=1; g=1; b=1;
		}

		ColorPoint cp = { (uchar)((r)*255), (uchar)((g)*255), (uchar)((b)*255) };

		int endSample = min(numFrames, startSample + hopSize);
		for(int k=startSample; k < endSample; ++k) {
			colors[k] = cp;
		}
	}
}

void update()
{
	if (!dirty) return;
	auto& music = gMusic->getSamples();
	if (!music.isCompleted()) return;

	int numFrames = music.getNumFrames();
	int samplerate = music.getFrequency();

	ProcessChannel(music.samplesL(), numFrames, samplerate, colorsL);
	ProcessChannel(music.samplesR(), numFrames, samplerate, colorsR);

	dirty = false;
}

}; // WaveFilterPitch.

// ================================================================================================
// WaveformImpl :: member data.

struct WaveformImpl : public Waveform {

Vector<WaveBlock*> waveformBlocks_;

WaveFilter* waveformFilter_;
WaveFilterRGB* waveformFilterRGB_;
WaveFilterSpectral* waveformFilterSpectral_;
WaveFilterPitch* waveformFilterPitch_;
Vector<uchar> waveformTextureBuffer_;

int waveformBlockWidth_, waveformSpacing_;

ColorScheme waveformColorScheme_;
WaveShape waveformShape_;
Luminance waveformLuminance_;
ColorMode waveformColorMode_;
int waveformAntiAliasingMode_;
bool waveformOverlayFilter_;
Spectrogram* m_spectrogram;
bool m_showSpectrogram;

// ================================================================================================
// ViewImpl :: constructor / destructor.

~WaveformImpl()
{
	for(auto block : waveformBlocks_) delete block;
	delete waveformFilter_;
	delete waveformFilterRGB_;
	delete waveformFilterSpectral_;
	delete waveformFilterPitch_;
}

WaveformImpl()
{
	setPreset(PRESET_VORTEX);

	waveformFilter_ = nullptr;
	waveformFilterRGB_ = new WaveFilterRGB();
	waveformFilterSpectral_ = new WaveFilterSpectral();
	waveformFilterPitch_ = new WaveFilterPitch();
	waveformOverlayFilter_ = true;

	updateBlockW();
	waveformTextureBuffer_.resize(TEX_W * TEX_H * 4); // RGBA size just in case

	clearBlocks();

	m_spectrogram = new Spectrogram(1024, 44100);
	m_showSpectrogram = false;
}

// ================================================================================================
// ViewImpl :: load / save settings.

static void SaveColor(XmrNode* n, const char* name, colorf col)
{
	String r = Str::val(col.r, 0, 2), g = Str::val(col.g, 0, 2);
	String b = Str::val(col.b, 0, 2), a = Str::val(col.a, 0, 2);
	const char* str[] = {r.str(), g.str(), b.str(), a.str()};
	n->addAttrib(name, str, 4);
}

static const char* ToString(WaveShape style)
{
	if(style == WS_SIGNED) return "signed";
	return "rectified";
}

static const char* ToString(Luminance lum)
{
	if(lum == LL_AMPLITUDE) return "amplitude";
	return "uniform";
}

static const char* ToString(ColorMode mode)
{
	if(mode == CM_RGB) return "rgb";
	if(mode == CM_SPECTRAL) return "spectral";
	if(mode == CM_PITCH) return "pitch";
	return "flat";
}

static WaveShape ToWaveShape(StringRef str)
{
	if(str == "signed") return WS_SIGNED;
	return WS_RECTIFIED;
}

static Luminance ToLuminance(StringRef str)
{
	if(str == "amplitude") return LL_AMPLITUDE;
	return LL_UNIFORM;
}

static ColorMode ToColorMode(StringRef str)
{
	if(str == "rgb") return CM_RGB;
	if(str == "spectral") return CM_SPECTRAL;
	if(str == "pitch") return CM_PITCH;
	return CM_FLAT;
}

void loadSettings(XmrNode& settings)
{
	XmrNode* waveform = settings.child("waveform");
	if(waveform)
	{
		waveform->get("bgColor", (float*)&waveformColorScheme_.bg, 4);
		waveform->get("waveColor", (float*)&waveformColorScheme_.wave, 4);
		waveform->get("filterColor", (float*)&waveformColorScheme_.filter, 4);

		const char* ll = waveform->get("luminance");
		if(ll) setLuminance(ToLuminance(ll));

		const char* ws = waveform->get("waveStyle");
		if(ws) setWaveShape(ToWaveShape(ws));	

		const char* cm = waveform->get("colorMode");
		if(cm) setColorMode(ToColorMode(cm));

		waveform->get("antiAliasing", &waveformAntiAliasingMode_);
		setAntiAliasing(clamp(waveformAntiAliasingMode_, 0, 3));
	}
}

void saveSettings(XmrNode& settings)
{
	XmrNode* waveform = settings.child("waveform");
	if(!waveform) waveform = settings.addChild("waveform");

	SaveColor(waveform, "bgColor", waveformColorScheme_.bg);
	SaveColor(waveform, "waveColor", waveformColorScheme_.wave);
	SaveColor(waveform, "filterColor", waveformColorScheme_.filter);

	waveform->addAttrib("luminance", ToString(waveformLuminance_));
	waveform->addAttrib("waveStyle", ToString(waveformShape_));
	waveform->addAttrib("colorMode", ToString(waveformColorMode_));
	waveform->addAttrib("antiAliasing", (long)waveformAntiAliasingMode_);
}

// ================================================================================================
// ViewImpl :: member functions.

void clearBlocks()
{
	for(auto block : waveformBlocks_)
	{
		block->id = UNUSED_BLOCK;
	}
}

void setOverlayFilter(bool enabled)
{
	waveformOverlayFilter_ = enabled;

	clearBlocks();
}

bool getOverlayFilter()
{
	return waveformOverlayFilter_;
}

void enableFilter(FilterType type, double strength)
{
	delete waveformFilter_;
	waveformFilter_ = new WaveFilter(type, strength);

	clearBlocks();
}

void disableFilter()
{
	delete waveformFilter_;
	waveformFilter_ = nullptr;

	clearBlocks();
}

int getWidth()
{
	return waveformBlockWidth_ * 2 + waveformSpacing_ * 2 + 8;
}

void onChanges(int changes)
{
	if(changes & VCM_MUSIC_IS_LOADED)
	{
		if(waveformFilter_) waveformFilter_->update();
		waveformFilterRGB_->dirty = true;
		waveformFilterSpectral_->dirty = true;
		waveformFilterPitch_->dirty = true;
		if(waveformColorMode_ == CM_RGB) waveformFilterRGB_->update();
		if(waveformColorMode_ == CM_SPECTRAL) waveformFilterSpectral_->update();
		if(waveformColorMode_ == CM_PITCH) waveformFilterPitch_->update();
	}
}

void tick()
{
}

void setPreset(Preset preset)
{
	switch(preset)
	{
	case PRESET_VORTEX:
		waveformColorScheme_.bg = {0.0f, 0.0f, 0.0f, 1};
		waveformColorScheme_.wave = {0.4f, 0.6, 0.4f, 1};
		waveformColorScheme_.filter = {0.8f, 0.8f, 0.5f, 1};
		waveformShape_ = WS_RECTIFIED;
		waveformLuminance_ = LL_UNIFORM;
		waveformColorMode_ = CM_FLAT;
		waveformAntiAliasingMode_ = 1;		
		break;
	case PRESET_DDREAM:
		waveformColorScheme_.bg = {0.45f, 0.6f, 0.11f, 0.8f};
		waveformColorScheme_.wave = {0.9f, 0.9f, 0.33f, 1};
		waveformColorScheme_.filter = {0.8f, 0.3f, 0.3f, 1};
		waveformShape_ = WS_SIGNED;
		waveformLuminance_ = LL_UNIFORM;
		waveformColorMode_ = CM_FLAT;
		waveformAntiAliasingMode_ = 0;
		break;
	};
	clearBlocks();
}

void setColors(ColorScheme colors)
{
	waveformColorScheme_ = colors;
}

ColorScheme getColors()
{
	return waveformColorScheme_;
}

void setLuminance(Luminance lum)
{
	waveformLuminance_ = lum;
	clearBlocks();
}

Luminance getLuminance()
{
	return waveformLuminance_;
}

void setColorMode(ColorMode mode)
{
	waveformColorMode_ = mode;
	if(mode == CM_RGB) {
		waveformFilterRGB_->update();
	}
	else if (mode == CM_SPECTRAL) {
		waveformFilterSpectral_->update();
	}
	else if (mode == CM_PITCH) {
		waveformFilterPitch_->update();
	}
	clearBlocks();
}

ColorMode getColorMode()
{
	return waveformColorMode_;
}

void setWaveShape(WaveShape style)
{
	waveformShape_ = style;
	clearBlocks();
}

WaveShape getWaveShape()
{
	return waveformShape_;
}

void setAntiAliasing(int level)
{
	waveformAntiAliasingMode_ = level;
	clearBlocks();
}

int getAntiAliasing()
{
	return waveformAntiAliasingMode_;
}

// ================================================================================================
// Waveform :: luminance functions.

struct WaveEdge { int l, r; uchar lum; };

void edgeLumUniform(WaveEdge* edge, int h)
{
	for(int y = 0; y < h; ++y, ++edge)
	{
		edge->lum = 255;
	}
}

void edgeLumAmplitude(WaveEdge* edge, int w, int h)
{
	int scalar = (255 << 16) * 2 / w;
	for(int y = 0; y < h; ++y, ++edge)
	{
		int mag = max(abs(edge->l), abs(edge->r));
		edge->lum = (mag * scalar) >> 16;
	}
}

// ================================================================================================
// Waveform :: edge shape functions.

void edgeShapeRectified(uchar* dst, const WaveEdge* edge, int w, int h)
{
	int cx = w / 2;
	for(int y = 0; y < h; ++y, dst += w, ++edge)
	{
		int mag = max(abs(edge->l), abs(edge->r));
		int l = cx - mag;
		int r = cx + mag;
		for(int x = l; x < r; ++x) dst[x] = edge->lum;
	}
}

void edgeShapeSigned(uchar* dst, const WaveEdge* edge, int w, int h)
{
	int cx = w / 2;
	for(int y = 0; y < h; ++y, dst += w, ++edge)
	{
		int l = cx + min(edge->l, 0);
		int r = cx + max(edge->r, 0);
		for(int x = l; x < r; ++x) dst[x] = edge->lum;
	}
}

void edgeShapeRectifiedRGB(uchar* dst, const WaveEdge* edge, int w, int h, int channelOffset)
{
	int cx = w / 2;
	for(int y = 0; y < h; ++y, dst += w * 4, ++edge)
	{
		int mag = max(abs(edge->l), abs(edge->r));
		int l = cx - mag;
		int r = cx + mag;
		for(int x = l; x < r; ++x) {
			// Additive blending
			int val = (int)dst[x * 4 + channelOffset] + edge->lum;
			dst[x * 4 + channelOffset] = (uchar)min(val, 255);

			int a = max((int)dst[x*4+3], (int)dst[x*4+channelOffset]);
			dst[x*4+3] = (uchar)a;
		}
	}
}

void edgeShapeSignedRGB(uchar* dst, const WaveEdge* edge, int w, int h, int channelOffset)
{
	int cx = w / 2;
	for(int y = 0; y < h; ++y, dst += w * 4, ++edge)
	{
		int l = cx + min(edge->l, 0);
		int r = cx + max(edge->r, 0);
		for(int x = l; x < r; ++x) {
			int val = (int)dst[x * 4 + channelOffset] + edge->lum;
			dst[x * 4 + channelOffset] = (uchar)min(val, 255);
			int a = max((int)dst[x*4+3], (int)dst[x*4+channelOffset]);
			dst[x*4+3] = (uchar)a;
		}
	}
}

void edgeShapeRectifiedSpectral(uchar* dst, const WaveEdge* edge, int w, int h, const WaveFilterSpectral::ColorPoint* colors)
{
	int cx = w / 2;
	for(int y = 0; y < h; ++y, dst += w * 4, ++edge)
	{
		int mag = max(abs(edge->l), abs(edge->r));
		int l = cx - mag;
		int r = cx + mag;
		const auto& cp = colors[y];
		for(int x = l; x < r; ++x) {
			// Use the spectral color, but modulated by edge luminance
			dst[x*4+0] = (cp.r * edge->lum) >> 8;
			dst[x*4+1] = (cp.g * edge->lum) >> 8;
			dst[x*4+2] = (cp.b * edge->lum) >> 8;
			dst[x*4+3] = 255;
		}
	}
}

void edgeShapeSignedSpectral(uchar* dst, const WaveEdge* edge, int w, int h, const WaveFilterSpectral::ColorPoint* colors)
{
	int cx = w / 2;
	for(int y = 0; y < h; ++y, dst += w * 4, ++edge)
	{
		int l = cx + min(edge->l, 0);
		int r = cx + max(edge->r, 0);
		const auto& cp = colors[y];
		for(int x = l; x < r; ++x) {
			dst[x*4+0] = (cp.r * edge->lum) >> 8;
			dst[x*4+1] = (cp.g * edge->lum) >> 8;
			dst[x*4+2] = (cp.b * edge->lum) >> 8;
			dst[x*4+3] = 255;
		}
	}
}

// ================================================================================================
// Waveform :: anti-aliasing functions.

void antiAlias2x(uchar* dst, int w, int h)
{
	int newW = w / 2, newH = h / 2;
	for(int y = 0; y < newH; ++y)
	{
		uchar* line = waveformTextureBuffer_.begin() + (y * 2) * w;
		for(int x = 0; x < newW; ++x, ++dst)
		{
			uchar* a = line + (x * 2), *b = a + w;
			int sum = 0;
			sum += a[0] + a[1];
			sum += b[0] + b[1];
			*dst = sum / 4;
		}
	}
}

void antiAlias3x(uchar* dst, int w, int h)
{
	int newW = w / 3, newH = h / 3;
	for(int y = 0; y < newH; ++y)
	{
		uchar* line = waveformTextureBuffer_.begin() + (y * 3) * w;
		for(int x = 0; x < newW; ++x, ++dst)
		{
			uchar* a = line + (x * 3);
			uchar* b = a + w, *c = b + w;
			int sum = 0;
			sum += a[0] + a[1] + a[2];
			sum += b[0] + b[1] + b[2];
			sum += c[0] + c[1] + c[2];
			*dst = sum / 9;
		}
	}
}

void antiAlias4x(uchar* dst, int w, int h)
{
	int newW = w / 4, newH = h / 4;
	for(int y = 0; y < newH; ++y)
	{
		uchar* line = waveformTextureBuffer_.begin() + (y * 4) * w;
		for(int x = 0; x < newW; ++x, ++dst)
		{
			uchar* a = line + (x * 4), *b = a + w;
			uchar* c = b + w, *d = c + w;
			int sum = 0;
			sum += a[0] + a[1] + a[2] + a[3];
			sum += b[0] + b[1] + b[2] + b[3];
			sum += c[0] + c[1] + c[2] + c[3];
			sum += d[0] + d[1] + d[2] + d[3];
			*dst = sum / 16;
		}
	}
}

void antiAlias2xRGB(uchar* dst, int w, int h)
{
	int newW = w / 2, newH = h / 2;
	for(int y = 0; y < newH; ++y)
	{
		uchar* line = waveformTextureBuffer_.begin() + (y * 2) * w * 4;
		for(int x = 0; x < newW; ++x, dst += 4)
		{
			uchar* a = line + (x * 2) * 4, *b = a + w * 4;
			for(int c = 0; c < 4; ++c) {
				int sum = 0;
				sum += a[c] + a[4+c];
				sum += b[c] + b[4+c];
				dst[c] = sum / 4;
			}
		}
	}
}

void antiAlias3xRGB(uchar* dst, int w, int h)
{
	int newW = w / 3, newH = h / 3;
	for(int y = 0; y < newH; ++y)
	{
		uchar* line = waveformTextureBuffer_.begin() + (y * 3) * w * 4;
		for(int x = 0; x < newW; ++x, dst += 4)
		{
			uchar* a = line + (x * 3) * 4;
			uchar* b = a + w * 4, *c = b + w * 4;
			for(int ch = 0; ch < 4; ++ch) {
				int sum = 0;
				sum += a[ch] + a[4+ch] + a[8+ch];
				sum += b[ch] + b[4+ch] + b[8+ch];
				sum += c[ch] + c[4+ch] + c[8+ch];
				dst[ch] = sum / 9;
			}
		}
	}
}

void antiAlias4xRGB(uchar* dst, int w, int h)
{
	int newW = w / 4, newH = h / 4;
	for(int y = 0; y < newH; ++y)
	{
		uchar* line = waveformTextureBuffer_.begin() + (y * 4) * w * 4;
		for(int x = 0; x < newW; ++x, dst += 4)
		{
			uchar* a = line + (x * 4) * 4, *b = a + w * 4;
			uchar* c = b + w * 4, *d = c + w * 4;
			for(int ch = 0; ch < 4; ++ch) {
				int sum = 0;
				sum += a[ch] + a[4+ch] + a[8+ch] + a[12+ch];
				sum += b[ch] + b[4+ch] + b[8+ch] + b[12+ch];
				sum += c[ch] + c[4+ch] + c[8+ch] + c[12+ch];
				sum += d[ch] + d[4+ch] + d[8+ch] + d[12+ch];
				dst[ch] = sum / 16;
			}
		}
	}
}

// ================================================================================================
// Waveform :: block rendering functions.

void sampleEdges(WaveEdge* edges, int w, int h, int channel, int blockId, bool filtered)
{
	auto& music = gMusic->getSamples();

	double samplesPerSec = (double)music.getFrequency();
	double samplesPerPixel = samplesPerSec / fabs(gView->getPixPerSec());
	double samplesPerBlock = (double)TEX_H * samplesPerPixel;

	int64_t srcFrames = filtered ? waveformFilter_->samplesL.size() : music.getNumFrames();
	int64_t samplePos = max((int64_t)0, (int64_t)(samplesPerBlock * (double)blockId));
	double sampleCount = min((double) srcFrames - samplePos, samplesPerBlock);

	if (samplePos >= srcFrames || sampleCount <= 0)
	{
		// A crash could occur if we try to access out-of-bounds memory.
		// Fill the edges with zeroes to avoid this issue.
		for (int y = 0; y < h; ++y)
		{
			edges[y] = {0, 0, 0};
		}
		return;
	}

	// A crash can occur if another thread is loading the audio. Just do nothing if it is.
	if (!music.isAllocated())
	{
		return;
	}

	double sampleSkip = max(0.001, (samplesPerPixel / 200.0));
	int wh = w / 2 - 1;

	const short* in = nullptr;
	if(filtered)
	{
		in = ((channel == 0) ? waveformFilter_->samplesL.begin() : waveformFilter_->samplesR.begin());
	}
	else
	{
		in = ((channel == 0) ? music.samplesL() : music.samplesR());
	}
	in += samplePos;

	double advance = samplesPerPixel * TEX_H / h;
	double ofs = 0;
	for(int y = 0; y < h; ++y)
	{
		// Determine the last sample of the line.
		double end = min(sampleCount, (double)(y + 1) * advance);

		// Find the minimum/maximum amplitude within the line.
		int minAmp = SHRT_MAX;
		int maxAmp = SHRT_MIN;
		while(ofs < end)
		{
			maxAmp = max(maxAmp, (int)*(in + (int) round(ofs)));
			minAmp = min(minAmp, (int)*(in + (int) round(ofs)));
			ofs += sampleSkip;
		}

		// Clamp the minimum/maximum amplitude.
		int l = (minAmp * wh) >> 15;
		int r = (maxAmp * wh) >> 15;
		if(r >= l)
		{
			edges[y] = {clamp(l, -wh, wh), clamp(r, -wh, wh), 0};
		}
		else
		{
			edges[y] = {0, 0, 0};
		}
	}
}

void sampleEdgesRGB(WaveEdge* edges, int w, int h, int channel, int blockId, int band)
{
	// band: 0=Low, 1=Mid, 2=High
	auto& music = gMusic->getSamples();

	double samplesPerSec = (double)music.getFrequency();
	double samplesPerPixel = samplesPerSec / fabs(gView->getPixPerSec());
	double samplesPerBlock = (double)TEX_H * samplesPerPixel;

	int64_t srcFrames = waveformFilterRGB_->samplesL[band].size();
	int64_t samplePos = max((int64_t)0, (int64_t)(samplesPerBlock * (double)blockId));
	double sampleCount = min((double) srcFrames - samplePos, samplesPerBlock);

	if (samplePos >= srcFrames || sampleCount <= 0)
	{
		for (int y = 0; y < h; ++y) edges[y] = {0, 0, 0};
		return;
	}

	if (!music.isAllocated()) return;

	double sampleSkip = max(0.001, (samplesPerPixel / 200.0));
	int wh = w / 2 - 1;

	const short* in = ((channel == 0) ? waveformFilterRGB_->samplesL[band].begin() : waveformFilterRGB_->samplesR[band].begin());
	in += samplePos;

	double advance = samplesPerPixel * TEX_H / h;
	double ofs = 0;
	for(int y = 0; y < h; ++y)
	{
		double end = min(sampleCount, (double)(y + 1) * advance);
		int minAmp = SHRT_MAX;
		int maxAmp = SHRT_MIN;
		while(ofs < end)
		{
			maxAmp = max(maxAmp, (int)*(in + (int) round(ofs)));
			minAmp = min(minAmp, (int)*(in + (int) round(ofs)));
			ofs += sampleSkip;
		}

		int l = (minAmp * wh) >> 15;
		int r = (maxAmp * wh) >> 15;
		if(r >= l)
		{
			edges[y] = {clamp(l, -wh, wh), clamp(r, -wh, wh), 0};
		}
		else
		{
			edges[y] = {0, 0, 0};
		}
	}
}

void renderWaveform(Texture* textures, WaveEdge* edgeBuf, int w, int h, int blockId, bool filtered)
{
	uchar* texBuf = waveformTextureBuffer_.begin();
	for(int channel = 0; channel < 2; ++channel)
	{
		memset(texBuf, 0, w * h);

		// Process edges
		sampleEdges(edgeBuf, w, h, channel, blockId, filtered);

		// Apply luminance
		if (waveformLuminance_ == LL_UNIFORM) {
			edgeLumUniform(edgeBuf, h);
		}
		else if (waveformLuminance_ == LL_AMPLITUDE) {
			edgeLumAmplitude(edgeBuf, w, h);
		}

		// Apply wave shape
		if (waveformShape_ == WS_RECTIFIED) {
			edgeShapeRectified(texBuf, edgeBuf, w, h);
		}
		else if (waveformShape_ == WS_SIGNED) {
			edgeShapeSigned(texBuf, edgeBuf, w, h);
		}

		// Apply anti-aliasing
		switch (waveformAntiAliasingMode_) {
		case 1: antiAlias2x(texBuf, w, h); break;
		case 2: antiAlias3x(texBuf, w, h); break;
		case 3: antiAlias4x(texBuf, w, h); break;
		}

		// Create or update texture
		if (!textures[channel].handle() || textures[channel].format() != Texture::ALPHA) {
			textures[channel] = Texture(TEX_W, TEX_H, Texture::ALPHA);
		}
		textures[channel].modify(0, 0, waveformBlockWidth_, TEX_H, texBuf);
	}
}

template <typename FilterType>
void renderWaveformColored(Texture* textures, WaveEdge* edgeBuf, int w, int h, int blockId, FilterType* filter)
{
	// Similar to standard render, but we pass a color buffer to the shape function
	uchar* texBuf = waveformTextureBuffer_.begin();

	auto& music = gMusic->getSamples();
	double samplesPerSec = (double)music.getFrequency();
	double samplesPerPixel = samplesPerSec / fabs(gView->getPixPerSec());
	double samplesPerBlock = (double)TEX_H * samplesPerPixel;
	int64_t samplePos = max((int64_t)0, (int64_t)(samplesPerBlock * (double)blockId));

	// Collect colors for this block
	// We need to sample the color array at the same rate as the pixels (lines)
	Vector<WaveFilterSpectral::ColorPoint> blockColors;
	blockColors.resize(h);

	for(int channel = 0; channel < 2; ++channel)
	{
		const auto& srcColors = (channel==0) ? filter->colorsL : filter->colorsR;
		if (samplePos < srcColors.size()) {
			double advance = samplesPerPixel * TEX_H / h;
			for(int y=0; y<h; ++y) {
				int idx = (int)(samplePos + y * advance);
				if (idx < srcColors.size()) {
					// We assume ColorPoint is compatible or castable
					const auto& sc = srcColors[idx];
					blockColors[y] = {sc.r, sc.g, sc.b};
				} else {
					blockColors[y] = {255, 255, 255};
				}
			}
		}

		memset(texBuf, 0, w * h * 4);

		// Process edges (using standard sampleEdges on original audio)
		sampleEdges(edgeBuf, w, h, channel, blockId, false);

		if (waveformLuminance_ == LL_UNIFORM) {
			edgeLumUniform(edgeBuf, h);
		}
		else if (waveformLuminance_ == LL_AMPLITUDE) {
			edgeLumAmplitude(edgeBuf, w, h);
		}

		if (waveformShape_ == WS_RECTIFIED) {
			edgeShapeRectifiedSpectral(texBuf, edgeBuf, w, h, blockColors.begin());
		}
		else if (waveformShape_ == WS_SIGNED) {
			edgeShapeSignedSpectral(texBuf, edgeBuf, w, h, blockColors.begin());
		}

		// Anti-alias
		switch (waveformAntiAliasingMode_) {
		case 1: antiAlias2xRGB(texBuf, w, h); break;
		case 2: antiAlias3xRGB(texBuf, w, h); break;
		case 3: antiAlias4xRGB(texBuf, w, h); break;
		}

		if (!textures[channel].handle() || textures[channel].format() != Texture::RGBA) {
			textures[channel] = Texture(TEX_W, TEX_H, Texture::RGBA);
		}
		textures[channel].modify(0, 0, waveformBlockWidth_, TEX_H, texBuf);
	}
}

void renderWaveformRGB(Texture* textures, WaveEdge* edgeBuf, int w, int h, int blockId)
{
	uchar* texBuf = waveformTextureBuffer_.begin();
	for(int channel = 0; channel < 2; ++channel)
	{
		memset(texBuf, 0, w * h * 4); // RGBA

		// Process edges for each band
		// Band 0: Low -> Red (channel offset 0)
		// Band 1: Mid -> Green (channel offset 1)
		// Band 2: High -> Blue (channel offset 2)

		for(int band = 0; band < 3; ++band) {
			sampleEdgesRGB(edgeBuf, w, h, channel, blockId, band);

			if (waveformLuminance_ == LL_UNIFORM) {
				edgeLumUniform(edgeBuf, h);
			}
			else if (waveformLuminance_ == LL_AMPLITUDE) {
				edgeLumAmplitude(edgeBuf, w, h);
			}

			if (waveformShape_ == WS_RECTIFIED) {
				edgeShapeRectifiedRGB(texBuf, edgeBuf, w, h, band);
			}
			else if (waveformShape_ == WS_SIGNED) {
				edgeShapeSignedRGB(texBuf, edgeBuf, w, h, band);
			}
		}

		// Apply anti-aliasing (needs RGB support)
		switch (waveformAntiAliasingMode_) {
		case 1: antiAlias2xRGB(texBuf, w, h); break;
		case 2: antiAlias3xRGB(texBuf, w, h); break;
		case 3: antiAlias4xRGB(texBuf, w, h); break;
		}

		// Create or update texture
		if (!textures[channel].handle() || textures[channel].format() != Texture::RGBA) {
			textures[channel] = Texture(TEX_W, TEX_H, Texture::RGBA);
		}
		textures[channel].modify(0, 0, waveformBlockWidth_, TEX_H, texBuf);
	}
}

void renderBlock(WaveBlock* block)
{
	int w = waveformBlockWidth_ * (waveformAntiAliasingMode_ + 1);
	int h = TEX_H * (waveformAntiAliasingMode_ + 1);
	waveformTextureBuffer_.resize(w * h * 4); // Resize for RGBA

	Vector<WaveEdge> edges;
	edges.resize(h);

	if(waveformColorMode_ == CM_RGB)
	{
		// Update RGB filter if needed (might have been lazy loaded)
		waveformFilterRGB_->update();
		renderWaveformRGB(block->tex, edges.begin(), w, h, block->id);
	}
	else if(waveformColorMode_ == CM_SPECTRAL)
	{
		waveformFilterSpectral_->update();
		renderWaveformColored(block->tex, edges.begin(), w, h, block->id, waveformFilterSpectral_);
	}
	else if(waveformColorMode_ == CM_PITCH)
	{
		waveformFilterPitch_->update();
		renderWaveformColored(block->tex, edges.begin(), w, h, block->id, waveformFilterPitch_);
	}
	else if(waveformFilter_)
	{
		if(waveformOverlayFilter_)
		{
			renderWaveform(block->tex + 0, edges.begin(), w, h, block->id, false);
			renderWaveform(block->tex + 2, edges.begin(), w, h, block->id, true);
		}
		else
		{
			renderWaveform(block->tex, edges.begin(), w, h, block->id, true);
		}
	}
	else
	{
		renderWaveform(block->tex, edges.begin(), w, h, block->id, false);
	}
}

WaveBlock* getBlock(int id)
{
	// Check if we already have the requested block.
	for(auto block : waveformBlocks_)
	{
		if(block->id == id)
		{
			return block;
		}
	}

	// If not, check if we have any free blocks available.
	for(auto block : waveformBlocks_)
	{
		if(block->id == UNUSED_BLOCK)
		{
			block->id = id;
			renderBlock(block);
			return block;
		}
	}

	// If not, create a new block.
	WaveBlock* block = new WaveBlock;
	waveformBlocks_.push_back(block);

	block->id = id;
	renderBlock(block);
	return block;
}

void updateBlockW()
{
	int width = waveformBlockWidth_;

	waveformBlockWidth_ = min(TEX_W, gView->applyZoom(256));
	waveformSpacing_ = gView->applyZoom(24);

	if(waveformBlockWidth_ != width) clearBlocks();
}

void drawBackground()
{
	updateBlockW();

	int w = waveformBlockWidth_ * 2 + waveformSpacing_ * 2 + 8;
	int h = gView->getHeight();
	int cx = gView->getReceptorCoords().xc;

	Draw::fill({cx - w / 2, 0, w, h}, ToColor32(waveformColorScheme_.bg));
}

void drawPeaks()
{
	if (m_showSpectrogram)
	{
		m_spectrogram->draw();
		return;
	}

	updateBlockW();

	bool reversed = gView->hasReverseScroll();
	int visibilityStartY, visibilityEndY;
	if(reversed)
	{
		visibilityEndY = gView->timeToY(0);
		visibilityStartY = visibilityEndY - gView->getHeight();
	}
	else
	{
		visibilityStartY = -gView->timeToY(0);
		visibilityEndY = visibilityStartY + gView->getHeight();
	}

	// Free up blocks that are no longer visible.
	for(auto block : waveformBlocks_)
	{
		if(block->id != UNUSED_BLOCK)
		{
			int y = block->id * TEX_H;
			if(y >= visibilityEndY || y + TEX_H <= visibilityStartY)
			{
				block->id = UNUSED_BLOCK;
			}
		}
	}

	// Show blocks for the regions of the song that are visible.
	int border = waveformSpacing_, pw = waveformBlockWidth_ / 2;
	int cx = gView->getReceptorCoords().xc;
	int xl = cx - pw - border;
	int xr = cx + pw + border;

	areaf uvs = {0, 0, waveformBlockWidth_ / (float)TEX_W, 1};
	if(reversed) swapValues(uvs.t, uvs.b);

	int id = max(0, visibilityStartY / TEX_H);
	for(; id * TEX_H < visibilityEndY; ++id)
	{
		auto block = getBlock(id);

		int y = id * TEX_H - visibilityStartY;
		if(reversed) y = gView->getHeight() - y - TEX_H;

		TextureHandle texL = block->tex[0].handle();
		TextureHandle texR = block->tex[1].handle();

		if(waveformColorMode_ == CM_RGB || waveformColorMode_ == CM_SPECTRAL || waveformColorMode_ == CM_PITCH)
		{
			// Draw white rect with RGBA texture
			Draw::fill({xl - pw, y, pw * 2, TEX_H}, Colors::white, texL, uvs, Texture::RGBA);
			Draw::fill({xr - pw, y, pw * 2, TEX_H}, Colors::white, texR, uvs, Texture::RGBA);
		}
		else
		{
			color32 waveCol = ToColor32(waveformColorScheme_.wave);
			Draw::fill({xl - pw, y, pw * 2, TEX_H}, waveCol, texL, uvs, Texture::ALPHA);
			Draw::fill({xr - pw, y, pw * 2, TEX_H}, waveCol, texR, uvs, Texture::ALPHA);

			if(waveformFilter_ && waveformOverlayFilter_)
			{
				TextureHandle texL = block->tex[2].handle();
				TextureHandle texR = block->tex[3].handle();

				color32 filterCol = ToColor32(waveformColorScheme_.filter);
				Draw::fill({xl - pw, y, pw * 2, TEX_H}, filterCol, texL, uvs, Texture::ALPHA);
				Draw::fill({xr - pw, y, pw * 2, TEX_H}, filterCol, texR, uvs, Texture::ALPHA);
			}
		}
	}
}

void toggleSpectrogram()
{
	m_showSpectrogram = !m_showSpectrogram;
}
}; // WaveformImpl.

// ================================================================================================
// Waveform :: API.

Waveform* gWaveform = nullptr;

void Waveform::create(XmrNode& settings)
{
	gWaveform = new WaveformImpl;
	((WaveformImpl*)gWaveform)->loadSettings(settings);
}

void Waveform::destroy()
{
	delete (WaveformImpl*)gWaveform;
	gWaveform = nullptr;
}

}; // namespace Vortex
