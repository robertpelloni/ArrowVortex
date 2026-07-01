@echo off
setlocal

cd /d "%~dp0"

if exist "bin\ArrowVortex_debug.exe" (
    echo Starting ArrowVortex (Debug)...
    start "" "bin\ArrowVortex_debug.exe"
    goto :eof
)

if exist "bin\ArrowVortex.exe" (
    echo Starting ArrowVortex (Release)...
    start "" "bin\ArrowVortex.exe"
    goto :eof
)

echo ERROR: ArrowVortex executable not found in the 'bin' directory.
echo Please run 'build.bat' or compile the project via CMake first.
pause
