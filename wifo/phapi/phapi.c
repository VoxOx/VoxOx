#include "phglobal.h"
#include <eXosip/eXosip.h>
#include "phapi.h"
#include "phcall.h"
#include "phmedia.h"
#include "phvline.h"
#include "owpl_log.h"
#include "owpl_plugin.h"
#include "owpl_adapter_internal.h"
#include "svoip_phapi.h"

#if !defined(OS_WINDOWS)
//#include "config.h"
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#ifndef OS_MACOSX
#include <sys/soundcard.h>
#endif
#include <sys/ioctl.h>
#include <fcntl.h>
#else  /* WIN32 */
#include <stdlib.h>
#define snprintf _snprintf
#define strncasecmp strnicmp
#define strcasecmp stricmp
#define usleep(usecs) Sleep((usecs)/1000)
#endif

#define stringize2(x) #x//VOXOX - CJC - 2009.06.27 
#define stringize(x) stringize2(x)//VOXOX - CJC - 2009.06.27 

int owplHttpTunnelIsInitialized = 0;
static int owslIsInitialized = 0;

/*
 * Declaration of some variables that were defined inside phapi-old.c
 */
// [
extern int phWaitTimeout;
extern pthread_mutex_t ph_media_stop_mutex;

void * ph_api_thread(void *arg);
void ph_calls_init();
void ph_payloads_init();
void ph_avcodec_init();
// ]


static OWPL_RESULT
owplInitOwsl
(
	unsigned short use_udp,
	unsigned short use_tcp,
	unsigned short use_tls
)
{
	int return_code ;
	int owslWasJustInitialized = 0 ;

	if (! owslIsInitialized)
	{
		return_code = owsl_initialize () ;
		if (return_code != 0)
		{
			goto error ;
		}
		owslIsInitialized = 1 ;
		owslWasJustInitialized = 1 ;
	}

	if (use_udp)
	{
		if
		(
			owsl_socket_type_get
			(
				OWSL_AF_IPV4,
				OWSL_MODE_DATAGRAM,
				OWSL_CIPHERING_DISABLED
			) == OWSL_TYPE_UNKNOWN
			&&
			owsl_socket_type_add (OWSL_TYPE_IPV4_UDP) != 0
		)
		{
			goto error ;
		}
		if
		(
			owsl_socket_type_get
			(
				OWSL_AF_IPV6,
				OWSL_MODE_DATAGRAM,
				OWSL_CIPHERING_DISABLED
			) == OWSL_TYPE_UNKNOWN
			&&
			owsl_socket_type_add (OWSL_TYPE_IPV6_UDP) != 0
		)
		{
			goto error ;
		}
	}
	if (use_tcp)
	{
		if
		(
			owsl_socket_type_get
			(
				OWSL_AF_IPV4,
				OWSL_MODE_STREAM,
				OWSL_CIPHERING_DISABLED
			) == OWSL_TYPE_UNKNOWN
			&&
			owsl_socket_type_add (OWSL_TYPE_IPV4_TCP) != 0
		)
		{
			goto error ;
		}
		if
		(
			owsl_socket_type_get
			(
				OWSL_AF_IPV6,
				OWSL_MODE_STREAM,
				OWSL_CIPHERING_DISABLED
			) == OWSL_TYPE_UNKNOWN
			&&
			owsl_socket_type_add (OWSL_TYPE_IPV6_TCP) != 0
		)
		{
			goto error ;
		}
	}
	if (use_tls)
	{
		if
		(
			owsl_socket_type_get
			(
				OWSL_AF_IPV4,
				OWSL_MODE_STREAM,
				OWSL_CIPHERING_ENABLED
			) == OWSL_TYPE_UNKNOWN
			&&
			owsl_socket_type_add (OWSL_TYPE_IPV4_TLS) != 0
		)
		{
			goto error ;
		}
		if
		(
			owsl_socket_type_get
			(
				OWSL_AF_IPV6,
				OWSL_MODE_STREAM,
				OWSL_CIPHERING_ENABLED
			) == OWSL_TYPE_UNKNOWN
			&&
			owsl_socket_type_add (OWSL_TYPE_IPV6_TLS) != 0
		)
		{
			goto error ;
		}
	}

	return OWPL_RESULT_SUCCESS ;

error :
	if (owslWasJustInitialized)
	{
		owsl_terminate () ;
	}
	return OWPL_RESULT_FAILURE ;
}

static void
owplOsipLogFunction(char* file, int line, osip_trace_level_t osipLevel, char* format, va_list ap)
{
	char buffer[1024]; /* osip logs can be quite verbose, avoid truncating them too much */
	char* endOfBuffer;
	OWPL_LOG_LEVEL owplLevel;

	/* Convert osipLevel to owplLevel */
	switch(osipLevel)
	{
	case END_TRACE_LEVEL:
		owplLogMessage(OWPL_LOG_LEVEL_ERROR, "osip log level should not be END_TRACE_LEVEL");
		/* Fall through */
	case OSIP_FATAL:
	case OSIP_BUG:
	case OSIP_ERROR:
		owplLevel = OWPL_LOG_LEVEL_ERROR;
		break;

	case OSIP_WARNING:
		owplLevel = OWPL_LOG_LEVEL_WARN;
		break;
	
	/* OSIP_INFO[1234] is quite lowlevel stuff, keep it at debug level for us */
	case OSIP_INFO1:
	case OSIP_INFO2:
	case OSIP_INFO3:
	case OSIP_INFO4:
		owplLevel = OWPL_LOG_LEVEL_DEBUG;
		break;
	}

	/* Format output */
	snprintf(buffer, sizeof(buffer), "osip: %s:%d: %s", file, line, format);

	/* Strip ending carriage returns */
	endOfBuffer = buffer + strlen(buffer) - 1;
	for (; endOfBuffer != buffer; endOfBuffer--)
	{
		if (*endOfBuffer == '\n' || *endOfBuffer == '\r')
		{
			*endOfBuffer = '\0';
		}
		else
		{
			break;
		}
	};

	if (endOfBuffer != buffer)
	{
		owplLogMessageAP(owplLevel, buffer, ap);
	}
	else
	{
		snprintf(buffer, sizeof(buffer), "osip: %s:%d: Empty log message!", file, line);
		owplLogMessage(owplLevel, buffer);
	}
}

