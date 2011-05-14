ow_create_project(boost-test)

ow_use_public_libraries(
	boost
)

set(boost-test_LIBRARIES
	${BOOST_UNIT_TEST_FRAMEWORK_LIBRARY}
	CACHE INTERNAL "${PROJECT_NAME} libraries"
)
