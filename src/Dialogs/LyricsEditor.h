#pragma once

#include <Dialogs/Dialog.h>
#include <Core/Widgets.h>
#include <Core/WidgetsLayout.h>
#include <Managers/LyricsMan.h>
#include <Core/Widgets.h>

namespace Vortex {

class DialogLyricsEditor : public EditorDialog
{
public:
	~DialogLyricsEditor();
	DialogLyricsEditor();

	void onChanges(int changes) override;

private:
	void myCreateWidgets();
	void myUpdateWidgets();
	void myRefreshList();

	void onAdd();
	void onRemove();
	void onSetTime();
	void onSelectLine(int index);
	void onSelectLineWrapper() { onSelectLine(mySelectedIndex); }
<<<<<<< HEAD
	void onTextChange(String& val);
=======
	void onTextChange();
>>>>>>> origin/feature-goto-quantize-insert

	WgSelectList* myLyricList;
	WgLineEdit* myTextInput;
	WgLabel* myTimeLabel;

	int mySelectedIndex;
<<<<<<< HEAD
=======
	String myCurrentText;
>>>>>>> origin/feature-goto-quantize-insert
	Vector<LyricLine> myLyrics;
};

}; // namespace Vortex
