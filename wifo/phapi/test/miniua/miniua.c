/*
* miniua -  minimalistic sip user agent
*
* Copyright (C) 2002,2003   Aymeric Moizard <jack@atosc.org>
* Copyright (C) 2004        Vadim Lebedev <vadim@mbdsys.com>
*
* This is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2,
* or (at your option) any later version.
*
* This is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public
* License along with dpkg; if not, write to the Free Software
* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
* @file miniua.c
* @brief minimalistic SIP User Agent
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
//#include <sys/wait.h>
#include <errno.h>
//#include <unistd.h>
//#include <dirent.h>
#include <limits.h>
#include <ctype.h>
#include <assert.h>
#include <signal.h>

#ifdef WIN32
#include <process.h>
#define snprintf _snprintf
#define strncasecmp strnicmp
#define sleep(x) Sleep(x*1000)
typedef unsigned int pthread_t;
#else
#include <pthread.h>
#endif

#include "phapi.h"
#include "phrpc.h"


enum CnfStates {
	NOCONF,
	WAITCALL1,
	WAITCALL2,
	WAITHOLD1,
	HALFCONF,
	CONFREADY
};

enum CMDS {
	CMD_CALL=1,
	CMD_HANGUP,
	CMD_ACCEPT,
	CMD_REJECT,
	CMD_RING,
	CMD_HOLD,
	CMD_RESUME,
	CMD_BUSY,
	CMD_DTMF,
	CMD_BTXFER,
	CMD_ATXFER,
	CMD_XFERCNF,
	CMD_OPT,
	CMD_VLADD,
	CMD_VLDEL,
	CMD_DTMFMODE,
	CMD_AUTOANS,
	CMD_AUTOREJ,
	CMD_AUTORING,
	CMD_SENDSF,
	CMD_AUTHADD,
	CMD_AUTHADD_2,
	CMD_SETID,
	CMD_EXIT,
	CMD_QUIT,
	CMD_LCALL,
	CMD_FOLLOW,
	CMD_SENDF,
	CMD_SLEEP,
	CMD_COMMENT,
	CMD_EBREAK,
	CMD_SETV,
	CMD_NATINFO,
	CMD_ECHO,
	CMD_CONF,
	CMD_NOCONF,
	CMD_LCONF,
	CMD_CONTACT,
	CMD_TUNPROXY,
	CMD_TUNSERVER,
	CMD_TUNCONF, 
	CMD_SIPPX, 
	CMD_PHINIT, 
	CMD_SUBSCRIBE, 
	CMD_HELP
};

struct cmd {
	const   char  *cmd;
	const   char  *help;
	enum    CMDS  code;
};


/**
* Call progress callback routine
* @param	category	the category of the received event
* @param	pInfo	the event info corresponding to that category event
* @param	pUserData	variable additional info
*/
static int phApiEventsHandler(OWPL_EVENT_CATEGORY category, void* pInfo, void* pUserData);

/**
* Call progress callback routine
* @param	info	the event info
*/
static void callProgress(OWPL_CALLSTATE_INFO * info);

/**
* Register progress callback routine
* @param	info	the event info
*/
static void regProgress(OWPL_LINESTATE_INFO *  info);

static int cmdloop(const char* userid, const char *regserver, FILE *file, int doecho);


static char default_sip_target[128] = "nowhere";
static char currentid[512];  /* current sip identity */
static int callbackStatus;
static int dtmf_mode = 3;
static int verboseFlag = 0;
static int autoring = 0;
static int autoans = 0;
static int autoreject = 0;
static int oldinit = 0;
static int novl = 0;
static int xfervlid = 0;
static int noinit = 0;
static int defvlid;
/* extern eXosip_t eXosip; */
static char  proxy_server[256], tunnel_server[256],  proxy_user[256], proxy_passwd[256];
int proxy_port, tunnel_port = 80;
static char phapi_server[32] = "127.0.0.1";
char cnfuri1[256], cnfuri2[256];
int cnfcid1, cnfcid2;
int cnfvlid;
int cnfstate;
#if defined(__DATE__) && defined(__TIME__)
static const char server_built[] = __DATE__ " " __TIME__;
#else
static const char server_built[] = "unknown";
#endif

