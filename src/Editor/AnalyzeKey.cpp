#include <Editor/AnalyzeKey.h>

#include <Gist.h>
#include <Core/Utils.h>
#include <vector>
#include <math.h>
#include <algorithm>

namespace Vortex {

// Krumhansl-Schmuckler Key Profiles (Major and Minor)
static const float kMajorProfile[12] = {
	6.35f, 2.23f, 3.48f, 2.33f, 4.38f, 4.09f, 2.52f, 5.19f, 2.39f, 3.66f, 2.29f, 2.88f
};

static const float kMinorProfile[12] = {
	6.33f, 2.68f, 3.52f, 5.38f, 2.60f, 3.53f, 2.54f, 4.75f, 3.98f, 2.69f, 3.34f, 3.17f
};

static const char* kKeyNames[24] = {
	"C Major", "C# Major", "D Major", "D# Major", "E Major", "F Major",
	"F# Major", "G Major", "G# Major", "A Major", "A# Major", "B Major",
	"C Minor", "C# Minor", "D Minor", "D# Minor", "E Minor", "F Minor",
	"F# Minor", "G Minor", "G# Minor", "A Minor", "A# Minor", "B Minor"
};

String DetectKey(const float* samples, int numSamples, int sampleRate)
{
	if (!samples || numSamples <= 0 || sampleRate <= 0) return "Unknown";

	// Use parameters consistent with Waveform.cpp
	const int frameSize = 4096;
	const int hopSize = 2048; // Overlap for better resolution

	if (numSamples < frameSize) return "Too Short";

	Gist<float> gist(frameSize, sampleRate);
	std::vector<float> audioFrame(frameSize);

	int numBlocks = (numSamples - frameSize) / hopSize;
	if (numBlocks < 1) numBlocks = 1;

	std::vector<float> totalChroma(12, 0.0f);

	for (int i = 0; i < numBlocks; ++i) {
		int start = i * hopSize;
		// Safely copy samples
		for (int k = 0; k < frameSize; ++k) {
			if (start + k < numSamples)
				audioFrame[k] = samples[start + k];
			else
				audioFrame[k] = 0.0f;
		}

		gist.processAudioFrame(audioFrame);
		const auto& spectrum = gist.getMagnitudeSpectrum();
		int numBins = spectrum.size();

		// Calculate Chromagram for this frame
		for (int b = 0; b < numBins; ++b) {
			float freq = b * sampleRate / (float)frameSize;
			if (freq < 27.5f) continue; // Ignore below A0

			float midiNote = 69.0f + 12.0f * log2(freq / 440.0f);
			int noteIndex = (int)round(midiNote);
			int chromaIdx = noteIndex % 12;
			if (chromaIdx < 0) chromaIdx += 12;

			// Add energy to the chroma bin
			totalChroma[chromaIdx] += spectrum[b];
		}
	}

	// Normalize total chroma
	float maxVal = 0.0f;
	for (float v : totalChroma) maxVal = max(maxVal, v);
	if (maxVal > 0) {
		for (float& v : totalChroma) v /= maxVal;
	}

	// Correlation with profiles
	int bestKey = -1;
	float bestCorr = -1.0f;

	// Calculate correlation for each of the 24 keys (12 Major, 12 Minor)
	// Major keys (0-11)
	for (int k = 0; k < 12; ++k) {
		float corr = 0.0f;
		for (int i = 0; i < 12; ++i) {
			int chromaIdx = (k + i) % 12;
			corr += totalChroma[chromaIdx] * kMajorProfile[i];
		}
		if (corr > bestCorr) {
			bestCorr = corr;
			bestKey = k;
		}
	}

	// Minor keys (12-23)
	for (int k = 0; k < 12; ++k) {
		float corr = 0.0f;
		for (int i = 0; i < 12; ++i) {
			int chromaIdx = (k + i) % 12; // Key root is k
			corr += totalChroma[chromaIdx] * kMinorProfile[i];
		}
		if (corr > bestCorr) {
			bestCorr = corr;
			bestKey = 12 + k;
		}
	}

	if (bestKey >= 0 && bestKey < 24) {
		return kKeyNames[bestKey];
	}

	return "Unknown";
}

}
