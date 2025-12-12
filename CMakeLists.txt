cmake_minimum_required(VERSION 3.10)

project(ThreeBodyProblem)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# =============================
# Include directories
# =============================
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/glad
    ${PROJECT_SOURCE_DIR}/include/glfw
    ${PROJECT_SOURCE_DIR}/include/glm
)

# =============================
# Source files
# =============================
set(SOURCES
    src/main.cpp
    src/threeBodyPhysics.cpp
    src/glad.c
)

# =============================
# Create executable
# =============================
add_executable(ThreeBodyProblem ${SOURCES})

# =============================
# Link GLFW + OpenGL (WINDOWS)
# =============================
target_link_libraries(ThreeBodyProblem
    ${PROJECT_SOURCE_DIR}/lib/libglfw3.a
    opengl32
    gdi32
    user32
    kernel32
)
