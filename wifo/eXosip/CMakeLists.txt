ow_create_static_library(exosip)

ow_use_public_libraries(
	transport_manager
	svoip
	osip2
)

ow_add_public_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/include/eXosip
	${CMAKE_CURRENT_SOURCE_DIR}/src/ow_osip
)

ow_use_private_libraries(
	pthread
)

if (WIN32)
	ow_add_private_libraries(
		ws2_32 # to remove (owsl is used instead of winsock)
		winmm
		iphlpapi
	)
endif (WIN32)

ow_add_sources(
	src/base64.c
	src/eXosip.c
	src/eXutils.c
	src/jauth.c
	src/jcall.c
	src/jcallback.c
	src/jdialog.c
	src/jevents.c
	src/jfreinds.c
	src/jidentity.c
	src/jmsg.c
	src/jnotify.c
	src/jpipe.c
	src/jreg.c
	src/jrequest.c
	src/jresponse.c
	src/jsubscribe.c
	src/jsubscribers.c
	src/misc.c
	src/sdp_offans.c
	src/udp.c
	src/owsip_account.c
	src/ow_osip/ow_osip.c
	src/ow_osip/osip_negotiation.c
)

ow_create_project_binary()
