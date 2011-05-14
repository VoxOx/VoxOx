ow_create_static_library(g722)

ow_add_public_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}
)

ow_add_sources(
	g722_decode.c
	g722_encode.c
	g722.h
)

ow_create_project_binary()
