ow_create_executable(testnetlib)

ow_use_public_libraries(
	netlib
	owsl
)

ow_add_sources(
	testnetlib.cpp
)

ow_create_project_binary(EXCLUDE_FROM_ALL)
