#pragma once

#include <Core/Types/Vec2.h>
#include <Vortex/Common.h>

namespace AV {

struct StreamGenerator {
    StreamGenerator();

<<<<<<< HEAD:src/Editor/StreamGenerator.h
    vec2i feetCols;
    int maxColRep;
    int maxBoxRep;
    bool allowBoxes;
    bool startWithRight;
    float patternDifficulty;

    void generate(int startRow, int endRow, SnapType spacing);
};

};  // namespace Vortex
=======
	int maxColRep;
	int maxBoxRep;
	bool allowBoxes;
	bool startWithRight;
	float patternDifficulty;

	void generate(int startRow, Row endRow, Vec2 feetCols, SnapType spacing);
};

} // namespace AV
>>>>>>> origin/fietsemaker-beta:src/Vortex/Edit/StreamGenerator.h
