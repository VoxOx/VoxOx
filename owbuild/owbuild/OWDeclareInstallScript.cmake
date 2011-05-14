# - ow_declare_install_script(further_command)
# Declares a CMakeLists.txt that will be run when ${PROJECT_NAME}-install target is run
#
# These targets are used to install a target. They are dependent on ${PROJECT_NAME}.
#
# Permits to have not only one install target but many ${PROJECT_NAME}-install targets:
# allows more flexibility
# Creates directory ${CMAKE_BINARY_DIR}/install/${PROJECT_NAME} and copies generated files
# from install target inside
#
# Copyright (C) 2006-2007  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


macro (ow_declare_install_script)

	ow_check_project()

	file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/install/${PROJECT_NAME}")

	set(moreOptions "")
	if (WIN32)
		set(moreOptions "-G" "NMake Makefiles")
	endif (WIN32)

	if (ARGN)
		string(REPLACE ";" "\\\ " myCommand ${ARGN})
	endif (ARGN)

	add_custom_target(${PROJECT_NAME}-install
		COMMAND
			"${CMAKE_COMMAND}" ${moreOptions}
			"${CMAKE_CURRENT_SOURCE_DIR}/install"
			"-DSOURCE_DIR=${CMAKE_SOURCE_DIR}"
			"-DBINARY_DIR=${CMAKE_BINARY_DIR}"
			"-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}"
			"-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
			${ARGN}
		WORKING_DIRECTORY
			"${CMAKE_BINARY_DIR}/install/${PROJECT_NAME}"
		COMMENT
			"Install target for ${PROJECT_NAME}"
	)

	add_dependencies(${PROJECT_NAME}-install ${PROJECT_NAME})

endmacro (ow_declare_install_script)
