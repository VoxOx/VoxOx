# - Try to find Subversion (svn)
# Once done this will define
#
#  SUBVERSION_FOUND - true if the command line client was found
#  SUBVERSION_SVN_EXECUTABLE - path to svn command line client
#  SUBVERSION_SVN_VERSION_EXECUTABLE - path to svnversion command line client
#
#  Copyright (c) 2007 Wengo
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (SUBVERSION_SVN_EXECUTABLE AND SUBVERSION_SVN_VERSION_EXECUTABLE)
  # in cache already
  set(SUBVERSION_FOUND TRUE)
else (SUBVERSION_SVN_EXECUTABLE AND SUBVERSION_SVN_VERSION_EXECUTABLE)

  find_program(SUBVERSION_SVN_EXECUTABLE
    NAMES
      svn
    PATHS
      /sw/bin
      /usr/bin
      /usr/local/bin
      C:/Program Files/Subversion/bin
	  C:/Program Files/CollabNet Subversion Client
  )

  find_program(SUBVERSION_SVN_VERSION_EXECUTABLE
    NAMES
      svnversion
    PATHS
      /sw/bin
      /usr/bin
      /usr/local/bin
      C:/Program Files/Subversion/bin
	  C:/Program Files/CollabNet Subversion Client
  )

  if (SUBVERSION_SVN_EXECUTABLE AND SUBVERSION_SVN_VERSION_EXECUTABLE)
    set(SUBVERSION_FOUND TRUE)
  endif (SUBVERSION_SVN_EXECUTABLE AND SUBVERSION_SVN_VERSION_EXECUTABLE)

  if (SUBVERSION_FOUND)
    if (NOT Subversion_FIND_QUIETLY)
      message(STATUS "Found Subversion (svn): ${SUBVERSION_SVN_EXECUTABLE}")
    endif (NOT Subversion_FIND_QUIETLY)
  else (SUBVERSION_FOUND)
    if (Subversion_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find Subversion (svn)")
    endif (Subversion_FIND_REQUIRED)
  endif (SUBVERSION_FOUND)

  # show the SUBVERSION_SVN_EXECUTABLE and SUBVERSION_SVN_VERSION_EXECUTABLE variables only in the advanced view
  mark_as_advanced(SUBVERSION_SVN_EXECUTABLE SUBVERSION_SVN_VERSION_EXECUTABLE)

endif (SUBVERSION_SVN_EXECUTABLE AND SUBVERSION_SVN_VERSION_EXECUTABLE)
