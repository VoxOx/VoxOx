# - Try to find aRts
# Once done this will define
#
#  ARTS_FOUND - system has aRts
#  ARTS_INCLUDE_DIRS - the aRts include directory
#  ARTS_LIBRARIES - Link these to use aRts
#  ARTS_DEFINITIONS - Compiler switches required for using aRts
#
#  Copyright (c) 2006 Andreas Schneider <mail@cynapses.org>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (ARTS_LIBRARIES AND ARTS_INCLUDE_DIRS)
  # in cache already
  set(ARTS_FOUND TRUE)
else (ARTS_LIBRARIES AND ARTS_INCLUDE_DIRS)
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  include(UseConfig)

  config(artsc-config _ARTSLinkFlags _ARTSCflags)

  set(ARTS_DEFINITIONS ${_ARTSCflags})

  find_path(ARTS_INCLUDE_DIR
    NAMES
      artsc.h
    PATHS
      ${_ARTSIncDir}
      /usr/include
      /usr/local/include
      /opt/local/include
      /opt/kde3/include
      /sw/include
    PATH_SUFFIXES
      artsc
  )

  find_library(ARTSC_LIBRARY
    NAMES
      artsc
    PATHS
      ${_ARTSLinkDir}
      /usr/lib
      /usr/lib/kde3
      /usr/local/lib
      /opt/local/lib
      /opt/kde3/lib
      /sw/lib
  )

  set(ARTS_INCLUDE_DIRS
    ${ARTS_INCLUDE_DIR}
  )
  set(ARTS_LIBRARIES
    ${ARTSC_LIBRARY}
  )

  if (ARTS_INCLUDE_DIRS AND ARTS_LIBRARIES)
    set(ARTS_FOUND TRUE)
  endif (ARTS_INCLUDE_DIRS AND ARTS_LIBRARIES)

  if (ARTS_FOUND)
    if (NOT ARTS_FIND_QUIETLY)
      message(STATUS "Found aRts: ${ARTS_LIBRARIES}")
    endif (NOT ARTS_FIND_QUIETLY)
  else (ARTS_FOUND)
    if (ARTS_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find aRts")
    endif (ARTS_FIND_REQUIRED)
  endif (ARTS_FOUND)

  # show the ARTS_INCLUDE_DIRS and ARTS_LIBRARIES variables only in the advanced view
  mark_as_advanced(ARTS_INCLUDE_DIRS ARTS_LIBRARIES)

endif (ARTS_LIBRARIES AND ARTS_INCLUDE_DIRS)
