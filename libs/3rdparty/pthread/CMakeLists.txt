if (UNIX)
	include (CMakeLists-external.txt)
else (UNIX)
	if (WIN32)
		include (CMakeLists-internal.txt)
	endif (WIN32)
endif (UNIX)
