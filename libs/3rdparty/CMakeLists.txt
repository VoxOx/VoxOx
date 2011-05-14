option(CHECK_BOOST_GCC_BUG "Check for wrong combination of Boost and GCC version" ON)

if (LINUX)
	option(FFMPEG_INTERNAL "Use internal FFmpeg" OFF)
	option(SPEEX_INTERNAL "Use internal Speex" OFF)
	option(PORTAUDIO_INTERNAL "Use internal PortAudio" ON)
	option(IAXCLIENT_INTERNAL "Use internal iaxclient" ON)
	option(SAMPLERATE_INTERNAL "Use internal libsamplerate" OFF)
	option(OSIP2_INTERNAL "Use internal oSIP" ON)
	option(CURL_INTERNAL "Use internal cURL" ON)
	option(LIBPURPLE_INTERNAL "Use internal libpurple" ON)
else (LINUX)
	option(PORTAUDIO_INTERNAL "Use internal PortAudio" ON)
	option(FFMPEG_INTERNAL "Use internal FFmpeg" ON)
	option(SPEEX_INTERNAL "Use internal Speex" ON)
	option(IAXCLIENT_INTERNAL "Use internal iaxclient" ON)
	option(SAMPLERATE_INTERNAL "Use internal libsamplerate" ON)
	option(OSIP2_INTERNAL "Use internal oSIP" ON)
	option(CURL_INTERNAL "Use internal cURL" ON)
	option(LIBPURPLE_INTERNAL "Use internal libpurple" ON)
endif (LINUX)

if (WIN32)
	if (FFMPEG_OLD_VERSION)
		subdirs(
			ffmpeg-bin-win32
		)
	else (FFMPEG_OLD_VERSION)
		subdirs(
			ffmpeg
		)
	endif (FFMPEG_OLD_VERSION)
else (WIN32)
	subdirs(
		ffmpeg
	)
endif (WIN32)

if (UNIX AND NOT WIN32)
	subdirs(
		libgcrypt #VOXOX -ASV- 2010.01.18
	)
endif (UNIX AND NOT WIN32)

subdirs(
	pthread
	libstdcpp
	boost
	boost-test
	qt4
	libintl
	glib
	openssl
	curl
	alsa
	arts
	esd
#	ffmpeg
	tinyxml
	libxml
	gnutls
	iconv
	zlib
	libpurple
#	gaim
	uuid
	x11
	python
	samplerate
	sndfile
	#iaxclient
	psiidle
	#sqlite
	json		#VOXOX - JRT - 2009.06.26

)

if (ENABLE_CRASHREPORT)
	subdirs(
		googlebreakpad
	)
endif (ENABLE_CRASHREPORT)

#if (LINUX)
#	subdirs(
#		coredumper
#	)
#endif (LINUX)
