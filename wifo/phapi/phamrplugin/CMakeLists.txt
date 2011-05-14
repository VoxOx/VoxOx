ow_create_plugin_library(phamrplugin)

ow_use_private_libraries(
	amrwb
	amrnb
)

ow_add_private_compile_flags(
	${PHAPI_OPTIMIZATION_FLAGS}
)

ow_add_sources(
	../phcodec-amr-wrapper.c
)

if (PHAPI_SPEEXWB_REPLACES_G726WB_HACK)
	ow_add_private_definitions(
		-DPH_SPEEXWB_REPLACES_G726WB
	)
else (PHAPI_SPEEXWB_REPLACES_G726WB_HACK)
	if (PHAPI_SPEEXWB_REPLACES_AMRNB_HACK)
		ow_add_private_definitions(
			-DPH_SPEEXWB_REPLACES_AMRNB
		)
	else (PHAPI_SPEEXWB_REPLACES_AMRNB_HACK)
		if (PHAPI_SPEEXWB_REPLACES_AMRWB_HACK)
			ow_add_private_definitions(
				-DPH_SPEEXWB_REPLACES_AMRWB
			)
		endif (PHAPI_SPEEXWB_REPLACES_AMRWB_HACK)
	endif (PHAPI_SPEEXWB_REPLACES_AMRNB_HACK)
endif (PHAPI_SPEEXWB_REPLACES_G726WB_HACK)

set(PROJECT_BINARY_COPY_DIR ${BUILD_DIR}/phapi-plugins/.)
ow_create_project_binary()
