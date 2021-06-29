
set(MM_MODULE_HEADER
	allocator.h
    linear_allocator.h
    stack_allocator.h
    free_list_allocator.h
    pool_allocator.h
    proxy_allocator.h
    mm.h
)

set(MM_MODULE_SOURCES
	${MM_MODULE_HEADER}
    linear_allocator.cpp
    stack_allocator.cpp
    free_list_allocator.cpp
    pool_allocator.cpp
    proxy_allocator.cpp
    mm.cpp
)

add_library(MM_Module ${MM_MODULE_SOURCES})

include_directories(MM_Module PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../inc)

target_include_directories(MM_Module
		PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}
		)

install(TARGETS MM_Module DESTINATION lib)
install(FILES ${MM_MODULE_HEADER} DESTINATION include)