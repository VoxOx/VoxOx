#ifndef PH_SHARED_LIB_H
#define PH_SHARED_LIB_H

#include <phapi-util/phapiutildll.h>

#if defined(WIN32) || defined(WIN32_WCE)

#include <windows.h>

#define RTLD_NOW 0

#define dlerror() 0
#define dlopen(a, b) LoadLibraryA(a)
#define dlsym(l, s)  GetProcAddress(l, s)
#define dlclose(l)  FreeLibrary(l)

#else

#define HLIB void *

#ifndef RTLD_NOW
/* for openbsd */
#define RTLD_NOW DL_LAZY
#endif

#endif


PHAPIUTIL_API int ph_is_shared_lib(const char *name);



#endif
