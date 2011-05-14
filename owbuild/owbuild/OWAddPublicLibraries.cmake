# - ow_add_public_libraries(lib1 ... libN)
# Adds public link libraries to the current project
#
# See ow_add_private_libraries() for complete documentation
#
# Term 'public' means that link libraries will be
# automatically used inside inherited projects
#
# Internally ow_add_public_libraries() wrapps CMake function target_link_libraries()
# It defines variable ${PROJECT_NAME}_LIBRARIES, see ow_finish_binary() for more details
# ${PROJECT_NAME}_LIBRARIES complies with modules conventions from
# http://www.cmake.org/cgi-bin/viewcvs.cgi/Modules/readme.txt?root=CMake&view=markup
#
# Copyright (C) 2006-2007  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


macro (ow_add_public_libraries)

	ow_check_project()

	if (${PROJECT_NAME}_LIBRARIES)
		set(${PROJECT_NAME}_LIBRARIES
			${${PROJECT_NAME}_LIBRARIES}
			${ARGN}
			CACHE INTERNAL "${PROJECT_NAME} public libraries"
		)
	else (${PROJECT_NAME}_LIBRARIES)
		set(${PROJECT_NAME}_LIBRARIES
			${ARGN}
			CACHE INTERNAL "${PROJECT_NAME} public libraries"
		)
	endif (${PROJECT_NAME}_LIBRARIES)

	ow_unique(unique ${${PROJECT_NAME}_LIBRARIES})
	set(${PROJECT_NAME}_LIBRARIES
		${unique}
		CACHE INTERNAL "${PROJECT_NAME} public libraries"
	)

endmacro (ow_add_public_libraries)
