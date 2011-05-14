# - ow_create_project_binary(arg1 ... argN)
# Creates a binary (static/shared/plugin library or a executable) using the current project
#
# This function should be the last one to be called in a CMakeLists.txt
# The binary generated is named after ${PROJECT_NAME} variable
# The type of binary generated depends on the function ow_create_*() that you called before:
# - ow_create_executable()
# - ow_create_shared_library()
# - ow_create_static_library()
# - ow_create_plugin_library()
# This info is stored inside ${PROJECT_NAME}_PROJECT_TYPE variable
# that can be either Static, Shared, Plugin or Executable
# ${PROJECT_NAME}_PROJECT_TYPE variable can be overwritten via CMake cache
# This allow you to change a static library to a shared one for example
# even if ow_create_static_library() is used inside the CMakeLists.txt
#
# Binaries are copied to ${PROJECT_BINARY_COPY_DIR} (=${BUILD_DIR} by default) directory using ow_post_build_copy_file()
# Not all binaries are copied, only *.pdb, *.exe, *.so, *.dylib, *.dll, *.app
# .a and .lib binaries are not copied (not useful to have them inside ${BUILD_DIR})
#
# The variable PROJECT_BINARY_COPY_DIR may be set before calling this macro
# the path where to copy the resulting project binary.
#
# If you used ow_create_executable(), arguments can be:
# WIN32, MACOSX_BUNDLE, EXCLUDE_FROM_ALL...
# Check CMake documentation for full documentation
#
# Example:
# ow_create_executable(mytest)
#
# ow_add_sources(
# 	mytest.c
# )
#
# set(PROJECT_BINARY_COPY_DIR ${BUILD_DIR}/mydir/)
# ow_create_project_binary()
#
# Output generated:
# gcc mytext.c -o mytest
#
# Internally ow_create_project_binary() wrapps CMake functions add_library() and add_executable()
# ow_create_project_binary() calls ow_prepare_project_binary() first, performs some tasks and then calls
# ow_finish_project_binary() in order to complete the binary generation processus
#
# Copyright (C) 2006-2007  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


