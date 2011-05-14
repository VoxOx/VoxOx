# CMake file for the idle component, copied from Psi 0.11.
ow_create_shared_library(psiidle)

ow_use_public_libraries(
	qt4
)

ow_add_public_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}/..
)

ow_add_private_include_dirs(
	${QT_QTCORE_INCLUDE_DIR}
	${QT_QTGUI_INCLUDE_DIR}
)

ow_add_private_definitions(
	-DBUILDING_PSIIDLE
)

ow_add_sources(
	idle.cpp
)

if (APPLE)
	ow_use_public_frameworks(
		Carbon
	)
	ow_add_sources(
		idle_mac.cpp
	)
endif (APPLE)

if (WIN32)
	ow_add_sources(
		idle_win.cpp
	)
endif (WIN32)

if (LINUX)
	include(UsePkgConfig)
	pkgconfig(xscrnsaver LIBXSS_INCLUDE_DIR LIBXSS_LIB_DIR LIBXSS_LINK_FLAGS LIBXSS_CFLAGS)
	if (LIBXSS_LINK_FLAGS)
		# FIXME: We need a way to integrate pkgconfig results to owbuild.
		ow_add_private_definitions(
			-DHAVE_XSS
		)
		ow_add_private_libraries(
			Xss
		)
	else (LIBXSS_LINK_FLAGS)
		message(STATUS "Warning: libxss not found, idle detection won't be accurate")
	endif (LIBXSS_LINK_FLAGS)
	ow_add_sources(
		idle_x11.cpp
	)
endif (LINUX)


qt4_wrap_cpp(${PROJECT_NAME}_SRCS
	idle.h
)

ow_create_project_binary()
