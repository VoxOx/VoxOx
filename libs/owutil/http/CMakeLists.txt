ow_create_static_library(owhttp)

ow_use_public_libraries(
	owthread
	curl
)

ow_use_private_libraries(
	curl
)

if (WIN32)
	ow_add_private_libraries(
		advapi32
		shell32
	)
endif (WIN32)

ow_add_sources(
	src/HttpRequest.cpp
	src/curl/CurlHttpRequest.cpp
	#src/null/NullHttpRequest.cpp
	#src/qt/QtHttpRequest.cpp
)

if (APPLE)
	ow_use_private_frameworks(
		CoreFoundation
	)
endif (APPLE)

ow_create_project_binary()
