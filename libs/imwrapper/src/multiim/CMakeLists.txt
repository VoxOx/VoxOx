ow_create_static_library(owmultiimwrapper)

ow_use_public_libraries(
	owpurplewrapper
	owphapiwrapper
)

ow_add_public_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}/../multiim
)

ow_add_sources(
	MultiIMFactory.cpp
)

ow_create_project_binary()
