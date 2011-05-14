ow_create_static_library(gsm)

ow_add_public_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}
)

ow_add_sources(
	add.c
	code.c
	debug.c
	decode.c
	gsm_create.c
	gsm_decode.c
	gsm_destroy.c
	gsm_encode.c
	gsm_explode.c
	gsm_implode.c
	gsm_option.c
	gsm_print.c
	long_term.c
	lpc.c
	preprocess.c
	rpe.c
	short_term.c
	table.c
)

ow_create_project_binary()