macro (ow_create_project_binary)

	ow_prepare_project_binary()

	if (${PROJECT_NAME}_PROJECT_TYPE STREQUAL Static)
		add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SRCS})
	endif (${PROJECT_NAME}_PROJECT_TYPE STREQUAL Static)

	# Default value for ${PROJECT_BINARY_COPY_DIR}
	if (NOT PROJECT_BINARY_COPY_DIR)
		set(PROJECT_BINARY_COPY_DIR ${BUILD_DIR})
	endif (NOT PROJECT_BINARY_COPY_DIR)

	# Shared library
	if (${PROJECT_NAME}_PROJECT_TYPE STREQUAL Shared)

		add_library(${PROJECT_NAME} SHARED ${${PROJECT_NAME}_SRCS})

		# Copy project binary
		if (MSVC)
			ow_post_build_copy_file(${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.dll ${PROJECT_BINARY_COPY_DIR}/)
			if (CMAKE_BUILD_TYPE STREQUAL Debug OR CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo)
				ow_post_build_copy_file(${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pdb ${PROJECT_BINARY_COPY_DIR}/)
			endif (CMAKE_BUILD_TYPE STREQUAL Debug OR CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo)
		endif (MSVC)

		if (MINGW)
			ow_post_build_copy_file(${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}.dll ${PROJECT_BINARY_COPY_DIR}/${PROJECT_NAME}.dll)
		endif (MINGW)

		if (UNIX AND NOT APPLE)
			ow_post_build_copy_file(${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}.so ${PROJECT_BINARY_COPY_DIR}/)
		endif (UNIX AND NOT APPLE)

	endif (${PROJECT_NAME}_PROJECT_TYPE STREQUAL Shared)

	# Plugin library
	if (${PROJECT_NAME}_PROJECT_TYPE STREQUAL Plugin)

		list(REMOVE_ITEM ${PROJECT_NAME}_LIBRARIES ${PROJECT_NAME})
		set(${PROJECT_NAME}_LIBRARIES
			${${PROJECT_NAME}_LIBRARIES}
			CACHE INTERNAL "${PROJECT_NAME} public libraries"
		)

		add_library(${PROJECT_NAME} MODULE ${${PROJECT_NAME}_SRCS})

		# Copy project binary
		if (MSVC)
			ow_post_build_copy_file(${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.dll ${PROJECT_BINARY_COPY_DIR}/)
			if (CMAKE_BUILD_TYPE STREQUAL Debug OR CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo)
				ow_post_build_copy_file(${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pdb ${PROJECT_BINARY_COPY_DIR}/)
			endif (CMAKE_BUILD_TYPE STREQUAL Debug OR CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo)
		endif (MSVC)

		if (MINGW)
			ow_post_build_copy_file(${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}.dll ${PROJECT_BINARY_COPY_DIR}/lib${PROJECT_NAME}.dll)
		endif (MINGW)

		if (UNIX AND NOT APPLE)
			ow_post_build_copy_file(${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}.so ${PROJECT_BINARY_COPY_DIR}/)
		endif (UNIX AND NOT APPLE)

	endif (${PROJECT_NAME}_PROJECT_TYPE STREQUAL Plugin)

	# Executable
	if (${PROJECT_NAME}_PROJECT_TYPE STREQUAL Executable)

		list(REMOVE_ITEM ${PROJECT_NAME}_LIBRARIES ${PROJECT_NAME})
		set(${PROJECT_NAME}_LIBRARIES
			${${PROJECT_NAME}_LIBRARIES}
			CACHE INTERNAL "${PROJECT_NAME} public libraries"
		)

		if (CMAKE_BUILD_TYPE STREQUAL Debug)
			# Remove WIN32 flag so it will compile with the console enabled
			# /SUBSYSTEM:CONSOLE = console mode
			# /SUBSYSTEM:WINDOWS = no console, graphical only

			# ARGN_BUGFIX
			# Bugfix: list(REMOVE_ITEM ${ARGN} WIN32)
			# does not work
			# neither if (ARGN) !!!!
			# no idea why
			# So we re-create the list without the WIN32 element

			set(ARGN_BUGFIX "")
			foreach (loop ${ARGN})
				if (NOT loop MATCHES WIN32)
					set(ARGN_BUGFIX
						${ARGN_BUGFIX}
						${loop}
					)
				endif (NOT loop MATCHES WIN32)
			endforeach (loop)

			add_executable(${PROJECT_NAME} ${ARGN_BUGFIX} ${${PROJECT_NAME}_SRCS})
		else (CMAKE_BUILD_TYPE STREQUAL Debug)
			add_executable(${PROJECT_NAME} ${ARGN} ${${PROJECT_NAME}_SRCS})
		endif (CMAKE_BUILD_TYPE STREQUAL Debug)

		# Copy project binary
		if (MSVC)
			ow_post_build_copy_file(${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.exe ${PROJECT_BINARY_COPY_DIR}/)
			if (CMAKE_BUILD_TYPE STREQUAL Debug OR CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo)
				ow_post_build_copy_file(${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pdb ${PROJECT_BINARY_COPY_DIR}/)
			endif (CMAKE_BUILD_TYPE STREQUAL Debug OR CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo)
		endif (MSVC)

		if (APPLE)
			# Quotes are needed around ${ARGN} because if empty on Mac OS X, function
			# call does not work.
			ow_list_contains("${ARGN}" MACOSX_BUNDLE result)
			if (result)
				ow_post_build_copy_dir(${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.app ${PROJECT_BINARY_COPY_DIR}/${PROJECT_NAME}.app)
			else (result)
				ow_post_build_copy_file(${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} ${PROJECT_BINARY_COPY_DIR}/)
			endif (result)
		endif (APPLE)

		if (UNIX AND NOT APPLE)
			ow_post_build_copy_file(${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} ${PROJECT_BINARY_COPY_DIR}/)
		endif (UNIX AND NOT APPLE)

	endif (${PROJECT_NAME}_PROJECT_TYPE STREQUAL Executable)

	ow_finish_project_binary()

endmacro (ow_create_project_binary)
