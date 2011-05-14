ow_create_shared_library(owbase)

ow_add_public_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

ow_add_sources(
	src/owlist.c
	src/owqueue.c
)

ow_use_private_libraries(
	pthread
)

ow_create_project_binary()
