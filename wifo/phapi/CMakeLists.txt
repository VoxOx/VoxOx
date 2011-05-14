ow_create_shared_library(phapi)

ow_use_private_libraries(
	osip2
	exosip
	ortp
	stun
	gsm
	ilbc
	phapiutil
	fidlib
	speexec
	wtimer
	svoip
	pthread
	g722
	g726
)

ow_add_private_compile_flags(
	${PHAPI_OPTIMIZATION_FLAGS}
)


ow_add_public_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}
)

if (WIN32)
	# MSVC or MinGW
	ow_use_private_libraries(
		msdirent
	)
endif (WIN32)

ow_add_private_definitions(
	-DUSE_PCM=1
	-DNEED_ECMUX=1
	-DMINIMIZE_COPYING=1
	-DSCHEDULING_MODE=0
	-DBLOCKING_MODE=0
	-DDO_ECHO_CAN=1
	-DUSE_WAVEIN_CBK=1
	-DPSEUDO_AEC=0
	-DNO_ECHO__SUPPRESSOR=1
#	-DOSIP_MT=1
#	-DENABLE_TRACE=0
	-DPH_VIDEO_USELOCK=1
	-DUSE_SPXEC=1
	-DPH_FORCE_16KHZ=1
	-DUSE_FLV1_OVER_H261=1
)

if (PHAPI_CODEC_AMR_SUPPORT)
	ow_add_private_definitions(
		-DENABLE_AMR_WB=1
		-DENABLE_AMR=1
	)
endif (PHAPI_CODEC_AMR_SUPPORT)


ow_add_private_definitions(
	-DENABLE_ILBC=1
)

if (MSVC)
	ow_add_private_definitions(
		-DWENGOPHONE_UA="""wengo/v1/wengophoneng/wengo/rev${SVN_REVISION}/trunk/"""
		-DVOXOXVERSION=${APPLICATION_VERSION}
	)
else (MSVC)
	ow_add_private_definitions(
		-DWENGOPHONE_UA='"wengo/v1/wengophoneng/wengo/rev${SVN_REVISION}/trunk/"'
		-DVOXOXVERSION=${APPLICATION_VERSION}
	)
endif (MSVC)

if (BUILDID_SUPPORT)
	ow_add_private_definitions(
		-DSUBVERSION_REVISION="${SVN_REVISION}"
	)
endif (BUILDID_SUPPORT)

# PH_FORCE_16KHZ : activate FORCED 16 KHZ SAMPLING
# SUBVERSION_REVISION : grab the svn revision number
# WENGOPHONE_UA : set the wengophone user agent. It should
#                 be later changed to "Wengophone NG"

ow_add_sources(
	aec2.cpp
	nortel.c
	owpl_adapter.c
	owpl_plugin.c
	owpl_log.c
	phapi-old.c
	phapi.c
	phaudiodriver.c
	phcodec.c
	phevents.c
	phlog.c
	phmbuf.c
	phmedia-audio-file.c
#	phmedia-audio-null.c
	phmedia-audio.c
	phmedia.c
	phrecorder.c
	phresample.c
	phvline.c
	tonegen.c
	wav.c
	wlm.c
)

