# - ow_add_public_definitions(def1 ... defN)
# Adds public -D define flags to the current project
#
# See ow_add_private_definitions() for complete documentation
#
# Term 'public' means that define flags will be
# automatically used inside inherited projects
#
# Internally ow_add_public_definitions() wrapps CMake function add_definitions()
# It defines variable ${PROJECT_NAME}_DEFINITIONS, see ow_prepare_binary() for more details
# ${PROJECT_NAME}_DEFINITIONS complies with modules conventions from
# http://www.cmake.org/cgi-bin/viewcvs.cgi/Modules/readme.txt?root=CMake&view=markup
#
# Copyright (C) 2006-2007  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


macro (ow_add_public_definitions)

	ow_check_project()

	if (${PROJECT_NAME}_DEFINITIONS)
		set(${PROJECT_NAME}_DEFINITIONS
			${${PROJECT_NAME}_DEFINITIONS}
			${ARGN}
			CACHE INTERNAL "${PROJECT_NAME} public definitions"
		)
	else (${PROJECT_NAME}_DEFINITIONS)
		set(${PROJECT_NAME}_DEFINITIONS
			${ARGN}
			CACHE INTERNAL "${PROJECT_NAME} public definitions"
		)
	endif (${PROJECT_NAME}_DEFINITIONS)

	ow_unique(unique ${${PROJECT_NAME}_DEFINITIONS})
	set(${PROJECT_NAME}_DEFINITIONS
		${unique}
		CACHE INTERNAL "${PROJECT_NAME} public definitions"
	)

endmacro (ow_add_public_definitions)
