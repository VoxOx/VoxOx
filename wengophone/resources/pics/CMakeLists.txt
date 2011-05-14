if (COPY_DATA_FILES_ON_CONFIGURE)
	# For some unknown reason, the commented line below fails under Windows
	# (Tested with cmake 2.4.5 and cmake 2.4.6). CMake enters an infinite loop
	# while copying "protocols/overlay_presence_away.png", it outputs an
	# endless string of broken chars. To workaround this, copy each folder by
	# hand :-(

	#ow_copy_dir_recursive(${CMAKE_CURRENT_SOURCE_DIR}/*.png ${DATA_COPY_DIR}/pics/)

	ow_copy_dir(${CMAKE_CURRENT_SOURCE_DIR}/*.png ${DATA_COPY_DIR}/pics/)
	ow_copy_dir_recursive(${CMAKE_CURRENT_SOURCE_DIR}/actions/*.png ${DATA_COPY_DIR}/pics/actions/)
	ow_copy_dir_recursive(${CMAKE_CURRENT_SOURCE_DIR}/audio/*.png ${DATA_COPY_DIR}/pics/audio/)
	ow_copy_dir_recursive(${CMAKE_CURRENT_SOURCE_DIR}/callbar/*.png ${DATA_COPY_DIR}/pics/callbar/)
	ow_copy_dir_recursive(${CMAKE_CURRENT_SOURCE_DIR}/chat/*.png ${DATA_COPY_DIR}/pics/chat/)
	ow_copy_dir_recursive(${CMAKE_CURRENT_SOURCE_DIR}/config/*.png ${DATA_COPY_DIR}/pics/config/)
	ow_copy_dir_recursive(${CMAKE_CURRENT_SOURCE_DIR}/contact/*.png ${DATA_COPY_DIR}/pics/contact/)
	ow_copy_dir_recursive(${CMAKE_CURRENT_SOURCE_DIR}/dialpad/*.png ${DATA_COPY_DIR}/pics/dialpad/)
	ow_copy_dir_recursive(${CMAKE_CURRENT_SOURCE_DIR}/emoticons/*.png ${DATA_COPY_DIR}/pics/emoticons/)
	ow_copy_dir_recursive(${CMAKE_CURRENT_SOURCE_DIR}/filetransfer/*.png ${DATA_COPY_DIR}/pics/filetransfer/)
	ow_copy_dir_recursive(${CMAKE_CURRENT_SOURCE_DIR}/headers/*.png ${DATA_COPY_DIR}/pics/headers/)
	ow_copy_dir_recursive(${CMAKE_CURRENT_SOURCE_DIR}/history/*.png ${DATA_COPY_DIR}/pics/history/)
	ow_copy_dir_recursive(${CMAKE_CURRENT_SOURCE_DIR}/iconbar/*.png ${DATA_COPY_DIR}/pics/iconbar/)
	ow_copy_dir_recursive(${CMAKE_CURRENT_SOURCE_DIR}/phonecall/*.png ${DATA_COPY_DIR}/pics/phonecall/)
	ow_copy_dir_recursive(${CMAKE_CURRENT_SOURCE_DIR}/profilebar/*.png ${DATA_COPY_DIR}/pics/profilebar/)
	ow_copy_dir_recursive(${CMAKE_CURRENT_SOURCE_DIR}/protocols/*.png ${DATA_COPY_DIR}/pics/protocols/)
	ow_copy_dir_recursive(${CMAKE_CURRENT_SOURCE_DIR}/status/*.png ${DATA_COPY_DIR}/pics/status/)
	ow_copy_dir_recursive(${CMAKE_CURRENT_SOURCE_DIR}/statusbar/*.png ${DATA_COPY_DIR}/pics/statusbar/)
	ow_copy_dir_recursive(${CMAKE_CURRENT_SOURCE_DIR}/systray/*.png ${DATA_COPY_DIR}/pics/systray/)
	ow_copy_dir_recursive(${CMAKE_CURRENT_SOURCE_DIR}/toaster/*.png ${DATA_COPY_DIR}/pics/toaster/)

	ow_copy_dir_recursive(${CMAKE_CURRENT_SOURCE_DIR}/*.mng ${DATA_COPY_DIR}/pics/)
	ow_copy_dir_recursive(${CMAKE_CURRENT_SOURCE_DIR}/*.xml ${DATA_COPY_DIR}/pics/)
endif (COPY_DATA_FILES_ON_CONFIGURE)
