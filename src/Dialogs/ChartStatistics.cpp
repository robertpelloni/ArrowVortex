#include <Dialogs/ChartStatistics.h>
#include <Core/WidgetsLayout.h>
#include <Managers/ChartMan.h>
#include <Managers/NoteMan.h>
#include <Core/StringUtils.h>
#include <Core/Draw.h>
#include <algorithm>

namespace Vortex {

struct GraphWidget : public GuiWidget
{
	GraphWidget(GuiContext* gui) : GuiWidget(gui) { setHeight(100); }

	Vector<int> counts;
	int maxCount = 0;

	void analyze(const Chart* chart) {
		counts.clear();
		maxCount = 0;
		if (!chart) return;

		int maxRow = 0;
		if (!chart->notes.empty()) maxRow = chart->notes.back().row;
		int numMeasures = (maxRow / 192) + 1;
		if (numMeasures < 1) numMeasures = 1;
		counts.resize(numMeasures, 0);

		for (const auto& n : chart->notes) {
			if (n.type != NOTE_MINE && n.type != NOTE_FAKE) {
				int m = n.row / 192;
				if (m < numMeasures) counts[m]++;
			}
		}

		for(int c : counts) maxCount = std::max(maxCount, c);
	}

	void onDraw() override {
		Draw::fill(rect_, RGBAtoColor32(30, 30, 30, 255));

		if (counts.empty()) return;

		float barW = (float)rect_.w / counts.size();
		float scale = (maxCount > 0) ? ((float)rect_.h / maxCount) : 0;

		for (int i = 0; i < counts.size(); ++i) {
			int h = (int)(counts[i] * scale);
			recti r = {
				rect_.x + (int)(i * barW),
				rect_.y + rect_.h - h,
				(int)barW,
				h
			};
			if (r.w < 1) r.w = 1;
			if (r.w > 1) r.w -= 1; // Gap
			Draw::fill(r, RGBAtoColor32(100, 200, 100, 200));
		}
	}
};

DialogChartStatistics::~DialogChartStatistics() {}

DialogChartStatistics::DialogChartStatistics()
{
	setTitle("Chart Statistics");
	setSize(400, 400);

	myLayout.row().col(380);

	const Chart* chart = gChart->get();
	if (!chart) {
		auto lbl = myLayout.add<WgLabel>();
		lbl->text.set("No chart open.");
		return;
	}

	// Graph
	auto graphLbl = myLayout.add<WgLabel>();
	graphLbl->text.set("Note Density (Notes per Measure):");
	myLayout.row().col(380);
	auto graph = myLayout.add<GraphWidget>();
	graph->analyze(chart);
	myLayout.row().col(380);
	myLayout.add<WgSeperator>();
	myLayout.row().col(380);

	// Stats
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
