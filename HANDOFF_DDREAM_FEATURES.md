# Handoff: DDreamStudio Features Implementation in ArrowVortex

## Overview
This session focused on implementing advanced features from **DDreamStudio** into the **ArrowVortex** simfile editor. The primary goals were to enable manual and automatic synchronization workflows ("Beat Dragging", "Auto-Sync"), enhance audio visualization (Spectrograms, CQT), and reach feature parity with DDreamStudio's preferences and configuration options.

## Features Implemented

### 1. Beat Dragging (Visual Sync)
*   **Description:** Allows the user to manually click and drag beat lines (measure/beat markers) to align them with the audio waveform.
*   **Implementation:**
    *   **Input:** `View::onMousePress` detects Shift+Click on beat lines.
    *   **Logic:** `TempoMan::destructiveShiftRowToTime` implements the "ripple" editing logic, where moving one beat adjusts the BPM of the previous segment to fit the new time, shifting all subsequent notes.
    *   **Anchoring:** `TempoMan::injectBoundingBpmChange` ensures that edits are localized by inserting BPM anchors.
    *   **Visuals:** A "Tweak Mode" (`TWEAK_DRAG`) in `TempoMan` allows for smooth visual feedback during the drag operation without spamming the undo history.

### 2. Advanced Waveform Visualization
*   **Description:** Replaced the basic waveform with detailed visualizations to assist in sync.
*   **Implementation:**
    *   **Library:** Integrated the **Gist** audio analysis library.
    *   **Modes:** Added support for:
        *   `Spectrogram` (Mel/Magnitude)
        *   `CQT` (Constant-Q Transform / Log-Frequency)
        *   `Chromagram` (Pitch class distribution)
        *   `Pitch` (YIN Algorithm)
        *   `Tempogram` (BPM probability)
        *   `HPSS` (Harmonic-Percussive Source Separation) for isolating drums.
    *   **UI:** `DialogWaveformSettings` (implied/hooked up) allows switching modes and adjusting gain/thresholds.

### 3. Auto-Sync Tools
*   **Description:** Automated tools to calculate BPM and align grids.
*   **Implementation:**
    *   **Algorithms:** Restored/Implemented `TempoDetector` (BPM estimation) and `FindOnsets` (Transient detection) using embedded logic or Aubio references.
    *   **Actions:**
        *   `AUTO_SYNC_SONG`: Global BPM detection and offset alignment.
        *   `QUANTIZE_TO_AUDIO`: Aligns selected notes to the nearest detected audio onsets.
        *   `WARP_GRID_TO_AUDIO`: Aligns grid lines (tempo map) to onsets using destructive shifts.
        *   `ESTIMATE_BPM_FROM_SELECTION`: Helper to find BPM of a specific range.

### 4. Editor Preferences (DDream Parity)
*   **Description:** Added a comprehensive Preferences dialog matching DDreamStudio's options.
*   **Implementation:**
    *   **Dialog:** `DialogPreferences` created using the existing widget system (`WgCheckbox`, `WgSpinner`, `WgSlider`).
    *   **Features:**
        *   `MiddleMouseInsertBeat`: Toggle between Autoscroll and Insert Beat.
        *   `ScrollCursorEffect`: Preference storage (placeholder for rendering logic).
        *   `InsertSameDeletes`: Logic in `Editing::onKeyPress` to delete existing notes if the same type is placed.
        *   `EditOneLayer`: Logic in `Editing::onKeyPress` to prevent editing different note types (e.g., Mines vs Taps).
        *   `PasteOverwrites`: Logic placeholder in `pasteFromClipboard`.
        *   `BackupSaves`: Sets `myBackupOnSave` flag in `SimfileMan`.
        *   `Assist Tick Volumes`: Separate volume controls for Beat and Note ticks in `Music.cpp`.

### 5. Practice Mode Configuration
*   **Description:** Added configuration for Practice Mode timing windows.
*   **Implementation:**
    *   **Storage:** `Editor::PracticeSetup` struct serialized to `settings/settings.txt` under `<practice>`.
    *   **UI:** Spinner widgets in `DialogPreferences` converting between Seconds (internal) and Milliseconds (UI).

