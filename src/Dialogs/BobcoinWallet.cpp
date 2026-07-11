#include "BobcoinWallet.h"
#include <Managers/MiningMan.h>
#include <Core/StringUtils.h>
#include <Core/Widgets.h>
#include <System/Thread.h>
#include <bobcoin.h>
#include <thread>

namespace Vortex {

struct DialogBobcoinWallet::WalletThread : public BackgroundThread {
    String address;
    double amount;
    String txid;
    bool success = false;

    WalletThread(StringRef a, double amt) : address(a), amount(amt) {}

    void exec() override {
        // Sleep to simulate network delay for transaction processing
        std::this_thread::sleep_for(std::chrono::seconds(2));
        std::string res = Bobcoin::SendTransaction(address.str(), amount);
        txid = res.c_str();
        success = !res.empty();
    }
};

DialogBobcoinWallet::DialogBobcoinWallet() {
    setTitle("BOBCOIN WALLET");
    setMinimumWidth(350);
    myCreateWidgets();
    myUpdateWidgets();
}

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

    myLayout.row().col(350).h(20);
    myStatusLabel = myLayout.add<WgLabel>();
    myStatusLabel->text.set("");
}

DialogBobcoinWallet::~DialogBobcoinWallet() {
    if (myThread) {
        myThread->terminate();
        delete myThread;
        myThread = nullptr;
    }
}

void DialogBobcoinWallet::myUpdateWidgets() {
    if (isSending) return; // Don't update buttons while sending

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
    if (!Bobcoin::IsWalletLoaded() || isSending) return;

    String addr = myRecipientInput->text.get();
    String amountStr = myAmountInput->text.get();

    if (addr.len() > 0 && amountStr.len() > 0) {
        double amount = Str::toDouble(amountStr);
        if (amount > 0 && amount <= Bobcoin::GetBalance()) {
            isSending = true;
            mySendButton->setEnabled(false);
            myStatusLabel->text.set("Sending transaction...");

            myThread = new WalletThread(addr, amount);
            myThread->start();
        } else {
            myStatusLabel->text.set("Invalid amount or insufficient balance.");
        }
    }
}

void DialogBobcoinWallet::onChanges(int changes) {
    myUpdateWidgets();
}

void DialogBobcoinWallet::onTick() {
    EditorDialog::onTick();

    if (isSending && myThread) {
        if (myThread->isDone()) {
            bool success = myThread->success;
            String txid = myThread->txid;

            myThread->waitUntilDone();
            delete myThread;
            myThread = nullptr;
            isSending = false;

            if (success) {
                myRecipientInput->text.set("");
                myAmountInput->text.set("");
                myStatusLabel->text.set(Str::fmt("Success! TXID: %s", txid.str()).str());
            } else {
                myStatusLabel->text.set("Transaction failed.");
            }

            myUpdateWidgets();
        }
    } else if (!isSending) {
        myUpdateWidgets();
    }
}

} // namespace Vortex
