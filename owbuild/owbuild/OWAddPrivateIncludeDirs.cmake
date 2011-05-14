# - ow_add_private_include_dirs(dir1 ... dirN)
# Adds private include directories to the current project
#
# Include directories specify where to search for include files (headers *.h)
# By default the directories are added to the head of the list of directories
# This default behavior can be changed by setting CMAKE_INCLUDE_DIRECTORIES_BEFORE to OFF
# inside OWDefaultConfig.cmake file
#
# Term 'private' means that include directories will be
# automatically used inside inherited projects
# See ow_add_public_include_dirs() for the 'public' version of this function
#
# GNU GCC Options for Directory Search: http://gcc.gnu.org/onlinedocs/gcc-4.1.1/gcc/Directory-Options.html
# /I Visual C++ Compiler Option: http://msdn2.microsoft.com/en-us/library/73f9s62w.aspx
#
# Tips:
# Try to minimize the number of include directories, this will increase a bit compilation time
# but keep in mind that "early optimization is evil"
#
# Example:
# ow_create_executable(mytest)
#
# ow_add_public_include_dirs(
# 	${CMAKE_CURRENT_SOURCE_DIR}/include
# )
#
# ow_add_sources(
# 	mytest.c
# )
#
# ow_create_project_binary()
#
# Output produced:
# gcc -I/home/me/myprogram/include mytest.c -o mytest
#
# Internally ow_add_private_include_dirs() uses a variable named ${PROJECT_NAME}_PRIVATE_INCLUDE_DIRS
# It wrapps CMake function include_directories(), see ow_prepare_binary() for more details
#
# Copyright (C) 2006-2007  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


macro (ow_add_private_include_dirs)

	ow_check_project()

	if (${PROJECT_NAME}_PRIVATE_INCLUDE_DIRS)
		set(${PROJECT_NAME}_PRIVATE_INCLUDE_DIRS
			${${PROJECT_NAME}_PRIVATE_INCLUDE_DIRS}
			${ARGN}
		)
	else (${PROJECT_NAME}_PRIVATE_INCLUDE_DIRS)
		set(${PROJECT_NAME}_PRIVATE_INCLUDE_DIRS
			${ARGN}
		)
	endif (${PROJECT_NAME}_PRIVATE_INCLUDE_DIRS)

endmacro (ow_add_private_include_dirs)
