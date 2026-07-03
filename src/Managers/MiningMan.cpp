#include <Managers/MiningMan.h>
#include <bobcoin.h>
#include <ProofOfDance.h>
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
        // We now delegate effort to the advanced ProofOfDance module
        // We simulate a basic session here since Practice Mode is note-by-note.
        // In a real session, we'd wait until the end of the song to calculate the total combo and notes.
        // For demonstration, we assume we just hit a single note in a sequence.

        if (accuracy <= 0.0f) {
            // Note miss or boo
            myCurrentCombo = 0;
            return;
        }

        // Arbitrary tracking for Practice Mode single-hit logic
        myTotalNotesHit++;
        myCurrentCombo++;
        if (myCurrentCombo > myMaxCombo) myMaxCombo = myCurrentCombo;

        // Create the localized proof and broadcast
        // In full gameplay, this happens ONCE at the evaluation screen
        Bobcoin::ProofOfDance::GenerateAndSubmitProof(
            difficulty,
            1, // Just this note for real-time accumulation
            myCurrentCombo,
            accuracy,
            "mock_chart_hash_abcd1234"
        );
    }

private:
    int myTotalNotesHit = 0;
    int myCurrentCombo = 0;
    int myMaxCombo = 0;

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
