if (UNIX)

	if (NOT XML2_FOUND)
		find_package(XML2 REQUIRED)
	endif (NOT XML2_FOUND)

	ow_create_project(libxml)

	ow_use_public_libraries(
		XML2
	)

endif (UNIX)