const struct cmd cmdtab[] =
{
	{ "lc", "vlid target\t- place Call to target using vlid is virtual line id", CMD_LCALL },
/*	{ "fl", "target\t - set Follow Me", CMD_FOLLOW }, // FIXME DISABLED */
	{ "h", "callid\t- Hangup the call", CMD_HANGUP },
	{ "a", "callid\t- Accept incoming call", CMD_ACCEPT },
	{ "r", "callid\t- Reject incoming call", CMD_REJECT },
/*	{ "n", "callid\t- send riNging event for a call", CMD_RING }, // TODO REFACTOR DISABLED */
	{ "o", "callid\t- hOld the call", CMD_HOLD },
	{ "u", "callid\t- resUme the call", CMD_RESUME },
/*	{ "b", "0/1\t- set Busy flag (when 1 all incoming calls are automatically rejected)", CMD_BUSY }, // FIXME DISABLED */
/*	{ "m", "callid dtmfchars\t- send dtMf signal", CMD_DTMF },  // TODO REFACTOR DISABLED */
/*	{ "bx", "callid target\t- Blind Xfer call to target", CMD_BTXFER },  // TODO REFACTOR DISABLED */
/*	{ "ax", "callid callid2\t- Assisted Xfer call to callid2", CMD_ATXFER },  // TODO REFACTOR DISABLED */
/*	{ "xp", "callid target\t- Place call in response to XferReq", CMD_XFERCNF },  // TODO REFACTOR DISABLED */
/*	{ "op", "callid\t- send OPtions", CMD_OPT }, // FIXME DISABLED */
	{ "vla", "username server [p=proxy t=timeout]\t- Add Virtual Line", CMD_VLADD },
/*	{ "vld", "vlid\t- delete virtual line", CMD_VLDEL }, // TODO REFACTOR DISABLED */
	{ "auta","vlid username userid  pass realm\t- Add authenitcation info", CMD_AUTHADD },
	{ "auta2","vlid username userid  pass\t- Add authenitcation info with realm empty", CMD_AUTHADD_2 },
	{ "id", "userid\t- set current identity (ex: user@192.168.10.13)", CMD_SETID },
	{ "sleep", "seconds\t- sleep number of seconds", CMD_SLEEP },
	{ "dtmfmode"," 1/2/3/0\t- dtmf mode mask (1 - INBAND, 2 - OUTBABAND 3 or 0 - BOTH)", CMD_DTMFMODE },
/*	{ "sf", "cid filname\t-  send sound file", CMD_SENDSF }, // TODO REFACTOR DISABLED */
	{ "q", "- Quit",  CMD_QUIT },
	{ "ebreak", "code\t- exit if previous command returned error(code is the program exit code)",  CMD_EBREAK },
	{ "exit", "code\t- exit with the specified error code",  CMD_EXIT },
	{ "autoring", "0/1\t- change autoring setting (when set automatically generate RINGING on incoming calls)",  CMD_AUTORING },
	{ "autoans", "0/1\t- change autoanswer setting (when set automatically accepts incoming calls)",  CMD_AUTOANS },
	{ "autorej", "0/1\t- change autrejectd setting (when set automatically rejects incoming calls)",  CMD_AUTOREJ },
	{ "verbose", "0/1\t- set verbose flag... When set the commands are echoed to stdout)",  CMD_SETV },
	{ "echo", "text\t- display the given text",  CMD_ECHO },
/*	{ "natinfo", "show NAT information", CMD_NATINFO }, // TODO REFACTOR DISABLED */
/*	{ "b", "vlid 0/1 set busy flag for vlid. If vlid==0 set global bust flag", CMD_BUSY }, // TODO REFACTOR DISABLED */
/*	{ "fl", "vlid uri\t set followme address for given vlid to given uri. If vlid==0 do global setting. Mising URI means reset it", CMD_FOLLOW },  // FIXME DISABLED */
/*	{ "k", "cid1 cid2", CMD_CONF }, // TODO REFACTOR DISABLED */
/*	{ "nk", "cid1 cid2", CMD_NOCONF }, // TODO REFACTOR DISABLED */
/*	{ "lk", "vlid uri1 uri2\t-  make confcall between ur1 and uri2", CMD_LCONF }, // TODO REFACTOR DISABLED */
/*	{ "vlc", "vlid contact", CMD_CONTACT },  // TODO REFACTOR DISABLED */
	{ "tp",  "addres port\t - set http proxy server and port", CMD_TUNPROXY },
	{ "ts",  "server port\t - set tunnel server and port", CMD_TUNSERVER },
	{ "tc",  "\t - configure the tunnel", CMD_TUNCONF },
	{ "sippx", "proxy:port\t - set SIP proxy address", CMD_SIPPX },
	{ "phi", "\t - do phInit",  CMD_PHINIT },
	{ "sub", "vlid uri winfoflag\t - send a subscribe request",  CMD_SUBSCRIBE },
	{ "?", "- show this message",  CMD_HELP }
};

#define NCMDS (sizeof(cmdtab)/sizeof(cmdtab[0]))



#ifndef _WIN32
pthread_t mua_thread_create (int stacksize, void *(*func) (void *), void *arg) {
	int i;
	pthread_t thread;

	i = pthread_create (&thread, NULL, func, (void *) arg);
	if (i != 0)
	{
		perror("phapi_client error creating thread:");
		exit(2);
		return 0;
	}
	return thread;
}
#else
pthread_t mua_thread_create (int stacksize, void *(*func) (void *), void *arg) {
	pthread_t thread;
	typedef void (*thrfun)(void *);


	thread = (pthread_t) _beginthread ((thrfun)func, 0, arg);

	if (thread == 0)
	{
		fprintf(stderr, "phapi_client: error creating thread");
		exit(2);
		return 0;
	}
	return thread;
}
#endif

static int phApiEventsHandler(OWPL_EVENT_CATEGORY category, void* pInfo, void* pUserData) {
	switch(category) {
			case EVENT_CATEGORY_CALLSTATE :
				callProgress((OWPL_CALLSTATE_INFO *)pInfo);
				break;

			case EVENT_CATEGORY_LINESTATE : 
				regProgress((OWPL_LINESTATE_INFO *)pInfo);
				break;

			case EVENT_CATEGORY_MESSAGE : 
				break;

			case EVENT_CATEGORY_SUB_STATUS : 
				break;

			case EVENT_CATEGORY_NOTIFY : 
				break;

			case EVENT_CATEGORY_ERROR :
				break;

			default :
				break;
	}

	return 0;
}

void usage(int code) {
	printf("\n\
		   usage:\n\
		   \n\
		   miniua args\n\
		   \n\
		   \t [-c default sip target]\n\
		   \t [-l <log file>]\n\
		   \t [-f <from url>]\n\
		   \t [-d <verbose level>]\n\
		   \t [-s <phapi server address>[:<port>]]\n\
		   \t [-cp <phapi callback port>]\n\
		   \t [-p SIP proxy address]\n\
		   \t [-fp force proxy\n\
		   \t [-r userid,pass,realm,server]\n\
		   \t [-n nattype  (auto, none, fcone, rcone, prcone, sym)]\n\
		   \t [-stun addr:port  stun server spec (nape:port or addr:port]\n\
		   \t [-codecs codec preference list (PCMU,PCMA,GSM)\n\
		   \t [-m dtmfmode (1 - inband, 2 - rtp, 3 - both)\n\
		   \t [-nomedia actived nomedia mode (use only SIP signalling - without RTP)\n\
		   \t [-sipport SIP port number]\n\
		   \t [-autoring activate automatic RINGING answer on icomming calls\n\
		   \t [-autoredir activate automatic handling of 3xx responses\n\
		   \t [-autoans activate automatic call accept on icomming calls\n\
		   \t [-verbose when activated miniua echo all commands]\n\
		   \t [-noinit  don't call phInit]\n\
		   \t [-novl force old init semantics + avoid phAddVline call\n\
		   \t [-vad force VAD\n\
		   \t [-cng force CNG\n\
		   \t [-hdx level\n\
		   \t [-noaec\n\
		   \t [-h]\n\
		   \t [-v]\n");

	exit (code);
}

