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

// Generates a mock RingCT stealth address using a simple random hash
static std::string GenerateStealthHash() {
    const char charset[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    std::string result = "stealth_";
    for(int i = 0; i < 32; i++) {
        result += charset[rand() % (sizeof(charset) - 1)];
    }
    return result;
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
    // Validate address format roughly
    if (address.length() < 10) return "";

    g_balance -= amount;

    return "txid_ringct_" + GenerateStealthHash();
}

}
