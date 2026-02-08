file(GLOB SRC "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB INC "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_library(Editor STATIC ${SRC} ${INC})
<<<<<<< HEAD
target_link_libraries(Editor PRIVATE Core Dialogs MAD::MAD Managers Simfile System Vorbis::vorbisfile Vorbis::vorbis Vorbis::vorbisenc Ogg::ogg)
=======
target_link_libraries(Editor PRIVATE Core Dialogs MAD::MAD Managers Simfile System Vorbis::vorbisfile Vorbis::vorbis Ogg::ogg)
>>>>>>> origin/stdminmax
