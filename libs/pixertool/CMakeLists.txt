include(CMakeLists-owbuild.txt)

if (PIXERTOOL_MODULE)
	ow_svn_checkout(https://dev.openwengo.com/svn/openwengo/owbuild/trunk/libs-3rdparty-cmakelists/ffmpeg libs/3rdparty/ffmpeg)
	add_subdirectory(libs/3rdparty/ffmpeg)
	set(OWUTIL_MODULE TRUE)
	ow_svn_checkout(https://dev.openwengo.com/svn/openwengo/owutil/trunk libs/owutil)
	add_subdirectory(libs/owutil)
endif (PIXERTOOL_MODULE)

ow_create_static_library(pixertool)

ow_use_public_libraries(
	owcutil
	ffmpeg
)

ow_add_public_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_BINARY_DIR}
)

ow_add_sources(
	src/pixertool.cpp
	src/ffmpeg/ffmpeg-pixertool.c
)

if (SWSCALE_INCLUDE_DIR)
  ow_add_private_definitions(-DHAVE_SWSCALE=1)
endif (SWSCALE_INCLUDE_DIR)

if (MSVC)
	ow_add_private_libraries(
		winmm
		strmiids
		comsupp
	)

	ow_add_sources(
		src/directx/directx-pixertool.cpp
	)
endif (MSVC)

if (UNIX AND NOT WIN32)
	if (APPLE)
		ow_add_sources(
			src/quicktime/quicktime-pixertool.c
		)
	else (APPLE)
		ow_add_sources(
			src/v4l/v4l-pixertool.c
			src/v4l2/v4l2-pixertool.c
		)
	endif(APPLE)
endif (UNIX AND NOT WIN32)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

ow_create_project_binary()
