#include <Dialogs/BgChanges.h>

#include <Editor/Editor.h>
#include <Editor/View.h>
#include <Core/StringUtils.h>
#include <Managers/MetadataMan.h>
#include <Managers/TempoMan.h>
#include <System/System.h>

namespace Vortex {

DialogBgChanges::~DialogBgChanges()
{
}

DialogBgChanges::DialogBgChanges()
{
	setTitle("BG CHANGES EDITOR");
	mySelectedIndex = -1;
	myCurrentLayer = 0;
	myCreateWidgets();
	myUpdateWidgets();
}

void DialogBgChanges::myCreateWidgets()
{
	// Layer selection
	myLayout.row().col(400);
	myLayerCycle = myLayout.add<WgCycleButton>("Layer");
	myLayerCycle->addItem("Background Layer 1");
	myLayerCycle->addItem("Background Layer 2");
	myLayerCycle->addItem("Foreground");
	myLayerCycle->value.bind(&myCurrentLayer);
	myLayerCycle->onChange.bind(this, &DialogBgChanges::onLayerChanged);

	// List
	myLayout.row().col(400);
	myChangeList = myLayout.add<WgList>();
	myChangeList->setHeight(300);
	myChangeList->onSelect.bind(this, &DialogBgChanges::onSelectLine);

	// Edit Area
	myLayout.row().col(100).col(300);
	auto timeBtn = myLayout.add<WgButton>("Set Beat");
	timeBtn->onPress.bind(this, &DialogBgChanges::onSetBeat);

	myFile = myLayout.add<WgLineEdit>("File");
	static String sFile;
	myFile->text.bind(&sFile);
	myFile->onChange.bind(this, &DialogBgChanges::onFileChange);

	myLayout.row().col(100).col(300);
	auto browseBtn = myLayout.add<WgButton>("Browse...");
	browseBtn->onPress.bind(this, &DialogBgChanges::onBrowseFile);

	myRate = myLayout.add<WgSpinner>("Rate");
	static double sRate = 1.0;
	myRate->value.bind(&sRate);
	myRate->onChange.bind(this, &DialogBgChanges::onRateChange);

	// Controls
	myLayout.row().col(100).col(100);
	auto addBtn = myLayout.add<WgButton>("Add");
	addBtn->onPress.bind(this, &DialogBgChanges::onAdd);

	auto remBtn = myLayout.add<WgButton>("Remove");
	remBtn->onPress.bind(this, &DialogBgChanges::onRemove);
}

void DialogBgChanges::myUpdateWidgets()
{
	if (!gMetadata) return;

	if (myCurrentLayer == 2) myChanges = gMetadata->getFgChanges();
	else myChanges = gMetadata->getBgChanges(myCurrentLayer);

	myRefreshList();
}

void DialogBgChanges::myRefreshList()
{
	myChangeList->clear();
	for (const auto& c : myChanges) {
		String s = Str::fmt("[%1] %2").arg(Str::val(c.startBeat, 3)).arg(c.file);
		myChangeList->addItem(s);
	}
	if (mySelectedIndex >= 0 && mySelectedIndex < myChanges.size()) {
		myChangeList->select(mySelectedIndex);
		myFile->setText(myChanges[mySelectedIndex].file);
		myRate->setValue(myChanges[mySelectedIndex].rate);
	} else {
		myFile->setText("");
		myRate->setValue(1.0);
	}
}

void DialogBgChanges::onLayerChanged(int layer)
{
	mySelectedIndex = -1;
	myUpdateWidgets();
}

void DialogBgChanges::onSelectLine(int index)
{
	mySelectedIndex = index;
	if (index >= 0 && index < myChanges.size()) {
		myFile->setText(myChanges[index].file);
		myRate->setValue(myChanges[index].rate);
	}
}

void DialogBgChanges::onAdd()
{
	double beat = gView->getCursorBeat();
	BgChange c;
	c.startBeat = beat;
	c.rate = 1.0;
	c.file = ""; // Empty initially

	// Add and sort
	myChanges.push_back(c);
	std::sort(myChanges.begin(), myChanges.end(), [](const BgChange& a, const BgChange& b){ return a.startBeat < b.startBeat; });

	// Update backend
	if (myCurrentLayer == 2) gMetadata->setFgChanges(myChanges);
	else gMetadata->setBgChanges(myChanges, myCurrentLayer);

	myUpdateWidgets();
}

void DialogBgChanges::onRemove()
{
	if (mySelectedIndex >= 0 && mySelectedIndex < myChanges.size()) {
		myChanges.erase(myChanges.begin() + mySelectedIndex);
		mySelectedIndex = -1;

		if (myCurrentLayer == 2) gMetadata->setFgChanges(myChanges);
		else gMetadata->setBgChanges(myChanges, myCurrentLayer);

		myUpdateWidgets();
	}
}

void DialogBgChanges::onSetBeat()
{
	if (mySelectedIndex >= 0 && mySelectedIndex < myChanges.size()) {
		myChanges[mySelectedIndex].startBeat = gView->getCursorBeat();
		std::sort(myChanges.begin(), myChanges.end(), [](const BgChange& a, const BgChange& b){ return a.startBeat < b.startBeat; });

		if (myCurrentLayer == 2) gMetadata->setFgChanges(myChanges);
		else gMetadata->setBgChanges(myChanges, myCurrentLayer);

		myUpdateWidgets();
	}
}

void DialogBgChanges::onFileChange(String& val)
{
	if (mySelectedIndex >= 0 && mySelectedIndex < myChanges.size()) {
		myChanges[mySelectedIndex].file = val;
		if (myCurrentLayer == 2) gMetadata->setFgChanges(myChanges);
		else gMetadata->setBgChanges(myChanges, myCurrentLayer);
		// Don't full refresh list to avoid losing focus?
		// But text update in list is needed.
		myChangeList->setItemText(mySelectedIndex, Str::fmt("[%1] %2").arg(Str::val(myChanges[mySelectedIndex].startBeat, 3)).arg(val));
	}
}

void DialogBgChanges::onRateChange(double val)
{
	if (mySelectedIndex >= 0 && mySelectedIndex < myChanges.size()) {
		myChanges[mySelectedIndex].rate = val;
		if (myCurrentLayer == 2) gMetadata->setFgChanges(myChanges);
		else gMetadata->setBgChanges(myChanges, myCurrentLayer);
	}
}

void DialogBgChanges::onBrowseFile()
{
	// Browse logic: Open file dialog for images/videos
	String path = gMetadata->findBackgroundFile(); // Reuse existing helper for now, or implement generic browse
	if (path.len()) {
		if (mySelectedIndex >= 0) {
			myChanges[mySelectedIndex].file = path;
			if (myCurrentLayer == 2) gMetadata->setFgChanges(myChanges);
			else gMetadata->setBgChanges(myChanges, myCurrentLayer);
			myUpdateWidgets();
		}
	} else {
		HudNote("No file selected.");
	}
}

void DialogBgChanges::onChanges(int changes)
{
	if (changes & VCM_SONG_PROPERTIES_CHANGED) {
		myUpdateWidgets();
	}
}

}; // namespace Vortex
