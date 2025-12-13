#include <Dialogs/ChartStatistics.h>

#include <Editor/Editor.h>
#include <Managers/ChartMan.h>
#include <Core/StringUtils.h>
#include <Core/Draw.h>

namespace Vortex {

struct DialogChartStatistics::GraphWidget : public GuiWidget
{
	GraphWidget(GuiContext* gui) : GuiWidget(gui) {
		width_ = 400;
		height_ = 100;
	}

	const Vector<float>* data = nullptr;
	float maxVal = 1.0f;

	void onDraw() override {
		// Draw background
		Draw::fill(rect_, Color32(20, 20, 20, 255));

		if (!data || data->empty()) return;

		int count = data->size();
		float barWidth = (float)width_ / count;

		for(int i=0; i<count; ++i) {
			float val = (*data)[i];
			float h = (val / maxVal) * height_;
			int x = rect_.x + (int)(i * barWidth);
			int y = rect_.y + height_ - (int)h;
			int w = (int)barWidth + 1; // Overlap slightly

			Draw::fill({x, y, w, (int)h}, Color32(100, 200, 100, 255));
		}
	}
};

DialogChartStatistics::~DialogChartStatistics()
{
}

DialogChartStatistics::DialogChartStatistics()
{
	setTitle("CHART STATISTICS");
	myCreateWidgets();
	myUpdateWidgets();
}

void DialogChartStatistics::myCreateWidgets()
{
	myLayout.row().col(200).col(200);
	myLayout.add<WgLabel>("Step Count:");
	myStepCount = myLayout.add<WgLabel>("");

	myLayout.row().col(200).col(200);
	myLayout.add<WgLabel>("Jumps:");
	myJumpCount = myLayout.add<WgLabel>("");

	myLayout.row().col(200).col(200);
	myLayout.add<WgLabel>("Hands:");
	myHandCount = myLayout.add<WgLabel>("");

	myLayout.row().col(200).col(200);
	myLayout.add<WgLabel>("Mines:");
	myMineCount = myLayout.add<WgLabel>("");

	myLayout.row().col(200).col(200);
	myLayout.add<WgLabel>("Holds:");
	myHoldCount = myLayout.add<WgLabel>("");

	myLayout.row().col(200).col(200);
	myLayout.add<WgLabel>("Rolls:");
	myRollCount = myLayout.add<WgLabel>("");

	myLayout.add<WgSeperator>();
	myLayout.row().col(400);
	myLayout.add<WgLabel>("Note Density Graph (Notes per measure)");
	myGraph = myLayout.add<GraphWidget>();
	myGraph->data = &myDensityData;
}

void DialogChartStatistics::myUpdateWidgets()
{
	const Chart* chart = gChart->get();
	if (!chart) return;

	int steps = 0, jumps = 0, hands = 0, mines = 0, holds = 0, rolls = 0;

	// Basic counts
	// Need to iterate notes row by row for jumps/hands
	// Notes are sorted.
	if (!chart->notes.empty()) {
		auto it = chart->notes.begin();
		while(it != chart->notes.end()) {
			int row = it->row;
			int notesOnRow = 0;
			bool hasMine = false;

			while(it != chart->notes.end() && it->row == row) {
				if (it->type == NoteType::NOTE_MINE) {
					mines++;
					hasMine = true;
				} else {
					if (it->type == NoteType::NOTE_HOLD_HEAD) holds++;
					if (it->type == NoteType::NOTE_ROLL_HEAD) rolls++;
					if (it->type == NoteType::NOTE_STEP_OR_HOLD || it->type == NoteType::NOTE_HOLD_HEAD || it->type == NoteType::NOTE_ROLL_HEAD || it->type == NoteType::NOTE_LIFT || it->type == NoteType::NOTE_FAKE) {
						notesOnRow++;
						steps++;
					}
				}
				it++;
			}

			if (notesOnRow == 2) jumps++;
			if (notesOnRow >= 3) hands++;
		}
	}

	myStepCount->setText(Str::val(steps));
	myJumpCount->setText(Str::val(jumps));
	myHandCount->setText(Str::val(hands));
	myMineCount->setText(Str::val(mines));
	myHoldCount->setText(Str::val(holds));
	myRollCount->setText(Str::val(rolls));

	// Density Graph
	// Bin by measure (192 rows)
	myDensityData.clear();
	if (!chart->notes.empty()) {
		int lastRow = (chart->notes.end() - 1)->row;
		int numMeasures = (lastRow / 192) + 1;
		myDensityData.resize(numMeasures, 0.0f);

		for(const auto& n : chart->notes) {
			if (n.type != NoteType::NOTE_MINE) {
				int measure = n.row / 192;
				if (measure < numMeasures) myDensityData[measure] += 1.0f;
			}
		}

		myMaxDensity = 0.0f;
		for(float v : myDensityData) if(v > myMaxDensity) myMaxDensity = v;
		if (myMaxDensity < 1.0f) myMaxDensity = 1.0f;

		myGraph->maxVal = myMaxDensity;
	}
}

void DialogChartStatistics::onChanges(int changes)
{
	if (changes & VCM_CHART_CHANGED) {
		myUpdateWidgets();
	}
}

}; // namespace Vortex
