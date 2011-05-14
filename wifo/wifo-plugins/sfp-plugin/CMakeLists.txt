ow_create_plugin_library(sfp-plugin)

ow_use_public_libraries(
	phapiutil
)

ow_use_private_libraries(
	phapi
	exosip
	curl
	pthread
)

ow_add_public_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

if (WIN32)
	# MSVC or MinGW
	ow_add_private_definitions(
		-DBUILD_SFP_PLUGIN_DLL
		-DWIN32
	)

	if (MINGW)
		ow_add_private_definitions(
			-D_UWIN
		)
	endif (MINGW)
endif (WIN32)

ow_add_sources(
	src/sfp-parser.c
	src/sfp-plugin.c
	src/sfp-transfer.c
)

ow_create_project_binary()
