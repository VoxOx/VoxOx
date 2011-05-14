if (NOT WIN32)
	# Not MSVC and not MinGW
	# include(CMakeLists-unix.txt)
else (NOT WIN32)
	# If MSVC or MinGW
	include(CMakeLists-win32.txt)
endif (NOT WIN32)
