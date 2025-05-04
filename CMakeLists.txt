cmake_minimum_required(VERSION 3.15)
project(OpenGLExample)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(deps/glfw)
add_subdirectory(deps/glm)

# GLAD
add_library(glad STATIC deps/glad/src/glad.c)
target_include_directories(glad PUBLIC deps/glad/include deps/KHR)

# Sources
add_executable(main
    src/main.cpp
    src/camera.cpp
    src/graph.cpp
    src/node.cpp
    src/universe.cpp
    src/utils/draw/line.cpp
    src/utils/draw/solid_sphere.cpp
    src/utils/rgb.cpp
    src/utils/vec3d.cpp
)


# Link
target_link_libraries(main PRIVATE glad glfw glm)

if(UNIX)
  find_package(OpenGL REQUIRED)
  target_link_libraries(main PRIVATE ${OPENGL_LIBRARIES} dl pthread X11)
endif()
