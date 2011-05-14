# - ow_copy_dir_recursive(src dst)
# Copies recursively a directory to ${dst} only if ${src} is different (newer) than ${dst}
#
# See ow_copy_dir() and ow_copy_file()
#
# Example:
# ow_copy_dir_recursive(${CMAKE_CURRENT_SOURCE_DIR}/* ${BUILD_DIR}/.)
# Copies recursively ${CMAKE_CURRENT_SOURCE_DIR} directory to ${BUILD_DIR} directory
#
# Copyright (C) 2006-2007  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


macro (ow_copy_dir_recursive src dst)

	get_filename_component(relativePath ${src} PATH)

	file(GLOB_RECURSE fileList ${src})
	if (fileList)
		foreach (file ${fileList})
			file(RELATIVE_PATH path ${relativePath} ${file})
			get_filename_component(path ${path} PATH)

			ow_copy_file(${file} ${dst}/${path}/.)
		endforeach (file ${fileList})
	endif (fileList)

endmacro (ow_copy_dir_recursive)
