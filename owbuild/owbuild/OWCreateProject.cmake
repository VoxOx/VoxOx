# - ow_create_project(name)
# Creates an empty project given its name
#
# Most of the time you won't need to use this function, use:
# - ow_create_executable()
# - ow_create_shared_library()
# - ow_create_static_library()
# - ow_create_plugin_library()
# instead
#
# Internally ow_create_project() calls project() CMake function
# and then initializes all project variables (${PROJECT_NAME}_INCLUDE_DIRS,
# ${PROJECT_NAME}_LIBRARIES, ${PROJECT_NAME}_LIBRARY_DIRS, ${PROJECT_NAME}_DEFINITIONS...)
#
# Copyright (C) 2006-2007  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


macro (ow_create_project name)

	# Creates the variable ${PROJECT_NAME} containing the project name
	project(${name})

	#set(${PROJECT_NAME}_PROJECT_TYPE "" CACHE STRING "${PROJECT_NAME} project type (Static, Shared, Plugin, Executable)")

	set(${PROJECT_NAME}_SRCS "")

	# Resets the include directories
	set_directory_properties(PROPERTIES INCLUDE_DIRECTORIES "")
	set(${PROJECT_NAME}_INCLUDE_DIRS "" CACHE INTERNAL "${PROJECT_NAME} public include directories")
	set(${PROJECT_NAME}_PRIVATE_INCLUDE_DIRS "")

	set(${PROJECT_NAME}_LIBRARIES "" CACHE INTERNAL "${PROJECT_NAME} public libraries")
	set(${PROJECT_NAME}_PRIVATE_LIBRARIES "")

	# Resets link directories (e.g library directories, e.g library paths)
	set_directory_properties(PROPERTIES LINK_DIRECTORIES "")
	set(${PROJECT_NAME}_LIBRARY_DIRS "" CACHE INTERNAL "${PROJECT_NAME} public library directories")
	set(${PROJECT_NAME}_PRIVATE_LIBRARY_DIRS "" CACHE INTERNAL "${PROJECT_NAME} private library directories")

	# Resets the definitions
	set_directory_properties(PROPERTIES DEFINITIONS "")
	set_directory_properties(PROPERTIES DEFINITION "")
	set(${PROJECT_NAME}_DEFINITIONS "" CACHE INTERNAL "${PROJECT_NAME} public definitions")
	set(${PROJECT_NAME}_PRIVATE_DEFINITIONS "")

	# Resets compile flags
	set(${PROJECT_NAME}_PRIVATE_COMPILE_FLAGS "")

	# Resets link flags
	set(${PROJECT_NAME}_PUBLIC_LINK_FLAGS "")
	set(${PROJECT_NAME}_PRIVATE_LINK_FLAGS "")

	# Resets build version and API version
	set(${PROJECT_NAME}_BUILD_VERSION "")
	set(${PROJECT_NAME}_API_VERSION "")

endmacro (ow_create_project)
