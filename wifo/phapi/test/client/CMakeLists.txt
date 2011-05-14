ow_create_executable(phapi-test)

ow_use_public_libraries(
	owsl
	phapi
)

ow_add_sources(
	PhApiTest.c
	wlm_password.c
)

ow_create_project_binary(EXCLUDE_FROM_ALL)
