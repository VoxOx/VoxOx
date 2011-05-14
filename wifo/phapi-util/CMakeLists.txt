ow_create_shared_library(phapiutil)

ow_use_public_libraries(
	owcutil
)

ow_add_public_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

if (WIN32)
	ow_add_public_include_dirs(
		${CMAKE_CURRENT_SOURCE_DIR}/include/phapi-util/msvc
	)
endif (WIN32)

ow_add_sources(
	src/util.c
	src/comparator.c
	src/linkedlist.c
	src/mappinglist.c
	src/phSharedLib.c
)

if (WIN32)
	ow_add_private_definitions(
		-DWIN32
	)

	ow_add_sources(
		src/msvc/unistd.c
	)

	ow_add_private_libraries(
		ws2_32
	)
endif (WIN32)

ow_create_project_binary()
