# For Qt-4 translations

add_custom_target(lupdate
	COMMAND
		"${CMAKE_COMMAND}"
		-DPRO_FILE="${CMAKE_SOURCE_DIR}/lang.pro"
		-P "${CMAKE_SOURCE_DIR}/owbuild/owbuild/OWLocaleUpdate.cmake"
	WORKING_DIRECTORY
		${CMAKE_SOURCE_DIR}
)

add_custom_target(lrelease
	COMMAND
		"${CMAKE_COMMAND}"
		-DLOCALE_COPY_DIR="${LOCALE_COPY_DIR}"
		-P "${CMAKE_SOURCE_DIR}/owbuild/owbuild/OWLocaleRelease.cmake"
	WORKING_DIRECTORY
		${CMAKE_SOURCE_DIR}
)
