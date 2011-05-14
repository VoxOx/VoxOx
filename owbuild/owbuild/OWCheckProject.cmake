# - ow_check_project()
# Internal function, checks if the project is valid or not
#
# Checks for the presence of the variable ${PROJECT_NAME}
#
# Copyright (C) 2006  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


macro (ow_check_project)

	if (NOT PROJECT_NAME)
		message(FATAL_ERROR "No project name defined, "
			"use ow_create_executable() or ow_create_plugin_library() or "
			"ow_create_shared_library() or ow_create_static_library() or "
			"ow_create_project() first")
	endif (NOT PROJECT_NAME)

endmacro (ow_check_project)
