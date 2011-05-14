ow_create_static_library(ilbc)

ow_add_public_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}
)

ow_add_sources(
	anaFilter.c
	constants.c
	createCB.c
	doCPLC.c
	enhancer.c
	filter.c
	FrameClassify.c
	gainquant.c
	getCBvec.c
	helpfun.c
	hpInput.c
	hpOutput.c
	iCBConstruct.c
	iCBSearch.c
	iLBC_decode.c
	iLBC_encode.c
	LPCdecode.c
	LPCencode.c
	lsf.c
	packing.c
	StateConstructW.c
	StateSearchW.c
	syntFilter.c
)

ow_add_private_compile_flags(
	${PHAPI_OPTIMIZATION_FLAGS}
)

ow_create_project_binary()
