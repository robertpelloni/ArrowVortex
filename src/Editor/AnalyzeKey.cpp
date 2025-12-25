#include <Editor/AnalyzeKey.h>

#include <libaca/inc/Key.h>
#include <Core/Utils.h>

namespace Vortex {

String DetectKey(const float* samples, int numSamples, int sampleRate)
{
	CKey keyDetector;
	// Use default block length (4096) and hop length (2048)
	if (keyDetector.init(samples, numSamples, (float)sampleRate) != Error_t::kNoError)
	{
		return "Unknown";
	}

	int keyIdx = keyDetector.compKey();
	std::string keyStr = CKey::getKeyString((CKey::Keys_t)keyIdx);

	// Format nicely if needed, but libaca strings are usually "C Major", etc.
	return String(keyStr.c_str());
}

}
