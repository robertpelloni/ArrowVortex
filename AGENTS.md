# ARROWVORTEX KNOWLEDGE BASE

**Context:** Rhythm Game Editor (C++/Qt) + ML (PyTorch)
**Parent:** [Workspace Root](../AGENTS.md)
**Master Instructions:** [LLM_INSTRUCTIONS.md](LLM_INSTRUCTIONS.md)

## OVERVIEW
Windows-based simfile editor for rhythm games (StepMania, ITG, osu!). Features automatic chart generation via DDC (Data-Driven Convolution).

## STRUCTURE
```
ArrowVortex/
├── src/
│   ├── Core/        # Engine (Graphics, Input)
│   ├── Editor/      # Application Logic
│   ├── Managers/    # Global State (SimfileMan, TempoMan)
│   └── Dialogs/     # UI Widgets
├── lib/
│   └── ddc/         # ML Component (PyTorch ONLY)
└── scripts/         # Lua Scripts
```

## WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| **ML/Generation** | `lib/ddc/` | **PyTorch ONLY**. No TensorFlow. |
| **Scripting** | `src/Managers/LuaMan.cpp` | Lua API bindings. |
| **UI Dialogs** | `src/Dialogs/` | Custom GuiWidget system. |
| **Parsing** | `src/Simfile/` | SM/SSC/DWI parsing logic. |

## CONVENTIONS
- **Build**: Visual Studio 2022 (Windows x64/x86).
- **Strings**: Use `Vortex::String` wrapper, not `std::string`.
- **Versioning**: MUST sync `VERSION` file AND `src/Version.h`.
- **ML Engine**: `lib/ddc` has migrated to **PyTorch**. Do not introduce TF dependencies.

## ANTI-PATTERNS
- **NO** TensorFlow usage. The environment forbids it.
- **NO** updating vendored libraries in `lib/` (except `ddc`).
- **NO** raw pointer memory management in new code (use `Vector` or smart pointers).
