# DDreamStudio Feature Set for ArrowVortex

This document outlines the new features integrated into ArrowVortex, inspired by the workflow and capabilities of DDreamStudio. These features focus on advanced audio visualization, precise tempo synchronization, and enhanced charting tools.

## 1. Beat Dragging (Manual Sync)

The "Beat Dragging" feature allows charters to manually adjust the tempo map by directly interacting with the visual beat lines on the note field.

### Usage
*   **Mouse Drag:**
    *   **Action:** Click and drag any beat line (measure, beat, or sub-beat) in the editor view.
    *   **Behavior:**
        *   **Standard Drag:** Adjusts the local timing (warps the grid around the beat).
        *   **Ripple Drag (`Shift` + Drag):** Adjusts the tempo of the *previous* segment, effectively shifting the dragged beat and all subsequent beats in time. This is the primary method for syncing drifting live music.
    *   **Visuals:** A temporary "Tweak" state is shown during the drag operation. The change is committed to the history upon mouse release.

*   **Keyboard Nudge:**
    *   **Shortcuts:** Bindable in `Options > Shortcuts` (Defaults: `Shift + Left/Right` or custom).
    *   **Action:** `Nudge Beat Forward` / `Nudge Beat Backward`.
    *   **Behavior:** Shifts the current beat by a small amount. If `Preferences > Nudge Based on Zoom` is enabled, the shift amount scales with the current zoom level for pixel-perfect precision.

## 2. Advanced Waveform Visualization

ArrowVortex now includes a suite of advanced audio visualization modes powered by the **Gist** library, allowing for detailed analysis of the audio spectrum.

### Usage
*   **Access:** `View > Waveform...` or `Ctrl+W` (default) to toggle. Settings via `View > Waveform Settings...`.
*   **Modes:**
    *   **RGB (3-Band):** Visualizes Low, Mid, and High frequencies as Red, Green, and Blue channels.
    *   **Spectrogram:** Full frequency spectrum heatmap.
    *   **CQT (Constant-Q Transform):** Logarithmic frequency mapping aligned to musical notes.
    *   **Chromagram:** Energy distribution across the 12 musical pitch classes.
    *   **Pitch:** Fundamental frequency estimation (YIN algorithm).
    *   **HPSS (Harmonic-Percussive Source Separation):**
        *   **Harmonic:** Isolates tonal elements (vocals, guitars).
        *   **Percussive:** Isolates transient elements (drums).
    *   **Novelty / Flux:** Visualizes change in spectrum, useful for detecting onset "attacks".
    *   **Tempogram:** Visualizes tempo probability over time.

### Settings
*   **Gain:** Adjusts the visual intensity of the waveform.
*   **Threshold:** Sets the cutoff for Onset detection visualization.
*   **Color Mapping:** Customizable RGB crossovers for the 3-Band mode.

## 3. Auto-Sync Tools

New automated tools assist in establishing the initial tempo map and correcting drift.

### Usage
*   **Auto-Sync Song:**
    *   **Location:** `Tempo > Auto-sync song (Global)`
    *   **Function:** Analyzes the entire audio track to detect the average BPM and the first beat offset. Replaces the entire timing map with a single constant BPM segment. Use this as a starting point.

*   **Quantize to Audio:**
    *   **Location:** `Tempo > Quantize to audio (Selection)`
    *   **Function:** Aligns selected *notes* to the nearest detected audio onsets. Useful for snapping notes to a loose performance.

*   **Warp Grid to Audio:**
    *   **Location:** `Tempo > Warp grid to audio (Selection)`
    *   **Function:** Iterates through the selected time range and aligns the *grid lines* (BPM/Warps) to the nearest audio onsets. This automates the "Beat Dragging" process for a selected section.

*   **Place Beat at Playhead:**
    *   **Location:** `Tempo > Place beat at playhead`
    *   **Function:** Instantly snaps the nearest grid line to the current playback position (or cursor). Useful for "Tap Syncing" while listening.

*   **Key Detection:**
    *   **Location:** `Tempo > Detect Key`
    *   **Function:** Analyzes the audio to estimate the musical key (e.g., "C Major", "F# Minor") using `Gist`-based Chromagram analysis.

## 4. Practice Mode

A gameplay-simulation mode for verifying timing without editing.

### Usage
*   **Toggle:** `Edit > Enable practice mode` (or shortcut).
*   **Behavior:** When enabled, pressing column keys (1-4, etc.) will **not** place notes. Instead, the editor will calculate the timing difference between your press and the nearest note, displaying a judgment (Marvelous, Perfect, Great, etc.) and offset in milliseconds on the HUD.
*   **Configuration:** Timing windows can be adjusted in `Edit > Preferences`.

## 5. UI & Preferences

Various workflow enhancements to match DDreamStudio's usability.

*   **Scroll Cursor Effect:** (`Preferences`) Keeps the chart static while the cursor/receptors move across the screen (Page Mode).
*   **Edit One Layer:** (`Preferences`) Prevents overwriting notes of a different type (e.g., placing a Tap won't delete a Mine).
*   **Paste Overwrites:** (`Preferences`) Configures whether pasting notes clears the underlying area.
*   **Select Pasted:** (`Preferences`) Automatically selects notes after pasting.
*   **Lyrics Editor:** (`Chart > Edit Lyrics...`) A dedicated dialog for adding and timing lyrics.
*   **Background Changes:** (`View > Background > Edit changes...`) Manage background image/video transitions.
*   **Chart Statistics:** (`Chart > Statistics...`) Detailed breakdown of note counts and density graphs.

---

## Technical Implementation Notes

### Libraries
*   **Gist:** Used for all frequency-domain analysis (Spectrogram, CQT, Pitch, MFCC) and Key Detection.
*   **Aubio (Embedded):** Ported logic used for robust Onset Detection.

### Compilation
The project configuration (`ArrowVortex.vcxproj`) has been updated to include all new source files. No additional external setup is required beyond the standard dependency installation.

### Key Source Files
*   `src/Editor/Waveform.cpp`: Visualization rendering pipeline.
*   `src/Managers/TempoMan.cpp`: Beat dragging and sync logic.
*   `src/Editor/Action.cpp`: Implementation of new menu commands.
*   `src/Editor/AnalyzeKey.cpp`: Implementation of Key Detection (via Gist).
*   `src/Editor/Editing.cpp`: Practice mode input interception.
