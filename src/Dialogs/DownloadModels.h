#pragma once

#include <Dialogs/Dialog.h>
#include <Core/Widgets.h>

namespace Vortex {

class DialogDownloadModels : public EditorDialog {
   public:
    DialogDownloadModels();
    ~DialogDownloadModels();

    void onTick() override;

   private:
    void myCreateWidgets();
    void myStartDownload();
    void myCancel();
    void myUpdateLog(StringRef text);

    WgTextbox* myLogBox;
    WgButton* myDownloadBtn;
    WgButton* myCancelBtn;

    struct DownloadThread;
    DownloadThread* myThread = nullptr;
    bool isDownloading = false;
    int myLastLogReadPos = 0;
};

};  // namespace Vortex
