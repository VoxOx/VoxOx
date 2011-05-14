ow_create_plugin_library(yealinkwenbox)

# Find precompiled YLTELBOX
find_library(YLTELBOX_LIBRARIES
	NAMES
		YLTELBOX
	PATH
		${CMAKE_CURRENT_SOURCE_DIR}
)

# Hack because the find_library does not work, why? no idea...
set(YLTELBOX_LIBRARIES
	${CMAKE_CURRENT_SOURCE_DIR}/YLTELBOX.lib
)

ow_add_public_definitions(
	-DOWWENBOX_DLL
)

ow_add_private_definitions(
	-DBUILD_OWWENBOX_DLL
)

ow_use_public_libraries(
	owwenbox
)

ow_add_public_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}
)

ow_add_private_libraries(
	${YLTELBOX_LIBRARIES}
)

ow_add_sources(
	YealinkWenbox.cpp
)

ow_copy_file(${CMAKE_CURRENT_SOURCE_DIR}/YLTELBOX.dll ${BUILD_DIR})

ow_create_project_binary()
