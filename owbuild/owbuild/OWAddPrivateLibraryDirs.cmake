# - ow_add_private_library_dirs(dir1 ... dirN)
# Adds private link directories (directories in which to search for libraries
# e.g library path) to the current project
#
# Specifies the paths in which the linker should search for libraries
#
# Term 'private' means that link directories will be
# automatically used inside inherited projects
# See ow_add_public_library_dirs() for the 'public' version of this function
#
# GNU GCC Options for Directory Search: http://gcc.gnu.org/onlinedocs/gcc-4.1.1/gcc/Directory-Options.html
# Microsoft Visual C++ Additional Libpath: http://msdn2.microsoft.com/en-us/library/1xhzskbe.aspx
#
# Example:
# ow_create_executable(mytest)
#
# ow_add_private_libraries(
# 	#Path to FFmpeg library
# 	/path/to/libavcodec
# )
#
# ow_add_private_libraries(
# 	#FFmpeg library
# 	libavcodec.a
# )
#
# ow_add_sources(
# 	mytest.c
# )
#
# ow_create_project_binary()
#
# Output produced:
# gcc -L/path/to/libavcodec mytest.c -lavcodec -o mytest
#
# Internally ow_add_private_library_dirs() uses a variable named ${PROJECT_NAME}_PRIVATE_LIBRARY_DIRS
# It uses CMake function link_directories(), see ow_prepare_binary() for more details
#
# Copyright (C) 2006-2007  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


macro (ow_add_private_library_dirs)

	ow_check_project()

	if (${PROJECT_NAME}_PRIVATE_LIBRARY_DIRS)
		set(${PROJECT_NAME}_PRIVATE_LIBRARY_DIRS
			${${PROJECT_NAME}_PRIVATE_LIBRARY_DIRS}
			${ARGN}
			CACHE INTERNAL "${PROJECT_NAME} private library directories"
		)
	else (${PROJECT_NAME}_PRIVATE_LIBRARY_DIRS)
		set(${PROJECT_NAME}_PRIVATE_LIBRARY_DIRS
			${ARGN}
			CACHE INTERNAL "${PROJECT_NAME} private library directories"
		)
	endif (${PROJECT_NAME}_PRIVATE_LIBRARY_DIRS)

	ow_unique(unique ${${PROJECT_NAME}_PRIVATE_LIBRARY_DIRS})
	set(${PROJECT_NAME}_PRIVATE_LIBRARY_DIRS
		${unique}
		CACHE INTERNAL "${PROJECT_NAME} private library directories"
	)

endmacro (ow_add_private_library_dirs)
