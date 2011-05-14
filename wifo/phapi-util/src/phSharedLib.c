#include <phapi-util/phSharedLib.h>

#if defined(WIN32) || defined(WIN32_WCE)

struct stat { int x; };

int ph_is_shared_lib(const char *name)
{
    return strstr(name, ".dll") || strstr(name, ".DLL");
}


#elif defined(OS_MACOSX)

#define HLIB void *
int ph_is_shared_lib(const char *name)
{
	return 0 != strstr(name, ".so"); // MH_BUNDLE library has a .so extension
}

#else /* !MACOSX */

#define HLIB void *
int ph_is_shared_lib(const char *name)
{
    return 0 != strstr(name, ".so");
}

#endif
