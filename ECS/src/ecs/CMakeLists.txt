
set(ECS_HEADERS
	entity.h
	component.h
	system.h
	ecs.h
)

set(ECS_SOURCES
	${UTIL_HEADERS}
	entity.cpp
	component.cpp
	system.cpp
	ecs.cpp
)

add_library(ECS_Module ${ECS_SOURCES})

include_directories(ECS_Module PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../inc
                    ECS_Module PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../mm
					ECS_Module PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../utils)

target_include_directories(ECS_Module
		PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}
		)

install(TARGETS ECS_Module DESTINATION lib)
install(FILES ${ECS_HEADERS} DESTINATION include)