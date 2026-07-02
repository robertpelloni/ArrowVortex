#pragma once

#include <Dialogs/Dialog.h>
#include <Core/Widgets.h>

namespace Vortex {

class DialogBobcoinWallet : public EditorDialog {
   public:
    DialogBobcoinWallet();
    ~DialogBobcoinWallet();

    void onChanges(int changes) override;
    void onTick() override;

   private:
    void myCreateWidgets();
    void myUpdateWidgets();

    WgLabel* myBalanceLabel;
    WgLabel* myAddressLabel;
    WgLineEdit* myRecipientInput;
    WgLineEdit* myAmountInput;
    WgButton* mySendButton;
    WgButton* myLoadButton;
    WgLineEdit* myPasswordInput;

    void onSend();
    void onLoad();
};

}  // namespace Vortex
