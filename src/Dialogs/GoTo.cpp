#include <Dialogs/GoTo.h>

#include <Editor/Editor.h>
#include <Editor/View.h>
#include <Managers/TempoMan.h>
#include <Core/StringUtils.h>
#include <System/System.h>

#include <Simfile/Tempo.h>


namespace Vortex {

DialogGoTo::~DialogGoTo()
{
}
-
DialogGoTo::DialogGoTo()
{
	setTitle("GO TO");
	myCreateWidgets();
	myUpdateWidgets();
}

void DialogGoTo::myCreateWidgets()
{
	myLayout.row().col(200);
	myInput = myLayout.add<WgLineEdit>("Time/Row/Beat");
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
		double measure_f = Str::toDouble(val);
		int measure = (int)floor(measure_f);
		double measure_frac = measure_f - measure;


		const TimingData &timingData = gTempo->getTimingData();
		if (timingData.sigs.empty()) {
			// no time signatures, assume 4/4
			targetTime = gTempo->beatToTime(measure_f * 4.0);
		} else {
			double beat = 0;
			int last_row = 0;
			int last_measure = 0;
			// find the beat for the start of the measure
			for (const auto &sig : timingData.sigs) {
				int next_measure = last_measure + (sig.row - last_row) / sig.rowsPerMeasure;
				if (next_measure > measure) {
					beat = (double)last_row / ROWS_PER_BEAT + (double)(measure - last_measure) * sig.rowsPerMeasure / ROWS_PER_BEAT;
					goto found;
				}

				last_row = sig.row;
				last_measure = next_measure;
			}
			// after all sigs
			beat = (double)last_row / ROWS_PER_BEAT + (double)(measure - last_measure) * timingData.sigs.back().rowsPerMeasure / ROWS_PER_BEAT;

		found:
			// add fractional part
			// how do we find rowspermeasure for the fractional part?
			// i guess it's just the sig that we landed on.
			last_row = 0;
			last_measure = 0;
			for (const auto &sig : timingData.sigs) {
				int next_measure = last_measure + (sig.row - last_row) / sig.rowsPerMeasure;
				if (next_measure > measure) {
					beat += measure_frac * sig.rowsPerMeasure / ROWS_PER_BEAT;
					goto found_frac;
				}

				last_row = sig.row;
				last_measure = next_measure;
			}
			beat += measure_frac * timingData.sigs.back().rowsPerMeasure / ROWS_PER_BEAT;
			
		found_frac:
			targetTime = gTempo->beatToTime(beat);
		}
	}
	else if (Str::contains(s, ":")) {
		// Time mm:ss.ms
		targetTime = Str::readTime(s);
	}
	else {
		// Row or Raw Seconds?
		// Usually raw number is Row in standard editors, or Measure?
		// In DDream Ctrl+G is "Go To [Time/Beat/Row]".
		// Let's assume Row if integer-like and large, or Time if small?
		// Or strictly Row.
		// Let's try to parse as int.
		if (Str::isNum(s)) {
			// Check if likely row or time.
			// If it contains a decimal point, it's time.
			if (Str::contains(s, ".")) {
				targetTime = Str::toDouble(s);
			} else {
				targetRow = Str::toInt(s);
			}
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
		// Maybe flash red or something?
	}
}

}; // namespace Vortex