static void getreginfo(const char *buf, char *user, char *pass, char *realm, char *server)
{
	const char *tok = strstr(buf, ",");

	while(buf < tok)
		*user++ = *buf++;
	*user = 0;

	tok = strstr(++buf, ",");
	while(buf < tok)
		*pass++ = *buf++;
	*pass = 0;

	tok = strstr(++buf, ",");
	while(buf < tok)
		*realm++ = *buf++;
	*realm = 0;

	strcpy(server, buf+1);
}


static char * fixsipid(char *str, int size)
{
	if (str[0] == 0)
		return str;

	if (0 == strstr(str, "sip:"))
	{
		char *tmp = strdup(str);
		snprintf(str, size, "sip:%s", tmp);
		free(tmp);
	}
	return str;
}




int main(int argc, const char *const *argv)
{

	/* deal with options */
	char c;
	int i;
	int send_subscription = 0;
	int needreg=0;
	char userid[256], regserver[256], realm[256], pass[256];
	char proxy[256];

	userid[0] =  regserver[0] = realm[0] =  pass[0] = proxy[0] = 0;

	phcfg.autoredir = 1;

	for( i = 1; i < argc; i++) {
		const char *arg = argv[i];

		if (!strcmp(arg, "-m"))
		{
			dtmf_mode = 3 & atoi(argv[++i]);
		}
		else if (!strcmp(arg, "-c"))
		{
			strncpy(default_sip_target, argv[++i], sizeof(default_sip_target));
		}
		else if (!strcmp(arg, "-f"))
		{
			if (i == argc - 1)
				usage(0);

			strncpy(phcfg.identity, argv[++i], sizeof(phcfg.identity));
			strncpy(currentid, argv[i], sizeof(currentid));

		}
		else if (!strcmp(arg, "-d"))
		{
			if (i == argc - 1)
				usage(0);

			phDebugLevel = atoi(argv[++i]);
		}
		else if (!strcmp(arg, "-s"))
		{
			char *colon;
			if (i == argc - 1)
				usage(0);

			strncpy(phapi_server, argv[++i], sizeof(phapi_server));
			if (colon = strstr(phapi_server,":"))
			{
				phServerPort = atoi(arg+1);
				*colon = 0;
			}
		}
		else if (!strcmp(arg, "-l"))
		{
			if (i == argc - 1)
				usage(0);

			phLogFileName = (char*) argv[++i];
		}
		else if (!strcmp(arg, "-p"))
		{
			if (i == argc - 1) {
				usage(0);
			}

			strncpy(phcfg.proxy, argv[++i], sizeof(phcfg.proxy));
			strncpy(proxy, argv[i], sizeof(proxy));
		}
		else if (!strcmp(arg, "-sipport"))
		{
			if (i == argc - 1)
				usage(0);


			strncpy(phcfg.sipport, argv[++i], sizeof(phcfg.sipport));
		}
		else if (!strcmp(arg, "-fp"))
			phcfg.force_proxy = 1;
		else if (!strcmp(arg, "-r"))
		{
			getreginfo(argv[++i], userid, pass, realm, regserver);
			snprintf(phcfg.identity, sizeof(phcfg.identity), "sip:%s@%s", userid,regserver);
			strncpy(currentid, phcfg.identity, sizeof(currentid));
			needreg = 1;
		}

		else if (!strcmp(arg, "-v") || !strcmp(arg, "-V"))
		{
#ifdef VERSION
			printf ("miniua: version:     %s\n", VERSION);
#endif
			printf ("build: %s\n", server_built);
			fflush(stdout);
		}
		else if (!strcmp(arg, "-h") || !strcmp(arg, "-?") || !strcmp(arg, "?"))
		{
#ifdef VERSION
			printf ("miniua: version:     %s\n", VERSION);
#endif
			printf ("build: %s\n", server_built);
			usage (0);
			fflush(stdout);
		}
		else if (!strcmp(arg, "-n"))
		{
			if (i == argc - 1)
				usage(0);

			strncpy(phcfg.nattype, argv[++i], sizeof(phcfg.nattype));
		}
		else if (!strcmp(arg, "-codecs"))
		{
			if (i == argc - 1)
				usage(0);

			strncpy(phcfg.audio_codecs, argv[++i], sizeof(phcfg.audio_codecs));
		}
		else if (!strcmp(arg, "-nomedia"))
			phcfg.nomedia = 1;
		else if (!strcmp(arg, "-autoredir"))
			phcfg.autoredir = 1;
		else if (!strcmp(arg, "-stun"))
		{
			strncpy(phcfg.stunserver, argv[++i], sizeof(phcfg.stunserver));
		}
		else if (!strcmp(arg, "-verbose"))
		{
			verboseFlag = 1;
		}
		else if (!strcmp(arg, "-autoring"))
		{
			autoring = 1;
		}
		else if (!strcmp(arg, "-autoans"))
		{
			autoans = 1;
		}
		else if (!strcmp(arg, "-noinit"))
		{
			noinit = 1;
		}
		else if (!strcmp(arg, "-novl"))
		{
			novl = 1;
		}
		else if (!strcmp(arg, "-vad"))
		{
#ifdef EMBED
			phcfg.vad = VAD_VALID_MASK | (500 & VAD_THRESHOLD_MASK);
#else
			phcfg.vad = VAD_VALID_MASK | (1000 & VAD_THRESHOLD_MASK);
#endif
		}
		else if (!strcmp(arg, "-cng"))
		{
			phcfg.cng = 1;
		}
		else if (!strcmp(arg, "-noaec"))
		{
			phcfg.noaec = 1;
		}
		else if (!strcmp(arg, "-hdx"))
		{
			phcfg.hdxmode = 1;
			phcfg.vad = atoi(argv[++i]);
		}
	}

	fixsipid(currentid, sizeof(currentid));

	phcfg.identity[0] = 0;
	owplConfigSetOutboundProxy(proxy);
	phcfg.proxy[0] = 0;

	if (phDebugLevel > 0)
	{
#ifdef VERSION
		printf ("miniua: %s\n", VERSION);
#endif
		printf ("Debug level:        %i\n", phDebugLevel);
		if (phLogFileName == NULL)
			printf ("Log name:           Standard output\n");
		else
			printf ("Log name:           %s\n", phLogFileName);

		fflush(stdout);
	}


#ifdef SIGTRAP
	signal(SIGTRAP, SIG_IGN);
#endif

	if(!noinit) {
		owplConfigSetAsyncCallbackMode (1);
		owplEventListenerAdd(phApiEventsHandler, 0);
		if(owplInit(5060, 0, 0, NULL, 0) != OWPL_RESULT_SUCCESS)  {
			fprintf (stderr, "miniua: could not initialize phoneapi\n");
			exit(0);
		}
	}

	/*
	* we've got -r parameters so we need to prepare for autentication and
	* register ourselves with the registration server
	*/
	if(!noinit && needreg) {
		owplLineAdd(userid, userid, regserver, proxy, 3600, &defvlid);
		owplLineAddCredential(defvlid, userid, pass, realm);
		owplLineRegister(defvlid, 1);

	} else if(currentid[0] != 0) {
		char *p;
		char server[256];

		strcpy(userid, currentid+4);
		p = strchr(userid, '@');
		if (p) {
			*p = 0;
			strncpy(server, p+1, sizeof(server));
		}
		if (!noinit && !novl) {
			owplLineAdd(userid, userid, regserver, proxy, 3600, &defvlid);
			owplLineRegister(defvlid, 1);
		}
	}

	printf("Welcome To Miniua\n");
	fflush(stdout);

	cmdloop(userid, regserver, stdin, 0);
	owplShutdown();
	exit(0);
	return(0);
}







