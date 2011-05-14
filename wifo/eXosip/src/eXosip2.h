/*
  eXosip - This is the eXtended osip library.
  Copyright (C) 2002, 2003  Aymeric MOIZARD  - jack@atosc.org
  
  eXosip is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  eXosip is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#ifdef ENABLE_MPATROL
#include <mpatrol.h>
#endif

#ifndef __EXOSIP2_H__
#define __EXOSIP2_H__

#include <stdio.h>
#include <stdlib.h>

/* TODO: remove this (should be included in OWSL) */
#if defined (_WIN32_WCE) || defined (WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#define snprintf _snprintf
#define close(s) closesocket(s)
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif


#include <osip2/osip.h>
#include <osip2/osip_dialog.h>

#include "ow_osip/ow_osip.h"
#include "ow_osip/osip_negotiation.h"

#include <eXosip/eXosip_cfg.h>
#include <eXosip/eXosip.h>

#include "jpipe.h"

#ifndef JD_EMPTY

#define JD_EMPTY          0
#define JD_INITIALIZED    1
#define JD_TRYING         2
#define JD_QUEUED         3
#define JD_RINGING        4
#define JD_ESTABLISHED    5
#define JD_REDIRECTED     6
#define JD_AUTH_REQUIRED  7
#define JD_CLIENTERROR    8
#define JD_SERVERERROR    9
#define JD_GLOBALFAILURE  10
#define JD_TERMINATED     11

#define JD_MAX            11

#endif

#define EXOSIP_VERSION "eXosip/0.1(wengophone)"


