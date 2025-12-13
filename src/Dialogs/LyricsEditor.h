#pragma once

#include <Dialogs/Dialog.h>
#include <Core/Widgets.h>
#include <Core/WidgetsLayout.h>
#include <Managers/LyricsMan.h>

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
	void onTextChange(String& val);

	WgList* myLyricList;
	WgLineEdit* myTextInput;
	WgLabel* myTimeLabel;

	int mySelectedIndex;
	Vector<LyricLine> myLyrics;
};

}; // namespace Vortex
