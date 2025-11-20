#include "Spectrogram.h"
#include "Music.h"
#include "View.h"

namespace Vortex {

Spectrogram::Spectrogram(int frameSize, int sampleRate)
    : gist(frameSize, sampleRate)
{
    audioFrame.resize(frameSize);
    onsetDetectionFunction.resize(0);
}

Spectrogram::~Spectrogram()
{
}

void Spectrogram::onChanges(int changes)
{
}

void Spectrogram::draw()
{
    auto& music = gMusic->getSamples();
    if (!music.isCompleted())
        return;

    int frameSize = gist.getAudioFrameSize();
    int sampleRate = gist.getSamplingFrequency();
    int numFrames = music.getNumFrames();
    int numBlocks = numFrames / frameSize;

    if (!tex.handle() || tex.size().x != numBlocks || tex.size().y != frameSize / 2)
    {
        tex = Texture(numBlocks, frameSize / 2, Texture::ALPHA);
    }

    std::vector<uchar> texBuf(numBlocks * (frameSize / 2));
    onsetDetectionFunction.resize(numBlocks);

    for (int i = 0; i < numBlocks; ++i)
    {
        for (int j = 0; j < frameSize; ++j)
        {
            audioFrame[j] = music.samplesL()[i * frameSize + j];
        }

        gist.processAudioFrame(audioFrame);
        onsetDetectionFunction[i] = gist.energyDifference();
        const std::vector<float>& magSpec = gist.getMagnitudeSpectrum();

        for (int j = 0; j < frameSize / 2; ++j)
        {
            texBuf[i * (frameSize / 2) + j] = (uchar)(log10(1 + magSpec[j] * 1000) * 255);
        }
    }

    tex.modify(0, 0, numBlocks, frameSize / 2, texBuf.data());

    int w = gView->getWidth();
    int h = gView->getHeight();
    Draw::fill({0, 0, w, h}, Colors::white, tex.handle());

    for (int i = 0; i < numBlocks; ++i)
    {
        float onset = onsetDetectionFunction[i];
        if (onset > 0.1)
        {
            int x = (int)((float)i / numBlocks * w);
            int y = (int)(onset * h);
            Draw::fill({x, h - y, 1, y}, Colors::red);
        }
    }
}

}; // namespace Vortex