OWPL_RESULT
owplInit
(
	const int asyncCallbackMode,
	short udpPort,
	short tcpPort,
	short tlsPort,
	const char* szBindToAddr,
	const int bUserSequentialPorts
)
{
	int return_code;
	short useUdp = (udpPort == -1 ? 0 : 1);
	short useTcp = (tcpPort == -1 ? 0 : 1);
	short useTls = (tlsPort == -1 ? 0 : 1);
	const char* ptime;

	return_code = owplAdapterInitialize();
	if (return_code != 0)
	{
		owplLogError("owplAdapterInitialize failed");
		return OWPL_RESULT_FAILURE;
	}
	return_code = owplAdapterNortelInitialize("nortel");
	if (return_code != 0)
	{
		owplLogError("owplAdapterNortelInitialize failed");
		return OWPL_RESULT_FAILURE;
	}

	phcb = (phCallbacks_t * ) malloc(sizeof(phCallbacks_t));
	memset(phcb, 0, sizeof(phCallbacks_t));

	phcfg.asyncmode = asyncCallbackMode;

	return_code = owplInitOwsl(useUdp, useTcp, useTls);
	if (return_code != 0)
	{
		owplLogError("owplInitOwsl failed");
		return OWPL_RESULT_FAILURE;
	}

	osip_trace_initialize_func(OSIP_INFO3, owplOsipLogFunction);

	return_code = eXosip_init(0, 0, udpPort, tcpPort, tlsPort);
	if (return_code != 0)
	{
		owplLogError("eXosip_init failed");
		return OWPL_RESULT_FAILURE;
	}

	{

		const char version[] = stringize(VOXOXVERSION);//VOXOX - CJC - 2009.06.27 
		// VOXOX CHANGE by ASV 06-27-2009: modified the code to be compatible with GCC
		char ua[50] = "VoxOx "; // We need to define he size to make sure strcat has enough space to copy version in ua
		strcat(ua, version);//VOXOX - CJC - 2009.06.27 
		// VOXOX CHANGE by ASV - end
		eXosip_set_user_agent(ua);
	}

	ph_avcodec_init();
	ph_calls_init();

#ifdef FORCE_VAD
/* HACK for test */
#ifdef EMBED
	phcfg.vad = VAD_VALID_MASK | (500 & VAD_THRESHOLD_MASK);
#else
	phcfg.vad = VAD_VALID_MASK | (1000 & VAD_THRESHOLD_MASK);
#endif
#endif

#ifdef FORCE_CNG
  /* HACK for test */
  phcfg.cng = 1;
#endif

	ph_media_init(phcfg.plugin_path);

	ph_vlines_init();

	ph_payloads_init();

	if (!phcfg.audio_dev || phcfg.audio_dev[0] == '\0')
	{
		// Set default audio device if no one has been set before
		owplAudioSetConfigString(0);
	}
#if 0	
	ptime = getenv("EXOSIP_FORCE_PTIME");
	if (!ptime || !*ptime)
	{
		putenv("EXOSIP_FORCE_PTIME=20");
	}
#endif	
	/* register callbacks? */
	eXosip_set_mode(EVENT_MODE);

	if (!phcfg.asyncmode)
	{
		phWaitTimeout = 1;
	}
	else 
	{
		phWaitTimeout = 500;
	}

	if (phcfg.asyncmode)
	{
		osip_thread_create(20000, ph_api_thread, 0);
	}

	pthread_mutex_init(&ph_media_stop_mutex, NULL);

	phIsInitialized = 1;

	owplLogDebug("owplInit finished");

	return OWPL_RESULT_SUCCESS;
}

OWPL_RESULT owplNetworkReinit(short udpPort, short tcpPort, short tlsPort)
{

	return eXosip_network_reinit(udpPort, tcpPort, tlsPort);
}


OWPL_RESULT
owplShutdown()
{
	phTerminate();
	owsl_terminate();
	owslIsInitialized = 0;
	owplEventListenerRemoveAll();
	owplAdapterTerminate();
	return OWPL_RESULT_SUCCESS;
}


/********************************************************************************************
 *								Phone CONFIG related functions								*
 ********************************************************************************************/

OWPL_RESULT 
owplConfigAddAudioCodecByName(const char* szCodecName)
{
	/* TODO */
	/* Verify that the code name is supported before adding to the list */
	if(szCodecName == NULL || strlen(szCodecName) == 0) {
		return OWPL_RESULT_INVALID_ARGS;
	}

	if (strlen(phcfg.audio_codecs) <= 0)
	{
		strcpy(phcfg.audio_codecs, szCodecName);
	}
	else if (strstr(phcfg.audio_codecs,szCodecName) == 0)
	{
		strcat(phcfg.audio_codecs, ",");
		strcat(phcfg.audio_codecs, szCodecName);
	}
	return OWPL_RESULT_SUCCESS;
}


OWPL_RESULT
owplConfigSetAudioCodecs(const char * szCodecs) {
	if(szCodecs == NULL || strlen(szCodecs) == 0) {
		phcfg.audio_codecs[0] = 0;
		return OWPL_RESULT_SUCCESS;
	}
	if(strlen(szCodecs) >= sizeof(phcfg.audio_codecs)) {
		return OWPL_RESULT_INVALID_ARGS;
	}
	memset(phcfg.audio_codecs, 0, sizeof(phcfg.audio_codecs));
	if(strlen(strncpy(phcfg.audio_codecs, szCodecs, sizeof(phcfg.audio_codecs))) == 0) {
		return OWPL_RESULT_FAILURE;
	}

	if (phIsInitialized) {
		ph_payloads_init();
	}

	return OWPL_RESULT_SUCCESS;
}

OWPL_RESULT
owplConfigGetAudioCodecs(const char * szCodecs, size_t size) {
	if(szCodecs == NULL) {
		return OWPL_RESULT_INVALID_ARGS;
	}
	/* TODO: why szCodecs is declared as const ? */
	memset((char *) szCodecs, 0, size);
	if(size < sizeof(phcfg.audio_codecs)) {
		return OWPL_RESULT_INSUFFICIENT_BUFFER;
	}
	if(strlen(strncpy((char *) szCodecs, phcfg.audio_codecs, size)) == 0) {
		return OWPL_RESULT_FAILURE;
	}
	return OWPL_RESULT_SUCCESS;
}

OWPL_RESULT 
owplConfigAddVideoCodecByName(const char* szCodecName)
{
	/* TODO */
	/* Verify that the code name is supported before adding to the list */
	if(szCodecName == NULL || strlen(szCodecName) == 0) {
		return OWPL_RESULT_INVALID_ARGS;
	}

	if (strlen(phcfg.video_codecs) <= 0)
	{
		strcpy(phcfg.video_codecs, szCodecName);
	}
	else if (strstr(phcfg.video_codecs,szCodecName) == NULL)
	{
		strcat(phcfg.video_codecs, ",");
		strcat(phcfg.video_codecs, szCodecName);
	}
	return OWPL_RESULT_SUCCESS;
}

OWPL_RESULT
owplConfigSetVideoCodecs(const char * szCodecs) {
	if(szCodecs == NULL || strlen(szCodecs) == 0) {
		phcfg.video_codecs[0] = 0;
		return OWPL_RESULT_SUCCESS;
	}
	if(strlen(szCodecs) >= sizeof(phcfg.video_codecs)) {
		return OWPL_RESULT_INVALID_ARGS;
	}
	memset(phcfg.video_codecs, 0, sizeof(phcfg.video_codecs));
	if(strlen(strncpy(phcfg.video_codecs, szCodecs, sizeof(phcfg.video_codecs))) == 0) {
		return OWPL_RESULT_FAILURE;
	}
	return OWPL_RESULT_SUCCESS;
}

OWPL_RESULT
owplConfigGetVideoCodecs(const char * szCodecs, size_t size) {
	if(szCodecs == NULL) {
		return OWPL_RESULT_INVALID_ARGS;
	}
	/* TODO: why szCodecs is declared as const ? */
	memset((char *)szCodecs, 0, size);
	if(size < sizeof(phcfg.video_codecs)) {
		return OWPL_RESULT_INSUFFICIENT_BUFFER;
	}
	if(strlen(strncpy((char *)szCodecs, phcfg.video_codecs, size)) == 0) {
		return OWPL_RESULT_FAILURE;
	}
	return OWPL_RESULT_SUCCESS;
}

