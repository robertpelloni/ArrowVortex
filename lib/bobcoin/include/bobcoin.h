#pragma once

#include <string>

namespace Bobcoin {
    void Initialize();
    void Mine(double effort);
    double GetBalance();

    // Phase 3 Extensions
    bool CreateWallet(const std::string& password);
    bool LoadWallet(const std::string& password);
    bool IsWalletLoaded();

    std::string GetStealthAddress();

    // Returns txid on success, empty string on failure
    std::string SendTransaction(const std::string& address, double amount);
}
