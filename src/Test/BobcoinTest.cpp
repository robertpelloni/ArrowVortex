#include <Precomp.h>
#include "TestUtils.h"

#ifdef UNIT_TEST_BUILD

#include <bobcoin.h>
#include <ProofOfDance.h>
#include <DecentralizedPool.h>

namespace Vortex {

using namespace std;

TestMethod(BobcoinIntegrationTest)
{
    // Test Initialization
    Bobcoin::Initialize();
    Bobcoin::DecentralizedPool::Initialize();

    // Initial State Check
    Check(Bobcoin::GetBalance() == 0.0);
    Check(!Bobcoin::IsWalletLoaded());

    // Wallet Operations
    Bobcoin::CreateWallet("testpass");
    Check(Bobcoin::IsWalletLoaded() == true);

    // Direct Mining
    Bobcoin::Mine(1.5);
    Check(Bobcoin::GetBalance() == 1.5);

    // Proof of Dance Generation and Pool Interaction
    Bobcoin::DecentralizedPool::ConnectToNetwork({"node.mock.com"});

    std::string proof = Bobcoin::ProofOfDance::GenerateAndSubmitProof(
        10, // difficulty
        100, // total notes
        50, // max combo
        0.95f, // accuracy
        "mockhash_123"
    );

    Check(!proof.empty());
    Check(Bobcoin::GetBalance() > 1.5); // Verify PoD updated balance via pool

    // Transaction Signing Flow
    double startBalance = Bobcoin::GetBalance();
    std::string txid = Bobcoin::SendTransaction("test_recipient_address_001", 1.0);

    Check(!txid.empty());
    Check(Bobcoin::GetBalance() == (startBalance - 1.0));
}

}; // namespace Vortex

#endif // UNIT_TEST_BUILD
