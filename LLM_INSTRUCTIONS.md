# ArrowVortex LLM Instructions

## Project Overview
ArrowVortex is a C++ simfile editor for rhythm games (StepMania, osu!, etc.). It uses a custom engine (Vortex) and depends on several libraries (FreeType, Lua, libvorbis, etc.) located in `lib/`.

## Build System
*   **IDE**: Visual Studio 2022
*   **Solution**: `build/VisualStudio/ArrowVortex.sln`
*   **Platform**: Windows (x64/x86)

## Codebase Structure
*   **`src/Core`**: Low-level engine (Graphics, Input, Utils).
*   **`src/Editor`**: Application logic, Views, Actions.
*   **`src/Managers`**: Global state managers (SimfileMan, TempoMan, etc.).
*   **`src/Dialogs`**: UI Dialogs (using custom GuiWidget system).
*   **`src/Simfile`**: Data structures and parsing logic.

## Coding Style
*   **Indentation**: Tabs or 4 spaces (consistent with file).
*   **Naming**: CamelCase for classes/functions, `myMemberVariable` for members.
*   **Strings**: Use `String` wrapper (Vortex::String) over `std::string` where possible, though `std::string` is used in newer modules.
*   **Memory**: Manual memory management is common in older parts; newer parts use smart pointers or `Vector` container.

## Versioning
*   **Version File**: `VERSION` (root) contains the current version string (e.g., `v1.3.0`).
*   **Header**: `src/Version.h` defines `ARROWVORTEX_VERSION`. Ensure this matches `VERSION`.
*   **Changelog**: Update `CHANGELOG.md` for every feature or fix.
*   **Protocol**: When bumping version:
    1.  Update `VERSION` file.
    2.  Update `src/Version.h`.
    3.  Update `CHANGELOG.md`.
    4.  Update `DASHBOARD.md`.
    5.  Commit with message "Bump version to vX.Y.Z".

## Submodules / Dependencies
*   **`lib/ddc`**: Git submodule. Use `git submodule update --init --recursive` if needed.
*   **Other `lib/`**: Vendored code (FreeType, Lua, etc.). Do not try to update via git.

## Key Features (Recent)
*   **Lua Scripting**: `src/Managers/LuaMan.cpp`, `scripts/`.
*   **Batch DDC**: `src/Dialogs/BatchDDC.cpp`, `lib/ddc/`.
*   **Visual Sync**: `src/Managers/TempoMan.cpp` (Ripple logic).

## Task Protocol
1.  **Analyze**: Understand the request and check `DASHBOARD.md` / `CHANGELOG.md`.
2.  **Implement**: Modify code, ensuring `src/Version.h` matches `VERSION` if bumping version.
3.  **Document**: Update `CHANGELOG.md` and `DASHBOARD.md` if dependencies change.
4.  **Commit**: Use descriptive commit messages.
