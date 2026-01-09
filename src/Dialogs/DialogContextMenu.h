#pragma once
#include <Dialogs/Dialog.h>

namespace Vortex {

class DialogContextMenu : public EditorDialog
{
public:
	DialogContextMenu();
	~DialogContextMenu();
<<<<<<< HEAD
=======

private:
	void onCut();
	void onCopy();
	void onPaste();
	void onDelete();
	void onInsertMeasure();
	void onDeleteMeasure();
	void onShuffle();
	void onTurnLeft();
	void onTurnRight();
	void onQuantize();
	void onWarp();

	void closeDialog();
>>>>>>> origin/feature-goto-quantize-insert
};

}
