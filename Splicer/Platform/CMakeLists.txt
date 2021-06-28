add_executable(SplicerPlatform WIN32 splicer_win32.cpp logger.cpp)

# target_compile_options(SplicerPlatform PRIVATE /link winmm.lib)

include_directories(SplicerPlatform PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../inc)
include_directories(SplicerPlatform PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../ext)
include_directories(SplicerPlatform PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../splicer/game)
include_directories(SplicerPlatform PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../splicer/engine/utils)
include_directories(SplicerPlatform PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../splicer/engine/mm)

target_link_libraries(SplicerPlatform PRIVATE glfw3 winmm)

target_include_directories(SplicerPlatform PRIVATE
  ../ext/glfw-3.3.2/include
  )

target_include_directories(SplicerPlatform PUBLIC
                           "${PROJECT_BINARY_DIR}"
                          )
