# - ow_get_current_date_time(time)
# Gets the current date time
#
# Example:
# ow_get_current_date_time(time)
# message(${time})
#
# Output generated:
# 20070323121316
#
# which means: 2007-03-23 12:13:16 (YYYY-MM-DD HH:MM:SS)
#
# Copyright (C) 2007  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


macro (ow_get_current_date_time time)

	if (UNIX AND NOT WIN32)
		set(flags -DLINK_LIBRARIES=stdc++)
	endif (UNIX AND NOT WIN32)

	try_run(runResult compileResult ${CMAKE_BINARY_DIR}
		"${CMAKE_CURRENT_SOURCE_DIR}/owbuild/owbuild/getcurrentdatetime.cpp"
		OUTPUT_VARIABLE ${time}
		CMAKE_FLAGS ${flags}
	)
	string(REGEX MATCH "[0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9]" ${time} ${${time}})

endmacro (ow_get_current_date_time)