OWPL_RESULT
owplConfigGetBoundLocalAddr(char * szLocalAddr, size_t size) {
	char ip[256]; // put a big buffer to prevent buffer overflow...

	if(szLocalAddr == NULL) {
		return OWPL_RESULT_INVALID_ARGS;
	}

	memset(szLocalAddr, 0, size);

	if(size <= 0) {
		return OWPL_RESULT_INSUFFICIENT_BUFFER;
	}
	memset(ip, 0, sizeof(ip));
	// HACK : eXosip_get_localip should get the size of the buffer to prevent buffer overflow
	eXosip_get_localip(ip);
	ip[sizeof(ip)-1] = '\0'; // be sure to have a zero terminated string
	if(strlen(ip) > size-1) {
		return OWPL_RESULT_INSUFFICIENT_BUFFER;
	}
	strncpy(szLocalAddr, ip, size-1);
	return OWPL_RESULT_SUCCESS;
}

OWPL_RESULT 
owplConfigSetLocalHttpProxy
(
	const char * address, 
	unsigned short port,
	const char * user,
	const char * password
)
{
	int return_code ;

	return_code = owsl_global_parameter_set ("proxy.local.addr", address) ;
	if (return_code != 0)
	{
		return OWPL_RESULT_FAILURE ;
	}

	return_code = owsl_global_parameter_set ("proxy.local.port", & port) ;
	if (return_code != 0)
	{
		return OWPL_RESULT_FAILURE ;
	}

	return_code = owsl_global_parameter_set ("proxy.local.login", user) ;
	if (return_code != 0)
	{
		return OWPL_RESULT_FAILURE ;
	}

	return_code = owsl_global_parameter_set ("proxy.local.passwd", password) ;
	if (return_code != 0)
	{
		return OWPL_RESULT_FAILURE ;
	}

	return OWPL_RESULT_SUCCESS ;
}

/* TODO: owplConfigLocalHttpProxyGetAddr */
OWPL_RESULT
owplConfigLocalHttpProxyGetAddr(char * szLocalProxyAddr, size_t size) {
	return OWPL_RESULT_SUCCESS;
}

/* TODO: owplConfigLocalHttpProxyGetPasswd */
OWPL_RESULT
owplConfigLocalHttpProxyGetPasswd(char * szLocalProxyPasswd, size_t size) {
	return OWPL_RESULT_SUCCESS;
}

/* TODO: owplConfigLocalHttpProxyGetPort */
OWPL_RESULT
owplConfigLocalHttpProxyGetPort(unsigned short * LocalProxyPort) {
	return OWPL_RESULT_SUCCESS;
}

/* TODO: owplConfigLocalHttpProxyGetUserName */
OWPL_RESULT
owplConfigLocalHttpProxyGetUserName(char * szLocalProxyUserName, size_t size) {
	return OWPL_RESULT_SUCCESS;
}

OWPL_RESULT owplConfigSetHttpTunnel
(
	const char * address,
	unsigned short port,
	unsigned int timeout
)
{
	int return_code ;

	return_code = owplInitOwsl (0, 0, 0) ;
	if (return_code != 0)
	{
		return OWPL_RESULT_FAILURE ;
	}

	return_code = owsl_global_parameter_set ("gateway.http.addr", address) ;
	if (return_code != 0)
	{
		return OWPL_RESULT_FAILURE ;
	}

	return_code = owsl_global_parameter_set ("gateway.http.port", & port) ;
	if (return_code != 0)
	{
		return OWPL_RESULT_FAILURE ;
	}

	return_code = owsl_global_parameter_set ("connection.timeout", & timeout) ;
	if (return_code != 0)
	{
		return OWPL_RESULT_FAILURE ;
	}

	owplHttpTunnelIsInitialized = 1 ;

	return OWPL_RESULT_SUCCESS ;
}

OWPL_RESULT owplConfigEnableHttpTunnel(int tunnelMode, int enableHttps)
{
	int return_code ;

	if (! owplHttpTunnelIsInitialized)
	{
		return OWPL_RESULT_FAILURE ;
	}

	if (tunnelMode == 0)
	{
		owsl_socket_type_remove(OWSL_TYPE_IPV4_UOH);
		owsl_socket_type_remove(OWSL_TYPE_IPV6_UOH);
		owsl_socket_type_remove(OWSL_TYPE_IPV4_UOHS);
		owsl_socket_type_remove(OWSL_TYPE_IPV6_UOHS);

		return_code = owsl_socket_type_add(OWSL_TYPE_IPV4_UDP);
		if (return_code != 0)
		{
			return OWPL_RESULT_FAILURE;
		}

		return_code = owsl_socket_type_add(OWSL_TYPE_IPV6_UDP);
		if (return_code != 0)
		{
			return OWPL_RESULT_FAILURE;
		}
	}
	else
	{
		owsl_socket_type_remove(OWSL_TYPE_IPV4_UDP);
		owsl_socket_type_remove(OWSL_TYPE_IPV6_UDP);
		
		if (enableHttps == 0)
		{
			owsl_socket_type_remove(OWSL_TYPE_IPV4_UOHS);
			owsl_socket_type_remove(OWSL_TYPE_IPV6_UOHS);

			return_code = owsl_socket_type_add(OWSL_TYPE_IPV4_UOH);
			if (return_code != 0)
			{
				return OWPL_RESULT_FAILURE;
			}

			return_code = owsl_socket_type_add(OWSL_TYPE_IPV6_UOH);
			if (return_code != 0)
			{
				return OWPL_RESULT_FAILURE;
			}
		}
		else
		{
			owsl_socket_type_remove(OWSL_TYPE_IPV4_UOH);
			owsl_socket_type_remove(OWSL_TYPE_IPV6_UOH);

			return_code = owsl_socket_type_add(OWSL_TYPE_IPV4_UOHS);
			if (return_code != 0)
			{
				return OWPL_RESULT_FAILURE;
			}

			return_code = owsl_socket_type_add(OWSL_TYPE_IPV6_UOHS);
			if (return_code != 0)
			{
				return OWPL_RESULT_FAILURE;
			}
		}
	}

	return OWPL_RESULT_SUCCESS;
}

OWPL_RESULT owplConfigSetEncryptionMode(int EncryptionMode)
{
	sVoIP_phapi_setCipherMode(EncryptionMode ? 1:0);
	return OWPL_RESULT_SUCCESS;
}

OWPL_RESULT owplConfigGetEncryptionMode(int *EncryptionMode)
{
	*EncryptionMode = sVoIP_phapi_getCipherMode();
	return OWPL_RESULT_SUCCESS;
}

// SPIKE_SPIKE_SIP_SIMPLE
OWPL_RESULT owplConfigEnablePIM(int PimEnabled)
{
	phcfg.pim_disabled = !PimEnabled;
	return OWPL_RESULT_SUCCESS;
}

/********************************************************************************************
 *								Phone AUDIO CONFIG related functions						*
 ********************************************************************************************/

/**
 * Set the audio config string
 * @param szAudioConfig		The audio configuration string
 */