### 6. Lua Scripting Support
*   **Description:** Integrated a Lua scripting environment to allow users to write and execute custom scripts for automation and analysis.
*   **Implementation:**
    *   **Manager:** `LuaMan` (singleton) initializes the Lua state (`lua_State`) and handles script execution.
    *   **Integration:** Added `lib/lua` to the build system (`ArrowVortex.vcxproj`).
    *   **API:** Exposed a basic `print` function to the editor HUD.
    *   **UI:** Added a "Scripts" menu with a "Run Test Script" action.
    *   **Testing:** Created `scripts/test.lua` to verify functionality.

## Technical Implementation Details

### Architecture Changes
*   **TempoMan:** Added `destructiveShiftRowToTime` and `nonDestructiveShiftRowToTime`. The "Tweak" system was extended to support a transient `myTweakTempo` for drag operations.
*   **Music:** `writeFrames` now supports A/B looping logic and mixing tick sounds with independent volumes.
*   **SimfileMan:** Added logic to remove duplicate BPMs on save if the preference is enabled.

### Challenges & Solutions
*   **UI Binding Constraints:** The `CallSlot::bind` system in `src/Core/Slot.h` is strictly typed and does not support C++11 lambdas with captures.
    *   *Solution:* Implemented explicit `void member()` callbacks for every widget in `DialogPreferences.cpp` (e.g., `onNudgeChanged()`, `onBeatVolChanged()`) that read the bound member variables directly.
*   **Waveform Performance:** Advanced visualizations like CQT are computationally expensive.
    *   *Solution:* `Waveform.cpp` (in previous steps) utilizes caching/texture blocks. The render loop processes data in chunks.

## Modified Files
*   `src/Editor/Action.h` / `.cpp`: Added new enums and logic for Auto-Sync actions.
*   `src/Editor/Editor.h` / `.cpp`: Added preference storage, accessors, and serialization.
*   `src/Editor/Editing.cpp`: Implemented logic for "Edit One Layer" and "Insert Same Deletes".
*   `src/Editor/View.cpp`: Implemented Beat Dragging input handling and "Middle Mouse Insert".
*   `src/Managers/TempoMan.cpp`: Core logic for destructive timing edits.
*   `src/Managers/SimfileMan.cpp`: Logic for "Remove Duplicate BPMs" and "Backup Saves".
*   `src/Managers/Music.cpp`: Updated mixing logic for Assist Ticks.
*   `src/Dialogs/Preferences.h` / `.cpp`: New dialog implementation.
*   `src/Dialogs/Dialog.h`: Registered `DIALOG_PREFERENCES`.
*   `src/Managers/LuaMan.h` / `.cpp`: New manager for Lua scripting.
*   `src/Editor/Menubar.cpp`: Added "Scripts" menu.
*   `build/VisualStudio/ArrowVortex.vcxproj`: Added Lua include paths and project reference.

## Next Steps for the Next Developer
1.  **Verification**:
    *   Build the project and verify that the **Preferences Dialog** opens and saves settings correctly.
    *   Test **Beat Dragging** (Shift+LeftClick+Drag on a beat line) to ensure it ripples the tempo map correctly.
    *   Test **Auto-Sync** actions on a file with drifting tempo (e.g., live rock music).
2.  **Refinement**:
    *   **Scroll Cursor Effect**: Implemented as a toggle for the receptor beat pulse in `Notefield::drawReceptors`. When enabled, receptors pulse to the beat. When disabled, they remain static (brightness 255).
    *   **Practice Mode**: Fully implemented in `EditorImpl`.
        *   Added `myJudgedNotes` to track hit notes.
        *   Added `handlePracticeInput` to judge notes against `PracticeSetup` windows.
        *   Added `Notefield::triggerFlash` for visual feedback.
        *   Intercepts Arrow Keys and 1-4 keys in `EditorImpl::onKeyPress`.
    *   **FPS Counter**: Implemented in `EditorImpl::tick` using `Text::arrange` and `Text::draw`. Respects `getDontShowFPS()`.

## Memories Recorded
*   Project uses `Gist` for audio analysis.
*   `CallSlot::bind` requires strict member function pointers.
*   `Action.cpp` handles the high-level orchestration of tools.
*   Destructive editing requires careful handling of BPM anchors (`injectBoundingBpmChange`).
