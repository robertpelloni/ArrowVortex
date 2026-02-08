#pragma once

#include <Core/Vector.h>

namespace Vortex {

struct Section {
    double time;
    double score;
};

void FindSections(const float* samples, int samplerate, int numFrames, Vector<Section>& out);

}; // namespace Vortex
