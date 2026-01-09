# Project Summary: ArrowVortex v0.4.0 (DDreamStudio Features)

## Overview
This release implements a comprehensive suite of features inspired by **DDreamStudio**, targeting advanced synchronization and charting capabilities. Significant architectural improvements and build system fixes accompany these features.

## Key Features Implemented

### 1. Manual Syncing (Beat Dragging)
*   **Destructive Drag (Ripple)**: Allows shifting a beat and all subsequent beats by adjusting the BPM of the preceding segment. This replicates DDream's primary workflow for syncing drifting audio.
*   **Local Drag**: Warps the grid locally around a beat without affecting the entire chart.
*   **Input**: Mouse drag logic implemented in `View.cpp` and `TempoMan.cpp` (`destructiveShiftRowToTime`).

### 2. Advanced Audio Visualization
*   **Engine**: Integrated `Gist` library for FFT-based analysis.
*   **Modes**:
    *   **Spectrogram**: High-resolution frequency heatmap.
    *   **CQT / Chromagram**: Musical pitch-based analysis.
    *   **HPSS**: Harmonic/Percussive source separation.
    *   **Novelty/Flux**: Onset detection visualization.
    *   **Tempogram**: BPM probability visualization.

### 3. Automated Synchronization
*   **Auto-Sync Song**: Detects global BPM and Offset (`Action.cpp`).
*   **Auto-Sync Sections**: Detects varying BPMs across song sections.
*   **Grid Warping**: `Quantize to Audio` and `Warp Grid to Audio` align chart elements to detected onsets.
*   **Architecture**: Refactored to use `AsyncDetector` and background threads to prevent UI freezing during analysis.

### 4. Key Detection
*   **Implementation**: Estimates musical key (e.g., "C Major") by calculating a Chromagram via `Gist` and correlating it with Krumhansl-Schmuckler profiles (`AnalyzeKey.cpp`).
*   **Note**: Originally targeted `libaca`, but switched to `Gist` due to missing source files in the `libaca` submodule.

### 5. Practice Mode
*   **Functionality**: Simulates gameplay input judgment without modifying the chart (`Editing.cpp`).
*   **Feedback**: Displays timing error (ms) and judgment (Marvelous, Perfect, etc.).

## Technical & Build Improvements

*   **Dependency Management**:
    *   Cleaned up `ArrowVortex.vcxproj` to remove broken `libaca` references.
    *   Verified integration of `Gist` and `Aubio` (embedded).
*   **Code Compliance**:
    *   Refactored inline member function definitions in `Notefield.cpp`, `TempoBoxes.cpp`, and `Editing.cpp` to use out-of-line qualification (`Impl::Func`), ensuring strict C++ compliance for virtual interface implementations.
*   **Documentation**:
    *   **User Guide**: `docs/DDREAM_FEATURES.md`
    *   **Dashboard**: `docs/DASHBOARD.md`
    *   **Roadmap**: `docs/ROADMAP.md`

## Status
The codebase is currently in a **Release Candidate** state (v0.4.0). All functional requirements have been met, and blocking build issues have been resolved.
