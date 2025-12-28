# Project Dashboard

## Project Structure

The ArrowVortex project is organized as follows:

*   **`bin/`**: Contains runtime assets, noteskins, and release binaries.
*   **`build/`**: Contains build scripts and project files (Visual Studio).
*   **`lib/`**: Contains third-party libraries and dependencies.
*   **`src/`**: Contains the source code for ArrowVortex.
    *   **`Core/`**: Core engine components (Graphics, Input, Utils).
    *   **`Dialogs/`**: UI Dialog implementations.
    *   **`Editor/`**: Main editor logic and views.
    *   **`Managers/`**: State managers (Simfile, Chart, Tempo).
    *   **`Simfile/`**: Simfile parsing and data structures.
    *   **`System/`**: OS-specific implementations (Windows).

## Submodules & Dependencies

| Library | Version | Location | Description |
| :--- | :--- | :--- | :--- |
| **FreeType** | 2.10.4 | `lib/freetype` | Font rendering engine. |
| **Gist** | *Unknown* | `lib/gist` | Audio analysis library (Spectrograms, CQT). |
| **libaca** | *Unknown* | `lib/libaca` | Audio Content Analysis library. |
| **libmad** | 0.15.1b | `lib/libmad` | MPEG audio decoder. |
| **libvorbis** | 1.3.7 | `lib/libvorbis` | Ogg Vorbis audio codec. |
| **Lua** | 5.4.3 | `lib/lua` | Scripting language. |
| **ddc** | v1.0-172-gc8caffe (2025-12-27) | `lib/ddc` | Dance Dance Convolution (Auto-charting). |
| **ddc_onset** | *Submodule* | `lib/ddc/ddc_onset` | Onset detection model for DDC. |
| **ffr-difficulty** | *Submodule* | `lib/ddc/ffr-difficulty-model` | Difficulty rating model for DDC. |

## Build Information

*   **Version**: v1.3.1
*   **Build System**:
    *   **Windows**: Visual Studio 2022 (`build/VisualStudio/ArrowVortex.sln`)
    *   **Cross-Platform**: CMake (`CMakeLists.txt`)
*   **Platform**: Windows (x64/x86), Linux (Experimental)

## Feature Roadmap

### Completed Features (v1.3.0)
- [x] **Lua Scripting Engine**: Integrated Lua 5.x, `LuaMan`, and `Scripts` menu.
- [x] **Visual Sync (Beat Dragging)**: "Ripple" editing logic for waveform alignment.
- [x] **Batch DDC Generation**: Integrated `ddc` submodule and UI.
- [x] **Practice Mode**: Gameplay logic, timing windows, visual feedback.
- [x] **Auto-Sync Refinement**: Improved `AUTO_SYNC_SONG` and `QUANTIZE_TO_AUDIO`.
- [x] **FPS Counter**: Added FPS display.
- [x] **osu! Support**: Basic `.osu` file loading.
- [x] **Multi-Platform Build**: Added `CMakeLists.txt` for Linux/macOS support.
- [x] **Advanced Waveform Analysis**: Verified implementation of CQT, HPSS, Chromagram, and multiple visualization modes.
- [x] **Theme Support**: Implemented a full Theme Editor (`View -> Theme Editor...`) allowing runtime customization of all UI colors, with XML save/load support.

### Planned Features
- [ ] **Undo/Redo for Batch DDC**: Currently Batch DDC is a destructive/external process.
- [ ] **Lua API Expansion**: Expose more internal systems (Graphics, Audio) to Lua.

## CMake Build Guide (Linux/macOS)

1.  **Install Dependencies**:
    *   Ubuntu: `sudo apt install cmake build-essential libfreetype6-dev liblua5.3-dev libvorbis-dev libmad0-dev libgl1-mesa-dev`
2.  **Build**:
    ```bash
    mkdir build && cd build
    cmake ..
    make
    ```
3.  **Run**: `./bin/ArrowVortex`

## DDC Setup Guide

1.  **Build**: Open `build/VisualStudio/ArrowVortex.sln` and build the solution.
2.  **Python Setup**:
    *   Install Python 3.x.
    *   Install dependencies: `pip install -r lib/ddc/requirements.txt`.
    *   Configure Python Path in ArrowVortex (`Edit -> Preferences`).
3.  **DDC Models**:
    *   **Download Data**: Run `lib/ddc/download_data.ps1` (PowerShell) to fetch training data.
    *   **Train Models**: Run `python lib/ddc/scripts/train_all.py <packs_dir> <work_dir>` to generate models.
    *   **Configure**: Point the Batch DDC dialog to the generated models directory.

## Batch DDC Generation

The Batch DDC Generation tool (`File -> Batch DDC Generation...`) allows you to automatically generate charts for multiple songs or entire folders.

### Features
*   **Batch Processing**: Add individual audio files or entire folders.
*   **Recursive Search**: Automatically finds all `.mp3`, `.ogg`, and `.wav` files in added folders.
*   **Difficulty Rating**: Uses the FFR Difficulty Model to estimate chart difficulty.
*   **Custom Models**: Specify custom paths for DDC and FFR models.

### Usage
1.  Open `File -> Batch DDC Generation...`.
2.  Click **Add Files...** or **Add Folder...** to select input audio.
3.  Set the **Output Directory** for the generated simfiles.
4.  Set the **DDC Models Directory** (e.g., `lib/ddc/models`).
5.  Set the **FFR Models Directory** (e.g., `lib/ddc/ffr_models`).
6.  Click **GENERATE CHARTS**.
7.  Check the log output for progress and errors.
