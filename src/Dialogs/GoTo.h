#pragma once

#include <Dialogs/Dialog.h>
#include <Core/Widgets.h>
#include <Core/WidgetsLayout.h>

namespace Vortex {

class DialogGoTo : public EditorDialog
{
public:
	~DialogGoTo();
	DialogGoTo();

	void onChanges(int changes) override;

private:
	void myCreateWidgets();
	void myUpdateWidgets();
	void onGo();

	WgLineEdit* myInput;
	String myValue;
};

}; // namespace Vortex
