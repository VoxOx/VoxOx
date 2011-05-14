# - ow_finish_project_binary()
# Internal function, finishes the creation of a binary file, used by ow_create_shared_library(), ow_create_executable()...
#
# General algorithm is:
# ow_create_project() calls ow_create_shared_library(), ow_create_executable()...
# ow_create_project_binary() calls ow_prepare_project_binary() and then ow_finish_project_binary()
#
# ow_finish_project_binary() exists since CMake functions target_link_libraries(),
# set_target_properties() need to be run after add_executable(),
# add_library() (called inside ow_create_project_binary())
#
# ow_finish_project_binary() handles link libraries, compile flags (including defines),
# link flags and api/build version
#
# Copyright (C) 2006-2007  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


macro (ow_finish_project_binary)

	ow_check_project()

	ow_unique(${PROJECT_NAME}_LIBRARIES ${${PROJECT_NAME}_LIBRARIES} ${${PROJECT_NAME}_PRIVATE_LIBRARIES})
	target_link_libraries(${PROJECT_NAME} ${${PROJECT_NAME}_LIBRARIES})

	if (${PROJECT_NAME}_PRIVATE_COMPILE_FLAGS)
		ow_unique(${PROJECT_NAME}_PRIVATE_COMPILE_FLAGS ${${PROJECT_NAME}_PRIVATE_COMPILE_FLAGS})

		string(REPLACE ";" " " ${PROJECT_NAME}_PRIVATE_COMPILE_FLAGS "${${PROJECT_NAME}_PRIVATE_COMPILE_FLAGS}")

		set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS ${${PROJECT_NAME}_PRIVATE_COMPILE_FLAGS})
	endif (${PROJECT_NAME}_PRIVATE_COMPILE_FLAGS)

	if (${PROJECT_NAME}_DEFINITIONS OR ${PROJECT_NAME}_PRIVATE_DEFINITIONS)
		ow_unique(unique ${${PROJECT_NAME}_DEFINITIONS} ${${PROJECT_NAME}_PRIVATE_DEFINITIONS})

		string(REPLACE ";" " " unique "${unique}")

		# Does not work under MacOSX
		#set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS ${unique})
		add_definitions(${unique})
	endif (${PROJECT_NAME}_DEFINITIONS OR ${PROJECT_NAME}_PRIVATE_DEFINITIONS)

	if (${PROJECT_NAME}_PUBLIC_LINK_FLAGS OR ${PROJECT_NAME}_PRIVATE_LINK_FLAGS)
		ow_unique(unique ${${PROJECT_NAME}_PUBLIC_LINK_FLAGS} ${${PROJECT_NAME}_PRIVATE_LINK_FLAGS})

		string(REPLACE ";" " " unique "${unique}")

		set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS ${unique})
	endif (${PROJECT_NAME}_PUBLIC_LINK_FLAGS OR ${PROJECT_NAME}_PRIVATE_LINK_FLAGS)

	if (${PROJECT_NAME}_BUILD_VERSION AND ${PROJECT_NAME}_API_VERSION)
		set_target_properties(${PROJECT_NAME}
			PROPERTIES
				VERSION ${${PROJECT_NAME}_BUILD_VERSION}
				SOVERSION ${${PROJECT_NAME}_API_VERSION}
		)
	endif (${PROJECT_NAME}_BUILD_VERSION AND ${PROJECT_NAME}_API_VERSION)

	if (${PROJECT_NAME}_PROJECT_TYPE MATCHES Shared OR ${PROJECT_NAME}_PROJECT_TYPE MATCHES Plugin)
		ow_add_shared_library_definitions()
	endif (${PROJECT_NAME}_PROJECT_TYPE MATCHES Shared OR ${PROJECT_NAME}_PROJECT_TYPE MATCHES Plugin)

	if (CMAKE_VERBOSE_MAKEFILE)
		ow_project_log()
	endif (CMAKE_VERBOSE_MAKEFILE)

endmacro (ow_finish_project_binary)
