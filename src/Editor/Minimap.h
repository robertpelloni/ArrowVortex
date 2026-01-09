#pragma once

#include <Core/Input.h>

namespace Vortex {

<<<<<<< HEAD
struct Minimap : public InputHandler {
    enum Mode { NOTES, DENSITY };
=======
struct Minimap : public InputHandler
{
	enum Mode { NOTES, DENSITY, WAVEFORM };
>>>>>>> origin/feature-goto-quantize-insert

    static void create();
    static void destroy();

    virtual void tick() = 0;
    virtual void draw() = 0;

    virtual void setMode(Mode mode) = 0;
    virtual Mode getMode() const = 0;

    /// Called by the editor when changes were made to the simfile.
    virtual void onChanges(int changes) = 0;
};

extern Minimap* gMinimap;

};  // namespace Vortex
