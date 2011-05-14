# - ow_create_plugin_library(name)
# Creates a plugin library (e.g a shared library) given its name
#
# ow_create_plugin_library() creates a project named ${name} (${PROJECT_NAME})
# and works in combination with ow_create_project_binary()
#
# Example:
# ow_create_plugin_library(mytest)
#
# ow_add_sources(
# 	mytest.c
# )
#
# ow_create_project_binary()
#
# Output generated:
# gcc mytext.c -shared -o mytest.so
#
# Internally, ow_create_plugin_library() sets variable ${PROJECT_NAME}_PROJECT_TYPE
# to Plugin
# This variable will be used later on by ow_create_project_binary()
#
# Copyright (C) 2006  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


macro (ow_create_plugin_library name)

	ow_create_project(${name})

	set(${PROJECT_NAME}_PROJECT_TYPE
		Plugin
		CACHE STRING "${PROJECT_NAME} project type (Static, Shared, Plugin, Executable)"
	)

endmacro (ow_create_plugin_library)
