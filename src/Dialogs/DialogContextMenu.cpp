#include <Dialogs/DialogContextMenu.h>
#include <Core/WidgetsLayout.h>
#include <Editor/Action.h>
#include <Editor/Editor.h>

namespace Vortex {

DialogContextMenu::~DialogContextMenu() {}

DialogContextMenu::DialogContextMenu()
{
	setTitle("Menu");
	setSize(200, 300);

	myLayout.row().col(190);

	auto addButton = [&](const char* label, Action::Type act) {
		WgButton* b = myLayout.add<WgButton>();
		b->text.set(label);
		b->onPress.bind([act](){
			Action::perform(act);
			// Hack to close dialog: Simulate Escape
			gSystem->getEvents().addKeyPress(Key::ESCAPE, 0, false);
		});
		myLayout.row().col(190);
	};

	addButton("Cut", Action::EDIT_CUT);
	addButton("Copy", Action::EDIT_COPY);
	addButton("Paste", Action::EDIT_PASTE);
	addButton("Delete", Action::EDIT_DELETE);
	myLayout.add<WgSeperator>();
	myLayout.row().col(190);
	addButton("Insert Measure", Action::INSERT_MEASURE);
	addButton("Delete Measure", Action::DELETE_MEASURE);
	myLayout.add<WgSeperator>();
	myLayout.row().col(190);
	addButton("Shuffle", Action::SHUFFLE_NOTES);
	addButton("Turn Left", Action::TURN_NOTES_LEFT);
	addButton("Turn Right", Action::TURN_NOTES_RIGHT);
	myLayout.add<WgSeperator>();
	myLayout.row().col(190);
	addButton("Quantize Selection", Action::QUANTIZE_TO_AUDIO);
	addButton("Warp Grid", Action::WARP_GRID_TO_AUDIO);
}

}
