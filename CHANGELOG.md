# Changelog

## [0.4.0] - 2024-05-20 (DDream Feature Set)

### Added
*   **DDreamStudio Features**: A suite of features inspired by DDreamStudio to enhance syncing and charting.
    *   **Beat Dragging**: Manual waveform syncing via mouse drag (Destructive/Ripple mode via Shift+Drag).
    *   **Advanced Waveforms**: New visualizations including Spectrogram, CQT, Chromagram, Pitch, HPSS, Novelty, and Tempogram.
    *   **Auto-Sync**: Tools for automatic BPM detection (`Auto-Sync Song`), section analysis (`Auto-Sync Sections`), and grid warping (`Warp Grid to Audio`).
    *   **Practice Mode**: Gameplay simulation mode for testing patterns without editing.
    *   **Key Detection**: Analyzes audio to estimate musical key using Chromagram correlation.
*   **Preferences**:
    *   New settings for waveform visuals, practice mode timing windows, and editing behaviors (e.g., "Edit One Layer").
    *   "Scroll Cursor Effect" for page-mode-like behavior.
*   **Documentation**: Added `docs/DDREAM_FEATURES.md` detailing new capabilities.

### Changed
*   **Build System**: Updated `ArrowVortex.vcxproj` to include new source files. Removed `libaca` dependency in favor of `Gist` for simplified build and licensing.
*   **Performance**: Tempo detection and analysis tasks now run asynchronously on background threads to prevent UI freezes.

### Fixed
*   **Syntax**: Addressed C++ syntax compliance issues in virtual interface implementations (`Notefield`, `TempoBoxes`, `Editing`).
