# - Try to find GnuTLS
# Once done this will define
#
#  GNUTLS_FOUND - system has GnuTLS
#  GNUTLS_INCLUDE_DIRS - the GnuTLS include directory
#  GNUTLS_LIBRARIES - Link these to use GnuTLS
#  GNUTLS_DEFINITIONS - Compiler switches required for using GnuTLS
#
#  Copyright (c) 2007 Wengo
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (GNUTLS_LIBRARIES AND GNUTLS_INCLUDE_DIRS)
  # in cache already
  set(GNUTLS_FOUND TRUE)
else (GNUTLS_LIBRARIES AND GNUTLS_INCLUDE_DIRS)
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  include(UsePkgConfig)

  pkgconfig(gnutls _GNUTLS_INCLUDE_DIRS _GNUTLS_LIBRARY_DIRS _GNUTLS_LINK_FLAGS _GNUTLS_DEFINITIONS)

  find_path(GNUTLS_INCLUDE_DIRS
    NAMES
      gnutls/gnutls.h
    PATHS
      ${_GNUTLS_INCLUDE_DIRS}
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
  )

  find_library(GNUTLS_LIBRARIES
    NAMES
      gnutls
    PATHS
      ${_GNUTLS_LIBRARY_DIRS}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  if (GNUTLS_INCLUDE_DIRS AND GNUTLS_LIBRARIES)
    set(GNUTLS_FOUND TRUE)
  endif (GNUTLS_INCLUDE_DIRS AND GNUTLS_LIBRARIES)

  if (GNUTLS_FOUND)
    if (NOT GnuTLS_FIND_QUIETLY)
      message(STATUS "Found GnuTLS: ${GNUTLS_LIBRARIES}")
    endif (NOT GnuTLS_FIND_QUIETLY)
  else (GNUTLS_FOUND)
    if (GnuTLS_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find GnuTLS")
    endif (GnuTLS_FIND_REQUIRED)
  endif (GNUTLS_FOUND)

  # show the GNUTLS_INCLUDE_DIRS and GNUTLS_LIBRARIES variables only in the advanced view
  mark_as_advanced(GNUTLS_INCLUDE_DIRS GNUTLS_LIBRARIES)

endif (GNUTLS_LIBRARIES AND GNUTLS_INCLUDE_DIRS)
