#pragma once

#include <vector>
#include <string>

namespace Bobcoin {

class ProofOfDance {
public:
    // Generates a local cryptographic proof of the chart performance
    // and submits it to the decentralized pool for Bobcoin reward distribution.
    // In a real system, this would hash the metrics with a user's private key
    // and broadcast the proof to validators via an RPC node.
    static std::string GenerateAndSubmitProof(int difficultyMeter, int totalNotes, int maxCombo, float accuracy, const std::string& chartHash);
};

}
