ow_create_static_library(amrwb)

ow_add_public_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}
)

ow_add_private_compile_flags(
	${PHAPI_OPTIMIZATION_FLAGS}
)

ow_add_sources(
	dec_acelp.c
	dec_dtx.c
	dec_gain.c
	dec_if.c
	dec_lpc.c
	dec_main.c
	dec_rom.c
	dec_util.c
	enc_acelp.c
	enc_dtx.c
	enc_gain.c
	enc_if.c
	enc_lpc.c
	enc_main.c
	enc_rom.c
	enc_util.c
	if_rom.c
)

ow_create_project_binary()
