# - ow_locale_update(proFile dir1 ... dirN)
# Generates translations .ts files, this is specific to Qt-4
#
# Generates a QMake .pro file ${CMAKE_CURRENT_SOURCE_DIR}/lang.pro for translations
# .cpp, .c, .ui, .ts... files are found recursively using file(GLOB_REVERSE)
# then it runs lupdate using the .pro file to generate .ts files
# See ow_locale_release()
#
# QMake .pro file ${CMAKE_CURRENT_SOURCE_DIR}/lang.pro generated:
#
# HEADERS += header.h \
#            header.hh \
#            header.hpp \
#            header.hxx \
#            header.h++ \
#
# SOURCES += source.c \
#            source.cpp \
#            source.cc \
#            source.cxx \
#            source.c++ \
#
# FORMS += form.ui \
#
# TRANSLATIONS += translation.ts \
#
# Example:
# add_custom_target(lupdate
# 	COMMAND
# 		"${CMAKE_COMMAND}"
# 		-DPRO_FILE="${PRO_FILE}"
# 		-P "${CMAKE_CURRENT_SOURCE_DIR}/owbuild/owbuild/OWLocaleUpdate.cmake"
# 	WORKING_DIRECTORY
# 		${CMAKE_CURRENT_SOURCE_DIR}
# )
# $> make lupdate
# This will create a file ${CMAKE_CURRENT_SOURCE_DIR}/lang.pro + update .ts files
#
# Copyright (C) 2007  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


