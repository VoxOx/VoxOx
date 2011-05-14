if (LINUX)
	if (NOT ALSA_FOUND)
		# FIXME
		# Original KDE FindAlsa.cmake does not work under KUbuntu 6.10 AMD64
		# couldn't fix it, I didn't understand why
		# So I replaced it by a more 'standard' FindALSA.cmake file
		#find_package(Alsa REQUIRED)
		#ALSA_CONFIGURE_FILE(${CMAKE_BINARY_DIR}/config-alsa.h)
		find_package(ALSA REQUIRED)
	endif (NOT ALSA_FOUND)

	ow_create_project(alsa)

	ow_use_public_libraries(
		ALSA
	)
endif (LINUX)
