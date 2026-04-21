#pragma once

#include <Core/Common/NonCopyable.h>
#include <Core/Common/Event.h>

#include <Simfile/Common.h>
#include <Simfile/Tempo/TimingData.h>
#include <Simfile/Tempo/SegmentSet.h>

namespace AV {

// Row/beat conversion constants.

constexpr double RowsPerBeat = 48.0;
constexpr double BeatsPerRow = 1.0 / 48.0;

// Converts a BPM value to seconds per row.
inline double SecPerRow(double beatsPerMin) {
    return 60.0 / (beatsPerMin * ROWS_PER_BEAT);
}

// Converts seconds per row to a BPM value.
inline double BeatsPerMin(double secPerRow) {
    return 60.0 /
           (secPerRow * ROWS_PER_BEAT);  // yes, this is identical to SecPerRow.
}

// Different ways to display the song BPM.
enum class DisplayBpmType { Actual, Random, Custom };

/// Unit used for attack timing.
enum AttackUnit { ATTACK_LENGTH, ATTACK_END };

/// Represents an attack (SM5).
struct Attack {
    double time, duration;
    std::string mods;
    AttackUnit unit;
};

/// Represents a BPM range.
struct BpmRange {
    double min, max;
};

/// Holds data that determines the tempo of a song or chart (and a few other
/// things).
struct Tempo : NonCopyable {
    Tempo();
    ~Tempo();

    /// Copies the data of another tempo.
    void copy(const Tempo* other);

    /// Returns true if the tempo contains one or more segments, false
    /// otherwise.
    bool hasSegments() const;

    /// Sanitizes the segments and makes sure there is a BPM change at row zero.
    void sanitize(const Chart* owner = nullptr);

    double offset;
    Vector<Attack> attacks;
    Vector<std::string> keysounds;
    Vector<Property> misc;
    SegmentGroup* segments;

    DisplayBpm displayBpmType;
    BpmRange displayBpmRange;
};

};  // namespace Vortex
