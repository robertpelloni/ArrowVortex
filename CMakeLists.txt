cmake_minimum_required(VERSION 3.30)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-extra-arg=/EHsc;-p=build")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(ArrowVortex)

set(CMAKE_CONFIGURATION_TYPES Debug Release)

include_directories("${PROJECT_SOURCE_DIR}/src")

find_package(Freetype REQUIRED)
find_package(MAD REQUIRED)
find_package(Ogg CONFIG REQUIRED)
find_package(Vorbis REQUIRED)
find_package(OpenGL REQUIRED)

file(GLOB SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/*/*.cpp")
add_executable(ArrowVortex WIN32
	${SRC}
	"${CMAKE_CURRENT_SOURCE_DIR}/src/System/ArrowVortex.rc"
)
add_custom_command(
	TARGET ArrowVortex POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${PROJECT_SOURCE_DIR}/bin
		$<TARGET_FILE_DIR:ArrowVortex>)
target_link_libraries(ArrowVortex PRIVATE
	Freetype::Freetype
	OpenGL::GL
	MAD::MAD
	Vorbis::vorbisfile
	Vorbis::vorbis
	winmm
	Ogg::ogg
)

install(TARGETS ArrowVortex RUNTIME)
install(DIRECTORY bin/assets DESTINATION bin)
install(DIRECTORY bin/noteskins DESTINATION bin)
install(DIRECTORY bin/settings DESTINATION bin)
