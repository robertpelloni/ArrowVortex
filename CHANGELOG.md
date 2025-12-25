# Changelog

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
