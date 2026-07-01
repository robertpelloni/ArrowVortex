# Changelog

## [1.3.3] - 2026-06-06

### Added
- **AI Integration**: Restored and fully integrated the `lib/ddc` submodule using PyTorch 2.12.0.
- **Model Training**: Successfully trained and verified PyTorch models for Onset Detection and FFR Difficulty.
- **Asynchronous UI**: Implemented non-blocking DDC generation in `BatchDDC.cpp` using background threading to prevent UI hangs.
- **Data Persistence**: Enabled Git tracking for DDC training data, extracted features, and resulting models to ensure reproducible research and deployment.
- **Documentation**: Created `VISION.md`, `MEMORY.md`, `DEPLOY.md`, `IDEAS.md`, `ROADMAP.md`, and `TODO.md` to establish strict documentation governance.

### Changed
- **Versioning**: Synchronized project versioning with `VERSION.md` (v1.3.3).
- **Inference**: Patched `autochart_lib.py` for compatibility with `simfile` 2.1+ API and standardized note padding (4 for single, 8 for double).
- **UI**: Improved model validation in `BatchDDC.cpp` to support PyTorch `.pth` files and added automated fallback path detection.
- **CI/CD**: Resolved Windows build timeouts and optimized dependency detection in `CMakeLists.txt`.

## [1.3.2] - 2026-01-08
...

## [1.3.1] - 2025-12-27

### Changed
- **Submodules**: Updated `lib/ddc` to latest version (v1.0-172-gc8caffe).
- **Documentation**: Updated DASHBOARD.md with latest submodule information.

## [1.3.0] - 2025-12-27

### Added
- **Lua Scripting Engine**:
  - Integrated Lua 5.x for advanced chart manipulation.
  - Added `Scripts` menu to the main menu bar.
  - Implemented `LuaMan` manager for handling script execution.
  - Exposed `Vortex` global API for scripts (Chart, Selection, Tempo, etc.).
  - **New API Functions**: `setSongTitle`, `setSongArtist`, `setChartMeter`.
  - Added `LUA_API.md` documentation.
  - Included example scripts: `quantize_4th.lua`, `chart_stats.lua`.
- **Visual Sync (Beat Dragging)**:
  - Implemented "Ripple" editing logic in `TempoMan`.
  - Added visual feedback for dragging beats/sub-beats on the waveform.
  - Bound to `Ctrl + Drag` (or configured modifier) in the waveform view.
- **Build System**:
  - Added `CMakeLists.txt` for cross-platform build support (Linux/macOS).
- **Waveform Analysis**:
  - Verified and documented existing implementation of advanced waveform modes (CQT, HPSS, Chromagram).
  - Confirmed full UI integration in `WaveformSettings`.

### Changed
- **Batch DDC**:
  - Finalized Python script integration.
  - Improved error handling and status reporting in the Batch DDC dialog.

## [1.2.0] - 2025-12-25

### Added
- **Batch DDC Generation**: Integrated Dance Dance Convolution (DDC) for auto-charting.
  - Added `lib/ddc` submodule.
  - Added Batch DDC Dialog (`File -> Batch DDC Generation...`).
  - Added support for processing entire folders recursively.
  - Added integration with FFR Difficulty Model for chart rating.
  - Added Python Path preference in Settings.
- **Practice Mode**:
  - Added Practice Mode toggle in Preferences.
  - Implemented timing windows (Marvelous, Perfect, Great, Good, Boo, Miss, etc.).
  - Added visual feedback for judgments.
  - Added logic to handle looping/seeking (resetting judgments).
- **FPS Counter**: Added optional FPS display (toggle in Preferences).
- **osu! Support**: Added basic support for loading `.osu` files.
- **Scroll Cursor Effect**: Added option to enable/disable cursor scrolling effect.

### Changed
- **Auto-Sync**:
  - Refined `AUTO_SYNC_SONG` to clear existing tempo map before applying new sync.
  - Refined `QUANTIZE_TO_AUDIO` to snap notes to onsets instead of warping grid.
- **Preferences**:
  - Reorganized Preferences dialog with tabs (Editor, Practice).
  - Added tooltips for settings.

### Fixed
- **Practice Mode**: Fixed issue where looping audio would not reset judged notes.

## [1.1.0] - Previous Release
- Initial DDreamStudio feature integration.

## [1.3.4] - 2026-06-22

### Added
- **Automated DDC Downloader**: Added a new dialog (`File -> Download DDC Models`) to asynchronously fetch the required PyTorch models directly into the `models/` directory without blocking the UI.
- **Batch DDC Cancellations**: Added an explicit UI "CANCEL" button to the Batch DDC Generation window, cleanly terminating the background thread and subprocess.
- **Log Streaming**: Replaced blocking system commands with a `BackgroundThread` and `onTick` file streaming implementation to provide real-time textual feedback during ML generation.

### Changed
- **Python Serialization**: Fixed a crash in the ML backend where literal "None" strings were polluting serialized `.sm` files.
- **Merge Governance**: Synced active feature branches into `main` and fully resolved overlapping UI framework conflicts.
