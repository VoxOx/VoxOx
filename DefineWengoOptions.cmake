option(BUILDID_SUPPORT "Build WengoPhone/phApi with buildId (builds faster without)" ON)

option(WENGOPHONE_RESOURCEDIR_PATH "WengoPhone: use ${DATA_INSTALL_DIR} as hardcoded resource path" ON)
option(WENGOPHONE_XV_SUPPORT "Build WengoPhone with XV hardware acceleration support" ON)

option(SIPWRAPPER_BACKEND_SIPX "SipX backend for SipWrapper" OFF)
option(SIPWRAPPER_BACKEND_PHAPI "PhApi backend for SipWrapper" OFF)
option(IMWRAPPER_BACKEND_MULTI "PhApi and Gaim backend for IMWrapper" ON)

option(LOGGER_ENABLE_EVENT "Enable the event notification in Logger" OFF)

option(PHAPI_VIDEO_SUPPORT "Build phApi with video support" ON)

if (LINUX)
	option(PHAPI_SAMPLERATE_SUPPORT "Build phApi with libsamplerate support" ON)
	option(PHAPI_AUDIO_ALSA_SUPPORT "Build phApi with ALSA support" ON)
	option(PHAPI_AUDIO_ARTS_SUPPORT "Build phApi with aRts support" OFF)
	option(PHAPI_AUDIO_ESD_SUPPORT "Build phApi with ESD support" OFF)
	option(PHAPI_AUDIO_OSS_SUPPORT "Build phApi with OSS support" ON)
else (LINUX)
	option(PHAPI_SAMPLERATE_SUPPORT "Build phApi with libsamplerate support" OFF)
	option(PHAPI_AUDIO_ALSA_SUPPORT "Build phApi with ALSA support" OFF)
	option(PHAPI_AUDIO_ARTS_SUPPORT "Build phApi with aRts support" OFF)
	option(PHAPI_AUDIO_ESD_SUPPORT "Build phApi with ESD support" OFF)
	option(PHAPI_AUDIO_OSS_SUPPORT "Build phApi with OSS support" OFF)
endif (LINUX)

option(PHAPI_HTTPTUNNEL_SUPPORT "Build phApi with httptunnel support" ON)
option(PHAPI_QOS_DEBUG_SUPPORT "Build phApi with QOS debug support" OFF)
option(PHAPI_EMBEDDED_SUPPORT "Build phApi with embedded target support" OFF)
option(PHAPI_CODEC_ILBC_SUPPORT "Build phApi with codec ILBC support" ON)
option(PHAPI_CODEC_AMR_SUPPORT "Build phApi with codec AMR support" OFF)
option(PHAPI_SPEEXWB_REPLACES_AMRWB_HACK "Build phApi with Speex-WB/AMR-WB hack" ON)
option(PHAPI_SPEEXWB_REPLACES_AMRNB_HACK "Build phApi with Speex-WB/AMR-NB hack" ON)
option(PHAPI_SPEEXWB_REPLACES_G726WB_HACK "Build phApi with Speex-WB/G726-WB hack" ON)
option(PHAPI_PYTHON_WRAPPER "Python wrapper for PhApi, used for unit testing" OFF)

option(COPY_DATA_FILES_ON_CONFIGURE "Copy data files on configure" ON)

#if (LINUX AND APPLE AND NOT "${CMAKE_SYSTEM_PROCESSOR}" MATCHES "i[3456]86")
#	option(ENABLE_CRASHREPORT "Enable integrated crashreport system" OFF)
#else (LINUX AND APPLE AND NOT "${CMAKE_SYSTEM_PROCESSOR}" MATCHES "i[3456]86")
#	option(ENABLE_CRASHREPORT "Enable integrated crashreport system" ON)
#endif (LINUX AND APPLE AND NOT "${CMAKE_SYSTEM_PROCESSOR}" MATCHES "i[3456]86")

if (APPLE OR LINUX)
	option(ENABLE_CRASHREPORT "Enable integrated crashreport system" OFF)
else (APPLE OR LINUX)
	option(ENABLE_CRASHREPORT "Enable integrated crashreport system" ON)
endif (APPLE OR LINUX)

option(CUSTOM_ACCOUNT "Enable custom account" OFF)

option(DISABLE_CALL_FORWARD "Disable call forwarding by http suport" ON)

option(DISABLE_SMS "Disable sms by http support" ON)

option(DISABLE_VOICE_MAIL "Disable voice mail by http support" ON)

if (WIN32)
	option(ENABLE_CONSOLE "Force generate of executable with Console Window enabled" OFF)
	option(FFMPEG_OLD_VERSION "Use the old version of ffmpeg" OFF)
	option(OWSOUND_PORTAUDIO_SUPPORT "Build libsound with PortAudio support" ON)
endif (WIN32)