static void do_holdit(void *arg) {
	sleep(1);
	printf("Holding this call : %d\n", cnfcid1);
	cnfstate = WAITHOLD1;
	phHoldCall(cnfcid1);
}


static void do_confit(void *arg) {
	sleep(1);
	printf("Creating the conf\n");
	phConf(cnfcid2, cnfcid1);
	cnfstate = HALFCONF;
	printf("Resuming %d\n", cnfcid1);
	phResumeCall(cnfcid1);
}

static void callProgress(OWPL_CALLSTATE_INFO * info) {
	if(info != NULL) {
		switch(info->event) {
			case CALLSTATE_UNKNOWN :			
				break;

			case CALLSTATE_NEWCALL :
				switch(info->cause) {
					case CALLSTATE_NEW_CALL_NORMAL :
						break;

					case CALLSTATE_NEW_CALL_TRANSFERRED :
						break;

					case CALLSTATE_NEW_CALL_TRANSFER :
						break;

					default :
						break;
				}
				break;

			case CALLSTATE_REMOTE_OFFERING :
				switch(info->cause) {
					case CALLSTATE_REMOTE_OFFERING_NORMAL :
						printf("[CALL PROGRESS] Line %d Call %d To %s : Remote Offering Normal\n", info->hLine, info->hCall, info->szRemoteIdentity);
						break;

					default :
						break;
				}
				break;

			case CALLSTATE_REMOTE_ALERTING :
				switch(info->cause) {
					case CALLSTATE_REMOTE_ALERTING_NORMAL :
						printf("[CALL PROGRESS] Line %d Call %d To %s : Remote alerting Normal\n", info->hLine, info->hCall, info->szRemoteIdentity);
						break;

					case CALLSTATE_REMOTE_ALERTING_MEDIA :
						break;

					default :
						break;
				}
				break;

			case CALLSTATE_CONNECTED :
				switch(info->cause) {
					case CALLSTATE_CONNECTED_ACTIVE :
						printf("[CALL PROGRESS] Line %d Call %d To %s : Connected Active\n", info->hLine, info->hCall, info->szRemoteIdentity);
						break;

					case CALLSTATE_CONNECTED_ACTIVE_HELD :
						break;

					case CALLSTATE_CONNECTED_INACTIVE :
						break;

					default :
						break;
				}
				break;

			case CALLSTATE_DISCONNECTED :
				switch(info->cause) {
					case CALLSTATE_DISCONNECTED_BADADDRESS :
						break;

					case CALLSTATE_DISCONNECTED_BUSY :
						printf("[CALL PROGRESS] Line %d Call %d To %s : Disconnected Busy\n", info->hLine, info->hCall, info->szRemoteIdentity);
						break;

					case CALLSTATE_DISCONNECTED_NORMAL :
						printf("[CALL PROGRESS] Line %d Call %d To %s : Disconnected Normal\n", info->hLine, info->hCall, info->szRemoteIdentity);
						break;

					case CALLSTATE_DISCONNECTED_RESOURCES :
						break;

					case CALLSTATE_DISCONNECTED_NETWORK :
						printf("[CALL PROGRESS] Line %d Call %d To %s : Disconnected Network\n", info->hLine, info->hCall, info->szRemoteIdentity);
						break;

					case CALLSTATE_DISCONNECTED_REDIRECTED :
						break;

					case CALLSTATE_DISCONNECTED_NO_RESPONSE :
						printf("[CALL PROGRESS] Line %d Call %d To %s : Disconnected No Response\n", info->hLine, info->hCall, info->szRemoteIdentity);
						break;

					case CALLSTATE_DISCONNECTED_AUTH :
						break;

					case CALLSTATE_DISCONNECTED_UNKNOWN :
						printf("[CALL PROGRESS] Line %d Call %d To %s : Disconnected Unknown\n", info->hLine, info->hCall, info->szRemoteIdentity);
						break;

					default :
						break;
				}
				break;

			case CALLSTATE_OFFERING :
				switch(info->cause) {
					case CALLSTATE_OFFERING_ACTIVE :
						printf("[CALL PROGRESS] Line %d Call %d To %s : Offering Active\n", info->hLine, info->hCall, info->szRemoteIdentity);
						break;

					default :
						break;
				}
				break;
				
			case CALLSTATE_ALERTING :
				switch(info->cause) {
					case CALLSTATE_ALERTING_NORMAL :
						break;

					default :
						break;
				}
				break;
				
			case CALLSTATE_DESTROYED :
				switch(info->cause) {
					case CALLSTATE_DESTROYED_NORMAL :
						break;

					default :
						break;
				}
				break;
				
			case CALLSTATE_AUDIO_EVENT :
				switch(info->cause) {
					case CALLSTATE_AUDIO_START :
						break;

					case CALLSTATE_AUDIO_STOP :
						break;

					case CALLSTATE_AUDIO_DTMF :
						break;

					default :
						break;
				}
				break;
				
			case CALLSTATE_VIDEO_EVENT :
				switch(info->cause) {
					case CALLSTATE_VIDEO_START :
						break;

					case CALLSTATE_VIDEO_STOP :
						break;

					case CALLSTATE_VIDEO_FRAME_RCV :
						printf("[CALL PROGRESS] Line %d Call %d To %s : Video Frame Received\n", info->hLine, info->hCall, info->szRemoteIdentity);
						break;

					default :
						break;
				}
				break;
				
			case CALLSTATE_TRANSFER :
				switch(info->cause) {
					case CALLSTATE_TRANSFER_INITIATED :					
						break;

					case CALLSTATE_TRANSFER_ACCEPTED :
						break;

					case CALLSTATE_TRANSFER_TRYING :
						break;

					case CALLSTATE_TRANSFER_RINGING :
						break;

					case CALLSTATE_TRANSFER_SUCCESS :
						break;

					case CALLSTATE_TRANSFER_FAILURE :
						break;

					default :
						break;
				}
				break;
				
			case CALLSTATE_REDIRECTED :
				switch(info->cause) {
					case CALLSTATE_REDIRECTED_NORMAL :
						printf("[CALL PROGRESS] Line %d Call %d To %s : Redirected Normal\n", info->hLine, info->hCall, info->szRemoteIdentity);
						break;

					default :
						break;
				}
				break;
				
			case CALLSTATE_HOLD :
				switch(info->cause) {
					case CALLSTATE_HOLD_STARTED :
						printf("[CALL PROGRESS] Line %d Call %d To %s : Hold started\n", info->hLine, info->hCall, info->szRemoteIdentity);
						break;

					case CALLSTATE_HOLD_RESUMED :
						printf("[CALL PROGRESS] Line %d Call %d To %s : Hold Resumed\n", info->hLine, info->hCall, info->szRemoteIdentity);
						break;

					default :
						break;
				}
				break;
				
			case CALLSTATE_SECURITY_EVENT :
				switch(info->cause) {
					case CALLSTATE_SECURITY_SELF_SIGNED_CERT :
						break;

					case CALLSTATE_SECURITY_SESSION_NOT_SECURED :
						break;

					case CALLSTATE_SECURITY_REMOTE_SMIME_UNSUPPORTED :
						break;

					default :
						break;
				}
				break;
				
			case CALLSTATE_IDENTITY_CHANGE :
				switch(info->cause) {
					case CALLSTATE_IDENTITY_CHANGE_UNKNOWN :
						break;

					default :
						break;
				}
				break;

			default :
				break;
		}

		fflush(stdout);
	}
}

