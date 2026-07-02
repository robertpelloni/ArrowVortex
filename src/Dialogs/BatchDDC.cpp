#include <Dialogs/BatchDDC.h>
#include <Editor/Editor.h>
#include <Core/WidgetsLayout.h>
#include <System/System.h>
#include <System/File.h>
#include <Core/Utils.h>
#include <System/Thread.h>
#include <thread>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef ERROR

namespace Vortex {

struct DialogBatchDDC::DDCThread : public BackgroundThread {
    String cmd;
    bool success = false;

    DDCThread(StringRef c) : cmd(c) {}

    void exec() override { success = gSystem->runSystemCommand(cmd); }
};

DialogBatchDDC::~DialogBatchDDC() {
    if (myThread) {
        myThread->terminate();
        delete myThread;
        myThread = nullptr;
    }
}

DialogBatchDDC::DialogBatchDDC() {
    setTitle("BATCH DDC GENERATION");

    // Default paths - use absolute paths from exe directory
    String exeDir = gSystem->getExeDir();
    myOutDir = Path(exeDir, "output");

    // Check for models in bin/models first (distribution state), fallback to
    // lib/ddc/models (dev state)
    myModelDir = Path(exeDir, "models");
    DWORD attr = GetFileAttributesW(Widen(myModelDir).str());
    if (attr == INVALID_FILE_ATTRIBUTES || !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
        myModelDir = Path(exeDir, "lib/ddc/models");
    }

    myFFRModelDir = Path(exeDir, "lib/ddc/ffr_models");

    myCreateWidgets();
}

void DialogBatchDDC::myCreateWidgets() {
    myLayout.row().col(true);
    myLayout.addBlank();

    // File List
    myLayout.row().col(300).h(200);
    myFileList = myLayout.add<WgListbox>();

    // File Buttons
    myLayout.row().col(95).col(10).col(95).col(10).col(90);
    WgButton* addBtn = myLayout.add<WgButton>();
    addBtn->text.set("Add Files...");
    addBtn->onPress.bind(this, &DialogBatchDDC::myAddFiles);

    WgButton* addFolderBtn = myLayout.add<WgButton>();
    addFolderBtn->text.set("Add Folder...");
    addFolderBtn->onPress.bind(this, &DialogBatchDDC::myAddFolder);

    WgButton* remBtn = myLayout.add<WgButton>();
    remBtn->text.set("Remove");
    remBtn->onPress.bind(this, &DialogBatchDDC::myRemoveFiles);

    // Output Dir
    myLayout.row().col(300);
    WgLabel* l1 = myLayout.add<WgLabel>();
    l1->text.set("Output Directory:");

    myLayout.row().col(270).col(30);
    myOutDirBox = myLayout.add<WgTextbox>();
    myOutDirBox->text.bind(&myOutDir);

    WgButton* outBtn = myLayout.add<WgButton>();
    outBtn->text.set("...");
    outBtn->onPress.bind(this, &DialogBatchDDC::mySelectOutDir);

    // Model Dir
    myLayout.row().col(300);
    WgLabel* l2 = myLayout.add<WgLabel>();
    l2->text.set("DDC Models Directory:");

    myLayout.row().col(270).col(30);
    myModelDirBox = myLayout.add<WgTextbox>();
    myModelDirBox->text.bind(&myModelDir);

    WgButton* modelBtn = myLayout.add<WgButton>();
    modelBtn->text.set("...");
    modelBtn->onPress.bind(this, &DialogBatchDDC::mySelectModelDir);

    // FFR Model Dir
    myLayout.row().col(300);
    WgLabel* l3 = myLayout.add<WgLabel>();
    l3->text.set("FFR Models Directory:");

    myLayout.row().col(270).col(30);
    myFFRModelDirBox = myLayout.add<WgTextbox>();
    myFFRModelDirBox->text.bind(&myFFRModelDir);

    WgButton* ffrModelBtn = myLayout.add<WgButton>();
    ffrModelBtn->text.set("...");
    ffrModelBtn->onPress.bind(this, &DialogBatchDDC::mySelectFFRModelDir);

    // Generate and Cancel
    myLayout.row().col(300).h(30);
    WgSection* btnSec = myLayout.add<WgSection>();
    btnSec->row().col(150).col(150).h(30);
    myGenBtn = btnSec->add<WgButton>();
    myGenBtn->text.set("GENERATE CHARTS");
    myGenBtn->onPress.bind(this, &DialogBatchDDC::myGenerate);
    myCancelBtn = btnSec->add<WgButton>();
    myCancelBtn->text.set("CANCEL");
    myCancelBtn->onPress.bind(this, &DialogBatchDDC::myCancel);
    myCancelBtn->setEnabled(false);

    // Progress Bar
    myLayout.row().col(300).h(30);
    myProgressBar = myLayout.add<WgProgressBar>();
    myProgressBar->setProgress(0.0f);

    // Log
    myLayout.row().col(300).h(100);
    myLogBox = myLayout.add<WgTextbox>();
    myLogBox->setReadOnly(true);
    myLogBox->setMultiLine(true);

    onUpdateSize();
}

void DialogBatchDDC::myAddFiles() {
    String filters =
        "Audio Files (*.mp3, *.ogg, *.wav)\0*.mp3;*.ogg;*.wav\0All Files "
        "(*.*)\0*.*\0";
    String path = gSystem->openFileDlg("Select Audio Files", "", filters);
    if (path.len()) {
        myFiles.push_back(path);
        myFileList->addItem(path);
    }
}

void DialogBatchDDC::myAddFolder() {
    String path =
        gSystem->openFileDlg("Select Folder (Select any file inside)", "");
    if (path.len()) {
        Path p(path);
        String dir = p.dir();
        myFiles.push_back(dir);
        myFileList->addItem(dir);
    }
}

void DialogBatchDDC::myRemoveFiles() {
    int idx = myFileList->getSelection();
    if (idx >= 0 && idx < myFiles.size()) {
        myFiles.erase(idx);
        myFileList->removeItem(idx);
    }
}

void DialogBatchDDC::mySelectOutDir() {
    // Fallback to file dialog since openDirDlg is missing
    String path = gSystem->openFileDlg(
        "Select Output Directory (Select any file inside)", myOutDir);
    if (path.len()) {
        Path p(path);
        myOutDir = p.dir();
    }
}

void DialogBatchDDC::mySelectModelDir() {
    String path = gSystem->openFileDlg(
        "Select Models Directory (Select any file inside)", myModelDir);
    if (path.len()) {
        Path p(path);
        myModelDir = p.dir();
    }
}

void DialogBatchDDC::mySelectFFRModelDir() {
    String path = gSystem->openFileDlg(
        "Select FFR Models Directory (Select any file inside)", myFFRModelDir);
    if (path.len()) {
        Path p(path);
        myFFRModelDir = p.dir();
    }
}

void DialogBatchDDC::myGenerate() {
    // Clear previous log
    myLogBox->text.set("");

    // Validation 1: Check files selected
    if (myFiles.empty()) {
        myUpdateLog("ERROR: No files selected.");
        myUpdateLog(
            "Please add audio files or folders using the buttons above.");
        return;
    }

    myUpdateLog("Starting validation...");

    // Validation 2: Check Python path
    String pythonPath = gEditor->getPythonPath();
    if (pythonPath.empty()) {
        myUpdateLog("ERROR: Python path not configured.");
        myUpdateLog("Please set Python path in Edit > Preferences.");
        return;
    }

    // Validation 3: Check Python executable exists (basic check)
    // Note: We can't easily check if python.exe exists if it's in PATH
    // So we'll just warn if it looks suspicious
    if (pythonPath != "python" && pythonPath != "python3") {
        FileReader pythonCheck;
        if (!pythonCheck.open(pythonPath)) {
            myUpdateLog("WARNING: Python executable not found at: " +
                        pythonPath);
            myUpdateLog(
                "If Python is in your PATH, you can ignore this warning.");
        }
        pythonCheck.close();
    }

    // Validation 4: Check autochart.py exists
    String exeDir = gSystem->getExeDir();
    String scriptPath = Path(exeDir, "lib/ddc/autochart.py");
    FileReader scriptCheck;
    if (!scriptCheck.open(scriptPath)) {
        myUpdateLog("ERROR: DDC script not found at: " + scriptPath);
        myUpdateLog(
            "Please ensure lib/ddc/autochart.py exists in the ArrowVortex "
            "directory.");
        return;
    }
    scriptCheck.close();

    // Validation 5: Check model directory exists
    DWORD modelDirAttr = GetFileAttributesW(Widen(myModelDir).str());
    if (modelDirAttr == INVALID_FILE_ATTRIBUTES) {
        myUpdateLog("ERROR: Model directory not found: " + myModelDir);
        myUpdateLog(
            "Please train DDC models first or select correct directory.");
        myUpdateLog("See documentation for training instructions.");
        return;
    }
    if (!(modelDirAttr & FILE_ATTRIBUTE_DIRECTORY)) {
        myUpdateLog("ERROR: Model path is not a directory: " + myModelDir);
        return;
    }

    // Validation 6: Check if at least one model exists (onset model)
    // Try both .pth (PyTorch) and .h5 (Legacy Keras)
    String onsetModelPath = Path(myModelDir, "onset/model.pth");
    FileReader onsetCheck;
    if (!onsetCheck.open(onsetModelPath)) {
        onsetModelPath = Path(myModelDir, "onset/model.h5");
        if (!onsetCheck.open(onsetModelPath)) {
            // Try model_05.pth which is common for 5-epoch training
            onsetModelPath = Path(myModelDir, "onset/model_05.pth");
            if (!onsetCheck.open(onsetModelPath)) {
                myUpdateLog("WARNING: Onset model not found in " + myModelDir +
                            "/onset/");
                myUpdateLog("Expected model.pth, model_05.pth, or model.h5.");
                myUpdateLog(
                    "Models may not be trained yet. Generation will likely "
                    "fail.");
                myUpdateLog("See documentation for training instructions.");
            }
        }
    }
    onsetCheck.close();

    // Validation 7: Check output directory exists or can be created
    DWORD outDirAttr = GetFileAttributesW(Widen(myOutDir).str());
    if (outDirAttr == INVALID_FILE_ATTRIBUTES) {
        myUpdateLog("Output directory does not exist: " + myOutDir);
        myUpdateLog("Attempting to create it...");
        if (!createFolder(myOutDir)) {
            myUpdateLog("ERROR: Failed to create output directory.");
            myUpdateLog("Please check permissions and path validity.");
            return;
        }
        myUpdateLog("Output directory created successfully.");
    }

    // Validation 8: Check input files exist and are valid audio formats
    for (const auto& f : myFiles) {
        DWORD attr = GetFileAttributesW(Widen(f).str());
        if (attr == INVALID_FILE_ATTRIBUTES) {
            myUpdateLog("ERROR: File or folder not found: " + f);
            return;
        }

        // If it's a file (not directory), check extension
        if (!(attr & FILE_ATTRIBUTE_DIRECTORY)) {
            Path p(f);
            if (!p.hasExt("mp3") && !p.hasExt("ogg") && !p.hasExt("wav")) {
                myUpdateLog(
                    "WARNING: File may not be a supported audio format: " + f);
                myUpdateLog("Supported formats: .mp3, .ogg, .wav");
            }
        }
    }

    myUpdateLog("Validation complete. Starting generation...");

    // Construct command using absolute paths
    String cmd = pythonPath;
    cmd += " \"";
    cmd += scriptPath;
    cmd += "\"";

    cmd += " --out_dir \"";
    cmd += myOutDir;
    cmd += "\"";

    cmd += " --models_dir \"";
    cmd += myModelDir;
    cmd += "\"";

    // FFR models are optional
    if (myFFRModelDir.len()) {
        DWORD ffrAttr = GetFileAttributesW(Widen(myFFRModelDir).str());
        if (ffrAttr != INVALID_FILE_ATTRIBUTES &&
            (ffrAttr & FILE_ATTRIBUTE_DIRECTORY)) {
            cmd += " --ffr_dir \"";
            cmd += myFFRModelDir;
            cmd += "\"";
        } else {
            myUpdateLog(
                "WARNING: FFR model directory not found, skipping difficulty "
                "rating.");
        }
    }

    // Add files/folders at the end (positional args)
    for (const auto& f : myFiles) {
        cmd += " \"";
        cmd += f;
        cmd += "\"";
    }

    // Redirect output to log file in exe directory
    String logPath = Path(exeDir, "ddc_log.txt");
    cmd += " > \"";
    cmd += logPath;
    cmd += "\" 2>&1";

    myUpdateLog("Executing command...");
    myUpdateLog("(This may take several minutes depending on file count)");
    myUpdateLog("");  // Blank line for readability

    // Ensure we start reading from the beginning of the new log
    myLastLogReadPos = 0;

    // Reset progress
    myProgressBar->setProgress(0.0f);

    // Start background thread
    myThread = new DDCThread(cmd);
    myThread->start();

    isGenerating = true;
    myGenBtn->setEnabled(false);
    myCancelBtn->setEnabled(true);
}

void DialogBatchDDC::myCancel() {
    if (myThread) {
        myUpdateLog("\nCancelling generation...");
        myThread->terminate();
        delete myThread;
        myThread = nullptr;
        isGenerating = false;
        myGenBtn->setEnabled(true);
        myCancelBtn->setEnabled(false);
    }
}

void DialogBatchDDC::myUpdateLog(StringRef text) {
    String current = myLogBox->text.get();
    if (current.len()) current += "\n";
    current += text;
    myLogBox->text.set(current);
}

void DialogBatchDDC::onTick() {
    EditorDialog::onTick();

    if (isGenerating && myThread) {
        String exeDir = gSystem->getExeDir();
        String logPath = Path(exeDir, "ddc_log.txt");

        // Periodically read log
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
                // Ensure we don't spam newlines if content already has them
                String current = myLogBox->text.get();
                current += newContent;

                // Limit log size to prevent UI issues
                if (current.len() > 100000) {
                    current = current.substr(current.len() - 100000);
                }

                myLogBox->text.set(current);

                // Update progress based on "Processing " mentions
                // Note: autochart_lib.py prints "Processing <file>..."
                int processedCount = 0;
                size_t pos = 0;
                while ((pos = current.find("Processing ", pos)) !=
                       String::npos) {
                    processedCount++;
                    pos += 11;  // length of "Processing "
                }

                // Approximate total count
                int totalCount = myFiles.size();
                if (totalCount > 0) {
                    // It could be more if they selected folders, but we
                    // estimate based on files/folders added Limit to 0.99 so it
                    // doesn't look fully done until the thread actually
                    // finishes
                    float progress =
                        min(0.99f, static_cast<float>(processedCount) /
                                       static_cast<float>(totalCount));
                    myProgressBar->setProgress(progress);
                }
            }
        }

        if (myThread->isDone()) {
            bool success = myThread->success;

            myThread->waitUntilDone();
            delete myThread;
            myThread = nullptr;

            if (success) {
                myProgressBar->setProgress(1.0f);
                myUpdateLog("");
                myUpdateLog("Generation complete!");
                myUpdateLog("Check output directory: " + myOutDir);
            } else {
                myUpdateLog("");
                myUpdateLog("ERROR: Command failed to execute.");
                myUpdateLog("Possible reasons:");
                myUpdateLog("- Python not found or not installed");
                myUpdateLog(
                    "- Required Python packages not installed (run: pip "
                    "install -r "
                    "lib/ddc/requirements.txt)");
                myUpdateLog("- Invalid file paths");
                myUpdateLog("- Models not trained");
                myUpdateLog("");
                myUpdateLog("Check the log file for details: " + logPath);
            }

            isGenerating = false;
            myGenBtn->setEnabled(true);
            myCancelBtn->setEnabled(false);
        }
    }
}

};  // namespace Vortex