#ifdef __cplusplus
extern "C"
{
#endif

void  eXosip_update(void);
void  __eXosip_wakeup(void);


/**************************
*         BASE 64         *
**************************/

/** @return out_length or a negative value if error */
int base64_encode
(
	const void * in,
	int in_length,
	char * out,
	int out_size,
	int line_size
) ;


/**************************
*         ACCOUNT         *
**************************/

typedef struct OWSIPAccountInfo
{
	OWSIPAccount account ;
	/* display name */
	char * name ;
	/* user name, login */
	char * user ;
	/* realm, server */
	char * domain ;
	/* UDP, TCP, TLS */
	TransportProtocol transport ;
	/* IPv4, IPv6 */
	/* TODO: add address_family in account instead of global ip_family */
	/*OWSLAddressFamily address_family ;*/
	/* IP or DNS */
	char * proxy ;
	/* remote port */
	unsigned int port ;
	/* if false (0), send authentication only after 401/407 response */
	/* for security reasons, it should be true only with TLS */
	int basic_authentication_in_first_message ;
	/* time of the last received message to check activity */
	time_t last_received_message_time ;
	/* maximum time since the last received message (infinite = 0) */
	time_t idle_time_max ;
	/* remove the first route if this one is the same as the destination host */
	/* some servers are disturbed by removing it, so let this setting as an option */
	int route_filter ;
	/* TODO: remove these variables from phAPI vline */
/*	long regTimeout ;
	long lastRegTime ;
	int rid ;
	OWPL_LINESTATE_EVENT LineState ;
	int busy ;
	phVLinePublishInfo publishInfo ;
	phVLineKeepAliveInfo keepAliveInfo ;
*/
	/* plugin for alternative SIP */
	OWSIPAccountMessageFilter message_filter ;
	/* user data for plugin */
	void * message_filter_user_data ;
} OWSIPAccountInfo ;

/* initialize / terminate account pool */
int owsip_account_initialize (void) ;
int owsip_account_terminate (void) ;

/* get an account */
OWSIPAccount owsip_account_get (const char * user, const char * domain) ;
OWSIPAccount owsip_account_get_from_uri (const osip_uri_t * uri) ;
OWSIPAccount owsip_account_get_from_uri_string (const char * uri) ;
OWSIPAccount owsip_account_get_from_received_request (const osip_message_t * request) ;
OWSIPAccount owsip_account_get_from_received_response (const osip_message_t * response) ;
OWSIPAccount owsip_account_get_from_received_message (const osip_message_t * message) ;
/* TODO: remove owsip_account_info_get from header (make it static) */
OWSIPAccountInfo * owsip_account_info_get (OWSIPAccount account) ;

/* get / set account parameter */

char * owsip_account_name_get (OWSIPAccount account) ;

char * owsip_account_user_get (OWSIPAccount account) ;

char * owsip_account_domain_get (OWSIPAccount account) ;

int owsip_account_domain_set (OWSIPAccount account, char * domain) ;

TransportProtocol owsip_account_transport_get (OWSIPAccount account) ;

int owsip_account_transport_set (OWSIPAccount account, TransportProtocol transport) ;

char * owsip_account_proxy_get (OWSIPAccount account) ;

int owsip_account_proxy_set (OWSIPAccount account, char * proxy) ;

unsigned int owsip_account_port_get (OWSIPAccount account) ;

int owsip_account_route_filter_enable (OWSIPAccount account, int enable) ;

int owsip_account_route_filter_is_enabled (OWSIPAccount account) ;

char *
owsip_account_contact_get
(
	OWSIPAccount account,
	char * contact,
	size_t contact_size
) ;

char *
owsip_account_via_get
(
	OWSIPAccount account,
	char * via,
	size_t via_size
) ;

int
owsip_account_basic_authentication_in_first_message_set
(
	OWSIPAccount account,
	int basic_authentication_in_first_message
) ;

time_t
owsip_account_idle_time_get
(
	OWSIPAccount account
) ;

int
owsip_account_idle_time_reset
(
	OWSIPAccount account
) ;

int
owsip_account_idle_time_refresh
(
	OWSIPAccount account
) ;

time_t
owsip_account_idle_time_max_get
(
	OWSIPAccount account
) ;

int
owsip_account_idle_time_max_set
(
	OWSIPAccount account,
	time_t idle_time_max
) ;

int
owsip_account_message_filter_set
(
	OWSIPAccount account,
	OWSIPAccountMessageFilter message_filter,
	void * user_data
) ;

int
owsip_account_message_filter
(
	OWSIPAccount account,
	osip_message_t * message,
	OWSIPDirection direction,
	osip_transaction_t * transaction
) ;


/*************************
*         DIALOG         *
*************************/

typedef struct eXosip_dialog_t eXosip_dialog_t;

struct eXosip_dialog_t {

  int              d_id;
  int              d_STATE;
  osip_dialog_t   *d_dialog;      /* active dialog */

  int             d_timer;
  int			  d_count;
  osip_message_t  *d_200Ok;
  osip_message_t  *d_ack;
  osip_list_t     *media_lines;
  
  osip_list_t     *d_inc_trs;
  osip_list_t     *d_out_trs;

  char             *d_localcontact;
  /* struct _BodyHandler     *d_bh; */

  OWSIPAccount account ;

  eXosip_dialog_t *next;
  eXosip_dialog_t *parent;
};

eXosip_dialog_t * owsip_dialog_get (osip_dialog_t * odialog) ;
OWSIPAccount owsip_dialog_account_get (eXosip_dialog_t * dialog) ;
int owsip_dialog_account_set (eXosip_dialog_t * dialog, OWSIPAccount account) ;


/****************************
*         SUBSCRIBE         *
****************************/

typedef struct eXosip_subscribe_t eXosip_subscribe_t;

struct eXosip_subscribe_t {

  int                 s_id;
  char                s_uri[255];
  int                 s_online_status;
  int                 s_ss_status;
  int                 s_ss_reason;
  time_t               s_ss_expires;
  int		      winfo;
  eXosip_dialog_t    *s_dialogs;

  osip_transaction_t *s_inc_tr;
  osip_transaction_t *s_out_tr;

  eXosip_subscribe_t *next;
  eXosip_subscribe_t *parent;
};


/*************************
*         NOTIFY         *
*************************/

typedef struct eXosip_notify_t eXosip_notify_t;

struct eXosip_notify_t {

  int                 n_id;
  char                n_uri[255];
  int                 n_online_status;
  char                *n_contact_info;

  int                 n_ss_status;
  int                 n_ss_reason;
  int                 n_ss_expires;
  eXosip_dialog_t    *n_dialogs;

  osip_transaction_t *n_inc_tr;
  osip_transaction_t *n_out_tr;

  eXosip_notify_t    *next;
  eXosip_notify_t    *parent;
};


/***********************
*         CALL         *
***********************/

typedef struct eXosip_call_t eXosip_call_t;

struct eXosip_call_t {

  int                      c_id;
  char                     c_subject[100];
  int                      c_ack_sdp; /* flag for alternative SDP offer-response model */
  eXosip_dialog_t         *c_dialogs;
  osip_transaction_t      *c_inc_tr;
  osip_transaction_t      *c_out_tr;
  osip_transaction_t      *c_inc_options_tr;
  osip_transaction_t      *c_out_options_tr;
  void                    *external_reference;
  int					   c_account;

  osip_negotiation_ctx_t  *c_ctx;
  char                     c_sdp_port[10];
  char                     c_video_port[10];
  int			  iscrypted; /* used by sVoIP to indicate if the call is crypted */
  eXosip_call_t           *next;
  eXosip_call_t           *parent;
};


#if 0
typedef struct eXosip_realm_t eXosip_realm_t;

struct eXosip_realm_t {

  int             r_id;

  char           *r_realm;
  char           *r_username;
  char           *r_passwd;

  eXosip_realm_t *next;
  eXosip_realm_t *parent;
};
#endif


/***************************
*         REGISTER         *
***************************/

typedef struct eXosip_reg_t eXosip_reg_t;

struct eXosip_reg_t {

  int             r_id;

  int             r_reg_period;     /* delay between registration */
  char           *r_aor;            /* sip identity */
  char           *r_registrar;      /* registrar */
#if 0
  eXosip_realm_t *r_realms;         /* list of realms */
#endif
  char           *r_contact;        /* list of contacts string */
  char           *r_route;         

  osip_transaction_t  *r_last_tr;
  eXosip_reg_t   *next;
  eXosip_reg_t   *parent;
  int             r_last_status;
  char           *r_cid;
  int             r_seq;

  OWSIPAccount account ;
};

OWSIPAccount owsip_register_account_get (eXosip_reg_t * reg) ;


/**************************
*         MESSAGE         *
**************************/

typedef struct eXosip_msg_t eXosip_msg_t;

struct eXosip_msg_t {

  int             m_id;
  int             m_last_status;
  osip_transaction_t            *m_last_tr;
  eXosip_msg_t   *next;
  eXosip_msg_t *parent;

};


/*********************************
*         AUTHENTICATION         *
*********************************/

typedef struct jauthinfo_t jauthinfo_t;

struct jauthinfo_t {
  char username[50];
  char userid[50];
  char passwd[1000];
  char ha1[50];
  char realm[50];
  jauthinfo_t *parent;
  jauthinfo_t *next;
};

int
__eXosip_create_authorization_header(osip_message_t *previous_answer,
				     const char *rquri, const char *username,
				     const char *passwd,
				     osip_authorization_t **auth);
int
__eXosip_create_proxy_authorization_header(osip_message_t *previous_answer,
					   const char *rquri,
					   const char *username,
					   const char *passwd,
					   osip_proxy_authorization_t **auth);


eXosip_event_t *eXosip_event_init_for_call(int type, eXosip_call_t *jc,
					      eXosip_dialog_t *jd);
int eXosip_event_add_sdp_info(eXosip_event_t *je, osip_message_t *message);

int eXosip_event_add_status(eXosip_event_t *je, osip_message_t *response);
eXosip_event_t *eXosip_event_init_for_subscribe(int type,
						   eXosip_subscribe_t *js,
						   eXosip_dialog_t *jd);
eXosip_event_t *eXosip_event_init_for_notify(int type, eXosip_notify_t *jn,
						eXosip_dialog_t *jd);
eXosip_event_t *eXosip_event_init_for_reg(int type, eXosip_reg_t *jr, osip_message_t *sip);
eXosip_event_t *eXosip_event_init_for_message(int type, osip_transaction_t
					      *tr, osip_message_t *sip);
int eXosip_event_init(eXosip_event_t **je, int type);
eXosip_call_t *eXosip_event_get_callinfo(eXosip_event_t *je);
eXosip_dialog_t *eXosip_event_get_dialoginfo(eXosip_event_t *je);
eXosip_reg_t *eXosip_event_get_reginfo(eXosip_event_t *je);
eXosip_notify_t *eXosip_event_get_notifyinfo(eXosip_event_t *je);
eXosip_subscribe_t *eXosip_event_get_subscribeinfo(eXosip_event_t *je);
int eXosip_event_add(eXosip_event_t *je);
eXosip_event_t *eXosip_event_wait(int tv_s, int tv_ms);
eXosip_event_t *eXosip_event_get(void);

typedef void (* eXosip_callback_t) (int type, eXosip_event_t *);

char *strdup_printf(const char *fmt, ...);

jfriend_t *jfriend_get(void);
jsubscriber_t *jsubscriber_get(void);
jidentity_t *jidentity_get(void);
int jfriend_get_and_set_next_token (char **dest, char *buf,
				    char **next);

#define eXosip_trace(loglevel,args)  do        \
{                       \
	char *__strmsg;  \
	__strmsg=strdup_printf args ;    \
	OSIP_TRACE(osip_trace(__FILE__,__LINE__,(loglevel),NULL,"%s\n",__strmsg)); \
	osip_free (__strmsg);        \
}while (0);

typedef struct eXosip_t eXosip_t;

struct eXosip_t {
	char				*mediaip;		/* use this address for media ip */

	char				*user_agent;

	const char			*subscribe_timeout;
	FILE				*j_input;
	FILE				*j_output;
	eXosip_call_t		*j_calls;		/* my calls        */
	eXosip_subscribe_t	*j_subscribes;	/* my friends      */
	eXosip_notify_t		*j_notifies;	/* my susbscribers */
	osip_list_t			*j_transactions;

	eXosip_reg_t		*j_reg;
	eXosip_msg_t		*j_msgs;

	void				*j_cond;
	void				*j_mutexlock;

	void				*j_socket_mutex;

	osip_t				*j_osip;
	int					j_stop_ua;
	void				*j_thread;
	jpipe_t				*j_socketctl;
	jpipe_t				*j_socketctl_event;

	jsubscriber_t		*j_subscribers;
	jfriend_t			*j_friends;
	jidentity_t			*j_identitys;

	int					j_runtime_mode;
	eXosip_callback_t	j_call_callbacks[EXOSIP_CALLBACK_COUNT];
	osip_fifo_t			*j_events;

	osip_negotiation_t	*osip_negotiation;
	jauthinfo_t			*authinfos;

	/* TODO: remove ip_family */
	OWSLAddressFamily	ip_family;
};

typedef struct jinfo_t jinfo_t;

struct jinfo_t {
  OWSIPAccount account ;
  eXosip_dialog_t     *jd;
  eXosip_call_t       *jc;
  eXosip_subscribe_t  *js;
  eXosip_notify_t     *jn;
  eXosip_msg_t        *jm;
};

int eXosip_guess_ip_for_via (int family, char *address, int size);

int  eXosip_sdp_negotiation_init(osip_negotiation_t **sn);
void eXosip_sdp_negotiation_free(osip_negotiation_t *sn);
#if 0
int eXosip_retrieve_sdp_negotiation_result(osip_negotiation_ctx_t *ctx, char *payload_name,  int pnsize);
#endif

int eXosip_retrieve_sdp_negotiation_result_for(osip_negotiation_ctx_t *ctx, const char *mtype, char *pname,  int pnsize);
int eXosip_retrieve_sdp_negotiation_audio_result(osip_negotiation_ctx_t *ctx, char *payload_name, int pnsize);
int eXosip_retrieve_sdp_negotiation_video_result(osip_negotiation_ctx_t *ctx, char *payload_name,  int pnsize);
int eXosip_get_sdp_media_info(sdp_message_t *sdp, const char *mtype, char *payload_name,  int pnsize, char *ipaddr, int ipsize, int *port, int *ptime);
int eXosip_get_sdp_media_format_info(sdp_message_t *sdp, eXosip_event_t *je);

sdp_message_t *eXosip_get_local_sdp_info(osip_transaction_t *invite_tr);
sdp_message_t *eXosip_get_remote_sdp_info(osip_transaction_t *invite_tr);
sdp_message_t *eXosip_get_local_sdp(osip_transaction_t *transaction);
sdp_message_t *eXosip_get_remote_sdp(osip_transaction_t *transaction);



int    eXosip_set_callbacks(osip_t *osip);
int cb_snd_message (osip_transaction_t * tr, osip_message_t * sip, char *host,
					int port, int out_socket);

char  *osip_call_id_new_random(void);
char  *osip_to_tag_new_random(void);
char  *osip_from_tag_new_random(void);
unsigned int via_branch_new_random(void);
void __eXosip_delete_jinfo(osip_transaction_t *transaction);
jinfo_t *__eXosip_new_jinfo(OWSIPAccount account, eXosip_call_t *jc, eXosip_dialog_t *jd,
			    eXosip_subscribe_t *js, eXosip_notify_t *jn);
OWSIPAccount owsip_transaction_account_get (osip_transaction_t * transaction) ;

int  eXosip_dialog_init_as_uac(eXosip_dialog_t **jd, OWSIPAccount account, osip_message_t *_200Ok);
int  eXosip_dialog_init_as_uas(eXosip_dialog_t **jd, OWSIPAccount account, osip_message_t *_invite, osip_message_t *_200Ok);
void eXosip_dialog_free(eXosip_dialog_t *jd);
void eXosip_dialog_set_state(eXosip_dialog_t *jd, int state);
void eXosip_delete_early_dialog(eXosip_dialog_t *jd);

int eXosip_get_addrinfo (struct addrinfo **addrinfo,
				char *hostname, int service);
int isrfc1918(char *ipaddr);
void eXosip_get_localip_from_via(osip_message_t *,char**localip);

int  generating_initial_publish(osip_message_t **message, char *to,
				 char *from, char *route);

int  generating_initial_subscribe(osip_message_t **message, char *to,
				 char *from, char *route);

int  eXosip_msg_init(eXosip_msg_t **jm);
void eXosip_msg_free(eXosip_msg_t *jm);
int  eXosip_msg_find(int mid, eXosip_msg_t **jm);
int  generating_message(osip_message_t **message, char *to, char *from,
		       char *route, char *buff, char *mime);
int  generating_cancel(osip_message_t **dest, osip_message_t *request_cancelled);
int  generating_options_within_dialog(osip_message_t **info, osip_dialog_t *dialog);
int  generating_info_within_dialog(osip_message_t **info, osip_dialog_t *dialog);
int  generating_bye(osip_message_t **bye, osip_dialog_t *dialog);
int  generating_refer(osip_message_t **refer, osip_dialog_t *dialog, char *refer_to);
int  generating_refer_outside_dialog(osip_message_t **refer, char *refer_to, char *from, char *to, char *proxy);
int  generating_invite_on_hold(osip_message_t **invite, osip_dialog_t *dialog,
				char *subject, char *sdp);
int  generating_invite_off_hold(osip_message_t **invite, osip_dialog_t *dialog,
				char *subject, char *sdp);
int  generating_options(osip_message_t **options, char *from, char *to, char *proxy);
int  generating_ack_for_2xx(osip_message_t **ack, osip_dialog_t *dialog);
int  generating_info(osip_message_t **info, char *from, char *to, char *proxy);



int  eXosip_reg_init(OWSIPAccount account, eXosip_reg_t **jr, char *from, char *proxy, char *route);
void eXosip_reg_free(eXosip_reg_t *jreg);
int  generating_register(osip_message_t **reg, char *transport, char *from, char *proxy, int expires, char *route, int seqnum);
char *generating_sdp_answer(osip_message_t *request, osip_negotiation_ctx_t *context);

int eXosip_call_dialog_find(int jid, eXosip_call_t **jc, eXosip_dialog_t **jd);
int eXosip_notify_dialog_find(int nid, eXosip_notify_t **jn, eXosip_dialog_t **jd);
int eXosip_subscribe_dialog_find(int nid, eXosip_subscribe_t **js, eXosip_dialog_t **jd);
//int eXosip_find_replaced_dialog(osip_message_t *sip,  eXosip_call_t **jc, eXosip_dialog_t **jd);

int eXosip_call_find(int cid, eXosip_call_t **jc);
int eXosip_dialog_set_200ok(eXosip_dialog_t *_jd, osip_message_t *_200Ok);

int _eXosip2_answer_invite_3456xx(eXosip_call_t *jc, eXosip_dialog_t *jd, int code, osip_message_t **answer);
int _eXosip2_answer_invite_2xx(eXosip_call_t *jc, eXosip_dialog_t *jd, int code, osip_message_t **answer);
int _eXosip2_answer_invite_1xx(eXosip_call_t *jc, eXosip_dialog_t *jd, int code, osip_message_t **answer);
int eXosip_answer_invite_1xx(eXosip_call_t *jc, eXosip_dialog_t *jd, int code, char *ctct);
int eXosip_answer_invite_2xx(eXosip_call_t *jc, eXosip_dialog_t *jd, int code, char *local_sdp_port, char *ctct, 
			     char *local_video_port,char *public_sdp_port,  char *public_video_port);
int eXosip_answer_invite_2xx_with_body(eXosip_call_t *jc, eXosip_dialog_t *jd, int code,const char*, const char*);
int eXosip_answer_invite_3456xx(eXosip_call_t *jc, eXosip_dialog_t *jd, int code, const char *contact);
int eXosip_answer_options_1xx(eXosip_call_t *jc, eXosip_dialog_t *jd, int code);
int eXosip_answer_options_2xx(eXosip_call_t *jc, eXosip_dialog_t *jd, int code);
int eXosip_answer_options_3456xx(eXosip_call_t *jc, eXosip_dialog_t *jd, int code);
void eXosip_notify_answer_subscribe_1xx(eXosip_notify_t *jc,
					eXosip_dialog_t *jd, int code);
void eXosip_notify_answer_subscribe_2xx(eXosip_notify_t *jn,
					eXosip_dialog_t *jd, int code);
void eXosip_notify_answer_subscribe_3456xx(eXosip_notify_t *jn,
					   eXosip_dialog_t *jd, int code);


int
_eXosip_answer_refer_123456xx(eXosip_call_t *jc, eXosip_dialog_t *jd, int code);


int eXosip_build_response_default(int jid, int status);
int _eXosip_build_response_default(osip_message_t **dest, osip_dialog_t *dialog,
				  int status, osip_message_t *request);
int complete_answer_that_establish_a_dialog(osip_message_t *response, osip_message_t *request);
int complete_answer_that_establish_a_dialog2(osip_message_t *response, osip_message_t *request, char *ctct);
int _eXosip_build_request_within_dialog(osip_message_t **dest, char *method_name,
				       osip_dialog_t *dialog);
int _eXosip_build_request_within_dialog2(osip_message_t **dest, char *method_name,
					 osip_dialog_t *dialog, int seqnum);

int eXosip_build_initial_options(osip_message_t **options, char *to, char *from,
				 char *route);

void eXosip_kill_transaction(osip_list_t * transactions);
int eXosip_remove_transaction_from_call(osip_transaction_t *tr, eXosip_call_t *jc);
osip_transaction_t *eXosip_find_last_inc_notify(eXosip_subscribe_t *jn, eXosip_dialog_t *jd);
osip_transaction_t *eXosip_find_last_out_notify(eXosip_notify_t *jn, eXosip_dialog_t *jd);
osip_transaction_t *eXosip_find_last_inc_subscribe(eXosip_notify_t *jn, eXosip_dialog_t *jd);
osip_transaction_t *eXosip_find_last_out_subscribe(eXosip_subscribe_t *js, eXosip_dialog_t *jd );

osip_transaction_t *eXosip_find_last_out_options(eXosip_call_t *jc, eXosip_dialog_t *jd );
osip_transaction_t *eXosip_find_last_inc_options(eXosip_call_t *jc, eXosip_dialog_t *jd );
osip_transaction_t *eXosip_find_last_options(eXosip_call_t *jc, eXosip_dialog_t *jd );
osip_transaction_t *eXosip_find_last_out_info(eXosip_call_t *jc, eXosip_dialog_t *jd );
osip_transaction_t *eXosip_find_last_inc_info(eXosip_call_t *jc, eXosip_dialog_t *jd );
osip_transaction_t *eXosip_find_last_info(eXosip_call_t *jc, eXosip_dialog_t *jd );
osip_transaction_t *eXosip_find_last_invite(eXosip_call_t *jc, eXosip_dialog_t *jd );
osip_transaction_t *eXosip_find_last_inc_invite(eXosip_call_t *jc, eXosip_dialog_t *jd);
osip_transaction_t *eXosip_find_last_out_invite(eXosip_call_t *jc, eXosip_dialog_t *jd);
osip_transaction_t *eXosip_find_last_refer(eXosip_call_t *jc, eXosip_dialog_t *jd );
osip_transaction_t *eXosip_find_last_inc_notify_for_refer(eXosip_call_t *jc, eXosip_dialog_t *jd);
osip_transaction_t *eXosip_find_last_out_notify_for_refer(eXosip_call_t *jc, eXosip_dialog_t *jd);
osip_transaction_t *eXosip_find_last_inc_bye(eXosip_call_t *jc, eXosip_dialog_t *jd);
osip_transaction_t *eXosip_find_last_out_bye(eXosip_call_t *jc, eXosip_dialog_t *jd);
osip_transaction_t *eXosip_find_last_inc_refer(eXosip_call_t *jc, eXosip_dialog_t *jd);
osip_transaction_t *eXosip_find_last_out_refer(eXosip_call_t *jc, eXosip_dialog_t *jd);

int eXosip_list_remove_element(osip_list_t *li, void *el);
void eXosip_transaction_free(osip_transaction_t *tr);


int  eXosip_call_init(eXosip_call_t **jc);
void eXosip_call_free(eXosip_call_t *jc);
void __eXosip_call_remove_dialog_reference_in_call(eXosip_call_t *jc, eXosip_dialog_t *jd);
void eXosip_call_set_subject(eXosip_call_t *jc, char *subject);
int  eXosip_read_message(int max_message_nb, int sec_max, int usec_max);
void eXosip_recv (OWSLSocket socket) ;
void eXosip_release_terminated_calls ( void );


int  eXosip_subscribe_init(eXosip_subscribe_t **js, char *uri);
void eXosip_subscribe_free(eXosip_subscribe_t *js);
int  _eXosip_subscribe_set_refresh_interval(eXosip_subscribe_t *js, osip_message_t *inc_subscribe);
int  eXosip_subscribe_need_refresh(eXosip_subscribe_t *js, time_t now);
int eXosip_subscribe_send_subscribe(eXosip_subscribe_t *js,
				    eXosip_dialog_t *jd, const char *expires);

int  eXosip_notify_init(eXosip_notify_t **jn, OWSIPAccount account, osip_message_t *inc_subscribe);
void eXosip_notify_free(eXosip_notify_t *jn);
int  _eXosip_notify_set_contact_info(eXosip_notify_t *jn, char *uri);
int  _eXosip_notify_set_refresh_interval(eXosip_notify_t *jn,
					 osip_message_t *inc_subscribe);
void _eXosip_notify_add_expires_in_2XX_for_subscribe(eXosip_notify_t *jn,
						     osip_message_t *answer);
int  _eXosip_notify_add_body(eXosip_notify_t *jn, osip_message_t *notify);
int  eXosip_notify_add_allowed_subscriber(char *sip_url);
int  _eXosip_notify_is_a_known_subscriber(osip_message_t *sip);
int  eXosip_notify_send_notify(eXosip_notify_t *jn, eXosip_dialog_t *jd,
			       int subsciption_status,
			       int online_status);
int  eXosip_notify_send_notify2(eXosip_notify_t *jn, eXosip_dialog_t *jd,
			       int subsciption_status,
			       int online_status, const char* ctt, const char* body);

int  _eXosip_transfer_send_notify(eXosip_call_t *jc,
				  eXosip_dialog_t *jd,
				  int subscription_status,
				  char *body);


void eXosip_report_call_event_with_status(int evt, eXosip_call_t *jc, eXosip_dialog_t *jd, osip_message_t *sip);

int eXosip_sdp_negotiate(osip_negotiation_ctx_t *ctx, sdp_message_t *remote_sdp);

int eXosip_is_public_address(const char *addr);

void eXosip_retransmit_lost200ok(void);

sdp_message_t *eXosip_get_sdp_body(osip_message_t *message);


/**
 * Finds the eXosip_reg_t associated with a local contact of form <sip:username@ip:port>
 *
 * @param	local_contact	<sip:username@ip:port>
 * @return	the eXosip_reg_t associated with a local contact
 */
eXosip_reg_t * eXosip_reg_find_by_local_contact(const char * local_contact);

/**
 * Finds the subscription id associated to a remote uri
 *
 * @param	remote_uri	a remote uri "sip:user@domain"
 * @param	sid	a buffer receiving the subscription id
 * @return	0 if succeeds; -1 else
 */
int eXosip_get_subscribe_id(const char * remote_uri, int * sid);


void eXosip_sdp_negotiation_set_mediaip(osip_negotiation_t *sn);
int eXosip_retrieve_sdp_negotiation_specific_result(osip_negotiation_ctx_t *ctx, char *payload_name,  int pnsize);

#ifdef __cplusplus
}
#endif

#endif
