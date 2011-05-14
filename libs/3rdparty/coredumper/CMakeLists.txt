ow_create_shared_library(coredumper)

ow_add_sources(
	src/coredumper.c
	src/elfcore.c
	src/linuxthreads.c
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
	${CMAKE_BINARY_DIR}/coredumper/config.h
)

ow_add_private_include_dirs(
	${CMAKE_BINARY_DIR}/coredumper
)

ow_add_public_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}/src
)

ow_create_project_binary()
