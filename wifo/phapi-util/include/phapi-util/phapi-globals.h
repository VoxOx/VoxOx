#ifndef __PHAPI_GLOBALS_H__
#define __PHAPI_GLOBALS_H__


#ifndef WIN32

#define SOCKET	int
#define HANDLE	int
#define __stdcall
#define _T(string) string
#define TCHAR   char

#endif /* WIN32 */

#ifdef WIN32

#define socklen_t	int
#define close	closesocket

#endif /* WIN32 */

#endif /* __PHAPI_GLOBALS_H__ */
