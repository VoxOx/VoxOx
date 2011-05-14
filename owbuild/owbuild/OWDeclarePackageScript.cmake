# - ow_declare_package_script()
# Declares a CMakeLists.txt that will be run when ${PROJECT_NAME}-package target is run
#
# These targets are used to generate a distributable package. They are dependent on
# ${PROJECT_NAME}-install
#
# Copyright (C) 2006-2007  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


macro (ow_declare_package_script)

	ow_check_project()

	file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/package/${PROJECT_NAME}")

	set(moreOptions "")
	if (WIN32)
		set(moreOptions "-G" "NMake Makefiles")
	endif (WIN32)

	if (ARGN)
		string(REPLACE ";" "\\\ " myCommand ${ARGN})
	endif (ARGN)

	add_custom_target(${PROJECT_NAME}-package
		COMMAND
			"${CMAKE_COMMAND}" ${moreOptions}
			"${CMAKE_CURRENT_SOURCE_DIR}/package"
			"-DSOURCE_DIR=${CMAKE_SOURCE_DIR}"
			"-DBINARY_DIR=${CMAKE_BINARY_DIR}"
			"-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}"
			"-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
			${ARGN}
		WORKING_DIRECTORY
			"${CMAKE_BINARY_DIR}/package/${PROJECT_NAME}"
		COMMENT
			"Package target for ${PROJECT_NAME}"
	)

	add_dependencies(${PROJECT_NAME}-package ${PROJECT_NAME}-install)

endmacro (ow_declare_package_script)
