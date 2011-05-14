# - ow_unique(uniqueList list)
# Internal function, makes the given list have only one instance of each unique element
#
# See http://voxel.jouy.inra.fr/darcs/contrib-itk/WrapITK-unstable/CMakeUtilityFunctions.cmake
#
# Example:
# ow_unique(unique ${${PROJECT_NAME}_PUBLIC_LINK_FLAGS} ${${PROJECT_NAME}_PRIVATE_LINK_FLAGS})
#
# Copyright (C) 2006-2007  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


macro (ow_unique uniqueList)
	set(uniqueTmp
		""
	)

	foreach (loop ${ARGN})
		if (NOT "${uniqueTmp}" MATCHES "(^|;)${loop}(;|$)")
			set(uniqueTmp
				${uniqueTmp}
				${loop}
			)
		endif (NOT "${uniqueTmp}" MATCHES "(^|;)${loop}(;|$)")
	endforeach (loop)

	set(${uniqueList}
		${uniqueTmp}
	)
endmacro (ow_unique)
