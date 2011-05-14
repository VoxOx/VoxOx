# Defines default compiler/linker flags
#
# GNU GCC Command Options: http://gcc.gnu.org/onlinedocs/gcc-4.1.1/gcc/Invoking-GCC.html
# GNU GCC Warning Options: http://gcc.gnu.org/onlinedocs/gcc-4.1.1/gcc/Warning-Options.html
# GNU GCC Link Options: http://gcc.gnu.org/onlinedocs/gcc-4.1.1/gcc/Link-Options.html
# Microsoft Visual C++ Compiler Options: http://msdn2.microsoft.com/en-us/library/fwkeyyhe.aspx
# Microsoft Visual C++ Linker Options: http://msdn2.microsoft.com/en-us/library/y0zzbyt4.aspx
#
# Copyright (C) 2006-2007  Wengo
# Copyright (C) 2006-2007  Andreas Schneider <mail@cynapses.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


# With -fPIC
if (UNIX AND NOT WIN32)
	if (CMAKE_SIZEOF_VOID_P MATCHES "8")
		ow_check_cxx_compiler_flag("-fPIC" WITH_FPIC)
		if (WITH_FPIC)
			add_definitions(-fPIC)
		endif (WITH_FPIC)
	endif (CMAKE_SIZEOF_VOID_P MATCHES "8")

	if (GCC4)
		# See http://www.cynapses.org/tmp/gcc/fortify_source
		add_definitions(-D_FORTIFY_SOURCE=2)
	endif (GCC4)
endif (UNIX AND NOT WIN32)

# Enable warnings
if (MSVC)
	#add_definitions(/W4)
else (MSVC)
	add_definitions(-Wall -Wstrict-aliasing)
	if (GCC4)
		add_definitions(-Wextra)
	endif (GCC4)
	add_definitions(-Wno-unused-parameter)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wmissing-prototypes")
endif (MSVC)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
	if (MSVC)
		# No MSVCRT.LIB linking under Visual C++ when in debug mode
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:MSVCRT.LIB")
		set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:MSVCRT.LIB")
		set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /NODEFAULTLIB:MSVCRT.LIB")
	endif (MSVC)

	if (APPLE)
		# Use dwarf-2 debugging format: it produces much smaller executables
		# (from 372M to 37MB on my machine)
		add_definitions(-gdwarf-2)
	endif (APPLE)

	# Defines DEBUG when in debug mode
	add_definitions(-DDEBUG)
else (CMAKE_BUILD_TYPE STREQUAL Debug)
	if (MSVC)
		# No MSVCRTD.LIB linking under Visual C++ when in release mode
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:MSVCRTD.LIB")
		set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:MSVCRTD.LIB")
		set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /NODEFAULTLIB:MSVCRTD.LIB")
	endif (MSVC)
endif (CMAKE_BUILD_TYPE STREQUAL Debug)

if (CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo)
	if (MSVC)
		# /O2 Creates fast code
		# /O1 Creates small code
		string(REPLACE "/O2" "/O1" CMAKE_C_FLAGS_RELWITHDEBINFO ${CMAKE_C_FLAGS_RELWITHDEBINFO})
		string(REPLACE "/O2" "/O1" CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
	endif (MSVC)

	add_definitions(-DDEBUG)
endif (CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo)

if (CMAKE_VERBOSE_MAKEFILE)
	if (NOT MSVC)
		add_definitions(-fmessage-length=0)
	endif (NOT MSVC)
endif (CMAKE_VERBOSE_MAKEFILE)

#ow_get_gcc_version(version)
#message("GCC Version: ${version}")
#message("CFLAGS: ${CMAKE_C_FLAGS}")
#message("CXXFLAGS: ${CMAKE_CXX_FLAGS}")
