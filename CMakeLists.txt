cmake_minimum_required(VERSION 3.12)
project(OpenGLExample)

set(CMAKE_CXX_STANDARD 17)

# Add GLFW (already cloned into deps/)
add_subdirectory(deps/glfw)

# Add GLAD (header and glad.c manually copied)
include_directories(include)
add_library(glad STATIC src/glad.c)

# Executable
add_executable(main main.cpp)
target_link_libraries(main PRIVATE glad glfw)

# For Linux: link with OpenGL and X11 dependencies
if(UNIX)
  find_package(OpenGL REQUIRED)
  target_link_libraries(main PRIVATE ${OPENGL_LIBRARIES} dl pthread X11)
endif()
