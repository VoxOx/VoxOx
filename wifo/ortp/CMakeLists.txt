include(TestBigEndian)
ow_create_static_library(ortp)

ow_add_public_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/include/ortp
)

ow_add_private_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}/build/win32native/include
)

ow_add_private_definitions(
	-DRTP_MAX_SIZE=8192
	-DORTP_INET6
)

ow_add_public_definitions(
	-DORTP_DEBUG_MODE
	-DORTP_STATIC
)

if (WIN32)
	ow_add_private_definitions(
		-DWIN32
		-DWINDOW_NATIVE
	)

	ow_add_private_libraries(
		ws2_32
		winmm
	)
endif (WIN32)

TEST_BIG_ENDIAN(BIG_ENDIAN)
if (BIG_ENDIAN)
	ow_add_public_definitions(
		-DORTP_BIGENDIAN
		-DWORDS_BIGENDIAN
	)
endif (BIG_ENDIAN)

ow_add_sources(
	src/avprofile.c
	src/event.c
	src/jitterctl.c
	src/ortp.c
	src/payloadtype.c
	src/port.c
	src/posixtimer.c
	src/rtcp.c
	src/rtcpparse.c
	src/rtpparse.c
	src/rtpsession.c
	src/rtpsession_inet.c
	src/rtpsignaltable.c
	src/rtptimer.c
	src/scheduler.c
	src/sessionset.c
	src/str_utils.c
	src/stun.c
	src/stun_udp.c
	src/telephonyevents.c
	src/utils.c
)

if (WIN32)
	ow_add_sources(
		src/dll_entry.c
	)
endif (WIN32)

ow_create_project_binary()
