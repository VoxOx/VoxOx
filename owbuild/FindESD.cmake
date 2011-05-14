# - Try to find ESD (Esound)
# Once done this will define
#
#  ESD_FOUND - system has ESD
#  ESD_INCLUDE_DIRS - the ESD include directory
#  ESD_LIBRARIES - Link these to use ESD
#  ESD_DEFINITIONS - Compiler switches required for using ESD
#
#  Copyright (c) 2006 Andreas Schneider <mail@cynapses.org>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (ESD_LIBRARIES AND ESD_INCLUDE_DIRS)
  # in cache already
  set(ESD_FOUND TRUE)
else (ESD_LIBRARIES AND ESD_INCLUDE_DIRS)
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  include(UseConfig)

  config(esd-config _ESDLinkFlags _ESDCflags)

  set(ESD_DEFINITIONS ${_ESDCflags})

  find_path(ESD_INCLUDE_DIR
    NAMES
      esd.h
    PATHS
      ${_ESDIncDir}
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
  )

  find_library(ESD_LIBRARY
    NAMES
      esd
    PATHS
      ${_ESDLinkDir}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )
  find_library(ESDDSP_LIBRARY
    NAMES
      esddsp
    PATHS
      ${_ESDLinkDir}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  set(ESD_INCLUDE_DIRS
    ${ESD_INCLUDE_DIR}
  )
  set(ESD_LIBRARIES
    ${ESD_LIBRARY}
    ${ESDDSP_LIBRARY}
  )

  if (ESD_INCLUDE_DIRS AND ESD_LIBRARIES)
    set(ESD_FOUND TRUE)
  endif (ESD_INCLUDE_DIRS AND ESD_LIBRARIES)

  if (ESD_FOUND)
    if (NOT ESD_FIND_QUIETLY)
      message(STATUS "Found ESD (Esound): ${ESD_LIBRARIES}")
    endif (NOT ESD_FIND_QUIETLY)
  else (ESD_FOUND)
    if (ESD_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find ESD (Esound)")
    endif (ESD_FIND_REQUIRED)
  endif (ESD_FOUND)

  # show the ESD_INCLUDE_DIRS and ESD_LIBRARIES variables only in the advanced view
  mark_as_advanced(ESD_INCLUDE_DIRS ESD_LIBRARIES)

endif (ESD_LIBRARIES AND ESD_INCLUDE_DIRS)
