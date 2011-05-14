# - ow_add_private_link_flags(flag1 ... flagN)
# Adds private link flags to the current project
#
# Adds extra flags to the link step of a project/target
# You won't generally need to use this function
#
# Term 'private' means that link flags will be
# automatically used inside inherited projects
# See ow_add_public_link_flags() for the 'public' version of this function
#
# GNU GCC Link Options: http://gcc.gnu.org/onlinedocs/gcc-4.1.1/gcc/Link-Options.html
# Microsoft Visual C++ Linker Options: http://msdn2.microsoft.com/en-us/library/y0zzbyt4.aspx
#
# Example:
# ow_create_executable(mytest)
#
# ow_add_private_link_flags(
# 	/NODEFAULTLIB:MSVCRT.LIB
# )
#
# ow_add_sources(
# 	mytest.c
# )
#
# ow_create_project_binary()
#
# Output produced:
# cl /out:mytest /NODEFAULTLIB:MSVCRT.LIB
#
# Internally, ow_add_private_link_flags() wrapps CMake function set_target_properties()
# using LINK_FLAGS property and defines variable ${PROJECT_NAME}_PRIVATE_LINK_FLAGS,
# see ow_finish_binary() for more details
#
# Copyright (C) 2006-2007  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


macro (ow_add_private_link_flags)

	ow_check_project()

	if (${PROJECT_NAME}_PRIVATE_LINK_FLAGS)
		set(${PROJECT_NAME}_PRIVATE_LINK_FLAGS
			${${PROJECT_NAME}_PRIVATE_LINK_FLAGS}
			${ARGN}
		)
	else (${PROJECT_NAME}_PRIVATE_LINK_FLAGS)
		set(${PROJECT_NAME}_PRIVATE_LINK_FLAGS
			${ARGN}
		)
	endif (${PROJECT_NAME}_PRIVATE_LINK_FLAGS)

endmacro (ow_add_private_link_flags)
