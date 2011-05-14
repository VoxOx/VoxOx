ow_create_plugin_library(philbcplugin)

ow_use_private_libraries(
	ilbc
	pthread
	owsl
)

ow_add_private_compile_flags(
	${PHAPI_OPTIMIZATION_FLAGS}
)

ow_add_sources(
	../phcodec-ilbc.c
)

set(PROJECT_BINARY_COPY_DIR ${BUILD_DIR}/phapi-plugins/.)
ow_create_project_binary()
