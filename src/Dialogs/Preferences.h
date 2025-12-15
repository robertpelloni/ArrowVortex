#pragma once

#include <Dialogs/Dialog.h>

namespace Vortex {

class DialogPreferences : public EditorDialog
{
public:
	DialogPreferences();

	void onChanges(int changes) override;

private:
	void myCreateWidgets();
	void myUpdateWidgets();
	void myRefreshLayout();

	// Tab switching
	enum Tab {
		TAB_EDITOR,
		TAB_PRACTICE
	};
	Tab myActiveTab;
	void mySetTab(Tab tab);

	// Editor Settings (Member copies for binding)
	bool myMiddleMouseInsertBeat;
	bool myScrollCursorEffect;
	bool myInsertSameDeletes;
	bool myEditOneLayer;
	bool myPasteOverwrites;
	bool mySelectPasted;
	bool myNudgeBasedOnZoom;
	bool myAssistTickBeats;
	bool myRemoveDuplicateBPMs;
	float myBeatAssistVol;
	float myNoteAssistVol;
	bool myBackupSaves;
	bool myDontShowFPS;

	// Practice Mode Settings
	bool myEnablePracticeMode;

	// Callback wrappers
	void onNudgeChanged();
	void onAssistTickChanged();
	void onDedupChanged();
	void onBeatVolChanged();
	void onNoteVolChanged();
};

}; // namespace Vortex
