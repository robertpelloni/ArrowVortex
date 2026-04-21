#pragma once

#include <Core/Types/Vec2.h>
#include <Vortex/Common.h>

namespace AV {

struct StreamGenerator {
    StreamGenerator();

    vec2i feetCols;
    int maxColRep;
    int maxBoxRep;
    bool allowBoxes;
    bool startWithRight;
    float patternDifficulty;

    void generate(int startRow, int endRow, SnapType spacing);
};

};  // namespace Vortex
