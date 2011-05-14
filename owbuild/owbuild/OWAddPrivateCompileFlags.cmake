# - ow_add_private_compile_flags(flag1 ... flagN)
# Adds private compilation flags to the current project
#
# A compilation flag is a command line option given to the compiler when it is
# compiling a file, for example it can be for GNU GCC: -O3 (optimisation level),
# -g (compile with debug symbol), -Wall (enable warnings)...
#
# Term 'private' means that compile flags will be
# automatically used inside inherited projects
# See ow_add_public_compile_flags() for the 'public' version of this function
#
# GNU GCC Command Options: http://gcc.gnu.org/onlinedocs/gcc-4.1.1/gcc/Invoking-GCC.html
# Microsoft Visual C++ Compiler Options: http://msdn2.microsoft.com/en-us/library/9s7c9wdw.aspx
#
# Example:
# ow_create_executable(mytest)
#
# ow_add_private_compile_flags(
# 	-Wall
#	-g
# )
#
# ow_add_sources(
# 	mytest.c
# )
#
# ow_create_project_binary()
#
# Output produced:
# gcc -g -Wall mytest.c -o mytest
#
# Internally ow_add_private_compile_flags() uses a variable named ${PROJECT_NAME}_PRIVATE_COMPILE_FLAGS
# It uses CMake function set_target_properties() with COMPILE_FLAGS property, see ow_finish_binary()
# for more details
#
# Copyright (C) 2006-2007  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


macro (ow_add_private_compile_flags)

	ow_check_project()

	if (${PROJECT_NAME}_PRIVATE_COMPILE_FLAGS)
		set(${PROJECT_NAME}_PRIVATE_COMPILE_FLAGS
			${${PROJECT_NAME}_PRIVATE_COMPILE_FLAGS}
			${ARGN}
		)
	else (${PROJECT_NAME}_PRIVATE_COMPILE_FLAGS)
		set(${PROJECT_NAME}_PRIVATE_COMPILE_FLAGS
			${ARGN}
		)
	endif (${PROJECT_NAME}_PRIVATE_COMPILE_FLAGS)

endmacro (ow_add_private_compile_flags)
