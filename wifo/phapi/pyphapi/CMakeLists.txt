if (PHAPI_PYTHON_WRAPPER)

	ow_create_shared_library(pyphapi)

	ow_use_private_libraries(
		phapi
		python
	)

	ow_add_sources(
		../pyphapi.c
	)

	ow_create_project_binary()

endif (PHAPI_PYTHON_WRAPPER)
