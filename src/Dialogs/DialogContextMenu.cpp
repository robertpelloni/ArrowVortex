#include <Dialogs/DialogContextMenu.h>
#include <Core/WidgetsLayout.h>
#include <Editor/Action.h>
#include <Editor/Editor.h>

namespace Vortex {

DialogContextMenu::~DialogContextMenu() {}

DialogContextMenu::DialogContextMenu()
{
	setTitle("Menu");
	setSize(200, 320);

	myLayout.row().col(190);

	myLayout.add<WgButton>("Cut")->onPress.bind(this, &DialogContextMenu::onCut);
	myLayout.row().col(190);
	myLayout.add<WgButton>("Copy")->onPress.bind(this, &DialogContextMenu::onCopy);
	myLayout.row().col(190);
	myLayout.add<WgButton>("Paste")->onPress.bind(this, &DialogContextMenu::onPaste);
	myLayout.row().col(190);
	myLayout.add<WgButton>("Delete")->onPress.bind(this, &DialogContextMenu::onDelete);

	myLayout.add<WgSeperator>();
	myLayout.row().col(190);
	myLayout.add<WgButton>("Insert Measure")->onPress.bind(this, &DialogContextMenu::onInsertMeasure);
	myLayout.row().col(190);
	myLayout.add<WgButton>("Delete Measure")->onPress.bind(this, &DialogContextMenu::onDeleteMeasure);

	myLayout.add<WgSeperator>();
	myLayout.row().col(190);
	myLayout.add<WgButton>("Shuffle")->onPress.bind(this, &DialogContextMenu::onShuffle);
	myLayout.row().col(190);
	myLayout.add<WgButton>("Turn Left")->onPress.bind(this, &DialogContextMenu::onTurnLeft);
	myLayout.row().col(190);
	myLayout.add<WgButton>("Turn Right")->onPress.bind(this, &DialogContextMenu::onTurnRight);

	myLayout.add<WgSeperator>();
	myLayout.row().col(190);
	myLayout.add<WgButton>("Quantize Selection")->onPress.bind(this, &DialogContextMenu::onQuantize);
	myLayout.row().col(190);
	myLayout.add<WgButton>("Warp Grid")->onPress.bind(this, &DialogContextMenu::onWarp);
}

void DialogContextMenu::closeDialog() { gSystem->getEvents().addKeyPress(Key::ESCAPE, 0, false); }

void DialogContextMenu::onCut() { Action::perform(Action::EDIT_CUT); closeDialog(); }
void DialogContextMenu::onCopy() { Action::perform(Action::EDIT_COPY); closeDialog(); }
void DialogContextMenu::onPaste() { Action::perform(Action::EDIT_PASTE); closeDialog(); }
void DialogContextMenu::onDelete() { Action::perform(Action::EDIT_DELETE); closeDialog(); }
void DialogContextMenu::onInsertMeasure() { Action::perform(Action::INSERT_MEASURE); closeDialog(); }
void DialogContextMenu::onDeleteMeasure() { Action::perform(Action::DELETE_MEASURE); closeDialog(); }
void DialogContextMenu::onShuffle() { Action::perform(Action::SHUFFLE_NOTES); closeDialog(); }
void DialogContextMenu::onTurnLeft() { Action::perform(Action::TURN_NOTES_LEFT); closeDialog(); }
void DialogContextMenu::onTurnRight() { Action::perform(Action::TURN_NOTES_RIGHT); closeDialog(); }
void DialogContextMenu::onQuantize() { Action::perform(Action::QUANTIZE_TO_AUDIO); closeDialog(); }
void DialogContextMenu::onWarp() { Action::perform(Action::WARP_GRID_TO_AUDIO); closeDialog(); }

}
