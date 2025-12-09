#include <Dialogs/GoTo.h>

#include <Editor/Editor.h>
#include <Editor/View.h>
#include <Managers/TempoMan.h>
#include <Core/StringUtils.h>
#include <System/System.h>

namespace Vortex {

DialogGoTo::~DialogGoTo()
{
}

DialogGoTo::DialogGoTo()
{
	setTitle("GO TO");
	myCreateWidgets();
	myUpdateWidgets();
}

void DialogGoTo::myCreateWidgets()
{
	myLayout.row().col(200);
	myInput = myLayout.add<WgLineEdit>("Time/Row/Beat/Measure");
	myInput->text.bind(&myValue);
	myInput->setTooltip("Enter time (mm:ss.ms), beat (b123), measure (m10), or row number.");
	myInput->onEnter.bind(this, &DialogGoTo::onGo);

	myLayout.row().col(200);
	auto btn = myLayout.add<WgButton>("Go");
	btn->onPress.bind(this, &DialogGoTo::onGo);
}

void DialogGoTo::myUpdateWidgets()
{
}

void DialogGoTo::onChanges(int changes)
{
}

void DialogGoTo::onGo()
{
	if (myValue.empty()) return;

	double targetTime = -1.0;
	int targetRow = -1;

	String s = myValue;
	Str::toLower(s);

	// Check prefixes
	if (Str::startsWith(s, "b")) {
		// Beat
		String val = s.subStr(1);
		double beat = Str::toDouble(val);
		targetTime = gTempo->beatToTime(beat);
	}
	else if (Str::startsWith(s, "m")) {
		// Measure
		String val = s.subStr(1);
		double measure = Str::toDouble(val);
		double beat = measure * 4.0; // Approximation for 4/4 time
		targetTime = gTempo->beatToTime(beat);
	}
	else if (Str::contains(s, ":")) {
		// Time mm:ss.ms
		targetTime = Str::readTime(s);
	}
	else if (Str::isNum(s)) {
		// Row or Time
		if (Str::contains(s, ".")) {
			targetTime = Str::toDouble(s);
		} else {
			targetRow = Str::toInt(s);
		}
	}

	if (targetRow != -1) {
		gView->setCursorRow(targetRow);
		close();
	} else if (targetTime >= 0.0) {
		gView->setCursorTime(targetTime);
		close();
	} else {
		// Invalid input
		HudWarning("Invalid format.");
	}
}

}; // namespace Vortex
