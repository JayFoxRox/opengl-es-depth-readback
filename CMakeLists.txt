cmake_minimum_required(VERSION 3.10)

find_package(GLUT REQUIRED)

add_executable(cube cube.c)
target_link_libraries(cube ${GLUT_LIBRARIES} -lGLU -lGL)
