# - ow_locale_release(tsDir)
# Generates translations, this is specific to Qt-4
#
# Runs lrelease using .ts files and generates .qm files
# See ow_locale_update()
#
# Example:
# ow_locale_release(path/to/ts_files)
# $> make lrelease
# This will generate the .qm files
#
# If you want to build generate .qm files automatically when running make, add
# this line:
# add_dependencies(your_main_target lrelease)
#
# Copyright (C) 2007  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


# qmFiles = output
# ARGN = input, .ts files
macro (ow_lrelease qmFiles)
	# Location for translation files (*.qm), ${BUILD_DIR}/lang by default
	file(MAKE_DIRECTORY ${LOCALE_COPY_DIR})

	foreach(tsFile ${ARGN})

		get_filename_component(basename ${tsFile} NAME_WE)

		get_filename_component(qmFile
			${LOCALE_COPY_DIR}/${basename}.qm
			ABSOLUTE
		)

		add_custom_command(
			OUTPUT
				${qmFile}
			COMMAND
				${QT_LRELEASE_EXECUTABLE}
				-nounfinished
				-verbose
				${tsFile}
				-qm ${qmFile}
			DEPENDS
				${tsFile}
			WORKING_DIRECTORY
				${CMAKE_CURRENT_SOURCE_DIR}
			COMMENT
				"Runs lrelease command"
		)

		set(qmFiles ${${qmFiles}} ${qmFile})

	endforeach(tsFile ${ARGN})

endmacro (ow_lrelease)


macro (ow_locale_release tsDir)

	find_program(QT_LRELEASE_EXECUTABLE
		NAMES
			lrelease-qt4
			lrelease
		PATHS
			$ENV{QTDIR}/bin
			"[HKEY_CURRENT_USER\\Software\\Trolltech\\Qt3Versions\\4.0.0;InstallDir]/bin"
			"[HKEY_CURRENT_USER\\Software\\Trolltech\\Versions\\4.0.0;InstallDir]/bin"
	)

	message(STATUS "Found lrelease: ${QT_LRELEASE_EXECUTABLE}")

	file(GLOB tsFiles ${tsDir}/*.ts)

	ow_lrelease(qmFiles ${tsFiles})
	add_custom_target(
		lrelease DEPENDS ${qmFiles}
	)

endmacro (ow_locale_release)
