ow_create_shared_library(owmemorydump)

ow_use_public_libraries(
	owutil
    openssl
    googlebreakpad
)

ow_add_sources(
	src/MemoryDump.cpp
)


if (MSVC)
	ow_add_private_libraries(
		shlwapi
	)
	ow_add_sources(
		src/win32/MSVCMemoryDump.cpp
	)
endif (MSVC)

if (UNIX AND NOT WIN32)
	if (APPLE)
 		ow_add_sources(
			src/mac/MACMemoryDump.cpp
		)
         ow_use_private_frameworks(
            CoreFoundation
        )
	else (APPLE)
		ow_add_sources(
			src/unix/UNIXMemoryDump.cpp
		)
	endif (APPLE)
endif (UNIX AND NOT WIN32)

ow_create_project_binary()
