set(GAME_HEADERS
    splicer.h
    mesh.h
    frontend_renderer.h
    texture.h
    material.h
    animations.h
    ai/fox_ai.h
)

set(GAME_SOURCES
    ${GAME_HEADERS}
    splicer.cpp
    mesh.cpp
    frontend_renderer.cpp
    texture.cpp
    material.cpp
    animations.cpp
    ai/fox_ai.cpp
)

target_sources(SplicerLib PRIVATE ${GAME_SOURCES})