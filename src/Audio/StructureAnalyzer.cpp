#include <Editor/StructureAnalyzer.h>
#include <Gist.h>
#include <Core/Utils.h>
#include <vector>
#include <cmath>
#include <algorithm>

namespace Vortex {

static float CosineDistance(const std::vector<float>& a, const std::vector<float>& b)
{
    float dot = 0.0f;
    float magA = 0.0f;
    float magB = 0.0f;
    for (size_t i = 0; i < a.size(); ++i) {
        dot += a[i] * b[i];
        magA += a[i] * a[i];
        magB += b[i] * b[i];
    }
    if (magA == 0 || magB == 0) return 1.0f;
    return 1.0f - (dot / (sqrt(magA) * sqrt(magB)));
}

void FindSections(const float* samples, int samplerate, int numFrames, Vector<Section>& out)
{
    const int frameSize = 4096; // Large window for texture
    const int hopSize = 2048;

    if (numFrames < frameSize) return;

    Gist<float> gist(frameSize, samplerate);
    std::vector<float> audioFrame(frameSize);

    // 1. Extract MFCCs
    std::vector<std::vector<float>> features;
    int numBlocks = (numFrames - frameSize) / hopSize;

    for (int i = 0; i < numBlocks; ++i) {
        int start = i * hopSize;
        for (int j = 0; j < frameSize; ++j) {
            audioFrame[j] = samples[start + j];
        }
        gist.processAudioFrame(audioFrame);
        features.push_back(gist.getMelFrequencyCepstralCoefficients());
    }

    if (features.empty()) return;

    // 2. Compute Self-Similarity Matrix (SSM) / Novelty Curve
    // Checkerboard kernel size
    const int kernelSize = 16; // blocks
    if (features.size() < kernelSize * 2) return;

    std::vector<float> novelty(features.size(), 0.0f);

    for (int i = kernelSize; i < features.size() - kernelSize; ++i) {
        // Convolve with checkerboard kernel along diagonal
        //       +   -
        //     |---|---|
        //   + | A | B |
        //     |---|---|
        //   - | C | D |
        //     |---|---|
        // Distance SSM: High distance = Dissimilar.
        // Novelty is high when A and D (self-sim) are low distance (high sim)
        // and B and C (cross-sim) are high distance (low sim).
        // So we sum distances in B+C and subtract A+D.

        float distAB = 0.0f; // Cross (Past vs Future)
        float distCD = 0.0f; // Cross (Future vs Past)
        float distAD = 0.0f; // Self (Past vs Past + Future vs Future)

        // A: [i-L, i] x [i-L, i]
        // B: [i-L, i] x [i, i+L]
        // C: [i, i+L] x [i-L, i]
        // D: [i, i+L] x [i, i+L]

        for (int x = 0; x < kernelSize; ++x) {
            for (int y = 0; y < kernelSize; ++y) {
                // A
                distAD += CosineDistance(features[i - 1 - x], features[i - 1 - y]);
                // D
                distAD += CosineDistance(features[i + x], features[i + y]);
                // B
                distAB += CosineDistance(features[i - 1 - x], features[i + y]);
                // C
                distCD += CosineDistance(features[i + x], features[i - 1 - y]);
            }
        }

        // Gaussian weight could be applied here, but box is fine for simple version.
        // Novelty = (Cross Similarity - Self Similarity).
        // We computed Distances.
        // Sim = 1 - Dist.
        // Nov = (Sim_Cross_High? No.
        // We want transition when Past is similar to Past, Future to Future, but Past != Future.
        // So Dist(Past, Future) is High. Dist(Self) is Low.
        // Score = (Dist_B + Dist_C) - (Dist_A + Dist_D).

        novelty[i] = (distAB + distCD) - distAD;
    }

    // 3. Peak Picking
    // Normalize
    float maxNov = 0.0f;
    for (float v : novelty) maxNov = std::max(maxNov, v);
    if (maxNov == 0) return;

    for (int i = kernelSize; i < novelty.size() - kernelSize; ++i) {
        float val = novelty[i] / maxNov;
        if (val > 0.2f) { // Threshold
            // Local max
            bool isPeak = true;
            int win = 10;
            for (int k = std::max(0, i - win); k <= std::min((int)novelty.size() - 1, i + win); ++k) {
                if (novelty[k] > novelty[i]) {
                    isPeak = false;
                    break;
                }
            }

            if (isPeak) {
                double time = (double)(i * hopSize) / samplerate;
                out.push_back({time, (double)val});
                // Skip forward to avoid clusters
                i += win;
            }
        }
    }
}

}; // namespace Vortex
