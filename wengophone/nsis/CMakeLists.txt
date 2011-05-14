# Bugfix, spaces don't work with install target
string(REPLACE "CMAKEBUGWITHSPACES" " " BUILD_DIR ${BUILD_DIR})
string(REPLACE "CMAKEBUGWITHSPACES" " " SOURCE_DIR ${SOURCE_DIR})

#-------------------------------------------------------------------------
# Ensure NSIS.exe exists and is executeable
#-------------------------------------------------------------------------
find_file(NSIS_PROGRAM
	NAMES
		makensis.exe
	PATHS
		"C:/Program Files/NSIS/ " "C:/Program Files (x86)/NSIS"	#JRT - 2010.01.27
)

if (NSIS_PROGRAM)
	message(STATUS "Found NSIS: ${NSIS_PROGRAM}")
else (NSIS_PROGRAM)
	message(FATAL_ERROR "Could not find NSIS")
endif (NSIS_PROGRAM)


#-------------------------------------------------------------------------
#JRT - determine early so we can use in Installer name.
#-------------------------------------------------------------------------
if (NOT SVN_REVISION OR SVN_REVISION EQUAL 0)
#	set(INSTALLER_NAME ${INSTALLER_NAME}-norev)
	set( REVNO revno )
else (NOT SVN_REVISION OR SVN_REVISION EQUAL 0)
#	set(INSTALLER_NAME ${INSTALLER_NAME}-${SVN_REVISION})
	set( REVNO ${SVN_REVISION} )
endif (NOT SVN_REVISION OR SVN_REVISION EQUAL 0)

set(INSTALLER_NAME
#	${APPLICATION_NAME}-${APPLICATION_VERSION}-setup
	${APPLICATION_NAME}-${APPLICATION_VERSION}.${REVNO}-setup
)

#-------------------------------------------------------------------------
# Set NSIS build arguments based on app name, build dir, etc.
#-------------------------------------------------------------------------
set(NSIS_ARGS
	/DPRODUCT_NAME=${APPLICATION_NAME}
	/DPRODUCT_VERSION=${APPLICATION_VERSION}
	/DBUILD_DIR=${BUILD_DIR}
	/DQTDIR=$ENV{QTDIR}
	/DBINARY_NAME=${BINARY_NAME}
	/DPRODUCT_VERSION_1=${APPLICATION_VERSION_1}
	/DPRODUCT_VERSION_2=${APPLICATION_VERSION_2}
	/DPRODUCT_VERSION_3=${APPLICATION_VERSION_3}
	/DPRODUCT_VERSION_4=${APPLICATION_VERSION_4}
)

#-------------------------------------------------------------------------
# Set NSIS build arguments based on build type.
#-------------------------------------------------------------------------
string(TOLOWER ${BUILD_TYPE} BUILD_TYPE)
if (BUILD_TYPE MATCHES debug)
	# Generate installer with .pdb files
	# Debug mode = with .pdb files always
	# MinSizeRel mode = no .pdb files
	# RelWithDebInfo mode = 2 setup.exe: one with .pdb files and one without
	set(NSIS_ARGS
		${NSIS_ARGS}
		/DDEBUG
		/DWITH_PDB
	)
endif (BUILD_TYPE MATCHES debug)

#-------------------------------------------------------------------------
# Set base installer name
#-------------------------------------------------------------------------
set(INSTALLER_NAME ${INSTALLER_NAME}-${BUILD_TYPE})


#-------------------------------------------------------------------------
# Because BuildBot cannot handle random setup names
# Generates 2 setup.exe: one with the -revrevnumber.exe and one with -rev${SVN_REVISION}.exe
#-------------------------------------------------------------------------
set(INSTALLER_NAME_REVNUMBER
	${INSTALLER_NAME}-revnumber
)

# moved up to generate better Installer name.
#if (NOT SVN_REVISION OR SVN_REVISION EQUAL 0)
#	set(INSTALLER_NAME ${INSTALLER_NAME}-norev)
#else (NOT SVN_REVISION OR SVN_REVISION EQUAL 0)
#	set(INSTALLER_NAME ${INSTALLER_NAME}-${SVN_REVISION})
#endif (NOT SVN_REVISION OR SVN_REVISION EQUAL 0)

