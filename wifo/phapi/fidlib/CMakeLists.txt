ow_create_static_library(fidlib)

ow_add_public_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

ow_add_sources(
	src/fidlib.c
)

if (WIN32)
	ow_add_private_definitions(
		-DT_MSVC
	)
endif (WIN32)

ow_create_project_binary()
