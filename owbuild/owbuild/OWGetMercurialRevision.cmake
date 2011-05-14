# - ow_get_hg_revision(revision)
# Gets current subversion revision number
#
# Performs a hg id command line on ${CMAKE_SOURCE_DIR} directory
# You need to install the mercurial command line, check http://www.selenic.com/mercurial
# if you don't have it already
# ow_get_hg_revision() is being called by OWInitializationInfo.cmake that sets
# HG_REVSION variable already thus you don't need to call ow_get_hg_revision()
#
# Example:
# set(HG_REVISION "0")
# ow_get_hg_revision(HG_REVISION)
# message(STATUS "hg revision: " ${HG_REVISION})
#
# Copyright (C) 2006  Andreas Schneider <mail@cynapses.org>
# Copyright (C) 2006-2007  Wengo
# Copyright (C) 2008       MBDSYS
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


macro (ow_get_hg_revision revision)

	if (NOT MERCURIAL_FOUND)
		find_package(Mercurial)
	endif (NOT MERCURIAL_FOUND)

	
	set(${output} 0)
	if (MERCURIAL_HG_EXECUTABLE)
		execute_process(
			COMMAND
				${MERCURIAL_HG_EXECUTABLE} id
			OUTPUT_VARIABLE output
		)

	else (MERCURIAL_HG_EXECUTABLE)
		message("Mercurial (hg) command line not found, it is recommended to install it")
	endif (MERCURIAL_HG_EXECUTABLE)

	STRING(REGEX REPLACE "\\+.*" "" rev "${output}")
	STRING(REGEX REPLACE " +.*" "" rev "${rev}")
	set(${revision} ${rev})

endmacro (ow_get_hg_revision)
