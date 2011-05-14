ow_create_static_library(owsocket)

ow_use_public_libraries(
	owthread
)

ow_add_sources(
	src/ServerSocket.cpp
	src/ClientSocket.cpp
)

if (WIN32)
	ow_add_private_libraries(
		Ws2_32
	)
endif (WIN32)

ow_create_project_binary()
