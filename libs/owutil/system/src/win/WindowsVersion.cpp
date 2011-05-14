/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2006  Wengo
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

#include <system/WindowsVersion.h>

#include <windows.h>
#include <stdio.h>

#define BUFSIZE 80
#define SM_SERVERR2 89

typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);

std::string WindowsVersion::_version = "";

char * WindowsVersion::getVersion() 
{
	OSVERSIONINFOEX osvi;
	SYSTEM_INFO si;
	PGNSI pGNSI;
	BOOL bOsVersionInfoEx;

	char toReturn[2048];
	char temp[512];

	//Try calling GetVersionEx using the OSVERSIONINFOEX structure.
	//If that fails, try using the OSVERSIONINFO structure.

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if (!(bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO *) &osvi))) {
		osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		if (!GetVersionEx((OSVERSIONINFO *) &osvi)) {
			return toReturn;
		}
	}

	switch (osvi.dwPlatformId) {

	//Test for the Windows NT product family.
	case VER_PLATFORM_WIN32_NT:
		//Test for the specific product.

		//VOXOX - JRT - 2009.11.03 - Add support for Windows 7
		if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1) 
		{
			if (osvi.wProductType == VER_NT_WORKSTATION) 
			{
				strcpy(toReturn, "Microsoft Windows 7 ");
				printf( toReturn );
			} 
			else 
			{
				strcpy(toReturn, "Windows Server 2008 R2 ");
				printf( toReturn );
			}
		}

		if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0) 
		{
			if (osvi.wProductType == VER_NT_WORKSTATION) 
			{
				strcpy(toReturn, "Microsoft Windows Vista ");
				printf(  toReturn );
			} 
			else 
			{
				strcpy(toReturn, "Windows Server 2008 ");
				printf( toReturn );
			}
		}

		if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2) 
		{
		//Use GetProcAddress to avoid load issues on Windows 2000
			pGNSI = (PGNSI) GetProcAddress(GetModuleHandle("kernel32.dll"), "GetNativeSystemInfo");
			if (NULL != pGNSI) 
			{
				pGNSI(&si);
			}

			if (GetSystemMetrics(SM_SERVERR2)) 
			{
				//toReturn = "Microsoft Windows Server 2003 \"R2\" ";
				strcpy(toReturn, "Microsoft Windows Server 2003 \"R2\" ");
				printf( toReturn );
			}
			else 
			{
				if (osvi.wProductType == VER_NT_WORKSTATION && si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64) 
				{
					strcpy(toReturn, "Microsoft Windows XP Professional x64 Edition");
					printf( toReturn );
				} 
				else 
				{
					strcpy(toReturn, "Microsoft Windows Server 2003, ");
					printf( toReturn );
				}
			}
		}

		if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1) 
		{
			strcpy(toReturn, "Microsoft Windows XP ");
			printf( toReturn );
		}

		if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0) 
		{
			strcpy(toReturn, "Microsoft Windows 2000 ");
			printf( toReturn );
		}

		if (osvi.dwMajorVersion <= 4) 
		{
			strcpy(toReturn, "Microsoft Windows NT ");
			printf( toReturn );
		}

		//Test for specific product on Windows NT 4.0 SP6 and later.
		if (bOsVersionInfoEx) 
		{
			//Test for the workstation type.
			if (osvi.wProductType == VER_NT_WORKSTATION && si.wProcessorArchitecture!=PROCESSOR_ARCHITECTURE_AMD64) 
			{
				if (osvi.dwMajorVersion == 4) 
				{
					strncat(toReturn, "Workstation 4.0 ", 16);
					printf("Workstation 4.0 ");
				} 
				else 
				{
					if (osvi.wSuiteMask & VER_SUITE_PERSONAL) 
					{
						strncat(toReturn, "Home Edition ", 13);
						printf("Home Edition ");
					} 
					else 
					{
						strncat(toReturn, "Professional ", 13);
						printf("Professional ");
					}
				}
			} 
			else 
			{
				if (osvi.wProductType == VER_NT_SERVER || osvi.wProductType == VER_NT_DOMAIN_CONTROLLER) {
					if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2) {
						if (si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_IA64) {
							if (osvi.wSuiteMask & VER_SUITE_DATACENTER) {
								strncat(toReturn, "Datacenter Edition for Itanium-based Systems", 44);
								printf("Datacenter Edition for Itanium-based Systems");
							} else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE) {
								strncat(toReturn, "Enterprise Edition for Itanium-based Systems", 44);
								printf("Enterprise Edition for Itanium-based Systems");
							}
						} else if (si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64) {
							if (osvi.wSuiteMask & VER_SUITE_DATACENTER) {
								strncat(toReturn, "Datacenter x64 Edition ", 23);
								printf("Datacenter x64 Edition ");
							} else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE) {
								strncat(toReturn, "Enterprise x64 Edition ", 23);
								printf("Enterprise x64 Edition ");
							} else {
								strncat(toReturn, "Standard x64 Edition ", 21);
								printf("Standard x64 Edition ");
							}
						} else {
							if (osvi.wSuiteMask & VER_SUITE_DATACENTER) {
								strncat(toReturn, "Datacenter Edition ", 19);
								printf("Datacenter Edition ");
							} else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE) {
								strncat(toReturn, "Enterprise Edition ", 19);
								printf("Enterprise Edition ");
							} else if (osvi.wSuiteMask == VER_SUITE_BLADE) {
								strncat(toReturn, "Web Edition ", 12);
								printf("Web Edition ");
							} else {
								strncat(toReturn, "Standard Edition ", 17);
								printf("Standard Edition ");
							}
						}
					}
				}
				else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0) {
					if (osvi.wSuiteMask & VER_SUITE_DATACENTER) {
						strncat(toReturn, "Datacenter Server ", 18);
						printf("Datacenter Server ");
					} else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE) {
						strncat(toReturn, "Advanced Server ", 16);
						printf("Advanced Server ");
					}
					else {
						strncat(toReturn, "Server ", 7);
						printf("Server ");
					}
				} else {
				//Windows NT 4.0
					if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE) {
						strncat(toReturn, "Server 4.0, Enterprise Edition ", 31);
						printf("Server 4.0, Enterprise Edition ");
					} else {
						strncat(toReturn, "Server 4.0 ", 11);
						printf("Server 4.0 ");
					}
				}
			}
		}
		else 		//Test for specific product on Windows NT 4.0 SP5 and earlier
		{
			HKEY hKey;
			char szProductType[BUFSIZE];
			DWORD dwBufLen=BUFSIZE;
			LONG lRet;

			lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
				"SYSTEM\\CurrentControlSet\\Control\\ProductOptions",
				0, KEY_QUERY_VALUE, &hKey);
			if (lRet != ERROR_SUCCESS) {
				return toReturn;
			}

			lRet = RegQueryValueEx(hKey, "ProductType", NULL, NULL,
				(LPBYTE) szProductType, &dwBufLen);
			RegCloseKey(hKey);

			if ((lRet != ERROR_SUCCESS) || (dwBufLen > BUFSIZE)) {
				return toReturn;
			}

			if (lstrcmpi("WINNT", szProductType) == 0) {
				strncat(toReturn, "Workstation ", 12);
				printf("Workstation ");
			}
			if (lstrcmpi("LANMANNT", szProductType) == 0) {
				strncat(toReturn, "Server ", 7);
				printf("Server ");
			}
			if (lstrcmpi("SERVERNT", szProductType) == 0) {
				strncat(toReturn, "Advanced Server ", 16);
				printf("Advanced Server ");
			}
			itoa(osvi.dwMajorVersion, temp, 10);
			strcat(toReturn, temp);
			strcat(toReturn, ".");
			itoa(osvi.dwMinorVersion, temp, 10);
			strcat(toReturn, temp);
			printf("%d.%d ", osvi.dwMajorVersion, osvi.dwMinorVersion);
		}

		//Display service pack (if any) and build number.
		if (osvi.dwMajorVersion == 4 && lstrcmpi(osvi.szCSDVersion, "Service Pack 6") == 0) 
		{
			HKEY hKey;
			LONG lRet;

			//Test for SP6 versus SP6a.
			lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Hotfix\\Q246009", 0, KEY_QUERY_VALUE, &hKey);
			if (lRet == ERROR_SUCCESS) 
			{
				strncat(toReturn, "Service Pack 6a (Build ", 23);
				itoa(osvi.dwBuildNumber & 0xFFFF, temp, 10);
				strcat(toReturn, temp);
				strcat(toReturn, ")");
				printf("Service Pack 6a (Build %d)\n", osvi.dwBuildNumber & 0xFFFF);
			} 
			else 
			{
				//Windows NT 4.0 prior to SP6a
				strcat(toReturn, osvi.szCSDVersion);
				strcat(toReturn, " (Build ");
				itoa(osvi.dwBuildNumber & 0xFFFF, temp, 10);
				strcat(toReturn, temp);
				strcat(toReturn, ")");
				printf("%s (Build %d)\n", osvi.szCSDVersion, osvi.dwBuildNumber & 0xFFFF);
			}
			RegCloseKey(hKey);
		} 
		else 
		{
			//not Windows NT 4.0
			strcat(toReturn, osvi.szCSDVersion);
			strcat(toReturn, " (Build ");
			itoa(osvi.dwBuildNumber, temp, 10);
			strcat(toReturn, temp);
			strcat(toReturn, ")");
			printf("%s (Build %d)\n", osvi.szCSDVersion, osvi.dwBuildNumber & 0xFFFF);
		}
		break;

	//Test for the Windows Me/98/95.
	case VER_PLATFORM_WIN32_WINDOWS:
		if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0) {
			strcpy(toReturn, "Microsoft Windows 95 ");
			//toReturn = "Microsoft Windows 95 ";
			printf("Microsoft Windows 95 ");
			if (osvi.szCSDVersion[1]=='C' || osvi.szCSDVersion[1]=='B') {
				strcat(toReturn, "OSR2 ");
				printf("OSR2 ");
			}
		}

		if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10) {
			strcpy(toReturn, "Microsoft Windows 98 ");
			//toReturn = "Microsoft Windows 98 ";
			printf("Microsoft Windows 98 ");
			if (osvi.szCSDVersion[1]=='A' || osvi.szCSDVersion[1]=='B') {
				strcat(toReturn, "SE ");
				printf("SE ");
			}
		}

		if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90) {
			strcpy(toReturn, "Microsoft Windows Millennium Edition\n");
			//toReturn = "Microsoft Windows Millennium Edition\n";
			printf("Microsoft Windows Millennium Edition\n");
		}
		break;

	case VER_PLATFORM_WIN32s:
		strcpy(toReturn, "Microsoft Win32s\n");
		//toReturn = "Microsoft Win32s\n";
		printf("Microsoft Win32s\n");
		break;
	}
	return toReturn;
}
