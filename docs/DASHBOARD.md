# Project Dashboard

## Project Structure

### Source Code (`src/`)
*   **Core/**: Fundamental systems (Graphics, Input, String utilities, Widgets).
*   **Editor/**: Main application logic, including the Editor loop (`Editor.cpp`), Rendering (`View.cpp`, `Notefield.cpp`), Audio Analysis (`Waveform.cpp`, `AnalyzeKey.cpp`), and Actions (`Action.cpp`).
*   **Managers/**: Singletons managing global state (`TempoMan`, `NoteskinMan`, `SimfileMan`).
*   **Simfile/**: Data structures for simfiles (`Note`, `Chart`, `TimingData`).
*   **Dialogs/**: UI dialog implementations.
*   **System/**: OS-specific implementations (File IO, Threading).

### Libraries (`lib/`)
External dependencies are located here. Note that `libaca` is currently excluded from the build due to missing internal headers.

*   **Gist**: Audio analysis library (Spectrograms, Pitch, MFCC). Used for advanced visualization and Key Detection.
*   **FreeType**: Font rendering.
*   **libmad**: MP3 decoding.
*   **libvorbis**: Ogg Vorbis decoding.
*   **libaca**: (Disabled) Audio Content Analysis.

### Documentation (`docs/`)
*   **DDREAM_FEATURES.md**: User guide and technical notes for DDreamStudio-inspired features.
*   **DASHBOARD.md**: This file.

## Build Information

*   **System**: Visual Studio Solution (`build/VisualStudio/ArrowVortex.sln`).
*   **Configuration**: Release/Debug, x86/x64.
*   **Dependencies**: All libraries are statically linked or included as source.

## Submodule Status

*   **Management**: Manual (No `.gitmodules` found in root).
*   **Status**:
    *   `Gist`: Active. Used for Waveform filters.
    *   `Aubio` (Embedded in `src/Editor`): Active. Used for Onset Detection.
    *   `libaca`: Inactive/Removed from build.

## Versioning

*   **Current Version**: (See `CHANGELOG.md`)
