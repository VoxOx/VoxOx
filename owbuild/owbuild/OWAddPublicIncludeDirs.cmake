# - ow_add_public_include_dirs(dir1 ... dirN)
# Adds public include directories to the current project
#
# See ow_add_private_include_dirs() for complete documentation
#
# Term 'public' means that include directories will be
# automatically used inside inherited projects
#
# Internally ow_add_public_include_dirs() wrapps CMake function include_directories()
# It defines variable ${PROJECT_NAME}_INCLUDE_DIRS, see ow_prepare_binary() for more details
# ${PROJECT_NAME}_INCLUDE_DIRS complies with modules conventions from
# http://www.cmake.org/cgi-bin/viewcvs.cgi/Modules/readme.txt?root=CMake&view=markup
#
# Copyright (C) 2006-2007  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


macro (ow_add_public_include_dirs)

	ow_check_project()

	if (${PROJECT_NAME}_INCLUDE_DIRS)
		set(${PROJECT_NAME}_INCLUDE_DIRS
			${${PROJECT_NAME}_INCLUDE_DIRS}
			${ARGN}
			CACHE INTERNAL "${PROJECT_NAME} public include directories"
		)
	else (${PROJECT_NAME}_INCLUDE_DIRS)
		set(${PROJECT_NAME}_INCLUDE_DIRS
			${ARGN}
			CACHE INTERNAL "${PROJECT_NAME} public include directories"
		)
	endif (${PROJECT_NAME}_INCLUDE_DIRS)

	ow_unique(unique ${${PROJECT_NAME}_INCLUDE_DIRS})
	set(${PROJECT_NAME}_INCLUDE_DIRS
		${unique}
		CACHE INTERNAL "${PROJECT_NAME} public include directories"
	)

endmacro (ow_add_public_include_dirs)
