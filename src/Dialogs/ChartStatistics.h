#pragma once

#include <Dialogs/Dialog.h>
#include <Core/Widgets.h>
#include <Core/WidgetsLayout.h>
#include <Simfile/Chart.h>

namespace Vortex {

class DialogChartStatistics : public EditorDialog
{
public:
	~DialogChartStatistics();
	DialogChartStatistics();

	void onChanges(int changes) override;

private:
	struct GraphWidget;

	void myCreateWidgets();
	void myUpdateWidgets();

	WgLabel* myStepCount;
	WgLabel* myJumpCount;
	WgLabel* myHandCount;
	WgLabel* myMineCount;
	WgLabel* myHoldCount;
	WgLabel* myRollCount;

	GraphWidget* myGraph;
	Vector<float> myDensityData;
	float myMaxDensity;
};

}; // namespace Vortex
