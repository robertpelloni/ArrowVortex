# Changelog

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
