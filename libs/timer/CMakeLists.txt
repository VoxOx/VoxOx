ow_create_static_library(wtimer)

ow_add_public_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

ow_add_sources(
	src/timer_impl.c
)

if (WIN32)
	ow_add_sources(
		src/win32_mm/impl_timer.c
	)

	ow_add_private_definitions(
		-DTIMER_ENABLE_WINMM_IMPL
	)

	ow_add_private_libraries(
		winmm
	)
else (WIN32)
	if (UNIX)
		if (NOT APPLE)
			ow_add_private_libraries(
				pthread
				rt
			)
		endif (NOT APPLE)
	endif (UNIX)

	ow_add_private_definitions(
		-DTIMER_ENABLE_CLOCK_GETTIME_IMPL
	)

	ow_add_sources(
		src/clock_gettime/impl_timer.c
	)
endif (WIN32)

ow_create_project_binary()
