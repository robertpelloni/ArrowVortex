#include <Editor/FindTempo.h>
#include <Editor/FindOnsets.h>
#include <Editor/Music.h>
#include <Managers/TempoMan.h>
#include <Core/Utils.h>
#include <math.h>
#include <algorithm>
#include <map>

namespace Vortex {

struct TempoDetectorImpl : public TempoDetector
{
	bool myHasResult;
	Vector<TempoResult> myResults;
	double myStartTime;
	double myLength;

	TempoDetectorImpl(double start, double len)
		: myStartTime(start), myLength(len), myHasResult(false)
	{
		// Launch detection in a separate thread?
		// For simplicity/stub restoration, we do it synchronously in constructor or on first check.
		// Since usage in Action.cpp polls with sleep, it expects async.
		// But here we can just do it instantly.
		detect();
	}

	void detect()
	{
		// Basic BPM detection logic
		// 1. Get audio samples
		auto& music = gMusic->getSamples();
		if (!music.isCompleted() || music.getNumFrames() == 0) {
			myHasResult = true;
			return;
		}

		int samplerate = music.getFrequency();
		int startFrame = (int)(myStartTime * samplerate);
		int numFrames = (int)(myLength * samplerate);

		if (startFrame < 0) startFrame = 0;
		if (startFrame + numFrames > music.getNumFrames()) numFrames = music.getNumFrames() - startFrame;
		if (numFrames < samplerate) { // Less than 1 second
			myHasResult = true;
			return;
		}

		// Copy samples to float buffer for FindOnsets
		std::vector<float> buffer(numFrames);
		const short* src = music.samplesL() + startFrame; // Mono (Left channel)
		for(int i=0; i<numFrames; ++i) {
			buffer[i] = src[i] / 32768.0f;
		}

		// Detect onsets
		Vector<Onset> onsets;
		FindOnsets(buffer.data(), samplerate, numFrames, 1, onsets);

		if (onsets.size() < 2) {
			myHasResult = true;
			return;
		}

		// Calculate IOIs (Inter-Onset Intervals)
		std::vector<double> iois;
		for(size_t i = 0; i < onsets.size() - 1; ++i) {
			double t1 = (double)onsets[i].pos / samplerate;
			double t2 = (double)onsets[i+1].pos / samplerate;
			double diff = t2 - t1;
			if (diff > 0.1 && diff < 2.0) { // 30 BPM to 600 BPM
				iois.push_back(diff);
			}
		}

		if (iois.empty()) {
			myHasResult = true;
			return;
		}

		// Histogram of BPM candidates
		std::map<int, int> bpmHist;
		for (double dur : iois) {
			double bpm = 60.0 / dur;
			// Clamp to reasonable range (e.g., 60-250)
			while (bpm < 60) bpm *= 2;
			while (bpm > 250) bpm /= 2;

			int bin = (int)(bpm + 0.5);
			bpmHist[bin]++;
		}

		// Find peak
		int bestBpm = 0;
		int maxCount = 0;
		for(auto const& [bpm, count] : bpmHist) {
			if (count > maxCount) {
				maxCount = count;
				bestBpm = bpm;
			}
		}

		if (bestBpm > 0) {
			TempoResult res;
			res.bpm = (double)bestBpm;
			// Estimate offset based on first strong onset
			res.offset = (double)onsets[0].pos / samplerate + myStartTime;
			res.confidence = (double)maxCount / iois.size();
			myResults.push_back(res);
		}

		myHasResult = true;
	}

	bool hasResult() override
	{
		return myHasResult;
	}

	Vector<TempoResult> getResult() override
	{
		return myResults;
	}
};

TempoDetector* TempoDetector::New(double startTime, double length)
{
	return new TempoDetectorImpl(startTime, length);
}

}; // namespace Vortex
