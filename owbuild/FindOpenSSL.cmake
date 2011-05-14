# - Try to find OpenSSL
# Once done this will define
#
#  OPENSSL_FOUND - system has OpenSSL
#  OPENSSL_INCLUDE_DIRS - the OpenSSL include directory
#  OPENSSL_LIBRARIES - Link these to use OpenSSL
#  OPENSSL_DEFINITIONS - Compiler switches required for using OpenSSL
#
#  Copyright (c) 2006 Andreas Schneider <mail@cynapses.org>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (OPENSSL_LIBRARIES AND OPENSSL_INCLUDE_DIRS)
  # in cache already
  set(OPENSSL_FOUND TRUE)
else (OPENSSL_LIBRARIES AND OPENSSL_INCLUDE_DIRS)
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  include(UsePkgConfig)

  pkgconfig(openssl _OpenSSLIncDir _OpenSSLLinkDir _OpenSSLLinkFlags _OpenSSLCflags)

  find_path(OPENSSL_INCLUDE_DIR
    NAMES
      openssl/ssl.h
    PATHS
      ${_OpenSSLIncDir}
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
  )

  set(VERSION "")

  if (APPLE)
    set(VERSION
      .0.9.7
    )
  endif (APPLE)

  find_library(CRYPTO_LIBRARY
    NAMES
      crypto${VERSION}
    PATHS
      ${_OpenSSLLinkDir}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  find_library(SSL_LIBRARY
    NAMES
      ssl${VERSION}
    PATHS
      ${_OpenSSLLinkDir}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  set(OPENSSL_INCLUDE_DIRS
    ${OPENSSL_INCLUDE_DIR}
  )
  set(OPENSSL_LIBRARIES
    ${CRYPTO_LIBRARY}
    ${SSL_LIBRARY}
  )

  if (OPENSSL_INCLUDE_DIRS AND OPENSSL_LIBRARIES)
    set(OPENSSL_FOUND TRUE)
  endif (OPENSSL_INCLUDE_DIRS AND OPENSSL_LIBRARIES)

  if (OPENSSL_FOUND)
    if (NOT OpenSSL_FIND_QUIETLY)
      message(STATUS "Found OpenSSL: ${OPENSSL_LIBRARIES}")
    endif (NOT OpenSSL_FIND_QUIETLY)
  else (OPENSSL_FOUND)
    if (OpenSSL_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find OpenSSL")
    endif (OpenSSL_FIND_REQUIRED)
  endif (OPENSSL_FOUND)

  # show the OPENSSL_INCLUDE_DIRS and OPENSSL_LIBRARIES variables only in the advanced view
  mark_as_advanced(OPENSSL_INCLUDE_DIRS OPENSSL_LIBRARIES)

endif (OPENSSL_LIBRARIES AND OPENSSL_INCLUDE_DIRS)
