#pragma once

#include <Dialogs/Dialog.h>
#include <Editor/Editor.h>

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
		TAB_PRACTICE,
		TAB_WAVEFORM
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
	String myPythonPath;

	// Practice Mode Settings
	bool myEnablePracticeMode;
	Editor::PracticeSetup myPracticeSetup;

	// Waveform Settings
	int myWaveformColorMode;
	double mySpectrogramGain;
	double myRGBLow;
	double myRGBHigh;
	bool myShowOnsets;
	double myOnsetThreshold;
	int myAntiAliasing;

	// Intermediate doubles for UI binding (milliseconds)
	double myWindowMarvelous;
	double myWindowPerfect;
	double myWindowGreat;
	double myWindowGood;
	double myWindowBoo;
	double myWindowFreeze;
	double myWindowMine;

	// Callback wrappers (All void because Wg*::onChange is void)
	void onNudgeChanged();
	void onAssistTickChanged();
	void onDedupChanged();

	void onMiddleMouseChanged();
	void onScrollCursorChanged();
	void onInsertSameChanged();
	void onEditOneLayerChanged();
	void onPasteOverwritesChanged();
	void onSelectPastedChanged();
	void onBackupSavesChanged();
	void onDontShowFPSChanged();
	void onPythonPathChanged();

	void onPracticeEnabledChanged();

	void onBeatVolChanged(); // Changed to void, reads myBeatAssistVol
	void onNoteVolChanged();

	void onWindowMarvelousChanged();
	void onWindowPerfectChanged();
	void onWindowGreatChanged();
	void onWindowGoodChanged();
	void onWindowBooChanged();
	void onWindowFreezeChanged();
	void onWindowMineChanged();

	void onWaveformColorModeChanged();
	void onSpectrogramGainChanged();
	void onRGBCrossoverChanged();
	void onShowOnsetsChanged();
	void onOnsetThresholdChanged();
	void onAntiAliasingChanged();
};

}; // namespace Vortex
