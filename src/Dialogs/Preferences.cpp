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
}

void DialogPreferences::myUpdateWidgets()
{
	// Sync local variables from Editor
	myNudgeBasedOnZoom = gEditor->getNudgeBasedOnZoom();
	myAssistTickBeats = gEditor->getAssistTickBeats();
	myRemoveDuplicateBPMs = gEditor->getRemoveDuplicateBPMs();
	myBeatAssistVol = gEditor->getBeatAssistVol();
	myNoteAssistVol = gEditor->getNoteAssistVol();
}

void DialogPreferences::onChanges(int changes)
{
	// If needed
}

// Callbacks
void DialogPreferences::onNudgeChanged() { gEditor->setNudgeBasedOnZoom(myNudgeBasedOnZoom); }
void DialogPreferences::onAssistTickChanged() { gEditor->setAssistTickBeats(myAssistTickBeats); }
void DialogPreferences::onDedupChanged() { gEditor->setRemoveDuplicateBPMs(myRemoveDuplicateBPMs); }
void DialogPreferences::onBeatVolChanged() { gEditor->setBeatAssistVol(myBeatAssistVol); }
void DialogPreferences::onNoteVolChanged() { gEditor->setNoteAssistVol(myNoteAssistVol); }

}; // namespace Vortex
