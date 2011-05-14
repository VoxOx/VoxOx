if (NOT WIN32)
	# Not MSVC and not MinGW
        if (LIBPURPLE_INTERNAL)
                include(CMakeLists-unix.txt)
        else (LIBPURPLE_INTERNAL)
                include(CMakeLists-external.txt)
        endif (LIBPURPLE_INTERNAL)
else (NOT WIN32)
	# If MSVC or MinGW
	include(CMakeLists-win32.txt)
endif (NOT WIN32)
