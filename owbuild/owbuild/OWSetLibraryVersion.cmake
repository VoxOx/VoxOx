# - ow_set_library_version(buildVersion apiVersion)
# Sets the library version (build + api) for the current project
#
# Example:
# ow_create_shared_library(mytest)
#
# ow_add_sources(
# 	mytest.c
# )
#
# ow_set_library_version(1.1.2 1)
#
# ow_create_project_binary()
#
# Output generated:
# libmytest-1.1.2.so
#
# Internally ow_set_library_version() uses variables named ${PROJECT_NAME}_BUILD_VERSION
# and ${PROJECT_NAME}_API_VERSION, see ow_finish_binary() for more details
#
# Copyright (C) 2006-2007  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


macro (ow_set_library_version buildVersion apiVersion)

	ow_check_project()

	set(${PROJECT_NAME}_BUILD_VERSION
		${buildVersion}
	)

	set(${PROJECT_NAME}_API_VERSION
		${apiVersion}
	)

endmacro (ow_set_library_version)
