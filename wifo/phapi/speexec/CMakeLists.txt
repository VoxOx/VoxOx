ow_create_static_library(speexec)

ow_add_public_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

ow_add_sources(
	src/spxec.c
	src/spxecfft.c
	src/spxecfftwrap.c
)

ow_create_project_binary()
