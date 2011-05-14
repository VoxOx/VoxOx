ow_create_shared_library(owutil)

ow_use_public_libraries(
	owcutil
	boost
)

if (WIN32)
	ow_use_public_libraries(
		gnutls #VOXOX - ASV - 2009.09.30: we only call the project library that was already created instead of loading the libraries again
	)
else (WIN32)
	ow_use_public_libraries(
		libgcrypt #VOXOX - ASV - 2010.01.18
	)
endif (WIN32)

ow_add_sources(
	src/Base64.cpp
	src/CountryList.cpp
	src/Date.cpp
	src/File.cpp
	src/Identifiable.cpp
	src/Logger.cpp
	src/OWPicture.cpp
	src/Path.cpp
	src/String.cpp
	src/StringList.cpp
	src/Time.cpp
	src/Uuid.cpp
	src/WebBrowser.cpp
	src/NonCopyable.cpp
	src/VoxMd5.cpp			#VOXOX - JRT - 2009.06.11
	src/md5.c				#VOXOX - JRT - 2009.06.11
	src/VoxApiTimer.cpp		#VOXOX - JRT - 2009.09.05
	src/VoxElapsedTimer.cpp	#VOXOX - JRT - 2009.09.05
	src/VoxEncrypt.cpp		#VOXOX - JRT - 2009.09.25
	src/Platform.cpp		#VOXOX - JRT - 2009.12.16
	
)

if (WIN32)
	ow_add_sources(
		src/win/UuidWin.cpp
	)
	ow_add_private_libraries(
		Rpcrt4
	)
endif (WIN32)

if (APPLE)
	ow_add_sources(
		src/mac/UuidMac.cpp
	)
	ow_use_private_frameworks(
		Carbon
		CoreFoundation
	)
endif (APPLE)

if (LINUX)
	ow_use_public_libraries(
		uuid
	)
	ow_add_sources(
		src/unix/UuidUnix.cpp
	)
endif (LINUX)

if (LOGGER_ENABLE_EVENT)
   ow_add_private_definitions(
       -DLOGGER_ENABLE_EVENT
   )
endif (LOGGER_ENABLE_EVENT)

ow_create_project_binary()

subdirs(
	tests
)
