# - ow_change_identification_name(file name)
# Mac OS X only.
# Calls "install_name_tool -id ${file} ${name}"
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


macro (ow_change_identification_name file name)

	if (APPLE)
		exec_program("install_name_tool"
			ARGS
				"-id ${name} ${file}"
		)
	endif (APPLE)

endmacro (ow_change_identification_name)
