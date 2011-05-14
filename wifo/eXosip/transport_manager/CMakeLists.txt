ow_create_static_library(transport_manager)

ow_add_public_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}
)

ow_use_public_libraries(
	owsl
	owbase
)

ow_use_private_libraries(
	pthread
)

ow_add_sources(
	transport_manager.c
)

ow_create_project_binary()
