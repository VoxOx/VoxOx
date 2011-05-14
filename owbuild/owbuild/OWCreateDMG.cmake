# - ow_create_dmg(name dir)
# Creates a .dmg for MacOSX
#
# ${name}: name of the .dmg to create
# ${dir}: path to .app to create the .dmg from
#
# Copyright (C) 2006-2007  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


macro (ow_create_dmg name dir)

	if (APPLE)
		execute_process(
			COMMAND
				"hdiutil" create -fs HFS+ -volname ${name} -srcfolder ${dir} ${BUILD_DIR}/${name}.dmg
		)
	endif (APPLE)

endmacro (ow_create_dmg)
