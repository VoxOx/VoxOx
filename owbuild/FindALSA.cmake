# - Try to find ALSA
# Once done this will define
#
#  ALSA_FOUND - system has ALSA
#  ALSA_INCLUDE_DIRS - the ALSA include directory
#  ALSA_LIBRARIES - Link these to use ALSA
#  ALSA_DEFINITIONS - Compiler switches required for using ALSA
#
#  Copyright (c) 2007 Wengo
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (ALSA_LIBRARIES AND ALSA_INCLUDE_DIRS)
  # in cache already
  set(ALSA_FOUND TRUE)
else (ALSA_LIBRARIES AND ALSA_INCLUDE_DIRS)
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  include(UsePkgConfig)

  pkgconfig(alsa ALSA_INCLUDE_DIRS ALSA_LIBRARY_DIRS _ALSA_LINK_FLAGS _ALSA_DEFINITIONS)

  find_library(ALSA_LIBRARIES
    NAMES
      asound
    PATHS
      ${_ALSA_LIBRARY_DIRS}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
  )

  if (ALSA_INCLUDE_DIRS AND ALSA_LIBRARIES)
    set(ALSA_FOUND TRUE)
  endif (ALSA_INCLUDE_DIRS AND ALSA_LIBRARIES)

  if (ALSA_FOUND)
    if (NOT ALSA_FIND_QUIETLY)
      message(STATUS "Found ALSA: ${ALSA_LIBRARIES}")
    endif (NOT ALSA_FIND_QUIETLY)
  else (ALSA_FOUND)
    if (ALSA_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find ALSA")
    endif (ALSA_FIND_REQUIRED)
  endif (ALSA_FOUND)

  # show the ALSA_INCLUDE_DIRS and ALSA_LIBRARIES variables only in the advanced view
  mark_as_advanced(ALSA_INCLUDE_DIRS ALSA_LIBRARIES)

endif (ALSA_LIBRARIES AND ALSA_INCLUDE_DIRS)
