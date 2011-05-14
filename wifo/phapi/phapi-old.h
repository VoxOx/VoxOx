/*
  The phapi module implements simple interface for eXosip stack
  Copyright (C) 2004  Vadim Lebedev  <vadim@mbdsys.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __PHAPI_H__
#define __PHAPI_H__ 1

/**
 * @file phoneapi.h
 * @brief softphone  API
 *
 * phoneapi is a library providing simplified api to create VOIP sessions
 * using eXosip library oSIP stack and oRTP stack
 * <P>
 */

/**
 * @defgroup phAPI  Phone API
 * @{
 *
 *  From the perspecitve of the phApi client the call can be in the following states:
 *
 *   [INCOMING], [ACCEPTING], [OUTGOING], [ESTABLISHED], [ONHOLD], [CLOSED]
 *
 *
 *                                              V
 *                           +--(INCALL)--------+----(phPlaceCall)-------------------+
 *                           |                                                       |
 *                           |                                                       |
 *                           v                                                       v
 *  +--(phRejectCall)--<-[INCOMING]<----<------- +                               [OUTGOING]--(DIALING/RINGING)-->-+
 *  |                        v                   ^                                v   v  ^                        |
 *  |                        |                   |                                |   |  |                        |
 *  |                        +--(phRingingCall)--+                                |   |  |                        |
 *  |                        |                                                    |   |  |                        |
 *  |                        v                                                    |   |  |                        |
 *  |                   (phAcceptCall)            +--------+                      |   |  +------------------------+
 *  |                        |                 (DTMF)      |                      |   |
 *  |                        |                    ^        |                  (CALLOK)|
 *  |                        v                    |        v                      v   |
 *  |                  [ACCEPTING]->--(ret==0)-->[ESTABLISHED]<--+---------+------+   +-------->+
 *  |                        |                    v        v               ^                    v
 *  |                        |                    |        |               |                    |
 *  |                (CALLCLOSED/CALLERROR)       +   (CALLHELD/HOLDOK)    |    (CALLCLOSED/CALLERROR/CALLBUSY/NOANSWER/REDIRECTED)
 *  |                        |                    |        |               |                    |
 *  |                        |             (CALLCLOSED)    |               ^                    |
 *  |                        v                    |        +->[ONHOLD]->(CALLRESUMED/RESUMEOK)  |
 *  |                        |                    |             v                               |
 *  |                        |                    |             |                               |
 *  |                        |                    +<-----(CALLCLOSED)                           |
 *  |                        |                    |                                             |
 *  v                        |                    v                                             v
 *  +------------------------+------->[CLOSED]<---+---------------------------------------------+
 *
 *
 *   Blind transfer operation
 *
 *   Suppose we have 2 parties A and B  and a call 'cid'  established between them.
 *   On the A's side the call is identified bu cidA  and on the B's side the call is identfied by cidB.
 *   Suppose the user A  want to transfer the call to a 3rd party  P.
 *   So A does :
 *     phBlindTransferCall(cidA, "P");
 *   this will cause following sequence of events:
 *     1. B  will receive a CALLHELD event
 *     2. B  will receive a XFERREQ  event for cidB containingg "P" as remoteUri and newcid field will
 *           contain a callid for an automatically generated call to the new destination
 *   A will receive a XFERPROGRESS event
 *   P will receive a INCALL event
 *   P will do:
 *     phAcceptCall
 *   B will receive CALLOK
 *   A will receive XFEROK
 *   A will get CALLCLOSED for cidA
 *   B will get CALLCLOSED  for cidB
 *
 *  In case of failure transfer for whatever reason A will receive an
 *   XFERFAIL event with status field containing SIP status code
 *
 *   the file ../miniua/minua.c contains the code demonstrating the usage of blind transfer
 *
 *
 *  Assisted Transfer operation:
 *
 *   Suppose we have 2 parties A and B  and a call 'cid'  established between them.
 *   On the A's side the call is identified bu cidA  and on the B's side the call is identfied by cidB.
 *   Suppose the user A  want to transfer the call to a 3rd party  P.
 *   So A does :
 *     phHoldCall(cidA);
 *   B will receive  CALLHELD event
 *   A does:
 *       newcid = phPlaceCall2("A", "P");
 *   P Gets INCALL event with cidP0 and accepts it
 *   A talks to B and then does
 *     phHoldCall(newcid)
 *   P gets CALLHELD event
 *   A does
 *     phTransferCall(cidA, newcid)
 *   B  will receive a XFERREQ  event for cidB containing "P" as remoteUri and newcid field will
 *      contain a callid for an automatically generated call to the new destination
 *   A gets XFERPROGRESS events
 *   P will get CALLREPLACED for cidP0 with newcid cidP1
 *   A gets XFEROK event
 *   A will get CALLCLOSED for cidA
 *   B will get CALLCLOSED for cidB
 *   P will get CALLCLOSED for cidP0
 *
 */


/**
 * Forward declarations
 */
struct phcall;

#ifdef WIN32
	#if defined(BUILD_PHAPI_DLL)
		#define MY_DLLEXPORT __declspec(dllexport)
	#elif defined(PHAPI_DLL)
		#define MY_DLLEXPORT __declspec(dllimport)
	#endif
