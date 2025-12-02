#include <Dialogs/ChartStatistics.h>
#include <Core/WidgetsLayout.h>
#include <Managers/ChartMan.h>
#include <Managers/NoteMan.h>
#include <Core/StringUtils.h>

namespace Vortex {

DialogChartStatistics::~DialogChartStatistics() {}

DialogChartStatistics::DialogChartStatistics()
{
	setTitle("Chart Statistics");
	setSize(400, 300);

	myLayout.row().col(380);

	const Chart* chart = gChart->get();
	if (!chart) {
		auto lbl = myLayout.add<WgLabel>();
		lbl->text.set("No chart open.");
		return;
	}

	int steps = 0;
	int jumps = 0;
	int hands = 0;
	int mines = 0;
	int holds = 0;
	int rolls = 0;

	int curRow = -1;
	int notesInRow = 0;

	for (const auto& n : chart->notes) {
		if (n.type == NOTE_MINE) {
			mines++;
			continue;
		}

		if ((int)n.row != curRow) {
			if (notesInRow == 1) steps++;
			else if (notesInRow == 2) jumps++;
			else if (notesInRow >= 3) hands++;

			curRow = n.row;
			notesInRow = 0;
		}

		if (n.type == NOTE_STEP_OR_HOLD || n.type == NOTE_ROLL || n.type == NOTE_LIFT) {
			notesInRow++;
			if (n.type == NOTE_STEP_OR_HOLD && n.endrow > n.row) holds++;
			if (n.type == NOTE_ROLL) rolls++;
		}
	}

	if (notesInRow == 1) steps++;
	else if (notesInRow == 2) jumps++;
	else if (notesInRow >= 3) hands++;

	auto addLbl = [&](String text) {
		auto lbl = myLayout.add<WgLabel>();
		lbl->text.set(text);
		myLayout.row().col(380);
	};

	addLbl(Str::fmt("Steps: %1").arg(steps));
	addLbl(Str::fmt("Jumps: %1").arg(jumps));
	addLbl(Str::fmt("Hands: %1").arg(hands));
	addLbl(Str::fmt("Mines: %1").arg(mines));
	addLbl(Str::fmt("Holds: %1").arg(holds));
	addLbl(Str::fmt("Rolls: %1").arg(rolls));
}

}
