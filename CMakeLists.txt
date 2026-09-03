cmake_minimum_required(VERSION 3.16)
project(SandPhysics LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(SandPhysics
    src/main.cpp
    src/shader.cpp
    src/renderer.cpp
    src/simulation.cpp
)

target_link_libraries(SandPhysics PRIVATE
    glfw
    GLEW::GLEW
    glm::glm
    OpenGL::GL
)
