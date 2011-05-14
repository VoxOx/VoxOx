# - Try to find PTHREAD
# Once done this will define
#
#  PTHREAD_FOUND - system has PTHREAD
#  PTHREAD_LINK_FLAGS - linker flags to use PTHREAD
#
#  Copyright (c) 2007 Wengo
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

include (FindThreads)

if (CMAKE_USE_PTHREADS_INIT)
	# hack to pass OWBuild check
	set (PTHREAD_INCLUDE_DIR "/usr/include")
	set (PTHREAD_LINK_FLAGS ${CMAKE_THREAD_LIBS_INIT})
	set (PTHREAD_FOUND TRUE)
	mark_as_advanced (PTHREAD_INCLUDE_DIR)
else (CMAKE_USE_PTHREADS_INIT)
	if (PTHREAD_FIND_REQUIRED)
		message (FATAL_ERROR "Could not find pthreads")
	endif (PTHREAD_FIND_REQUIRED)
endif (CMAKE_USE_PTHREADS_INIT)
