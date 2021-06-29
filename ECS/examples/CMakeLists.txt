add_executable(ECS example.cpp)

include_directories(ECS PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../inc)
include_directories(ECS PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src/utils)
include_directories(ECS PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src/ecs)

target_link_libraries(ECS PRIVATE MM_Module)
# target_link_libraries(ECS PRIVATE Util_Module)
target_link_libraries(ECS PRIVATE ECS_Module)

target_include_directories(ECS PUBLIC
                           "${PROJECT_BINARY_DIR}"
                          )