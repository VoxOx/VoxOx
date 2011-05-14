ow_create_executable(webcam-test)

ow_use_private_libraries(
	webcam
	qt4
)

ow_add_private_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}
)

qt4_wrap_cpp(${PROJECT_NAME}_SRCS
	webcam_test.h
)

ow_add_sources(
	webcam_test.cpp
)

ow_create_project_binary(MACOSX_BUNDLE EXCLUDE_FROM_ALL)
