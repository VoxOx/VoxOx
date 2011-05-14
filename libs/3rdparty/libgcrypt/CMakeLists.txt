# VOXOX -ASV- 2010.01.18
ow_create_project(libgcrypt)

if (APPLE)
ow_add_public_libraries(
	${CMAKE_CURRENT_SOURCE_DIR}/libgcrypt-1.4.5/lib/macosx/libgcrypt.11.dylib
)
endif(APPLE)

if (LINUX)
#TODO
endif (LINUX)

ow_add_public_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}/libgcrypt-1.4.5/include
)
