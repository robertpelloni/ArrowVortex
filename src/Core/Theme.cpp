#include <Core/Theme.h>
#include <Core/Xmr.h>
#include <Core/Utils.h>

namespace Vortex {

static Theme g_theme;

Theme& Theme::get()
{
	return g_theme;
}

void Theme::set(const Theme& theme)
{
	g_theme = theme;
}

// Helper to read/write colors
static void WriteColor(XmrNode* node, const char* name, const colorf& c)
{
	double v[4] = {c.r, c.g, c.b, c.a};
	node->addAttrib(name, v, 4);
}

static void ReadColor(const XmrNode* node, const char* name, colorf& c)
{
	double v[4];
	if(node->get(name, v, 4) == 4)
	{
		c = {(float)v[0], (float)v[1], (float)v[2], (float)v[3]};
	}
}

static void WriteColor32(XmrNode* node, const char* name, color32 c)
{
	colorf cf = ToColorf(c);
	WriteColor(node, name, cf);
}

static void ReadColor32(const XmrNode* node, const char* name, color32& c)
{
	colorf cf;
	ReadColor(node, name, cf);
	c = ToColor32(cf);
}

bool Theme::save(const char* path)
{
	XmrDoc doc;
	XmrNode* root = doc.addChild("theme");

	// Dialog
	XmrNode* nDialog = root->addChild("dialog");
	WriteColor(nDialog, "titlebar0", g_theme.dialog.titlebar[0]);
	WriteColor(nDialog, "titlebar1", g_theme.dialog.titlebar[1]);
	WriteColor(nDialog, "titlebar2", g_theme.dialog.titlebar[2]);
	WriteColor(nDialog, "frame0", g_theme.dialog.frame[0]);
	WriteColor(nDialog, "frame1", g_theme.dialog.frame[1]);
	WriteColor(nDialog, "frame2", g_theme.dialog.frame[2]);
	WriteColor(nDialog, "vshape0", g_theme.dialog.vshape[0]);
	WriteColor(nDialog, "vshape1", g_theme.dialog.vshape[1]);
	WriteColor(nDialog, "vshape2", g_theme.dialog.vshape[2]);

	// Button
	XmrNode* nButton = root->addChild("button");
	WriteColor(nButton, "base0", g_theme.button.base[0]);
	WriteColor(nButton, "base1", g_theme.button.base[1]);
	WriteColor(nButton, "base2", g_theme.button.base[2]);
	WriteColor(nButton, "hover0", g_theme.button.hover[0]);
	WriteColor(nButton, "hover1", g_theme.button.hover[1]);
	WriteColor(nButton, "hover2", g_theme.button.hover[2]);
	WriteColor(nButton, "pressed0", g_theme.button.pressed[0]);
	WriteColor(nButton, "pressed1", g_theme.button.pressed[1]);
	WriteColor(nButton, "pressed2", g_theme.button.pressed[2]);

	// Scrollbar
	XmrNode* nScroll = root->addChild("scrollbar");
	WriteColor(nScroll, "bar0", g_theme.scrollbar.bar[0]);
	WriteColor(nScroll, "bar1", g_theme.scrollbar.bar[1]);
	WriteColor(nScroll, "bar2", g_theme.scrollbar.bar[2]);
	WriteColor(nScroll, "base0", g_theme.scrollbar.base[0]);
	WriteColor(nScroll, "base1", g_theme.scrollbar.base[1]);
	WriteColor(nScroll, "base2", g_theme.scrollbar.base[2]);
	WriteColor(nScroll, "hover0", g_theme.scrollbar.hover[0]);
	WriteColor(nScroll, "hover1", g_theme.scrollbar.hover[1]);
	WriteColor(nScroll, "hover2", g_theme.scrollbar.hover[2]);
	WriteColor(nScroll, "pressed0", g_theme.scrollbar.pressed[0]);
	WriteColor(nScroll, "pressed1", g_theme.scrollbar.pressed[1]);
	WriteColor(nScroll, "pressed2", g_theme.scrollbar.pressed[2]);

	// TextBox
	XmrNode* nText = root->addChild("textbox");
	WriteColor(nText, "base0", g_theme.textbox.base[0]);
	WriteColor(nText, "base1", g_theme.textbox.base[1]);
	WriteColor(nText, "base2", g_theme.textbox.base[2]);
	WriteColor(nText, "base3", g_theme.textbox.base[3]);
	WriteColor(nText, "hover0", g_theme.textbox.hover[0]);
	WriteColor(nText, "hover1", g_theme.textbox.hover[1]);
	WriteColor(nText, "hover2", g_theme.textbox.hover[2]);
	WriteColor(nText, "hover3", g_theme.textbox.hover[3]);
	WriteColor(nText, "active0", g_theme.textbox.active[0]);
	WriteColor(nText, "active1", g_theme.textbox.active[1]);
	WriteColor(nText, "active2", g_theme.textbox.active[2]);
	WriteColor(nText, "active3", g_theme.textbox.active[3]);

	// Icons
	XmrNode* nIcons = root->addChild("icons");
	WriteColor(nIcons, "grab0", g_theme.icons.grab[0]);
	WriteColor(nIcons, "grab1", g_theme.icons.grab[1]);
	WriteColor(nIcons, "pin", g_theme.icons.pin);
	WriteColor(nIcons, "unpin", g_theme.icons.unpin);
	WriteColor(nIcons, "cross", g_theme.icons.cross);
	WriteColor(nIcons, "minus", g_theme.icons.minus);
	WriteColor(nIcons, "plus", g_theme.icons.plus);
	WriteColor(nIcons, "arrow", g_theme.icons.arrow);
	WriteColor(nIcons, "check", g_theme.icons.check);

	// Misc
	XmrNode* nMisc = root->addChild("misc");
	WriteColor32(nMisc, "colDisabled", g_theme.misc.colDisabled);
	WriteColor(nMisc, "imgSelect0", g_theme.misc.imgSelect[0]);
	WriteColor(nMisc, "imgSelect1", g_theme.misc.imgSelect[1]);
	WriteColor(nMisc, "checkerboard0", g_theme.misc.checkerboard[0]);
	WriteColor(nMisc, "checkerboard1", g_theme.misc.checkerboard[1]);

	return doc.saveFile(path, XmrSaveSettings()) == XMR_SUCCESS;
}

bool Theme::load(const char* path)
{
	XmrDoc doc;
	if(doc.loadFile(path) != XMR_SUCCESS) return false;

	XmrNode* root = doc.child("theme");
	if(!root) return false;

	// Dialog
	if(XmrNode* n = root->child("dialog"))
	{
		ReadColor(n, "titlebar0", g_theme.dialog.titlebar[0]);
		ReadColor(n, "titlebar1", g_theme.dialog.titlebar[1]);
		ReadColor(n, "titlebar2", g_theme.dialog.titlebar[2]);
		ReadColor(n, "frame0", g_theme.dialog.frame[0]);
		ReadColor(n, "frame1", g_theme.dialog.frame[1]);
		ReadColor(n, "frame2", g_theme.dialog.frame[2]);
		ReadColor(n, "vshape0", g_theme.dialog.vshape[0]);
		ReadColor(n, "vshape1", g_theme.dialog.vshape[1]);
		ReadColor(n, "vshape2", g_theme.dialog.vshape[2]);
	}

	// Button
	if(XmrNode* n = root->child("button"))
	{
		ReadColor(n, "base0", g_theme.button.base[0]);
		ReadColor(n, "base1", g_theme.button.base[1]);
		ReadColor(n, "base2", g_theme.button.base[2]);
		ReadColor(n, "hover0", g_theme.button.hover[0]);
		ReadColor(n, "hover1", g_theme.button.hover[1]);
		ReadColor(n, "hover2", g_theme.button.hover[2]);
		ReadColor(n, "pressed0", g_theme.button.pressed[0]);
		ReadColor(n, "pressed1", g_theme.button.pressed[1]);
		ReadColor(n, "pressed2", g_theme.button.pressed[2]);
	}

	// Scrollbar
	if(XmrNode* n = root->child("scrollbar"))
	{
		ReadColor(n, "bar0", g_theme.scrollbar.bar[0]);
		ReadColor(n, "bar1", g_theme.scrollbar.bar[1]);
		ReadColor(n, "bar2", g_theme.scrollbar.bar[2]);
		ReadColor(n, "base0", g_theme.scrollbar.base[0]);
		ReadColor(n, "base1", g_theme.scrollbar.base[1]);
		ReadColor(n, "base2", g_theme.scrollbar.base[2]);
		ReadColor(n, "hover0", g_theme.scrollbar.hover[0]);
		ReadColor(n, "hover1", g_theme.scrollbar.hover[1]);
		ReadColor(n, "hover2", g_theme.scrollbar.hover[2]);
		ReadColor(n, "pressed0", g_theme.scrollbar.pressed[0]);
		ReadColor(n, "pressed1", g_theme.scrollbar.pressed[1]);
		ReadColor(n, "pressed2", g_theme.scrollbar.pressed[2]);
	}

	// TextBox
	if(XmrNode* n = root->child("textbox"))
	{
		ReadColor(n, "base0", g_theme.textbox.base[0]);
		ReadColor(n, "base1", g_theme.textbox.base[1]);
		ReadColor(n, "base2", g_theme.textbox.base[2]);
		ReadColor(n, "base3", g_theme.textbox.base[3]);
		ReadColor(n, "hover0", g_theme.textbox.hover[0]);
		ReadColor(n, "hover1", g_theme.textbox.hover[1]);
		ReadColor(n, "hover2", g_theme.textbox.hover[2]);
		ReadColor(n, "hover3", g_theme.textbox.hover[3]);
		ReadColor(n, "active0", g_theme.textbox.active[0]);
		ReadColor(n, "active1", g_theme.textbox.active[1]);
		ReadColor(n, "active2", g_theme.textbox.active[2]);
		ReadColor(n, "active3", g_theme.textbox.active[3]);
	}

	// Icons
	if(XmrNode* n = root->child("icons"))
	{
		ReadColor(n, "grab0", g_theme.icons.grab[0]);
		ReadColor(n, "grab1", g_theme.icons.grab[1]);
		ReadColor(n, "pin", g_theme.icons.pin);
		ReadColor(n, "unpin", g_theme.icons.unpin);
		ReadColor(n, "cross", g_theme.icons.cross);
		ReadColor(n, "minus", g_theme.icons.minus);
		ReadColor(n, "plus", g_theme.icons.plus);
		ReadColor(n, "arrow", g_theme.icons.arrow);
		ReadColor(n, "check", g_theme.icons.check);
	}

	// Misc
	if(XmrNode* n = root->child("misc"))
	{
		ReadColor32(n, "colDisabled", g_theme.misc.colDisabled);
		ReadColor(n, "imgSelect0", g_theme.misc.imgSelect[0]);
		ReadColor(n, "imgSelect1", g_theme.misc.imgSelect[1]);
		ReadColor(n, "checkerboard0", g_theme.misc.checkerboard[0]);
		ReadColor(n, "checkerboard1", g_theme.misc.checkerboard[1]);
	}

	return true;
}

void Theme::resetToDefault()
{
	// Dialog
	g_theme.dialog.titlebar[0] = {0.05f, 0.05f, 0.05f, 1.0f};
	g_theme.dialog.titlebar[1] = {0.18f, 0.18f, 0.18f, 1.0f};
	g_theme.dialog.titlebar[2] = {0.15f, 0.15f, 0.15f, 1.0f};

	g_theme.dialog.frame[0] = {0.05f, 0.05f, 0.05f, 1.0f};
	g_theme.dialog.frame[1] = {0.35f, 0.35f, 0.35f, 1.0f};
	g_theme.dialog.frame[2] = {0.20f, 0.20f, 0.20f, 1.0f};

	g_theme.dialog.vshape[0] = {0.05f, 0.05f, 0.05f, 1.0f};
	g_theme.dialog.vshape[1] = {0.35f, 0.35f, 0.35f, 1.0f};
	g_theme.dialog.vshape[2] = {0.20f, 0.20f, 0.20f, 1.0f};

	// Button
	g_theme.button.base[0] = {0.10f, 0.10f, 0.10f, 1.0f};
	g_theme.button.base[1] = {0.32f, 0.32f, 0.32f, 1.0f};
	g_theme.button.base[2] = {0.28f, 0.28f, 0.28f, 1.0f};

	g_theme.button.hover[0] = {0.20f, 0.20f, 0.20f, 1.0f};
	g_theme.button.hover[1] = {1.00f, 1.00f, 1.00f, 0.2f};
	g_theme.button.hover[2] = {1.00f, 1.00f, 1.00f, 0.025f};

	g_theme.button.pressed[0] = {0.20f, 0.20f, 0.20f, 1.0f};
	g_theme.button.pressed[1] = {1.00f, 1.00f, 1.00f, 0.3f};
	g_theme.button.pressed[2] = {1.00f, 1.00f, 1.00f, 0.0f};

	// Scrollbar
	g_theme.scrollbar.bar[0] = {0.25f, 0.25f, 0.25f, 1.0f};
	g_theme.scrollbar.bar[1] = {0.08f, 0.08f, 0.08f, 1.0f};
	g_theme.scrollbar.bar[2] = {0.12f, 0.12f, 0.12f, 1.0f};

	g_theme.scrollbar.base[0] = {0.10f, 0.10f, 0.10f, 1.0f};
	g_theme.scrollbar.base[1] = {0.32f, 0.32f, 0.32f, 1.0f};
	g_theme.scrollbar.base[2] = {0.28f, 0.28f, 0.28f, 1.0f};

	g_theme.scrollbar.hover[0] = {0.20f, 0.20f, 0.20f, 1.0f};
	g_theme.scrollbar.hover[1] = {1.00f, 1.00f, 1.00f, 0.2f};
	g_theme.scrollbar.hover[2] = {1.00f, 1.00f, 1.00f, 0.025f};

	g_theme.scrollbar.pressed[0] = {0.20f, 0.20f, 0.20f, 1.0f};
	g_theme.scrollbar.pressed[1] = {1.00f, 1.00f, 1.00f, 0.3f};
	g_theme.scrollbar.pressed[2] = {1.00f, 1.00f, 1.00f, 0.0f};

	// TextBox
	g_theme.textbox.base[0] = {0.25f, 0.25f, 0.25f, 1.0f};
	g_theme.textbox.base[1] = {0.25f, 0.25f, 0.25f, 1.0f};
	g_theme.textbox.base[2] = {0.10f, 0.10f, 0.10f, 1.0f};
	g_theme.textbox.base[3] = {0.13f, 0.13f, 0.13f, 1.0f};

	g_theme.textbox.hover[0] = {0.25f, 0.25f, 0.25f, 1.0f};
	g_theme.textbox.hover[1] = {0.30f, 0.30f, 0.30f, 1.0f};
	g_theme.textbox.hover[2] = {0.10f, 0.10f, 0.10f, 1.0f};
	g_theme.textbox.hover[3] = {0.13f, 0.13f, 0.13f, 1.0f};

	g_theme.textbox.active[0] = {0.25f, 0.25f, 0.25f, 1.0f};
	g_theme.textbox.active[1] = {0.40f, 0.40f, 0.40f, 1.0f};
	g_theme.textbox.active[2] = {0.10f, 0.10f, 0.10f, 1.0f};
	g_theme.textbox.active[3] = {0.13f, 0.13f, 0.13f, 1.0f};

	// Icons
	g_theme.icons.grab[0] = {0.2f, 0.2f, 0.2f, 1.0f};
	g_theme.icons.grab[1] = {0.6f, 0.6f, 0.6f, 1.0f};
	g_theme.icons.pin = {1.0f, 1.0f, 1.0f, 1.0f};
	g_theme.icons.unpin = {1.0f, 1.0f, 1.0f, 1.0f};
	g_theme.icons.cross = {1.0f, 1.0f, 1.0f, 1.0f};
	g_theme.icons.minus = {1.0f, 1.0f, 1.0f, 1.0f};
	g_theme.icons.plus = {1.0f, 1.0f, 1.0f, 1.0f};
	g_theme.icons.arrow = {1.0f, 1.0f, 1.0f, 1.0f};
	g_theme.icons.check = {1.0f, 1.0f, 1.0f, 1.0f};

	// Misc
	g_theme.misc.colDisabled = Color32(166);
	g_theme.misc.imgSelect[0] = {0.5f, 0.5f, 0.6f, 1.0f};
	g_theme.misc.imgSelect[1] = {0.3f, 0.3f, 0.4f, 1.0f};
	g_theme.misc.checkerboard[0] = {0.4f, 0.4f, 0.4f, 1.0f};
	g_theme.misc.checkerboard[1] = {0.6f, 0.6f, 0.6f, 1.0f};
}

}; // namespace Vortex
