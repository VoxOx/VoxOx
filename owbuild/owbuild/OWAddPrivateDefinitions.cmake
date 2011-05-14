# - ow_add_private_definitions(def1 ... defN)
# Adds private -D define flags to the current project
#
# -D define flags are added to the command line of C and C++ compilers
# A definitions or a define is for example: -DDEBUG, -DWIN32... and can be tested
# in C/C++ language via #ifdef #else preprocessor directives
#
# Term 'private' means that define flags will be
# automatically used inside inherited projects
# See ow_add_public_definitions() for the 'public' version of this function
#
# GNU GCC Preprocessor Options: http://gcc.gnu.org/onlinedocs/gcc-4.1.1/gcc/Preprocessor-Options.html
# Microsoft C/C++ Preprocessor Reference: http://msdn2.microsoft.com/en-us/library/y4skk93w.aspx
# Microsoft Visual C++ Compiler Options: http://msdn2.microsoft.com/en-us/library/9s7c9wdw.aspx
#
# Example:
# ow_create_executable(mytest)
#
# if (CMAKE_BUILD_TYPE STREQUAL Debug)
# 	ow_add_private_definitions(
# 		-DDEBUG
# 	)
# endif (CMAKE_BUILD_TYPE STREQUAL Debug)
#
# ow_add_private_definitions(
# 	-DUNICODE
# )
#
# ow_add_sources(
# 	mytest.c
# )
#
# ow_create_project_binary()
#
# Output produced in debug mode:
# gcc -DUNICODE -DDEBUG mytest.c -o mytest
#
# Internally ow_add_private_definitions() wrapps CMake function add_definitions()
# ow_add_private_definitions() adds defines to a variable (a list)
# named ${PROJECT_NAME}_PRIVATE_DEFINITIONS, see ow_prepare_binary() for more details
#
# Copyright (C) 2006-2007  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


macro (ow_add_private_definitions)

	ow_check_project()

	if (${PROJECT_NAME}_PRIVATE_DEFINITIONS)
		set(${PROJECT_NAME}_PRIVATE_DEFINITIONS
			${${PROJECT_NAME}_PRIVATE_DEFINITIONS}
			${ARGN}
		)
	else (${PROJECT_NAME}_PRIVATE_DEFINITIONS)
		set(${PROJECT_NAME}_PRIVATE_DEFINITIONS
			${ARGN}
		)
	endif (${PROJECT_NAME}_PRIVATE_DEFINITIONS)

	ow_unique(unique ${${PROJECT_NAME}_PRIVATE_DEFINITIONS})
	set(${PROJECT_NAME}_PRIVATE_DEFINITIONS
		${unique}
	)

endmacro (ow_add_private_definitions)
