#pragma once
#include <Core/String.h>
#include <Core/Vector.h>
#include <Core/Core.h>
#include <Core/Color.h>

namespace Vortex {

struct LyricLine {
	double time;
	String text;
	color32 color; // For editor highlighting
};

struct LyricsMan {
	static void create();
	static void destroy();

	virtual void load(StringRef path) = 0;
	virtual void save(StringRef path) = 0;
	virtual const Vector<LyricLine>& getLyrics() const = 0;
	virtual void setLyrics(const Vector<LyricLine>& lyrics) = 0;
	virtual void addLyric(double time, StringRef text) = 0;
	virtual void removeLyric(int index) = 0;
	virtual void clear() = 0;
};

extern LyricsMan* gLyrics;

}
