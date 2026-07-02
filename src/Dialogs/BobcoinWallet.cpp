#include "BobcoinWallet.h"
#include <Managers/MiningMan.h>
#include <Core/StringUtils.h>
#include <Core/Widgets.h>

namespace Vortex {

DialogBobcoinWallet::DialogBobcoinWallet() {
    setTitle("BOBCOIN WALLET");
    setMinimumWidth(300);
    myCreateWidgets();
    myUpdateWidgets();
}

DialogBobcoinWallet::~DialogBobcoinWallet() {}

void DialogBobcoinWallet::myCreateWidgets() {
    myLayout.row().col(300).h(30);
    myLayout.add<WgLabel>()->text.set("Wallet Balance:");

    myLayout.row().col(300).h(40);
    myBalanceLabel = myLayout.add<WgLabel>();
    myBalanceLabel->text.set("0.0 BC");
}

void DialogBobcoinWallet::myUpdateWidgets() {
    if (gMining) {
        double balance = gMining->getBalance();
        myBalanceLabel->text.set(Str::fmt("%.4f BC", balance).str());
    }
}

void DialogBobcoinWallet::onChanges(int changes) {
    // If we trigger changes periodically or something triggers wallet update,
    // update the UI
    myUpdateWidgets();
}

void DialogBobcoinWallet::onTick() {
    // Continuously update the wallet balance in UI (since Practice Mode might
    // not emit generic changes)
    myUpdateWidgets();
    EditorDialog::onTick();
}

}  // namespace Vortex
