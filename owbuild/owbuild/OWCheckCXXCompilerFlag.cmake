# - ow_check_cxx_compiler_flag(flag result)
# Checks whether the compiler supports a given flag
#
# Example:
# ow_check_cxx_compiler_flag("-fPIC" WITH_FPIC)
# if (WITH_FPIC)
# 	add_definitions(-fPIC)
# endif (WITH_FPIC)
#
# Copyright (C) 2006  Alexander Neundorf <neundorf@kde.org>
# Copyright (C) 2006-2007  Wengo
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING file.


include(CheckCXXSourceCompiles)

macro (ow_check_cxx_compiler_flag flag result)
	set(SAFE_CMAKE_REQUIRED_DEFINITIONS "${CMAKE_REQUIRED_DEFINITIONS}")
	set(CMAKE_REQUIRED_DEFINITIONS "${flag}")
	check_cxx_source_compiles("int main(){return 0;}" ${result})
	set(CMAKE_REQUIRED_DEFINITIONS "${SAFE_CMAKE_REQUIRED_DEFINITIONS}")
endmacro (ow_check_cxx_compiler_flag)
