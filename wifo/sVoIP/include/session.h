 /*
	Zenbow Secure VoIP.
	Copyright (C) 2006 Everbee Networks 

	* This program is free software; you can redistribute it and/or
	* modify it under the terms of the GNU General Public License
	* as published by the Free Software Foundation; either version 2
	* of the License, or (at your option) any later version.

	* This program is distributed in the hope that it will be useful,
	* but WITHOUT ANY WARRANTY; without even the implied warranty of
	* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	* GNU General Public License for more details.

	* You should have received a copy of the GNU General Public License
	* along with this program; if not, write to the Free Software
	* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

	* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
	* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
	* COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
	* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
	* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
	* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
	* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
	* OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef _SESSION_H_
#define _SESSION_H_

#define EVRB_SES_TIMEOUT (4*3600)
#define EVRB_CALLID_SIZE 64

#define EVRB_BADPARAM -1
#define EVRB_SESSION_EXISTS -2
#define EVRB_NOSESSION -3
#define EVRB_MEMERROR -4


#ifdef WIN32
#include <winsock.h>
#include <stdlib.h>
#include <io.h>
#include <time.h>
#else

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h> 
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <net/if.h>

#endif

#include <shmem.h>

typedef struct evrb_seskey_t {
	char callid[EVRB_CALLID_SIZE];
} EVRB_SESKEY;

typedef enum {EVRB_SES_ACTIVE, EVRB_SES_LOCAL, EVRB_SES_DIST, EVRB_SES_NONE} EVRB_SES_STATUS;

typedef struct evrb_station_t {
	struct in_addr ip;
	unsigned short port;
	void* crypto;
} EVRB_STATION;

typedef struct evrb_session_t {
	EVRB_SESKEY key;
	EVRB_SES_STATUS status; //POC status
	time_t timestamp;
	EVRB_STATION local;
	EVRB_STATION distant;
	struct evrb_session_t* previous;
	struct evrb_session_t* next;
	int state; //-1 -> 0 -> 1 -> 2 -> -1
} EVRB_SESSION;

typedef struct evrb_session_list_t {
	unsigned int count;
	EVRB_SESSION* first;
} EVRB_SESSION_LIST;

#endif
