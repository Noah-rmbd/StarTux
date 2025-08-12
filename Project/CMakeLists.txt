cmake_minimum_required(VERSION 3.10...3.27)  # Version moderne avec plage de compatibilité
project(OpenGLProgram)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configuration pour libsndfile (solution alternative)
find_path(SNDFILE_INCLUDE_DIR sndfile.h)
find_library(SNDFILE_LIBRARY NAMES sndfile)

if(NOT SNDFILE_INCLUDE_DIR OR NOT SNDFILE_LIBRARY)
    message(FATAL_ERROR "libsndfile not found. Install with: sudo dnf install libsndfile-devel")
endif()

# Find other packages
find_package(assimp REQUIRED)
find_package(Freetype REQUIRED)
find_package(OpenAL REQUIRED)

# Set directories
set(SRC_DIR src)
set(INCLUDE_DIR include)
set(EXTERNAL_DIR external)
set(GLEW_DIR ${EXTERNAL_DIR}/glew-cmake)
set(GLFW_DIR ${EXTERNAL_DIR}/glfw)
set(GLM_DIR ${EXTERNAL_DIR}/glm)

# Include directories
include_directories(
    ${INCLUDE_DIR}
    ${GLEW_DIR}/include
    ${GLFW_DIR}/include
    ${GLM_DIR}
    ${ASSIMP_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS}
    ${OPENAL_INCLUDE_DIR}
    ${SNDFILE_INCLUDE_DIR}
)

# Source files
set(SOURCES
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/node.cpp
    ${SRC_DIR}/shapes/shape.cpp
    ${SRC_DIR}/graphics/shader.cpp
    ${SRC_DIR}/viewer.cpp
    ${SRC_DIR}/game/camera.cpp
    ${SRC_DIR}/game/game.cpp
    ${SRC_DIR}/game/player.cpp
    ${SRC_DIR}/game/light_projectile.cpp
    ${SRC_DIR}/game/projectile.cpp
    ${SRC_DIR}/game/asteroid.cpp
    ${SRC_DIR}/game/explosion.cpp
    ${SRC_DIR}/game/ring.cpp
    ${SRC_DIR}/game/statistics.cpp
    ${SRC_DIR}/game/missions.cpp
    ${SRC_DIR}/shapes/shape_model.cpp
    ${SRC_DIR}/shapes/sphere.cpp
    ${SRC_DIR}/graphics/texture.cpp
    ${SRC_DIR}/shapes/textured_sphere.cpp
    ${SRC_DIR}/shapes/lighting_sphere.cpp
    ${SRC_DIR}/shapes/transparent_sphere.cpp
    ${SRC_DIR}/interfaces/interface.cpp
    ${SRC_DIR}/interfaces/startup_screen.cpp
    ${SRC_DIR}/interfaces/hud.cpp
    ${SRC_DIR}/audio_manager.cpp
)

# Shader and resources directories as compile-time definitions
add_compile_definitions(SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders/")
add_compile_definitions(TEXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/textures/")
add_compile_definitions(RESSOURCES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/ressources/")
add_compile_definitions(AUDIO_DIR="${CMAKE_CURRENT_SOURCE_DIR}/audio/")

# Add external subdirectories
add_subdirectory(${GLEW_DIR})
add_subdirectory(${GLFW_DIR})

# Define executable
add_executable(opengl_program ${SOURCES})
target_link_libraries(opengl_program ${LIBS})
target_link_libraries(opengl_program ${ASSIMP_LIBRARIES})
target_link_libraries(opengl_program ${FREETYPE_LIBRARIES})
target_link_libraries(opengl_program ${OPENAL_LIBRARY})

# Modern target-based includes
target_include_directories(opengl_program PRIVATE
    ${INCLUDE_DIR}
    ${INCLUDE_DIR}/game
    ${INCLUDE_DIR}/graphics
    ${INCLUDE_DIR}/interfaces
    ${INCLUDE_DIR}/shapes
    ${GLEW_DIR}/include
    ${GLM_DIR}
    ${ASSIMP_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS}
    ${SNDFILE_INCLUDE_DIR}  # Ajout des includes de libsndfile
)
# Chercher libsndfile de manière explicite
find_path(SNDFILE_INCLUDE_DIR sndfile.h HINTS /usr/include /usr/local/include)
find_library(SNDFILE_LIBRARY NAMES sndfile HINTS /usr/lib /usr/local/lib)

if(NOT SNDFILE_INCLUDE_DIR OR NOT SNDFILE_LIBRARY)
    message(FATAL_ERROR "libsndfile not found. Install with: sudo dnf install libsndfile-devel")
else()
    message(STATUS "Found libsndfile: ${SNDFILE_LIBRARY}")
    include_directories(${SNDFILE_INCLUDE_DIR})
endif()
# Link libraries - version moderne et organisée
if(APPLE)
    # macOS specific linking
    target_link_libraries(opengl_program
        libglew_static
        glfw
        OpenAL::OpenAL
        ${SNDFILE_LIBRARY}
        ${ASSIMP_LIBRARIES}
        ${FREETYPE_LIBRARIES}
    )
    
    # Add macOS frameworks explicitly
    find_library(OPENGL_LIBRARY OpenGL)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COREVIDEO_LIBRARY CoreVideo)
    target_link_libraries(opengl_program ${OPENGL_LIBRARY} ${COCOA_LIBRARY} ${IOKIT_LIBRARY} ${COREVIDEO_LIBRARY})
else()
    # Non-macOS linking
    target_link_libraries(opengl_program
        libglew_static
        glfw
        OpenAL::OpenAL
        ${SNDFILE_LIBRARY}
        ${ASSIMP_LIBRARIES}
        ${FREETYPE_LIBRARIES}
    )
endif()

# Custom run target
add_custom_target(run
    COMMAND opengl_program
    DEPENDS opengl_program
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
