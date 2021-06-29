cmake_minimum_required(VERSION 3.10)

# project name
project(ECS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(src/mm)
# add_subdirectory(src/utils)
add_subdirectory(src/ecs)
add_subdirectory(examples)