static void regProgress(OWPL_LINESTATE_INFO *  info) {
	if(info != NULL) {
		switch(info->event) {
			case LINESTATE_UNKNOWN :
				printf("[REGISTER PROGRESS] Line %d : State Unknown\n", info->hLine);
				break;

			case LINESTATE_REGISTERING :
				switch(info->cause) {
			case LINESTATE_REGISTERING_NORMAL :
				break;

			default :
				break;
				}
				break;

			case LINESTATE_REGISTERED :
				switch(info->cause) {
			case LINESTATE_REGISTERED_NORMAL :
				printf("[REGISTER PROGRESS] Line %d : Registered Normal\n", info->hLine);
				break;

			default :
				break;
				}
				break;

			case LINESTATE_UNREGISTERING :
				switch(info->cause) {
			case LINESTATE_UNREGISTERING_NORMAL :
				break;

			default :
				break;
				}
				break;

			case LINESTATE_UNREGISTERED :
				switch(info->cause) {
			case LINESTATE_UNREGISTERED_NORMAL :
				printf("[REGISTER PROGRESS] Line %d : Unregistered normal\n", info->hLine);
				break;

			default :
				break;
				}
				break;

			case LINESTATE_REGISTER_FAILED :
				switch(info->cause) {
			case LINESTATE_REGISTER_FAILED_COULD_NOT_CONNECT :
				printf("[REGISTER PROGRESS] Line %d : Could Not connect\n", info->hLine);
				break;

			case LINESTATE_REGISTER_FAILED_NOT_AUTHORIZED :
				printf("[REGISTER PROGRESS] Line %d : Not Authorized\n", info->hLine);
				break;

			case LINESTATE_REGISTER_FAILED_TIMEOUT :
				printf("[REGISTER PROGRESS] Line %d : Timeout\n", info->hLine);
				break;

			case LINESTATE_REGISTER_FAILED_NOT_FOUND :
				printf("[REGISTER PROGRESS] Line %d : Not Found\n", info->hLine);
				break;

			default :
				printf("[REGISTER PROGRESS] Line %d : State Unknown\n", info->hLine);
				break;
				}
				break;

			case LINESTATE_UNREGISTER_FAILED :
				switch(info->cause) {
			case LINESTATE_UNREGISTER_FAILED_COULD_NOT_CONNECT :
				break;

			case LINESTATE_UNREGISTER_FAILED_NOT_AUTHORIZED :
				break;

			case LINESTATE_UNREGISTER_FAILED_TIMEOUT :
				break;

			default :
				break;
				}
				break;

			case LINESTATE_PROVISIONED :
				switch(info->cause) {
			case LINESTATE_PROVISIONED_NORMAL :
				break;

			default :
				break;
				}
				break;

			default :
				printf("[REGISTER PROGRESS] Line %d : State Unknown\n", info->hLine);
				break;
		}

		fflush(stdout);
	}
}

