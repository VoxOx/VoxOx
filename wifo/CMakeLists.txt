# opt flags for phapi and plugins
if (MSVC)
	# TODO this has to be reworked
	# with comments explaining each flag
	set(PHAPI_OPTIMIZATION_FLAGS
		#/Ox
		#/Ob2
		/Oi
		/Ot
		/Oy
		#/G7 unknow option under Visual C++ 2005
		#/GX- deprecated
		/EHs-c-
		/GR-
		/arch:SSE
	)
else (MSVC)
	# TODO we need a flag on other platforms
	# maybe unroll-loops is nice to have
	set(PHAPI_OPTIMIZATION_FLAGS
		-funroll-loops
	)
endif (MSVC)

if (WIN32)
	subdirs(
		phapi/msdirent
	)
endif (WIN32)

if (PHAPI_CODEC_AMR_SUPPORT)
	subdirs(
		phapi/phamrplugin
	)
endif (PHAPI_CODEC_AMR_SUPPORT)

subdirs(
	owbase
	owsl
	netlib
	ortp
	srtp
	libosip2
	sVoIP
	eXosip/transport_manager
	eXosip
	phapi-util
	phapi/fidlib
	phapi/gsm
	phapi/ilbc
	phapi/g722
    phapi/g726
	phapi/amrnb
	phapi/amrwb
	phapi/speex
	phapi/speexec
	phapi/stun
	phapi/phspeexplugin
	phapi/philbcplugin
	phapi
	wifo-plugins
	phapi/pyphapi
	#miniua
	#tests/WifoPluginsConsole
)