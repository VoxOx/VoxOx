# - ow_copy_dir(src dst)
# Copies a directory to ${dst} only if ${src} is different (newer) than ${dst}
#
# See ow_copy_file()
#
# Example:
# ow_copy_dir(${CMAKE_CURRENT_SOURCE_DIR}/*.png ${BUILD_DIR}/.)
# Copies all *.png files from ${CMAKE_CURRENT_SOURCE_DIR} to ${BUILD_DIR} directory
#
# Copyright (C) 2006-2007  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


macro (ow_copy_dir src dst)

	file(GLOB fileList ${src})
	if (fileList)
		foreach (file ${fileList})
			ow_copy_file(${file} ${dst})
		endforeach (file ${fileList})
	endif (fileList)

endmacro (ow_copy_dir)