OWPL_RESULT owplAudioSetConfigString(const char* szAudioConfig)
{
	char *szEnvValue;
	char *szFinalAudioConfig;
	/*
	Audio device selection:
	if we have PH_FORCE_AUDIO_DEVICE env var it overrides everything else
	otherwise we try to use the device specified by the UI....
	if UI didn't specify anything we try to use content of PH_AUDIO_DEVICE env var (if it is nonempty)
	and in the last resort we use PortAudio default device
	*/
	szEnvValue = getenv("PH_FORCE_AUDIO_DEVICE");

	if (szEnvValue && szEnvValue[0])
	{
		szFinalAudioConfig = szEnvValue;
	}
	else if (szAudioConfig && szAudioConfig[0])
	{
		szFinalAudioConfig = (char *)szAudioConfig;
	}
	else
	{
		szEnvValue = getenv("PH_AUDIO_DEVICE");
		if (szEnvValue && szEnvValue[0])
		{
			szFinalAudioConfig = szEnvValue;
		}
		else
		{
#if defined(OS_MACOSX)
			strncpy(phcfg.audio_dev, "ca:", sizeof(phcfg.audio_dev));
#else
			strncpy(phcfg.audio_dev, "pa:", sizeof(phcfg.audio_dev));
#endif
			return OWPL_RESULT_SUCCESS;
		}
	}

	if (strlen(szFinalAudioConfig) >= sizeof(phcfg.audio_dev)) {
		return OWPL_RESULT_INVALID_ARGS;
	}
	strncpy(phcfg.audio_dev, szFinalAudioConfig, sizeof(phcfg.audio_dev));
	return OWPL_RESULT_SUCCESS;
}

/********************************************************************************************
 *								Phone LINE related functions								*
 ********************************************************************************************/

/**
 * owplLineAdd
 *
 * add a virtual line 
 * 
 * @param  displayname
 * @param  username
 * @param  server
 * @param  proxy
 * @param  regTimeout	The value that will be put in Expire header of register message
 * @param(out)  phLine  The newly created line handle
 * @return				OWPL_RESULT_SUCCESS in case of success
 */
OWPL_RESULT
owplLineAdd(const char * displayname,
			 const char * sipUsername,
			 const char * sipServer,
			 const char * sipProxy,
			 OWPL_TRANSPORT_PROTOCOL sipTransport,
			 int regTimeout,
			 OWPL_LINE * phLine)
{	
	int ret;
	phVLine *vl;

	vl = ph_find_matching_vline2(sipUsername, sipServer, 0);

	if (vl) {
		vl->regTimeout = regTimeout;
		*phLine = ph_vline2vlid(vl);
	}
	else {
		/* Call phAddVline2 with timeout = 0 to avoid sending right away the REGISTER message.
		 * (this function send a REGISTER message if timeout > 0 and we have shouldn't change it
		 * for backward compatibility
		 */
		ret = phAddVline2(displayname, sipUsername, sipServer, sipProxy, sipTransport, 0); 
		if (ret < 0) {
			return OWPL_RESULT_FAILURE;
		}
		vl = ph_vlid2vline(ret);
		if (vl)	{
			vl->regTimeout = regTimeout;
		}
		*phLine = ret;
		owplLineSetAutoKeepAlive(ret, 1, 30);
	}
	return OWPL_RESULT_SUCCESS;
}

OWPL_RESULT
owplLineDelete(OWPL_LINE hLine, unsigned int skipUnregister) {
	if(skipUnregister) {
		if(phDelVline2(hLine, 0 , 1) == 0) {
			return OWPL_RESULT_SUCCESS;
		} else {
			return OWPL_RESULT_FAILURE;
		}
	}

	if(phDelVline2(hLine, -1,skipUnregister) == 0) {
		return OWPL_RESULT_SUCCESS;
	} else {
		return OWPL_RESULT_FAILURE;
	}
}

OWPL_RESULT
owplLineSetAdapter
(
	const OWPL_LINE hLine,
	const char * adapterName,
	void * lineConfigurationHookUserData,
	void * sipMessageFilterUserData
)
{
	OWPL_RESULT return_code ;
	OWPL_ADAPTER * adapter ;
	phVLine * vline ;

	adapter = owplAdapterGet (adapterName) ;
	if (adapter == NULL)
	{
		return OWPL_RESULT_FAILURE ;
	}

	vline = ph_vlid2vline (hLine) ;
	if (vline == NULL)
	{
		return OWPL_RESULT_FAILURE ;
	}

	if (adapter->sipDomain != NULL)
	{
		return_code = owsip_account_domain_set
		(
			vline->sipAccount,
			adapter->sipDomain
		) ;
		if (return_code != 0)
		{
			return OWPL_RESULT_FAILURE ;
		}
	}

	if (adapter->sipProxy != NULL)
	{
		return_code = owsip_account_proxy_set
		(
			vline->sipAccount,
			adapter->sipProxy
		) ;
		if (return_code != 0)
		{
			return OWPL_RESULT_FAILURE ;
		}
	}

	if (adapter->sipTransport != TRANSPORT_UNKNOWN)
	{
		return_code = owsip_account_transport_set
		(
			vline->sipAccount,
			adapter->sipTransport
		) ;
		if (return_code != 0)
		{
			return OWPL_RESULT_FAILURE ;
		}
	}

	if (adapter->lineConfigurationHook != NULL)
	{
		vline->configurationHook = adapter->lineConfigurationHook ;
		vline->configurationHookUserData = lineConfigurationHookUserData ;
	}

	if (adapter->sipMessageFilter != NULL)
	{
		return_code = owsip_account_message_filter_set
		(
			vline->sipAccount,
			adapter->sipMessageFilter,
			sipMessageFilterUserData
		) ;
		if (return_code != 0)
		{
			return OWPL_RESULT_FAILURE ;
		}
	}

	if (vline->configurationHook != NULL)
	{
		return_code = vline->configurationHook
		(
			hLine,
			vline->configurationHookUserData
		) ;
		if (return_code != 0)
		{
			return OWPL_RESULT_FAILURE ;
		}
	}

	return OWPL_RESULT_SUCCESS ;
}

/**
 * owplLineRegister
 *
 * register a virtual line 
 * 
 * @param  hLine	   handle to the virtual line
 * @param  bRegister   1: Register the line; 0: Unregister the line
 * @return             OWPL_RESULT_SUCCESS in case of success
 */
OWPL_RESULT
owplLineRegister(const OWPL_LINE hLine, const int bRegister)
{
	int ret;
	if (bRegister) {
		ret = phvlRegister(hLine);
	}
	else {
		ret = phvlUnregister(hLine);
	}
	
	if (ret < 0) {
		return OWPL_RESULT_FAILURE;
	}
	return OWPL_RESULT_SUCCESS;
}


OWPL_RESULT
owplLineSetOpts(const OWPL_LINE hLine, LineOptEnum Opt, const void *Data)
{
	int *n;
	phVLine *vl = 0;
	char *s;
	OWSIPAccount account ;

	account = owplLineSipAccountGet (hLine) ;
	if (account <= 0)
	{
		return OWPL_RESULT_INVALID_ARGS ;
	}

	vl = ph_valid_vlid(hLine);
	if (!vl) {
		return OWPL_RESULT_FAILURE;
	}

	switch (Opt)
	{
	case OWPL_LINE_OPT_REG_TIMEOUT:
		n = (int*) Data;
		vl->regTimeout = *n;
		break;
	case OWPL_LINE_OPT_PROXY:
		s = (char*) Data;
		if (strcmp(owsip_account_proxy_get (account), s) != 0)
		{
			if (vl->LineState == LINESTATE_REGISTERED) {
				// Unregister it first
				owplLineRegister(hLine, 0);
			}
			owsip_account_proxy_set (account, s) ;
		}
		break;
	case OWPL_LINE_OPT_DOMAIN:
		s = (char*) Data;
		if (strcmp(owsip_account_domain_get (account), s) != 0)
		{
			if (vl->LineState == LINESTATE_REGISTERED) {
				// Unregister it first
				owplLineRegister(hLine, 0);
			}
			strcpy(owsip_account_domain_get (account), s);
		}
		break;
	
	case OWPL_LINE_OPT_ROUTEFILTERING:
		owsip_account_route_filter_enable(account, (int) Data);
		break;
	}

	return OWPL_RESULT_SUCCESS;
}

