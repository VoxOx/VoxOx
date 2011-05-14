ow_create_static_library(svoip)

ow_use_private_libraries(
	srtp
	osip2
	ortp
	openssl
)

ow_add_public_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

ow_add_private_definitions(
	-DUSE_SRTP
)

ow_add_sources(
	src/error.c
	src/sdp.c
	src/crypto.c
	src/sessionsManager.c
	src/svoip.c
	src/svoip_phapi.c
	src/dh.c
)

if (WIN32)
	ow_add_private_definitions(
		-DWIN32
	)
endif (WIN32)

ow_create_project_binary()
