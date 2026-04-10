@echo off
echo Building arrowvortex...
cmake -B build -S .
cmake --build build --config Release
echo Build complete.
pause