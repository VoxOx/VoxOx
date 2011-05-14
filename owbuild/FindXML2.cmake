# - Try to find libxml2
# Once done this will define
#
#  XML2_FOUND - system has libxml2
#  XML2_INCLUDE_DIRS - the libxml2 include directory
#  XML2_LIBRARIES - Link these to use libxml2
#  XML2_DEFINITIONS - Compiler switches required for using libxml2
#
#  Copyright (c) 2006 Andreas Schneider <mail@cynapses.org>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (XML2_LIBRARIES AND XML2_INCLUDE_DIRS)
  # in cache already
  set(XML2_FOUND TRUE)
else (XML2_LIBRARIES AND XML2_INCLUDE_DIRS)
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  include(UsePkgConfig)

  pkgconfig(libxml-2.0 _XML2IncDir _XML2LinkDir _XML2LinkFlags _XML2Cflags)

  set(XML2_DEFINITIONS ${_XML2Cflags})

  find_path(XML2_INCLUDE_DIR
    NAMES
      libxml/xpath.h
    PATHS
      ${_XML2IncDir}
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
    PATH_SUFFIXES
      libxml2
  )

  find_library(XML2_LIBRARY
    NAMES
      xml2
    PATHS
      ${_XML2LinkDir}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  if (XML2_LIBRARY)
    set(XML2_FOUND TRUE)
  endif (XML2_LIBRARY)

  set(XML2_INCLUDE_DIRS
    ${XML2_INCLUDE_DIR}
  )

  if (XML2_FOUND)
    set(XML2_LIBRARIES
      ${XML2_LIBRARIES}
      ${XML2_LIBRARY}
    )
  endif (XML2_FOUND)

  if (XML2_INCLUDE_DIRS AND XML2_LIBRARIES)
    set(XML2_FOUND TRUE)
  endif (XML2_INCLUDE_DIRS AND XML2_LIBRARIES)

  if (XML2_FOUND)
    if (NOT XML2_FIND_QUIETLY)
      message(STATUS "Found libxml2: ${XML2_LIBRARIES}")
    endif (NOT XML2_FIND_QUIETLY)
  else (XML2_FOUND)
    if (XML2_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find libxml2")
    endif (XML2_FIND_REQUIRED)
  endif (XML2_FOUND)

  # show the XML2_INCLUDE_DIRS and XML2_LIBRARIES variables only in the advanced view
  mark_as_advanced(XML2_INCLUDE_DIRS XML2_LIBRARIES)

endif (XML2_LIBRARIES AND XML2_INCLUDE_DIRS)
