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

## Dependencies (Submodules & Vendored Libraries)

The following libraries are included in the `lib/` directory. While they are not currently configured as git submodules, they are essential dependencies.

| Library | Version | Location | Description |
| :--- | :--- | :--- | :--- |
| **FreeType** | 2.10.4 | `lib/freetype` | Font rendering engine. |
| **Gist** | *Unknown* | `lib/gist` | Audio analysis library (Spectrograms, CQT). |
| **libaca** | *Unknown* | `lib/libaca` | Audio Content Analysis library. |
| **libmad** | 0.15.1b | `lib/libmad` | MPEG audio decoder. |
| **libvorbis** | 1.3.7 | `lib/libvorbis` | Ogg Vorbis audio codec. |
| **Lua** | 5.4.3 | `lib/lua` | Scripting language. |
| **ddc** | *Submodule* | `lib/ddc` | Dance Dance Convolution (Auto-charting). |

## Build Information

*   **Version**: 1.1.0
*   **Build System**: Visual Studio 2022 (`build/VisualStudio/ArrowVortex.sln`)
*   **Platform**: Windows (x64/x86)

## Recent Updates

*   **DDreamStudio Features**: Integrated advanced audio analysis, visual sync, and auto-sync tools.
*   **Auto-Sync Tools**: Refined `AUTO_SYNC_SONG` (Replace All) and `QUANTIZE_TO_AUDIO` (Note Snapping).
*   **Batch DDC**: Integrated `ddc` (Dance Dance Convolution) submodule and added a Batch Generation UI.
*   **Practice Mode**: Implemented gameplay logic with custom timing windows and visual feedback.
*   **Scroll Cursor Effect**: Implemented receptor pulse toggle.
*   **FPS Counter**: Added FPS display with toggle preference.
*   **osu! Support**: Added support for loading `.osu` files.
*   **Formatting**: Codebase formatting updates (pending integration).
