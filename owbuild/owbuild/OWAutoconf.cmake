# - ow_autoconf(configureCommand makeCommand)
# Runs Autoconf tool
#
# Runs ${configureCommand} and then ${makeCommand}
# Both commands are executed from ${CMAKE_CURRENT_SOURCE_DIR} directory
#
# Example:
# ow_autoconf("./configure" "make")
#
# Copyright (C) 2006-2007  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


macro (ow_autoconf _configureCommand _configureOptions _makeCommand)

	execute_process(
		COMMAND
			${_configureCommand} ${_configureOptions}
		WORKING_DIRECTORY
			${CMAKE_CURRENT_SOURCE_DIR}
	)

	execute_process(
		COMMAND
			${_makeCommand}
		WORKING_DIRECTORY
			${CMAKE_CURRENT_SOURCE_DIR}
	)

endmacro (ow_autoconf)
