#include <winsock.h>

#include <unistd.h>

int fcntl(int fd, int cmd, long arg) {
	if(cmd == F_SETFL) {
		return ioctlsocket(fd, cmd, &arg);
	}
	if(cmd == F_GETFL) {
		return 1;
	}
	return 0;
}
