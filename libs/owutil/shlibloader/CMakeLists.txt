ow_create_static_library(owshlibloader)

ow_use_public_libraries(
	owutil
)

ow_add_sources(
	src/SharedLibLoader.cpp
)

if (WIN32)
	ow_add_sources(
		src/win32/Win32SharedLibLoader.cpp
	)
endif (WIN32)

if (UNIX)
	ow_add_sources(
		src/posix/PosixSharedLibLoader.cpp
	)
endif (UNIX)

ow_create_project_binary()