/**
* construct a valid uri from given input
* @param buf   output buffer
* @param uri   input
* @param size  size of the output buffer
*/
static int geturi(char *buf, char* uri, int size) {
	while(buf && isspace(*buf))
		buf++;

	uri[0] = 0;

	if(!buf) {
		return 0;
	}

	if(strncasecmp(buf, "sip:", 4)) {
		strncat(uri, "sip:", size);
		uri += 4; size -= 4;
	}

	if(strchr(buf, '@')) {
		snprintf(uri, size, buf);
	} else {
		snprintf(uri, size, "%s@%s", buf, default_sip_target);
	}

	return 0;
}

static int parseCmd(char *buf,  char **args) {
	char *space;
	int  i, len, blen;

	if (buf[0] == '#')
		return CMD_COMMENT;

	*args = 0;
	space = strchr(buf, ' ');
	if (space)
	{
		*space++ = 0;
		while(isspace(*space))
			space++;

		*args = space;
	}

	blen = strlen(buf);
	for(i = 0; i < NCMDS; i++)
	{
		len = strlen(cmdtab[i].cmd);
		if ((len == blen) &&  !strncasecmp(buf, cmdtab[i].cmd, len))
			return cmdtab[i].code;
	}

	return CMD_HELP;
}


static void showhelp() {
	int i;

	for(i = 0; i < NCMDS; i++)
		printf("%s\t%s\n", cmdtab[i].cmd, cmdtab[i].help);

	fflush(stdout);
}


static int doinclude(const char *userid, const char *server, char *buf) {
	FILE *file;
	int doexit;

	while(*buf && isspace(*buf))
		buf++;

	file = fopen(buf, "r");
	if (file == NULL)
	{
		perror(buf);
		return 0;
	}

	printf("Including: %s\n", buf);
	fflush(stdout);

	doexit = cmdloop(userid, server, file, 1);

	fclose(file);

	printf("Done: %s\n", buf);
	fflush(stdout);

	return doexit;
}


static void dobreak(int x) {
	exit(x);
}

int mkconf(int vlid, const char *uri1, const char *uri2) {
	cnfvlid = vlid;

	geturi(uri1, cnfuri1, sizeof(cnfuri1));
	geturi(uri2, cnfuri2, sizeof(cnfuri2));

	printf("Making conf between <%s> and <%s>\n", uri1, uri2);

	cnfcid1 = phLinePlaceCall(vlid, cnfuri1, 0, 0);

	if (cnfcid1 > 0) {
		cnfstate = WAITCALL1;
		return 0;
	}
	return cnfcid1;
}

