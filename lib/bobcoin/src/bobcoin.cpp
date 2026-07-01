#include "bobcoin.h"

namespace Bobcoin {

static double g_balance = 0.0;

void Initialize() {
    // Reset or load from disk in future
    g_balance = 0.0;
}

void Mine(double effort) {
    if (effort > 0.0) {
        g_balance += effort;
    }
}

double GetBalance() {
    return g_balance;
}

}
