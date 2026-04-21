#include <Precomp.h>

#include <Core/Utils/Util.h>
#include <Core/Utils/Enum.h>
#include <Core/Utils/String.h>
#include <Core/Common/Serialize.h>

#include <Core/Graphics/Color.h>
#include <Core/System/Debug.h>
#include <Core/System/Log.h>

#include <Simfile/Chart.h>
#include <Simfile/NoteSet.h>
#include <Simfile/NoteUtils.h>
#include <Simfile/GameMode.h>
#include <Simfile/Tempo.h>
#include <Simfile/Simfile.h>
#include <Simfile/History.h>

namespace AV {

Chart::Chart()
    : style(nullptr), difficulty(DIFF_BEGINNER), meter(1), tempo(nullptr) {}

Chart::~Chart() {
    style = nullptr;
    difficulty = DIFF_BEGINNER;
    meter = 1;
    delete tempo;
}

std::string Chart::description() const {
    return Str::fmt("%1 %2").arg(GetDifficultyName(difficulty)).arg(meter);
}

bool Chart::hasTempo() const {
    return (tempo && (tempo->hasSegments() || tempo->offset != 0));
}

Tempo* Chart::getTempo(Simfile* simfile) {
    return hasTempo() ? tempo : simfile->tempo;
}

const Tempo* Chart::getTempo(const Simfile* simfile) const {
    return hasTempo() ? tempo : simfile->tempo;
}

int Chart::stepCount() const {
    int count = 0;
    for (auto& note : notes) {
        count += (note.type != NOTE_MINE);
    }
    return count;
}

void Chart::sanitize() {
    if (!style) {
        HudWarning("%s is missing a style.", description().c_str());
    }

    if (meter <= 0) {
        HudWarning("%s has an invalid meter value %i.", description().c_str(),
                   meter);
        meter = 1;
    }

    if (hasTempo()) {
        tempo->sanitize(this);
    }

    notes.sanitize(this);
}

static const char* DifficultyNames[NUM_DIFFICULTIES] = {
    "Beginner", "Easy", "Medium", "Hard", "Challenge", "Edit"};

const char* GetDifficultyName(Difficulty difficulty) {
    return DifficultyNames[difficulty];
}

};  // namespace Vortex
