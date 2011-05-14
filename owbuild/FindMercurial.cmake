# - Try to find Subversion (svn)
# Once done this will define
#
#  MERCURIAL_FOUND - true if the command line client was found
#  MERCURIAL_HG_EXECUTABLE - path to hg command line client
#
#  Copyright (c) 2007 Wengo
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (MERCURIAL_HG_EXECUTABLE)
  # in cache already
  set(MERCURIAL_FOUND TRUE)
else (MERCURIAL_HG_EXECUTABLE)

  find_program(MERCURIAL_HG_EXECUTABLE
    NAMES
      hg
    PATHS
      /sw/bin
      /usr/bin
      /usr/local/bin
      C:/Program Files/Mercurial/bin
  )


  if (MERCURIAL_HG_EXECUTABLE)
    set(MERCURIAL_FOUND TRUE)
  endif (MERCURIAL_HG_EXECUTABLE)

  if (MERCURIAL_FOUND)
    if (NOT Mercurial_FIND_QUIETLY)
      message(STATUS "Found Mercurial (hg): ${MERCURIAL_HG_EXECUTABLE}")
    endif (NOT Mercurial_FIND_QUIETLY)
  else (MERCURIAL_FOUND)
    if (Subversion_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find Mercurial (hg)")
    endif (Subversion_FIND_REQUIRED)
  endif (MERCURIAL_FOUND)

  # show the MERCURIAL_HG_EXECUTABLE variable only in the advanced view
  mark_as_advanced(MERCURIAL_HG_EXECUTABLE)

endif (MERCURIAL_HG_EXECUTABLE)
