cmake_minimum_required(VERSION 3.10)

project(ArrowVortex VERSION 1.3.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Sources
file(GLOB_RECURSE AV_SOURCES "src/*.cpp")
file(GLOB_RECURSE AV_HEADERS "src/*.h")

# Gist (Audio Analysis)
file(GLOB GIST_SOURCES "lib/gist/src/*.cpp")
file(GLOB KISS_FFT_SOURCES "lib/gist/libs/kiss_fft130/*.c")

# LibACA (Audio Content Analysis)
file(GLOB LIBACA_SOURCES "lib/libaca/src/*.cpp")

# Include Directories
include_directories(
    src
    lib
    lib/gist/src
    lib/gist/libs/kiss_fft130
    lib/libaca/inc
)

# Definitions
add_definitions(-DUSE_KISS_FFT)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE)
endif()

# Dependencies
find_package(OpenGL REQUIRED)
find_package(Freetype REQUIRED)
find_package(Lua REQUIRED)

# LibVorbis and LibMad are less standard in CMake, might need custom find modules or pkg-config
find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(VORBIS REQUIRED vorbis vorbisfile ogg)
    pkg_check_modules(MAD REQUIRED mad)
endif()

# Executable
add_executable(ArrowVortex
    ${AV_SOURCES}
    ${AV_HEADERS}
    ${GIST_SOURCES}
    ${KISS_FFT_SOURCES}
    ${LIBACA_SOURCES}
)

# Linking
target_link_libraries(ArrowVortex PRIVATE
    OpenGL::GL
    Freetype::Freetype
    ${LUA_LIBRARIES}
)

if(VORBIS_FOUND)
    target_link_libraries(ArrowVortex PRIVATE ${VORBIS_LIBRARIES})
    target_include_directories(ArrowVortex PRIVATE ${VORBIS_INCLUDE_DIRS})
endif()

if(MAD_FOUND)
    target_link_libraries(ArrowVortex PRIVATE ${MAD_LIBRARIES})
    target_include_directories(ArrowVortex PRIVATE ${MAD_INCLUDE_DIRS})
endif()

if(WIN32)
    target_link_libraries(ArrowVortex PRIVATE winmm)
endif()

# Installation
install(TARGETS ArrowVortex DESTINATION bin)
install(DIRECTORY bin/assets DESTINATION bin)
install(DIRECTORY bin/noteskins DESTINATION bin)
install(DIRECTORY bin/models DESTINATION bin)
