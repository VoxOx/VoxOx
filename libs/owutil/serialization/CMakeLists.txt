ow_create_static_library(owserialization)

ow_use_public_libraries(
	owutil
)

ow_add_sources(
	src/fake.cpp
)

ow_create_project_binary()
