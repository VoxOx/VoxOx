include(CMakeLists-owbuild.txt)

if (OWUTIL_MODULE)
	ow_svn_checkout(https://dev.openwengo.com/svn/openwengo/owbuild/trunk/libs-3rdparty-cmakelists/boost libs/3rdparty/boost)
	add_subdirectory(libs/3rdparty/boost)
	ow_svn_checkout(https://dev.openwengo.com/svn/openwengo/owbuild/trunk/libs-3rdparty-cmakelists/boost-test libs/3rdparty/boost-test)
	add_subdirectory(libs/3rdparty/boost-test)
	ow_svn_checkout(https://dev.openwengo.com/svn/openwengo/owbuild/trunk/libs-3rdparty-cmakelists/openssl libs/3rdparty/openssl)
	add_subdirectory(libs/3rdparty/openssl)
	ow_svn_checkout(https://dev.openwengo.com/svn/openwengo/owbuild/trunk/libs-3rdparty-cmakelists/tinyxml libs/3rdparty/tinyxml)
	add_subdirectory(libs/3rdparty/tinyxml)
	ow_svn_checkout(https://dev.openwengo.com/svn/openwengo/owbuild/trunk/libs-3rdparty-cmakelists/uuid libs/3rdparty/uuid)
	add_subdirectory(libs/3rdparty/uuid)
	ow_svn_checkout(https://dev.openwengo.com/svn/openwengo/owbuild/trunk/libs-3rdparty-cmakelists/curl libs/3rdparty/curl)
	add_subdirectory(libs/3rdparty/curl)

	#VOXOX - JRT - 2009.03.30
#	ow_svn_checkout(https://dev.openwengo.com/svn/openwengo/owbuild/trunk/libs-3rdparty-cmakelists/curl libs/3rdparty/curl)
	#add_subdirectory(libs/3rdparty/sqlite)
	add_subdirectory(libs/3rdparty/json)		#VOXOX - JRT - 2009.06.26
	#End VOXOX
endif (OWUTIL_MODULE)

set(LIBUTIL_INCLUDE_DIRS
	${CMAKE_CURRENT_SOURCE_DIR}
)

subdirs(
	cutil
	util
	serialization
	serializer
	thread
	settings
	shlibloader
	socket
	system
	http
)

if (ENABLE_CRASHREPORT)
	subdirs(
		memorydump
	)
endif (ENABLE_CRASHREPORT)
