# - ow_use_public_libraries(lib1 ... libN)
# Uses publicly a library inside the current project: imports properties from a library to the current project
#
# See ow_use_private_libraries() for complete documentation
#
# Term 'public' means that public attributes inherited will stay
# public attributes once inherited and will be propagated to subprojects
#
# Copyright (C) 2006-2007  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


macro (ow_use_public_libraries)

	ow_check_project()

	foreach (loop ${ARGN})
		if (NOT ${loop}_INCLUDE_DIRS)
			if (NOT ${loop}_INCLUDE_DIR)
				message(FATAL_ERROR "${PROJECT_NAME}: ${loop}_INCLUDE_DIRS and ${loop}_INCLUDE_DIR empty,"
					" check that ${loop} is declared before ${PROJECT_NAME}")
			endif (NOT ${loop}_INCLUDE_DIR)
		endif (NOT ${loop}_INCLUDE_DIRS)
		ow_add_public_include_dirs(
			${${loop}_INCLUDE_DIRS}
			${${loop}_INCLUDE_DIR}
		)

		#if (NOT ${loop}_LIBRARIES)
		#	message(FATAL_ERROR "${loop}_LIBRARIES empty")
		#endif (NOT ${loop}_LIBRARIES)
		if (${loop}_LIBRARIES)
			ow_add_public_libraries(
				${${loop}_LIBRARIES}
			)
		endif (${loop}_LIBRARIES)
		if (${loop}_LIBRARY)
			ow_add_public_libraries(
				${${loop}_LIBRARY}
			)
		endif (${loop}_LIBRARY)

		if (${loop}_LIBRARY_DIRS)
			ow_add_public_library_dirs(
				${${loop}_LIBRARY_DIRS}
			)
		endif (${loop}_LIBRARY_DIRS)

		if (${loop}_DEFINITIONS)
			ow_add_public_definitions(
				${${loop}_DEFINITIONS}
			)
		endif (${loop}_DEFINITIONS)

		if (${loop}_PUBLIC_LINK_FLAGS)
			ow_add_public_link_flags(
				${${loop}_PUBLIC_LINK_FLAGS}
			)
		endif (${loop}_PUBLIC_LINK_FLAGS)
	endforeach (loop)

endmacro (ow_use_public_libraries)
