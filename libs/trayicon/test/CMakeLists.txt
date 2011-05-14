ow_create_executable(testtrayicon)

ow_use_public_libraries(
	qt4
	owtrayicon
)

qt4_wrap_cpp(${PROJECT_NAME}_SRCS
	TestTray.h
)

ow_add_sources(
	TestTray.cpp
)

ow_create_project_binary(MACOSX_BUNDLE EXCLUDE_FROM_ALL)