/**
 * owplLineGetProxy
 *
 * Get the proxy address of the line
 * 
 * @param  hLine	   handle to the virtual line
 * @param  szBuffer    The buffer to contain the result. 
 * @param  nBuffer	   The size of the buffer. On return, nBuffer will contain the actual by written to the buffer.
 * @return             OWPL_RESULT_SUCCESS in case of success. On failure, nBuffer will contain
 *					   the size needed to store the result.
 */
OWPL_RESULT 
owplLineGetProxy(const OWPL_LINE hLine,
                        char*  szBuffer,
                        int * nBuffer)
{
	int n = 1; /* +1 for \0 at the end of the string */
	char * proxy ;
	OWSIPAccount account ;

	account = owplLineSipAccountGet (hLine) ;
	if (account <= 0)
	{
		return OWPL_RESULT_INVALID_ARGS ;
	}

	if (!nBuffer) {
		return OWPL_RESULT_FAILURE;
	}

	proxy = owsip_account_proxy_get (account) ;
	if (szBuffer)
	{
		if (proxy != NULL)
		{
			if (proxy)
			{
				n += strlen(proxy) ;
			}
			if (n >= * nBuffer)
			{
				* nBuffer = n ;
				return OWPL_RESULT_INSUFFICIENT_BUFFER ;
			}
			strncpy (szBuffer, proxy, n) ;
			* nBuffer = n ;
		}
		else
		{
			* szBuffer = 0 ;
			* nBuffer = 1 ;
		}
	}
	return OWPL_RESULT_SUCCESS;
}

/**
 * owplLineGetUserName
 *
 * Get username of the line
 * 
 * @param  hLine	   handle to the virtual line
 * @param  szBuffer    The buffer to contain the result. 
 * @param  nBuffer	   The size of the buffer. On return, nBuffer will contain the actual by written to the buffer.
 * @return             OWPL_RESULT_SUCCESS in case of success. On failure, nBuffer will contain
 *					   the size needed to store the result.
 */
OWPL_RESULT 
owplLineGetLocalUserName(const OWPL_LINE hLine,
                        char*  szLocalUserName,
                        int * nBuffer)
{
	int n = 0;
	OWSIPAccount account ;

	account = owplLineSipAccountGet (hLine) ;
	if (account <= 0)
	{
		return OWPL_RESULT_INVALID_ARGS ;
	}

	if (owsip_account_user_get (account) && strlen(owsip_account_user_get (account))) {
		n += strlen(owsip_account_user_get (account)); // +1 for \0 at the end of the string
	}
	
	if (n >= *nBuffer) {
		*nBuffer = n;
		return OWPL_RESULT_INSUFFICIENT_BUFFER;
	}
	if (szLocalUserName) {		
		strncpy(szLocalUserName, owsip_account_user_get (account), n);
		szLocalUserName[n] = '\0';
	}
	return OWPL_RESULT_SUCCESS;
}

/**
 * owplLineGetUri
 *
 * Get uri of the line
 * 
 * @param  hLine	   handle to the virtual line
 * @param  szBuffer    The buffer to contain the result. 
 * @param  nBuffer	   The size of the buffer. If 
 * @return             OWPL_RESULT_SUCCESS in case of success. On failure, nBuffer will contain
 *					   the size needed to store the result.
 */
OWPL_RESULT 
owplLineGetUri(const OWPL_LINE hLine,
                        char*  szBuffer,
                        int * nBuffer)
{
	char tmp[10];
	int n = 0;
	OWSIPAccount account ;

	account = owplLineSipAccountGet (hLine) ;
	if (account <= 0)
	{
		return OWPL_RESULT_INVALID_ARGS ;
	}

	if (owsip_account_domain_get (account) && strlen(owsip_account_domain_get (account))) {
		n += strlen(owsip_account_domain_get (account));
	}
	if (owsip_account_user_get (account) && strlen(owsip_account_user_get (account))) {
		n += strlen(owsip_account_user_get (account));
	}
	if (owsip_account_port_get (account) != 5060) {
		snprintf(tmp, sizeof(tmp), "%d", owsip_account_port_get (account));
		n += strlen(tmp) + 1;
	}
	n += 5;	
	if (n > *nBuffer) {
		*nBuffer = n;
		return OWPL_RESULT_INSUFFICIENT_BUFFER;
	}
	if (szBuffer) {
		if (owsip_account_port_get (account) != 5060) {
			snprintf(szBuffer, *nBuffer, "sip:%s@%s:%d", owsip_account_user_get (account), owsip_account_domain_get (account), owsip_account_port_get (account));
		}
		else {
			snprintf(szBuffer, *nBuffer, "sip:%s@%s", owsip_account_user_get (account), owsip_account_domain_get (account));
		}
	}
	return OWPL_RESULT_SUCCESS;
}

/**
 * owplLineAddCredential
 *
 * Add credential information to a line
 */
OWPL_RESULT
owplLineAddCredential(const OWPL_LINE hLine,                                                 
						const char* szUserID,
						const char* szPasswd,
						const char* szRealm)
{
	int ret;
	OWSIPAccount account ;

	account = owplLineSipAccountGet (hLine) ;
	if (account <= 0)
	{
		return OWPL_RESULT_INVALID_ARGS ;
	}

	if (szUserID == NULL || szPasswd == NULL)
	{
		return OWPL_RESULT_INVALID_ARGS ;
	}

	if (szRealm == NULL)
	{
		szRealm = owsip_account_domain_get (account) ;
		if (szRealm == NULL)
		{
			return OWPL_RESULT_FAILURE ;
		}
	}

	eXosip_lock();
	ret = eXosip_add_authentication_info(szUserID, szUserID, szPasswd, "", szRealm);
	eXosip_unlock();

	if(ret != 0) {
		return OWPL_RESULT_FAILURE;
	}
	return OWPL_RESULT_SUCCESS;
}

OWPL_RESULT
owplLineSetBasicAuthenticationAtFirstMessage
(
	const OWPL_LINE hLine,
	int basicAuthenticationAtFirstMessage
)
{
	OWSIPAccount account ;

	account = owplLineSipAccountGet (hLine) ;
	if (account <= 0)
	{
		return OWPL_RESULT_INVALID_ARGS ;
	}

	if
	(
		owsip_account_basic_authentication_in_first_message_set
		(
			account,
			basicAuthenticationAtFirstMessage
		)
	)
	{
		return OWPL_RESULT_FAILURE ;
	}

	return OWPL_RESULT_SUCCESS ;
}

OWPL_RESULT
owplLineSetBusy(OWPL_LINE hLine,
				unsigned int bBusy) {
	if(phLineSetBusy(hLine, bBusy) != 0) {
		return OWPL_RESULT_FAILURE;
	}
	return OWPL_RESULT_SUCCESS;
}

