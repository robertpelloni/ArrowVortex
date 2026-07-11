#include "ProofOfDance.h"
#include "bobcoin.h"
#include "DecentralizedPool.h"
#include <cmath>

namespace Bobcoin {

std::string ProofOfDance::GenerateAndSubmitProof(int difficultyMeter, int totalNotes, int maxCombo, float accuracy, const std::string& chartHash) {
    if (!IsWalletLoaded() || accuracy <= 0.0f) {
        return "";
    }

    // "Proof of Dance" effort calculation
    // Complexity scales with difficulty meter, notes hit, and combo multiplier
    double effort = (accuracy * totalNotes) * (1.0 + (maxCombo / 100.0)) * std::sqrt(difficultyMeter);

    // Scale effort down so it's not printing millions of coins
    double reward = effort * 0.001;

    // Mock cryptographic proof hash
    std::string proof = "pod_proof_" + chartHash + "_reward_" + std::to_string(reward);

    // Connect to decentralized pool and process
    DecentralizedPool::ProcessProof(proof, effort, reward);

    return proof;
}

}
