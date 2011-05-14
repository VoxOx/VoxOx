ow_create_static_library(owthread)

ow_use_public_libraries(
	owutil
)

ow_add_sources(
	src/Lockable.cpp
	src/Timer.cpp
	src/Thread.cpp
)

ow_create_project_binary()

subdirs(
	tests
)
