if (MSVC)
	# Copies Microsoft C and C++ runtime shared libraries to the ${BUILD_DIR} directory

	# Huston, we have a problem!
	# Under Visual C++ 8.0 it is not enough to copy runtime files
	# See http://msdn2.microsoft.com/en-us/library/ms235299(VS.80).aspx
	# See http://blogs.gotdotnet.com/martynl/archive/2005/10/13/480880.aspx

	if (CMAKE_BUILD_TYPE MATCHES Debug)
		if (MSVC71)
			ow_copy_file(C:/WINDOWS/system32/msvcr71d.dll ${BUILD_DIR})
			ow_copy_file(C:/WINDOWS/system32/msvcp71d.dll ${BUILD_DIR})
		endif (MSVC71)
		if (MSVC80)
			#ow_copy_file(C:/WINDOWS/WinSxS/x86_Microsoft.VC80.DebugCRT_1fc8b3b9a1e18e3b_8.0.50727.42_x-ww_f75eb16c/msvcr80d.dll ${BUILD_DIR})
			#ow_copy_file(C:/WINDOWS/WinSxS/x86_Microsoft.VC80.DebugCRT_1fc8b3b9a1e18e3b_8.0.50727.42_x-ww_f75eb16c/msvcp80d.dll ${BUILD_DIR})
			#ow_copy_file(C:/WINDOWS/WinSxS/x86_Microsoft.VC80.DebugCRT_1fc8b3b9a1e18e3b_8.0.50727.42_x-ww_f75eb16c/msvcm80d.dll ${BUILD_DIR})
		endif (MSVC80)
	else (CMAKE_BUILD_TYPE MATCHES Debug)
		if (MSVC71)
			ow_copy_file(C:/WINDOWS/system32/msvcr71.dll ${BUILD_DIR})
			ow_copy_file(C:/WINDOWS/system32/msvcp71.dll ${BUILD_DIR})
		endif (MSVC71)
		if (MSVC80)
			#ow_copy_file(C:/WINDOWS/WinSxS/x86_Microsoft.VC80.CRT_1fc8b3b9a1e18e3b_8.0.50727.42_x-ww_0de06acd/msvcr80.dll ${BUILD_DIR})
			#ow_copy_file(C:/WINDOWS/WinSxS/x86_Microsoft.VC80.CRT_1fc8b3b9a1e18e3b_8.0.50727.42_x-ww_0de06acd/msvcp80.dll ${BUILD_DIR})
			#ow_copy_file(C:/WINDOWS/WinSxS/x86_Microsoft.VC80.CRT_1fc8b3b9a1e18e3b_8.0.50727.42_x-ww_0de06acd/msvcm80.dll ${BUILD_DIR})
		endif (MSVC80)

	endif (CMAKE_BUILD_TYPE MATCHES Debug)
endif (MSVC)
