@echo off
setlocal

set VCPKG_ROOT=C:\vcpkg
if not exist "%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" (
    echo WARNING: vcpkg not found at %VCPKG_ROOT%.
    echo If you encounter CMake configuration errors, please ensure vcpkg is installed and VCPKG_ROOT is set correctly.
) else (
    set CMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake
)

echo Configuring ArrowVortex with CMake...
if defined CMAKE_TOOLCHAIN_FILE (
    cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="%CMAKE_TOOLCHAIN_FILE%" -DCMAKE_BUILD_TYPE=Debug
) else (
    cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
)

if %errorlevel% neq 0 (
    echo ERROR: CMake configuration failed.
    pause
    goto :eof
)

echo Building ArrowVortex...
cmake --build build --config Debug -j

if %errorlevel% neq 0 (
    echo ERROR: Build failed.
    pause
    goto :eof
)

echo Build complete.
echo You can now run the application using start.bat
pause
