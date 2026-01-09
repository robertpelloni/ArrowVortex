# Roadmap

## v0.4.0 (Current)
*   **DDreamStudio Feature Set**:
    *   Beat Dragging (Ripple/Local)
    *   Advanced Waveform Visualization (Spectrogram, CQT, etc.)
    *   Auto-Sync Tools (BPM Detection, Key Detection)
    *   Practice Mode

## v0.5.0 (Planned)
*   **Audio Engine Upgrade**:
    *   Replace `libmad`/`libvorbis` with `miniaudio` or `RtAudio` for lower latency.
    *   Support for FLAC and Ogg Opus.
*   **Lua Scripting**:
    *   Expose Editor API to Lua for custom plugins and macros.
    *   Port `DancingBot` logic to Lua.

## Future Ideas
*   **Multi-Platform Support**:
    *   CMake build system for Linux/macOS support.
    *   Remove Windows-specific dependencies (e.g., `winmm.lib`).
*   **Cloud Integration**:
    *   Integration with cloud storage for simfile backup.
*   **Modern UI**:
    *   Migrate custom widget system to ImGui or similar?