static int cmdloop(const char* userid, const char *regserver, FILE *file, int doecho) {
	char buf[256];
	char uri[256];
	int cid;
	char *args;
	int vlid;
	int prevret;

	while(!feof(file)) {
		char *p;
		char tmp[16];
		int ret = -1;
		int  dtmf, skipresult = 0;
		char *rd;

		fflush(stdout);
		rd = fgets(buf, sizeof(buf), file);

		if(!rd) {
			return 0;
		}

		if(doecho || verboseFlag) {
			printf(buf);
			fflush(stdout);
		}

		p = strstr(buf, "\n");
		if(p) {
			*p = 0;
		}

		if (buf[0] == '!') {
			int doexit = doinclude(userid, regserver, buf+1);
			if (!doexit) {
				skipresult = 1;
				continue;
			}
			return 1;
		}

		switch(parseCmd(buf, &args)) {
		case CMD_BTXFER:
			/* TODO REFACTOR DISABLED
			p = args;
			cid = atoi(args);

			while(*p && isdigit(*p)) {
				p++;
			}

			while(*p && isspace(*p)) {
				p++;
			}

			if (!geturi(p, uri, 256)) {
				ret = phBlindTransferCall(cid, uri);
			}
			// TODO REFACTOR implement appropriate function
			*/
			break;

		case CMD_ATXFER:
			{
				/* TODO REFACTOR DISABLED
				int cid2;

				p = args;
				cid = atoi(args);


				while(*p && isdigit(*p)) {
					p++;
				}

				while(*p && isspace(*p)) {
					p++;
				}

				cid2 = atoi(p);
				ret = phTransferCall(cid, cid2);
				// TODO REFACTOR implement appropriate function
				*/
				break;
			}

#if 1
		case CMD_CONF:
			{
				/* TODO REFACTOR DISABLED
				int cid2;

				p = args;
				cid = atoi(args);


				while(*p && isdigit(*p)) {
					p++;
				}

				while(*p && isspace(*p)) {
					p++;
				}

				cid2 = atoi(p);
				ret = phConf(cid, cid2);
				// TODO REFACTOR implement appropriate function
				*/
				break;
			}

		case CMD_NOCONF:
			{
				/* TODO REFACTOR DISABLED
				int cid2;

				p = args;
				cid = atoi(args);


				while(*p && isdigit(*p)) {
					p++;
				}

				while(*p && isspace(*p)) {
					p++;
				}

				cid2 = atoi(p);
				ret = phStopConf(cid, cid2);
				// TODO REFACTOR implement appropriate function
				*/
				break;
			}

		case CMD_LCONF:
			{
				/* TODO REFACTOR DISABLED
				char *uri1, *uri2;

				p = args;

				p = strtok(args, " ");
				vlid = atoi(p);

				uri1 = strtok(NULL, " ");
				uri2 = strtok(NULL, " ");

				ret = mkconf(vlid, uri1, uri2);
				// TODO REFACTOR
				*/
				break;
			}
#endif

		case CMD_CONTACT:
			/* TODO REFACTOR DISABLED
			vlid = atoi(args);
			p = args;

			while(*p && isdigit(*p)) {
				p++;
			}

			while(*p && isspace(*p)) {
				p++;
			}

			// TODO REFACTOR
			ret = phSetContact(vlid, p);
			*/
			break;

		case CMD_XFERCNF:
			/* TODO REFACTOR DISABLED
			p = args;
			cid = atoi(args);

			while(*p && isdigit(*p)) {
				p++;
			}

			while(*p && isspace(*p)) {
				p++;
			}

			if (!geturi(p, uri, 256)) {
				ret = owplCallCreate(xfervlid, &cid);
				if(ret == OWPL_RESULT_SUCCESS) {
					ret = owplCallConnect(cid, uri, OWPL_STREAM_AUDIO); // FIXME REFACTOR what stream?
				}
				printf("Call %d to %s\n", cid, uri);
				fflush(stdout);
				skipresult = 1;
			}
			*/
			break;

		case CMD_HANGUP:
			cid = atoi(args);
			ret = owplCallDisconnect(cid);
			break;

		case CMD_ACCEPT:
		case 'A':
			cid = atoi(args);
			ret = owplCallAnswer(cid, OWPL_STREAM_AUDIO); // FIXME REFACTOR what stream?
			break;

		case CMD_REJECT:
			cid = atoi(args);
			ret = owplCallReject(cid, 488, NULL);
			break;

		case CMD_RING:
			/* TODO REFACTOR DISABLED
			cid = atoi(args);
			// TODO REFACTOR
			ret = phRingingCall(cid);
			*/
			break;

		case CMD_HOLD:
			cid = atoi(args);
			ret = owplCallHold(cid);
			break;

		case CMD_RESUME:
			cid = atoi(args);
			ret = owplCallUnhold(cid);
			break;

		case CMD_QUIT:
			return 1;

		case CMD_BUSY:
			{
				char *tok;

				vlid = atoi(args);
				tok = strstr(args, " ");

				if (!vlid) {
					// TODO REFACTOR
					ret = phSetBusy(atoi(tok));
				} else {
					tok = strstr(NULL, " ");
					ret = owplLineSetBusy(vlid, atoi(tok));
				}
				break;
			}

		case CMD_DTMF:
			
			{
				/* TODO REFACTOR DISABLED
				sscanf(args, "%d %s", &cid, tmp);
				p = tmp;
				ret = 0;
				while(!ret && *p)
					// TODO REFACTOR
					ret = phSendDtmf(cid, *p++, dtmf_mode);
				*/
				break;
			}

		case CMD_OPT:
			/* FIXME DISABLED because phSendOptions can't be accessed
			// TODO REFACTOR
			ret = phSendOptions(currentid, "sip:nobody@nobody.com");
			*/
			break;

		case CMD_SETID:
			p = args;
			while(*p && isspace(*p))
				p++;
			strncpy(currentid, p, sizeof(currentid));
			fixsipid(currentid, sizeof(currentid));
			skipresult = 1;
			break;

		case CMD_AUTHADD:
			{
				char *uname, *uid, *pass, *realm;

				//uname = strtok(NULL, " ");
				uid = strtok(args, " ");
				pass = strtok(NULL, " ");
				realm = strtok(NULL, " ");

				ret = owplLineAddCredential(0, uid, pass, realm);
				break;
			}

		case CMD_AUTHADD_2:
			{
				char *hLine, *uname, *uid, *pass;

				hLine = strtok(args, " ");
				uname = strtok(NULL, " ");
				uid = strtok(NULL, " ");
				pass = strtok(NULL, " ");

				ret = owplLineAddCredential(atoi(hLine), uid, pass, "");
				break;
			}

		case CMD_LCALL:
			{
				char *tok;

				vlid = atoi(args);

				tok = strtok(args, " ");
				tok = strtok(NULL, " ");

				if(!geturi(tok, uri, 256)) {
					ret = owplCallCreate(vlid, &cid);
					if(ret == OWPL_RESULT_SUCCESS) {
						ret = owplCallConnect(cid, tok, OWPL_STREAM_AUDIO); // FIXME REFACTOR what stream?
					}
					printf("Call %d to %s ...\n", cid, uri);
					fflush(stdout);
				}

				break;
			}

		case CMD_SUBSCRIBE:
			{
				char *tok;
				int sid;

				vlid = atoi(args);

				tok = strtok(args, " ");
				tok = strtok(NULL, " ");

				if(!geturi(tok, uri, 256)) {
					int winfo;
					tok = strtok(NULL, " ");
					winfo = tok ? atoi(tok) : 0;

					ret = owplPresenceSubscribe(vlid, uri, winfo, &sid);
					printf("Subscribe %d to %s\n", sid, uri);
					fflush(stdout);
					skipresult = 1;
				}

				break;
			}

		case CMD_SENDSF:
			{
				/* TODO REFACTOR DISABLED
				char *tok;

				cid = atoi(args);

				tok = strtok(args, " ");
				tok = strtok(NULL, " ");

				// TODO REFACTOR
				ret = phSendSoundFile(cid, tok);
				*/
				break;
			}

		case CMD_FOLLOW:
			{
				/* FIXME DISABLED because phSetFollowMe can't be accessed
				char *tok;
				vlid = atoi(args);


				tok = strtok(args, " ");
				tok = strtok(NULL, " ");

				geturi(tok , uri, 256);

				if (!vlid)
				{
					if (*uri != '\0')
						printf("Global Call Forward to uri=%s\n", uri);
					else
						printf("Removing Global Call Forward\n");
					// TODO REFACTOR
					ret = phSetFollowMe(uri);
					break;
				}

				if (*uri != '\0')
					printf("Local Call Forward on VL %d to uri=%s\n", vlid, uri);
				else
					printf("Removing Local Call Forward on VL %d\n", vlid);
				// TODO REFACTOR
				ret = phLineSetFollowMe(vlid, uri);
				*/
				break;
			}

		case CMD_VLDEL:
			vlid = atoi(args);
			ret = owplLineDelete(vlid, 0);
			break;

		case CMD_VLADD:
			{
				int timeout = 0;
				char *username, *server = 0, *proxy = 0;

				p = args;

				while(*p && isspace(*p)) {
					p++;
				}

				username = p;

				/* find timeout parameter */
				p = strstr(args , "t=");
				if(!p) {
					p = strstr(args , "T=");
				}
				if(p) {
					timeout = atoi(p+2);
				}

				/* find proxy parameter */
				p = strstr(args, "p=");
				if(!p) {
					p = strstr(args , "P=");
				}

				if(p) {
					proxy = p + 2;
					p += 2;
					while(*p && !isspace(*p)) {
						p++;
					}
					*p = 0;
				}

				/* insert NUL after the server parameter */
				p = strchr(username, ' ');
				if(p) {
					*p++ = 0;
					while(*p && isspace(*p)) {
						p++;
					}

					server = p;
					while(*p && !isspace(*p)) {
						p++;
					}
					*p = 0;
				}

				/* insert NUL after the username parameter */
				p = username;
				while(*p && !isspace(*p)) {
					p++;
				}

				*p = 0;

				ret = owplLineAdd("unknown", username, server, proxy, timeout, &vlid);
				if(ret == OWPL_RESULT_SUCCESS) {
					ret = owplLineRegister(vlid, 1);
				}
				break;
			}

		case CMD_DTMFMODE:
			{
				static const char *modestr[] = { "INBAND", "OUTBAND", "IN+OUT BAND" };
				dtmf_mode = atoi(args);
				dtmf_mode &= 3;
				if(!dtmf_mode) {
					dtmf_mode = 3;
				}
				printf("DTMF MODE is %s\n", modestr[dtmf_mode-1]);
				fflush(stdout);
				skipresult = 1;
			}
			break;

		case CMD_SLEEP:
			{
				int secs;

				secs = atoi(args);

				if(!secs) {
					secs = 1;
				}

				sleep(secs);
				skipresult = 1;
				ret = prevret;

				break;
			}

		case CMD_EBREAK:
			if(prevret < 0 || callbackStatus != 0) {
				dobreak(atoi(args));
			}
			skipresult = 1;
			ret = prevret;
			break;

		case CMD_EXIT:
			exit(atoi(args));
			break;

		case CMD_SETV:
			verboseFlag = atoi(args) & 1;
			printf(verboseFlag ? "Verbose ON\n" : "Verbose OFF\n");   fflush(stdout);
			skipresult = 1;
			break;

		case CMD_AUTORING:
			autoring = atoi(args) & 1;
			printf(autoring ? "AUTORING ON\n" : "AUTORING OFF\n");   fflush(stdout);
			skipresult = 1;
			break;

		case CMD_AUTOANS:
			autoans = atoi(args) & 1;
			printf(autoans ? "AUTOANSWER ON\n" : "AUTOANSWER OFF\n");   fflush(stdout);
			skipresult = 1;
			break;

		case CMD_AUTOREJ:
			autoreject = atoi(args) & 1;
			printf(autoreject ? "AUTOREJECT ON\n" : "AUTOREJECT OFF\n");   fflush(stdout);
			skipresult = 1;
			break;

		case CMD_NATINFO:
			{
				/* TODO REFACTOR DISABLED
				char  nttp[16], fwip[64];

				// TODO REFACTOR
				ret = phGetNatInfo(nttp, sizeof(nttp), fwip, sizeof(fwip));

				printf("NAT TYPE=%s, FW=%s\n",  nttp, fwip);   fflush(stdout);
				*/
				break;
			}

		case CMD_COMMENT:
			ret = prevret;
			skipresult = 1;
			break;

		case CMD_ECHO:
			ret = prevret;
			skipresult = 1;
			printf("%s\n", args);   fflush(stdout);

		case CMD_TUNPROXY:
			{
				char *s, *p;

				s = strtok(args, " ");
				p = strtok(NULL, " ");

				strcpy(proxy_server, s);
				if(p) {
					proxy_port = atoi(p);
				}

				ret = prevret;
				skipresult = 1;
				break;
			}

		case CMD_TUNSERVER:
			{
				char *s, *p;

				s = strtok(args, " ");
				p = strtok(NULL, " ");

				strcpy(tunnel_server, s);
				if(p) {
					tunnel_port = atoi(p);
				}

				ret = prevret;
				skipresult = 1;
				break;
			}

		case CMD_TUNCONF:
			ret = owplConfigSetLocalHttpProxy(proxy_server, proxy_port, proxy_user, proxy_passwd);
			if(ret == OWPL_RESULT_SUCCESS) {
				ret = owplConfigSetTunnel(tunnel_server, tunnel_port, OWPL_TUNNEL_USE);
			}
			break;

		case CMD_SIPPX:
			{
				char *p = args;

				while(*p && isspace(*p)) {
					p++;
				}

				ret = owplConfigSetOutboundProxy(p);
				break;
			}

		case CMD_PHINIT:

			ret = owplEventListenerAdd(phApiEventsHandler, 0);
			if(ret == OWPL_RESULT_SUCCESS) {
				ret = owplConfigSetAsyncCallbackMode (1);
			}
			if(ret == OWPL_RESULT_SUCCESS) {
				ret = owplInit(5060, 0, 0, NULL, 0);
			}
			break;

		default:
			skipresult = 1;
			showhelp();
			break;

		}

		if (!skipresult) {
			printf("result = %d\n", ret);
			fflush(stdout);
		}
		prevret = ret;
	}

	return 0;
}



