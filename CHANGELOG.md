# Changelog

All notable changes to this project will be documented in this file.

## [1.1.0] - 2025-12-25

### Added
- **DDreamStudio Features Integration**:
    - **Visual Sync**: Beat dragging (Shift+Click+Drag) on the grid to align beats with audio.
    - **Advanced Waveforms**: Spectrogram, CQT, and other visualization modes.
    - **Auto-Sync**: Tools for automatic BPM detection and grid alignment (`AUTO_SYNC_SONG`, `QUANTIZE_TO_AUDIO`).
    - **Preferences Dialog**: New dialog for editor settings and practice mode configuration.
- **osu! Support**: Added support for loading `.osu` simfiles.
- **Lyrics Editor**: New dialog for editing lyrics.
- **Background Changes**: New dialog for managing background changes.
- **Go To**: New dialog for jumping to specific times or measures.

### Changed
- Updated `ArrowVortex` version to 1.1.0.
- Improved `SimfileMan` to handle new file formats.
- Refactored `ByteStream` for better template support.

### Fixed
- Resolved conflicts in `ByteStream.h` and `SimfileMan.cpp`.
- Fixed submodule references.

## [1.0.1] - Previous Release
- Initial open source release.
- Restored functionality from 2017 release.
