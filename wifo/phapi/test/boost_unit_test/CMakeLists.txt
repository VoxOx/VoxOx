ow_create_executable(phapitest)

ow_use_private_libraries(
	phapi
	boost-test
)

ow_add_sources(
	main.cpp
)

ow_create_project_binary()
