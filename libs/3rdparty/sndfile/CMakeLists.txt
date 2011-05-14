if (LINUX)
	if (NOT SNDFILE_FOUND)
		find_package(SndFile REQUIRED)
	endif (NOT SNDFILE_FOUND)

	ow_create_project(sndfile)

	ow_use_public_libraries(
		SNDFILE
	)
endif (LINUX)
