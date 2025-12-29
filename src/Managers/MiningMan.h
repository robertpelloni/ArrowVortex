#pragma once

#include <Core/Core.h>

namespace Vortex {

class MiningMan
{
public:
    static void create();
    static void destroy();

    virtual void update(float delta) = 0;

    // Called when a note is hit in Practice Mode (or Gameplay)
    virtual void onNoteHit(float accuracy, int difficulty) = 0;

    virtual double getBalance() const = 0;
};

extern MiningMan* gMining;

}
