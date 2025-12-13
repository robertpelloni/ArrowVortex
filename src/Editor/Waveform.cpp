#include <Editor/Waveform.h>
#include "Spectrogram.h"
#include <Editor/FindOnsets.h>

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

static const int TEX_W = 512;
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

void update(float lowFreq, float highFreq)
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

	double lowNorm = lowFreq * 2.0 / samplerate;
	double highNorm = highFreq * 2.0 / samplerate;

	// Low: LowPass lowFreq
	LowPassFilter(samplesL[0].begin(), music.samplesL(), numFrames, lowNorm);
	LowPassFilter(samplesR[0].begin(), music.samplesR(), numFrames, lowNorm);

	// High: HighPass highFreq
	HighPassFilter(samplesL[2].begin(), music.samplesL(), numFrames, highNorm);
	HighPassFilter(samplesR[2].begin(), music.samplesR(), numFrames, highNorm);

	// Mid: BandPass lowFreq-highFreq (HighPass lowFreq, then LowPass highFreq)
	// Reuse Mid buffer for intermediate
	LowPassFilter(samplesL[1].begin(), music.samplesL(), numFrames, highNorm);
	HighPassFilter(samplesL[1].begin(), samplesL[1].begin(), numFrames, lowNorm);

	LowPassFilter(samplesR[1].begin(), music.samplesR(), numFrames, highNorm);
	HighPassFilter(samplesR[1].begin(), samplesR[1].begin(), numFrames, lowNorm);

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
// WaveFilterCQT.

// Uses Gist to calculate CQT-like spectrum (Log-Frequency spectrogram).
struct WaveFilterCQT {

struct ColorPoint { uchar r, g, b; };
Vector<Vector<uchar>> intensitiesL; // [note][time_block]
Vector<Vector<uchar>> intensitiesR;
int timeBlockSize;
double samplesPerBlock;
bool dirty = true;

void update(float gain)
{
	if (!dirty) return;
	auto& music = gMusic->getSamples();
	if (!music.isCompleted()) return;

	int numFrames = music.getNumFrames();
	int samplerate = music.getFrequency();

	// CQT parameters
	const int frameSize = 4096;
	const int hopSize = 512; // Higher resolution in time
	const int minNote = 21;
	const int maxNote = 108;
	const int numNotes = maxNote - minNote + 1;

	timeBlockSize = hopSize;
	samplesPerBlock = (double)hopSize;

	int numBlocks = (numFrames - frameSize) / hopSize;
	if (numBlocks < 0) numBlocks = 0;

	intensitiesL.resize(numNotes);
	intensitiesR.resize(numNotes);
	for(int i=0; i<numNotes; ++i) {
		intensitiesL[i].resize(numBlocks);
		intensitiesR[i].resize(numBlocks);
	}

	Gist<float> gist(frameSize, samplerate);
	std::vector<float> audioFrame(frameSize);

	for(int channel=0; channel<2; ++channel) {
		const short* samples = (channel == 0) ? music.samplesL() : music.samplesR();
		auto& intensities = (channel == 0) ? intensitiesL : intensitiesR;

		for(int i=0; i<numBlocks; ++i) {
			int start = i * hopSize;
			for(int k=0; k<frameSize; ++k) {
				audioFrame[k] = samples[start+k] / 32768.0f;
			}

			gist.processAudioFrame(audioFrame);
			const auto& spectrum = gist.getMagnitudeSpectrum();
			int numBins = spectrum.size();

			for(int n=0; n<numNotes; ++n) {
				float noteFloat = (float)(minNote + n);
				float freq = 440.0f * pow(2.0f, (noteFloat - 69.0f) / 12.0f);
				int bin = (int)(freq * frameSize / samplerate);
				bin = clamp(bin, 0, numBins - 1);

				float val = spectrum[bin];
				val = log10(1 + val * gain) * 2.0f;
				val = clamp(val, 0.0f, 1.0f);

				intensities[n][i] = (uchar)(val * 255);
			}
		}
	}

	dirty = false;
}

}; // WaveFilterCQT.

// ================================================================================================
// WaveFilterChromagram.

// Chromagram: Folds CQT into 12 pitch classes.
struct WaveFilterChromagram {

Vector<Vector<uchar>> chromaL; // [chroma_idx][time_block]
Vector<Vector<uchar>> chromaR;
double samplesPerBlock;
bool dirty = true;

void update(float gain)
{
	if (!dirty) return;
	auto& music = gMusic->getSamples();
	if (!music.isCompleted()) return;

	int numFrames = music.getNumFrames();
	int samplerate = music.getFrequency();

	// Use same parameters as CQT for consistency, or use Gist's chromagram if available (Gist typically calculates it from Magnitude Spectrum)
	const int frameSize = 4096;
	const int hopSize = 512;
	samplesPerBlock = (double)hopSize;

	int numBlocks = (numFrames - frameSize) / hopSize;
	if (numBlocks < 0) numBlocks = 0;

	chromaL.resize(12);
	chromaR.resize(12);
	for(int i=0; i<12; ++i) {
		chromaL[i].resize(numBlocks);
		chromaR[i].resize(numBlocks);
	}

	Gist<float> gist(frameSize, samplerate);
	std::vector<float> audioFrame(frameSize);

	for(int channel=0; channel<2; ++channel) {
		const short* samples = (channel == 0) ? music.samplesL() : music.samplesR();
		auto& chroma = (channel == 0) ? chromaL : chromaR;

		for(int i=0; i<numBlocks; ++i) {
			int start = i * hopSize;
			for(int k=0; k<frameSize; ++k) {
				audioFrame[k] = samples[start+k] / 32768.0f;
			}

			gist.processAudioFrame(audioFrame);
			// Gist doesn't have built-in getChromagram(). We must calculate it from Spectrum.
			// Simple mapping: map each bin frequency to midi note, take modulo 12.
			const auto& spectrum = gist.getMagnitudeSpectrum();
			int numBins = spectrum.size();

			std::vector<float> chromaEnergy(12, 0.0f);

			for(int b=0; b<numBins; ++b) {
				float freq = b * samplerate / (float)frameSize;
				if (freq < 27.5f) continue; // Skip sub-A0

				float midiNote = 69.0f + 12.0f * log2(freq / 440.0f);
				int noteIndex = (int)round(midiNote);
				int chromaIdx = noteIndex % 12;
				if (chromaIdx < 0) chromaIdx += 12;

				chromaEnergy[chromaIdx] += spectrum[b];
			}

			// Normalize and scale
			float maxE = 0.0f;
			for(float e : chromaEnergy) maxE = max(maxE, e);

			for(int c=0; c<12; ++c) {
				float val = (maxE > 0) ? chromaEnergy[c] / maxE : 0.0f;
				val = log10(1 + val * gain) * 2.0f;
				val = clamp(val, 0.0f, 1.0f);
				chroma[c][i] = (uchar)(val * 255);
			}
		}
	}

	dirty = false;
}

}; // WaveFilterChromagram.

// ================================================================================================
// WaveFilterNovelty.

// Novelty/Flux Curve.
struct WaveFilterNovelty {

Vector<uchar> fluxL; // [time_block]
Vector<uchar> fluxR;
double samplesPerBlock;
bool dirty = true;

void update(float gain)
{
	if (!dirty) return;
	auto& music = gMusic->getSamples();
	if (!music.isCompleted()) return;

	int numFrames = music.getNumFrames();
	int samplerate = music.getFrequency();

	const int frameSize = 1024;
	const int hopSize = 256; // Higher resolution for timing
	samplesPerBlock = (double)hopSize;

	int numBlocks = (numFrames - frameSize) / hopSize;
	if (numBlocks < 0) numBlocks = 0;

	fluxL.resize(numBlocks);
	fluxR.resize(numBlocks);

	Gist<float> gist(frameSize, samplerate);
	std::vector<float> audioFrame(frameSize);

	for(int channel=0; channel<2; ++channel) {
		const short* samples = (channel == 0) ? music.samplesL() : music.samplesR();
		auto& flux = (channel == 0) ? fluxL : fluxR;

		for(int i=0; i<numBlocks; ++i) {
			int start = i * hopSize;
			for(int k=0; k<frameSize; ++k) {
				audioFrame[k] = samples[start+k] / 32768.0f;
			}

			gist.processAudioFrame(audioFrame);
			float val = gist.complexSpectralDifference(); // ODF

			val = log10(1 + val * gain) * 2.0f;
			val = clamp(val, 0.0f, 1.0f);
			flux[i] = (uchar)(val * 255);
		}
	}

	dirty = false;
}

}; // WaveFilterNovelty.

// ================================================================================================
// WaveFilterChromagram.
// ================================================================================================
// WaveFilterTempogram.

// Tempogram: Visualizes BPM probability over time using Autocorrelation of Flux.
struct WaveFilterTempogram {

Vector<Vector<uchar>> tempogramL; // [bpm_bin][time_block]
Vector<Vector<uchar>> tempogramR;
double samplesPerBlock;
bool dirty = true;
int minBPM = 60;
int maxBPM = 240;

void update(float gain)
{
	if (!dirty) return;
	auto& music = gMusic->getSamples();
	if (!music.isCompleted()) return;

	int numFrames = music.getNumFrames();
	int samplerate = music.getFrequency();

	// Parameters for Flux calculation
	const int frameSize = 1024;
	const int hopSize = 256;
	double fluxRate = (double)samplerate / hopSize; // ~172 Hz

	int numBlocks = (numFrames - frameSize) / hopSize;
	if (numBlocks < 0) numBlocks = 0;
	samplesPerBlock = (double)hopSize;

	// Compute Flux first (reuse logic from Novelty but keep local to be safe/independent)
	std::vector<float> flux(numBlocks);
	Gist<float> gist(frameSize, samplerate);
	std::vector<float> audioFrame(frameSize);

	// Compute flux for Left channel only for simplicity/speed (Tempo is usually mono)
	// Or average L+R
	const short* samplesL = music.samplesL();
	const short* samplesR = music.samplesR();

	for(int i=0; i<numBlocks; ++i) {
		int start = i * hopSize;
		for(int k=0; k<frameSize; ++k) {
			float s = (samplesL[start+k] + samplesR[start+k]) / 65536.0f;
			audioFrame[k] = s;
		}
		gist.processAudioFrame(audioFrame);
		flux[i] = gist.complexSpectralDifference();
	}

	// Tempogram parameters
	// Window size for autocorrelation: ~6 seconds?
	// 6 seconds * 172 Hz = ~1032 samples.
	int winSize = 1024;

	int numBins = maxBPM - minBPM; // 1 bin per BPM
	tempogramL.resize(numBins);
	tempogramR.resize(numBins); // Duplicate for display
	for(int i=0; i<numBins; ++i) {
		tempogramL[i].resize(numBlocks); // Store same resolution as flux
		tempogramR[i].resize(numBlocks);
	}

	std::vector<float> window(winSize);

	// Pre-calculate lags for BPMs
	std::vector<int> bpmLags(numBins);
	for(int b=0; b<numBins; ++b) {
		float bpm = (float)(minBPM + b);
		float bps = bpm / 60.0f;
		// lag in samples = fluxRate / bps
		bpmLags[b] = (int)(fluxRate / bps);
	}

	// Heavy loop warning!
	// Optimizing: only update every N blocks? No, we need full resolution.
	// But for 3 min song @ 172Hz = 30k blocks. 30k * 180 bins * window...
	// That's slow.
	// Let's optimize: only check lags that are within window.
	// Let's sub-sample the calculation. Calculate every 10th block, interpolate.

	int step = 10;

	for(int i=0; i<numBlocks; i+=step) {
		// Sliding window centered at i
		int start = i - winSize / 2;
		// Fill window, zero pad
		for(int k=0; k<winSize; ++k) {
			int idx = start + k;
			if (idx >= 0 && idx < numBlocks) window[k] = flux[idx];
			else window[k] = 0.0f;
		}

		float maxVal = 0.0f;
		std::vector<float> bpmEnergies(numBins);

		for(int b=0; b<numBins; ++b) {
			int lag = bpmLags[b];
			if (lag >= winSize) continue;

			float sum = 0.0f;
			// Only compute dot product where overlap exists
			// window[k] * window[k+lag]
			for(int k=0; k < winSize - lag; k+=4) { // Skip some samples for speed?
				sum += window[k] * window[k+lag];
			}
			bpmEnergies[b] = sum;
			if (sum > maxVal) maxVal = sum;
		}

		// Store and fill neighbors
		for(int b=0; b<numBins; ++b) {
			float val = (maxVal > 0) ? bpmEnergies[b] / maxVal : 0.0f;
			val *= gain / 50.0f; // Scale
			val = clamp(val, 0.0f, 1.0f);
			uchar u = (uchar)(val * 255);

			for(int s=0; s<step && (i+s) < numBlocks; ++s) {
				tempogramL[b][i+s] = u;
				tempogramR[b][i+s] = u;
			}
		}
	}

	dirty = false;
}

}; // WaveFilterTempogram.

