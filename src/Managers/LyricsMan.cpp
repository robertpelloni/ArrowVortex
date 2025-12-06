#include <Managers/LyricsMan.h>
#include <System/File.h>
#include <Core/Utils.h>
#include <Core/StringUtils.h>
#include <algorithm>
#include <stdio.h>

namespace Vortex {

struct LyricsManImpl : public LyricsMan {
	Vector<LyricLine> myLyrics;

	void load(StringRef path) override {
		myLyrics.clear();
		if (path.empty()) return;

		bool success;
		Vector<String> lines = File::getLines(path, &success);
		if (!success) return;

		for (const auto& line : lines) {
			int endBracket = Str::find(line, "]");
			if (endBracket != String::npos && Str::startsWith(line, "[")) {
				String timeStr = Str::substr(line, 1, endBracket - 1);
				String text = Str::substr(line, endBracket + 1);

				int colon = Str::find(timeStr, ":");
				if (colon != String::npos) {
					String mm = Str::substr(timeStr, 0, colon);
					String ss = Str::substr(timeStr, colon + 1);
					double time = atof(mm.str()) * 60.0 + atof(ss.str());
					myLyrics.push_back({time, text, 0xFFFFFFFF});
				}
			}
		}
		std::sort(myLyrics.begin(), myLyrics.end(), [](const LyricLine& a, const LyricLine& b){ return a.time < b.time; });
	}

	void save(StringRef path) override {
		FileWriter out;
		if (out.open(path)) {
			for (const auto& l : myLyrics) {
				int m = (int)(l.time / 60);
				double s = l.time - m * 60;
				String line;
				Str::fmt(line, "[%02d:%05.2f]%s\n");
				line.arg(m).arg(s).arg(l.text);
				out.write(line.str(), 1, line.len());
			}
		}
	}

	const Vector<LyricLine>& getLyrics() const override {
		return myLyrics;
	}

	void setLyrics(const Vector<LyricLine>& lyrics) override {
		myLyrics = lyrics;
	}

	void addLyric(double time, StringRef text) override {
		myLyrics.push_back({time, text, 0xFFFFFFFF});
		std::sort(myLyrics.begin(), myLyrics.end(), [](const LyricLine& a, const LyricLine& b){ return a.time < b.time; });
	}

	void removeLyric(int index) override {
		if (index >= 0 && index < myLyrics.size()) {
			myLyrics.erase(index);
		}
	}

	void clear() override {
		myLyrics.clear();
	}
};

LyricsMan* gLyrics = nullptr;

void LyricsMan::create() { gLyrics = new LyricsManImpl; }
void LyricsMan::destroy() { delete gLyrics; gLyrics = nullptr; }

}
