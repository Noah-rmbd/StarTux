cmake_minimum_required(VERSION 3.5)

project(OpenGLProgram)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find external packages
find_package(assimp REQUIRED)
find_package(Freetype REQUIRED)

# Set directories
set(SRC_DIR src)
set(INCLUDE_DIR include)
set(EXTERNAL_DIR external)
set(GLEW_DIR ${EXTERNAL_DIR}/glew-cmake)
set(GLFW_DIR ${EXTERNAL_DIR}/glfw)
set(GLM_DIR ${EXTERNAL_DIR}/glm)

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
    ${SRC_DIR}/game/particle_system.cpp
    ${SRC_DIR}/game/asteroid.cpp
    ${SRC_DIR}/shapes/shape_model.cpp
    ${SRC_DIR}/shapes/sphere.cpp
    ${SRC_DIR}/graphics/texture.cpp
    ${SRC_DIR}/shapes/textured_sphere.cpp
    ${SRC_DIR}/shapes/lighting_sphere.cpp
    ${SRC_DIR}/interfaces/interface.cpp
    ${SRC_DIR}/interfaces/startup_screen.cpp
    ${SRC_DIR}/interfaces/hud.cpp
)

# Shader and resources directories as compile-time definitions
add_compile_definitions(SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders/")
add_compile_definitions(TEXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/textures/")
add_compile_definitions(RESSOURCES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/ressources/")

# Add external subdirectories
add_subdirectory(${GLEW_DIR})
add_subdirectory(${GLFW_DIR})
add_subdirectory(${GLM_DIR})  # GLM is header-only, but this ensures it is set up

# Define executable
add_executable(opengl_program ${SOURCES})

# Include directories (modern style)
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
)

# Link libraries
target_link_libraries(opengl_program
    libglew_static
    glfw
    ${ASSIMP_LIBRARIES}
    ${FREETYPE_LIBRARIES}
)

# Custom run target
add_custom_target(run
    COMMAND opengl_program
    DEPENDS opengl_program
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

