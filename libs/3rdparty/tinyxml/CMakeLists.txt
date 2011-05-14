ow_create_static_library(tinyxml)

ow_add_public_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}
)

ow_add_sources(
	tinyxml.cpp
	tinyxmlerror.cpp
	tinyxmlparser.cpp
)

ow_create_project_binary()
