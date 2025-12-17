#include <Dialogs/LyricsEditor.h>

#include <Editor/Editor.h>
#include <Editor/View.h>
#include <Core/StringUtils.h>
#include <Managers/LyricsMan.h>

namespace Vortex {

DialogLyricsEditor::~DialogLyricsEditor()
{
}

DialogLyricsEditor::DialogLyricsEditor()
{
	setTitle("LYRICS EDITOR");
	mySelectedIndex = -1;
	myCreateWidgets();
	myUpdateWidgets();
}

void DialogLyricsEditor::myCreateWidgets()
{
	// List of lyrics
	myLayout.row().col(400);
	myLyricList = myLayout.add<WgSelectList>();
	myLyricList->setHeight(300);
	// WgSelectList doesn't have onSelect? It has onChange.
	// And value binds to int index.
	myLyricList->onChange.bind(this, &DialogLyricsEditor::onSelectLineWrapper);
	myLyricList->value.bind(&mySelectedIndex);

	// Edit Area
	myLayout.row().col(80).col(240).col(80);
	auto timeBtn = myLayout.add<WgButton>("Set Time");
	timeBtn->onPress.bind(this, &DialogLyricsEditor::onSetTime);

	myTextInput = myLayout.add<WgLineEdit>("Text");
	String dummy;
	myTextInput->text.bind(&dummy); // Manual handling
	myTextInput->onChange.bind(this, &DialogLyricsEditor::onTextChange);

	myTimeLabel = myLayout.add<WgLabel>("0.000");

	// Controls
	myLayout.row().col(100).col(100).col(200);
	auto addBtn = myLayout.add<WgButton>("Add Line");
	addBtn->onPress.bind(this, &DialogLyricsEditor::onAdd);

	auto remBtn = myLayout.add<WgButton>("Remove");
	remBtn->onPress.bind(this, &DialogLyricsEditor::onRemove);
}

void DialogLyricsEditor::myUpdateWidgets()
{
	if (gLyrics) {
		myLyrics = gLyrics->getLyrics();
		myRefreshList();
	}
}

void DialogLyricsEditor::myRefreshList()
{
	myLyricList->clear();
	for (const auto& line : myLyrics) {
		String s = Str::fmt("[%1] %2").arg(Str::formatTime(line.time)).arg(line.text);
		myLyricList->addItem(s);
	}
	if (mySelectedIndex >= 0 && mySelectedIndex < myLyrics.size()) {
		myLyricList->select(mySelectedIndex);
		myTimeLabel->setText(Str::formatTime(myLyrics[mySelectedIndex].time));
		myTextInput->setText(myLyrics[mySelectedIndex].text);
	} else {
		myTimeLabel->setText("0.000");
		myTextInput->setText("");
	}
}

void DialogLyricsEditor::onChanges(int changes)
{
	// If external lyrics change, update?
	// For now, assuming this dialog is the main editor.
}

void DialogLyricsEditor::onSelectLine(int index)
{
	mySelectedIndex = index;
	if (index >= 0 && index < myLyrics.size()) {
		myTimeLabel->setText(Str::formatTime(myLyrics[index].time));
		myTextInput->setText(myLyrics[index].text);
	}
}

void DialogLyricsEditor::onAdd()
{
	double time = gView->getCursorTime();
	gLyrics->addLyric(time, "New Line");
	myLyrics = gLyrics->getLyrics(); // Refresh local
	mySelectedIndex = -1; // Deselect or select new?
	// Find index of new line
	for(int i=0; i<myLyrics.size(); ++i) {
		if (myLyrics[i].time == time) {
			mySelectedIndex = i;
			break;
		}
	}
	myRefreshList();
}

void DialogLyricsEditor::onRemove()
{
	if (mySelectedIndex >= 0 && mySelectedIndex < myLyrics.size()) {
		gLyrics->removeLyric(mySelectedIndex);
		myLyrics = gLyrics->getLyrics();
		mySelectedIndex = -1;
		myRefreshList();
	}
}

void DialogLyricsEditor::onSetTime()
{
	if (mySelectedIndex >= 0 && mySelectedIndex < myLyrics.size()) {
		double time = gView->getCursorTime();
		// Update backend
		// Remove and Add to keep sorted? Or modify?
		// LyricsMan api: setLyrics exists.
		myLyrics[mySelectedIndex].time = time;
		// Re-sort
		std::sort(myLyrics.begin(), myLyrics.end(), [](const LyricLine& a, const LyricLine& b){
			return a.time < b.time;
		});
		gLyrics->setLyrics(myLyrics);
		myRefreshList();
	}
}

void DialogLyricsEditor::onTextChange(String& val)
{
	if (mySelectedIndex >= 0 && mySelectedIndex < myLyrics.size()) {
		myLyrics[mySelectedIndex].text = val;
		// We avoid full setLyrics on every keypress for performance if list is huge?
		// But for simple text it's fine.
		gLyrics->setLyrics(myLyrics);
		// Update list item text only?
		// myLyricList->setItemText... if available.
		// For now refresh all.
		// Optimize: don't refresh list if only text changed?
		// Just update the item string.
		// But WgList API might not expose direct update.
		// Let's defer full refresh to focus loss or Enter?
		// No, usually immediate.
		// But refreshing re-builds widgets.
		// Let's just update backend.
	}
}

}; // namespace Vortex
