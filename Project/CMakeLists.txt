cmake_minimum_required(VERSION 3.10)

project(OpenGLProgram)

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
    ${SRC_DIR}/shape.cpp
    ${SRC_DIR}/shader.cpp
    ${SRC_DIR}/viewer.cpp
    ${SRC_DIR}/camera.cpp
    ${SRC_DIR}/game.cpp
    ${SRC_DIR}/player.cpp
    ${SRC_DIR}/triangle.cpp
    ${SRC_DIR}/pyramid.cpp
    ${SRC_DIR}/cylinder.cpp
    ${SRC_DIR}/shape_model.cpp
    ${SRC_DIR}/sphere.cpp
    ${SRC_DIR}/texture.cpp
    ${SRC_DIR}/textured_sphere.cpp
    ${SRC_DIR}/lighting_sphere.cpp
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