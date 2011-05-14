# - ow_add_public_library_dirs(dir1 ... dirN)
# Adds public link directories (directories in which to search for libraries
# e.g library path) to the current project
#
# See ow_add_private_library_dirs() for complete documentation
#
# Term 'public' means that link directories will be
# automatically used inside inherited projects
#
# Internally ow_add_public_library_dirs() wrapps CMake function link_directories()
# It defines variable ${PROJECT_NAME}_LIBRARY_DIRS, see ow_prepare_binary() for more details
# ${PROJECT_NAME}_LIBRARY_DIRS complies with modules conventions from
# http://www.cmake.org/cgi-bin/viewcvs.cgi/Modules/readme.txt?root=CMake&view=markup
#
# Copyright (C) 2006-2007  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


macro (ow_add_public_library_dirs)

	ow_check_project()

	if (${PROJECT_NAME}_LIBRARY_DIRS)
		set(${PROJECT_NAME}_LIBRARY_DIRS
			${${PROJECT_NAME}_LIBRARY_DIRS}
			${ARGN}
			CACHE INTERNAL "${PROJECT_NAME} public library directories"
		)
	else (${PROJECT_NAME}_LIBRARY_DIRS)
		set(${PROJECT_NAME}_LIBRARY_DIRS
			${ARGN}
			CACHE INTERNAL "${PROJECT_NAME} public library directories"
		)
	endif (${PROJECT_NAME}_LIBRARY_DIRS)

	ow_unique(unique ${${PROJECT_NAME}_LIBRARY_DIRS})
	set(${PROJECT_NAME}_LIBRARY_DIRS
		${unique}
		CACHE INTERNAL "${PROJECT_NAME} public library directories"
	)

endmacro (ow_add_public_library_dirs)
