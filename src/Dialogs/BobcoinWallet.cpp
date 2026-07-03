#include "BobcoinWallet.h"
#include <Managers/MiningMan.h>
#include <Core/StringUtils.h>
#include <Core/Widgets.h>
#include <bobcoin.h>

namespace Vortex {

DialogBobcoinWallet::DialogBobcoinWallet() {
    setTitle("BOBCOIN WALLET");
    setMinimumWidth(350);
    myCreateWidgets();
    myUpdateWidgets();
}

DialogBobcoinWallet::~DialogBobcoinWallet() {}

void DialogBobcoinWallet::myCreateWidgets() {
    myLayout.row().col(350).h(30);
    myLayout.add<WgLabel>()->text.set("Wallet Balance:");

    myLayout.row().col(350).h(40);
    myBalanceLabel = myLayout.add<WgLabel>();
    myBalanceLabel->text.set("0.0 BC");

    myLayout.row().col(350).h(30);
    myLayout.add<WgLabel>()->text.set("Wallet Password:");

    myLayout.row().col(200).h(30).col(100).h(30);
    myPasswordInput = myLayout.add<WgLineEdit>();
    myLoadButton = myLayout.add<WgButton>();
    myLoadButton->text.set("Load/Create");
    myLoadButton->onPress.bind(this, &DialogBobcoinWallet::onLoad);

    myLayout.row().col(350).h(30);
    myAddressLabel = myLayout.add<WgLabel>();
    myAddressLabel->text.set("Not loaded");

    myLayout.row().col(350).h(30);
    myLayout.add<WgSeperator>();

    myLayout.row().col(350).h(30);
    myLayout.add<WgLabel>()->text.set("Send Bobcoin (RingCT):");

    myLayout.row().col(350).h(30);
    myRecipientInput = myLayout.add<WgLineEdit>();

    myLayout.row().col(150).h(30).col(150).h(30);
    myAmountInput = myLayout.add<WgLineEdit>();
    mySendButton = myLayout.add<WgButton>();
    mySendButton->text.set("Send");
    mySendButton->onPress.bind(this, &DialogBobcoinWallet::onSend);
}

void DialogBobcoinWallet::myUpdateWidgets() {
    if (gMining) {
        double balance = gMining->getBalance();
        myBalanceLabel->text.set(Str::fmt("%.4f BC", balance).str());
    }

    if (Bobcoin::IsWalletLoaded()) {
        myAddressLabel->text.set(Str::fmt("Addr: %s", Bobcoin::GetStealthAddress().c_str()).str());
        mySendButton->setEnabled(true);
    } else {
        myAddressLabel->text.set("Wallet Not Loaded");
        mySendButton->setEnabled(false);
    }
}

void DialogBobcoinWallet::onLoad() {
    String pass = myPasswordInput->text.get();
    if (pass.len() > 0) {
        // Attempt load, fallback to create
        if (!Bobcoin::LoadWallet(pass.str())) {
            Bobcoin::CreateWallet(pass.str());
        }
        myUpdateWidgets();
    }
}

void DialogBobcoinWallet::onSend() {
    if (!Bobcoin::IsWalletLoaded()) return;

    String addr = myRecipientInput->text.get();
    String amountStr = myAmountInput->text.get();

    if (addr.len() > 0 && amountStr.len() > 0) {
        double amount = Str::toDouble(amountStr);
        if (amount > 0 && amount <= Bobcoin::GetBalance()) {
            std::string txid = Bobcoin::SendTransaction(addr.str(), amount);
            if (!txid.empty()) {
                // Success
                myRecipientInput->text.set("");
                myAmountInput->text.set("");
            }
        }
    }
}

void DialogBobcoinWallet::onChanges(int changes) {
    myUpdateWidgets();
}

void DialogBobcoinWallet::onTick() {
    myUpdateWidgets();
    EditorDialog::onTick();
}

} // namespace Vortex
