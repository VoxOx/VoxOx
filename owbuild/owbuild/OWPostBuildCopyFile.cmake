# - ow_post_build_copy_file(src dst)
# Internal function, copies a file to ${dst} using the current project
#
# The copy command is run after target ${PROJECT_NAME} has been built (POST_BUILD option from CMake)
# Works the same as ow_copy_file()
#
# Example:
# ow_create_executable(mytest)
#
# ow_add_sources(
# 	mytest.c
# )
#
# ow_post_build_copy_file(${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.exe ${BUILD_DIR}/.)
#
# ow_create_project_binary(mytest)
#
# Copyright (C) 2006-2007  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


macro (ow_post_build_copy_file src dst)

	ow_check_project()

	# Creates directory if necessary
	get_filename_component(_path ${dst} PATH)
	file(MAKE_DIRECTORY ${_path})

	add_custom_command(
		TARGET
			${PROJECT_NAME}
			POST_BUILD
		COMMAND
			${CMAKE_COMMAND} -E copy_if_different
			\"${src}\"
			\"${dst}\"
		COMMENT
			"Copying file ${dst}"
	)

endmacro (ow_post_build_copy_file)
