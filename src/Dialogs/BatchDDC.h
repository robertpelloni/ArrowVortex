#pragma once

#include <Dialogs/Dialog.h>
#include <Core/Widgets.h>

namespace Vortex {

class DialogBatchDDC : public EditorDialog {
   public:
    ~DialogBatchDDC();
    DialogBatchDDC();

    void onTick() override;

   private:
    void myCreateWidgets();
    void myAddFiles();
    void myAddFolder();
    void myRemoveFiles();
    void mySelectOutDir();
    void mySelectModelDir();
    void mySelectFFRModelDir();
    void myGenerate();
    void myCancel();
    void myUpdateLog(StringRef text);

    WgListbox* myFileList;
    WgTextbox* myOutDirBox;
    WgTextbox* myModelDirBox;
    WgTextbox* myFFRModelDirBox;
    WgTextbox* myLogBox;
    WgButton* myGenBtn;
    WgButton* myCancelBtn;

    Vector<String> myFiles;
    String myOutDir;
    String myModelDir;
    String myFFRModelDir;

    struct DDCThread;
    DDCThread* myThread = nullptr;
    bool isGenerating = false;
    int myLastLogReadPos = 0;
};

};  // namespace Vortex
