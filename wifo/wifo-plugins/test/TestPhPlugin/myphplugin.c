#include <windows.h>
#include <stdio.h>
#include "myPhPlugin.h"

#ifdef __cplusplus
extern "C" {
#endif

static int natping_interval;
static int ping_nated_only;
static char rtpproxy_sock[50] = "unix:/var/run/rtpproxy.sock\0"; /* list */


int cmd_function1(int int1, char * string1, int int2, char * string2);
int callback_function1(OWPL_EVENT_CATEGORY category, 
                                         void* pInfo, 
                                         void* pUserData);

FileTransferReceivedCb_T OnFileTransferReceived = 0;
FileTransferProgressCb_T OnFileTransferProgress = 0;


OWPL_PLUGIN_CMD cmds[] = {
	{"fix_nated_contact",  cmd_function1,    0, 0},
	{"fix_nated_sdp",      cmd_function1,        4, 0},
	{"unforce_rtp_proxy",  cmd_function1,    0, 0},
	{"force_rtp_proxy",    cmd_function1,     0, 0},
	{"force_rtp_proxy",    cmd_function1,     1, 0},
	{"force_rtp_proxy",    cmd_function1,     2, 0},
	{"nat_uac_test",       cmd_function1,         1, 0},
	{"fix_nated_register", cmd_function1,   0, 0},
	{"add_rcv_param",      cmd_function1,        0, 0},
	{0, 0, 0, 0 }
};

OWPL_PLUGIN_CALLBACK callbacks[] = {
	{"FileTransferReveiced",  (void*)&OnFileTransferReceived},
	{"FileTransferProgress",  (void*)&OnFileTransferProgress},
	{"callback3",  0},
	{0, 0}
};

OWPL_PLUGIN_PARAM params[] = {
	{"natping_interval",      OWPL_PARAM_INT, &natping_interval, sizeof(int)},
	{"ping_nated_only",       OWPL_PARAM_INT, &ping_nated_only, sizeof(int)},
	{"rtpproxy_sock",         OWPL_PARAM_STR, rtpproxy_sock, sizeof(rtpproxy_sock)},	
	{0, 0, 0}
};

OWPL_PLUGIN_CONTENT_TYPE content_type  = {"SomeContentType", callback_function1 };

OWPL_DECLARE_EXPORT OWPL_PLUGIN_EXPORT exports = {
	"MyTestPlugin",
	OWPL_PLUGIN_VERSION,
	&content_type,
	cmds,
	callbacks,
	params,
	0,
	0, /* destroy function */
	0, /* param getter function*/
	0  /* param setter function*/
};

int cmd_function1(int int1, char * string1, int int2, char * string2)
{
	char buf[1000];
	
	_snprintf(buf, sizeof(buf), "%d   %s    %d   %s", int1, string1, int2, string2);

	MessageBoxA(NULL, buf, "The function cmd_function1 is called with following param", 0);

	if (OnFileTransferReceived)
	{
		OnFileTransferReceived("FileName1", "someone", 100);
	}
	return 0;
}

int callback_function1(OWPL_EVENT_CATEGORY category, 
                                         void* pInfo, 
                                         void* pUserData)
{
	MessageBoxA(NULL, "The CALLBACK function callback_function1 is called", "INFO", 0);
	return 0;
}

#ifdef __cplusplus
}
#endif