OWPL_RESULT
owplLineIsBusy(OWPL_LINE hLine,
			   unsigned int * bBusy) {
	phVLine *vl = ph_valid_vlid(hLine);

	if(!vl) {
		return OWPL_RESULT_FAILURE;
	}

	*bBusy = vl->busy;
	return OWPL_RESULT_SUCCESS;
}

/**
 * owplLineSetAutoKeepAlive
 *
 * @param  hLine		handle to the virtual line
 * @param  Enabled		1: Enabled, 0: Disabled
 * @param  Period		The timeout period between 2 keep alive messages (in seconds)
 * @return				OWPL_RESULT
 */
OWPL_RESULT
owplLineSetAutoKeepAlive(OWPL_LINE hLine,
			   unsigned int Enabled,
			   unsigned int Period)
{
	phVLine *vl = ph_valid_vlid(hLine);

	if(!vl) {
		return OWPL_RESULT_FAILURE;
	}
	
	vl->keepAliveInfo.enabled = Enabled;
	vl->keepAliveInfo.period = Period;
	vl->keepAliveInfo.lastSentTime = time(0);
	return OWPL_RESULT_SUCCESS;
}

/********************************************************************************************
 *								Phone CALL related functions								*
 ********************************************************************************************/

int owStaticCallId = 0;

int getNextCallId()
{
	return ++owStaticCallId;
}

/**
 * Create a call object
 *
 * @param hLine			Handle to the line that is used for the call
 * @param hCall			The newly created call handle will be stored in this param
 */

OWPL_RESULT
owplCallCreate( const OWPL_LINE hLine,
				 OWPL_CALL * hCall)
{
	phVLine *vl;
	int newCallId;
	phcall_t *ca;

	//Check hLine validity
	vl = ph_valid_vlid(hLine);
	if (!vl) {
		return OWPL_RESULT_INVALID_ARGS;
	}

	newCallId = getNextCallId();
	ca = ph_allocate_call(newCallId);
	if ( ca == 0)
	{
		return OWPL_RESULT_FAILURE;
	}

	ca->vlid = hLine;
	*hCall = newCallId;
	return OWPL_RESULT_SUCCESS;
}

/**
 * Connect an already created call
 *
 * @param hCall				The handle to the created call.
 * @param szAddress			The SIP URI to call
 * @param mediaStreams		The flag to say which kind of media will be available.
 *							The value can be composed of the following bit :
 *							OWPL_STREAM_AUDIO, OWPL_STREAM_VIDEO_RX, OWPL_STREAM_VIDEO_TX
 */
OWPL_RESULT
owplCallConnect(const OWPL_CALL hCall,
				const char* szAddress,
				int mediaStreams)
{
	phcall_t *ca = ph_locate_call_by_cid(hCall);

	if (phLinePlaceCall_withCa(ca->vlid, szAddress, 0, 0, mediaStreams, ca) <= 0)
	{
		return OWPL_RESULT_FAILURE;
	}

	return OWPL_RESULT_SUCCESS;
}

/**
 * Connect an already created call with a custom body
 *
 * @param hCall				The handle to the created call.
 * @param szAddress			The SIP URI to call
 * @param szContentType		The content type of the message
 * @param body				The body of the message 
 * @param BodySize			The size of the body of the message 
 *
 * ATTENTION: BodySize param is not yet taken into account by implementation. For now, the size is strlen(body).
 * So, body should be a zero terminated string
 *							
 */

OWPL_RESULT
owplCallConnectWithBody(const OWPL_CALL hCall,
				const char* szAddress,
				const char* szContentType,
				const char* szBody,
				int BodySize)
{
	int i;
	osip_message_t *invite;
	char *proxy ;
	phVLine *vl;
	char from[512];
	OWSIPAccount account ;

	phcall_t *ca = ph_locate_call_by_cid(hCall);
	if (ca == NULL)
	{
		return OWPL_RESULT_INVALID_ARGS ;
	}

	account = owplLineSipAccountGet (ca->vlid) ;
	if (account <= 0)
	{
		return OWPL_RESULT_INVALID_ARGS ;
	}

	// TODO verif des arguments
	if (!szAddress){
		return OWPL_RESULT_INVALID_ARGS;
	}

	vl = ph_valid_vlid(ca->vlid);
	if (!vl) {
		return OWPL_RESULT_INVALID_ARGS;
	}

	ph_vline_get_from(from, sizeof(from), vl);

	proxy = owsip_account_proxy_get (account) ;

	if((i = eXosip_build_initial_invite(&invite, (char *)szAddress, from, proxy, "")) != 0){
		return -1;
	}

	eXosip_lock();

	i = eXosip_initiate_call_with_body(account, invite, szContentType, szBody, 0);
	if (i <= 0) {
		return OWPL_RESULT_FAILURE;
	}

	ca->extern_cid = i;
	ca->vlid = ph_vline2vlid(vl);

	eXosip_unlock(); 

	owplAssociateCall2PluginByContentType(ca->cid, szContentType);

	return OWPL_RESULT_SUCCESS;
}

/**
 * Accept an incoming call. The message 180 RINGING is sent *ONLY* after the call to this function
 *
 * @param hCall				The handle to the incoming call.
 * @param mediaStreams		The flag to say which kind of media will be available.
 *							The value can be composed of the following bit :
 *							OWPL_STREAM_AUDIO, OWPL_STREAM_VIDEO_RX, OWPL_STREAM_VIDEO_TX
 */
OWPL_RESULT
owplCallAccept(const OWPL_CALL hCall, int mediaStreams)
{
	int ret = phRingingCall(hCall);

	if (ret != 0) {
		return OWPL_RESULT_FAILURE;
	}
	return OWPL_RESULT_SUCCESS;
}


/**
 * Answer an incoming call
 *
 * @param hCall				The handle to the incoming call. 
 * @param mediaStreams		The flag to say which kind of media will be available.
 *							The value can be composed of the following bit :
 *							OWPL_STREAM_AUDIO, OWPL_STREAM_VIDEO_RX, OWPL_STREAM_VIDEO_TX
 */
OWPL_RESULT
owplCallAnswer(const OWPL_CALL hCall, int mediaStreams)
{
	int ret = phAcceptCall3(hCall, 0, mediaStreams);

	if (ret != 0) {
		return OWPL_RESULT_FAILURE;
	}
	return OWPL_RESULT_SUCCESS;
}

/**
 * Answer an incoming call with the specified body
 *
 * @param hCall				The handle to the incoming call.
 * @param szContentType		The content type of the message
 * @param body				The body of the message 
 * @param BodySize			The size of the body of the message
 */
OWPL_RESULT
owplCallAnswerWithBody (const OWPL_CALL hCall,
			const char* szContentType,
			const char* szBody,
			int BodySize) {
	int i;
	phcall_t *ca = ph_locate_call_by_cid(hCall);

	if(!ca) {
		return OWPL_RESULT_INVALID_ARGS;
	}

	eXosip_lock();
	i = eXosip_answer_call_with_body(ca->did, 200, szContentType, szBody); // returns 0 on success; -1 else
	eXosip_unlock();

	if(i != 0) {
		return OWPL_RESULT_FAILURE;
	}

	return OWPL_RESULT_SUCCESS;
}

