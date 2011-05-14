ow_create_static_library(amrnb)

ow_add_public_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}
)

ow_add_private_compile_flags(
	${PHAPI_OPTIMIZATION_FLAGS}
)

ow_add_sources(
	interf_dec.c
	interf_enc.c
	sp_dec.c
	sp_enc.c
)

ow_create_project_binary()
