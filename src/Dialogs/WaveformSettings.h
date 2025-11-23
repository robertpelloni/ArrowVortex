#pragma once

#include <Dialogs/Dialog.h>

#include <Core/Widgets.h>
#include <Core/WidgetsLayout.h>

#include <Editor/Waveform.h>

namespace Vortex {

class DialogWaveformSettings : public EditorDialog
{
public:
	~DialogWaveformSettings();
	DialogWaveformSettings();

private:
	void myApplyPreset();
	void myUpdateSettings();
	void myEnableFilter();
	void myDisableFilter();
	void myToggleOverlayFilter();
	void myToggleShowOnsets();
	void myUpdateOnsets();
	void myUpdateSpectrogram();
	void myUpdateRGB();

	Waveform::ColorScheme settingsColorScheme_;
	int presetIndex_;
	int luminanceValue_;
	int colorMode_;
	int waveShape_;
	int antiAliasingMode_;

	int filterType_;
	float filterStrength_;
	bool isOverlayFilterActive_;
	bool isShowingOnsets_;
	float onsetThreshold_;
	float spectrogramGain_;
	float rgbLow_;
	float rgbHigh_;
};

}; // namespace Vortex
