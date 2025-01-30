# FindSDL2_image.cmake
find_package(SDL2 REQUIRED)

# Look for SDL2_image header files
find_path(SDL2_IMAGE_INCLUDE_DIR SDL2/SDL_image.h)

# Look for the SDL2_image library
find_library(SDL2_IMAGE_LIBRARY NAMES SDL2_image)

# Check if both the include directory and library were found
if (SDL2_IMAGE_INCLUDE_DIR AND SDL2_IMAGE_LIBRARY)
    set(SDL2_IMAGE_FOUND TRUE)
else()
    set(SDL2_IMAGE_FOUND FALSE)
endif()

# Provide feedback to the user
if (SDL2_IMAGE_FOUND)
    message(STATUS "Found SDL2_image: ${SDL2_IMAGE_INCLUDE_DIR} ${SDL2_IMAGE_LIBRARY}")
else()
    message(WARNING "SDL2_image not found.")
endif()
