#pragma once

#include <Core/Reference.h>
#include <Core/Vector.h>

namespace Vortex {

struct TempoResult
{
	double bpm;
	double offset;
	double confidence;
};

struct ReferenceCounted {
    virtual ~ReferenceCounted() {}
};

struct TempoDetector : public ReferenceCounted
{
	virtual ~TempoDetector() {}
	static TempoDetector* New(double startTime, double length);

	virtual bool hasResult() = 0;
	virtual Vector<TempoResult> getResult() = 0;
};

}; // namespace Vortex
