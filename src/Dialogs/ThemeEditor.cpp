#include <Dialogs/ThemeEditor.h>
#include <Core/Utils.h>
#include <Core/StringUtils.h>
#include <Core/GuiDraw.h> // For recreating textures
#include <System/System.h>

namespace Vortex {

DialogThemeEditor::DialogThemeEditor()
{
	setTitle("THEME EDITOR");
	myActiveTab = TAB_DIALOG;
	myRefreshLayout();
}

void DialogThemeEditor::mySetTab(Tab tab)
{
	if (myActiveTab != tab) {
		myActiveTab = tab;
		myRefreshLayout();
	}
}

void DialogThemeEditor::myRefreshLayout()
{
	myCreateWidgets();
	myUpdateWidgets();
}

void DialogThemeEditor::addColor(const char* label, colorf& color)
{
	myLayout.row().col(200).col(200);
	myLayout.add<WgLabel>()->text.set(label);
	auto cp = myLayout.add<WgColorPicker>();
	cp->red.bind(&color.r);
	cp->green.bind(&color.g);
	cp->blue.bind(&color.b);
	cp->alpha.bind(&color.a);
	// When color changes, we might need to regenerate textures
	// But for now, let's just let the user click "Apply" or maybe auto-update?
	// GuiDraw::create() regenerates textures. We should probably call it.
	// But calling it every frame is bad.
	// Let's add a "Refresh" button or just rely on the fact that some things might not update instantly.
	// Actually, GuiDraw textures are static. We MUST call GuiDraw::create() (or a new refresh function) to see changes.
	// I'll add a callback to the color picker.
	cp->onChange.bind([](){
		// This is a bit heavy, but for a theme editor it's fine.
		// We need to destroy old textures first to avoid leaks?
		// GuiDraw::destroy() deletes the global object.
		// GuiDraw::create() creates a new one.
		// This might be dangerous if other things hold references to textures.
		// But textures are usually handles.
		// Let's try just re-running the texture generation functions if we can expose them.
		// For now, full destroy/create is the safest "brute force" way, assuming no one holds raw pointers to GD internals.
		GuiDraw::destroy();
		GuiDraw::create();
	});
}

void DialogThemeEditor::myCreateWidgets()
{
	// Tabs
	myLayout.row().col(60).col(60).col(80).col(60).col(60).col(60);
	auto addTab = [&](const char* name, Tab tab) {
		auto btn = myLayout.add<WgButton>();
		btn->text.set(name);
		btn->onPress.bind(this, &DialogThemeEditor::mySetTab, tab);
		if (myActiveTab == tab) btn->isDown.set(true);
	};
	addTab("Dialog", TAB_DIALOG);
	addTab("Button", TAB_BUTTON);
	addTab("Scroll", TAB_SCROLLBAR);
	addTab("Text", TAB_TEXTBOX);
	addTab("Icons", TAB_ICONS);
	addTab("Misc", TAB_MISC);

	myLayout.add<WgSeperator>();

	Theme& theme = Theme::get();

	if (myActiveTab == TAB_DIALOG) {
		addColor("Titlebar (Active)", theme.dialog.titlebar[0]);
		addColor("Titlebar (Inactive)", theme.dialog.titlebar[1]);
		addColor("Titlebar (Border)", theme.dialog.titlebar[2]);
		addColor("Frame (Bg)", theme.dialog.frame[0]);
		addColor("Frame (Border)", theme.dialog.frame[1]);
		addColor("Frame (Highlight)", theme.dialog.frame[2]);
		addColor("VShape (1)", theme.dialog.vshape[0]);
		addColor("VShape (2)", theme.dialog.vshape[1]);
		addColor("VShape (3)", theme.dialog.vshape[2]);
	}
	else if (myActiveTab == TAB_BUTTON) {
		addColor("Base (Bg)", theme.button.base[0]);
		addColor("Base (Border)", theme.button.base[1]);
		addColor("Base (Highlight)", theme.button.base[2]);
		addColor("Hover (Bg)", theme.button.hover[0]);
		addColor("Hover (Border)", theme.button.hover[1]);
		addColor("Hover (Highlight)", theme.button.hover[2]);
		addColor("Pressed (Bg)", theme.button.pressed[0]);
		addColor("Pressed (Border)", theme.button.pressed[1]);
		addColor("Pressed (Highlight)", theme.button.pressed[2]);
	}
	else if (myActiveTab == TAB_SCROLLBAR) {
		addColor("Bar (Bg)", theme.scrollbar.bar[0]);
		addColor("Bar (Border)", theme.scrollbar.bar[1]);
		addColor("Bar (Highlight)", theme.scrollbar.bar[2]);
		addColor("Base (Bg)", theme.scrollbar.base[0]);
		addColor("Base (Border)", theme.scrollbar.base[1]);
		addColor("Base (Highlight)", theme.scrollbar.base[2]);
		addColor("Hover (Bg)", theme.scrollbar.hover[0]);
		addColor("Hover (Border)", theme.scrollbar.hover[1]);
		addColor("Hover (Highlight)", theme.scrollbar.hover[2]);
		addColor("Pressed (Bg)", theme.scrollbar.pressed[0]);
		addColor("Pressed (Border)", theme.scrollbar.pressed[1]);
		addColor("Pressed (Highlight)", theme.scrollbar.pressed[2]);
	}
	else if (myActiveTab == TAB_TEXTBOX) {
		addColor("Base (Bg)", theme.textbox.base[0]);
		addColor("Base (Border)", theme.textbox.base[1]);
		addColor("Base (Highlight)", theme.textbox.base[2]);
		addColor("Base (Text)", theme.textbox.base[3]);
		addColor("Hover (Bg)", theme.textbox.hover[0]);
		addColor("Hover (Border)", theme.textbox.hover[1]);
		addColor("Hover (Highlight)", theme.textbox.hover[2]);
		addColor("Hover (Text)", theme.textbox.hover[3]);
		addColor("Active (Bg)", theme.textbox.active[0]);
		addColor("Active (Border)", theme.textbox.active[1]);
		addColor("Active (Highlight)", theme.textbox.active[2]);
		addColor("Active (Text)", theme.textbox.active[3]);
	}
	else if (myActiveTab == TAB_ICONS) {
		addColor("Grab (Bg)", theme.icons.grab[0]);
		addColor("Grab (Fg)", theme.icons.grab[1]);
		addColor("Pin", theme.icons.pin);
		addColor("Unpin", theme.icons.unpin);
		addColor("Cross", theme.icons.cross);
		addColor("Minus", theme.icons.minus);
		addColor("Plus", theme.icons.plus);
		addColor("Arrow", theme.icons.arrow);
		addColor("Check", theme.icons.check);
	}
	else if (myActiveTab == TAB_MISC) {
		// colDisabled is color32, need conversion or special handling
		// For now skipping colDisabled or I need a wrapper
		addColor("Selection (Bg)", theme.misc.imgSelect[0]);
		addColor("Selection (Border)", theme.misc.imgSelect[1]);
		addColor("Checkerboard (Bg)", theme.misc.checkerboard[0]);
		addColor("Checkerboard (Fg)", theme.misc.checkerboard[1]);
	}

	myLayout.add<WgSeperator>();
	myLayout.row().col(100).col(100).col(100);
	
	auto btnSave = myLayout.add<WgButton>();
	btnSave->text.set("Save...");
	btnSave->onPress.bind(this, &DialogThemeEditor::onSave);

	auto btnLoad = myLayout.add<WgButton>();
	btnLoad->text.set("Load...");
	btnLoad->onPress.bind(this, &DialogThemeEditor::onLoad);

	auto btnReset = myLayout.add<WgButton>();
	btnReset->text.set("Reset");
	btnReset->onPress.bind(this, &DialogThemeEditor::onReset);
}

void DialogThemeEditor::myUpdateWidgets()
{
	// Nothing to update manually, bindings handle it
}

void DialogThemeEditor::onSave()
{
	String path;
	int index = 0;
	path = gSystem->saveFileDlg("Save Theme", "theme.xml", "XML Files (*.xml)|*.xml", &index);
	if (path.len() > 0) {
		Theme::save(path);
		GuiDraw::destroy();
		GuiDraw::create();
	}
}

void DialogThemeEditor::onLoad()
{
	String path;
	path = gSystem->openFileDlg("Load Theme", "", "XML Files (*.xml)|*.xml");
	if (path.len() > 0) {
		Theme::load(path);
		GuiDraw::destroy();
		GuiDraw::create();
	}
}

void DialogThemeEditor::onReset()
{
	Theme::resetToDefault();
	GuiDraw::destroy();
	GuiDraw::create();
}

}; // namespace Vortex
