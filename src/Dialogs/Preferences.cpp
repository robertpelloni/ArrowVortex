#include <Dialogs/Preferences.h>
#include <Editor/Editor.h>
#include <Editor/View.h>
#include <Core/Utils.h>
#include <Core/StringUtils.h>

namespace Vortex {

DialogPreferences::DialogPreferences()
{
	setTitle("OPTIONS");
	myActiveTab = TAB_EDITOR;
	myRefreshLayout();
}

void DialogPreferences::mySetTab(Tab tab)
{
	if (myActiveTab != tab) {
		myActiveTab = tab;
		myRefreshLayout();
	}
}

void DialogPreferences::myRefreshLayout()
{
	myCreateWidgets();
	myUpdateWidgets();
}

void DialogPreferences::myCreateWidgets()
{
	// Only create once.
	static bool created = false;
	if (created) return;
	created = true;

	myLayout.row().col(400); // Approximate width

	// --- EDITOR SETTINGS ---
	myLayout.add<WgLabel>()->text.set("--- Editor Settings ---");

	auto cbMiddle = myLayout.add<WgCheckbox>("Middle Mouse Button to insert beat");
	cbMiddle->value.bind(&myMiddleMouseInsertBeat);
	cbMiddle->onChange.bind(this, &DialogPreferences::onMiddleMouseChanged);
	cbMiddle->setTooltip("Uncheck: Middle Mouse Button to autoscroll");

	auto cbScroll = myLayout.add<WgCheckbox>("Enable scrolling cursor effect during playback");
	cbScroll->value.bind(&myScrollCursorEffect);
	cbScroll->onChange.bind(this, &DialogPreferences::onScrollCursorChanged);
	cbScroll->setTooltip("Uncheck: Cursor stays still during playback");

	auto cbInsert = myLayout.add<WgCheckbox>("Inserting the same arrow will delete it");
	cbInsert->value.bind(&myInsertSameDeletes);
	cbInsert->onChange.bind(this, &DialogPreferences::onInsertSameChanged);
	cbInsert->setTooltip("Uncheck: Inserting the same arrow won't do anything");

	auto cbEdit = myLayout.add<WgCheckbox>("Edit on one layer at a time");
	cbEdit->value.bind(&myEditOneLayer);
	cbEdit->onChange.bind(this, &DialogPreferences::onEditOneLayerChanged);
	cbEdit->setTooltip("Uncheck: Edit on all layers at the same time");

	auto cbPaste = myLayout.add<WgCheckbox>("Pasting will overwrite old items with the new");
	cbPaste->value.bind(&myPasteOverwrites);
	cbPaste->onChange.bind(this, &DialogPreferences::onPasteOverwritesChanged);
	cbPaste->setTooltip("Uncheck: Pasting will merge new and old items");

	auto cbSelPaste = myLayout.add<WgCheckbox>("Select newly pasted items");
	cbSelPaste->value.bind(&mySelectPasted);
	cbSelPaste->onChange.bind(this, &DialogPreferences::onSelectPastedChanged);
	cbSelPaste->setTooltip("Uncheck: Don't select newly pasted items");

	auto cbNudge = myLayout.add<WgCheckbox>("Nudge beats based on zoom level");
	cbNudge->value.bind(&myNudgeBasedOnZoom);
	cbNudge->onChange.bind(this, &DialogPreferences::onNudgeChanged);
	cbNudge->setTooltip("Uncheck: Nudge based on absolute time");

	auto cbAssist = myLayout.add<WgCheckbox>("Assist Tick on beats and sub-beats");
	cbAssist->value.bind(&myAssistTickBeats);
	cbAssist->onChange.bind(this, &DialogPreferences::onAssistTickChanged);
	cbAssist->setTooltip("Uncheck: Assist Tick on beats only");

	auto cbDedup = myLayout.add<WgCheckbox>("Remove duplicate BPMs on save");
	cbDedup->value.bind(&myRemoveDuplicateBPMs);
	cbDedup->onChange.bind(this, &DialogPreferences::onDedupChanged);
	cbDedup->setTooltip("Uncheck: Save BPM of every beat");

	auto cbBackup = myLayout.add<WgCheckbox>("Backup all saves");
	cbBackup->value.bind(&myBackupSaves);
	cbBackup->onChange.bind(this, &DialogPreferences::onBackupSavesChanged);
	cbBackup->setTooltip("Don't backup saves");

	auto cbFPS = myLayout.add<WgCheckbox>("Don't Show FPS");
	cbFPS->value.bind(&myDontShowFPS);
	cbFPS->onChange.bind(this, &DialogPreferences::onDontShowFPSChanged);
	cbFPS->setTooltip("Uncheck: Show FPS");

	// Python Path
	myLayout.row().col(100).col(300);
	myLayout.add<WgLabel>()->text.set("Python Path:");
	auto tbPython = myLayout.add<WgTextbox>();
	tbPython->text.bind(&myPythonPath);
	tbPython->onChange.bind(this, &DialogPreferences::onPythonPathChanged);

	// Volumes
	myLayout.row().col(190).col(20).col(190);
	myLayout.add<WgLabel>()->text.set("Beat Assist Vol.");
	myLayout.add<WgLabel>(); // Spacer
	myLayout.add<WgLabel>()->text.set("Note Assist Vol.");

	myLayout.row().col(190).col(20).col(190);
	auto slBeatVol = myLayout.add<WgSlider>();
	slBeatVol->setRange(0.0, 1.0);
	slBeatVol->value.bind(&myBeatAssistVol);
	slBeatVol->onChange.bind(this, &DialogPreferences::onBeatVolChanged);

	auto slNoteVol = myLayout.add<WgSlider>();
	slNoteVol->setRange(0.0, 1.0);
	slNoteVol->value.bind(&myNoteAssistVol);
	slNoteVol->onChange.bind(this, &DialogPreferences::onNoteVolChanged);

	// --- PRACTICE MODE ---
	myLayout.row().col(400);
	myLayout.add<WgSeperator>();
	myLayout.add<WgLabel>()->text.set("--- Practice Mode ---");

	auto cbPractice = myLayout.add<WgCheckbox>("Enable Practice Mode");
	cbPractice->value.bind(&myEnablePracticeMode);
	cbPractice->onChange.bind(this, &DialogPreferences::onPracticeEnabledChanged);
	cbPractice->setTooltip("Uncheck: AutoPlay (Inputs handled by Editor logic)");

	// Timing windows (Milliseconds)
	// UI layout: Label | Input (Spinner) | Unit

	auto addRow = [&](const char* label, double* val, void (DialogPreferences::*cb)()) {
		myLayout.row().col(150).col(100).col(50);
		myLayout.add<WgLabel>()->text.set(label);
		auto spin = myLayout.add<WgSpinner>();
		spin->setRange(0.0, 500.0);
		spin->setStep(1.0);
		spin->value.bind(val);
		spin->onChange.bind(this, cb);
		myLayout.add<WgLabel>()->text.set("ms");
	};

	addRow("Marvelous: +/-", &myWindowMarvelous, &DialogPreferences::onWindowMarvelousChanged);
	addRow("Perfect: +/-", &myWindowPerfect, &DialogPreferences::onWindowPerfectChanged);
	addRow("Great: +/-", &myWindowGreat, &DialogPreferences::onWindowGreatChanged);
	addRow("Good: +/-", &myWindowGood, &DialogPreferences::onWindowGoodChanged);
	addRow("Boo: +/-", &myWindowBoo, &DialogPreferences::onWindowBooChanged);
	addRow("Freeze OK: +/-", &myWindowFreeze, &DialogPreferences::onWindowFreezeChanged);
	addRow("Mine: +/-", &myWindowMine, &DialogPreferences::onWindowMineChanged);
}

void DialogPreferences::myUpdateWidgets()
{
	// Sync local variables from Editor
	myNudgeBasedOnZoom = gEditor->getNudgeBasedOnZoom();
	myAssistTickBeats = gEditor->getAssistTickBeats();
	myRemoveDuplicateBPMs = gEditor->getRemoveDuplicateBPMs();
	myBeatAssistVol = gEditor->getBeatAssistVol();
	myNoteAssistVol = gEditor->getNoteAssistVol();

	myMiddleMouseInsertBeat = gEditor->getMiddleMouseInsertBeat();
	myScrollCursorEffect = gEditor->getScrollCursorEffect();
	myInsertSameDeletes = gEditor->getInsertSameDeletes();
	myEditOneLayer = gEditor->getEditOneLayer();
	myPasteOverwrites = gEditor->getPasteOverwrites();
	mySelectPasted = gEditor->getSelectPasted();
	myBackupSaves = gEditor->getBackupSaves();
	myDontShowFPS = gEditor->getDontShowFPS();
	myPythonPath = gEditor->getPythonPath();

	myEnablePracticeMode = gEditor->isPracticeMode();
	myPracticeSetup = gEditor->getPracticeSetup();

	// Convert seconds to ms for UI
	myWindowMarvelous = myPracticeSetup.windowMarvelous * 1000.0;
	myWindowPerfect = myPracticeSetup.windowPerfect * 1000.0;
	myWindowGreat = myPracticeSetup.windowGreat * 1000.0;
	myWindowGood = myPracticeSetup.windowGood * 1000.0;
	myWindowBoo = myPracticeSetup.windowBoo * 1000.0;
	myWindowFreeze = myPracticeSetup.windowFreeze * 1000.0;
	myWindowMine = myPracticeSetup.windowMine * 1000.0;
}

void DialogPreferences::onChanges(int changes)
{
	// If needed
}

// Callbacks - Use bound member variables (which are updated by widget before callback)
void DialogPreferences::onNudgeChanged() { gEditor->setNudgeBasedOnZoom(myNudgeBasedOnZoom); }
void DialogPreferences::onAssistTickChanged() { gEditor->setAssistTickBeats(myAssistTickBeats); }
void DialogPreferences::onDedupChanged() { gEditor->setRemoveDuplicateBPMs(myRemoveDuplicateBPMs); }
void DialogPreferences::onBeatVolChanged() { gEditor->setBeatAssistVol(myBeatAssistVol); }
void DialogPreferences::onNoteVolChanged() { gEditor->setNoteAssistVol(myNoteAssistVol); }

void DialogPreferences::onMiddleMouseChanged() { gEditor->setMiddleMouseInsertBeat(myMiddleMouseInsertBeat); }
void DialogPreferences::onScrollCursorChanged() { gEditor->setScrollCursorEffect(myScrollCursorEffect); }
void DialogPreferences::onInsertSameChanged() { gEditor->setInsertSameDeletes(myInsertSameDeletes); }
void DialogPreferences::onEditOneLayerChanged() { gEditor->setEditOneLayer(myEditOneLayer); }
void DialogPreferences::onPasteOverwritesChanged() { gEditor->setPasteOverwrites(myPasteOverwrites); }
void DialogPreferences::onSelectPastedChanged() { gEditor->setSelectPasted(mySelectPasted); }
void DialogPreferences::onBackupSavesChanged() { gEditor->setBackupSaves(myBackupSaves); }
void DialogPreferences::onDontShowFPSChanged() { gEditor->setDontShowFPS(myDontShowFPS); }
void DialogPreferences::onPythonPathChanged() { gEditor->setPythonPath(myPythonPath); }

void DialogPreferences::onPracticeEnabledChanged() { gEditor->setPracticeMode(myEnablePracticeMode); }

// Helper to update struct
#define UPDATE_SETUP(field, memberVar) \
	myPracticeSetup = gEditor->getPracticeSetup(); \
	myPracticeSetup.field = (float)(memberVar / 1000.0); \
	gEditor->setPracticeSetup(myPracticeSetup);

void DialogPreferences::onWindowMarvelousChanged() { UPDATE_SETUP(windowMarvelous, myWindowMarvelous); }
void DialogPreferences::onWindowPerfectChanged() { UPDATE_SETUP(windowPerfect, myWindowPerfect); }
void DialogPreferences::onWindowGreatChanged() { UPDATE_SETUP(windowGreat, myWindowGreat); }
void DialogPreferences::onWindowGoodChanged() { UPDATE_SETUP(windowGood, myWindowGood); }
void DialogPreferences::onWindowBooChanged() { UPDATE_SETUP(windowBoo, myWindowBoo); }
void DialogPreferences::onWindowFreezeChanged() { UPDATE_SETUP(windowFreeze, myWindowFreeze); }
void DialogPreferences::onWindowMineChanged() { UPDATE_SETUP(windowMine, myWindowMine); }

}; // namespace Vortex
