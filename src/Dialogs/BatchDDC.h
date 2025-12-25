#pragma once

#include <Dialogs/Dialog.h>
#include <Core/Widgets.h>

namespace Vortex {

class DialogBatchDDC : public EditorDialog
{
public:
	~DialogBatchDDC();
	DialogBatchDDC();

private:
	void myCreateWidgets();
	void myAddFiles();
	void myAddFolder();
	void myRemoveFiles();
	void mySelectOutDir();
	void mySelectModelDir();
	void mySelectFFRModelDir();
	void myGenerate();
	void myUpdateLog(StringRef text);

	WgListbox* myFileList;
	WgTextbox* myOutDirBox;
	WgTextbox* myModelDirBox;
	WgTextbox* myFFRModelDirBox;
	WgTextbox* myLogBox;
	
	Vector<String> myFiles;
	String myOutDir;
	String myModelDir;
	String myFFRModelDir;
};

}; // namespace Vortex
