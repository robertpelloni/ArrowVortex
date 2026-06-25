cmake_minimum_required(VERSION 3.28)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(ArrowVortex)

set(CMAKE_CONFIGURATION_TYPES Debug;Release;MinSizeRel)

include_directories("${PROJECT_SOURCE_DIR}/src")

find_package(PkgConfig REQUIRED)

if(APPLE)
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
        set(HOMEBREW_PREFIX "/opt/homebrew")
    else()
        set(HOMEBREW_PREFIX "/usr/local")
    endif()
    
    set(ENV{PKG_CONFIG_PATH} "${HOMEBREW_PREFIX}/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
    message(STATUS "Added Homebrew PKG_CONFIG_PATH: ${HOMEBREW_PREFIX}/lib/pkgconfig")
endif()

find_package(aubio CONFIG QUIET)
if(NOT aubio_FOUND)
    pkg_check_modules(aubio REQUIRED IMPORTED_TARGET aubio)
    add_library(Aubio::aubio ALIAS PkgConfig::aubio)
endif()

pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET libavcodec libavformat libavutil libswscale)
find_package(Freetype REQUIRED)

find_package(iir CONFIG QUIET)
if(NOT iir_FOUND)
    pkg_check_modules(iir REQUIRED IMPORTED_TARGET iir)
endif()

pkg_check_modules(MAD REQUIRED IMPORTED_TARGET mad)
add_library(MAD::MAD ALIAS PkgConfig::MAD)

find_package(Ogg CONFIG QUIET)
if(NOT Ogg_FOUND)
    pkg_check_modules(Ogg REQUIRED IMPORTED_TARGET ogg)
endif()

find_package(OpenGL REQUIRED)

find_package(Stb CONFIG QUIET)
if(NOT Stb_FOUND AND NOT TARGET unofficial::stb::stb)
    find_path(STB_INCLUDE_DIR NAMES stb_image.h PATHS 
        /usr/local/include 
        /opt/homebrew/include
        ${CMAKE_CURRENT_SOURCE_DIR}/external
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party)
    if(STB_INCLUDE_DIR)
        add_library(stb INTERFACE)
        target_include_directories(stb INTERFACE ${STB_INCLUDE_DIR})
        add_library(unofficial::stb::stb ALIAS stb)
    endif()
endif()

find_package(Vorbis CONFIG QUIET)
if(NOT Vorbis_FOUND)
    pkg_check_modules(Vorbis REQUIRED IMPORTED_TARGET vorbis vorbisfile)
endif()

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
install(DIRECTORY bin/models DESTINATION bin)

file(GLOB_RECURSE path_sources "src/*.cpp" "src/*.hpp" "src/*.c" "src/*.h")
add_custom_target(format ALL)
add_custom_command(
    TARGET format
    POST_BUILD
    COMMAND clang-format ${format_flags} -Werror -style=file ${path_sources}
    USES_TERMINAL
)
add_dependencies(format ArrowVortex)
