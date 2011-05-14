ow_create_static_library(owbrowser)

ow_use_public_libraries(
	owutil
	qt4
)

ow_use_private_libraries(
	owqtutil
)

ow_add_public_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

ow_add_sources(
	src/QtBrowser.cpp
)

qt4_wrap_cpp(${PROJECT_NAME}_SRCS
	include/owbrowser/QtBrowser.h
)

ow_create_project_binary()
