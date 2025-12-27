#include <Core/Theme.h>

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
