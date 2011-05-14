ow_create_static_library(owsound)

ow_use_public_libraries(
	owthread
)

ow_add_public_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

ow_add_private_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}/src
)

ow_add_sources(
	src/AudioDevice.cpp
	src/VolumeControl.cpp
	src/EnumDeviceType.cpp
	src/Sound.cpp
)

if (OWSOUND_PORTAUDIO_SUPPORT)
	ow_add_public_definitions(
		-DOWSOUND_PORTAUDIO_SUPPORT=1
	)
endif (OWSOUND_PORTAUDIO_SUPPORT)

if (WIN32)
	ow_add_private_libraries(
		winmm
		advapi32
	)

	ow_add_sources(
		src/win32/Sound.cpp
		src/win32/Win32VolumeControl.cpp
		src/win32/Win32AudioDevice.cpp
		src/win32/SoundThread.cpp
		src/win32/Win32AudioDeviceId.cpp
		src/win32/playsound/SoundFile.cpp
		src/win32/playsound/PlaySoundFile.cpp
	)

	if (OWSOUND_PORTAUDIO_SUPPORT)
		ow_add_sources(
			src/portaudio/AudioDeviceManager.cpp
		)
		ow_use_private_libraries(
			portaudio
		)
	else (OWSOUND_PORTAUDIO_SUPPORT)
		ow_add_sources(
			src/win32/AudioDeviceManager.cpp
		)
	endif(OWSOUND_PORTAUDIO_SUPPORT)
endif (WIN32)

if (APPLE)
	ow_add_sources(
		src/mac/AudioDeviceManager.cpp
		src/mac/CoreAudioUtilities.cpp
		src/mac/MacAudioDevice.cpp
		src/mac/MacSound.cpp
		src/mac/MacVolumeControl.cpp
		src/mac/Sound.cpp
	)
	ow_use_public_frameworks(
		AudioToolBox
		AudioUnit
		CoreAudio
		Cocoa
	)
endif (APPLE)

if (LINUX)
	ow_use_private_libraries(
		alsa
		sndfile
	)
	ow_add_sources(
		src/linux/AudioDeviceManager.cpp
		src/linux/LinuxAudioDevice.cpp
		src/linux/LinuxVolumeControl.cpp
		src/linux/LinuxSound.cpp
		src/linux/Sound.cpp
		src/linux/alsa_mixer.cpp
		src/linux/alsa_sndfile.cpp
	)
endif (LINUX)

ow_create_project_binary()

if (APPLE)
	set_target_properties(${PROJECT_NAME}
		PROPERTIES
			COMPILE_FLAGS "-x objective-c++"
	)
endif (APPLE)

subdirs(
	tests
)
