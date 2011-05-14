# - ow_dump_owbuild()
# Internal function, dump OWBuild source code in one file
#
# This is for documentation propose: it is easier to make a search in one
# file where all source code is contained
#
# Copyright (C) 2007  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


macro (ow_dump_owbuild)

	set(owbuildPath ${CMAKE_SOURCE_DIR}/owbuild/owbuild)
	set(owbuildDumpFile ${owbuildPath}/owbuild-dump.txt)

	# Create dump file
	file(WRITE ${owbuildDumpFile} "")

	# File title
	file(APPEND ${owbuildDumpFile} "OWBuild Source Code Dump")

	file(GLOB fileList ${owbuildPath}/*.cmake)
	if (fileList)
		foreach (file ${fileList})
			file(READ ${file} output)
			get_filename_component(filename ${file} NAME)
			file(APPEND ${owbuildDumpFile} "\n\n*****  ${filename}  *****\n\n${output}")
		endforeach (file ${fileList})
	endif (fileList)

endmacro (ow_dump_owbuild)

#ow_dump_owbuild()