# Creates the QMake .pro file
# tsFiles = output
# proFile = intput
# ARGN = intput, directories where to look for source files
macro (ow_generate_pro_file tsFiles proFile)

	set(headers "")
	set(sources "")
	set(forms "")
	set(translations "")

	message(STATUS "Please wait while generating the QMake .pro file...")
	message(STATUS "QMake .pro file: " ${proFile})

	# Other possible algorithm:
	# file(GLOB_RECURSE headers ${dir}/*.h)
	# file(GLOB_RECURSE sources ${dir}/*.cpp)
	# file(GLOB_RECURSE forms ${dir}/*.ui)
	# file(GLOB_RECURSE translations ${dir}/*.ts)

	foreach (dir ${ARGN})

		message(STATUS "Scan directory: "${dir})

		file(GLOB_RECURSE fileList ${dir}/*)

		if (fileList)
			foreach (file ${fileList})

				get_filename_component(ext ${file} EXT)

				# Removes all path containing .svn or CVS
				if (NOT ${file} MATCHES ".*\\.svn|CVS.*")

					if (ext)

						# Add file to ${headers}
						if ("${ext}" STREQUAL ".h")
							set(headers ${headers} ${file})
						endif ("${ext}" STREQUAL ".h")
						if ("${ext}" STREQUAL ".hh")
							set(headers ${headers} ${file})
						endif ("${ext}" STREQUAL ".hh")
						if ("${ext}" STREQUAL ".hpp")
							set(headers ${headers} ${file})
						endif ("${ext}" STREQUAL ".hpp")
						if ("${ext}" STREQUAL ".hxx")
							set(headers ${headers} ${file})
						endif ("${ext}" STREQUAL ".hxx")
						#if ("${ext}" STREQUAL ".h++")
						#	set(headers ${headers} ${file})
						#endif ("${ext}" STREQUAL ".h++")

						# Add file to ${sources}
						if ("${ext}" STREQUAL ".c")
							set(sources ${sources} ${file})
						endif ("${ext}" STREQUAL ".c")
						if ("${ext}" STREQUAL ".cpp")
							set(sources ${sources} ${file})
						endif ("${ext}" STREQUAL ".cpp")
						if ("${ext}" STREQUAL ".cc")
							set(sources ${sources} ${file})
						endif ("${ext}" STREQUAL ".cc")
						if ("${ext}" STREQUAL ".cxx")
							set(sources ${sources} ${file})
						endif ("${ext}" STREQUAL ".cxx")
						#if ("${ext}" STREQUAL ".c++")
						#	set(sources ${sources} ${file})
						#endif ("${ext}" STREQUAL ".c++")

						# Add file to ${forms}
						if ("${ext}" STREQUAL ".ui")
							set(forms ${forms} ${file})
						endif ("${ext}" STREQUAL ".ui")

						# Add file to ${translations}
						if ("${ext}" STREQUAL ".ts")
							set(translations ${translations} ${file})
						endif ("${ext}" STREQUAL ".ts")

					endif (ext)

				endif (NOT ${file} MATCHES ".*\\.svn|CVS.*")

			endforeach (file ${fileList})
		endif (fileList)

	endforeach (dir ${ARGN})

	message(STATUS "Write ${proFile}...")

	# file(WRITE ) overwrites the file if it already exists, creates the file if it does not exist
	file(WRITE ${proFile} "")

	if (headers)
		file(APPEND ${proFile} "HEADERS += \\\n")
		foreach (header ${headers})
			file(APPEND ${proFile} "           \"${header}\" \\\n")
		endforeach (header ${headers})
		file(APPEND ${proFile} "\n\n")
	endif (headers)

	if (sources)
		file(APPEND ${proFile} "SOURCES += \\\n")
		foreach (source ${sources})
			file(APPEND ${proFile} "           \"${source}\" \\\n")
		endforeach (source ${sources})
		file(APPEND ${proFile} "\n\n")
	endif (sources)

	if (forms)
		file(APPEND ${proFile} "FORMS += \\\n")
		foreach (form ${forms})
			file(APPEND ${proFile} "         \"${form}\" \\\n")
		endforeach (form ${forms})
		file(APPEND ${proFile} "\n\n")
	endif (forms)

	if (translations)
		file(APPEND ${proFile} "TRANSLATIONS += \\\n")
		foreach (translation ${translations})
			file(APPEND ${proFile} "                \"${translation}\" \\\n")
		endforeach (translation ${translations})
		file(APPEND ${proFile} "\n\n")
	endif (translations)

	set(${tsFiles} ${translations})

endmacro (ow_generate_pro_file)


# Runs lupdate
# proFile = input
macro (ow_lupdate proFile)

	find_program(QT_LUPDATE_EXECUTABLE
		NAMES
			lupdate-qt4
			lupdate
		PATHS
			$ENV{QTDIR}/bin
			"[HKEY_CURRENT_USER\\Software\\Trolltech\\Qt3Versions\\4.0.0;InstallDir]/bin"
			"[HKEY_CURRENT_USER\\Software\\Trolltech\\Versions\\4.0.0;InstallDir]/bin"
	)

	message(STATUS "Found lupdate: ${QT_LUPDATE_EXECUTABLE}")

	execute_process(
		COMMAND
			${QT_LUPDATE_EXECUTABLE}
			-noobsolete
			-verbose
			${proFile}
		WORKING_DIRECTORY
			${CMAKE_CURRENT_SOURCE_DIR}
	)

endmacro (ow_lupdate)


macro (ow_locale_update proFile)
	ow_generate_pro_file(tsFiles ${proFile} ${ARGN})
	ow_lupdate(${proFile})
	message(STATUS "Translations .ts files generated")
endmacro (ow_locale_update)


# FIXME This is specific to WengoPhone
# Didn't find another way to do it :(
# maybe via ${CMAKE_COMMAND} -D
ow_locale_update(
	${PRO_FILE}
	${CMAKE_CURRENT_SOURCE_DIR}/wengophone/src/presentation
	${CMAKE_CURRENT_SOURCE_DIR}/crashreport
	${CMAKE_CURRENT_SOURCE_DIR}/libs/qtutil
	${CMAKE_CURRENT_SOURCE_DIR}/libs/owbrowser
)
