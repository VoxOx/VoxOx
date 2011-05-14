if (LINUX)
	if (PHAPI_AUDIO_ARTS_SUPPORT)

		if (NOT ARTS_FOUND)
			find_package(ARTS REQUIRED)
		endif (NOT ARTS_FOUND)

		ow_create_project(arts)

		ow_use_public_libraries(
			ARTS
		)

	endif (PHAPI_AUDIO_ARTS_SUPPORT)
endif (LINUX)
