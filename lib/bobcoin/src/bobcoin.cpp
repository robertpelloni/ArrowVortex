#include "bobcoin.h"
#include <iostream>

namespace Bobcoin {

static double g_balance = 0.0;
static bool g_walletLoaded = false;
static std::string g_address = "stealth_addr_placeholder_948fjc2a";

void Initialize() {
    // Reset or load from disk in future
    g_balance = 0.0;
    g_walletLoaded = false;
}

void Mine(double effort) {
    if (effort > 0.0) {
        g_balance += effort;
    }
}

double GetBalance() {
    return g_balance;
}

bool CreateWallet(const std::string& password) {
    // Mock implementation
    g_walletLoaded = true;
    return true;
}

bool LoadWallet(const std::string& password) {
    // Mock implementation
    if (password == "password") { // very secure
        g_walletLoaded = true;
        return true;
    }
    return false;
}

bool IsWalletLoaded() {
    return g_walletLoaded;
}

std::string GetStealthAddress() {
    if (!g_walletLoaded) return "";
    return g_address;
}

std::string SendTransaction(const std::string& address, double amount) {
    if (!g_walletLoaded || amount > g_balance) {
        return "";
    }

    // Apply RingCT/stealth logic (mocked)
    g_balance -= amount;

    return "txid_mock_1234567890abcdef";
}

}
