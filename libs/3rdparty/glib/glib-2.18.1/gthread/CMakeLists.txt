ow_create_shared_library(gthread)

ow_use_private_libraries(
	glib
)

ow_add_public_include_dirs(	
	${CMAKE_CURRENT_SOURCE_DIR}/
)

ow_add_private_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}/../config
)

ow_add_private_definitions(
	-DHAVE_CONFIG_H
	-DG_LOG_DOMAIN=\\\\\\\\\\\\\\\\\\\\"GThread\\\\\\\\\\\\\\\\\\\\"
	-DG_DISABLE_DEPRECATED
)

ow_add_sources(
	gthread.def
	gthread-impl.c
)

ow_create_project_binary()

