# FindVorbis.cmake

find_path(VORBIS_INCLUDE_DIR vorbis/vorbisfile.h)
find_library(VORBIS_LIBRARIES NAMES vorbisfile vorbis)

# Check if both the include directory and library were found
if (VORBIS_INCLUDE_DIR AND VORBIS_LIBRARIES)
    set(VORBIS_FOUND TRUE)
else()
    set(VORBIS_FOUND FALSE)
endif()

# Provide feedback to the user
if (VORBIS_FOUND)
    message(STATUS "Found Vorbis: ${VORBIS_INCLUDE_DIR} ${VORBIS_LIBRARIES}")
else()
    message(WARNING "Vorbis not found.")
endif()
