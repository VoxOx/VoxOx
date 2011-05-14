if (UNIX)

	ow_create_project(uuid)

	if (APPLE)
		ow_use_public_frameworks(
			CoreFoundation
		)
	else (APPLE)
		if (NOT UUID_FOUND)
			find_package(UUID REQUIRED)
		endif (NOT UUID_FOUND)

		ow_use_public_libraries(
			UUID
		)
	endif (APPLE)

endif (UNIX)
