# - ow_hdiutil(volname src dst)
# Mac OS X only.
#  calls "hdiutil create -fs HFS+ -volname ${volname} -srcfolder ${src} ${dst}"
#
# hdiutil creates a .dmg file.
#
# Copyright (C) 2006-2007  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


macro (ow_hdiutil volname src dst)

	if (APPLE)
		execute_process(
			COMMAND
				"hdiutil" create -fs HFS+ -volname ${volname} -srcfolder ${src} ${dst}
			RESULT_VARIABLE _result
		)
		if (NOT _result EQUAL 0)
			message(FATAL_ERROR "hdiutil failed with code ${_result}")
		endif (NOT _result EQUAL 0)
	endif (APPLE)

endmacro (ow_hdiutil)