/**
 * Reject an incoming call
 *
 * @param hCall				The handle to the incoming call.
 * @param errorCode			The SIP error code (eg. 486 - Busy Here, 488 - Not acceptable here)
 * @param szErrorText		The text that will be sent in SIP response with the error code
 */
OWPL_RESULT
owplCallReject(const OWPL_CALL hCall,
				const int errorCode,
				const char* szErrorText)
{
	int ret = phRejectCall(hCall, errorCode);
	if (ret != 0) {
		return OWPL_RESULT_FAILURE;
	}
	return OWPL_RESULT_SUCCESS; 
}

/**
 * Close an active call
 *
 * @param hCall				The handle to the active call. 
 */

OWPL_RESULT owplCallDisconnect(OWPL_CALL hCall)
{
	int ret = phCloseCall(hCall);
	if (ret != 0) {
		return OWPL_RESULT_FAILURE;
	}
	return OWPL_RESULT_SUCCESS; 
}

/**
 * Reject an incoming call with a predefined reason
 *
 * @param hCall				The handle to the incoming call.
 * @param Reason			The predefined reason
 */
OWPL_RESULT
owplCallRejectWithPredefinedReason(const OWPL_CALL hCall,
				   OWPL_CALL_REFUSED_REASON Reason)
{
	switch(Reason) {
		case OWPL_CALL_REFUSED_BUSY :
			if(phReject(hCall) != 0) {
				return OWPL_RESULT_FAILURE;
			}
			break;
		default :
			return OWPL_RESULT_INVALID_ARGS;
			break;
	}

	return OWPL_RESULT_SUCCESS;
}

OWPL_RESULT 
owplCallHold(const OWPL_CALL hCall) {
	if(phHoldCall(hCall) == 0) {
		return OWPL_RESULT_SUCCESS;
	}
	return OWPL_RESULT_FAILURE;
}

OWPL_RESULT 
owplCallHoldWithBody(const OWPL_CALL hCall, 
		     const char * szContentType, 
		     const char * szBody, 
		     int BodySize) {
	phcall_t *ca = ph_locate_call_by_cid(hCall);
	int i;

	if(!ca) {
		return OWPL_RESULT_FAILURE;
	}

	if(ca->localhold) {
		return OWPL_RESULT_FAILURE;
	}

	ca->localhold = 1;

	eXosip_lock();
	i = eXosip_on_hold_call_with_body(ca->did, szContentType, szBody);
	eXosip_unlock();

	if(i==0) {
		return OWPL_RESULT_SUCCESS;
	}
	return OWPL_RESULT_FAILURE;
}

OWPL_RESULT
owplCallUnhold(const OWPL_CALL hCall) {
	if(phResumeCall(hCall) == 0) {
		return OWPL_RESULT_SUCCESS;
	}
	return OWPL_RESULT_FAILURE;
}

OWPL_RESULT
owplCallUnholdWithBody(const OWPL_CALL hCall, 
					   const char * szContentType, 
					   const char * szBody, 
					   int BodySize) 
{
	phcall_t *ca = ph_locate_call_by_cid(hCall);
	int i;

	if(!ca) {
		return OWPL_RESULT_FAILURE;
	}

	if(ca->localhold != 1) {
		return OWPL_RESULT_FAILURE;
	}

	ca->localhold = 0;

	eXosip_lock();
	i = eXosip_off_hold_call_with_body(ca->did, szContentType, szBody);
	eXosip_unlock();

	if(i==0) {
		return OWPL_RESULT_SUCCESS;
	}
	return OWPL_RESULT_FAILURE;
}

OWPL_RESULT owplCallGetEncryptionMode(const OWPL_CALL hCall,
													int * EncryptionMode)
{
	phcall_t *ca = ph_locate_call_by_cid(hCall);

	if(!ca) {
		return OWPL_RESULT_FAILURE;
	}
	*EncryptionMode = sVoIP_phapi_isCrypted(ca->extern_cid);

	return OWPL_RESULT_SUCCESS;
}

/********************************************************************************************
 *								Presence related functions								*
 ********************************************************************************************/

/**
 * Sends a SUBSCRIBE message to a sip URI.
 * 
 * @param hLine		The handle to the line.
 * @param szUri		Sip address of the person to subscribe to
 * @param winfo		0: Subscribe for presence
					1: Subscribe for watcher info
 * @param hSub		The handle to this subscription

 */
OWPL_RESULT owplPresenceSubscribe(OWPL_LINE  hLine,
                                          const char* szUri,
                                          const int winfo,
										  OWPL_SUB *hSub)
{
	OWSIPAccount account ;
	char UriBuf[100];
	char ProxyBuf[100];
	int n = sizeof(UriBuf);
	int i;
	
	// SPIKE_SPIKE_SIP_SIMPLE
	if (phcfg.pim_disabled) {
		return OWPL_RESULT_SUCCESS;
	}

	account = owplLineSipAccountGet (hLine) ;
	if (account <= 0)
	{
		return OWPL_RESULT_INVALID_ARGS ;
	}

	owplLineGetUri(hLine, UriBuf, &n);

	n = sizeof(ProxyBuf);
	owplLineGetProxy(hLine, ProxyBuf, &n);

	eXosip_lock();
	i = eXosip_subscribe(account, (char *)szUri, UriBuf, ProxyBuf, winfo);
	eXosip_unlock();
	if (i < 0) {
		return OWPL_RESULT_FAILURE;
	}
	if (hSub) {
		*hSub = i;
	}
	return OWPL_RESULT_SUCCESS;
}

OWPL_RESULT owplPresenceUnsubscribe(OWPL_SUB hSub)
{
	int i = 0;
	// SPIKE_SPIKE_SIP_SIMPLE
	if (phcfg.pim_disabled) {
		return OWPL_RESULT_SUCCESS;
	}

	eXosip_lock();
	i = eXosip_subscribe_close(hSub);
	eXosip_unlock();

	if(i != 0) {
		owplFireSubscriptionEvent(hSub,
			OWPL_SUBSCRIPTION_CLOSE_FAILED,
			SUBSCRIPTION_CAUSE_UNKNOWN,
			NULL, NULL);
	} else {
		owplFireSubscriptionEvent(hSub,
			OWPL_SUBSCRIPTION_CLOSED,
			SUBSCRIPTION_CAUSE_NORMAL,
			NULL, NULL);
	}
	return OWPL_RESULT_SUCCESS;
}

OWPL_RESULT
owplPresenceUnsubscribeFromUri(OWPL_LINE  hLine, const char * szRemoteUri) 
{
	OWPL_SUB hSub;
	int i = 0;

	// SPIKE_SPIKE_SIP_SIMPLE
	if (phcfg.pim_disabled) {
		return OWPL_RESULT_SUCCESS;
	}

	if(szRemoteUri == NULL || strlen(szRemoteUri) <= 0) {
		return OWPL_RESULT_INVALID_ARGS;
	}

	eXosip_lock();
	i = eXosip_get_subscribe_id(szRemoteUri, &hSub);
	if(i == 0) {
		i = eXosip_subscribe_close(hSub);
	}
	eXosip_unlock();
	if(i != 0) {
		owplFireSubscriptionEvent(hSub,
			OWPL_SUBSCRIPTION_CLOSE_FAILED,
			SUBSCRIPTION_CAUSE_UNKNOWN,
			szRemoteUri, NULL);
	} else {
		owplFireSubscriptionEvent(hSub,
			OWPL_SUBSCRIPTION_CLOSED,
			SUBSCRIPTION_CAUSE_NORMAL,
			szRemoteUri, NULL);
	}
	return OWPL_RESULT_SUCCESS;
}

