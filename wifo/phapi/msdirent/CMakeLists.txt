ow_create_static_library(msdirent)

ow_add_public_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}
)

ow_add_sources(
	msdirent.c
)

ow_create_project_binary()
