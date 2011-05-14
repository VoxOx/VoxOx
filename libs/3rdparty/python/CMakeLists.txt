if (NOT PYTHON_FOUND)
	find_package(PythonLibs REQUIRED)
	set(PYTHON_FOUND
		TRUE
	)
endif (NOT PYTHON_FOUND)

ow_create_project(python)

ow_add_public_libraries(
	${PYTHON_LIBRARIES}
)

ow_add_public_include_dirs(
	${PYTHON_INCLUDE_PATH}
)
