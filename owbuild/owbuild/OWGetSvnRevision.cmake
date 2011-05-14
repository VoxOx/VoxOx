# - ow_get_svn_revision(revision)
# Gets current subversion revision number
#
# Performs a svnversion command line on ${CMAKE_SOURCE_DIR} directory
# You need to install the subversion command line, check http://subversion.tigris.org/
# if you don't have it already
# ow_get_svn_revision() is being called by OWInitializationInfo.cmake that sets
# SVN_REVSION variable already thus you don't need to call ow_get_svn_revision()
#
# Example:
# set(SVN_REVISION "0")
# ow_get_svn_revision(SVN_REVISION)
# message(STATUS "svn revision: " ${SVN_REVISION})
#
# Copyright (C) 2006  Andreas Schneider <mail@cynapses.org>
# Copyright (C) 2006-2007  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


macro (ow_get_svn_revision revision)
	if (NOT SUBVERSION_FOUND)
		find_package(Subversion)
	endif (NOT SUBVERSION_FOUND)

	find_file(SVN_DOT_DIR
		NAMES
			entries
		PATHS
			${CMAKE_SOURCE_DIR}/.svn
	)

	if (SUBVERSION_SVN_VERSION_EXECUTABLE AND SVN_DOT_DIR)
		execute_process(
			COMMAND
				${SUBVERSION_SVN_VERSION_EXECUTABLE} ${CMAKE_SOURCE_DIR} --no-newline
			RESULT_VARIABLE
				SVN_REVISION_RESULT_VARIABLE
			OUTPUT_VARIABLE
				SVN_REVISION_OUTPUT_VARIABLE
		)

		if (SVN_REVISION_RESULT_VARIABLE EQUAL 0)
			string(REGEX MATCH "^[0-9]+" ${revision} ${SVN_REVISION_OUTPUT_VARIABLE})
		else (SVN_REVISION_RESULT_VARIABLE EQUAL 0)
			set(${revision} 0)
		endif (SVN_REVISION_RESULT_VARIABLE EQUAL 0)
	else (SUBVERSION_SVN_VERSION_EXECUTABLE AND SVN_DOT_DIR)
		message("Subversion (svn) command line not found, it is recommended to install it")
	endif (SUBVERSION_SVN_VERSION_EXECUTABLE AND SVN_DOT_DIR)

endmacro (ow_get_svn_revision)
