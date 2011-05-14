# - ow_svn_checkout(url dst)
# Does a subversion checkout from an url to a destination directory ${CMAKE_CURRENT_SOURCE_DIR}/${dst}
#
# svn special options can be passed via ${ARGN}
# ow_svn_checkout() is a bloquant function that runs command line 'svn co'
# thus you need to install the subversion command line, check http://subversion.tigris.org/
# if you don't have it already
#
# Example:
# ow_svn_checkout(https://dev.openwengo.com/svn/openwengo/webcam/trunk libs/webcam -r 9053 -q)
#
# This will make a svn checkout from https://dev.openwengo.com/svn/openwengo/webcam/trunk (revision 9053
# and quiet mode) and copy it to directory ${CMAKE_CURRENT_SOURCE_DIR}/libs/webcam
#
# Copyright (C) 2007  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


macro (ow_svn_checkout url dst)

	if (NOT SUBVERSION_FOUND)
		find_package(Subversion REQUIRED)
	endif (NOT SUBVERSION_FOUND)

	execute_process(
		COMMAND
			${SUBVERSION_SVN_EXECUTABLE} co ${url} ${CMAKE_CURRENT_SOURCE_DIR}/${dst} ${ARGN}
	)

endmacro (ow_svn_checkout)
