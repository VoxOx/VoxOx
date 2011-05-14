# - ow_copy_file(src dst)
# Copies a file to ${dst} only if ${src} is different (newer) than ${dst}
#
# See ow_copy_dir() and ow_copy_dir_recursive()
#
# Example:
# ow_copy_file(${CMAKE_CURRENT_SOURCE_DIR}/icon.png ${BUILD_DIR}/.)
# Copies file icon.png to ${BUILD_DIR} directory
#
# Copyright (C) 2006-2007  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


macro (ow_copy_file src dst)

	# Removes all path containing .svn or CVS or CMakeLists.txt during the copy
	if (NOT ${src} MATCHES ".*\\.svn|CVS|CMakeLists\\.txt.*")

		if (CMAKE_VERBOSE_MAKEFILE)
			message(STATUS "Copy file from ${src} to ${dst}")
		endif (CMAKE_VERBOSE_MAKEFILE)

		# Creates directory if necessary
		get_filename_component(_path ${dst} PATH)
		file(MAKE_DIRECTORY ${_path})

		exec_program(
				${CMAKE_COMMAND}
			ARGS
				"-E copy_if_different \"${src}\" \"${dst}\""
		)
	endif (NOT ${src} MATCHES ".*\\.svn|CVS|CMakeLists\\.txt.*")

endmacro (ow_copy_file)
