cmake_minimum_required(VERSION 3.15)
project(project CXX C)

# Build target
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# GLAD
add_subdirectory(external/glad/cmake)
glad_add_library(glad_gl_core_33 STATIC REPRODUCIBLE API gl:core=3.3) # OpenGL 3.3

# GLFW
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE) # unnecessary in the project context
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # do not need examples
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE) # no need for documentation
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE) # no need for tests, do it manually

add_subdirectory(external/glfw)

# GLM
add_subdirectory(external/glm)

# EXE
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp)
add_executable(project ${PROJECT_SOURCES})

target_link_libraries( project PRIVATE 
    glad_gl_core_33
    glfw 
    glm::glm
)

target_include_directories( project PRIVATE ${CMAKE_SOURCE_DIR}/src)