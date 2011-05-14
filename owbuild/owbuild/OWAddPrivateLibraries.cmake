# - ow_add_private_libraries(lib1 ... libN)
# Adds private link libraries to the current project
#
# Links current project/target with the given libraries
# Libraries specified using ow_add_public_libraries() should be accessible
# You can give a full path to a library to link with
# See ow_add_private_library_dirs() to add a library path to your project
#
# Term 'private' means that link libraries will be
# automatically used inside inherited projects
# See ow_add_public_libraries() for the 'public' version of this function
#
# GNU GCC Link Options: http://gcc.gnu.org/onlinedocs/gcc-4.1.1/gcc/Link-Options.html
# Microsoft Visual C++ Linker Options: http://msdn2.microsoft.com/en-us/library/y0zzbyt4.aspx
#
# Tips:
# If you link with unnecessary libraries it does not matter (besides the ugliness),
# the resulting program won't depend on these libraries if not needed
#
# Example:
# ow_create_executable(mytest)
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
# gcc mytest.c -lavcodec -o mytest
#
# Internally, ow_add_private_libraries() wrapps CMake function target_link_libraries()
# and defines variable ${PROJECT_NAME}_PRIVATE_LIBRARIES, see ow_finish_binary() for
# more details
#
# Copyright (C) 2006-2007  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


macro (ow_add_private_libraries)

	ow_check_project()

	if (${PROJECT_NAME}_PRIVATE_LIBRARIES)
		set(${PROJECT_NAME}_PRIVATE_LIBRARIES
			${${PROJECT_NAME}_PRIVATE_LIBRARIES}
			${ARGN}
		)
	else (${PROJECT_NAME}_PRIVATE_LIBRARIES)
		set(${PROJECT_NAME}_PRIVATE_LIBRARIES
			${ARGN}
		)
	endif (${PROJECT_NAME}_PRIVATE_LIBRARIES)

endmacro (ow_add_private_libraries)
