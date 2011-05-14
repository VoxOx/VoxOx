# - ow_change_install_name(src dst file)
# Mac OS X only.
# Calls "install_name_tool -change ${src} ${dst} ${file}"
#
# install_name_tool allows to change information about where an application
# looks for libraries or what these libraries are called
#
# Manual Page For install_name_tool():
# http://developer.apple.com/documentation/Darwin/Reference/ManPages/man1/install_name_tool.1.html
# Deploying Applications on MacOSX from Trolltech:
# http://doc.trolltech.com/qq/qq09-mac-deployment.html
#
# Copyright (C) 2006-2007  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


macro (ow_change_install_name src dst file)

	if (APPLE)
		exec_program("install_name_tool"
			ARGS
				"-change ${src} ${dst} ${file}"
		)
	endif (APPLE)

endmacro (ow_change_install_name)
