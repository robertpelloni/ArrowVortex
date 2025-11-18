#pragma once

#include <Core/Draw.h>
#include <Core/Vector.h>
#include <Gist.h>

namespace Vortex {

struct Spectrogram
{
public:
    Spectrogram(int frameSize, int sampleRate);
    ~Spectrogram();

    void onChanges(int changes);

    void draw();

private:
    Gist<float> gist;
    Texture tex;
    std::vector<float> audioFrame;
    std::vector<float> onsetDetectionFunction;
};

}; // namespace Vortex
