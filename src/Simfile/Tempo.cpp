#include <Precomp.h>

#include <Core/Utils/Util.h>
#include <Core/Utils/String.h>
#include <Core/Utils/Enum.h>
#include <Core/Common/Serialize.h>

#include <Core/System/Debug.h>

#include <Simfile/Common.h>
#include <Simfile/Simfile.h>
#include <Simfile/History.h>
#include <Simfile/Tempo.h>
#include <Simfile/Chart.h>

namespace AV {

Tempo::Tempo()
    : offset(0),
      displayBpmType(BPM_ACTUAL),
      displayBpmRange({0.0, 0.0}),
      segments(new SegmentGroup) {}

Tempo::~Tempo() { delete segments; }

void Tempo::copy(const Tempo* other) {
    offset = other->offset;
    attacks = other->attacks;
    keysounds = other->keysounds;
    misc = other->misc;

    segments->clear();
    segments->insert(*other->segments);

    displayBpmType = other->displayBpmType;
    displayBpmRange = other->displayBpmRange;
}

bool Tempo::hasSegments() const { return (segments->numSegments() > 0); }

void Tempo::sanitize(const Chart* owner) {
    segments->sanitize(owner);

    // Make sure there is a BPM change at row zero.
    auto bpmc = segments->begin<BpmChange>();
    auto end = segments->end<BpmChange>();
    if (bpmc == end || bpmc->row != 0) {
        double bpm = (bpmc != end) ? bpmc->bpm : SIM_DEFAULT_BPM;
        segments->insert(BpmChange(0, bpm));
        std::string suffix;
        if (owner) {
            Str::append(suffix, " to ");
            Str::append(suffix, owner->description());
        }
        HudNote("Added an initial BPM of %.3f%s.", bpm, suffix.c_str());
    }
}

};  // namespace Vortex
