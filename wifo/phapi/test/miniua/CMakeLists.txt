ow_create_executable(miniua)

ow_use_private_libraries(
	phapi
)

ow_add_sources(
	miniua.c
)

ow_create_project_binary()
