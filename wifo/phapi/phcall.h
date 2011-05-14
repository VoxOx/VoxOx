#ifndef __PHCALL_H__
#define __PHCALL_H__ 1


#define PH_MAX_CALLS  32

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Forward declaration
 */
struct phConfig;
struct ph_msession_s;
struct eXosip_event;
struct owpl_plugin;



/**
 * @struct memory representation of a call
 */
struct phcall {
	int cid;
	int extern_cid; /* External call ID (eXosip callId for now) */
	int did;
	/* call id to which we're trasferring this call */
	int txcid;
	/* virtual line id */
	int vlid;
	/* SIP uri of the remote contact*/
	char * remote_uri;

	int redirs;
	char remote_sdp_audio_ip[64];
	int  remote_sdp_audio_port;
	char remote_sdp_video_ip[64];
	int  remote_sdp_video_port;
	char audio_payload_name[32];
	int  audio_payload;
	char video_payload_name[32];
	int video_payload;
	int wanted_streams;
	int local_sdp_audio_port;
	int local_sdp_video_port;
	struct phConfig *cfg;
	int  _hasaudio;
	int  isringing; //DEPRECATED. This property will be removed soon. Do not use it anymore.
	int  remotehold;
	/* used by the 'hold call' state machine */
	int  localhold;
	int  localresume;
	int  localrefer;
	int  remoterefer;
#if 0
	void *ph_audio_stream;
#endif
	/* structure holding specific information on the video stream. NULL when the stream is not activated */
	void *ph_video_stream;
	/* Call ID for the call created by REFER */
	int  rcid;
	/* REFER did */
	int  rdid;
	/*  close requested by lower layer */
	int  closereq;
	char cng_name[32];
	/* CNAME Field for RTCP reports */
	char cname[64];
	int  cng;
	/* current negociated media flags for the call */
	int  nego_mflags;
	/* user media flags for the call (flags given by the user when placing or accepting the call) */
	int  user_mflags;
	/* structure holding the description of all the media sessions related to the call */
	struct ph_msession_s *mses;
	/* used by sVoIP to indicate that the call is crypted */
	int iscrypted; 

	struct owpl_plugin * owplPlugin;
	/* used to indicate the call origination */
	int waitaccept;
};

typedef struct phcall phcall_t;

/*
 * phcall functions
 *
 */

phcall_t *ph_locate_call(struct eXosip_event *je, int creatit);
phcall_t *ph_allocate_call(int cid);
phcall_t *ph_locate_call_for_refer(struct eXosip_event *je);

phcall_t *ph_locate_call_by_cid(int cid);
phcall_t *ph_locate_call_by_rcid(int cid);
phcall_t *ph_locate_call_by_rdid(int did);
phcall_t *ph_locate_call_by_remote_uri(char *remote_uri);

#ifdef __cplusplus
}
#endif

#endif
