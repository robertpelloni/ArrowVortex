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
	void myRemoveFiles();
	void mySelectOutDir();
	void mySelectModelDir();
	void myGenerate();
	void myUpdateLog(StringRef text);

	WgListbox* myFileList;
	WgTextbox* myOutDirBox;
	WgTextbox* myModelDirBox;
	WgTextbox* myLogBox;
	
	Vector<String> myFiles;
	String myOutDir;
	String myModelDir;
};

}; // namespace Vortex