#endif

#ifndef MY_DLLEXPORT
	#define MY_DLLEXPORT
#endif


#ifndef PHAPI_VERSION
#define PHAPI_VERSION "0.2.0"
#endif

/**************************************
 MINH: Added in order to implement new APIs
 **************************************/
int getNextCallId();

#if defined(WIN32) || defined(WIN32_WCE)
#define PH_DEPRECATED
/*
#pragma deprecated(phHoldOn,phNoAnswer,phGlobalFailure,phServerFailure,phRequestFailure,phEndCall,\
phCancel,phBye,phReject,phAccept,phStopRinging,phNewCall,phInvite,ph_get_username)
*/
#else
#define PH_DEPRECATED(NewFuncName) __attribute__ ((deprecated))
#endif
/**************************************/

#ifdef __cplusplus
extern "C" {
#endif

typedef enum OWPL_TRANSPORT_PROTOCOL
{
	OWPL_TRANSPORT_UNKNOWN = -1,
	OWPL_TRANSPORT_UDP,
	OWPL_TRANSPORT_TCP,
	OWPL_TRANSPORT_TLS
} OWPL_TRANSPORT_PROTOCOL ;

enum phErrors {
  PH_ERROR=1,    /* generic error */
  PH_HOLDERR,     /* HOLD/RESUME error */
  PH_BADID,      /* bad identity -  usually ther is no virtual line which correpond to 'from' or userid parameter */
  PH_BADVLID,   /*  Bad virual line ID  */
  PH_BADCID,     /* Bad callid */
  PH_NOMEDIA,     /* No media stream avalable */
  PH_NOTUNNEL,    /* Unable to create tunnel */
  PH_NORESOURCES, /* No resources for operation */
  PH_RPCERR,     /* RPC error */
  PH_BADARG,      /* BAD argument */
  PH_VLBUSY,       /* There is oparation pending on this VLINE */
  PH_BADCFID,       /* bad conf call id */
  PH_REDIRLOOP,    /* setFollomMe or blindTransfer creates a loop */
  PH_NOAUDIODEVICE /* failed to open an audio device */
};

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#endif

#define PH_REFRESH_INTERVAL		30  /* 30 seconds */

#define PH_SOCK_MODE_UDP 0
#define PH_SOCK_MODE_HTTP_TUNNEL 1
#define PH_STREAM_AUDIO (1 << 0)
#define PH_STREAM_VIDEO_RX (1 << 1)
#define PH_STREAM_VIDEO_TX (1 << 2)

/********************TELEPHONY*********************/

/**
 * Add virtual line
 * The virtual line corresponds to identity/server/proxy triplet
 *
 * @param  displayname display name component of the SIP identity "displayname" <sip:username@host>
 * @param  username    username
 * @param  host        the host component of SIP identity username@host corresponding to this virtual line
 *                     if regTimeout != 0 'host' will designate the REGISTRAR server, in this case it may have form of host:port
 *                     otherwise it should be set to IP address or hostname of the local machine
 * @param  proxy       outgoing proxy URI  (all calls using this virtual line will be routed
 *                     through this proxy)
 * @param  regTimeout  registration timeout  (when 0 will NOT use registrations)
 *                     to unergister one should do phDelVline (or phUnregister -- depreciated)
 * @return             -1 in case of error vlid  in case of success
 */
MY_DLLEXPORT int phAddVline(const char* username, const char *host, const char*  proxy, OWPL_TRANSPORT_PROTOCOL transport, int regTimeout);
MY_DLLEXPORT int phAddVline2(const char* displayname, const char* username, const char *host, const char*  proxy, OWPL_TRANSPORT_PROTOCOL transport, int regTimeout);

/**
 * Delete virtual line
 *  This will cause REGISTER request with timeout=0 to be sent to server if needed
 *
 * @param  vlid        Virual line id to remove
 * @param  regTimeout  0: no unregister has to be done, -1: timeout unchanged, or new unregister timeout
 * @param  skipUnregister  when true, avoid sending unregister request, simply delete the vline
 * @return             0 in case of success
 */
MY_DLLEXPORT int phDelVline(int vlid, int regTimeout);
MY_DLLEXPORT int phDelVline2(int vlid, int regTimeout, int skipUnregister);

/**
 * Place an outgoing call using given virtual line
 *
 * @param vlid         virtual line id to use for the call
 * @param uri          call destination address
 * @param userData     application specific data
 * @param rcid         call id of the original call (MUST BE ZERO)
 * @return             if positive the call id else error indication
 */
MY_DLLEXPORT int phLinePlaceCall(int vlid, const char *uri, void *userData, int rcid);
MY_DLLEXPORT int phLinePlaceCall2(int vlid, const char *uri, void *userData, int rcid, int streams);
int phLinePlaceCall_withCa(int vlid, const char *uri, void *userdata, int rcid, int streams, struct phcall *ca0);

/**
 * Accept an incoming a call.
 *
 * @param cid          call id of call.
 * @return             0 in case of success
 */
MY_DLLEXPORT int phAcceptCall2(int cid, void *userData);
MY_DLLEXPORT int phAcceptCall3(int cid, void *userData, int streams);
#define phAcceptCall(cid) phAcceptCall2(cid, 0)

/**
 * Reject the incoming call.
 *
 * @param cid          call id of call.
 * @param reason       SIP reason code for the rejection
 *                     suggested values: 486 - Busy Here, 488 - Not acceptable here
 * @return             0 in case of success
 */
MY_DLLEXPORT int phRejectCall(int cid, int reason);

/**
 * Signal ringing event to the remote side.
 *
 * @param cid          Call id of call.
 * @return             0 in case of success
 */
MY_DLLEXPORT int phRingingCall(int cid);

/**
 * Terminate a call.
 *
 * @param cid          call id of call.
 * @return             0 in case of success
 */
MY_DLLEXPORT int phCloseCall(int cid);

/**
 * Perform a blind call transfer
 *
 * @param cid          call id of call.
 * @param uri          call transfer destination address
 * @return             txid  used in the subsequent transferProgress callback
 */
MY_DLLEXPORT int phBlindTransferCall(int cid, const char *uri);

/**
 * Perform an assisted  call transfer
 *
 * @param cid          call id of call.
 * @param targetCid    call id of the destination call
 * @return             txid  used in the subsequent transferProgress callback
 */
MY_DLLEXPORT int phTransferCall(int cid, int targetCid);

/**
 * Resume previously held call
 *
 * @param cid          call id of call.
 * @return             0 in case of success
 */
MY_DLLEXPORT int phResumeCall(int cid);

/**
 * Put a call on hold
 *
 * @param cid          call id of call.
 * @return             0 in case of success
 */
MY_DLLEXPORT int phHoldCall(int cid);

/**
 * Configure follow me address  for a virtual line.
 * All incoming calls on this line  will be redirected to this address
 *
 * @param uri          destination of the forwarding
 * @return             0 in case of success
 */
MY_DLLEXPORT int phLineSetFollowMe(int vlid, const char *uri);

/**
 * Set busy mode for a virtual line
 * When activated all incoming calls will be answerd by busy signal
 *
 * @param busyFlag          when 0 busy mode is deactivated else activated
 * @return             0 in case of success
 */
MY_DLLEXPORT int phLineSetBusy(int vlid, int busyFlag);



/********************CHAT & PRESENCE*********************/

/**
 * Send a MESSAGE request
 *
 * @param vlid    virtual line id
 * @param to      uri to put in the To: header
 * @param message  message to send
 * @param mime	message mime type
 * @return  if positive msgid
*/
MY_DLLEXPORT int phLineSendMessage(int vlid, const char *uri,
								   const char *buff, const char *mime);

/*
   winfo = 0 -> subscribe with event = presence
   winfo = 1 -> subscribe with event = presence.winfo
*/

/**
 * Subscribe to presence
 *
 * @param vlid virtual line id
 * @param to subscribe to
 * @param winfo message to send
 * @return 0 if success else -1
*/
MY_DLLEXPORT int phLineSubscribe(int vlid, const char *to, const int winfo);

/**
 * Publish my presence
 *
 * @param vlid virtual line id
 * @param to subscribe to
 * @param winfo message to send
 * @param content_type content type ("application/pidf+xml", for example)
 * @param content content
 * @return 0 if success else -1
*/
MY_DLLEXPORT int phLinePublish(int vlid, const char *to, const int winfo, const char * content_type, const char * content);


MY_DLLEXPORT int phAcceptSubscribe(int vlid, int sid, int status, int online, const char* note);
#define phRejectSubscribe(vlid, sid, code) phAcceptSubscribe(vlid, sid, code, 0, 0)



/********************AUDIO*********************/

/**
 * Send a DTMF to remote party
 *
 * @param cid          call id of call.
 * @param dtmfChar     DTMF event to send
 *                     ('0','1','2','3','4','5','6','7','8','9','0','#','A','B','C','D','!')
 * @mode               bitmask specifying DTMF geneartion mode
 *                     INBAND - the DTMF signal is mixed in the outgoing
 *                     RTPPAYLOAD - the DTMF signal will be sent using telephone_event RTP payload
 * @return             0 in case of success
 */
#define PH_DTMF_MODE_INBAND 1
#define PH_DTMF_MODE_RTPPAYLOAD 2
#define PH_DTMF_MODE_ALL 3
MY_DLLEXPORT int phSendDtmf(int cid, int dtmfChar, int mode);

/**
 * Play a sound file
 *
 * @param fileName     file to play (the file externsion will determine the codec to use
 *                     .sw - 16bit signed PCM, .ul - uLaw, .al - aLaw, .gsm - GSM, (.wav on Windows)
 * @param loop         when TRUE play the file in loop
 * @return             0 in case of success
 */
MY_DLLEXPORT int phPlaySoundFile(const char *fileName , int loop);

/**
 * Stop playing a sound file
 *
 * @return             0 in case of success
 */
MY_DLLEXPORT int phStopSoundFile( void );

/**
 * Mix a sound file into the outgoing network audio stream
 *
 * @param cid          call id
 * @param fileName     file to play - for the moment only
 *                      RAW audio files containing 16Bit signed PCM sampled at 16KHZ are supported
 * @return             0 in case of success
 */
MY_DLLEXPORT int phSendSoundFile(int cid, const char *fileName);

/**
 * Set speaker volume
 *
 * @param      cid       call id (-1 for general volume, -2 for playing sounds)
 * @param      volume    0 - 100
 * @return             0 in case of success
 */
MY_DLLEXPORT int phSetSpeakerVolume(int cid,  int volume);

/**
 * Set recording level
 *
 * @param      cid - call id (-1 for general recording level)
 * @param      level    0 - 100
 * @return             0 in case of success
 */
MY_DLLEXPORT int phSetRecLevel(int cid,  int level);

/**
 * Change Audio devices
 * change the audio devices used by phApi. The change will take effect for
 * the new calls only.
 *
 * @param  devstr    the same value as in phcfg.audio_dev
 * @return           0 on success, or error code
 *
 */
MY_DLLEXPORT int phChangeAudioDevices(const char *devstr);


/********************CONFERENCING*********************/

/*
 *  conferencing APIs (not yet finished)
 *
 *  These Fuctions and callback events are avaialable to the confernce intiator application
 *  The conference members are for the moment completely unaware of the fact that they are participating
 *  in a conf call
 *
 */

/**
 * Create a conference
 * Given a call id transform this call into a conference
 * In case of success this API will provoke a deliverey of phCONFCREATED event, following
 * by the phCONFJOINED event with memberCid = cid
 *
 * @param   cid  call id to tranform into the conference
 * @return        if positive conference id  else error code
 *
 */
MY_DLLEXPORT int phConfCreate(int cid);

/**
 * Invite a party to a conference
 * This call is more or less equivalent to phLinePlaceCall followed by phConfAddMember
 *
 * @param   cid  call id to tranform into the conference
 * @return        callid of the created call or error code
 *
 */
MY_DLLEXPORT int phLineConfInvite(int vlid, int cfid, const char *uri);

/**
 * Add a given call to the given conference
 * This will provoke phCONFJOINED event or phCONFJOINERROR event
 *
 * @param    cfid conference id
 * @param    cid  call id to add ito the conference
 * @return   0 or error code
 *
 */
MY_DLLEXPORT int phConfAddMember(int cfid, int callid);

/**
 * Remove a call from the conference
 * This will provoke phCONFLEFT event
 * The removed call will be moved to LOCALHOLD state, if the application wish to terminate it
 * it should call phCloseCall explicitly
 *
 * @param    cfid conference id
 * @param    cid  call id to remove from the conference
 * @return   0 or error code
 *
 */
MY_DLLEXPORT int phConfRemoveMember(int cfid, int cid);

/**
 * Close the conference
 * This will provoke phCONFCLOSED event
 * all member calls will be moved to LOCALHOLD state
 *
 * @param    cfid conference id
 * @return   0 or error code
 *
 */
MY_DLLEXPORT int phConfClose(int cfid);


/* Conference Test: simple implementation */

/**
 * Start conferencing btewwen to calls
 * This will start mixing of audio streams coming from two correspodents
 * @param    cid1 first call id to mix
 * @param    cid2 second call id to mix
 * @return   0 or error code
 *
 */
MY_DLLEXPORT int phConf(int cid1, int cid2);

/**
 * Stop  conferencing between to calls
 * This will stop mixing of audio streams coming from two correspodents
 * @param    cid1 first call id to mix
 * @param    cid2 second call id to mix
 * @return   0 or error code
 *
 */
MY_DLLEXPORT int phStopConf(int cid1, int cid2);





/********************UTILS*********************/

/**
 * Set value of SIP "Contact:"  associated with given virtual line
 * @param    vlid virtual line id
 * @param    uri value of the "Contact:" header
 * @return   0 or error code
 *
 */
MY_DLLEXPORT  int phSetContact(int vlid, const char *uri);


/**
 * Return the SIP address associated to the given virtual ine id
 *
 * @param vlid   --   virtual line id
 * @param buf     --  the buffer that receive the SIP address
 * @param bufsize -- the size of the buffer
 * @return --  0 in case of success
*/
MY_DLLEXPORT int phLineGetSipAddress(int vlid, char buf[], int bufsize);


/*Deprecated, use phLineSendOptions instead*/
int
phSendOptions(int vlid, const char *from, const char *uri);

/**
 * Send an OPTIONS packet send OPTIONS packet using given destination over given
 * virtual line
 *
 * @param vlid  --  vlid
 * @param to    --  uri to put in the To: header
*/
MY_DLLEXPORT int phLineSendOptions(int vlid, const char *to);

/**
 * Return the codecs used for the given call
 *
 * @param   cid  call id in question
 * @param   audioCodecBuf  buffer to return audio codec used (or NULL)
 * @param   aBufLen  size of audioCodecBuf
 * @param   videoCodecBuf  buffer to return video codec used (or NULL)
 * @param   vBufLen  size of videoCodecBuf
 * @return  0 or error code
 *
 */

MY_DLLEXPORT int phCallGetCodecs(int cid, char *audioCodecBuf, int aBufLen, char *videoCodecBuf, int vBufLen);



/**
 * @brief Try to crash the application
*/
MY_DLLEXPORT int phCrash();


/********************STRUCTS*********************/

/**
 * @enum phCallStateEvent
 * @brief call progress events.
 *
 */
enum  phCallStateEvent {
	phDIALING, phRINGING, phNOANSWER, phCALLBUSY,
	phCALLREDIRECTED, phCALLOK,	phCALLHELD,
	phCALLRESUMED, phHOLDOK, phRESUMEOK, phINCALL,
	phCALLCLOSED, phCALLERROR, phDTMF, phXFERPROGRESS,
	phXFEROK, phXFERFAIL, phXFERREQ, phCALLREPLACED,
	phRINGandSTART, phRINGandSTOP, phCALLCLOSEDandSTOPRING
};
/**
 * @struct phCallStateInfo
 */
struct phCallStateInfo {
  enum phCallStateEvent event;
  void *userData;              /*!< used to match placeCall with callbacks */
  const char *localUri;        /*!< valid for all events execpt CALLCLOSED and DTMF */
  int   newcid;                /*!< valid for CALLREPLACED and XFERREQ */
  int   vlid;                  /*! virtual line id */
  int   streams;               /*!  proposed (for phINCALL) and active (for other events) streams for the call */
  union {
    const char  *remoteUri;    /*!< valid for all events execpt CALLCLOSED, DTMF and CALLERROR */
    int   errorCode;           /*!< valid for CALLERROR */
    int   dtmfDigit;           /*!< valid for DTMF */
  } u;
};
typedef struct phCallStateInfo phCallStateInfo_t;


/**
 * @enum phMsgEvent
 */
enum phMsgEvent {
    phMsgNew, phMsgOk, phMsgError
};
/**
 * @struct phMsgStateInfo
 */
struct phMsgStateInfo {
  enum phMsgEvent event;
  int   status;
  const char *from;
  const char *to;
  const char *ctype;
  const char *subtype;
  const char *content;
};
typedef struct phMsgStateInfo  phMsgStateInfo_t;


/**
 * @enum phSubscriptionEvent
 */
enum phSubscriptionEvent {
  phSubscriptionOk, phSubscriptionErrNotFound, phSubscriptionError,
  phSubscriptionIn,   phSubscriptionInUpdate,   phSubscriptionInClosed
};
/**
 * @struct phSubscriptionStateInfo
 */
struct phSubscriptionStateInfo  {
	enum phSubscriptionEvent event;
	int status;
	const char *from;
	const char *to;
        int timeout;
        const char *evtType;

};
typedef struct phSubscriptionStateInfo  phSubscriptionStateInfo_t;


/**
 * @struct phVideoFrameReceivedEvent
 */
struct phVideoFrameReceivedEvent {
	struct _piximage *frame_remote;
	struct _piximage *frame_local;
};
typedef struct phVideoFrameReceivedEvent  phVideoFrameReceivedEvent_t;


/**
 * @enum phConfEvent
 */
enum phConfEvent {
  phCONFCREATED,   /* conference is created */
  phCONFJOINED,    /* memeber joined a the conferences */
  phCONFLEFT,      /* member left a conference */
  phCONFCLOSED,     /* coneference closed       */
  phCONFJOINERROR,   /* error joining a member to a conference */
  phCONFERROR       /* generic error */
};
/**
 * @enum phConfStateInfo
 */
struct phConfStateInfo {
  int  confEvent;
  int  memberCid;  /* call id's for the calls participating in the conference */
                   /* valid for CONFJOINED,CONFLEFT,CONFJOINERROR events      */
  int  errorCode;
};
typedef struct phConfStateInfo phConfStateInfo_t;

/**
 * @struct phVideoConfig
 */
struct phVideoConfig {
#define PHAPI_VIDEO_LINE_128KBPS	1
#define PHAPI_VIDEO_LINE_256KBPS	2
#define PHAPI_VIDEO_LINE_512KBPS	3
#define PHAPI_VIDEO_LINE_1024KBPS	4
#define PHAPI_VIDEO_LINE_2048KBPS	5
#define PHAPI_VIDEO_LINE_AUTOMATIC	6
	int video_fps;
	int video_camera_flip_frame;
	int video_max_frame_size;
	int video_webcam_capture_width; /** width x height for capture must be given. 320x240 is a good guess */
	int video_webcam_capture_height;
	int video_line_configuration;
	int video_codec_max_bitrate;
	int video_codec_min_bitrate;
	char video_device[256];
};

/**
 * @struct phVideoCodecConfig
 * @brief temporary structure that holds codec config, to be set from GUI
 */
struct phVideoCodecConfig {
	int minrate;
	int maxrate;
	int gopsize;
	int qmin;
	int qmax;
	float b_offset;
	float b_factor;
	float i_offset;
	float i_factor;
	int compress;
	int max_b_frame;
	int f_quality;
};

typedef struct phTransferStateInfo phTransferStateInfo_t;
typedef struct phRegStateInfo phRegStateInfo_t;
typedef void (*phFrameDisplayCbk)(int cid, phVideoFrameReceivedEvent_t *ev);



/********************MAIN*********************/

/**
 * @struct phCallbacks
 * @brief  callbacks to the MMI
 */
struct phCallbacks {
  void  (*callProgress)(int cid, const phCallStateInfo_t *info);       /*!< call progress callback routine */
  void  (*transferProgress)(int cid, const phTransferStateInfo_t *info); /*!< transfer progress callback routine */
  void  (*confProgress)(int cfid, const phConfStateInfo_t *info);        /*!< conference progress callback routine */
  void  (*regProgress)(int regid, int regStatus);                       /*!< registration status (0 - OK, else SIP error code */
  void  (*msgProgress)(int mid,  const phMsgStateInfo_t *info);
  void  (*onNotify) (const char* event, const char* from, const char* content);
  void  (*subscriptionProgress)(int sid,  const phSubscriptionStateInfo_t *info);
  phFrameDisplayCbk onFrameReady;
  void  (*errorNotify) (enum phErrors error);
  void  (*debugTrace) (const char * message);
};
typedef struct phCallbacks phCallbacks_t;

/**
 * @var phcb
 * @brief pointer to callback structure
 *
 */
MY_DLLEXPORT extern phCallbacks_t *phcb;


void DEBUGTRACE(const char * mess);

/**
 * Initilize phApi
 *
 * @param cbk          pointer to callback descriptor
 * @param server       string containing an ip address of the phApi server
 *                     (ignored when in direct link mode)
 * @param asyncmode    when != 0 a thread will be created to deliver
 *                     callbacks asyncronously, othewise the client
 *                     is supposed to call phPoll periodically to get
 *                     phApi events delivered
 *                     In DIRECT link mode this parameter is copied to the phcfg.asyncmode structure
 *                     in client/server mode this parameter has client local meaning.
 */
MY_DLLEXPORT int phInit(phCallbacks_t *cbk, char *server, int asyncmode);

/**
 *  Terminate phApi
 */
MY_DLLEXPORT void phTerminate( void );

/**
 * Add authentication info
 * the given info will be to send as authentication information
 * when server request it.
 *
 * @param  username    username which will figure in the From: headers
 *                     (usually the same as userid)
 * @param  userid      userid field value
 * @param  realm       authentication realm
 * @param  passwd      password correspoinding to the userid
 * @return             0 in case of success
 */
MY_DLLEXPORT int phAddAuthInfo(const char *username, const char *userid,
	      const char *passwd, const char *ha1,
	      const char *realm);

/**
  @var phIsInitialize
  @brief 1 : if phInit has been called and phTerminate has not been called
	 	 0 : Otherwise
*/
MY_DLLEXPORT extern int phIsInitialized;

/**
 * Get the version of the phAPI module
 *
 * @return  encoded value corresponding to Version.Subversion.Release
 *
 */
MY_DLLEXPORT int phGetVersion(void);

/**
 * poll for phApi events
 */
MY_DLLEXPORT int phPoll( void );

/**
 * variable storing the name of the log file
 */
MY_DLLEXPORT extern char *phLogFileName;

/**
 * Debugging level (between 0 and 9)
 */
MY_DLLEXPORT extern int phDebugLevel;

/**
 * Set debugging level (between 0 and 9)
 */
MY_DLLEXPORT void phSetDebugLevel(int);

/**
 * Internal function. Do a register for all virtual lines
 */
MY_DLLEXPORT void phRefresh(void);

/**
 * In the case of rpc mode, the phapi server port
 */
MY_DLLEXPORT extern unsigned short phServerPort;


#define phRegister(u, s) phRegister2(u, s, 3600)
#define phUnregister(u, s) phRegister2(u, s, 0)
#define PH_UNREG_MASK               0x8000            /* this mask is ored with regStatus to distingush REGISTER from UNREGISTER */

#define phRelease(v) ((v) & 0xff)
#define phSubversion(v) ((v >> 8) & 0xff)
#define phVersion(v) ((v >> 16) & 0xff)


/********************VIDEO*********************/

MY_DLLEXPORT int  phVideoControlChangeFps(int callid, int fps);
MY_DLLEXPORT int  phVideoControlChangeQuality(int callid, int quality);
MY_DLLEXPORT int  phVideoControlSetCameraFlip(int flip);
MY_DLLEXPORT int  phVideoControlSetWebcamCaptureResolution(int width, int height);
MY_DLLEXPORT int  phVideoControlSetBitrate(int callid, int maxrate, int minrate);
MY_DLLEXPORT void phVideoControlCodecSet(int, struct phVideoCodecConfig *);
MY_DLLEXPORT void phVideoControlCodecGet(int, struct phVideoCodecConfig *);

/*****************FILE-TRANSFER****************/
// <ncouturier>
/** DEPRECATED, no replacement
* Gets the virtual line id associated to a user_id
*
* @param	[in]	user_id : ?
* @param	[in]	alt_id : ?
* @return	the virtual line id
*/
MY_DLLEXPORT int ph_get_vlid(const char * user_id, const char * alt_id);

/** DEPRECATED, Use owplLineGetUserName
* Gets the local username
*
* @param	[in]	vlid : a virtual line id
* @return	the local username
*/
MY_DLLEXPORT char * ph_get_username(int vlid);

/** DEPRECATED: Use owplCallConnectWithBody instead
*
* Generic PhApi service. Sends an invite with a custom body
*
* @param	[in]	vlid : a virtual line id
* @param	[in]	userdata : ?
* @param	[in]	uri : the destination uri (ex : "<sip:user@domain>")
* @param	[in]	bodytype : the type of body message (ex : "type/subtype")
* @param	[in]	body : the custom message body
* @param	[in]	call_id : the call id
* @return	TRUE if succeeds; FALSE else
*/
MY_DLLEXPORT int phInvite(int vlid, void *userdata, char * uri, const char * bodytype, const char * body, int * call_id);

/**
* Generic PhApi service. Creates a new call in PhApi.
*
* @param	[in]	cid : a call id
* @param	[in]	did : a dialog id
* @param	[in]	local_uri : ?
* @param	[in]	req_uri : ?
* @return	TRUE if succeeds; FALSE else
*/
MY_DLLEXPORT int phNewCall(int cid, int did, const char * local_uri, const char * req_uri);

/** DEPRECATED,no replacement is needed
* Generic PhApi service. Stops the state ringing of a call.
*
* @param	[in]	call_id : a call id
* @return	TRUE if succeeds; FALSE else
*/
MY_DLLEXPORT int phStopRinging(int call_id);

/** DEPRECATED, use owplCallAnswerWithBody
* Generic PhApi service. Sends a 200OK with a custom body
*
* @param	[in]	cid : the call id
* @param	[in]	bodytype : the type of body message (ex : "type/subtype")
* @param	[in]	body : the custom message body
* @return	TRUE if succeeds; FALSE else
*/
MY_DLLEXPORT int phAccept(int cid, const char * bodytype, const char * body);

/** [DEPRECATED], use owplCallReject instead
* Generic PhApi service. Sends a 486 BUSY
*
* @param	[in]	cid : the call id
* @return	TRUE if succeeds; FALSE else
*/
MY_DLLEXPORT int phReject(int cid);

/** *DEPRECATED* , use owplCallDisconnect instead!
* Generic PhApi service. Sends a CANCEL, DECLINE or a BYE that must be sent
*
* @param	[in]	call_id : the call id
* @return	TRUE if succeeds; FALSE else
*/
MY_DLLEXPORT int phBye(int call_id);

/** [DEPRECATED], use owplCallDisconnect instead

* Generic PhApi service. Is the same as a phBye, because the function eXosip_terminate_call() used inside decides
* wether it is a CANCEL, DECLINE or a BYE that must be sent
*
* @param	[in]	call_id : the call id
* @return	TRUE if succeeds; FALSE else
*/
MY_DLLEXPORT int phCancel(int sid);

/** [DEPRECATED], use owplCallDisconnect instead

* Generic PhApi service.
*
* @param	[in]
* @return	TRUE if succeeds; FALSE else
*/
MY_DLLEXPORT int phEndCall(int call_id, int status_code);

/** DEPRECATED  use owplCallRejectWithPredefinedReason
* Generic PhApi service.
*
* @param	[in]	call_id : the call id
* @param	[in]	status_code : the status code of the originating eXosip event
* @return	TRUE if succeeds; FALSE else
*/
MY_DLLEXPORT int phRequestFailure(int call_id, int status_code);

/** DEPRECATED  use owplCallRejectWithPredefinedReason
* Generic PhApi service.
*
* @param	[in]	call_id : the call id
* @param	[in]	status_code : the status code of the originating eXosip event
* @return	TRUE if succeeds; FALSE else
*/
MY_DLLEXPORT int phServerFailure(int call_id, int status_code);

/** DEPRECATED  use owplCallRejectWithPredefinedReason
* Generic PhApi service.
*
* @param	[in]	call_id : the call id
* @param	[in]	status_code : the status code of the originating eXosip event
* @return	TRUE if succeeds; FALSE else
*/
MY_DLLEXPORT int phGlobalFailure(int call_id, int status_code);

/** DEPRECATED  use owplCallRejectWithPredefinedReason
* Generic PhApi service.
*
* @param	[in]	call_id : the call id
* @param	[in]	status_code : the status code of the originating eXosip event
* @return	TRUE if succeeds; FALSE else
*/
MY_DLLEXPORT int phNoAnswer(int call_id, int status_code);


//DEPRECATED
MY_DLLEXPORT int phHoldOn(int call_id, const char * bodytype);

// DEPRECATED
MY_DLLEXPORT int phHoldOff(int call_id, const char * bodytype);

// </ncouturier>

/********************CONFIG*********************/

/**
 * @struct phConfig
 * @brief ph API configuration info
 */
#define VAD_VALID_MASK 0x80000000
#define VAD_THRESHOLD_MASK 0x7fffffff

// SPIKE_HDX:different configuration modes are available
enum PH_HDX_MODES {
  PH_HDX_MODE_MIC = 1,   /*!< Half Duplex mode where microphone signal has priority over speaker signal */
  PH_HDX_MODE_SPK = 2    /*!< Half Duplex mode where speaker signal has priority over microphone signal */
};


#ifdef EMBED
struct phConfig {
  char local_rtp_port[16]; /*!< port number used for RTP data */ /* range 1st - last */
  char local_rtcp_port[16]; /*!< port number used for RTCP data */
  char sipport[16];         /*!< sip port number */
  char identity[256];       /*!< DEPRECIATED! Use Virtual Lines instead.  my sip address (this field is temporary hack) */
  char audio_codecs[128];         /*!< comma separate list of codecs in order of priority */
  char video_codecs[128];         /*!< comma separate list of codecs in order of priority */
                            /* example: PCMU,PCMA,GSM,ILBC,SPEEX   */
  int  asyncmode;           /*!< when true phApi creates a separate eXosip polling thread... in client/server mode MUST be TRUE */
  char audio_dev[64];       /*!< audio device identifier */
			    /* example: IN=2 OUT=1 ; 2 is input device and 1 is ouput device */
  int softboost;            /* to be removed */
  int nomedia;
  int noaec;		    /* when non-zero - disable aec */
  unsigned int vad;         /* if bit31=1  DTX/VAD features activated and bits0-30 contains the power threshold */
  int cng;                  /* if 1,  CNG feature will be negotiated */
  int nat_refresh_time;       /* timeout for sip address/port refresh (when 0 no-refresh) */
  int jitterdepth;           /* jitter buffer depth in miliseconds (if 0 default of 60 msecs is used) */
  int nodefaultline;         /* temporary hack for implementing backward compatibility... Don't touch it */
  int autoredir;            /*!< when NONZERO the redirect requests will be automatically executed by phApi
			      the new CID will be deliverd in newcid field  in the CALLREDIRECTED event */

#define PH_TUNNEL_AUTOCONF 2
#define PH_TUNNEL_USE  1
  int use_tunnel;

  char httpt_server[128];
  int  httpt_server_port;
  char http_proxy[128];
  int  http_proxy_port;
  char http_proxy_user[128];
  char http_proxy_passwd[128];

 char video_codecs[128];         /*!< comma separate list of codecs in order of priority */
 char local_video_rtp_port[16]; /*!< port number used for video RTP data */
 char local_video_rtcp_port[16]; /*!< port number used for video RTCP data */
};
#else
struct phConfig {
  char local_rtp_port[16];        /*!< port number used for RTP data */
  char local_audio_rtcp_port[16]; /*!< port number used for RTCP data */
  char local_video_rtp_port[16];  /*!< port number used for video RTP data */
  char local_video_rtcp_port[16]; /*!< port number used for video RTCP data */

  char identity[256];       /*!< DEPRECIATED! Use Virtual Lines instead.  my sip address (this field is temporary hack) */
  char audio_codecs[128];   /*!< comma separate list of codecs in order of priority */
  char video_codecs[128];   /*!< comma separate list of codecs in order of priority */
                            /* example: PCMU,PCMA,GSM,ILBC,SPEEX   */

  int  asyncmode;           /*!< when true phApi creates a separate eXosip polling thread... in client/server mode MUST be TRUE */
  char audio_dev[64];       /*!< audio device identifier */
			                /* example: IN=2 OUT=1 ; 2 is input device and 1 is ouput device */
  int softboost;            /* to be removed */
  int nomedia;
  int noaec;				/* when non-zero - disable aec */
  unsigned int vad;         /* if bit31=1  DTX/VAD features activated and bits0-30 contains the power threshold */
  int cng;                  /* if 1,  CNG feature will be negotiated */

  // SPIKE_HDX: setting of hdxmode in phconfig
  int hdxmode;              /* if 0, half duplex mode is desactivated. otherwise check enum PH_HDX_MODES */

  // SPIKE_SPIKE_SIP_SIMPLE: enable or disable presence and IM
  int pim_disabled;			/* if 1, disable presence & instant message functions, otherwise enable them*/

  int nat_refresh_time;     /* timeout for sip address/port refresh (when 0 no-refresh) */
  int jitterdepth;          /* jitter buffer depth in miliseconds (if 0 default of 60 msecs is used) */
  int nodefaultline;        /* temporary hack for implementing backward compatibility... Don't touch it */
  int autoredir;            /*!< when NONZERO the redirect requests will be automatically executed by phApi
			                   the new CID will be deliverd in newcid field  in the CALLREDIRECTED event */
#ifdef WIN32
  HWND videoHandle;
#endif

  struct phVideoConfig video_config;
  char  plugin_path[256];  /*!< where to look for plugin modules */
};
#endif
typedef struct phConfig phConfig_t;

/**
 * @var phconfig
 * @brief variable storing the ph API configuration
 */
MY_DLLEXPORT extern phConfig_t phcfg;

MY_DLLEXPORT phConfig_t *phGetConfig();

/* JT */
#ifdef QOS_DEBUG_ENABLE
MY_DLLEXPORT void phrtcp_QoS_enable_rtcp_report(int ToF);
MY_DLLEXPORT void phrtcp_report_set_cb(jt_rtcpCallbacks_t *cbk);
MY_DLLEXPORT int phrtcp_report_begin();
MY_DLLEXPORT int phrtcp_report_end();
#endif /* QOS_DEBUG_ENABLE */

#ifdef __cplusplus
}
#endif

/** @} */
#endif
