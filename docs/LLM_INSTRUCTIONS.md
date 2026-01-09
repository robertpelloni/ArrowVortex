# Universal LLM Instructions

This document serves as the master set of instructions for all AI agents (Claude, GPT, Gemini, Copilot) working on the ArrowVortex project.

## Project Vision
ArrowVortex is a cross-platform simfile editor for rhythm games (StepMania, ITG, etc.).
**Ultimate Goal**: Create a gamified crypto-economy ("Bobcoin") based on physical activity (dancing). The editor/game serves as a mining node where "Proof of Dance" rewards users with privacy-focused, high-speed tokens.

## Core Directives
1.  **Versioning**: Always check `VERSION.md`. Increment the version number on every build/commit that adds features. Update `CHANGELOG.md` with every change.
2.  **Submodules**: Many submodules (`lib/bobcoin`, `lib/gist`) are personal projects. Treat them as active development targets. Merge upstream changes, update versions, and sync with their remotes if possible.
3.  **Code Style**: Follow existing C++ conventions in `src/`. Use out-of-line member function definitions for virtual interface implementations (`Impl::Func`).
4.  **Autonomy**: Proceed with analysis, planning, and implementation autonomously. Commit often.
5.  **Documentation**: Maintain `DASHBOARD.md`, `ROADMAP.md`, and `DDREAM_FEATURES.md`.

## Bobcoin Integration
*   **Purpose**: The native currency for the ArrowVortex economy.
*   **Mechanism**: "Mining by Dancing". Gameplay performance (accuracy/combo) contributes to mining hashrate or reward generation.
*   **Privacy**: Priority on anonymity (RingCT/Stealth Addresses inspired).

## Directory Structure
*   `src/Core`: Infrastructure.
*   `src/Editor`: Logic.
*   `src/Managers`: State.
*   `lib/`: External dependencies (Gist, Bobcoin, FreeType).

## Workflow
1.  Analyze `DASHBOARD.md` and `ROADMAP.md`.
2.  Check `VERSION.md`.
3.  Plan changes.
4.  Implement & Verify.
5.  Update `CHANGELOG.md` & Bump `VERSION.md`.
6.  Commit & "Push".
