ow_create_static_library(stun)

ow_add_public_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}
)

if (WIN32)
	ow_add_private_definitions(
		-DWIN32
	)
endif (WIN32)

ow_add_sources(
	udp.c
	stun.c
)

ow_create_project_binary()
