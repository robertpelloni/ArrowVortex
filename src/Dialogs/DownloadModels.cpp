#include "DownloadModels.h"
#include <System/System.h>
#include <System/File.h>
#include <System/Thread.h>
#include <Core/Utils.h>
#include <Editor/Editor.h>

namespace Vortex {

struct DialogDownloadModels::DownloadThread : public BackgroundThread {
    String cmd;
    bool success = false;

    DownloadThread(StringRef c) : cmd(c) {}

    void exec() override { success = gSystem->runSystemCommand(cmd); }
};

DialogDownloadModels::DialogDownloadModels() {
    setTitle("DOWNLOAD DDC MODELS");
    setSize({400, 300});
    myCreateWidgets();
}

DialogDownloadModels::~DialogDownloadModels() {
    if (myThread) {
        myThread->terminate();
        delete myThread;
        myThread = nullptr;
    }
}

void DialogDownloadModels::myCreateWidgets() {
    myLayout.row().col(380).h(20);
    WgText* infoText = myLayout.add<WgText>();
    infoText->text.set("Download pre-trained PyTorch models for DDC.");

    myLayout.addSpace(10);

    myLayout.row().col(380).h(30);
    myProgressBar = myLayout.add<WgProgressBar>();
    myProgressBar->setProgress(0.0f);

    myLayout.addSpace(10);

    myLayout.row().col(380).h(30);
    WgSection* btnSec = myLayout.add<WgSection>();
    btnSec->row().col(180).col(180).h(30);

    myDownloadBtn = btnSec->add<WgButton>();
    myDownloadBtn->text.set("DOWNLOAD");
    myDownloadBtn->onPress.bind(this, &DialogDownloadModels::myStartDownload);

    myCancelBtn = btnSec->add<WgButton>();
    myCancelBtn->text.set("CANCEL");
    myCancelBtn->onPress.bind(this, &DialogDownloadModels::myCancel);
    myCancelBtn->setEnabled(false);

    myLayout.addSpace(10);

    myLayout.row().col(380).h(140);
    myLogBox = myLayout.add<WgTextbox>();
    myLogBox->setMultiline(true);
    myLogBox->setReadOnly(true);
}

void DialogDownloadModels::myStartDownload() {
    myLogBox->text.set("Preparing to download models...\n");

    String pythonPath = gEditor->getPythonPath();
    if (pythonPath.empty()) {
        myUpdateLog("ERROR: Python path not configured.");
        myUpdateLog("Please set Python path in Edit > Preferences.");
        return;
    }

    String exeDir = gSystem->getExeDir();
    String scriptPath = Path(exeDir, "lib/ddc/download_data.py");
    FileReader scriptCheck;
    if (!scriptCheck.open(scriptPath)) {
        myUpdateLog("ERROR: Download script not found at: " + scriptPath);
        return;
    }
    scriptCheck.close();

    String cmd = "\"";
    cmd += pythonPath;
    cmd += "\" \"";
    cmd += scriptPath;
    cmd += "\" --models_only";

    String logPath = Path(exeDir, "ddc_download_log.txt");
    cmd += " > \"";
    cmd += logPath;
    cmd += "\" 2>&1";

    myUpdateLog("Starting download process...");
    myUpdateLog("This may take several minutes.");

    myLastLogReadPos = 0;

    myThread = new DownloadThread(cmd);
    myProgressBar->setProgress(0.0f);
    myThread->start();

    isDownloading = true;
    myDownloadBtn->setEnabled(false);
    myCancelBtn->setEnabled(true);
}

void DialogDownloadModels::myCancel() {
    if (myThread) {
        myUpdateLog("\nCancelling download...");
        myThread->terminate();
        delete myThread;
        myThread = nullptr;
        isDownloading = false;
        myDownloadBtn->setEnabled(true);
        myCancelBtn->setEnabled(false);
    }
}

void DialogDownloadModels::myUpdateLog(StringRef text) {
    String current = myLogBox->text.get();
    if (current.len()) current += "\n";
    current += text;
    myLogBox->text.set(current);

}

void DialogDownloadModels::onTick() {
    EditorDialog::onTick();

    if (isDownloading && myThread) {
        String exeDir = gSystem->getExeDir();
        String logPath = Path(exeDir, "ddc_download_log.txt");

        FileReader reader;
        if (reader.open(logPath)) {
            reader.seek(myLastLogReadPos);
            String newContent;
            char buf[4096];
            while (true) {
                int read = reader.read(buf, 1, 4095);
                if (read <= 0) break;
                buf[read] = 0;
                newContent += buf;
            }
            myLastLogReadPos = reader.tell();
            reader.close();

            if (newContent.len() > 0) {
                String current = myLogBox->text.get();
                current += newContent;
                if (current.len() > 100000) {
                    current = current.substr(current.len() - 100000);
                }
                myLogBox->text.set(current);
                size_t pctPos = newContent.find("%|");
                if (pctPos != String::npos) {
                    myProgressBar->setProgress(myProgressBar->getProgress() + 0.05f);
                }
                myLogBox->setScrollPos(1.0);
            }
        }

        if (myThread->isDone()) {
            bool success = myThread->success;

            myThread->waitUntilDone();
            delete myThread;
            myThread = nullptr;

            if (success) {
                myUpdateLog("\nDownload complete!");
                myProgressBar->setProgress(1.0f);
            } else {
                myUpdateLog("\nERROR: Download failed. Check log for details.");
            }

            isDownloading = false;
            myDownloadBtn->setEnabled(true);
            myCancelBtn->setEnabled(false);
        }
    }
}
