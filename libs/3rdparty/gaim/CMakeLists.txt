if (NOT WIN32)
	# Not MSVC and not MinGW
	include(CMakeLists-internal-unix.txt)
else (NOT WIN32)
	# If MSVC or MinGW
	include(CMakeLists-internal-win32.txt)
endif (NOT WIN32)
