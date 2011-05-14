ow_create_static_library(owtrayicon)

ow_use_public_libraries(
	qt4
)

ow_add_public_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}
)

qt4_wrap_cpp(owtrayicon_SRCS
	trayicon.h
)

ow_add_sources(
	trayicon.cpp
)

if (WIN32)
	ow_add_sources(
		trayicon_win.cpp
	)
endif (WIN32)

if (UNIX AND NOT WIN32)
	if (APPLE)
		ow_add_sources(
			trayicon_mac.cpp
		)
	else (APPLE)
		ow_add_sources(
			trayicon_x11.cpp
		)

		ow_use_private_libraries(
			x11
		)
	endif (APPLE)
endif (UNIX AND NOT WIN32)

subdirs(
	test
)

ow_create_project_binary()