// ================================================================================================
// WaveFilterHPSS.

// Harmonic-Percussive Source Separation
struct WaveFilterHPSS {

// Stores spectrogram magnitude [time][freq]
// We store compressed uint8 to save space: 0-255 maps to 0.0-1.0 magnitude
struct SpectrogramData {
	int width; // numFrames (time)
	int height; // numBins (freq)
	Vector<uchar> data; // row-major: data[time * height + freq]

	uchar get(int t, int f) const { return data[t * height + f]; }
	void set(int t, int f, uchar v) { data[t * height + f] = v; }
	void resize(int w, int h) { width = w; height = h; data.resize(w * h); }
};

SpectrogramData specP[2]; // Percussive (Left/Right)
SpectrogramData specH[2]; // Harmonic (Left/Right)
double samplesPerBlock;
bool dirty = true;

// Median filter helper
static uchar Median(std::vector<uchar>& v) {
	if (v.empty()) return 0;
	size_t n = v.size() / 2;
	std::nth_element(v.begin(), v.begin() + n, v.end());
	return v[n];
}

void update(float gain)
{
	if (!dirty) return;
	auto& music = gMusic->getSamples();
	if (!music.isCompleted()) return;

	int numFrames = music.getNumFrames();
	int samplerate = music.getFrequency();

	const int frameSize = 1024;
	const int hopSize = 512;
	samplesPerBlock = (double)hopSize;

	int numBlocks = (numFrames - frameSize) / hopSize;
	if (numBlocks <= 0) return;

	Gist<float> gist(frameSize, samplerate);
	std::vector<float> audioFrame(frameSize);

	// Temporary full float spectrogram for processing
	// [time][freq]
	int numBins = frameSize / 2;
	std::vector<std::vector<float>> S(numBlocks, std::vector<float>(numBins));

	for(int channel=0; channel<2; ++channel) {
		const short* samples = (channel == 0) ? music.samplesL() : music.samplesR();

		// 1. Compute Spectrogram
		for(int i=0; i<numBlocks; ++i) {
			int start = i * hopSize;
			for(int k=0; k<frameSize; ++k) audioFrame[k] = samples[start+k] / 32768.0f;
			gist.processAudioFrame(audioFrame);
			const auto& spectrum = gist.getMagnitudeSpectrum();
			for(int f=0; f<numBins; ++f) {
				S[i][f] = spectrum[f];
			}
		}

		// 2. Harmonic: Horizontal Median (Time)
		// Window size: ~0.2s. Hop 512 @ 44100 = 11ms. 0.2s = ~18 frames. Use 17.
		int wH = 17;
		int wP = 17; // Vertical Median (Freq). 17 bins * 43Hz = ~700Hz window.

		specP[channel].resize(numBlocks, numBins);
		specH[channel].resize(numBlocks, numBins);

		std::vector<uchar> buf;
		buf.reserve(32);

		for(int t=0; t<numBlocks; ++t) {
			for(int f=0; f<numBins; ++f) {
				float val = S[t][f];

				// Horizontal Median
				std::vector<float> winH;
				winH.reserve(wH);
				for(int k = -wH/2; k <= wH/2; ++k) {
					int tt = clamp(t+k, 0, numBlocks-1);
					winH.push_back(S[tt][f]);
				}
				size_t nH = winH.size()/2;
				std::nth_element(winH.begin(), winH.begin()+nH, winH.end());
				float H = winH[nH];

				// Vertical Median
				std::vector<float> winP;
				winP.reserve(wP);
				for(int k = -wP/2; k <= wP/2; ++k) {
					int ff = clamp(f+k, 0, numBins-1);
					winP.push_back(S[t][ff]);
				}
				size_t nP = winP.size()/2;
				std::nth_element(winP.begin(), winP.begin()+nP, winP.end());
				float P = winP[nP];

				// Masking (Soft or Binary)
				// Soft mask: M_p = P^2 / (P^2 + H^2)
				// Let's use simple Wiener-like masking
				// P_component = S * (P / (P + H + epsilon))
				float epsilon = 1e-6f;
				float maskP = P / (P + H + epsilon);
				float maskH = H / (P + H + epsilon);

				float valP = val * maskP;
				float valH = val * maskH;

				// Log scale for display
				valP = log10(1 + valP * gain) * 2.0f;
				valH = log10(1 + valH * gain) * 2.0f;

				specP[channel].set(t, f, (uchar)clamp(valP * 255.0f, 0.0f, 255.0f));
				specH[channel].set(t, f, (uchar)clamp(valH * 255.0f, 0.0f, 255.0f));
			}
		}
	}

	dirty = false;
}

}; // WaveFilterHPSS.

// ================================================================================================
// WaveformImpl :: member data.

struct WaveformImpl : public Waveform {

Vector<WaveBlock*> waveformBlocks_;

WaveFilter* waveformFilter_;
WaveFilterRGB* waveformFilterRGB_;
WaveFilterSpectral* waveformFilterSpectral_;
WaveFilterPitch* waveformFilterPitch_;
WaveFilterCQT* waveformFilterCQT_;
WaveFilterHPSS* waveformFilterHPSS_;
WaveFilterChromagram* waveformFilterChromagram_;
WaveFilterNovelty* waveformFilterNovelty_;
WaveFilterTempogram* waveformFilterTempogram_;
Vector<uchar> waveformTextureBuffer_;

int waveformBlockWidth_, waveformSpacing_;

ColorScheme waveformColorScheme_;
WaveShape waveformShape_;
Luminance waveformLuminance_;
ColorMode waveformColorMode_;
int waveformAntiAliasingMode_;
bool waveformOverlayFilter_;
bool waveformShowOnsets_;
float waveformOnsetThreshold_;
float waveformSpectrogramGain_;
float waveformRGBLow_;
float waveformRGBHigh_;
Vector<Onset> waveformOnsets_;
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
	delete waveformFilterCQT_;
	delete waveformFilterHPSS_;
	delete waveformFilterChromagram_;
	delete waveformFilterNovelty_;
	delete waveformFilterTempogram_;
}

