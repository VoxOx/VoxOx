ow_create_shared_library(owsl)

ow_add_public_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

ow_add_private_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}/src
)

ow_add_sources(
	src/core/owsl.c
	src/core/owsl_address.c
	src/core/owsl_asynchronous.c
	src/core/owsl_monitor.c
	src/core/owsl_openssl.c
	src/core/owsl_socket.c
	src/core/owsl_socket_type.c
	src/core/owsl_system_socket.c
	src/protocols/owsl_base.c
	src/protocols/owsl_tcp.c
	src/protocols/owsl_tls.c
	src/protocols/owsl_udp.c
	src/protocols/owsl_uoh.c
	src/protocols/owsl_uohs.c
	src/protocols/owsl_uoh_common.c
)

ow_use_public_libraries(
	owbase
)

ow_use_private_libraries(
	curl
	openssl
	pthread
)

if (WIN32)
	ow_add_private_libraries(
		ws2_32
		iphlpapi # used by owsl_address_public_ip_get
	)
endif (WIN32)

if (APPLE)
	ow_add_private_link_flags(
		# avoid a -multi_module error (see http://gcc.gnu.org/ml/gcc/2005-06/msg00184.html)
		-Wl,-single_module
		# avoid a "has local relocation entries in non-writable section" (see http://lists.mplayerhq.hu/pipermail/ffmpeg-devel/2006-October/017814.html)
		-read_only_relocs suppress
	)
endif (APPLE)

ow_create_project_binary()
