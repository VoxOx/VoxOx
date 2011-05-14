# TODO : test dir

subdirs(
	src/buildid
	src
	resources
	#test
)

if (COPY_DATA_FILES_ON_CONFIGURE)
	ow_copy_file(${CMAKE_CURRENT_SOURCE_DIR}/COPYING ${BUILD_DIR})
	# Copy AUTHORS to DATA_COPY_DIR because it's used by QtAbout
	ow_copy_file(${CMAKE_CURRENT_SOURCE_DIR}/AUTHORS ${DATA_COPY_DIR}/.)
endif (COPY_DATA_FILES_ON_CONFIGURE)
