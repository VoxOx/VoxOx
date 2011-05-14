# - Try to find FFMPEG
# Once done this will define
#
#  FFMPEG_FOUND - system has FFMPEG
#  FFMPEG_INCLUDE_DIRS - the FFMPEG include directory
#  FFMPEG_LIBRARIES - Link these to use FFMPEG
#  FFMPEG_DEFINITIONS - Compiler switches required for using FFMPEG
#
#  Copyright (c) 2008 Andreas Schneider <mail@cynapses.org>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (FFMPEG_LIBRARIES AND FFMPEG_INCLUDE_DIRS)
  # in cache already
  set(FFMPEG_FOUND TRUE)
else (FFMPEG_LIBRARIES AND FFMPEG_INCLUDE_DIRS)
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  if (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
    include(UsePkgConfig)
    pkgconfig(libavcodec _AVCODEC_INCLUDEDIR _AVCODEC_LIBDIR _AVCODEC_LDFLAGS _AVCODEC_CFLAGS)
    pkgconfig(libavformat _AVFORMAT_INCLUDEDIR _AVFORMAT_LIBDIR _AVFORMAT_LDFLAGS _AVFORMAT_CFLAGS)
    pkgconfig(libavutil _AVUTIL_INCLUDEDIR _AVUTIL_LIBDIR _AVUTIL_LDFLAGS _AVUTIL_CFLAGS)
#    pkgconfig(libpostproc _POSTPROC_INCLUDEDIR _POSTPROC_LIBDIR _POSTPROC_LDFLAGS _POSTPROC_CFLAGS)
    pkgconfig(libswscale _SWSCALE_INCLUDEDIR _SWSCALE_LIBDIR _SWSCALE_LDFLAGS _SWSCALE_CFLAGS)
  else (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
    message("Doing find_package(PkgConfig)")
    find_package(PkgConfig)
    if (PKG_CONFIG_FOUND)
      pkg_check_modules(_AVCODEC libavcodec)
      pkg_check_modules(_AVFORMAT libavformat)
      pkg_check_modules(_AVUTIL libavutil)
#      pkg_check_modules(_POSTPROC libpostproc)
      pkg_check_modules(_SWSCALE libswscale)
    endif (PKG_CONFIG_FOUND)
  endif (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)

  message("_AVCODEC_INCLUDEDIR = ${_AVCODEC_INCLUDEDIR}")	
  message("_AVCODEC_LIBDIR = ${_AVCODEC_LIBDIR}")	
  message("_AVCODEC_LIBS = ${_AVCODEC_LIBRARIES}")	
  message("_AVCODEC_LIBS_L = ${_AVCODEC_LIBS_L}")	
  message("_AVCODEC_LIBS_PATH = ${_AVCODEC_LIBS_PATH}")	

  find_path(AVCODEC_INCLUDE_DIR
    NAMES
      avcodec.h
    PATHS
     ${_AVCODEC_INCLUDEDIR}
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
    PATH_SUFFIXES
      libavcodec
      ffmpeg
    NO_DEFAULT_PATH
  )

  message("AVCODEC_INCLUDE_DIR = ${AVCODEC_INCLUDE_DIR}")	

  mark_as_advanced(AVCODEC_INCLUDE_DIR)

  find_path(AVUTIL_INCLUDE_DIR
    NAMES
      avutil.h
    PATHS
      ${_AVUTIL_INCLUDEDIR}
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
    PATH_SUFFIXES
      libavutil
      ffmpeg
    NO_DEFAULT_PATH
  )
  mark_as_advanced(AVUTIL_INCLUDE_DIR)

  find_path(AVFORMAT_INCLUDE_DIR
    NAMES
      avformat.h
    PATHS
      ${_AVFORMAT_INCLUDEDIR}
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
    PATH_SUFFIXES
      libavformat
      ffmpeg
    NO_DEFAULT_PATH
  )
  mark_as_advanced(AVFORMAT_INCLUDE_DIR)

  if (FALSE) 
  find_path(POSTPROC_INCLUDE_DIR
    NAMES
      postprocess.h
    PATHS
      ${_POSTPROC_INCLUDEDIR}
      /usr/include/postproc
      /usr/local/include/postproc
      /opt/local/include/postproc
      /sw/include
    PATH_SUFFIXES
      libpostproc
      ffmpeg
  )
  mark_as_advanced(POSTPROC_INCLUDE_DIR)
  endif(FALSE)
	
  find_path(SWSCALE_INCLUDE_DIR
    NAMES
      swscale.h
    PATHS
      ${_SWCALE_INCLUDEDIR}
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
    PATH_SUFFIXES
      libswscale
      ffmpeg
    NO_DEFAULT_PATH
  )
  mark_as_advanced(SWSCALE_INCLUDE_DIR)

  find_library(AVCODEC_LIBRARY
    NAMES
      avcodec
    PATHS
      ${_FFMPEG_LIBDIR}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
    NO_DEFAULT_PATH
  )
  mark_as_advanced(AVCODEC_LIBRARY)

  find_library(AVUTIL_LIBRARY
    NAMES
      avutil
    PATHS
      ${_FFMPEG_LIBDIR}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
    NO_DEFAULT_PATH
  )
  mark_as_advanced(AVUTIL_LIBRARY)

  find_library(AVFORMAT_LIBRARY
    NAMES
      avformat
    PATHS
      ${_FFMPEG_LIBDIR}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
    NO_DEFAULT_PATH
  )
  mark_as_advanced(AVFORMAT_LIBRARY)

  if (FALSE)
  find_library(POSTPROC_LIBRARY
    NAMES
      postproc
    PATHS
      ${_FFMPEG_LIBDIR}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
    NO_DEFAULT_PATH
  )
  mark_as_advanced(POSTPROC_LIBRARY)
  endif(FALSE)

  find_library(SWSCALE_LIBRARY
    NAMES
      swscale
    PATHS
      ${_FFMPEG_LIBDIR}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
    NO_DEFAULT_PATH
  )
  mark_as_advanced(SWSCALE_LIBRARY)

  if (AVCODEC_LIBRARY)
    set(AVCODEC_FOUND TRUE)
  endif (AVCODEC_LIBRARY)
  if (AVUTIL_LIBRARY)
    set(AVUTIL_FOUND TRUE)
  endif (AVUTIL_LIBRARY)
  if (AVFORMAT_LIBRARY)
    set(AVFORMAT_FOUND TRUE)
  endif (AVFORMAT_LIBRARY)
 
 if (POSTPROC_LIBRARY)
    set(POSTPROC_FOUND TRUE)
  endif (POSTPROC_LIBRARY)
  if (SWSCALE_LIBRARY)
    set(SWSCALE_FOUND TRUE)
  endif (SWSCALE_LIBRARY)

  set(FFMPEG_INCLUDE_DIRS
    ${_AVCODEC_INCLUDEDIR}
    ${AVCODEC_INCLUDE_DIR}
    ${AVFORMAT_INCLUDE_DIR}
    ${AVUTIL_INCLUDE_DIR}
    ${POSTPROC_INCLUDE_DIR}
    ${SWSCALE_INCLUDE_DIR}
  )

  if (AVCODEC_FOUND)
    set(FFMPEG_LIBRARIES
      ${FFMPEG_LIBRARIES}
      ${AVCODEC_LIBRARY}
    )
  endif (AVCODEC_FOUND)
  if (AVUTIL_FOUND)
    set(FFMPEG_LIBRARIES
      ${FFMPEG_LIBRARIES}
      ${AVUTIL_LIBRARY}
    )
  endif (AVUTIL_FOUND)
  if (AVFORMAT_FOUND)
    set(FFMPEG_LIBRARIES
      ${FFMPEG_LIBRARIES}
      ${AVFORMAT_LIBRARY}
    )
  endif (AVFORMAT_FOUND)
  if (POSTPROC_FOUND)
    set(FFMPEG_LIBRARIES
      ${FFMPEG_LIBRARIES}
      ${POSTPROC_LIBRARY}
    )
  endif (POSTPROC_FOUND)
  if (SWSCALE_FOUND)
    set(FFMPEG_LIBRARIES
      ${FFMPEG_LIBRARIES}
      ${SWSCALE_LIBRARY}
    )
  endif (SWSCALE_FOUND)

  if (FFMPEG_INCLUDE_DIRS AND FFMPEG_LIBRARIES)
     set(FFMPEG_FOUND TRUE)
  endif (FFMPEG_INCLUDE_DIRS AND FFMPEG_LIBRARIES)

  if (FFMPEG_FOUND)
    if (NOT FFMPEG_FIND_QUIETLY)
      message(STATUS "Found FFMPEG: ${FFMPEG_LIBRARIES}")
    endif (NOT FFMPEG_FIND_QUIETLY)
  else (FFMPEG_FOUND)
    if (FFMPEG_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find FFMPEG")
    endif (FFMPEG_FIND_REQUIRED)
  endif (FFMPEG_FOUND)

  # show the FFMPEG_INCLUDE_DIRS and FFMPEG_LIBRARIES variables only in the advanced view
  mark_as_advanced(FFMPEG_INCLUDE_DIRS FFMPEG_LIBRARIES)

endif (FFMPEG_LIBRARIES AND FFMPEG_INCLUDE_DIRS)

