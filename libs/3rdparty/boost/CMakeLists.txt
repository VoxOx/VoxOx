if (NOT BOOST_FOUND)
	find_package(Boost REQUIRED)
endif (NOT BOOST_FOUND)

ow_create_project(boost)

ow_use_public_libraries(
	BOOST
)

if (NOT MSVC)
	set(boost_LIBRARIES
		${BOOST_PROGRAM_OPTIONS_LIBRARY}
		${BOOST_SERIALIZATION_LIBRARY}
		${BOOST_SIGNALS_LIBRARY}
		${BOOST_THREAD_LIBRARY}
		CACHE INTERNAL "${PROJECT_NAME} libraries"
	)
endif (NOT MSVC)

ow_add_public_definitions(
	-DBOOST_ALL_DYN_LINK
	${BOOST_LIB_DIAGNOSTIC_DEFINITIONS}
)

if (LINUX)
	# Bug fix under Mandriva 2007, -pthread should be defined
	ow_add_public_definitions(
		-pthread
	)
endif (LINUX)

if (MSVC)
	# Disable warnings with templates under Visual C++
	ow_add_public_definitions(
		/wd4251
		/wd4275
		/wd4675
	)

	ow_copy_file(${BOOST_LIBRARY_DIRS}/boost_thread${BOOST_LIBRARIES_SUFFIX}.dll ${BUILD_DIR})
	ow_copy_file(${BOOST_LIBRARY_DIRS}/boost_signals${BOOST_LIBRARIES_SUFFIX}.dll ${BUILD_DIR})
	ow_copy_file(${BOOST_LIBRARY_DIRS}/boost_serialization${BOOST_LIBRARIES_SUFFIX}.dll ${BUILD_DIR})
	ow_copy_file(${BOOST_LIBRARY_DIRS}/boost_program_options${BOOST_LIBRARIES_SUFFIX}.dll ${BUILD_DIR})
	ow_copy_file(${BOOST_LIBRARY_DIRS}/boost_python${BOOST_LIBRARIES_SUFFIX}.dll ${BUILD_DIR})
	#ow_copy_file(${BOOST_LIBRARY_DIRS}/boost_python${BOOST_LIBRARIES_SUFFIX}.dll ${BUILD_DIR})
endif (MSVC)

# Check compiler version
if (GCC41)
    if (BOOST_VERSION EQUAL 103301)
        set(gcc_message1 "There are problems with GCC 4.1.x and Boost 1.33.1.")
        set(gcc_message2 "See https://bugzilla.novell.com/show_bug.cgi?id=228524")
        if (CHECK_BOOST_GCC_BUG)
            message(FATAL_ERROR "
${gcc_message1}
${gcc_message2}

Please use a different version of Boost or GCC. If you really want to bypass this test, set
the CHECK_BOOST_GCC_BUG CMake variable to OFF.")
        else (CHECK_BOOST_GCC_BUG)
            message(STATUS "")
            message(STATUS "-------------------------------------------------------")
            message(STATUS "                      W A R N I N G")
            message(STATUS "")
            message(STATUS "${gcc_message1}")
            message(STATUS "${gcc_message2}")
            message(STATUS "")
            message(STATUS "Continuing since CHECK_BOOST_GCC_BUG is set to OFF.")
            message(STATUS "-------------------------------------------------------")
            message(STATUS "")
        endif (CHECK_BOOST_GCC_BUG)
    endif(BOOST_VERSION EQUAL 103301)
endif (GCC41)
