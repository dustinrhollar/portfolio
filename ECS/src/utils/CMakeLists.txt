
set(UTIL_HEADERS
	linked_list.h
)

set(UTIL_SOURCES
	${UTIL_HEADERS}
)

add_library(Util_Module ${UTIL_SOURCES})

include_directories(Util_Module PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../inc
                    Util_Module PRIVATE ${CMAKE_CURRENT_SOURCE_DIR/../mm})

target_include_directories(Util_Module
		PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}
		)

install(TARGETS Util_Module DESTINATION lib)
install(FILES ${UTIL_HEADERS} DESTINATION include)