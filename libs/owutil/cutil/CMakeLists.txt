ow_create_static_library(owcutil)

ow_add_sources(
	src/fake.c
	src/strlcat.c
	src/strlcpy.c
)

ow_add_public_include_dirs(
	${LIBUTIL_INCLUDE_DIRS}
)

if (MSVC)
	ow_add_public_include_dirs(
		${CMAKE_CURRENT_SOURCE_DIR}/msvc
		${CMAKE_CURRENT_SOURCE_DIR}/strndup
	)
	ow_add_sources(
		src/msvc/dirent.c
		src/strndup/strndup.c
	)
endif (MSVC)

if (APPLE)
	ow_add_public_include_dirs(
		${CMAKE_CURRENT_SOURCE_DIR}/strndup
	)
	ow_add_sources(
		src/strndup/strndup.c
	)
endif (APPLE)

ow_create_project_binary()
