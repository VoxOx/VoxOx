include(CMakeLists-owbuild.txt)
cmake_minimum_required(VERSION 2.4.6)

project(voxox)

# Global variables
set(APPLICATION_NAME VoxOx  CACHE STRING "Name of application")
set(APPLICATION_VERSION "2.0.6" CACHE STRING "Version of application")

# Split APPLICATION_VERSION into separate vars:
# If APPLICATION_VERSION is x.y.z, then APPLICATION_VERSION_1 will be x,
# APPLICATION_VERSION_2 will be y and so on.
set(tmp "${APPLICATION_VERSION}.0.0.0")
set(tmp_rx "([0-9]+)\\.([0-9]+)\\.([0-9]+)\\.([0-9]+).*")
string(REGEX REPLACE ${tmp_rx} "\\1" APPLICATION_VERSION_1 "${tmp}")
string(REGEX REPLACE ${tmp_rx} "\\2" APPLICATION_VERSION_2 "${tmp}")
string(REGEX REPLACE ${tmp_rx} "\\3" APPLICATION_VERSION_3 "${tmp}")
string(REGEX REPLACE ${tmp_rx} "\\4" APPLICATION_VERSION_4 "${tmp}")

# Add options values
include(DefineWengoOptions.cmake)
include(DefineInstallationPaths.cmake)
include(DefineCPackDefaults.cmake)

subdirs(
	libs/3rdparty
)

if (WIN32)
	subdirs(
		libs/3rdparty/portaudio
	)
endif (WIN32)

subdirs(
	libs/owutil
	libs/qtutil
	libs/timer
	libs/pixertool
	libs/webcam
	wifo
	libs/wenbox
	libs/owbrowser
	libs/trayicon
	libs/softupdater
	libs/sound
	libs/imwrapper
#	libs/imwrapper/src/gaim
        libs/imwrapper/src/purple
	libs/sipwrapper
	libs/sipwrapper/src/phapi
	libs/imwrapper/src/multiim
	wengophone
        crashreport
)
