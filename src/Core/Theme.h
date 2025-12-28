#pragma once

#include <Core/Draw.h>

namespace Vortex {

struct Theme
{
	struct Dialog
	{
		colorf titlebar[3];
		colorf frame[3];
		colorf vshape[3];
	} dialog;

	struct Button
	{
		colorf base[3];
		colorf hover[3];
		colorf pressed[3];
	} button;

	struct Scrollbar
	{
		colorf bar[3];
		colorf base[3];
		colorf hover[3];
		colorf pressed[3];
	} scrollbar;

	struct TextBox
	{
		colorf base[4];
		colorf hover[4];
		colorf active[4];
	} textbox;

	struct Icons
	{
		colorf grab[2]; // bg, fg
		colorf pin;
		colorf unpin;
		colorf cross;
		colorf minus;
		colorf plus;
		colorf arrow;
		colorf check;
	} icons;

	struct Misc
	{
		color32 colDisabled;
		colorf imgSelect[2]; // bg, border
		colorf checkerboard[2]; // bg, fg
	} misc;

	static Theme& get();
	static void set(const Theme& theme);
	static void resetToDefault();

	static bool load(const char* path);
	static bool save(const char* path);
};

}; // namespace Vortex
