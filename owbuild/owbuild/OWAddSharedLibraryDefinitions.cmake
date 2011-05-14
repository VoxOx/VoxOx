# - ow_add_shared_library_definitions()
# Internal function, adds shared library definitions for declspec(dllimport) and declspec(dllexport)
#
# To build a shared library under Windows you need to define:
# #ifdef MYLIB_DLL
# 	#ifdef BUILD_MYLIB_DLL
# 		#define MYLIB_API __declspec(dllexport)
# 	#else
# 		#define MYLIB_API __declspec(dllimport)
# 	#endif
# #else
# 	#define MYLIB_API
# #endif
#
# MYLIB_API int get_a_value();
#
# The original CMakeLists.txt:
# ow_create_shared_library(mylib)
#
# ow_add_public_definitions(
# 	-DMYLIB_DLL
# )
#
# ow_add_private_definitions(
# 	-DMYLIB_DLL
# 	-DBUILD_MYLIB_DLL
# )
#
# ow_add_sources(
# 	mylib.c
# )
#
# ow_create_project_binary()
#
# ow_add_shared_library_definitions() does the job for you, thus your
# CMakeLists.txt looks like:
# ow_create_shared_library(mylib)
#
# ow_add_sources(
# 	mylib.c
# )
#
# ow_create_project_binary()
#
# If mylib project is set to static library rather than shared library from cache,
# then defines will be automatically removed
#
# Copyright (C) 2006-2007  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


macro (ow_add_shared_library_definitions)

	ow_check_project()

	string(TOUPPER ${PROJECT_NAME} tmp)

	# Avoid warning like this:
	# <command line>:1:4: warning: missing whitespace after the macro name
	string(REPLACE "-" "_" tmp ${tmp})

	ow_add_public_definitions(
		-D${tmp}_DLL
	)

	#ow_add_private_definitions(
	#	-D${tmp}_DLL
	#	-DBUILD_${tmp}_DLL
	#)
	set_target_properties(${PROJECT_NAME} PROPERTIES DEFINE_SYMBOL "BUILD_${tmp}_DLL -D${tmp}_DLL")

endmacro (ow_add_shared_library_definitions)