ow_add_sources(
	owpl.h
	owpl_plugin.h
	phapi.h
	phapi-old.h
	phcall.h
	phevents.h
	phglobal.h
	phvline.h
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

if (PHAPI_EMBEDDED_SUPPORT)
	ow_add_private_definitions(
		-DSCHEDULING_MODE
	)
endif (PHAPI_EMBEDDED_SUPPORT)

if (PHAPI_QOS_DEBUG_SUPPORT)
	ow_add_private_definitions(
		-DPHAPI_QOS_DEBUG
	)
endif (PHAPI_QOS_DEBUG_SUPPORT)

if (PHAPI_VIDEO_SUPPORT)
	ow_use_private_libraries(
		webcam
	)

	ow_add_private_definitions(
		-DPHAPI_VIDEO_SUPPORT
	)

	ow_add_sources(
		phcodec-h263.c
		phcodec-h263plus.c
		phcodec-h263flv1.c
		phcodec-h264.c
		phcodec-mpeg4.c
		phmedia-video.c
		phmedia-video-control.c
		phmedia-video-bwcontrol.c
		phcodec-avcodec-wrapper.c
	)
endif (PHAPI_VIDEO_SUPPORT)

if (WIN32)
	ow_add_private_libraries(
		winmm
		ws2_32
		user32
		shell32
		advapi32
		wininet
		iphlpapi
		strmiids
		ole32
	)

	ow_add_sources(
		phmedia-winmm.c
	)

	ow_add_private_definitions(
		-DT_MSVC
	)

	if (MSVC)
		ow_add_private_libraries(
			comsupp
		)
	else (MSVC)
		ow_add_private_libraries(
			gdi32
		)
	endif (MSVC)
endif (WIN32)

if (UNIX AND NOT WIN32)
	if (APPLE)
		ow_add_sources(
			phmedia-coreaudio.c
		)

		ow_add_private_link_flags(
			# avoid a -multi_module error (see http://gcc.gnu.org/ml/gcc/2005-06/msg00184.html)
			-Wl,-single_module
			# avoid a "has local relocation entries in non-writable section" (see http://lists.mplayerhq.hu/pipermail/ffmpeg-devel/2006-October/017814.html)
			-read_only_relocs suppress
		)
	else (APPLE)
		ow_use_private_libraries(
			glib2
		)

		ow_add_private_libraries(
			pthread
			rt
		)

		ow_add_private_definitions(
			-DPORTAUDIO_MONO_STREAM
		)

		if (PHAPI_SAMPLERATE_SUPPORT)
			ow_use_private_libraries(
				samplerate
			)

			ow_add_private_definitions(
				-DPH_USE_RESAMPLE
			)
		endif (PHAPI_SAMPLERATE_SUPPORT)

		if (PHAPI_AUDIO_ALSA_SUPPORT)
			ow_use_private_libraries(
				alsa
			)
			ow_add_private_definitions(
				-DENABLE_ALSA
			)
			ow_add_sources(
				phmedia-alsa.c
			)
		endif (PHAPI_AUDIO_ALSA_SUPPORT)

		if (PHAPI_AUDIO_OSS_SUPPORT)
			ow_add_private_definitions(
				-DENABLE_OSS
			)
			ow_add_sources(
				phmedia-oss.c
			)
		endif (PHAPI_AUDIO_OSS_SUPPORT)

		if (PHAPI_AUDIO_ARTS_SUPPORT)
			ow_use_private_libraries(
				arts
			)

			ow_add_private_definitions(
				-DENABLE_ARTS
			)

			ow_add_sources(
				phmedia-audio-arts.c
			)
		endif (PHAPI_AUDIO_ARTS_SUPPORT)

		if (PHAPI_AUDIO_ESD_SUPPORT)
			ow_use_private_libraries(
				esd
			)

			ow_add_private_definitions(
				-DENABLE_ESD
			)

			ow_add_sources(
				phmedia-audio-esd.c
			)
		endif (PHAPI_AUDIO_ESD_SUPPORT)
	endif (APPLE)
endif (UNIX AND NOT WIN32)

if (OWSOUND_PORTAUDIO_SUPPORT)
	ow_use_private_libraries(
		portaudio
	)

	ow_add_private_definitions(
		-DENABLE_PORTAUDIO
	)

	ow_add_sources(
		phmedia-portaudio.c
	)
endif (OWSOUND_PORTAUDIO_SUPPORT)

if (APPLE)
	ow_use_public_frameworks(
		AudioUnit
		CoreAudio
		AudioToolbox
		CoreServices
	)
endif (APPLE)

ow_set_library_version(0.2.0 0)

ow_create_project_binary()

subdirs(
	test
)
