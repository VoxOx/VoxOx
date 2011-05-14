# - Try to find OSIP2
# Once done this will define
#
#  OSIP2_FOUND - system has OSIP2
#  OSIP2_INCLUDE_DIRS - the OSIP2 include directory
#  OSIP2_LIBRARIES - Link these to use OSIP2
#  OSIP2_DEFINITIONS - Compiler switches required for using OSIP2
#
#  Copyright (c) 2006 Andreas Schneider <mail@cynapses.org>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (OSIP2_LIBRARIES AND OSIP2_INCLUDE_DIRS)
  # in cache already
  set(OSIP2_FOUND TRUE)
else (OSIP2_LIBRARIES AND OSIP2_INCLUDE_DIRS)
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  include(UsePkgConfig)

  pkgconfig(libosip2 _OSIP2IncDir _OSIP2LinkDir _OSIP2LinkFlags _OSIP2Cflags)

  set(OSIP2_DEFINITIONS ${_OSIP2Cflags})

  find_path(OSIP2_INCLUDE_DIR
    NAMES
      osip2/osip.h
    PATHS
      ${_OSIP2IncDir}
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
  )

  find_path(OSIPPARSER2_INCLUDE_DIR
    NAMES
      osipparser2/osip_parser.h
    PATHS
      ${_OSIP2IncDir}
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
  )

  find_library(OSIPPARSER2_LIBRARY
    NAMES
      osipparser2
    PATHS
      ${_OSIP2LinkDir}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )
  find_library(OSIP2_LIBRARY
    NAMES
      osip2
    PATHS
      ${_OSIP2LinkDir}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  set(OSIP2_INCLUDE_DIRS
    ${OSIP2_INCLUDE_DIR}
    ${OSIPPARSER2_INCLUDE_DIR}
  )
  set(OSIP2_LIBRARIES
    ${OSIPPARSER2_LIBRARY}
    ${OSIP2_LIBRARY}
)

  if (OSIP2_INCLUDE_DIRS AND OSIP2_LIBRARIES)
     set(OSIP2_FOUND TRUE)
  endif (OSIP2_INCLUDE_DIRS AND OSIP2_LIBRARIES)

  if (OSIP2_FOUND)
    if (NOT OSIP2_FIND_QUIETLY)
      message(STATUS "Found OSIP2: ${OSIP2_LIBRARIES}")
    endif (NOT OSIP2_FIND_QUIETLY)
  else (OSIP2_FOUND)
    if (OSIP2_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find OSIP2")
    endif (OSIP2_FIND_REQUIRED)
  endif (OSIP2_FOUND)

  # show the OSIP2_INCLUDE_DIRS and OSIP2_LIBRARIES variables only in the advanced view
  mark_as_advanced(OSIP2_INCLUDE_DIRS OSIP2_LIBRARIES)

endif (OSIP2_LIBRARIES AND OSIP2_INCLUDE_DIRS)

