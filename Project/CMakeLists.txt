cmake_minimum_required(VERSION 3.5)

project(OpenGLProgram)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Assimp package
find_package(assimp REQUIRED)
find_package(Freetype REQUIRED) 

# Set directories
set(SRC_DIR src)
set(INCLUDE_DIR include)
set(EXTERNAL_DIR external)
set(GLEW_DIR ${EXTERNAL_DIR}/glew-cmake)
set(GLFW_DIR ${EXTERNAL_DIR}/glfw)
set(GLM_DIR ${EXTERNAL_DIR}/glm)

# Include directories
include_directories(${INCLUDE_DIR})
include_directories(${GLEW_DIR}/include)
include_directories(${GLFW_DIR}/include)
include_directories(${GLM_DIR})
include_directories(${ASSIMP_INCLUDE_DIRS})
include_directories(${FREETYPE_INCLUDE_DIRS})

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
    ${SRC_DIR}/shapes/shape_model.cpp
    ${SRC_DIR}/shapes/sphere.cpp
    ${SRC_DIR}/graphics/texture.cpp
    ${SRC_DIR}/shapes/textured_sphere.cpp
    ${SRC_DIR}/shapes/lighting_sphere.cpp
    ${SRC_DIR}/interfaces/interface.cpp
    ${SRC_DIR}/interfaces/startup_screen.cpp
    ${SRC_DIR}/interfaces/hud.cpp

)

# Define Shader director
add_compile_definitions(SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders/")
add_compile_definitions(TEXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/textures/")
add_compile_definitions(RESSOURCES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/ressources/")

# GLEW library
add_subdirectory(${GLEW_DIR})
set(LIBS ${LIBS} libglew_static)

# GLFW library
add_subdirectory(${GLFW_DIR})
set(LIBS ${LIBS} glfw)

# GLM library
add_subdirectory(${GLM_DIR})

# Executable
add_executable(opengl_program ${SOURCES})
target_link_libraries(opengl_program ${LIBS})
target_link_libraries(opengl_program ${ASSIMP_LIBRARIES})
target_link_libraries(opengl_program ${FREETYPE_LIBRARIES})

add_custom_target(run
    COMMAND opengl_program
    DEPENDS opengl_program
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
)