set(INSTALLER_DIR
	${SOURCE_DIR}/wengophone/nsis
)

#-------------------------------------------------------------------------
# JRT - 2009.08.30 - so we can upload this file.
#-------------------------------------------------------------------------
set( ENV{CURBUILDPATH} "${INSTALLER_DIR}/${INSTALLER_NAME}.exe" )	#does not work as ENV VAR.
file( WRITE curbuildpath.txt "$ENV{CURBUILDPATH}\n" )			#Full path name for FTP
file( WRITE curbuild.txt "${INSTALLER_NAME}.exe\n" )			#File name to add to FTP path in email.

# Clear second file info, in case it is not used.
file( WRITE curbuildpath2.txt "" )
file( WRITE curbuild2.txt "" )

# Set file info for second files if needed.
if (BUILD_TYPE MATCHES relwithdebinfo)
	set( ENV{CURBUILDPATH2} "${INSTALLER_DIR}/${INSTALLER_NAME}-pdb.exe" )	#does not work as ENV VAR.
	file( WRITE curbuildpath2.txt "$ENV{CURBUILDPATH2}\n" )			#Full path name for FTP
	file( WRITE curbuild2.txt "${INSTALLER_NAME}-pdb.exe\n" )		#File name to add to FTP path in email.
endif (BUILD_TYPE MATCHES relwithdebinfo)

#-------------------------------------------------------------------------
# Execute NSIS command to create the installer
#-------------------------------------------------------------------------
file( WRITE nsiscmd.txt "COMMAND ${NSIS_PROGRAM} ${NSIS_ARGS} /DINSTALLER_NAME=${INSTALLER_NAME}.exe ${INSTALLER_DIR}/installer_2005.nsi\n" )
execute_process(
	COMMAND ${NSIS_PROGRAM} ${NSIS_ARGS} /DINSTALLER_NAME=${INSTALLER_NAME}.exe ${INSTALLER_DIR}/installer_2005.nsi
)

#-------------------------------------------------------------------------
# Because BuildBot cannot handle random setup names
# Generates 2 setup.exe: one with the -revnumber.exe and one with -${SVN_REVISION}.exe
# JRT - TODO: Review this.  Why do we want two .exe's which appear to just be copies of each other.
#-------------------------------------------------------------------------
# JRT - 2009.11.01 - I see no purpose in doing this copy.
#exec_program(
#		${CMAKE_COMMAND}
#	ARGS
#		"-E copy \"${INSTALLER_DIR}/${INSTALLER_NAME}.exe\" \"${INSTALLER_DIR}/${INSTALLER_NAME_REVNUMBER}.exe\""
#)

#-------------------------------------------------------------------------------------
# Now create the installer that contains the .PDB files used for debugging, if needed.
#-------------------------------------------------------------------------------------
if (BUILD_TYPE MATCHES relwithdebinfo)
	# Generate installer with .pdb files
	# Debug mode = with .pdb files always
	# MinSizeRel mode = no .pdb files
	# RelWithDebInfo mode = 2 setup.exe: one with .pdb files and one without
	set(NSIS_ARGS
		${NSIS_ARGS}
		/DWITH_PDB
	)

	execute_process(
#JRT		COMMAND ${NSIS_PROGRAM} ${NSIS_ARGS} /DINSTALLER_NAME=${INSTALLER_NAME}-pdb.exe ${INSTALLER_DIR}/installer.nsi
		COMMAND ${NSIS_PROGRAM} ${NSIS_ARGS} /DINSTALLER_NAME=${INSTALLER_NAME}-pdb.exe ${INSTALLER_DIR}/installer_2005.nsi
	)

#	# Because BuildBot cannot handle random setup names
#	# Generates 2 setup.exe: one with the -revnumber.exe and one with -${SVN_REVISION}.exe
# JRT - 2009.11.01 - I see no purpose in doing this copy.
#	exec_program(
#			${CMAKE_COMMAND}
#		ARGS
#			"-E copy \"${INSTALLER_DIR}/${INSTALLER_NAME}-pdb.exe\" \"${INSTALLER_DIR}/${INSTALLER_NAME_REVNUMBER}-pdb.exe\""
#	)
endif (BUILD_TYPE MATCHES relwithdebinfo)
