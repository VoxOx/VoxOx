ow_create_executable(settings-unit-tests)

ow_use_public_libraries(
	owsettings
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
