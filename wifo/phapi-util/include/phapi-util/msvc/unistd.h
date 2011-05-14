#ifndef __UNISTD_H__
#define __UNISTD_H__

#include <winbase.h>

#include <phapi-util/phapiutildll.h>

#define sleep(sec)			Sleep(sec*1000)
#define usleep(microsec)	Sleep((int)microsec/1000)


#define F_GETFL		0
#define F_SETFL		FIONBIO
#define O_NONBLOCK	1

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	PHAPIUTIL_API int fcntl(int fd, int cmd, long arg);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __UNISTD_H__ */
