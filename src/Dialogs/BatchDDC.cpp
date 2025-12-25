#include <Dialogs/BatchDDC.h>
#include <Editor/Editor.h>
#include <Core/WidgetsLayout.h>
#include <System/System.h>
#include <System/File.h>
#include <Core/Utils.h>
#include <thread>

namespace Vortex {

DialogBatchDDC::~DialogBatchDDC() {}

DialogBatchDDC::DialogBatchDDC()
{
	setTitle("BATCH DDC GENERATION");
	
	// Default paths
	myOutDir = "output";
	myModelDir = "lib/ddc/models"; 
	myFFRModelDir = "lib/ddc/ffr_models";

	myCreateWidgets();
}

void DialogBatchDDC::myCreateWidgets()
{
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

	// Generate
	myLayout.row().col(300).h(30);
	WgButton* genBtn = myLayout.add<WgButton>();
	genBtn->text.set("GENERATE CHARTS");
	genBtn->onPress.bind(this, &DialogBatchDDC::myGenerate);

	// Log
	myLayout.row().col(300).h(100);
	myLogBox = myLayout.add<WgTextbox>();
	myLogBox->setReadOnly(true);
	myLogBox->setMultiLine(true);
	
	onUpdateSize();
}

void DialogBatchDDC::myAddFiles()
{
	String filters = "Audio Files (*.mp3, *.ogg, *.wav)\0*.mp3;*.ogg;*.wav\0All Files (*.*)\0*.*\0";
	String path = gSystem->openFileDlg("Select Audio Files", "", filters); 
	if(path.len()) {
		myFiles.push_back(path);
		myFileList->addItem(path);
	}
}

void DialogBatchDDC::myAddFolder()
{
	String path = gSystem->openFileDlg("Select Folder (Select any file inside)", "");
	if(path.len()) {
		Path p(path);
		String dir = p.dir();
		myFiles.push_back(dir);
		myFileList->addItem(dir);
	}
}

void DialogBatchDDC::myRemoveFiles()
{
	int idx = myFileList->getSelection();
	if(idx >= 0 && idx < myFiles.size()) {
		myFiles.erase(idx);
		myFileList->removeItem(idx);
	}
}

void DialogBatchDDC::mySelectOutDir()
{
	// Fallback to file dialog since openDirDlg is missing
	String path = gSystem->openFileDlg("Select Output Directory (Select any file inside)", myOutDir);
	if(path.len()) {
		Path p(path);
		myOutDir = p.dir();
	}
}

void DialogBatchDDC::mySelectModelDir()
{
	String path = gSystem->openFileDlg("Select Models Directory (Select any file inside)", myModelDir);
	if(path.len()) {
		Path p(path);
		myModelDir = p.dir();
	}
}

void DialogBatchDDC::mySelectFFRModelDir()
{
	String path = gSystem->openFileDlg("Select FFR Models Directory (Select any file inside)", myFFRModelDir);
	if(path.len()) {
		Path p(path);
		myFFRModelDir = p.dir();
	}
}

void DialogBatchDDC::myGenerate()
{
	if(myFiles.empty()) {
		myUpdateLog("No files selected.");
		return;
	}

	myUpdateLog("Starting generation...");

	// Construct command
	String cmd = gEditor->getPythonPath();
	cmd += " lib/ddc/autochart.py";
	
	cmd += " --out_dir \"";
	cmd += myOutDir;
	cmd += "\"";
	
	cmd += " --models_dir \"";
	cmd += myModelDir;
	cmd += "\"";

	if(myFFRModelDir.len()) {
		cmd += " --ffr_dir \"";
		cmd += myFFRModelDir;
		cmd += "\"";
	}

	// Add files/folders at the end (positional args)
	for(const auto& f : myFiles) {
		cmd += " \"";
		cmd += f;
		cmd += "\"";
	}

	// Redirect output to log file
	cmd += " > ddc_log.txt 2>&1";

	myUpdateLog("Executing: " + cmd);
	
	// Run in a separate thread to avoid freezing UI?
	// But gSystem->runSystemCommand might be blocking anyway.
	// Let's just run it.
	
	bool success = gSystem->runSystemCommand(cmd);
	
	if(success) {
		myUpdateLog("Command executed successfully.");
		// Read log file
		FileReader reader;
		if(reader.open("ddc_log.txt")) {
			String logContent;
			// Read all
			// Assuming FileReader has readAll or similar, or just read in chunks
			// Let's just read first 1024 bytes for now or implement a loop
			char buf[1024];
			while(true) {
				int read = reader.read(buf, 1, 1023);
				if(read <= 0) break;
				buf[read] = 0;
				logContent += buf;
			}
			myUpdateLog("Log Output:\n" + logContent);
		}
	} else {
		myUpdateLog("Command failed to execute.");
	}
}

void DialogBatchDDC::myUpdateLog(StringRef text)
{
	String current = myLogBox->text.get();
	if(current.len()) current += "\n";
	current += text;
	myLogBox->text.set(current);
}

}; // namespace Vortex
