#pragma once

#include <Dialogs/Dialog.h>
#include <Core/Theme.h>

namespace Vortex {

class DialogThemeEditor : public EditorDialog
{
public:
	DialogThemeEditor();

private:
	void myCreateWidgets();
	void myUpdateWidgets();
	void myRefreshLayout();

	enum Tab {
		TAB_DIALOG,
		TAB_BUTTON,
		TAB_SCROLLBAR,
		TAB_TEXTBOX,
		TAB_ICONS,
		TAB_MISC
	};
	Tab myActiveTab;
	void mySetTab(Tab tab);

	void onSave();
	void onLoad();
	void onReset();

	// Helper to add a color picker
	void addColor(const char* label, colorf& color);
	void addColor32(const char* label, color32& color); // Needs special handling
};

}; // namespace Vortex
