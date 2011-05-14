# - ow_add_public_link_flags(flag1 ... flagN)
# Adds public link flags to the current project
#
# See ow_add_private_link_flags() for complete documentation
#
# Term 'public' means that link flags will be
# automatically used inside inherited projects
#
# Internally ow_add_public_link_flags() wrapps CMake function set_target_properties()
# using LIN_FLAGS property
# It defines variable ${PROJECT_NAME}_PUBLIC_LINK_FLAGS, see ow_finish_binary() for more details
#
# Copyright (C) 2006-2007  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


macro (ow_add_public_link_flags)

	ow_check_project()

	if (${PROJECT_NAME}_PUBLIC_LINK_FLAGS)
		set(${PROJECT_NAME}_PUBLIC_LINK_FLAGS
			${${PROJECT_NAME}_PUBLIC_LINK_FLAGS}
			${ARGN}
			CACHE INTERNAL "${PROJECT_NAME} public link flags"
		)
	else (${PROJECT_NAME}_PUBLIC_LINK_FLAGS)
		set(${PROJECT_NAME}_PUBLIC_LINK_FLAGS
			${ARGN}
			CACHE INTERNAL "${PROJECT_NAME} public link flags"
		)
	endif (${PROJECT_NAME}_PUBLIC_LINK_FLAGS)

	ow_unique(unique ${${PROJECT_NAME}_PUBLIC_LINK_FLAGS})
	set(${PROJECT_NAME}_PUBLIC_LINK_FLAGS
		${unique}
		CACHE INTERNAL "${PROJECT_NAME} public link flags"
	)

endmacro (ow_add_public_link_flags)
