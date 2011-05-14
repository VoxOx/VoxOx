# - ow_prepare_project_binary()
# Internal function, prepares the creation of a binary file, used by ow_create_project_binary()
#
# ow_prepare_project_binary() uses include_directories()
# and link_directories() CMake functions
# See ow_finish_project_binary() and ow_create_project_binary() for complete documentation
#
# Copyright (C) 2006-2007  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


macro (ow_prepare_project_binary)

	ow_check_project()

	if (NOT ${PROJECT_NAME}_SRCS)
		message(FATAL_ERROR "No sources added, use ow_add_sources()")
	endif (NOT ${PROJECT_NAME}_SRCS)

	ow_add_public_libraries(${PROJECT_NAME})

	ow_unique(unique ${${PROJECT_NAME}_INCLUDE_DIRS} ${${PROJECT_NAME}_PRIVATE_INCLUDE_DIRS})
	include_directories(${unique})

	ow_unique(unique ${${PROJECT_NAME}_LIBRARY_DIRS} ${${PROJECT_NAME}_PRIVATE_LIBRARY_DIRS})
	link_directories(${unique})

endmacro (ow_prepare_project_binary)
