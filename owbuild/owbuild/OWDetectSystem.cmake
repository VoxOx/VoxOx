# Detects system compiler and OS used
#
# Operating systems:
# - WIN32
# - UNIX
#   - CYGWIN
#   - APPLE
#   - LINUX
#
# Compilers:
# - MSVC (Microsoft Visual C++)
#   - MSVC60 (Visual C++ 6)
#   - MSVC70 (Visual C++ .NET)
#   - MSVC71 (Visual C++ 2003)
#   - MSVC80 (Visual C++ 2005)
# - GCC
#   - MINGW (Native GCC under Windows)
#   - GCC3 (GNU GCC 3.x)
#   - GCC4 (GNU GCC 4.x)
#     - GCC40 (GNU GCC 4.0.x)
#     - GCC41 (GNU GCC 4.1.x)
# - BORLAND (Borland C++)
# - WATCOM (Watcom C/C++ Compiler)
#
# Copyright (C) 2007  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


# GCC
set(GCC ${CMAKE_COMPILER_IS_GNUCC})
ow_get_gcc_version(version)
if (version MATCHES "3.*")
	set(GCC3 true)
endif (version MATCHES "3.*")
if (version MATCHES "4.*")
	set(GCC4 true)
endif (version MATCHES "4.*")
if (version MATCHES "4\\.0.*")
	set(GCC40 true)
endif (version MATCHES "4\\.0.*")
if (version MATCHES "4\\.1.*")
	set(GCC41 true)
endif (version MATCHES "4\\.1.*")

# LINUX
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set(LINUX true)
endif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
