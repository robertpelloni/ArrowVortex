#pragma once

#include <Core/String.h>

namespace Vortex {

	/// Detects the musical key of the provided audio samples.
	/// Returns a string describing the key (e.g. "C Major", "A Minor").
	String DetectKey(const float* samples, int numSamples, int sampleRate);

}
