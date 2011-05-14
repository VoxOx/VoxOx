if (LINUX)

	if (NOT X11_FOUND)
		find_package(X11 REQUIRED)
	endif (NOT X11_FOUND)

	ow_create_project(x11)

	ow_use_public_libraries(
		X11
	)

	ow_add_public_libraries(
		${X11_Xv_LIB}
	)

endif (LINUX)
