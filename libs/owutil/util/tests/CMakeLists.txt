ow_create_executable(owutil-unit-tests)

ow_use_public_libraries(
	owutil
	boost-test
)

if (APPLE)
	ow_use_private_frameworks(
		CoreFoundation
	)
endif (APPLE)

ow_add_sources(
	main.cpp
)
ow_create_project_binary(EXCLUDE_FROM_ALL)


ow_create_executable(loggertest)

ow_use_public_libraries(
	owutil
)

ow_add_sources(
	LoggerTest.cpp
)
ow_create_project_binary(EXCLUDE_FROM_ALL)
