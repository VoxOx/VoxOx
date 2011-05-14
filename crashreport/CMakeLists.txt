ow_create_executable(owcrashreport)

ow_use_public_libraries(
	owutil
	owthread
	owsystem
	curl
	owqtutil
	owsettings
)

ow_add_public_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}
)

# Because of UIC and MOC
ow_add_private_include_dirs(
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
)

qt4_wrap_cpp(${PROJECT_NAME}_SRCS
	FtpUpload.h
	QtCrashReport.h
)

qt4_wrap_ui(${PROJECT_NAME}_SRCS
	CrashReport.ui
)

ow_add_sources(
	main.cpp
	FtpUpload.cpp
	QtCrashReport.cpp
)

if (WIN32)
	ow_add_private_libraries(
		qtmain.lib
	)
endif (WIN32)

# FIXME: This has been copied from wengophone/src/CMakeLists.txt, it would be
# nicer to avoid duplication.
if (LINUX)
	if (WENGOPHONE_RESOURCEDIR_PATH)
		ow_add_private_definitions(
			-DOW_RESOURCEDIR='"${CMAKE_INSTALL_PREFIX}/${DATA_INSTALL_DIR}"'
		)
	endif (WENGOPHONE_RESOURCEDIR_PATH)
endif (LINUX)

if (WIN32)
if (COPY_DATA_FILES_ON_CONFIGURE)
	ow_copy_dir_recursive(${CMAKE_CURRENT_SOURCE_DIR}/crashreport.xml ${DATA_COPY_DIR}/config)
endif (COPY_DATA_FILES_ON_CONFIGURE)

install(FILES
	${CMAKE_CURRENT_SOURCE_DIR}/crashreport.xml
	DESTINATION ${DATA_INSTALL_DIR}/config)
endif (WIN32)

if(APPLE)
    set(PROJECT_BINARY_COPY_DIR ${BUILD_DIR}/${APPLICATION_NAME}.app/Contents/MacOS/.)
endif(APPLE)

ow_create_project_binary()

