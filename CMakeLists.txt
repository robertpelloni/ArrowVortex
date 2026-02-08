cmake_minimum_required(VERSION 3.30)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

<<<<<<< HEAD
set(CMAKE_CXX_STANDARD 20)
=======
set(CMAKE_CXX_STANDARD 17)
>>>>>>> origin/stdminmax
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(ArrowVortex)

<<<<<<< HEAD
set(CMAKE_CONFIGURATION_TYPES Debug;Release;MinSizeRel)

# clang-tidy might not be available on AppleClang systems by default
find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
if(CLANG_TIDY_EXE)
    if("${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel")
        set(tidy_flags "--fix-errors;--fix-notes")
        set(format_flags "-i")
    else()
        set(tidy_flags "--warnings-as-errors=*")
        set(format_flags "--dry-run")
    endif()
    set(CMAKE_CXX_CLANG_TIDY "clang-tidy;${tidy_flags};-extra-arg=/EHsc;-p=build")
    message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
else()
    message(STATUS "clang-tidy not found, skipping static analysis")
endif()

include_directories("${PROJECT_SOURCE_DIR}/src")

find_package(PkgConfig REQUIRED)

# Some things needed for macOS are available via Homebrew but not vcpkg.
# To support both, we adjust PKG_CONFIG_PATH to include Homebrew paths.
if(APPLE)
    # Homebrew pkg-config paths for macOS
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
        # Apple Silicon
        set(HOMEBREW_PREFIX "/opt/homebrew")
    else()
        # Intel Mac
        set(HOMEBREW_PREFIX "/usr/local")
    endif()
    
    set(ENV{PKG_CONFIG_PATH} "${HOMEBREW_PREFIX}/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
    message(STATUS "Added Homebrew PKG_CONFIG_PATH: ${HOMEBREW_PREFIX}/lib/pkgconfig")
endif()

find_package(aubio CONFIG QUIET)
if(NOT aubio_FOUND)
    pkg_check_modules(aubio REQUIRED IMPORTED_TARGET aubio)
    message(STATUS "Found aubio via pkg-config (likely Homebrew)")
    add_library(Aubio::aubio ALIAS PkgConfig::aubio)
else()
    message(STATUS "Found aubio via CONFIG (likely vcpkg)")
endif()

pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET libavcodec libavformat libavutil libswscale)

find_package(Freetype REQUIRED)

find_package(iir CONFIG QUIET)
if(NOT iir_FOUND)
    pkg_check_modules(iir REQUIRED IMPORTED_TARGET iir)
    message(STATUS "Found iir via pkg-config")
else()
    message(STATUS "Found iir via CONFIG")
endif()

pkg_check_modules(MAD REQUIRED IMPORTED_TARGET mad)

# Without this alias, later references to MAD::MAD could fail
add_library(MAD::MAD ALIAS PkgConfig::MAD)

find_package(Ogg CONFIG QUIET)
if(NOT Ogg_FOUND)
    pkg_check_modules(Ogg REQUIRED IMPORTED_TARGET ogg)
    message(STATUS "Found Ogg via pkg-config")
else()
    message(STATUS "Found Ogg via CONFIG")
endif()

find_package(OpenGL REQUIRED)

find_package(Stb CONFIG QUIET)
if(Stb_FOUND OR TARGET unofficial::stb::stb)
    message(STATUS "Found Stb via CONFIG (vcpkg)")
else()
    find_path(STB_INCLUDE_DIR NAMES stb_image.h PATHS 
        /usr/local/include 
        /opt/homebrew/include
        ${CMAKE_CURRENT_SOURCE_DIR}/external
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party)
    if(STB_INCLUDE_DIR)
        message(STATUS "Found stb headers at: ${STB_INCLUDE_DIR}")
        add_library(stb INTERFACE)
        target_include_directories(stb INTERFACE ${STB_INCLUDE_DIR})
        add_library(unofficial::stb::stb ALIAS stb)
    else()
        message(FATAL_ERROR "Could not find stb headers")
    endif()
endif()

find_package(Vorbis CONFIG QUIET)
if(NOT Vorbis_FOUND)
    pkg_check_modules(Vorbis REQUIRED IMPORTED_TARGET vorbis vorbisfile)
    message(STATUS "Found Vorbis via pkg-config")
else()
    message(STATUS "Found Vorbis via CONFIG")
endif()

=======
set(CMAKE_CONFIGURATION_TYPES Debug Release)

include_directories("${PROJECT_SOURCE_DIR}/src")

find_package(Freetype REQUIRED)
find_package(MAD REQUIRED)
find_package(Ogg CONFIG REQUIRED)
find_package(Vorbis REQUIRED)
find_package(OpenGL REQUIRED)

>>>>>>> origin/stdminmax
add_subdirectory(src/Core)
add_subdirectory(src/Dialogs)
add_subdirectory(src/Editor)
add_subdirectory(src/Managers)
add_subdirectory(src/Simfile)
add_subdirectory(src/System)

install(TARGETS ArrowVortex RUNTIME)
install(DIRECTORY bin/assets DESTINATION bin)
install(DIRECTORY bin/noteskins DESTINATION bin)
install(DIRECTORY bin/settings DESTINATION bin)
<<<<<<< HEAD

file(GLOB_RECURSE path_sources "src/*.cpp" "src/*.hpp" "src/*.c" "src/*.h")
add_custom_target(format ALL)
add_custom_command(
    TARGET format
    POST_BUILD
    COMMAND clang-format ${format_flags} -Werror -style=file ${path_sources}
    USES_TERMINAL
)
add_dependencies(format ArrowVortex)
=======
>>>>>>> origin/stdminmax
