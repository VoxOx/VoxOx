
ow_create_static_library(sqlite)

ow_add_public_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

ow_add_private_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}/src
)

ow_add_sources(
	src/sqlite3.c
	src/CppSQLite3.cpp

#	src/Components.cpp
#	src/EnumVisibility.cpp
#	src/VoxClientDb.cpp
#	src/VoxSQLite.cpp
)



ow_create_project_binary()
