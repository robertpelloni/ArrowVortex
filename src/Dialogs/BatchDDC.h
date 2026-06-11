#pragma once

#include <Dialogs/Dialog.h>
#include <Core/Widgets.h>
#include <System/Thread.h>

namespace Vortex {

class DialogBatchDDC : public EditorDialog
{
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
	void myHandleCompletion();
	void myUpdateLog(StringRef text);

	WgListbox* myFileList;
	WgTextbox* myOutDirBox;
	WgTextbox* myModelDirBox;
	WgTextbox* myFFRModelDirBox;
	WgTextbox* myLogBox;
	WgButton* myGenerateBtn;
	
	Vector<String> myFiles;
	String myOutDir;
	String myModelDir;
	String myFFRModelDir;

	class DDCThread : public BackgroundThread {
	public:
		String cmd;
		bool success = false;
		void exec() override;
	};
	DDCThread* myThread = nullptr;
};

}; // namespace Vortex
