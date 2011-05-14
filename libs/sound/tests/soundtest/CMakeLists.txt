ow_create_executable(soundtest)

ow_use_public_libraries(
	qt4
	owsound
)

ow_add_public_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}/../..
	${CMAKE_CURRENT_BINARY_DIR}
)

qt4_wrap_cpp(${PROJECT_NAME}_SRCS
	SoundWidget.h
)

qt4_wrap_ui(${PROJECT_NAME}_SRCS
	SoundWidget.ui
)

ow_add_sources(
	SoundWidget.cpp
	main.cpp
)

ow_create_project_binary(MACOSX_BUNDLE EXCLUDE_FROM_ALL)
