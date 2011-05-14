ow_create_static_library(owsoftupdater)

ow_use_public_libraries(
	owhttp
)

ow_add_public_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

ow_add_sources(
	src/SoftUpdater.cpp
)

ow_create_project_binary()
