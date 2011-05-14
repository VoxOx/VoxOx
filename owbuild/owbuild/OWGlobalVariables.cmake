# Defines global public variables
#
# CMAKE_BUILD_TYPE = set to Debug by default
# BUILD_TYPE = same as CMAKE_BUILD_TYPE but lowercase (debug, release, minsizerel...)
# BUILD_DIR = directory where compiled files will be copied, ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE} by default
# LOCALE_COPY_DIR = location for translations files (*.qm), ${BUILD_DIR}/lang by default
# SVN_REVISION = subversion revision number, see ow_get_svn_revision()
# CURRENT_DATE_TIME = current date time: 20070323121316 (e.g 2007-03-23 12:13:16)
#
# Useful CMake variables:
# http://www.cmake.org/Wiki/CMake_Useful_Variables
# All variables defined by CMake:
# http://www.cmake.org/Wiki/CMake_Useful_Variables/Get_Variables_From_CMake_Dashboards
#
# Copyright (C) 2006-2007  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


# Sets a global variable to test if OWBuild is already included or not in the build system
set(OWBUILD_INCLUDED TRUE)

# Sets the default build type to Debug
if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE
		Debug
		CACHE STRING "Choose the build type, options are: None Debug Release RelWithDebInfo MinSizeRel" FORCE
	)
endif (NOT CMAKE_BUILD_TYPE)

# Directory where compiled files will be copied, ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE} by default
# In lower case
string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
set(BUILD_DIR
	${CMAKE_BINARY_DIR}/${BUILD_TYPE}
	CACHE PATH "Build directory, depends on build type" FORCE
)

# Location for translation files (*.qm), ${BUILD_DIR}/lang by default
set(LOCALE_COPY_DIR
	"${BUILD_DIR}/lang"
	CACHE PATH "Location for translations (*.qm files)" FORCE
)
file(MAKE_DIRECTORY ${LOCALE_COPY_DIR})

# Gets svn revision
set(SVN_REVISION "0")
ow_get_svn_revision(SVN_REVISION)
# Bugfix with svn revision number that can integrate a : and
# this does not work under Windows for the installer name, replace it by -
string(REPLACE ":" "-" SVN_REVISION "${SVN_REVISION}")

# Gets current date time
set(CURRENT_DATE_TIME "0")
ow_get_current_date_time(CURRENT_DATE_TIME)
