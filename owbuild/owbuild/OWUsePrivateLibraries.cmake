# - ow_use_private_libraries(lib1 ... libN)
# Uses privately a library inside the current project: imports properties from a library to the current project
#
# ow_use_private_libraries() and ow_use_public_libraries() implement
# OWBuild inheritance system
#
# A project (ow_create_executable(), ow_create_shared_library()...)
# has public/private attributes (changed via ow_add_public_include_dirs(),
# ow_add_public_definitions()...)
# If a new project inherits from another, all public attributes from
# the last one will be inherited and become attributes of the new one
# This means that project B public include dirs, definitions, compile flags, link flags...
# will get into project A if A inherits from B
# Of course if project B inherits publicly from a project C (and D...), then A will get all
# the public attributes from B and C (and D...)
#
#       C     D
#        \   /
#         \ /
#          B
#           \
#            \
#             A (<- inherits from B that inherits from C and D)
#
# You can inherit from a project publicly or privately, this will
# respectively propagate inherited attributes to subproject or not
#
# This has to be seen as mutli-inheritance from Oriented Object Programming,
# thus UML diagrams perfectly fit the design sep of a build system based on OWBuild
#
# Of course OWBuild does not provide full-feature inheritance but only a subset
# that fits the needs for a simple and yet powerful build system
# If you are not familiar with inheritance, check:
# http://en.wikipedia.org/wiki/Inheritance_%28computer_science%29
#
# Term 'private' means that public attributes inherited will become
# private attributes once inherited and won't be propagated to subprojects
# See ow_use_public_libraries() for the 'public' version of this function
#
# Example:
# ow_create_shared_library(mylib)
#
# ow_add_public_include_dirs(
# 	${CMAKE_CURRENT_SOURCE_DIR}/include
# )
#
# ow_add_public_definitions(
# 	-DMYDEFINE
# )
#
# ow_add_sources(
# 	mylib.c
# )
#
# ow_create_project_binary()
#
#
# ow_create_executable(mytest)
#
# ow_use_private_libraries(
# 	mylib
# )
#
# ow_add_sources(
# 	mytest.c
# )
#
# ow_create_project_binary()
#
# Output generated:
# gcc -I/home/me/mylib/include -DMYDEFINE mytest.c -lmylib -o mytest
#
# Project mytest gets -I/home/me/mylib/include, -DMYDEFINE, -lmylib
# from project mylib
#
# Internally ow_use_private_libraries() uses variables ${PROJECT_NAME}_INCLUDE_DIRS,
# ${PROJECT_NAME}_LIBRARIES, ${PROJECT_NAME}_LIBRARY_DIRS, ${PROJECT_NAME}_DEFINITIONS,
# ${PROJECT_NAME}_PUBLIC_LINK_FLAGS...
# All these variables are project attributes, ow_add_public_*() ow_add_private_*() are
# accessors (setters)
#
# Copyright (C) 2006-2007  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


macro (ow_use_private_libraries)

	ow_check_project()

	foreach (loop ${ARGN})
		if (NOT ${loop}_INCLUDE_DIRS)
			if (NOT ${loop}_INCLUDE_DIR)
				message(FATAL_ERROR "${PROJECT_NAME}: ${loop}_INCLUDE_DIRS and ${loop}_INCLUDE_DIR empty,"
					" check that ${loop} is declared before ${PROJECT_NAME}")
			endif (NOT ${loop}_INCLUDE_DIR)
		endif (NOT ${loop}_INCLUDE_DIRS)
		ow_add_private_include_dirs(
			${${loop}_INCLUDE_DIRS}
			${${loop}_INCLUDE_DIR}
		)

		#if (NOT ${loop}_LIBRARIES)
		#	message(FATAL_ERROR "${loop}_LIBRARIES empty")
		#endif (NOT ${loop}_LIBRARIES)
		if (${loop}_LIBRARIES)
			ow_add_private_libraries(
				${${loop}_LIBRARIES}
			)
		endif (${loop}_LIBRARIES)
		if (${loop}_LIBRARY)
			ow_add_private_libraries(
				${${loop}_LIBRARY}
			)
		endif (${loop}_LIBRARY)

		if (${loop}_LIBRARY_DIRS)
			ow_add_private_library_dirs(
				${${loop}_LIBRARY_DIRS}
			)
		endif (${loop}_LIBRARY_DIRS)

		if (${loop}_DEFINITIONS)
			ow_add_private_definitions(
				${${loop}_DEFINITIONS}
			)
		endif (${loop}_DEFINITIONS)

		if (${loop}_PUBLIC_LINK_FLAGS)
			ow_add_private_link_flags(
				${${loop}_PUBLIC_LINK_FLAGS}
			)
		endif (${loop}_PUBLIC_LINK_FLAGS)
	endforeach (loop)

endmacro (ow_use_private_libraries)
