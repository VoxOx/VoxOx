ow_create_project(ffmpeg)

ow_add_public_libraries(
	${CMAKE_CURRENT_SOURCE_DIR}/binary-lib/avcodec.lib
)

ow_add_public_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}/include/
)

ow_copy_file(${CMAKE_CURRENT_SOURCE_DIR}/binary-lib/avcodec.dll ${BUILD_DIR})
