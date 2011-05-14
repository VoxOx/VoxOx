# - ow_add_sources(src1 ... srcN)
# Adds source files (.cpp, .c...) to the current project
#
# Example:
# ow_create_executable(mytest)
#
# ow_add_sources(
# 	mytest.c
# )
#
# ow_create_project_binary()
#
# Output produced:
# gcc mytest.c -o mytest
#
# Internally ow_add_sources() wrapps CMake function add_library or add_executable()
# It defines variable ${PROJECT_NAME}_SRCS, see ow_create_project_binary() for more details
#
# Copyright (C) 2006-2007  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


macro (ow_add_sources)

	ow_check_project()

	if (${PROJECT_NAME}_SRCS)
		set(${PROJECT_NAME}_SRCS
			${${PROJECT_NAME}_SRCS}
			${ARGN}
		)
	else (${PROJECT_NAME}_SRCS)
		set(${PROJECT_NAME}_SRCS
			${ARGN}
		)
	endif (${PROJECT_NAME}_SRCS)

endmacro (ow_add_sources)