WaveformImpl()
{
	setPreset(PRESET_VORTEX);

	waveformFilter_ = nullptr;
	waveformFilterRGB_ = new WaveFilterRGB();
	waveformFilterSpectral_ = new WaveFilterSpectral();
	waveformFilterPitch_ = new WaveFilterPitch();
	waveformFilterCQT_ = new WaveFilterCQT();
	waveformFilterHPSS_ = new WaveFilterHPSS();
	waveformFilterChromagram_ = new WaveFilterChromagram();
	waveformFilterNovelty_ = new WaveFilterNovelty();
	waveformFilterTempogram_ = new WaveFilterTempogram();
	waveformOverlayFilter_ = true;
	waveformShowOnsets_ = false;
	waveformOnsetThreshold_ = 0.3f;
	waveformSpectrogramGain_ = 1000.0f;
	waveformRGBLow_ = 250.0f;
	waveformRGBHigh_ = 2500.0f;

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
	if(mode == CM_CQT) return "cqt";
	if(mode == CM_PERCUSSION) return "percussion";
	if(mode == CM_HARMONIC) return "harmonic";
	if(mode == CM_CHROMAGRAM) return "chromagram";
	if(mode == CM_NOVELTY) return "novelty";
	if(mode == CM_TEMPOGRAM) return "tempogram";
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
	if(str == "cqt") return CM_CQT;
	if(str == "percussion") return CM_PERCUSSION;
	if(str == "harmonic") return CM_HARMONIC;
	if(str == "chromagram") return CM_CHROMAGRAM;
	if(str == "novelty") return CM_NOVELTY;
	if(str == "tempogram") return CM_TEMPOGRAM;
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

		waveform->get("showOnsets", &waveformShowOnsets_);
		waveform->get("onsetThreshold", &waveformOnsetThreshold_);
		waveform->get("spectrogramGain", &waveformSpectrogramGain_);
		waveform->get("rgbLow", &waveformRGBLow_);
		waveform->get("rgbHigh", &waveformRGBHigh_);
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
	waveform->addAttrib("showOnsets", waveformShowOnsets_);
	waveform->addAttrib("onsetThreshold", waveformOnsetThreshold_);
	waveform->addAttrib("spectrogramGain", waveformSpectrogramGain_);
	waveform->addAttrib("rgbLow", waveformRGBLow_);
	waveform->addAttrib("rgbHigh", waveformRGBHigh_);
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
		waveformFilterCQT_->dirty = true;
		if(waveformColorMode_ == CM_RGB) waveformFilterRGB_->update(waveformRGBLow_, waveformRGBHigh_);
		if(waveformColorMode_ == CM_SPECTRAL) waveformFilterSpectral_->update();
		if(waveformColorMode_ == CM_PITCH) waveformFilterPitch_->update();
		if(waveformColorMode_ == CM_CQT) waveformFilterCQT_->update(waveformSpectrogramGain_);
		if(waveformColorMode_ == CM_CHROMAGRAM) waveformFilterChromagram_->update(waveformSpectrogramGain_);
		if(waveformColorMode_ == CM_NOVELTY) waveformFilterNovelty_->update(waveformSpectrogramGain_);
		if(waveformColorMode_ == CM_TEMPOGRAM) waveformFilterTempogram_->update(waveformSpectrogramGain_);
		if(waveformColorMode_ == CM_PERCUSSION || waveformColorMode_ == CM_HARMONIC) waveformFilterHPSS_->update(waveformSpectrogramGain_);

		// Update onsets
		updateOnsets();
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
		waveformShowOnsets_ = false;
		break;
	case PRESET_DDREAM:
		waveformColorScheme_.bg = {0.45f, 0.6f, 0.11f, 0.8f};
		waveformColorScheme_.wave = {0.9f, 0.9f, 0.33f, 1};
		waveformColorScheme_.filter = {0.8f, 0.3f, 0.3f, 1};
		waveformShape_ = WS_RECTIFIED;
		waveformLuminance_ = LL_UNIFORM;
		waveformColorMode_ = CM_FLAT;
		waveformAntiAliasingMode_ = 1;
		waveformShowOnsets_ = true; // Enable onsets for DDream preset
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
		waveformFilterRGB_->update(waveformRGBLow_, waveformRGBHigh_);
	}
	else if (mode == CM_SPECTRAL) {
		waveformFilterSpectral_->update();
	}
	else if (mode == CM_PITCH) {
		waveformFilterPitch_->update();
	}
	else if (mode == CM_CQT) {
		waveformFilterCQT_->update(waveformSpectrogramGain_);
	}
	else if (mode == CM_CHROMAGRAM) {
		waveformFilterChromagram_->update(waveformSpectrogramGain_);
	}
	else if (mode == CM_NOVELTY) {
		waveformFilterNovelty_->update(waveformSpectrogramGain_);
	}
	else if (mode == CM_TEMPOGRAM) {
		waveformFilterTempogram_->update(waveformSpectrogramGain_);
	}
	else if (mode == CM_PERCUSSION || mode == CM_HARMONIC) {
		waveformFilterHPSS_->update(waveformSpectrogramGain_);
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

void setShowOnsets(bool show)
{
	waveformShowOnsets_ = show;
}

bool hasShowOnsets()
{
	return waveformShowOnsets_;
}

void setOnsetThreshold(float threshold)
{
	if (waveformOnsetThreshold_ != threshold)
	{
		waveformOnsetThreshold_ = threshold;
		updateOnsets();
	}
}

float getOnsetThreshold()
{
	return waveformOnsetThreshold_;
}

void setSpectrogramGain(float gain)
{
	if (waveformSpectrogramGain_ != gain)
	{
		waveformSpectrogramGain_ = gain;
		clearBlocks();
	}
}

float getSpectrogramGain()
{
	return waveformSpectrogramGain_;
}

void setRGBCrossovers(float low, float high)
{
	if (waveformRGBLow_ != low || waveformRGBHigh_ != high)
	{
		waveformRGBLow_ = low;
		waveformRGBHigh_ = high;
		if (waveformColorMode_ == CM_RGB)
		{
			waveformFilterRGB_->dirty = true;
			waveformFilterRGB_->update(waveformRGBLow_, waveformRGBHigh_);
			clearBlocks();
		}
	}
}

float getRGBLowHigh(bool high)
{
	return high ? waveformRGBHigh_ : waveformRGBLow_;
}

void updateOnsets()
{
	waveformOnsets_.clear();
	auto& music = gMusic->getSamples();
	if(music.isCompleted() && music.getNumFrames() > 0)
	{
		// Convert to float buffer for FindOnsets
		int numFrames = music.getNumFrames();
		int samplerate = music.getFrequency();
		std::vector<float> floatSamples(numFrames);
		const short* src = music.samplesL(); // Use Left channel for onsets
		for(int i=0; i<numFrames; ++i) {
			floatSamples[i] = src[i] / 32768.0f;
		}

		FindOnsets(floatSamples.data(), samplerate, numFrames, 1, waveformOnsets_, waveformOnsetThreshold_);
	}
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

float getSpectrogramGain()
{
	return waveformSpectrogramGain_;
}

void setRGBCrossovers(float low, float high)
{
	if (waveformRGBLow_ != low || waveformRGBHigh_ != high)
	{
		waveformRGBLow_ = low;
		waveformRGBHigh_ = high;
		if (waveformColorMode_ == CM_RGB)
		{
			waveformFilterRGB_->dirty = true;
			waveformFilterRGB_->update(waveformRGBLow_, waveformRGBHigh_);
			clearBlocks();
		}
	}
}

float getRGBLowHigh(bool high)
{
	return high ? waveformRGBHigh_ : waveformRGBLow_;
}

void updateOnsets()
{
	waveformOnsets_.clear();
	auto& music = gMusic->getSamples();
	if(music.isCompleted() && music.getNumFrames() > 0)
	{
		// Convert to float buffer for FindOnsets
		int numFrames = music.getNumFrames();
		int samplerate = music.getFrequency();
		std::vector<float> floatSamples(numFrames);
		const short* src = music.samplesL(); // Use Left channel for onsets
		for(int i=0; i<numFrames; ++i) {
			floatSamples[i] = src[i] / 32768.0f;
		}

		FindOnsets(floatSamples.data(), samplerate, numFrames, 1, waveformOnsets_, waveformOnsetThreshold_);
	}
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

void renderWaveformSpectrogram(Texture* textures, WaveEdge* edgeBuf, int w, int h, int blockId)
{
	uchar* texBuf = waveformTextureBuffer_.begin();

	auto& music = gMusic->getSamples();
	double samplesPerSec = (double)music.getFrequency();
	double samplesPerPixel = samplesPerSec / fabs(gView->getPixPerSec());
	double samplesPerBlock = (double)TEX_H * samplesPerPixel;
	int64_t samplePos = max((int64_t)0, (int64_t)(samplesPerBlock * (double)blockId));

	// We use Gist for each column (line) of the texture
	const int frameSize = 1024;
	Gist<float> gist(frameSize, music.getFrequency());
	std::vector<float> audioFrame(frameSize);

	int texW = w / 2; // Texture width is half of total width (stereo split) but actually w is width of block texture?
	// The texture is TEX_W x TEX_H. w passed in is width including antialiasing multiplier.
	// But wait, w is for texture buffer size.
	// renderBlock calculates w = waveformBlockWidth_ * (AA + 1).
	// And the texture is modified with that size?
	// renderWaveform uses w for sampleEdges loop.

	// For spectrogram, Y axis of texture is Frequency. X axis is Time.
	// We need to fill texBuf (size w * h * 4).
	// w is typically 256 * (AA+1). h is 128 * (AA+1).

	// To keep it simple and fast, let's ignore AA for spectrogram content (or do simple linear interpolation).
	// Actually, we should just calculate one FFT per output column.

	double advance = samplesPerPixel * TEX_H / h; // Samples per vertical pixel in the final texture?
	// Wait, the texture is oriented vertically in the game view?
	// The waveform is drawn vertically. Texture X is Time?
	// In standard render:
	// for(int y = 0; y < h; ++y) ... determines the line.
	// Texture coordinate u (0..1) maps to width of block. v (0..1) maps to height (Time).
	// So H is Time dimension. W is Amplitude dimension.
	// For Spectrogram, we want H (Time) x W (Frequency).

	// So for each row y (Time), we compute FFT.
	// Then we map Frequency bins to columns x.

	memset(texBuf, 0, w * h * 4);

	for(int channel = 0; channel < 2; ++channel)
	{
		const short* samples = (channel == 0) ? music.samplesL() : music.samplesR();
		int64_t totalFrames = music.getNumFrames();

		for(int y = 0; y < h; ++y)
		{
			int64_t currentSample = samplePos + (int64_t)(y * advance);
			if (currentSample >= totalFrames) break;

			// Extract frame centered at currentSample
			int start = currentSample - frameSize / 2;
			for(int k=0; k<frameSize; ++k) {
				if (start + k >= 0 && start + k < totalFrames) {
					audioFrame[k] = samples[start+k] / 32768.0f;
				} else {
					audioFrame[k] = 0.0f;
				}
			}

			gist.processAudioFrame(audioFrame);
			const auto& magSpec = gist.getMelFrequencySpectrum();
			// Mel spectrum is better for visualization. Gist default mel bank size is usually 40?
			// Let's check Gist.h... getMelFrequencySpectrum returns vector.
			// If it's small, we upscale. If we use getMagnitudeSpectrum(), it's frameSize/2 (512).
			// w is around 256 or 512.

			const auto& spectrum = gist.getMagnitudeSpectrum();
			int numBins = spectrum.size();

			// We map bins to x (0..w).
			// Standard waveform is centered at w/2.
			// Spectrogram usually goes low->high freq.
			// Let's draw: Center = Low Freq (Bass). Edges = High Freq.
			// This mimics the waveform shape (amplitude highest in center).
			// Or: Left = Low, Right = High? No, w is the width of the notefield lane basically.
			// If we duplicate (mirror) it looks like a waveform.
			// Center (x=w/2) = 0Hz. x=0 and x=w = Nyquist.

			int cx = w / 2;
			for(int x = 0; x < cx; ++x)
			{
				// Map x to frequency bin (logarithmic looks better but linear is easier first)
				// Linear mapping: bin = x * numBins / cx;
				// Log mapping:
				float t = (float)x / cx; // 0..1 (Low..High)
				// We want more resolution at low freqs.
				// bin = numBins * (pow(100, t) - 1) / 99?
				// Let's stick to linear for now or simple log.

				int bin = (int)(t * t * numBins); // Quadratic
				bin = clamp(bin, 0, numBins-1);

				float val = spectrum[bin];
				val = log10(1 + val * waveformSpectrogramGain_) * 2.0f; // Scale for visibility
				val = clamp(val, 0.0f, 1.0f);

				uchar intensity = (uchar)(val * 255);

				// Magma-like colormap (Black -> Red -> Yellow -> White)
				uchar r = intensity;
				uchar g = (intensity > 128) ? (intensity - 128) * 2 : 0;
				uchar b = (intensity > 192) ? (intensity - 192) * 4 : 0;

				// Mirror
				int l = cx - 1 - x;
				int r_pos = cx + x;

				if(l >= 0) {
					texBuf[(y * w + l) * 4 + 0] = r;
					texBuf[(y * w + l) * 4 + 1] = g;
					texBuf[(y * w + l) * 4 + 2] = b;
					texBuf[(y * w + l) * 4 + 3] = intensity; // Use intensity as alpha? Or full opaque?
					// Standard waveform uses alpha for shape.
					// If we use full opaque, we fill the box.
					// Let's use intensity as alpha to blend with background.
				}
				if(r_pos < w) {
					texBuf[(y * w + r_pos) * 4 + 0] = r;
					texBuf[(y * w + r_pos) * 4 + 1] = g;
					texBuf[(y * w + r_pos) * 4 + 2] = b;
					texBuf[(y * w + r_pos) * 4 + 3] = intensity;
				}
			}
		}

		// Apply anti-aliasing
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

void renderWaveformChromagram(Texture* textures, WaveEdge* edgeBuf, int w, int h, int blockId)
{
	uchar* texBuf = waveformTextureBuffer_.begin();

	auto& music = gMusic->getSamples();
	double samplesPerSec = (double)music.getFrequency();
	double samplesPerPixel = samplesPerSec / fabs(gView->getPixPerSec());
	double samplesPerBlock = (double)TEX_H * samplesPerPixel;
	int64_t samplePos = max((int64_t)0, (int64_t)(samplesPerBlock * (double)blockId));

	double advance = samplesPerPixel * TEX_H / h;

	memset(texBuf, 0, w * h * 4);

	for(int channel = 0; channel < 2; ++channel)
	{
		const auto& chroma = (channel == 0) ? waveformFilterChromagram_->chromaL : waveformFilterChromagram_->chromaR;
		int numBlocks = chroma.empty() ? 0 : chroma[0].size();

		for(int y = 0; y < h; ++y)
		{
			int64_t currentSample = samplePos + (int64_t)(y * advance);
			int blockIndex = (int)(currentSample / waveformFilterChromagram_->samplesPerBlock);

			if (blockIndex < 0 || blockIndex >= numBlocks) continue;

			int cx = w / 2;

			// Draw chromagram (12 bins). Map x to bin.
			for(int x = 0; x < cx; ++x)
			{
				float t = (float)x / cx; // 0..1
				int bin = (int)(t * 12);
				bin = clamp(bin, 0, 11);

				uchar intensity = chroma[bin][blockIndex];

				// Map bin to hue (Circle of Fifths or Chromatic?)
				// Chromatic: C=Red, C#=Orange...
				float hue = bin * 30.0f; // 360 / 12

				float s = 0.8f;
				float v = intensity / 255.0f;

				float c = v * s;
				float hh = hue / 60.0f;
				float xx = c * (1 - fabs(fmod(hh, 2) - 1));
				float r=0, g=0, b=0;

				if (hh < 1) { r=c; g=xx; b=0; }
				else if (hh < 2) { r=xx; g=c; b=0; }
				else if (hh < 3) { r=0; g=c; b=xx; }
				else if (hh < 4) { r=0; g=xx; b=c; }
				else if (hh < 5) { r=xx; g=0; b=c; }
				else { r=c; g=0; b=xx; }

				uchar R = (uchar)(r * 255);
				uchar G = (uchar)(g * 255);
				uchar B = (uchar)(b * 255);

				// Mirror
				int l = cx - 1 - x;
				int r_pos = cx + x;

				if(l >= 0) {
					texBuf[(y * w + l) * 4 + 0] = R;
					texBuf[(y * w + l) * 4 + 1] = G;
					texBuf[(y * w + l) * 4 + 2] = B;
					texBuf[(y * w + l) * 4 + 3] = intensity;
				}
				if(r_pos < w) {
					texBuf[(y * w + r_pos) * 4 + 0] = R;
					texBuf[(y * w + r_pos) * 4 + 1] = G;
					texBuf[(y * w + r_pos) * 4 + 2] = B;
					texBuf[(y * w + r_pos) * 4 + 3] = intensity;
				}
			}
		}

		// Apply anti-aliasing
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

void renderWaveformNovelty(Texture* textures, WaveEdge* edgeBuf, int w, int h, int blockId)
{
	uchar* texBuf = waveformTextureBuffer_.begin();

	auto& music = gMusic->getSamples();
	double samplesPerSec = (double)music.getFrequency();
	double samplesPerPixel = samplesPerSec / fabs(gView->getPixPerSec());
	double samplesPerBlock = (double)TEX_H * samplesPerPixel;
	int64_t samplePos = max((int64_t)0, (int64_t)(samplesPerBlock * (double)blockId));

	double advance = samplesPerPixel * TEX_H / h;

	memset(texBuf, 0, w * h * 4);

	for(int channel = 0; channel < 2; ++channel)
	{
		const auto& flux = (channel == 0) ? waveformFilterNovelty_->fluxL : waveformFilterNovelty_->fluxR;
		int numBlocks = flux.size();

		for(int y = 0; y < h; ++y)
		{
			int64_t currentSample = samplePos + (int64_t)(y * advance);
			int blockIndex = (int)(currentSample / waveformFilterNovelty_->samplesPerBlock);

			if (blockIndex < 0 || blockIndex >= numBlocks) continue;

			int cx = w / 2;
			uchar intensity = flux[blockIndex];

			// Draw flux as a solid block width proportional to intensity
			int width = (int)(intensity / 255.0f * cx);

			for(int x = 0; x < cx; ++x)
			{
				// Color: Green -> Red (Low -> High Novelty)
				uchar r = intensity;
				uchar g = 255 - intensity;
				uchar b = 0;
				uchar a = (x <= width) ? 255 : 0;

				if (a == 0) { r=0; g=0; b=0; }

				// Mirror
				int l = cx - 1 - x;
				int r_pos = cx + x;

				if(l >= 0) {
					texBuf[(y * w + l) * 4 + 0] = r;
					texBuf[(y * w + l) * 4 + 1] = g;
					texBuf[(y * w + l) * 4 + 2] = b;
					texBuf[(y * w + l) * 4 + 3] = a;
				}
				if(r_pos < w) {
					texBuf[(y * w + r_pos) * 4 + 0] = r;
					texBuf[(y * w + r_pos) * 4 + 1] = g;
					texBuf[(y * w + r_pos) * 4 + 2] = b;
					texBuf[(y * w + r_pos) * 4 + 3] = a;
				}
			}
		}

		// Apply anti-aliasing
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

void renderWaveformChromagram(Texture* textures, WaveEdge* edgeBuf, int w, int h, int blockId)
{
	uchar* texBuf = waveformTextureBuffer_.begin();

	auto& music = gMusic->getSamples();
	double samplesPerSec = (double)music.getFrequency();
	double samplesPerPixel = samplesPerSec / fabs(gView->getPixPerSec());
	double samplesPerBlock = (double)TEX_H * samplesPerPixel;
	int64_t samplePos = max((int64_t)0, (int64_t)(samplesPerBlock * (double)blockId));

	double advance = samplesPerPixel * TEX_H / h;

	memset(texBuf, 0, w * h * 4);

	for(int channel = 0; channel < 2; ++channel)
	{
		const auto& chroma = (channel == 0) ? waveformFilterChromagram_->chromaL : waveformFilterChromagram_->chromaR;
		int numBlocks = chroma.empty() ? 0 : chroma[0].size();

		for(int y = 0; y < h; ++y)
		{
			int64_t currentSample = samplePos + (int64_t)(y * advance);
			int blockIndex = (int)(currentSample / waveformFilterChromagram_->samplesPerBlock);

			if (blockIndex < 0 || blockIndex >= numBlocks) continue;

			int cx = w / 2;

			// Draw chromagram (12 bins). Map x to bin.
			for(int x = 0; x < cx; ++x)
			{
				float t = (float)x / cx; // 0..1
				int bin = (int)(t * 12);
				bin = clamp(bin, 0, 11);

				uchar intensity = chroma[bin][blockIndex];

				// Map bin to hue (Circle of Fifths or Chromatic?)
				// Chromatic: C=Red, C#=Orange...
				float hue = bin * 30.0f; // 360 / 12

				float s = 0.8f;
				float v = intensity / 255.0f;

				float c = v * s;
				float hh = hue / 60.0f;
				float xx = c * (1 - fabs(fmod(hh, 2) - 1));
				float r=0, g=0, b=0;

				if (hh < 1) { r=c; g=xx; b=0; }
				else if (hh < 2) { r=xx; g=c; b=0; }
				else if (hh < 3) { r=0; g=c; b=xx; }
				else if (hh < 4) { r=0; g=xx; b=c; }
				else if (hh < 5) { r=xx; g=0; b=c; }
				else { r=c; g=0; b=xx; }

				uchar R = (uchar)(r * 255);
				uchar G = (uchar)(g * 255);
				uchar B = (uchar)(b * 255);

				// Mirror
				int l = cx - 1 - x;
				int r_pos = cx + x;

				if(l >= 0) {
					texBuf[(y * w + l) * 4 + 0] = R;
					texBuf[(y * w + l) * 4 + 1] = G;
					texBuf[(y * w + l) * 4 + 2] = B;
					texBuf[(y * w + l) * 4 + 3] = intensity;
				}
				if(r_pos < w) {
					texBuf[(y * w + r_pos) * 4 + 0] = R;
					texBuf[(y * w + r_pos) * 4 + 1] = G;
					texBuf[(y * w + r_pos) * 4 + 2] = B;
					texBuf[(y * w + r_pos) * 4 + 3] = intensity;
				}
			}
		}

		// Apply anti-aliasing
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

void renderWaveformNovelty(Texture* textures, WaveEdge* edgeBuf, int w, int h, int blockId)
{
	uchar* texBuf = waveformTextureBuffer_.begin();

	auto& music = gMusic->getSamples();
	double samplesPerSec = (double)music.getFrequency();
	double samplesPerPixel = samplesPerSec / fabs(gView->getPixPerSec());
	double samplesPerBlock = (double)TEX_H * samplesPerPixel;
	int64_t samplePos = max((int64_t)0, (int64_t)(samplesPerBlock * (double)blockId));

	double advance = samplesPerPixel * TEX_H / h;

	memset(texBuf, 0, w * h * 4);

	for(int channel = 0; channel < 2; ++channel)
	{
		const auto& flux = (channel == 0) ? waveformFilterNovelty_->fluxL : waveformFilterNovelty_->fluxR;
		int numBlocks = flux.size();

		for(int y = 0; y < h; ++y)
		{
			int64_t currentSample = samplePos + (int64_t)(y * advance);
			int blockIndex = (int)(currentSample / waveformFilterNovelty_->samplesPerBlock);

			if (blockIndex < 0 || blockIndex >= numBlocks) continue;

			int cx = w / 2;
			uchar intensity = flux[blockIndex];

			// Draw flux as a solid block width proportional to intensity
			int width = (int)(intensity / 255.0f * cx);

			for(int x = 0; x < cx; ++x)
			{
				// Color: Green -> Red (Low -> High Novelty)
				uchar r = intensity;
				uchar g = 255 - intensity;
				uchar b = 0;
				uchar a = (x <= width) ? 255 : 0;

				if (a == 0) { r=0; g=0; b=0; }

				// Mirror
				int l = cx - 1 - x;
				int r_pos = cx + x;

				if(l >= 0) {
					texBuf[(y * w + l) * 4 + 0] = r;
					texBuf[(y * w + l) * 4 + 1] = g;
					texBuf[(y * w + l) * 4 + 2] = b;
					texBuf[(y * w + l) * 4 + 3] = a;
				}
				if(r_pos < w) {
					texBuf[(y * w + r_pos) * 4 + 0] = r;
					texBuf[(y * w + r_pos) * 4 + 1] = g;
					texBuf[(y * w + r_pos) * 4 + 2] = b;
					texBuf[(y * w + r_pos) * 4 + 3] = a;
				}
			}
		}

		// Apply anti-aliasing
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

void renderWaveformCQT(Texture* textures, WaveEdge* edgeBuf, int w, int h, int blockId)
{
	uchar* texBuf = waveformTextureBuffer_.begin();

	auto& music = gMusic->getSamples();
	double samplesPerSec = (double)music.getFrequency();
	double samplesPerPixel = samplesPerSec / fabs(gView->getPixPerSec());
	double samplesPerBlock = (double)TEX_H * samplesPerPixel;
	int64_t samplePos = max((int64_t)0, (int64_t)(samplesPerBlock * (double)blockId));

	double advance = samplesPerPixel * TEX_H / h;

	memset(texBuf, 0, w * h * 4);

	const int minNote = 21;
	const int maxNote = 108;
	const int numNotes = maxNote - minNote + 1;

	for(int channel = 0; channel < 2; ++channel)
	{
		const auto& intensities = (channel == 0) ? waveformFilterCQT_->intensitiesL : waveformFilterCQT_->intensitiesR;
		int numBlocks = intensities.empty() ? 0 : intensities[0].size();

		for(int y = 0; y < h; ++y)
		{
			int64_t currentSample = samplePos + (int64_t)(y * advance);
			int blockIndex = (int)(currentSample / waveformFilterCQT_->samplesPerBlock);

			if (blockIndex < 0 || blockIndex >= numBlocks) continue;

			int cx = w / 2;

			for(int x = 0; x < cx; ++x)
			{
				float t = (float)x / cx; // 0..1
				float noteFloat = minNote + t * (maxNote - minNote);
				int note = (int)noteFloat;

				if (note < minNote || note > maxNote) continue;
				int noteIdx = note - minNote;
				if (noteIdx >= numNotes) continue;

				uchar intensity = intensities[noteIdx][blockIndex];

				float chroma = fmod(noteFloat, 12.0f);
				float hue = chroma * 30.0f;

				float s = 0.8f;
				float v = intensity / 255.0f;

				float c = v * s;
				float hh = hue / 60.0f;
				float xx = c * (1 - fabs(fmod(hh, 2) - 1));
				float r=0, g=0, b=0;

				if (hh < 1) { r=c; g=xx; b=0; }
				else if (hh < 2) { r=xx; g=c; b=0; }
				else if (hh < 3) { r=0; g=c; b=xx; }
				else if (hh < 4) { r=0; g=xx; b=c; }
				else if (hh < 5) { r=xx; g=0; b=c; }
				else { r=c; g=0; b=xx; }

				uchar R = (uchar)(r * 255);
				uchar G = (uchar)(g * 255);
				uchar B = (uchar)(b * 255);

				// Mirror
				int l = cx - 1 - x;
				int r_pos = cx + x;

				if(l >= 0) {
					texBuf[(y * w + l) * 4 + 0] = R;
					texBuf[(y * w + l) * 4 + 1] = G;
					texBuf[(y * w + l) * 4 + 2] = B;
					texBuf[(y * w + l) * 4 + 3] = intensity;
				}
				if(r_pos < w) {
					texBuf[(y * w + r_pos) * 4 + 0] = R;
					texBuf[(y * w + r_pos) * 4 + 1] = G;
					texBuf[(y * w + r_pos) * 4 + 2] = B;
					texBuf[(y * w + r_pos) * 4 + 3] = intensity;
				}
			}
		}

		// Apply anti-aliasing
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

void renderWaveformTempogram(Texture* textures, WaveEdge* edgeBuf, int w, int h, int blockId)
{
	uchar* texBuf = waveformTextureBuffer_.begin();

	auto& music = gMusic->getSamples();
	double samplesPerSec = (double)music.getFrequency();
	double samplesPerPixel = samplesPerSec / fabs(gView->getPixPerSec());
	double samplesPerBlock = (double)TEX_H * samplesPerPixel;
	int64_t samplePos = max((int64_t)0, (int64_t)(samplesPerBlock * (double)blockId));

	double advance = samplesPerPixel * TEX_H / h;

	memset(texBuf, 0, w * h * 4);

	for(int channel = 0; channel < 2; ++channel)
	{
		const auto& tempogram = (channel == 0) ? waveformFilterTempogram_->tempogramL : waveformFilterTempogram_->tempogramR;
		int numBins = tempogram.size();
		int numBlocks = (numBins > 0) ? tempogram[0].size() : 0;

		for(int y = 0; y < h; ++y)
		{
			int64_t currentSample = samplePos + (int64_t)(y * advance);
			int blockIndex = (int)(currentSample / waveformFilterTempogram_->samplesPerBlock);

			if (blockIndex < 0 || blockIndex >= numBlocks) continue;

			int cx = w / 2;

			for(int x = 0; x < cx; ++x)
			{
				float t = (float)x / cx; // 0..1
				int bin = (int)(t * numBins);
				bin = clamp(bin, 0, numBins-1);

				uchar intensity = tempogram[bin][blockIndex];

				// Heatmap: Blue->Red->Yellow
				uchar r, g, b;
				if (intensity < 128) {
					// Blue to Green
					r = 0;
					g = intensity * 2;
					b = 255 - intensity * 2;
				} else {
					// Green to Red
					r = (intensity - 128) * 2;
					g = 255 - (intensity - 128) * 2;
					b = 0;
				}

				int l = cx - 1 - x;
				int r_pos = cx + x;

				if(l >= 0) {
					texBuf[(y * w + l) * 4 + 0] = r;
					texBuf[(y * w + l) * 4 + 1] = g;
					texBuf[(y * w + l) * 4 + 2] = b;
					texBuf[(y * w + l) * 4 + 3] = intensity;
				}
				if(r_pos < w) {
					texBuf[(y * w + r_pos) * 4 + 0] = r;
					texBuf[(y * w + r_pos) * 4 + 1] = g;
					texBuf[(y * w + r_pos) * 4 + 2] = b;
					texBuf[(y * w + r_pos) * 4 + 3] = intensity;
				}
			}
		}

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

void renderWaveformHPSS(Texture* textures, WaveEdge* edgeBuf, int w, int h, int blockId, bool harmonic)
{
	uchar* texBuf = waveformTextureBuffer_.begin();

	auto& music = gMusic->getSamples();
	double samplesPerSec = (double)music.getFrequency();
	double samplesPerPixel = samplesPerSec / fabs(gView->getPixPerSec());
	double samplesPerBlock = (double)TEX_H * samplesPerPixel;
	int64_t samplePos = max((int64_t)0, (int64_t)(samplesPerBlock * (double)blockId));

	double advance = samplesPerPixel * TEX_H / h;

	memset(texBuf, 0, w * h * 4);

	for(int channel = 0; channel < 2; ++channel)
	{
		const auto& spec = harmonic ? waveformFilterHPSS_->specH[channel] : waveformFilterHPSS_->specP[channel];
		if (spec.width == 0) continue;

		for(int y = 0; y < h; ++y)
		{
			int64_t currentSample = samplePos + (int64_t)(y * advance);
			// Calculate time index in spectrogram
			int t = (int)(currentSample / waveformFilterHPSS_->samplesPerBlock);
			if (t < 0 || t >= spec.width) continue;

			int cx = w / 2;
			int numBins = spec.height;

			for(int x = 0; x < cx; ++x)
			{
				float u = (float)x / cx;
				// Linear mapping for now to match spectrogram
				int bin = (int)(u * u * numBins);
				bin = clamp(bin, 0, numBins-1);

				uchar intensity = spec.get(t, bin);

				// Color map
				// Harmonic: Blue/Greenish? Percussive: Red/Orange?
				uchar r, g, b;
				if (harmonic) {
					// Cyan/Blue
					r = 0;
					g = intensity;
					b = (intensity > 128) ? 255 : intensity * 2;
				} else {
					// Red/Orange
					r = intensity;
					g = (intensity > 128) ? (intensity - 128) * 2 : 0;
					b = 0;
				}

				// Mirror
				int l = cx - 1 - x;
				int r_pos = cx + x;

				if(l >= 0) {
					texBuf[(y * w + l) * 4 + 0] = r;
					texBuf[(y * w + l) * 4 + 1] = g;
					texBuf[(y * w + l) * 4 + 2] = b;
					texBuf[(y * w + l) * 4 + 3] = intensity;
				}
				if(r_pos < w) {
					texBuf[(y * w + r_pos) * 4 + 0] = r;
					texBuf[(y * w + r_pos) * 4 + 1] = g;
					texBuf[(y * w + r_pos) * 4 + 2] = b;
					texBuf[(y * w + r_pos) * 4 + 3] = intensity;
				}
			}
		}

		// Apply anti-aliasing
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
		waveformFilterRGB_->update(waveformRGBLow_, waveformRGBHigh_);
		renderWaveformRGB(block->tex, edges.begin(), w, h, block->id);
	}
	else if(waveformColorMode_ == CM_SPECTRAL)
	{
		waveformFilterSpectral_->update();
		renderWaveformColored(block->tex, edges.begin(), w, h, block->id, waveformFilterSpectral_);
	}
	else if(waveformColorMode_ == CM_SPECTROGRAM)
	{
		waveformFilterSpectral_->update();
		renderWaveformSpectrogram(block->tex, edges.begin(), w, h, block->id);
	}
	else if(waveformColorMode_ == CM_PITCH)
	{
		waveformFilterPitch_->update();
		renderWaveformColored(block->tex, edges.begin(), w, h, block->id, waveformFilterPitch_);
	}
	else if(waveformColorMode_ == CM_CQT)
	{
		waveformFilterCQT_->update(waveformSpectrogramGain_);
		renderWaveformCQT(block->tex, edges.begin(), w, h, block->id);
	}
	else if(waveformColorMode_ == CM_CHROMAGRAM)
	{
		waveformFilterChromagram_->update(waveformSpectrogramGain_);
		renderWaveformChromagram(block->tex, edges.begin(), w, h, block->id);
	}
	else if(waveformColorMode_ == CM_NOVELTY)
	{
		waveformFilterNovelty_->update(waveformSpectrogramGain_);
		renderWaveformNovelty(block->tex, edges.begin(), w, h, block->id);
	}
	else if(waveformColorMode_ == CM_TEMPOGRAM)
	{
		waveformFilterTempogram_->update(waveformSpectrogramGain_);
		renderWaveformTempogram(block->tex, edges.begin(), w, h, block->id);
	}
	else if(waveformColorMode_ == CM_PERCUSSION)
	{
		waveformFilterHPSS_->update(waveformSpectrogramGain_);
		renderWaveformHPSS(block->tex, edges.begin(), w, h, block->id, false);
	}
	else if(waveformColorMode_ == CM_HARMONIC)
	{
		waveformFilterHPSS_->update(waveformSpectrogramGain_);
		renderWaveformHPSS(block->tex, edges.begin(), w, h, block->id, true);
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
	uchar* texBuf = waveformTextureBuffer_.begin();

	auto& music = gMusic->getSamples();
	double samplesPerSec = (double)music.getFrequency();
	double samplesPerPixel = samplesPerSec / fabs(gView->getPixPerSec());
	double samplesPerBlock = (double)TEX_H * samplesPerPixel;
	int64_t samplePos = max((int64_t)0, (int64_t)(samplesPerBlock * (double)blockId));

	double advance = samplesPerPixel * TEX_H / h;

	memset(texBuf, 0, w * h * 4);

	for(int channel = 0; channel < 2; ++channel)
	{
		const auto& flux = (channel == 0) ? waveformFilterNovelty_->fluxL : waveformFilterNovelty_->fluxR;
		int numBlocks = flux.size();

		for(int y = 0; y < h; ++y)
		{
			int64_t currentSample = samplePos + (int64_t)(y * advance);
			int blockIndex = (int)(currentSample / waveformFilterNovelty_->samplesPerBlock);

			if (blockIndex < 0 || blockIndex >= numBlocks) continue;

			int cx = w / 2;
			uchar intensity = flux[blockIndex];

			// Draw flux as a solid block width proportional to intensity
			int width = (int)(intensity / 255.0f * cx);

			for(int x = 0; x < cx; ++x)
			{
				// Color: Green -> Red (Low -> High Novelty)
				uchar r = intensity;
				uchar g = 255 - intensity;
				uchar b = 0;
				uchar a = (x <= width) ? 255 : 0;

				if (a == 0) { r=0; g=0; b=0; }

				// Mirror
				int l = cx - 1 - x;
				int r_pos = cx + x;

				if(l >= 0) {
					texBuf[(y * w + l) * 4 + 0] = r;
					texBuf[(y * w + l) * 4 + 1] = g;
					texBuf[(y * w + l) * 4 + 2] = b;
					texBuf[(y * w + l) * 4 + 3] = a;
				}
				if(r_pos < w) {
					texBuf[(y * w + r_pos) * 4 + 0] = r;
					texBuf[(y * w + r_pos) * 4 + 1] = g;
					texBuf[(y * w + r_pos) * 4 + 2] = b;
					texBuf[(y * w + r_pos) * 4 + 3] = a;
				}
			}
		}

		// Apply anti-aliasing
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

void renderWaveformChromagram(Texture* textures, WaveEdge* edgeBuf, int w, int h, int blockId)
{
	uchar* texBuf = waveformTextureBuffer_.begin();

	auto& music = gMusic->getSamples();
	double samplesPerSec = (double)music.getFrequency();
	double samplesPerPixel = samplesPerSec / fabs(gView->getPixPerSec());
	double samplesPerBlock = (double)TEX_H * samplesPerPixel;
	int64_t samplePos = max((int64_t)0, (int64_t)(samplesPerBlock * (double)blockId));

	double advance = samplesPerPixel * TEX_H / h;

	memset(texBuf, 0, w * h * 4);

	for(int channel = 0; channel < 2; ++channel)
	{
		const auto& chroma = (channel == 0) ? waveformFilterChromagram_->chromaL : waveformFilterChromagram_->chromaR;
		int numBlocks = chroma.empty() ? 0 : chroma[0].size();

		for(int y = 0; y < h; ++y)
		{
			int64_t currentSample = samplePos + (int64_t)(y * advance);
			int blockIndex = (int)(currentSample / waveformFilterChromagram_->samplesPerBlock);

			if (blockIndex < 0 || blockIndex >= numBlocks) continue;

			int cx = w / 2;

			// Draw chromagram (12 bins). Map x to bin.
			for(int x = 0; x < cx; ++x)
			{
				float t = (float)x / cx; // 0..1
				int bin = (int)(t * 12);
				bin = clamp(bin, 0, 11);

				uchar intensity = chroma[bin][blockIndex];

				// Map bin to hue (Circle of Fifths or Chromatic?)
				// Chromatic: C=Red, C#=Orange...
				float hue = bin * 30.0f; // 360 / 12

				float s = 0.8f;
				float v = intensity / 255.0f;

				float c = v * s;
				float hh = hue / 60.0f;
				float xx = c * (1 - fabs(fmod(hh, 2) - 1));
				float r=0, g=0, b=0;

				if (hh < 1) { r=c; g=xx; b=0; }
				else if (hh < 2) { r=xx; g=c; b=0; }
				else if (hh < 3) { r=0; g=c; b=xx; }
				else if (hh < 4) { r=0; g=xx; b=c; }
				else if (hh < 5) { r=xx; g=0; b=c; }
				else { r=c; g=0; b=xx; }

				uchar R = (uchar)(r * 255);
				uchar G = (uchar)(g * 255);
				uchar B = (uchar)(b * 255);

				// Mirror
				int l = cx - 1 - x;
				int r_pos = cx + x;

				if(l >= 0) {
					texBuf[(y * w + l) * 4 + 0] = R;
					texBuf[(y * w + l) * 4 + 1] = G;
					texBuf[(y * w + l) * 4 + 2] = B;
					texBuf[(y * w + l) * 4 + 3] = intensity;
				}
				if(r_pos < w) {
					texBuf[(y * w + r_pos) * 4 + 0] = R;
					texBuf[(y * w + r_pos) * 4 + 1] = G;
					texBuf[(y * w + r_pos) * 4 + 2] = B;
					texBuf[(y * w + r_pos) * 4 + 3] = intensity;
				}
			}
		}

		// Apply anti-aliasing
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

void renderWaveformNovelty(Texture* textures, WaveEdge* edgeBuf, int w, int h, int blockId)
{
	uchar* texBuf = waveformTextureBuffer_.begin();

	auto& music = gMusic->getSamples();
	double samplesPerSec = (double)music.getFrequency();
	double samplesPerPixel = samplesPerSec / fabs(gView->getPixPerSec());
	double samplesPerBlock = (double)TEX_H * samplesPerPixel;
	int64_t samplePos = max((int64_t)0, (int64_t)(samplesPerBlock * (double)blockId));

	double advance = samplesPerPixel * TEX_H / h;

	memset(texBuf, 0, w * h * 4);

	for(int channel = 0; channel < 2; ++channel)
	{
		const auto& flux = (channel == 0) ? waveformFilterNovelty_->fluxL : waveformFilterNovelty_->fluxR;
		int numBlocks = flux.size();

		for(int y = 0; y < h; ++y)
		{
			int64_t currentSample = samplePos + (int64_t)(y * advance);
			int blockIndex = (int)(currentSample / waveformFilterNovelty_->samplesPerBlock);

			if (blockIndex < 0 || blockIndex >= numBlocks) continue;

			int cx = w / 2;
			uchar intensity = flux[blockIndex];

			// Draw flux as a solid block width proportional to intensity
			int width = (int)(intensity / 255.0f * cx);

			for(int x = 0; x < cx; ++x)
			{
				// Color: Green -> Red (Low -> High Novelty)
				uchar r = intensity;
				uchar g = 255 - intensity;
				uchar b = 0;
				uchar a = (x <= width) ? 255 : 0;

				if (a == 0) { r=0; g=0; b=0; }

				// Mirror
				int l = cx - 1 - x;
				int r_pos = cx + x;

				if(l >= 0) {
					texBuf[(y * w + l) * 4 + 0] = r;
					texBuf[(y * w + l) * 4 + 1] = g;
					texBuf[(y * w + l) * 4 + 2] = b;
					texBuf[(y * w + l) * 4 + 3] = a;
				}
				if(r_pos < w) {
					texBuf[(y * w + r_pos) * 4 + 0] = r;
					texBuf[(y * w + r_pos) * 4 + 1] = g;
					texBuf[(y * w + r_pos) * 4 + 2] = b;
					texBuf[(y * w + r_pos) * 4 + 3] = a;
				}
			}
		}

		// Apply anti-aliasing
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

void renderWaveformCQT(Texture* textures, WaveEdge* edgeBuf, int w, int h, int blockId)
{
	uchar* texBuf = waveformTextureBuffer_.begin();

	auto& music = gMusic->getSamples();
	double samplesPerSec = (double)music.getFrequency();
	double samplesPerPixel = samplesPerSec / fabs(gView->getPixPerSec());
	double samplesPerBlock = (double)TEX_H * samplesPerPixel;
	int64_t samplePos = max((int64_t)0, (int64_t)(samplesPerBlock * (double)blockId));

	double advance = samplesPerPixel * TEX_H / h;

	memset(texBuf, 0, w * h * 4);

	const int minNote = 21;
	const int maxNote = 108;
	const int numNotes = maxNote - minNote + 1;

	for(int channel = 0; channel < 2; ++channel)
	{
		const auto& intensities = (channel == 0) ? waveformFilterCQT_->intensitiesL : waveformFilterCQT_->intensitiesR;
		int numBlocks = intensities.empty() ? 0 : intensities[0].size();

		for(int y = 0; y < h; ++y)
		{
			int64_t currentSample = samplePos + (int64_t)(y * advance);
			int blockIndex = (int)(currentSample / waveformFilterCQT_->samplesPerBlock);

			if (blockIndex < 0 || blockIndex >= numBlocks) continue;

			int cx = w / 2;

			for(int x = 0; x < cx; ++x)
			{
				float t = (float)x / cx; // 0..1
				float noteFloat = minNote + t * (maxNote - minNote);
				int note = (int)noteFloat;

				if (note < minNote || note > maxNote) continue;
				int noteIdx = note - minNote;
				if (noteIdx >= numNotes) continue;

				uchar intensity = intensities[noteIdx][blockIndex];

				float chroma = fmod(noteFloat, 12.0f);
				float hue = chroma * 30.0f;

				float s = 0.8f;
				float v = intensity / 255.0f;

				float c = v * s;
				float hh = hue / 60.0f;
				float xx = c * (1 - fabs(fmod(hh, 2) - 1));
				float r=0, g=0, b=0;

				if (hh < 1) { r=c; g=xx; b=0; }
				else if (hh < 2) { r=xx; g=c; b=0; }
				else if (hh < 3) { r=0; g=c; b=xx; }
				else if (hh < 4) { r=0; g=xx; b=c; }
				else if (hh < 5) { r=xx; g=0; b=c; }
				else { r=c; g=0; b=xx; }

				uchar R = (uchar)(r * 255);
				uchar G = (uchar)(g * 255);
				uchar B = (uchar)(b * 255);

				// Mirror
				int l = cx - 1 - x;
				int r_pos = cx + x;

				if(l >= 0) {
					texBuf[(y * w + l) * 4 + 0] = R;
					texBuf[(y * w + l) * 4 + 1] = G;
					texBuf[(y * w + l) * 4 + 2] = B;
					texBuf[(y * w + l) * 4 + 3] = intensity;
				}
				if(r_pos < w) {
					texBuf[(y * w + r_pos) * 4 + 0] = R;
					texBuf[(y * w + r_pos) * 4 + 1] = G;
					texBuf[(y * w + r_pos) * 4 + 2] = B;
					texBuf[(y * w + r_pos) * 4 + 3] = intensity;
				}
			}
		}

		// Apply anti-aliasing
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

void renderWaveformTempogram(Texture* textures, WaveEdge* edgeBuf, int w, int h, int blockId)
{
	uchar* texBuf = waveformTextureBuffer_.begin();

	auto& music = gMusic->getSamples();
	double samplesPerSec = (double)music.getFrequency();
	double samplesPerPixel = samplesPerSec / fabs(gView->getPixPerSec());
	double samplesPerBlock = (double)TEX_H * samplesPerPixel;
	int64_t samplePos = max((int64_t)0, (int64_t)(samplesPerBlock * (double)blockId));

	double advance = samplesPerPixel * TEX_H / h;

	memset(texBuf, 0, w * h * 4);

	for(int channel = 0; channel < 2; ++channel)
	{
		const auto& tempogram = (channel == 0) ? waveformFilterTempogram_->tempogramL : waveformFilterTempogram_->tempogramR;
		int numBins = tempogram.size();
		int numBlocks = (numBins > 0) ? tempogram[0].size() : 0;

		for(int y = 0; y < h; ++y)
		{
			int64_t currentSample = samplePos + (int64_t)(y * advance);
			int blockIndex = (int)(currentSample / waveformFilterTempogram_->samplesPerBlock);

			if (blockIndex < 0 || blockIndex >= numBlocks) continue;

			int cx = w / 2;

			for(int x = 0; x < cx; ++x)
			{
				float t = (float)x / cx; // 0..1
				int bin = (int)(t * numBins);
				bin = clamp(bin, 0, numBins-1);

				uchar intensity = tempogram[bin][blockIndex];

				// Heatmap: Blue->Red->Yellow
				uchar r, g, b;
				if (intensity < 128) {
					// Blue to Green
					r = 0;
					g = intensity * 2;
					b = 255 - intensity * 2;
				} else {
					// Green to Red
					r = (intensity - 128) * 2;
					g = 255 - (intensity - 128) * 2;
					b = 0;
				}

				int l = cx - 1 - x;
				int r_pos = cx + x;

				if(l >= 0) {
					texBuf[(y * w + l) * 4 + 0] = r;
					texBuf[(y * w + l) * 4 + 1] = g;
					texBuf[(y * w + l) * 4 + 2] = b;
					texBuf[(y * w + l) * 4 + 3] = intensity;
				}
				if(r_pos < w) {
					texBuf[(y * w + r_pos) * 4 + 0] = r;
					texBuf[(y * w + r_pos) * 4 + 1] = g;
					texBuf[(y * w + r_pos) * 4 + 2] = b;
					texBuf[(y * w + r_pos) * 4 + 3] = intensity;
				}
			}
		}

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

void renderWaveformHPSS(Texture* textures, WaveEdge* edgeBuf, int w, int h, int blockId, bool harmonic)
{
	uchar* texBuf = waveformTextureBuffer_.begin();

	auto& music = gMusic->getSamples();
	double samplesPerSec = (double)music.getFrequency();
	double samplesPerPixel = samplesPerSec / fabs(gView->getPixPerSec());
	double samplesPerBlock = (double)TEX_H * samplesPerPixel;
	int64_t samplePos = max((int64_t)0, (int64_t)(samplesPerBlock * (double)blockId));

	double advance = samplesPerPixel * TEX_H / h;

	memset(texBuf, 0, w * h * 4);

	for(int channel = 0; channel < 2; ++channel)
	{
		const auto& spec = harmonic ? waveformFilterHPSS_->specH[channel] : waveformFilterHPSS_->specP[channel];
		if (spec.width == 0) continue;

		for(int y = 0; y < h; ++y)
		{
			int64_t currentSample = samplePos + (int64_t)(y * advance);
			// Calculate time index in spectrogram
			int t = (int)(currentSample / waveformFilterHPSS_->samplesPerBlock);
			if (t < 0 || t >= spec.width) continue;

			int cx = w / 2;
			int numBins = spec.height;

			for(int x = 0; x < cx; ++x)
			{
				float u = (float)x / cx;
				// Linear mapping for now to match spectrogram
				int bin = (int)(u * u * numBins);
				bin = clamp(bin, 0, numBins-1);

				uchar intensity = spec.get(t, bin);

				// Color map
				// Harmonic: Blue/Greenish? Percussive: Red/Orange?
				uchar r, g, b;
				if (harmonic) {
					// Cyan/Blue
					r = 0;
					g = intensity;
					b = (intensity > 128) ? 255 : intensity * 2;
				} else {
					// Red/Orange
					r = intensity;
					g = (intensity > 128) ? (intensity - 128) * 2 : 0;
					b = 0;
				}

				// Mirror
				int l = cx - 1 - x;
				int r_pos = cx + x;

				if(l >= 0) {
					texBuf[(y * w + l) * 4 + 0] = r;
					texBuf[(y * w + l) * 4 + 1] = g;
					texBuf[(y * w + l) * 4 + 2] = b;
					texBuf[(y * w + l) * 4 + 3] = intensity;
				}
				if(r_pos < w) {
					texBuf[(y * w + r_pos) * 4 + 0] = r;
					texBuf[(y * w + r_pos) * 4 + 1] = g;
					texBuf[(y * w + r_pos) * 4 + 2] = b;
					texBuf[(y * w + r_pos) * 4 + 3] = intensity;
				}
			}
		}

		// Apply anti-aliasing
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
		waveformFilterRGB_->update(waveformRGBLow_, waveformRGBHigh_);
		renderWaveformRGB(block->tex, edges.begin(), w, h, block->id);
	}
	else if(waveformColorMode_ == CM_SPECTRAL)
	{
		waveformFilterSpectral_->update();
		renderWaveformColored(block->tex, edges.begin(), w, h, block->id, waveformFilterSpectral_);
	}
	else if(waveformColorMode_ == CM_SPECTROGRAM)
	{
		waveformFilterSpectral_->update();
		renderWaveformSpectrogram(block->tex, edges.begin(), w, h, block->id);
	}
	else if(waveformColorMode_ == CM_PITCH)
	{
		waveformFilterPitch_->update();
		renderWaveformColored(block->tex, edges.begin(), w, h, block->id, waveformFilterPitch_);
	}
	else if(waveformColorMode_ == CM_CQT)
	{
		waveformFilterCQT_->update(waveformSpectrogramGain_);
		renderWaveformCQT(block->tex, edges.begin(), w, h, block->id);
	}
	else if(waveformColorMode_ == CM_CHROMAGRAM)
	{
		waveformFilterChromagram_->update(waveformSpectrogramGain_);
		renderWaveformChromagram(block->tex, edges.begin(), w, h, block->id);
	}
	else if(waveformColorMode_ == CM_NOVELTY)
	{
		waveformFilterNovelty_->update(waveformSpectrogramGain_);
		renderWaveformNovelty(block->tex, edges.begin(), w, h, block->id);
	}
	else if(waveformColorMode_ == CM_TEMPOGRAM)
	{
		waveformFilterTempogram_->update(waveformSpectrogramGain_);
		renderWaveformTempogram(block->tex, edges.begin(), w, h, block->id);
	}
	else if(waveformColorMode_ == CM_PERCUSSION)
	{
		waveformFilterHPSS_->update(waveformSpectrogramGain_);
		renderWaveformHPSS(block->tex, edges.begin(), w, h, block->id, false);
	}
	else if(waveformColorMode_ == CM_HARMONIC)
	{
		waveformFilterHPSS_->update(waveformSpectrogramGain_);
		renderWaveformHPSS(block->tex, edges.begin(), w, h, block->id, true);
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

	auto& music = gMusic->getSamples();
	double samplesPerSec = (double)music.getFrequency();
	double samplesPerPixel = samplesPerSec / fabs(gView->getPixPerSec());
	double samplesPerBlock = (double)TEX_H * samplesPerPixel;
	int64_t samplePos = max((int64_t)0, (int64_t)(samplesPerBlock * (double)blockId));

	double advance = samplesPerPixel * TEX_H / h;

	memset(texBuf, 0, w * h * 4);

	for(int channel = 0; channel < 2; ++channel)
	{
		const auto& chroma = (channel == 0) ? waveformFilterChromagram_->chromaL : waveformFilterChromagram_->chromaR;
		int numBlocks = chroma.empty() ? 0 : chroma[0].size();

		for(int y = 0; y < h; ++y)
		{
			int64_t currentSample = samplePos + (int64_t)(y * advance);
			int blockIndex = (int)(currentSample / waveformFilterChromagram_->samplesPerBlock);

			if (blockIndex < 0 || blockIndex >= numBlocks) continue;

			int cx = w / 2;

			// Draw chromagram (12 bins). Map x to bin.
			for(int x = 0; x < cx; ++x)
			{
				float t = (float)x / cx; // 0..1
				int bin = (int)(t * 12);
				bin = clamp(bin, 0, 11);

				uchar intensity = chroma[bin][blockIndex];

				// Map bin to hue (Circle of Fifths or Chromatic?)
				// Chromatic: C=Red, C#=Orange...
				float hue = bin * 30.0f; // 360 / 12

				float s = 0.8f;
				float v = intensity / 255.0f;

				float c = v * s;
				float hh = hue / 60.0f;
				float xx = c * (1 - fabs(fmod(hh, 2) - 1));
				float r=0, g=0, b=0;

				if (hh < 1) { r=c; g=xx; b=0; }
				else if (hh < 2) { r=xx; g=c; b=0; }
				else if (hh < 3) { r=0; g=c; b=xx; }
				else if (hh < 4) { r=0; g=xx; b=c; }
				else if (hh < 5) { r=xx; g=0; b=c; }
				else { r=c; g=0; b=xx; }

				uchar R = (uchar)(r * 255);
				uchar G = (uchar)(g * 255);
				uchar B = (uchar)(b * 255);

				// Mirror
				int l = cx - 1 - x;
				int r_pos = cx + x;

				if(l >= 0) {
					texBuf[(y * w + l) * 4 + 0] = R;
					texBuf[(y * w + l) * 4 + 1] = G;
					texBuf[(y * w + l) * 4 + 2] = B;
					texBuf[(y * w + l) * 4 + 3] = intensity;
				}
				if(r_pos < w) {
					texBuf[(y * w + r_pos) * 4 + 0] = R;
					texBuf[(y * w + r_pos) * 4 + 1] = G;
					texBuf[(y * w + r_pos) * 4 + 2] = B;
					texBuf[(y * w + r_pos) * 4 + 3] = intensity;
				}
			}
		}

		// Apply anti-aliasing
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

void renderWaveformNovelty(Texture* textures, WaveEdge* edgeBuf, int w, int h, int blockId)
{
	uchar* texBuf = waveformTextureBuffer_.begin();

	auto& music = gMusic->getSamples();
	double samplesPerSec = (double)music.getFrequency();
	double samplesPerPixel = samplesPerSec / fabs(gView->getPixPerSec());
	double samplesPerBlock = (double)TEX_H * samplesPerPixel;
	int64_t samplePos = max((int64_t)0, (int64_t)(samplesPerBlock * (double)blockId));

	double advance = samplesPerPixel * TEX_H / h;

	memset(texBuf, 0, w * h * 4);

	for(int channel = 0; channel < 2; ++channel)
	{
		const auto& flux = (channel == 0) ? waveformFilterNovelty_->fluxL : waveformFilterNovelty_->fluxR;
		int numBlocks = flux.size();

		for(int y = 0; y < h; ++y)
		{
			int64_t currentSample = samplePos + (int64_t)(y * advance);
			int blockIndex = (int)(currentSample / waveformFilterNovelty_->samplesPerBlock);

			if (blockIndex < 0 || blockIndex >= numBlocks) continue;

			int cx = w / 2;
			uchar intensity = flux[blockIndex];

			// Draw flux as a solid block width proportional to intensity
			int width = (int)(intensity / 255.0f * cx);

			for(int x = 0; x < cx; ++x)
			{
				// Color: Green -> Red (Low -> High Novelty)
				uchar r = intensity;
				uchar g = 255 - intensity;
				uchar b = 0;
				uchar a = (x <= width) ? 255 : 0;

				if (a == 0) { r=0; g=0; b=0; }

				// Mirror
				int l = cx - 1 - x;
				int r_pos = cx + x;

				if(l >= 0) {
					texBuf[(y * w + l) * 4 + 0] = r;
					texBuf[(y * w + l) * 4 + 1] = g;
					texBuf[(y * w + l) * 4 + 2] = b;
					texBuf[(y * w + l) * 4 + 3] = a;
				}
				if(r_pos < w) {
					texBuf[(y * w + r_pos) * 4 + 0] = r;
					texBuf[(y * w + r_pos) * 4 + 1] = g;
					texBuf[(y * w + r_pos) * 4 + 2] = b;
					texBuf[(y * w + r_pos) * 4 + 3] = a;
				}
			}
		}

		// Apply anti-aliasing
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

void renderWaveformChromagram(Texture* textures, WaveEdge* edgeBuf, int w, int h, int blockId)
{
	uchar* texBuf = waveformTextureBuffer_.begin();

	auto& music = gMusic->getSamples();
	double samplesPerSec = (double)music.getFrequency();
	double samplesPerPixel = samplesPerSec / fabs(gView->getPixPerSec());
	double samplesPerBlock = (double)TEX_H * samplesPerPixel;
	int64_t samplePos = max((int64_t)0, (int64_t)(samplesPerBlock * (double)blockId));

	double advance = samplesPerPixel * TEX_H / h;

	memset(texBuf, 0, w * h * 4);

	for(int channel = 0; channel < 2; ++channel)
	{
		const auto& chroma = (channel == 0) ? waveformFilterChromagram_->chromaL : waveformFilterChromagram_->chromaR;
		int numBlocks = chroma.empty() ? 0 : chroma[0].size();

		for(int y = 0; y < h; ++y)
		{
			int64_t currentSample = samplePos + (int64_t)(y * advance);
			int blockIndex = (int)(currentSample / waveformFilterChromagram_->samplesPerBlock);

			if (blockIndex < 0 || blockIndex >= numBlocks) continue;

			int cx = w / 2;

			// Draw chromagram (12 bins). Map x to bin.
			for(int x = 0; x < cx; ++x)
			{
				float t = (float)x / cx; // 0..1
				int bin = (int)(t * 12);
				bin = clamp(bin, 0, 11);

				uchar intensity = chroma[bin][blockIndex];

				// Map bin to hue (Circle of Fifths or Chromatic?)
				// Chromatic: C=Red, C#=Orange...
				float hue = bin * 30.0f; // 360 / 12

				float s = 0.8f;
				float v = intensity / 255.0f;

				float c = v * s;
				float hh = hue / 60.0f;
				float xx = c * (1 - fabs(fmod(hh, 2) - 1));
				float r=0, g=0, b=0;

				if (hh < 1) { r=c; g=xx; b=0; }
				else if (hh < 2) { r=xx; g=c; b=0; }
				else if (hh < 3) { r=0; g=c; b=xx; }
				else if (hh < 4) { r=0; g=xx; b=c; }
				else if (hh < 5) { r=xx; g=0; b=c; }
				else { r=c; g=0; b=xx; }

				uchar R = (uchar)(r * 255);
				uchar G = (uchar)(g * 255);
				uchar B = (uchar)(b * 255);

				// Mirror
				int l = cx - 1 - x;
				int r_pos = cx + x;

				if(l >= 0) {
					texBuf[(y * w + l) * 4 + 0] = R;
					texBuf[(y * w + l) * 4 + 1] = G;
					texBuf[(y * w + l) * 4 + 2] = B;
					texBuf[(y * w + l) * 4 + 3] = intensity;
				}
				if(r_pos < w) {
					texBuf[(y * w + r_pos) * 4 + 0] = R;
					texBuf[(y * w + r_pos) * 4 + 1] = G;
					texBuf[(y * w + r_pos) * 4 + 2] = B;
					texBuf[(y * w + r_pos) * 4 + 3] = intensity;
				}
			}
		}

		// Apply anti-aliasing
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

void renderWaveformNovelty(Texture* textures, WaveEdge* edgeBuf, int w, int h, int blockId)
{
	uchar* texBuf = waveformTextureBuffer_.begin();

	auto& music = gMusic->getSamples();
	double samplesPerSec = (double)music.getFrequency();
	double samplesPerPixel = samplesPerSec / fabs(gView->getPixPerSec());
	double samplesPerBlock = (double)TEX_H * samplesPerPixel;
	int64_t samplePos = max((int64_t)0, (int64_t)(samplesPerBlock * (double)blockId));

	double advance = samplesPerPixel * TEX_H / h;

	memset(texBuf, 0, w * h * 4);

	for(int channel = 0; channel < 2; ++channel)
	{
		const auto& flux = (channel == 0) ? waveformFilterNovelty_->fluxL : waveformFilterNovelty_->fluxR;
		int numBlocks = flux.size();

		for(int y = 0; y < h; ++y)
		{
			int64_t currentSample = samplePos + (int64_t)(y * advance);
			int blockIndex = (int)(currentSample / waveformFilterNovelty_->samplesPerBlock);

			if (blockIndex < 0 || blockIndex >= numBlocks) continue;

			int cx = w / 2;
			uchar intensity = flux[blockIndex];

			// Draw flux as a solid block width proportional to intensity
			int width = (int)(intensity / 255.0f * cx);

			for(int x = 0; x < cx; ++x)
			{
				// Color: Green -> Red (Low -> High Novelty)
				uchar r = intensity;
				uchar g = 255 - intensity;
				uchar b = 0;
				uchar a = (x <= width) ? 255 : 0;

				if (a == 0) { r=0; g=0; b=0; }

				// Mirror
				int l = cx - 1 - x;
				int r_pos = cx + x;

				if(l >= 0) {
					texBuf[(y * w + l) * 4 + 0] = r;
					texBuf[(y * w + l) * 4 + 1] = g;
					texBuf[(y * w + l) * 4 + 2] = b;
					texBuf[(y * w + l) * 4 + 3] = a;
				}
				if(r_pos < w) {
					texBuf[(y * w + r_pos) * 4 + 0] = r;
					texBuf[(y * w + r_pos) * 4 + 1] = g;
					texBuf[(y * w + r_pos) * 4 + 2] = b;
					texBuf[(y * w + r_pos) * 4 + 3] = a;
				}
			}
		}

		// Apply anti-aliasing
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

void renderWaveformCQT(Texture* textures, WaveEdge* edgeBuf, int w, int h, int blockId)
{
	uchar* texBuf = waveformTextureBuffer_.begin();

	auto& music = gMusic->getSamples();
	double samplesPerSec = (double)music.getFrequency();
	double samplesPerPixel = samplesPerSec / fabs(gView->getPixPerSec());
	double samplesPerBlock = (double)TEX_H * samplesPerPixel;
	int64_t samplePos = max((int64_t)0, (int64_t)(samplesPerBlock * (double)blockId));

	double advance = samplesPerPixel * TEX_H / h;

	memset(texBuf, 0, w * h * 4);

	const int minNote = 21;
	const int maxNote = 108;
	const int numNotes = maxNote - minNote + 1;

	for(int channel = 0; channel < 2; ++channel)
	{
		const auto& intensities = (channel == 0) ? waveformFilterCQT_->intensitiesL : waveformFilterCQT_->intensitiesR;
		int numBlocks = intensities.empty() ? 0 : intensities[0].size();

		for(int y = 0; y < h; ++y)
		{
			int64_t currentSample = samplePos + (int64_t)(y * advance);
			int blockIndex = (int)(currentSample / waveformFilterCQT_->samplesPerBlock);

			if (blockIndex < 0 || blockIndex >= numBlocks) continue;

			int cx = w / 2;

			for(int x = 0; x < cx; ++x)
			{
				float t = (float)x / cx; // 0..1
				float noteFloat = minNote + t * (maxNote - minNote);
				int note = (int)noteFloat;

				if (note < minNote || note > maxNote) continue;
				int noteIdx = note - minNote;
				if (noteIdx >= numNotes) continue;

				uchar intensity = intensities[noteIdx][blockIndex];

				float chroma = fmod(noteFloat, 12.0f);
				float hue = chroma * 30.0f;

				float s = 0.8f;
				float v = intensity / 255.0f;

				float c = v * s;
				float hh = hue / 60.0f;
				float xx = c * (1 - fabs(fmod(hh, 2) - 1));
				float r=0, g=0, b=0;

				if (hh < 1) { r=c; g=xx; b=0; }
				else if (hh < 2) { r=xx; g=c; b=0; }
				else if (hh < 3) { r=0; g=c; b=xx; }
				else if (hh < 4) { r=0; g=xx; b=c; }
				else if (hh < 5) { r=xx; g=0; b=c; }
				else { r=c; g=0; b=xx; }

				uchar R = (uchar)(r * 255);
				uchar G = (uchar)(g * 255);
				uchar B = (uchar)(b * 255);

				// Mirror
				int l = cx - 1 - x;
				int r_pos = cx + x;

				if(l >= 0) {
					texBuf[(y * w + l) * 4 + 0] = R;
					texBuf[(y * w + l) * 4 + 1] = G;
					texBuf[(y * w + l) * 4 + 2] = B;
					texBuf[(y * w + l) * 4 + 3] = intensity;
				}
				if(r_pos < w) {
					texBuf[(y * w + r_pos) * 4 + 0] = R;
					texBuf[(y * w + r_pos) * 4 + 1] = G;
					texBuf[(y * w + r_pos) * 4 + 2] = B;
					texBuf[(y * w + r_pos) * 4 + 3] = intensity;
				}
			}
		}

		// Apply anti-aliasing
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

void renderWaveformTempogram(Texture* textures, WaveEdge* edgeBuf, int w, int h, int blockId)
{
	uchar* texBuf = waveformTextureBuffer_.begin();

	auto& music = gMusic->getSamples();
	double samplesPerSec = (double)music.getFrequency();
	double samplesPerPixel = samplesPerSec / fabs(gView->getPixPerSec());
	double samplesPerBlock = (double)TEX_H * samplesPerPixel;
	int64_t samplePos = max((int64_t)0, (int64_t)(samplesPerBlock * (double)blockId));

	double advance = samplesPerPixel * TEX_H / h;

	memset(texBuf, 0, w * h * 4);

	for(int channel = 0; channel < 2; ++channel)
	{
		const auto& tempogram = (channel == 0) ? waveformFilterTempogram_->tempogramL : waveformFilterTempogram_->tempogramR;
		int numBins = tempogram.size();
		int numBlocks = (numBins > 0) ? tempogram[0].size() : 0;

		for(int y = 0; y < h; ++y)
		{
			int64_t currentSample = samplePos + (int64_t)(y * advance);
			int blockIndex = (int)(currentSample / waveformFilterTempogram_->samplesPerBlock);

			if (blockIndex < 0 || blockIndex >= numBlocks) continue;

			int cx = w / 2;

			for(int x = 0; x < cx; ++x)
			{
				float t = (float)x / cx; // 0..1
				int bin = (int)(t * numBins);
				bin = clamp(bin, 0, numBins-1);

				uchar intensity = tempogram[bin][blockIndex];

				// Heatmap: Blue->Red->Yellow
				uchar r, g, b;
				if (intensity < 128) {
					// Blue to Green
					r = 0;
					g = intensity * 2;
					b = 255 - intensity * 2;
				} else {
					// Green to Red
					r = (intensity - 128) * 2;
					g = 255 - (intensity - 128) * 2;
					b = 0;
				}

				int l = cx - 1 - x;
				int r_pos = cx + x;

				if(l >= 0) {
					texBuf[(y * w + l) * 4 + 0] = r;
					texBuf[(y * w + l) * 4 + 1] = g;
					texBuf[(y * w + l) * 4 + 2] = b;
					texBuf[(y * w + l) * 4 + 3] = intensity;
				}
				if(r_pos < w) {
					texBuf[(y * w + r_pos) * 4 + 0] = r;
					texBuf[(y * w + r_pos) * 4 + 1] = g;
					texBuf[(y * w + r_pos) * 4 + 2] = b;
					texBuf[(y * w + r_pos) * 4 + 3] = intensity;
				}
			}
		}

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

void renderWaveformHPSS(Texture* textures, WaveEdge* edgeBuf, int w, int h, int blockId, bool harmonic)
{
	uchar* texBuf = waveformTextureBuffer_.begin();

	auto& music = gMusic->getSamples();
	double samplesPerSec = (double)music.getFrequency();
	double samplesPerPixel = samplesPerSec / fabs(gView->getPixPerSec());
	double samplesPerBlock = (double)TEX_H * samplesPerPixel;
	int64_t samplePos = max((int64_t)0, (int64_t)(samplesPerBlock * (double)blockId));

	double advance = samplesPerPixel * TEX_H / h;

	memset(texBuf, 0, w * h * 4);

	for(int channel = 0; channel < 2; ++channel)
	{
		const auto& spec = harmonic ? waveformFilterHPSS_->specH[channel] : waveformFilterHPSS_->specP[channel];
		if (spec.width == 0) continue;

		for(int y = 0; y < h; ++y)
		{
			int64_t currentSample = samplePos + (int64_t)(y * advance);
			// Calculate time index in spectrogram
			int t = (int)(currentSample / waveformFilterHPSS_->samplesPerBlock);
			if (t < 0 || t >= spec.width) continue;

			int cx = w / 2;
			int numBins = spec.height;

			for(int x = 0; x < cx; ++x)
			{
				float u = (float)x / cx;
				// Linear mapping for now to match spectrogram
				int bin = (int)(u * u * numBins);
				bin = clamp(bin, 0, numBins-1);

				uchar intensity = spec.get(t, bin);

				// Color map
				// Harmonic: Blue/Greenish? Percussive: Red/Orange?
				uchar r, g, b;
				if (harmonic) {
					// Cyan/Blue
					r = 0;
					g = intensity;
					b = (intensity > 128) ? 255 : intensity * 2;
				} else {
					// Red/Orange
					r = intensity;
					g = (intensity > 128) ? (intensity - 128) * 2 : 0;
					b = 0;
				}

				// Mirror
				int l = cx - 1 - x;
				int r_pos = cx + x;

				if(l >= 0) {
					texBuf[(y * w + l) * 4 + 0] = r;
					texBuf[(y * w + l) * 4 + 1] = g;
					texBuf[(y * w + l) * 4 + 2] = b;
					texBuf[(y * w + l) * 4 + 3] = intensity;
				}
				if(r_pos < w) {
					texBuf[(y * w + r_pos) * 4 + 0] = r;
					texBuf[(y * w + r_pos) * 4 + 1] = g;
					texBuf[(y * w + r_pos) * 4 + 2] = b;
					texBuf[(y * w + r_pos) * 4 + 3] = intensity;
				}
			}
		}

		// Apply anti-aliasing
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
		waveformFilterRGB_->update(waveformRGBLow_, waveformRGBHigh_);
		renderWaveformRGB(block->tex, edges.begin(), w, h, block->id);
	}
	else if(waveformColorMode_ == CM_SPECTRAL)
	{
		waveformFilterSpectral_->update();
		renderWaveformColored(block->tex, edges.begin(), w, h, block->id, waveformFilterSpectral_);
	}
	else if(waveformColorMode_ == CM_SPECTROGRAM)
	{
		waveformFilterSpectral_->update();
		renderWaveformSpectrogram(block->tex, edges.begin(), w, h, block->id);
	}
	else if(waveformColorMode_ == CM_PITCH)
	{
		waveformFilterPitch_->update();
		renderWaveformColored(block->tex, edges.begin(), w, h, block->id, waveformFilterPitch_);
	}
	else if(waveformColorMode_ == CM_CQT)
	{
		waveformFilterCQT_->update(waveformSpectrogramGain_);
		renderWaveformCQT(block->tex, edges.begin(), w, h, block->id);
	}
	else if(waveformColorMode_ == CM_CHROMAGRAM)
	{
		waveformFilterChromagram_->update(waveformSpectrogramGain_);
		renderWaveformChromagram(block->tex, edges.begin(), w, h, block->id);
	}
	else if(waveformColorMode_ == CM_NOVELTY)
	{
		waveformFilterNovelty_->update(waveformSpectrogramGain_);
		renderWaveformNovelty(block->tex, edges.begin(), w, h, block->id);
	}
	else if(waveformColorMode_ == CM_TEMPOGRAM)
	{
		waveformFilterTempogram_->update(waveformSpectrogramGain_);
		renderWaveformTempogram(block->tex, edges.begin(), w, h, block->id);
	}
	else if(waveformColorMode_ == CM_PERCUSSION)
	{
		waveformFilterHPSS_->update(waveformSpectrogramGain_);
		renderWaveformHPSS(block->tex, edges.begin(), w, h, block->id, false);
	}
	else if(waveformColorMode_ == CM_HARMONIC)
	{
		waveformFilterHPSS_->update(waveformSpectrogramGain_);
		renderWaveformHPSS(block->tex, edges.begin(), w, h, block->id, true);
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

	// Draw split background for Left/Right channels to match DDreamStudio style
	int pw = waveformBlockWidth_;
	int border = waveformSpacing_;

	// Left Channel Background
	// Center of left channel is cx - pw/2 - border
	int xl = cx - pw / 2 - border - pw / 2;
	Draw::fill({xl, 0, pw, h}, ToColor32(waveformColorScheme_.bg));

	// Right Channel Background
	// Center of right channel is cx + pw/2 + border
	int xr = cx + pw / 2 + border - pw / 2;
	Draw::fill({xr, 0, pw, h}, ToColor32(waveformColorScheme_.bg));
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

		if(waveformColorMode_ == CM_RGB || waveformColorMode_ == CM_SPECTRAL || waveformColorMode_ == CM_PITCH || waveformColorMode_ == CM_SPECTROGRAM || waveformColorMode_ == CM_CQT || waveformColorMode_ == CM_PERCUSSION || waveformColorMode_ == CM_HARMONIC || waveformColorMode_ == CM_CHROMAGRAM || waveformColorMode_ == CM_NOVELTY || waveformColorMode_ == CM_TEMPOGRAM)
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

	// Draw onset lines
	if (waveformShowOnsets_)
	{
		auto& music = gMusic->getSamples();
		double samplesPerSec = (double)music.getFrequency();
		int viewH = gView->getHeight();

		for(const auto& onset : waveformOnsets_)
		{
			double time = (double)onset.pos / samplesPerSec;
			int y = gView->timeToY(time);

			if (y >= -2 && y <= viewH + 2)
			{
				Draw::fill({xl - pw, y, pw * 2 + border * 2 + pw * 2, 1}, RGBAtoColor32(255, 50, 50, 200));
			}
		}
	}

	// Draw BPM and Section markers
	if (gTempo)
	{
		const SegmentGroup* segs = gTempo->getSegments();
		if (segs)
		{
			int viewH = gView->getHeight();

			// Draw BPM changes (Cyan)
			const auto& bpms = segs->get<BpmChange>();
			for(const auto& bpm : bpms)
			{
				double time = gTempo->rowToTime(bpm.row);
				int y = gView->timeToY(time);
				if (y >= -2 && y <= viewH + 2)
				{
					// Draw a distinct line for BPM change
					Draw::fill({xl - pw, y, pw * 2 + border * 2 + pw * 2, 2}, RGBAtoColor32(0, 255, 255, 200));
				}
			}

			// Draw Stops (Red zone)
			const auto& stops = segs->get<Stop>();
			for(const auto& stop : stops)
			{
				double time = gTempo->rowToTime(stop.row);
				int y1 = gView->timeToY(time);
				// Stop freezes the time, so on the waveform (time-based) it corresponds to a duration?
				// Actually, in ArrowVortex time-based view, stops are gaps?
				// Or does the waveform skip the stop duration?
				// Usually waveform is strictly linear with audio time.
				// Stops pause the chart scrolling relative to audio.
				// So a stop happens at a specific audio time.
				// But visually in the chart, it takes up 0 space?
				// Wait, if I'm drawing on the waveform texture, the texture is Time vs Amplitude.
				// A Stop doesn't affect the audio file itself.
				// But we want to see WHERE the stop is.
				// So just a line is fine.
				if (y1 >= -2 && y1 <= viewH + 2)
				{
					Draw::fill({xl - pw, y1, pw * 2 + border * 2 + pw * 2, 2}, RGBAtoColor32(255, 0, 0, 200));
				}
			}

			// Draw Labels/Sections (Yellow)
			const auto& labels = segs->get<Label>();
			for(const auto& label : labels)
			{
				double time = gTempo->rowToTime(label.row);
				int y = gView->timeToY(time);
				if (y >= -2 && y <= viewH + 2)
				{
					Draw::fill({xl - pw, y, pw * 2 + border * 2 + pw * 2, 2}, RGBAtoColor32(255, 255, 0, 200));
				}
			}
		}
	}
}

void toggleSpectrogram()
{
	m_showSpectrogram = !m_showSpectrogram;
}

const Vector<Onset>& getOnsets()
{
	return waveformOnsets_;
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
