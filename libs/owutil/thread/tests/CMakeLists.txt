ow_create_executable(thread-tests)

ow_use_public_libraries(
	owutil
	owthread
	boost-test
)

ow_add_sources(
	main.cpp
)

ow_create_project_binary(EXCLUDE_FROM_ALL)
