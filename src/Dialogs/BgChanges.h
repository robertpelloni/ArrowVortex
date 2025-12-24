#pragma once

#include <Dialogs/Dialog.h>
#include <Core/Widgets.h>
#include <Core/WidgetsLayout.h>
#include <Managers/MetadataMan.h>
#include <Core/Widgets.h>

namespace Vortex {

class DialogBgChanges : public EditorDialog
{
public:
	~DialogBgChanges();
	DialogBgChanges();

	void onChanges(int changes) override;

private:
	void myCreateWidgets();
	void myUpdateWidgets();
	void myRefreshList();

	void onAdd();
	void onRemove();
	void onSetBeat();
	void onSelectLine(int index);
	void onSelectLineWrapper() { onSelectLine(mySelectedIndex); }
	void onFileChange();
	void onRateChange();
	void onBrowseFile();

	void onLayerChanged(int layer);

	WgSelectList* myChangeList;
	WgLineEdit* myFile;
	WgSpinner* myRate;
	WgCycleButton* myLayerCycle; // Layer 1, Layer 2, Foreground

	int mySelectedIndex;
	String myCurrentFile;
	double myCurrentRate;
	int myCurrentLayer; // 0=BG1, 1=BG2, 2=FG
	Vector<BgChange> myChanges;
};

}; // namespace Vortex
