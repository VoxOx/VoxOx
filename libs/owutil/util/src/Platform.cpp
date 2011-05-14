/*
 * VOXOX!!!

 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2007  Wengo
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <util/Platform.h>

#ifdef OS_WINDOWS
#include <windows.h>
#endif

//-----------------------------------------------------------------------------

std::string Platform::getPlatformCode()
{
	std::string platform = "";

#ifdef OS_WINDOWS
	if		( Platform::is64BitOs() )
		platform = "W64";
	else if ( Platform::is32BitOs() )
		platform = "W32";
	else
		platform = "W32";

#elif defined (OS_MACOSX)
	#if defined (__BIG_ENDIAN__)
		platform = "MPPC";
	#else
		platform = "MI";
	#endif
#elif defined (OS_LINUX)
	platform = "L";
#endif

	return platform;
}

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
#ifdef OS_WINDOWS

bool Platform::is64BitOs()
{
	bool result = false;
#if defined _WIN64
	result = true;
#elif defined _WIN32
	// 32-bit programs run on both 32-bit and 64-bit Windows so must check
	result = isWow64();
#endif

	return result;
}

//-----------------------------------------------------------------------------

bool Platform::is32BitOs()
{
	bool result = false;
#if defined _WIN64
	result = false;
#elif defined _WIN32
	// 32-bit programs run on both 32-bit and 64-bit Windows so must check
	result = !isWow64();
#endif

	return result;
}

//-----------------------------------------------------------------------------

//VOXOX - JRT - 2009.12.16 - Not all Windows versions support IsWow64Process,
//							 so we must check that it exists in kernel32.dll
typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

LPFN_ISWOW64PROCESS fnIsWow64Process;

bool Platform::isWow64()
{
    BOOL bIsWow64 = FALSE;

    fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress( GetModuleHandle(TEXT("kernel32")),"IsWow64Process");
  
    if (NULL != fnIsWow64Process)
    {
        if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
        {
            // handle error
        }
    }

    return (bIsWow64 != 0);
}

//-----------------------------------------------------------------------------
#endif //OS_WINDOWS