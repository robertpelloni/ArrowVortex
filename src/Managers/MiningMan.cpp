#include <Managers/MiningMan.h>
#include <bobcoin.h>
#include <Editor/Editor.h> // For preferences if needed

namespace Vortex {

struct MiningManImpl : public MiningMan {

    MiningManImpl() {
        Bobcoin::Initialize();
    }

    ~MiningManImpl() {
    }

    void update(float delta) override {
        // Periodic sync or network update could go here
    }

    void onNoteHit(float accuracy, int difficulty) override {
        // "Dance to Mine" logic
        // accuracy: 0.0 to 1.0 (1.0 = Marvelous)
        // difficulty: meter (e.g. 10)

        if (accuracy <= 0.0f) return;

        double effort = (double)accuracy * (double)difficulty;

        // Scale effort based on time?
        // For now, simple accumulation.
        Bobcoin::Mine(effort);
    }

    double getBalance() const override {
        return Bobcoin::GetBalance();
    }
};

MiningMan* gMining = nullptr;

void MiningMan::create() {
    gMining = new MiningManImpl();
}

void MiningMan::destroy() {
    delete (MiningManImpl*)gMining;
    gMining = nullptr;
}

}