/**
 * Sends a PUBLISH message to change the presence of the current user.
 * 
 * @param hLine			The handle to the line.
 * @param Online		0: Offline
						1: Online
 * @param szStatus		The status message if Online is 1
 * @param hPub			The handle to this publish (Unused now)
 */

#define PUBLISH_MSG_TEMPLATE "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\
<presence xmlns=\"urn:ietf:params:xml:ns:pidf\"\n\
entity=\"%s\">\n\
<tuple id=\"azersdqre\">\n\
<status><basic>%s</basic></status>\n\
<note>%s</note>\n\
<contact priority=\"1\">%s\
</contact>\n\
</tuple>\n\
</presence>\n"

#define PRESENCE_CONTENT_TYPE "application/pidf+xml"


static void buildPidfPayload(OWPL_LINE  hLine, char *bodyBuf, int size, int Online, const char *szStatus)
{
   char UriBuf[100];

   int n = sizeof(UriBuf);
   owplLineGetUri(hLine, UriBuf, &n);
   snprintf(bodyBuf, size, PUBLISH_MSG_TEMPLATE, UriBuf, Online ? "open" : "closed", szStatus, UriBuf);

}


OWPL_RESULT owplPresencePublish(OWPL_LINE  hLine,
                                          const int Online,
                                          const char * szStatus,
										  OWPL_PUB *hPub)
{
	char UriBuf[100];
	char MsgBodyBuf[500];
	char ProxyBuf[100];
	int n = sizeof(UriBuf);
	int i;
	phVLine * vl = NULL;
	OWSIPAccount account ;

	// SPIKE_SPIKE_SIP_SIMPLE
	if (phcfg.pim_disabled) {
		return OWPL_RESULT_SUCCESS;
	}

	account = owplLineSipAccountGet (hLine) ;
	if (account <= 0)
	{
		return OWPL_RESULT_INVALID_ARGS ;
	}

	// save infos for later user from a timer event
	if((vl = ph_valid_vlid(hLine)) != NULL) {
		vl->publishInfo.onlineState = Online;
		if (szStatus) {
			vl->publishInfo.szStatus = strdup(szStatus);
		} else {
			vl->publishInfo.szStatus = strdup("");
		}
		vl->publishInfo.hPub = hPub;
		// nine minutes timeout i.e. 540s
		vl->publishInfo.publishTimeout = 540;
		vl->publishInfo.lastPublishTime = time(0);
	} else {
		return OWPL_RESULT_FAILURE;
	}

	buildPidfPayload(hLine, MsgBodyBuf, sizeof(MsgBodyBuf), Online, szStatus);

	n = sizeof(UriBuf);
	owplLineGetUri(hLine, UriBuf, &n);


	n = sizeof(ProxyBuf);
	owplLineGetProxy(hLine, ProxyBuf, &n);

	eXosip_lock();
	i = eXosip_publish(account, UriBuf, UriBuf, ProxyBuf, 0, PRESENCE_CONTENT_TYPE, MsgBodyBuf);
	eXosip_unlock();
	
	if (i != 0) {
		return OWPL_RESULT_FAILURE;
	}
	return OWPL_RESULT_SUCCESS;
}



/**
 * Accept or reject incoming SUBSCRIBE request.
 *
 * @param id			incoming subscribe id
 * @param status                status code
 *
 */
OWPL_RESULT
owplSubscribeAccept(OWPL_LINE hLine, int id, int status, int online, const char *note)
{
  return phAcceptSubscribe(hLine, id, status, online, note);
}


OWPL_RESULT
owplPresenceNotify(OWPL_LINE hLine, int id, int Online, const char *szStatus, const char *body)
{
  char bodyBuf[1024];
  int i;

  if (!body)
    {
	buildPidfPayload(hLine, bodyBuf, sizeof(bodyBuf), Online, szStatus);
	body = bodyBuf;
    }

  eXosip_lock();
  i = eXosip_notify2(id,  EXOSIP_SUBCRSTATE_ACTIVE, 0,  PRESENCE_CONTENT_TYPE, body);
  eXosip_unlock();

  return i;


}


/********************************************************************************************
 *								Message related functions								*
 ********************************************************************************************/

OWPL_RESULT
owplMessageSend(OWPL_LINE hLine,
				const char * szRemoteUri,
				const char * szContent,
				const char * szMIME,
				int * messageId)
{
	phVLine *vl;
	char from[512];
	OWSIPAccount account ;

	// SPIKE_SPIKE_SIP_SIMPLE
	if (phcfg.pim_disabled) {
		return OWPL_RESULT_SUCCESS;
	}

	account = owplLineSipAccountGet (hLine) ;
	if (account <= 0)
	{
		return OWPL_RESULT_INVALID_ARGS ;
	}

	if(hLine <= 0 
		|| szRemoteUri == NULL
		|| strlen(szRemoteUri) <= 0
		|| szMIME == NULL
		|| strlen(szMIME) <= 0
		|| szContent == NULL
		|| strlen(szContent) <= 0
		|| messageId == NULL)
	{
		return OWPL_RESULT_INVALID_ARGS;
	}

	vl = ph_valid_vlid(hLine);
	if(!vl) {
		return OWPL_RESULT_INVALID_ARGS;
	}

	ph_vline_get_from(from, sizeof(from), vl);

	eXosip_lock();
	* messageId = eXosip_message
	(
		account,
		(char *) szRemoteUri,
		from,
		owsip_account_proxy_get (account),
		(char *) szContent,
		(char *) szMIME
	) ;
	eXosip_unlock();

	return OWPL_RESULT_SUCCESS;
}

OWPL_RESULT
owplMessageSendPlainText(OWPL_LINE hLine,
				const char * szRemoteUri,
				const char * szContent,
				int * messageId)
{
	return owplMessageSend(hLine, szRemoteUri, szContent, "text/plain", messageId);
}



OWPL_RESULT
owplMessageSendTypingState(OWPL_LINE hLine,
				const char * szRemoteUri,
				OWPL_TYPING_STATE state,
				int * messageId)
{
	switch(state) {
		case OWPL_TYPING_STATE_TYPING :
			return owplMessageSend(hLine, szRemoteUri, "is typing", "typingstate/typing", messageId);

		case OWPL_TYPING_STATE_STOP_TYPING :
			return owplMessageSend(hLine, szRemoteUri, "stops typing", "typingstate/stoptyping", messageId);

		case OWPL_TYPING_STATE_NOT_TYPING :
			return owplMessageSend(hLine, szRemoteUri, "is not typing", "typingstate/nottyping", messageId);

		default :
			break;
	}
	return OWPL_RESULT_FAILURE;
}

OWPL_RESULT
owplMessageSendIcon(OWPL_LINE hLine,
				const char * szRemoteUri,
				const char * szIconFileName, 
				int * messageId)
{
	char szMime[1024] = "buddyicon/";

	return OWPL_RESULT_SUCCESS;
/*
	strncat(szMime, szIconFileName, sizeof(szMime));

	return owplMessageSend(hLine, szRemoteUri, "has changed his icon", szMime, messageId);
*/
}
