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

#include <errno.h>

#ifdef ENABLE_MPATROL
#include <mpatrol.h>
#endif

#include <stdlib.h>

#if defined(_WIN32_WCE) || defined(WIN32)
#include <windowsx.h>
#include <winsock2.h>
#include <Ws2tcpip.h>
#else
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#endif

#include <eXosip/eXosip.h>
#include "eXosip2.h"
#include <eXosip/eXosip_cfg.h>

#include <svoip_phapi.h>

extern eXosip_t eXosip;

#ifdef TEST_AUDIO
static pid_t pid = 0;
#endif

/* Private functions */

static eXosip_reg_t *find_reg_for_transaction(osip_transaction_t *tr);

static void rcvregister_failure(int type, osip_transaction_t *tr,osip_message_t *sip);
static int cb_udp_snd_message(osip_transaction_t *tr, osip_message_t *sip,
			      char *host, int port, int out_socket);
static void cb_ict_kill_transaction(int type, osip_transaction_t *tr);
static void cb_ist_kill_transaction(int type, osip_transaction_t *tr);
static void cb_nict_kill_transaction(int type, osip_transaction_t *tr);
static void cb_nist_kill_transaction(int type, osip_transaction_t *tr);
static void cb_rcvinvite  (int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_rcvack     (int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_rcvack2    (int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_rcvregister(int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_rcvbye     (int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_rcvcancel  (int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_rcvinfo    (int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_rcvoptions (int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_rcvnotify  (int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_rcvsubscribe (int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_rcvunkrequest(int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_sndinvite  (int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_sndack     (int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_sndregister(int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_sndbye     (int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_sndcancel  (int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_sndinfo    (int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_sndoptions (int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_sndnotify  (int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_sndsubscribe(int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_sndunkrequest(int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_rcv1xx(int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_rcv2xx_4invite(osip_transaction_t *tr,osip_message_t *sip);
static void cb_rcv2xx_4subscribe(osip_transaction_t *tr,osip_message_t *sip);
static void cb_rcv2xx(int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_rcv3xx(int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_rcv4xx(int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_rcv5xx(int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_rcv6xx(int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_snd1xx(int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_snd2xx(int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_snd3xx(int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_snd4xx(int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_snd5xx(int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_snd6xx(int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_rcvresp_retransmission(int type, osip_transaction_t *tr, osip_message_t *sip);
static void cb_sndreq_retransmission(int type, osip_transaction_t *tr, osip_message_t *sip);
static void cb_sndresp_retransmission(int type, osip_transaction_t *tr, osip_message_t *sip);
static void cb_rcvreq_retransmission(int type, osip_transaction_t *tr, osip_message_t *sip);
static void cb_transport_error(int type, osip_transaction_t *tr, int error);
static void report_event_with_status(eXosip_event_t *je, osip_message_t *sip);

/* owsip_send:
 * This function really send the packet over the network.
 */
static int
owsip_send (OWSIPAccount account, const void *buf, int len, int flags, const struct sockaddr *to, int tolen)
{
	TransportProtocol transport ;
	int res ;

	transport = owsip_account_transport_get (account) ;
	if (transport == TRANSPORT_UNKNOWN)
	{
		OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL,
			"no matching SIP account found\n")) ;
		return -1 ;
	}
	eXosip_lock_jsocket();
	res = transport_sendto
	(
		transport,
		buf,
		len,
		flags,
		to,
		tolen
	);
	eXosip_unlock_jsocket();

	if (res < 0)
		return -1;
	return 0;
}

int cb_snd_message (osip_transaction_t * tr, osip_message_t * sip, char *host,
                int port, int out_socket)
{
	int i;
	osip_via_t *via;

	via = (osip_via_t *) osip_list_get (&sip->vias, 0);
	if (via == NULL || via->protocol == NULL)
		return -1;

	if (host == NULL)
	{
		if (MSG_IS_REQUEST(sip))
		{
			osip_route_t *route;

			osip_message_get_route (sip, 0, &route);
			if (route != NULL)
			{
				osip_uri_param_t *lr_param = NULL;

				osip_uri_uparam_get_byname (route->url, "lr", &lr_param);
				if (lr_param == NULL)
					route = NULL;
			}

			if (route != NULL)
			{
				port = 5060;
				if (route->url->port != NULL)
					port = osip_atoi (route->url->port);
				host = route->url->host;
			}
			else
			{
				port = 5060;
				if (sip->req_uri->port != NULL)
					port = osip_atoi (sip->req_uri->port);
				host = sip->req_uri->host;
			}

		}
		else
		{
			osip_generic_param_t *maddr;
			osip_generic_param_t *received;
			osip_generic_param_t *rport;

			osip_via_param_get_byname (via, "maddr", &maddr);
			osip_via_param_get_byname (via, "received", &received);
			osip_via_param_get_byname (via, "rport", &rport);
			if (maddr != NULL)
				host = maddr->gvalue;
			else if (received != NULL)
				host = received->gvalue;
			else
				host = via->host;

			if (rport == NULL || rport->gvalue == NULL)
			{
				if (via->port != NULL)
					port = osip_atoi (via->port);
				else
					port = 5060;
			} else
				port = osip_atoi (rport->gvalue);
		}
	}

	i = -1;

	i = cb_udp_snd_message (tr, sip, host, port, out_socket);

	if (i != 0)
	{
		return -1;
	}

	return 0;
}

int cb_udp_snd_message(osip_transaction_t *tr, osip_message_t *sip, char *host,
		int port, int out_socket)
{
	OWSIPAccount account = 0 ;
	size_t length = 0;
	static int num = 0;
	struct sockaddr * address ;
	char *message;
	int i;
	int remove = 0;
	char * proxy = NULL ;
	osip_route_t *o_proxy = 0;
	jinfo_t *jinfo = 0;
	char *tmp_host = 0;
	if (host) {
		tmp_host = osip_strdup(host);
	}

	jinfo = (jinfo_t *) osip_transaction_get_your_instance(tr) ;
	if (jinfo != NULL)
	{
		account = jinfo->account ;
		proxy = owsip_account_proxy_get (account) ;
	}

	if (proxy != NULL && proxy [0] != '\0')
	{
		size_t lr_proxy_size ;
		char * lr_proxy ;

#ifndef __VXWORKS_OS__
		osip_route_init(&o_proxy);
#else
		osip_route_init2(&o_proxy);
#endif

		lr_proxy_size = strlen (proxy) + 10 ;
		lr_proxy = malloc (lr_proxy_size) ;
		if (lr_proxy == NULL)
		{
			free(tmp_host);
			return -1 ;
		}
		snprintf (lr_proxy, lr_proxy_size, "<sip:%s;lr>", proxy) ;
		osip_route_parse (o_proxy, lr_proxy) ;
		free (lr_proxy) ;

		osip_free(tmp_host);
		tmp_host = osip_strdup(o_proxy->url->host);
		port = (o_proxy->url->port) ? atoi(o_proxy->url->port) : 5060;
	}

	if (tmp_host==NULL)
	{
		tmp_host = strdup(sip->req_uri->host);
		if (sip->req_uri->port!=NULL)
			port = osip_atoi(sip->req_uri->port);
		else
			port = 5060;
	}

	address = owsl_address_port_new (OWSL_AF_UNDEFINED, tmp_host, (unsigned short) port) ;
	if (address == NULL)
	{
		osip_free(tmp_host);
		return -1 ;
	}

	if (o_proxy)
	{
		osip_route_free(o_proxy);
		o_proxy = 0;
	}

	if (jinfo != NULL)
	{
		/* Check if the first route header is the same as the destination address */
		/* Remove it just before creating the SIP buffer to send, and then add it */
		/* again to avoid non desired behavior in eXosip */
		osip_message_get_route(sip, 0, &o_proxy);
		if (owsip_account_route_filter_is_enabled(account) && 
			(!jinfo->jd || jinfo->jd->d_STATE == JD_AUTH_REQUIRED) && 
			o_proxy && o_proxy->url && (strcmp(o_proxy->url->host, tmp_host) == 0))
		{
			owsip_list_remove_element(&sip->routes, o_proxy);
			remove = 1;
		}
	}
	osip_free(tmp_host);

	/* sVoIP integration */
	// SPIKE_SRTP: Check if a outgoing packet has to be processed by sVoIP
	if (tr)
	{
		int cid = -1;
		jinfo = osip_transaction_get_your_instance(tr);
		/* Check that the jcall structure is available and 
		has enabled crypting */
		if (jinfo && jinfo->jc/* && jinfo->jc->iscrypted*/)
		{
			/* Get the CID from the jcall structure */
			cid = jinfo->jc->c_id;  
		}
		if (cid >= 0)
		{
			/* The connection is crypted */
			if (MSG_IS_RESPONSE_FOR(sip, "INVITE") && sip->status_code == 200)
				sVoIP_phapi_handle_ok_out(cid, sip); // 200OK augmented here
			else
			/*	  if (MSG_IS_RESPONSE_FOR(sip, "INVITE") && sip->status_code == 180)
				sVoIP_phapi_handle_hanging_out(cid, sip); // 180 Hanging augmented here
			else*/
				if (MSG_IS_INVITE(sip))
					sVoIP_phapi_handle_invite_out(cid, sip); // INVITE augmented here
			else
				if (MSG_IS_BYE(sip))
					sVoIP_phapi_handle_bye_out(cid, sip); // Close crypted session here
			jinfo->jc->iscrypted = sVoIP_phapi_isCrypted(cid);
		}
	}
	/* sVoIP */

	/* let a plugin modify the message */
	if (owsip_account_message_filter (account, sip, OWSIP_OUT, tr))
	{
		free (address) ;
		return -1 ;
	}

	/* convert message into string */
	i = osip_message_to_str(sip, &message, &length);

	//JULIEN: re-add the previously removed route header
	if (remove)
	{
		osip_list_add(&sip->routes, o_proxy, 0);
	}

	if (i!=0 || length<=0) 
	{
		free (address) ;
		return -1;
	}

	OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,
		"Message sent: \n%s\n", message));

	// Really send the packet over network
	i = owsip_send (account, (const void*) message, length, 0, address, OWSL_ADDRESS_SIZE);
	free (address) ;

	if (i < 0)
	{
#if defined(_WIN32_WCE) || defined(WIN32)
		if (WSAECONNREFUSED==WSAGetLastError())
#else
		if (ECONNREFUSED==errno)
#endif
		{
			/* This can be considered as an error, but for the moment,
			I prefer that the application continue to try sending
			message again and again... so we are not in a error case.
			Nevertheless, this error should be announced!
			ALSO, UAS may not have any other options than retry always
			on the same port.
			*/
			osip_free(message);
			return 1;
		}
		else
		{
			/* SIP_NETWORK_ERROR; */
			osip_free(message);
			return -1;
		}
	}
	if (strncmp(message, "INVITE", 7)==0)
	{
		num++;
		OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO4,NULL,"number of message sent: %i\n", num));
	}

	/* for debug use */
	/*printf ("SENT:\n%s\n", message) ;*/

	osip_free(message);
	//<MINHPQ>
	/*
	if (tr->ctx_type == NIST && MSG_IS_RESPONSE(sip)) {
	__osip_transaction_set_state(tr,NIST_COMPLETED);
	}
	*/
	//</MINHPQ>
	return 0;
}

static void cb_ict_kill_transaction(int type, osip_transaction_t *tr)
{
  int i;
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_ict_kill_transaction (id=%i)\r\n", tr->transactionid));

  i = osip_remove_transaction(eXosip.j_osip, tr);
  if (i!=0)
    {
      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_BUG,NULL,"cb_ict_kill_transaction Error: Could not remove transaction from the oSIP stack? (id=%i)\r\n", tr->transactionid));
    }
}

static void cb_ist_kill_transaction(int type, osip_transaction_t *tr)
{
  int i;
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_ist_kill_transaction (id=%i)\r\n", tr->transactionid));
  i = osip_remove_transaction(eXosip.j_osip, tr);
  if (i!=0)
    {
      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_BUG,NULL,"cb_ist_kill_transaction Error: Could not remove transaction from the oSIP stack? (id=%i)\r\n", tr->transactionid));
    }
}

static void cb_nict_kill_transaction(int type, osip_transaction_t *tr)
{
  int i;
  eXosip_dialog_t    *jd;
  eXosip_call_t      *jc;
  eXosip_subscribe_t *js;
  eXosip_notify_t    *jn;
  jinfo_t *jinfo =  (jinfo_t *)osip_transaction_get_your_instance(tr);
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_nict_kill_transaction (id=%i)\r\n", tr->transactionid));
  i = osip_remove_transaction(eXosip.j_osip, tr);
  if (i!=0)
    {
      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_BUG,NULL,"cb_nict_kill_transaction Error: Could not remove transaction from the oSIP stack? (id=%i)\r\n", tr->transactionid));
    }

//<MINHPQ>
	if (!tr->orig_request)
		return;
//</MINHPQ>

  if (MSG_IS_REGISTER(tr->orig_request)
      && type==OSIP_NICT_KILL_TRANSACTION
      && tr->last_response==NULL)
    {
      eXosip_event_t *je;
      eXosip_reg_t *jr;

      jr = find_reg_for_transaction(tr);
      if (!jr)
	return;

      je = eXosip_event_init_for_reg(EXOSIP_REGISTRATION_FAILURE, jr, 0);
      je->status_code = 408;
      report_event_with_status(je, NULL);
      return;
    }

  if (jinfo==NULL)
    return;
  jd = jinfo->jd;
  jc = jinfo->jc;
  jn = jinfo->jn;
  js = jinfo->js;


  if (MSG_IS_REFER(tr->orig_request)
      && type==OSIP_NICT_KILL_TRANSACTION
      && tr->last_response==NULL)
    {
      eXosip_event_t *je;

      je = eXosip_event_init_for_call(EXOSIP_CALL_REFER_FAILURE, jc, jd);
      je->status_code = 408;
      osip_strncpy(je->reason_phrase,"Request Timeout", sizeof(je->reason_phrase));
      report_event_with_status(je, NULL);
      return;
    }


  if (jn==NULL && js==NULL)
    return;

  /* no answer to a NOTIFY request! */
  if (MSG_IS_NOTIFY(tr->orig_request)
      && type==OSIP_NICT_KILL_TRANSACTION
      && tr->last_response==NULL)
    {
      /* delete the dialog! */
      REMOVE_ELEMENT(eXosip.j_notifies, jn);
      eXosip_notify_free(jn);
      return;
    }

  if (MSG_IS_NOTIFY(tr->orig_request)
      && type==OSIP_NICT_KILL_TRANSACTION
      && tr->last_response!=NULL
      && tr->last_response->status_code > 299)
    {
      /* delete the dialog! */
      REMOVE_ELEMENT(eXosip.j_notifies, jn);
      eXosip_notify_free(jn);
      return;
    }

  if (MSG_IS_NOTIFY(tr->orig_request)
      && type==OSIP_NICT_KILL_TRANSACTION
      && tr->last_response!=NULL
      && tr->last_response->status_code > 199
      && tr->last_response->status_code < 300)
    {
      if (jn->n_ss_status==EXOSIP_SUBCRSTATE_TERMINATED)
	{
	  /* delete the dialog! */
	  REMOVE_ELEMENT(eXosip.j_notifies, jn);
	  eXosip_notify_free(jn);
	  return;
	}
    }

 #if 0  /* WE want to retransmit subscribtion requests evene if the server does not answer */
  /* no answer to a SUBSCRIBE request! */
  if (MSG_IS_SUBSCRIBE(tr->orig_request)
      && type==OSIP_NICT_KILL_TRANSACTION
      && tr->last_response==NULL)
    {
      /* delete the dialog! */
      REMOVE_ELEMENT(eXosip.j_subscribes, js);
      eXosip_subscribe_free(js);
      return;
    }
#endif

  /* detect SUBSCRIBE request that close the dialogs! */
  /* expires=0 with MSN */
  if (MSG_IS_SUBSCRIBE(tr->orig_request)
      && type==OSIP_NICT_KILL_TRANSACTION)
    {
      osip_header_t *expires;
      osip_message_get_expires(tr->orig_request, 0, &expires);
      if (expires==NULL || expires->hvalue==NULL)
	{
	}
      else if (0==strcmp(expires->hvalue, "0"))
	{
	  /* delete the dialog! */
	  REMOVE_ELEMENT(eXosip.j_subscribes, js);
	  eXosip_subscribe_free(js);
	  return;
	}
    }
}

static void cb_nist_kill_transaction(int type, osip_transaction_t *tr)
{
  int i;
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_nist_kill_transaction (id=%i)\r\n", tr->transactionid));
  i = osip_remove_transaction(eXosip.j_osip, tr);
  if (i!=0)
    {
      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_BUG,NULL,"cb_nist_kill_transaction Error: Could not remove transaction from the oSIP stack? (id=%i)\r\n", tr->transactionid));
    }

}

static void cb_rcvinvite  (int type, osip_transaction_t *tr,osip_message_t *sip)
{
  /* sVoIP integration */
  // SPIKE_SRTP: Check if an incoming INVITE packet has to be processed by sVoIP
  if (tr)
    {
      int	cid = -1;
      jinfo_t	*jinfo = NULL;
      jinfo = osip_transaction_get_your_instance(tr);
      /* Check call structure */
      if ( jinfo  && jinfo->jc /*&&  jinfo->jc->iscrypted */)
	  cid = jinfo->jc->c_id;
      if (cid >= 0)
	{
	  sVoIP_phapi_handle_invite_in(cid, sip);
	  jinfo->jc->iscrypted = sVoIP_phapi_isCrypted(cid);
	}
    }
  /* sVoIP */

  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcvinvite (id=%i)\n", tr->transactionid));
}

static void cb_rcvack     (int type, osip_transaction_t *tr,osip_message_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcvack (id=%i)\n", tr->transactionid));
}

static void cb_rcvack2    (int type, osip_transaction_t *tr,osip_message_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcvack2 (id=%i)\r\n", tr->transactionid));
}

static void cb_rcvregister(int type, osip_transaction_t *tr,osip_message_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcvregister (id=%i)\r\n", tr->transactionid));
}

static void cb_rcvbye     (int type, osip_transaction_t *tr,osip_message_t *sip)
{
  /* sVoIP integration */
  // SPIKE_SRTP: Check if an incoming BYE packet has to be processed by sVoIP
  if (tr)
    {
      int	cid = -1;
      jinfo_t	*jinfo = NULL;
      jinfo = osip_transaction_get_your_instance(tr);
      /* Check call structure */
      if ( jinfo  && jinfo->jc /*&&  jinfo->jc->iscrypted */)
	  cid = jinfo->jc->c_id;
      if (cid >= 0)
	sVoIP_phapi_handle_bye_in(cid, sip);
    }
  /* sVoIP */

  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcvbye (id=%i)\r\n", tr->transactionid));
#ifdef TEST_AUDIO
  if (pid!=0)
    {
      int i = kill(pid, SIGINT);
      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"audio command kill return %i %i\n", i, pid));
      pid = 0;
    }
#endif
}

static void cb_rcvcancel  (int type, osip_transaction_t *tr,osip_message_t *sip)
{
  /* sVoIP integration */
  // SPIKE_SRTP: Check if an incoming CANCEL packet has to be processed by sVoIP
  if (tr)
    {
      int	cid = -1;
      jinfo_t	*jinfo = NULL;
      jinfo = osip_transaction_get_your_instance(tr);
      /* Check call structure */
      if ( jinfo  && jinfo->jc /*&& jinfo->jc->iscrypted*/)
	  cid = jinfo->jc->c_id;
      if (cid >= 0)
	sVoIP_phapi_handle_cancel_in(cid, sip);
    }
  /* sVoIP */

  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcvcancel (id=%i)\r\n", tr->transactionid));
}

static void cb_rcvinfo    (int type, osip_transaction_t *tr,osip_message_t *sip)
{
  eXosip_event_t     *je;
  jinfo_t *jinfo =  (jinfo_t *)osip_transaction_get_your_instance(tr);
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcvinfo (id=%i)\r\n", tr->transactionid));

  if (jinfo==NULL)
    return;
  if (jinfo->jc==NULL)
    return;

  je = eXosip_event_init_for_call(EXOSIP_INFO_NEW, jinfo->jc, jinfo->jd);
  if (je!=NULL)
    {
      char *tmp;
      osip_uri_to_str(sip->req_uri, &tmp);
      if (tmp!=NULL)
 	{
 	  snprintf(je->req_uri, 255, "%s", tmp);
 	  osip_free(tmp);
 	}

      if (sip!=NULL)
	{
	  int pos;
	  /* get content-type info */
	  osip_content_type_clone(osip_message_get_content_type(sip), &(je->i_ctt));
	  /* get list of bodies */
	  je->i_bodies = (osip_list_t*) osip_malloc(sizeof(osip_list_t));
	  osip_list_init(je->i_bodies);
	  for (pos=0;!osip_list_eol(&sip->bodies, pos);pos++)
	    {
	      osip_body_t *body;
	      osip_body_t *_body;
	      body = (osip_body_t *)osip_list_get(&sip->bodies, pos);
	      osip_body_clone(body, &_body);
	      osip_list_add(je->i_bodies, _body, -1);
	    }
	}
    }

  report_event_with_status(je, NULL);
}

static void cb_rcvoptions (int type, osip_transaction_t *tr,osip_message_t *sip)
{
  eXosip_event_t     *je;
  eXosip_dialog_t    *jd;
  eXosip_call_t      *jc;
  eXosip_subscribe_t *js;
  eXosip_notify_t    *jn;
  jinfo_t *jinfo =  (jinfo_t *)osip_transaction_get_your_instance(tr);

  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcvoptions (id=%i)\r\n", tr->transactionid));

  if (jinfo==NULL)
    return;
  jd = jinfo->jd;
  jc = jinfo->jc;
  jn = jinfo->jn;
  js = jinfo->js;
  if (jinfo->jc==NULL)
    return;

  je = eXosip_event_init_for_call(EXOSIP_OPTIONS_NEW, jc, jd);
  if (je!=NULL)
    {
      char *tmp;
      osip_uri_to_str(sip->req_uri, &tmp);
      if (tmp!=NULL)
	{
	  snprintf(je->req_uri, 255, "%s", tmp);
	  osip_free(tmp);
	}
    }
  report_event_with_status(je, NULL);

}

static void cb_rcvnotify  (int type, osip_transaction_t *tr,osip_message_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcvnotify (id=%i)\r\n", tr->transactionid));
}

static void cb_rcvsubscribe (int type, osip_transaction_t *tr,osip_message_t *sip)
{
  eXosip_event_t     *je;
  eXosip_dialog_t    *jd;
  eXosip_notify_t    *jn;
  jinfo_t *jinfo =  (jinfo_t *)osip_transaction_get_your_instance(tr);

  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcvsubscribe (id=%i)\r\n", tr->transactionid));

  if (jinfo==NULL)
    return;
  jd = jinfo->jd;
  jn = jinfo->jn;
  if (jinfo->jn==NULL)
    return;

  je = eXosip_event_init_for_notify(EXOSIP_IN_SUBSCRIPTION_NEW, jn, jd);
  if (je!=NULL)
    {
      char *tmp;
      osip_uri_to_str(sip->req_uri, &tmp);
      if (tmp!=NULL)
	{
	  snprintf(je->req_uri, 255, "%s", tmp);
	  osip_free(tmp);
	}
    }
  report_event_with_status(je, NULL);
}

static void cb_rcvunkrequest(int type, osip_transaction_t *tr,osip_message_t *sip)
{
  eXosip_dialog_t    *jd;
  eXosip_call_t      *jc;
  jinfo_t *jinfo =  (jinfo_t *)osip_transaction_get_your_instance(tr);

  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcvunkrequest (id=%i)\r\n", tr->transactionid));

  if (jinfo==NULL)
    return;
  jd = jinfo->jd;
  jc = jinfo->jc;
  if (jinfo->jc==NULL)
    return;


  if (!MSG_IS_REFER(sip))
    return;



  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcvrefer (id=%i)\r\n", tr->transactionid));

  eXosip_report_call_event_with_status(EXOSIP_CALL_REFERED, jc, jd, NULL);


}

static void cb_sndinvite  (int type, osip_transaction_t *tr,osip_message_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_sndinvite (id=%i)\r\n", tr->transactionid));
}

static void cb_sndack     (int type, osip_transaction_t *tr,osip_message_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_sndack (id=%i)\r\n", tr->transactionid));
}

static void cb_sndregister(int type, osip_transaction_t *tr,osip_message_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_sndregister (id=%i)\r\n", tr->transactionid));
}

static void cb_sndbye     (int type, osip_transaction_t *tr,osip_message_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_sndbye (id=%i)\r\n", tr->transactionid));
#ifdef TEST_AUDIO
  if (pid!=0)
    {
      int i = kill(pid, SIGINT);
      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"audio command kill return %i %i\n", i, pid));
      pid = 0;
    }
#endif

}

static void cb_sndcancel  (int type, osip_transaction_t *tr,osip_message_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_sndcancel (id=%i)\r\n", tr->transactionid));
}

static void cb_sndinfo    (int type, osip_transaction_t *tr,osip_message_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_sndinfo (id=%i)\r\n", tr->transactionid));
}

static void cb_sndoptions (int type, osip_transaction_t *tr,osip_message_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_sndoptions (id=%i)\r\n", tr->transactionid));
}

static void cb_sndnotify  (int type, osip_transaction_t *tr,osip_message_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_sndnotify (id=%i)\r\n", tr->transactionid));
}

static void cb_sndsubscribe(int type, osip_transaction_t *tr,osip_message_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_sndsubscibe (id=%i)\r\n", tr->transactionid));
}

static void cb_sndunkrequest(int type, osip_transaction_t *tr,osip_message_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_sndunkrequest (id=%i)\r\n", tr->transactionid));
}

void __eXosip_delete_jinfo(osip_transaction_t *transaction)
{
  jinfo_t *ji;
  if (transaction==NULL)
    return;
  ji = osip_transaction_get_your_instance(transaction);
  osip_transaction_set_your_instance(transaction, NULL);
  osip_free(ji);
}

jinfo_t *__eXosip_new_jinfo(OWSIPAccount account, eXosip_call_t *jc, eXosip_dialog_t *jd,
			    eXosip_subscribe_t *js, eXosip_notify_t *jn)
{
  jinfo_t *ji = (jinfo_t *) osip_malloc(sizeof(jinfo_t));
  if (ji==NULL) return NULL;
  ji->account = account ;
  ji->jd = jd;
  ji->jc = jc;
  ji->js = js;
  ji->jn = jn;
  ji->jm = 0;
  return ji;
}


int
eXosip_sdp_negotiate(osip_negotiation_ctx_t *ctx, sdp_message_t *remote_sdp)
{
  int i ;
  sdp_message_t *tmp  = osip_negotiation_ctx_get_remote_sdp(ctx);

  if (tmp!=NULL)
    sdp_message_free(tmp);

  osip_negotiation_ctx_set_remote_sdp(ctx, remote_sdp);
  tmp = osip_negotiation_ctx_get_local_sdp(ctx);
  if (tmp)
    {
      sdp_message_free(tmp);
      osip_negotiation_ctx_set_local_sdp(ctx, NULL);
    }

  i = osip_negotiation_ctx_execute_negotiation(eXosip.osip_negotiation, ctx);
  return i;
}


static void
check_for_answer_with_sdp(eXosip_call_t *jc, osip_message_t *sip)
{
  sdp_message_t *remote_sdp = eXosip_get_sdp_body(sip);

  if (remote_sdp)
    eXosip_sdp_negotiate(jc->c_ctx, remote_sdp);
}



static void cb_rcv1xx(int type, osip_transaction_t *tr,osip_message_t *sip)
{
  eXosip_dialog_t    *jd;
  eXosip_call_t      *jc;
  eXosip_subscribe_t *js;
  eXosip_notify_t    *jn;
  jinfo_t *jinfo =  (jinfo_t *)osip_transaction_get_your_instance(tr);
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcv1xx (id=%i)\r\n", tr->transactionid));
  if (jinfo==NULL)
    return;
  jd = jinfo->jd;
  jc = jinfo->jc;
  jn = jinfo->jn;
  js = jinfo->js;

  if (MSG_IS_RESPONSE_FOR(sip, "OPTIONS"))
    {
      if (jc==NULL)
	{
	  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcv1xx (id=%i) Error: no call or transaction info for OPTIONS transaction\r\n", tr->transactionid));
	  return;
	}
      else if (jc->c_out_options_tr==NULL)
	{
	  /* options is within a call */
	}
      eXosip_report_call_event_with_status(EXOSIP_OPTIONS_PROCEEDING, jc, jd, sip);
      return;
    }

  if ((MSG_IS_RESPONSE_FOR(sip, "INVITE")
       || MSG_IS_RESPONSE_FOR(sip, "SUBSCRIBE")))
      /*      && MSG_TEST_CODE(sip, 100)) */
    {
      int i;
      if (jd == NULL) /* This transaction initiate a dialog in the case of
			 INVITE (else it would be attached to a "jd" element. */
	{
	  /* allocate a jd */
	  i = eXosip_dialog_init_as_uac(&jd, owsip_transaction_account_get (tr), sip);
	  if (i!=0)
	    {
         OSIP_TRACE (osip_trace
		     (__FILE__, __LINE__, OSIP_ERROR, NULL,
	         "eXosip: cannot establish a dialog\n"));
	      return;
	    }
	  if (jc!=NULL)
	    {
	      ADD_ELEMENT(jc->c_dialogs, jd);
	      jinfo->jd = jd;
	      eXosip_update();
	    }
	  else if (js!=NULL)
	    {
	      ADD_ELEMENT(js->s_dialogs, jd);
	      jinfo->jd = jd;
	      eXosip_update();
	    }
	  else if (jn!=NULL)
	    {
	      ADD_ELEMENT(jn->n_dialogs, jd);
	      jinfo->jd = jd;
	      eXosip_update();
	    }
	  else
	    {
#if !(defined(_WIN32_WCE) || defined(WIN32))
	      assert(0==0);
#else
		  exit(0);
#endif
	    }
	  osip_transaction_set_your_instance(tr, jinfo);
	}
      else if (jd->d_dialog)
	{
	  osip_dialog_update_route_set_as_uac(jd->d_dialog, sip);
	}
      else
	{
         OSIP_TRACE (osip_trace
		     (__FILE__, __LINE__, OSIP_ERROR, NULL,
	         "eXosip: have eXosip_dialog with NULL osip_dialog\n"));
	      return;

	}

    }

  if ( jd!=NULL)
    jd->d_STATE = JD_TRYING;
  if ( jd!=NULL && MSG_IS_RESPONSE_FOR(sip, "INVITE")
       && sip->status_code < 180)
    {
      check_for_answer_with_sdp(jc, sip);
      eXosip_report_call_event_with_status(EXOSIP_CALL_PROCEEDING, jc, jd, sip);
    }
  else if ( jd!=NULL && MSG_IS_RESPONSE_FOR(sip, "INVITE")
	    && sip->status_code >= 180)
    {
      check_for_answer_with_sdp(jc, sip);
      eXosip_report_call_event_with_status(EXOSIP_CALL_RINGING, jc, jd, sip);
    }
  else if ( jd!=NULL && MSG_IS_RESPONSE_FOR(sip, "SUBSCRIBE"))
    {
      eXosip_event_t *je;
      je = eXosip_event_init_for_subscribe(EXOSIP_SUBSCRIPTION_PROCEEDING, js, jd);
      if (je!=NULL)
	report_event_with_status(je, sip);
    }
  if (MSG_TEST_CODE(sip, 180) && jd!=NULL)
    {
      jd->d_STATE = JD_RINGING;
    }
  else if (MSG_TEST_CODE(sip, 183) && jd!=NULL)
    {
      jd->d_STATE = JD_QUEUED;
    }


}

sdp_message_t *eXosip_get_sdp_body(osip_message_t *message)
{
  osip_body_t *body;
  sdp_message_t *sdp;
  int i, pos = 0;

  sdp=NULL;
  body = (osip_body_t *)osip_list_get(&message->bodies,0);
  while (body!=NULL)
    {
      i = sdp_message_init(&sdp);
      if (i!=0)
	break;

      i = sdp_message_parse(sdp,body->body);
      if (i==0)
	break;

      sdp_message_free(sdp);
      sdp = NULL;
      pos++;
      body = (osip_body_t *)osip_list_get(&message->bodies,pos);
    }
  return sdp;
}


sdp_message_t *eXosip_get_remote_sdp(osip_transaction_t *transaction)
{
  osip_message_t *message;

  if (transaction->ist_context!=NULL)
    /* remote sdp is in INVITE (or ACK!) */
    message = transaction->orig_request;
  else
    /* remote sdp is in response */
    message = transaction->last_response;

  if (message==NULL)
    {
      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"No remote sdp body found\r\n"));
      return NULL;
    }

  return eXosip_get_sdp_body(message);

}

sdp_message_t *eXosip_get_local_sdp(osip_transaction_t *transaction)
{
  osip_message_t *message;

  if (transaction->ict_context!=NULL)
    /* local sdp is in INVITE (or ACK!) */
    message = transaction->orig_request;
  else
    /* local sdp is in response */
    message = transaction->last_response;

  return eXosip_get_sdp_body(message);
}


void
eXosip_report_call_event_with_status(int evt, eXosip_call_t *jc, eXosip_dialog_t *jd, osip_message_t *sip)
{
	eXosip_event_t *je;

	// <ncouturier>
	osip_body_t * body = NULL;
	// </ncouturier>

	je = eXosip_event_init_for_call(evt, jc, jd);

	// <ncouturier>
	//if(jd != null && jd->d_200ok != null && jd->d_200ok->bodies != null){
	//	if(!osip_list_eol(jd->d_200ok->bodies, 0)){
	//		body = (osip_body_t *)osip_list_get(jd->d_200ok->bodies, 0);
	if (sip->status_code == 200) {
		if (!osip_list_eol(&sip->bodies, 0)) {
			body = (osip_body_t *)osip_list_get(&sip->bodies, 0);

			if (je->msg_body) 
				osip_free(je->msg_body);

			je->msg_body = osip_strdup(body->body);
			if (!je->msg_body){
				eXosip_event_free(je);
				return;
			}
		}
	}
	// </ncouturier>

	if (je!=NULL)
	{
		if (sip != NULL)
		{
			osip_contact_t *ct = 0;

			eXosip_event_add_status(je, sip);
			eXosip_event_add_sdp_info(je, sip);

			osip_message_get_contact(sip, 0, &ct);
			if (ct)
			{
				char *tmp;

				osip_contact_to_str(ct, &tmp);
				osip_strncpy(je->remote_contact, tmp, sizeof(je->remote_contact)-1);
				osip_free(tmp);
			}
		}

		if (eXosip.j_call_callbacks[evt]!=NULL)
			eXosip.j_call_callbacks[evt](evt, je);
		else if (eXosip.j_runtime_mode==EVENT_MODE)
			eXosip_event_add(je);
    }
}

static
void report_event_with_status(eXosip_event_t *je, osip_message_t *sip)
{
  if (je!=NULL)
    {
      int evt = je->type;

      if (sip != NULL)
	eXosip_event_add_status(je, sip);
      if (eXosip.j_call_callbacks[evt]!=NULL)
	eXosip.j_call_callbacks[evt](evt, je);
      else if (eXosip.j_runtime_mode==EVENT_MODE)
	eXosip_event_add(je);
    }
}


#if 0
void eXosip_update_audio_session(osip_transaction_t *transaction)
{
  char *remaddr;
  sdp_message_t *remote_sdp;
  sdp_message_t *local_sdp;
  char *remote_port;
  char *local_port;
  char *payload;
  char *media_type;
  int pos;
  /* look for the SDP informations */

  remote_sdp = eXosip_get_remote_sdp(transaction);
  if (remote_sdp==NULL)
    return ;
  local_sdp = eXosip_get_local_sdp(transaction);
  if (local_sdp==NULL)
    {
      sdp_message_free(remote_sdp);
      return ;
    }
  remaddr=sdp_message_c_addr_get(remote_sdp,-1,0);
  if (remaddr==NULL){
    remaddr=sdp_message_c_addr_get(remote_sdp,0,0);
  }

  pos=0;
  local_port=sdp_message_m_port_get(local_sdp,pos);
  media_type = sdp_message_m_media_get(local_sdp,pos);
  while (local_port!=NULL && media_type!=NULL)
    { /* If we have refused some media lines, the port is set to 0 */
      if (0!=strncmp(local_port,"0", 1)&&0==osip_strcasecmp(media_type,"audio"))
	break;
      pos++;
      media_type = sdp_message_m_media_get(local_sdp,pos);
      local_port=sdp_message_m_port_get(local_sdp,pos);
    }

  if (media_type!=NULL && local_port!=NULL)
    {
      remote_port = sdp_message_m_port_get(remote_sdp,pos);
      payload = sdp_message_m_payload_get(local_sdp,pos,0);
    }
  else
    {
      remote_port = NULL;
      payload = NULL;
    }
  if (remote_port!=NULL && media_type!=NULL) /* if codec has been found */
    {
      char tmp[256];
      sprintf(tmp, "mediastream --local %s --remote %s:%s --payload %s > debug_rtp 2>&1" , local_port, remaddr, remote_port, payload);
      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"audio command %s\n", tmp));

#ifdef TEST_AUDIO
      if (pid!=0)
	{
	  int i = kill(pid, SIGINT);
	  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"audio command kill return %i %i\n", i, pid));
	  pid = 0;
	}

      pid = fork();
      if (pid==0)
	{
	  int ret;
#ifndef USE_EXECL
	  ret = system(tmp);
	  if (WIFSIGNALED(ret) &&
	      (WTERMSIG(ret) == SIGINT || WTERMSIG(ret) == SIGQUIT))
	    {
	      exit(-1);
	    }
	  if (ret==0)
	    {
	      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"Could not start audio\n", tmp));
	    }
	  exit(0);
#else
	  char _remoteipport[100];
	  snprintf(_remoteipport, 100, "%s:%s", remaddr, remote_port);
	  ret = execl("mediastream","--local", local_port,
			  "--remote", _remoteipport, "--payload", payload);
#endif
	}
#endif

    }
  else
    {
      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"Could not create audio session.\r\n"));
    }
  sdp_message_free(local_sdp);
  sdp_message_free(remote_sdp);
}
#endif

static void cb_rcv2xx_4invite(osip_transaction_t *tr,osip_message_t *sip)
{
  int i;
  eXosip_dialog_t *jd = NULL;
  eXosip_call_t *jc = NULL;
  jinfo_t *jinfo =  (jinfo_t *)osip_transaction_get_your_instance(tr);
  if (jinfo==NULL)
    return;
  jd = jinfo->jd;
  jc = jinfo->jc;

  /* sVoIP integration */
  // SPIKE_SRTP: Check if an incoming 200OK packet has to be processed by sVoIP
  if (jc /* && jc->iscrypted*/)
  {
    int	cid = -1;
    cid = jinfo->jc->c_id;
    if (cid >= 0)
      sVoIP_phapi_handle_ok_in(cid, sip);
  }
  /* sVoIP */

  if (jd == NULL) /* This transaction initiate a dialog in the case of
		     INVITE (else it would be attached to a "jd" element. */
    {
      /* allocate a jd */
      i = eXosip_dialog_init_as_uac(&jd, owsip_transaction_account_get (tr), sip);
      if (i!=0)
	{
     OSIP_TRACE (osip_trace
	    (__FILE__, __LINE__, OSIP_ERROR, NULL,
	     "eXosip: cannot establish a dialog\n"));
	  return;
	}
      ADD_ELEMENT(jc->c_dialogs, jd);
      jinfo->jd = jd;
      eXosip_update();
      osip_transaction_set_your_instance(tr, jinfo);
    }
  else
    {
      /* Here is a special case:
	 We have initiated a dialog and we have received informationnal
	 answers from 2 or more remote SIP UA. Those answer can be
	 differentiated with the "To" header's tag.

	 We have used the first informationnal answer to create a
	 dialog, but we now want to be sure the 200ok received is
	 for the dialog this dialog.

	 We have to check the To tag and if it does not match, we
	 just have to modify the existing dialog and replace it. */
      osip_generic_param_t *tag;
      int i;
      i = osip_to_get_tag (sip->to, &tag);
      i=1; /* default is the same dialog */

	  if (jd->d_dialog == NULL)
	  {
		  osip_dialog_init_as_uac(&(jd->d_dialog), sip);
	  }


#if 1
      if (jd->d_dialog==NULL)
	{
	  /* There are real use-case where a BYE is received/processed before
	     the 200ok of the previous INVITE. In this case, jd->d_dialog is
	     empty and the transaction should be silently discarded. */
	  /* a ACK should still be sent... -but there is no dialog built- */
	  return;
	}
#endif

      if (jd->d_dialog->remote_tag==NULL && tag==NULL)
	{  } /* non compliant remote UA -> assume it is the same dialog */
      else if (jd->d_dialog->remote_tag!=NULL && tag==NULL)
	{ i=0; } /* different dialog! */
      else if (jd->d_dialog->remote_tag==NULL && tag!=NULL)
	{ i=0; } /* different dialog! */
      else if (jd->d_dialog->remote_tag!=NULL && tag!=NULL && tag->gvalue!=NULL
	       && 0!=strcmp(jd->d_dialog->remote_tag, tag->gvalue))
	{ i=0; } /* different dialog! */

      if (i==1) /* just update the dialog */
	{
	  osip_dialog_update_route_set_as_uac(jd->d_dialog, sip);
	  osip_dialog_set_state(jd->d_dialog, DIALOG_CONFIRMED);
	}
      else
	{
	  /* the best thing is to update the repace the current dialog
	     information... Much easier than creating a useless dialog! */
	  osip_dialog_free(jd->d_dialog);
	  i = osip_dialog_init_as_uac(&(jd->d_dialog), sip);
	  if (i!=0)
	    {
	      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_ERROR,NULL,"Cannot replace the dialog.\r\n"));
	    }
	  else
	    {
	      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_WARNING,NULL,"The dialog has been replaced with the new one fro 200ok.\r\n"));
	    }
	}
    }

  jd->d_STATE = JD_ESTABLISHED;

  //eXosip_dialog_set_200ok(jd, sip);
  check_for_answer_with_sdp(jc, sip);


  {
    osip_route_t *route;
    char *host;
    int port;
    osip_message_t *ack;
    i = _eXosip_build_request_within_dialog2(&ack, "ACK", jd->d_dialog, atoi(tr->cseq->number));
    if (i!=0) {
      jd->d_STATE = JD_ESTABLISHED;
      return ;
    }

    if(jc->c_ack_sdp) /* need to build sdp answer */
      {
	char *body;
	char *size;

	body = generating_sdp_answer(tr->last_response, jc->c_ctx);
	if (body==NULL)
	  {
	    return;
	  }

	i = osip_message_set_body(ack, body, strlen(body));
	if (i!=0)
	  {
	    return;
	  }

	size = (char *) osip_malloc(6*sizeof(char));
#ifdef __APPLE_CC__
	sprintf(size,"%li",strlen(body));
#else
	sprintf(size,"%i",strlen(body));
#endif
	osip_free(body);
	i = osip_message_set_content_length(ack, size);
	osip_free(size);
	if (i!=0)
	  {
	    return;
	  }
	i = osip_message_set_content_type(ack, "application/sdp");
	if (i!=0)
	  {
	    return;
	  }
      }

    osip_message_get_route(ack, 0, &route);
    if (route!=NULL)
      {
	port = 5060;
	if (route->url->port!=NULL)
	  port = osip_atoi(route->url->port);
	host = route->url->host;
      }
    else
      {
	port = 5060;
	if (ack->req_uri->port!=NULL)
	  port = osip_atoi(ack->req_uri->port);
	host = ack->req_uri->host;
      }

    cb_udp_snd_message(tr /* tmonjalo: was NULL */, ack, host, port, 0);

    jd->d_ack  = ack;

  }


  eXosip_report_call_event_with_status(EXOSIP_CALL_ANSWERED, jc, jd, sip);
  eXosip_report_call_event_with_status(EXOSIP_CALL_STARTAUDIO, jc, jd, sip);


  /* look for the SDP information and decide if this answer was for
     an initial INVITE, an HoldCall, or a RetreiveCall */

  /* don't handle hold/unhold by now... */
  /* eXosip_update_audio_session(tr); */

}

static void cb_rcv2xx_4subscribe(osip_transaction_t *tr,osip_message_t *sip)
{
  int i;
  eXosip_dialog_t    *jd = NULL;
  eXosip_subscribe_t *js = NULL;
  char *tmp;
  jinfo_t *jinfo =  (jinfo_t *)osip_transaction_get_your_instance(tr);
  if (jinfo==NULL)
    return;
  jd = jinfo->jd;
  js = jinfo->js;
  _eXosip_subscribe_set_refresh_interval(js, sip);
  if (jd == NULL) /* This transaction initiate a dialog in the case of
		     SUBSCRIBE (else it would be attached to a "jd" element. */
    {
      /* allocate a jd */
      i = eXosip_dialog_init_as_uac(&jd, owsip_transaction_account_get (tr), sip);
      if (i!=0)
	{
     OSIP_TRACE (osip_trace
	    (__FILE__, __LINE__, OSIP_ERROR, NULL,
	     "eXosip: cannot establish a dialog\n"));
	  return;
	}

	if (jd && jd->d_dialog && (jd->d_dialog->remote_contact_uri == NULL)) {
		osip_contact_t *contact;
		i = osip_contact_init (&contact);
		if (i != 0)
			return;
		i = osip_contact_parse (contact, js->s_uri);
		if (i != 0)	{
			osip_contact_free (contact);
			return;
		}
		jd->d_dialog->remote_contact_uri = contact;
	}


      ADD_ELEMENT(js->s_dialogs, jd);
      jinfo->jd = jd;
      eXosip_update();
      osip_transaction_set_your_instance(tr, jinfo);
    }
  else
    {
      osip_dialog_update_route_set_as_uac(jd->d_dialog, sip);
      osip_dialog_set_state(jd->d_dialog, DIALOG_CONFIRMED);
    }

  jd->d_STATE = JD_ESTABLISHED;
  /* look for the body information */

  {
    eXosip_event_t *je;
    je = eXosip_event_init_for_subscribe(EXOSIP_SUBSCRIPTION_ANSWERED, js, jd);
    if (je!=NULL)
    {
		//<MINHPQ>
	if (sip->from != NULL)
		{
			osip_from_to_str(sip->to, &tmp);
			if (tmp!=NULL)	{
				snprintf(je->remote_uri, 255, "%s", tmp);
				osip_free(tmp);
			}
		}
		if (sip->to != NULL) {
			osip_to_to_str(sip->from, &tmp);
			if (tmp!=NULL) {
				snprintf(je->local_uri, 255, "%s", tmp);
				osip_free(tmp);
			}
		}
		//</MINHPQ>
	report_event_with_status(je, sip);
      }
  }

}

static void cb_rcv2xx(int type, osip_transaction_t *tr,osip_message_t *sip)
{
  eXosip_dialog_t    *jd;
  eXosip_call_t      *jc;
  eXosip_subscribe_t *js;
  eXosip_notify_t    *jn;
  jinfo_t *jinfo =  NULL;
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcv2xx (id=%i)\r\n", tr->transactionid));

  jinfo = (jinfo_t *)osip_transaction_get_your_instance(tr);

  if (MSG_IS_RESPONSE_FOR(sip, "REGISTER"))
    {
      eXosip_event_t *je;
      eXosip_reg_t *jr;

      jr = find_reg_for_transaction(tr);
      if (jr==NULL) return;
      je = eXosip_event_init_for_reg(EXOSIP_REGISTRATION_SUCCESS, jr, sip);
      if (je!=NULL)
	{
	  report_event_with_status(je, sip);
	}

    }

  if (jinfo==NULL)
    return;
  jd = jinfo->jd;
  jc = jinfo->jc;
  jn = jinfo->jn;
  js = jinfo->js;

  if (MSG_IS_RESPONSE_FOR(sip, "OPTIONS"))
    {

      if (jc==NULL)
	{
	  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcv2xx (id=%i) Error: no call or transaction info for OPTIONS transaction\r\n", tr->transactionid));
	  return;
	}
      else if (jc->c_out_options_tr==NULL)
	{
	  /* options is within a call */
	}
      eXosip_report_call_event_with_status(EXOSIP_OPTIONS_ANSWERED, jc, jd, sip);
      return;
    }

  if (MSG_IS_RESPONSE_FOR(sip, "INVITE"))
    {
      cb_rcv2xx_4invite(tr, sip);
    }
  else if (MSG_IS_RESPONSE_FOR(sip, "SUBSCRIBE"))
    {
      cb_rcv2xx_4subscribe(tr, sip);
    }
  else if (MSG_IS_RESPONSE_FOR(sip, "BYE"))
    {
      if (jd!=NULL)
	jd->d_STATE = JD_TERMINATED;
    }
  else if (MSG_IS_RESPONSE_FOR(sip, "MESSAGE"))
    {
      eXosip_event_t *je;
      je = eXosip_event_init_for_message(EXOSIP_MESSAGE_SUCCESS, tr, sip);
      if (je!=NULL)
	report_event_with_status(je, sip);
      return;
    }
  else if (MSG_IS_RESPONSE_FOR(sip, "NOTIFY"))
    {
      osip_header_t  *sub_state;
      osip_message_header_get_byname(tr->orig_request, "subscription-state",
				     0, &sub_state);
      if (sub_state==NULL || sub_state->hvalue==NULL)
	{
	  /* UNCOMPLIANT UA without a subscription-state header */
	}
      else if (0==osip_strncasecmp(sub_state->hvalue, "terminated", 10))
	{
	  /* delete the dialog! */
	  if (jn!=NULL)
	    {
	      REMOVE_ELEMENT(eXosip.j_notifies, jn);
	      eXosip_notify_free(jn);
	    }
	}
    }
}

void eXosip_delete_early_dialog(eXosip_dialog_t *jd)
{
  if (jd == NULL) /* bug? */
      return;

  /* an early dialog was created, but the call is not established */
  if (jd->d_dialog!=NULL && jd->d_dialog->state==DIALOG_EARLY)
    {
      osip_dialog_free(jd->d_dialog);
      jd->d_dialog = NULL;
      eXosip_dialog_set_state(jd, JD_TERMINATED);
    }
}


static eXosip_reg_t *
find_reg_for_transaction(osip_transaction_t *tr)
{
  eXosip_reg_t *jr;

  for(jr = eXosip.j_reg; jr; jr = jr->next)
    {
      if (jr->r_last_tr == tr)
	return jr;
    }

  return 0;
}


static void
rcvregister_failure(int type, osip_transaction_t *tr, osip_message_t *sip)
{
  eXosip_event_t *je;
  eXosip_reg_t *jr;

  jr = find_reg_for_transaction(tr);
  if (jr==NULL) return;

  je = eXosip_event_init_for_reg(EXOSIP_REGISTRATION_FAILURE, jr, sip);
  report_event_with_status(je, sip);
}

static void cb_rcv3xx(int type, osip_transaction_t *tr,osip_message_t *sip)
{
  eXosip_dialog_t *jd;
  eXosip_call_t *jc;
  eXosip_subscribe_t *js;
  jinfo_t *jinfo =  (jinfo_t *)osip_transaction_get_your_instance(tr);
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcv3xx (id=%i)\r\n", tr->transactionid));

  if (MSG_IS_RESPONSE_FOR(sip, "REGISTER"))
    {
      rcvregister_failure(type, tr, sip);
      return;
    }

  if (jinfo==NULL) return;
  jd = jinfo->jd;
  jc = jinfo->jc;
  js = jinfo->js;

  if (MSG_IS_RESPONSE_FOR(sip, "OPTIONS"))
    {

      if (jc==NULL)
	{
	  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcv3xx (id=%i) Error: no call or transaction info for INFO transaction\r\n", tr->transactionid));
	  return;
	}
      else if (jc->c_out_options_tr==NULL)
	{
	  /* options is within a call */
	}

      eXosip_report_call_event_with_status(EXOSIP_OPTIONS_REDIRECTED, jc, jd, sip);
      return;
    }

  if (MSG_IS_RESPONSE_FOR(sip, "INVITE"))
    {
      eXosip_report_call_event_with_status(EXOSIP_CALL_REDIRECTED, jc, jd, sip);
    }
  else if (MSG_IS_RESPONSE_FOR(sip, "MESSAGE"))
    {
      eXosip_event_t *je;
      je = eXosip_event_init_for_message(EXOSIP_MESSAGE_FAILURE, tr, sip);
      if (je)
	report_event_with_status(je, sip);
      return;
    }
  else if (MSG_IS_RESPONSE_FOR(sip, "SUBSCRIBE"))
    {
      eXosip_event_t *je;
      je = eXosip_event_init_for_subscribe(EXOSIP_SUBSCRIPTION_REDIRECTED, js, jd);
      if (je)
	report_event_with_status(je, sip);
    }

  if (jd==NULL) return;
  if (MSG_IS_RESPONSE_FOR(sip, "INVITE")
      || MSG_IS_RESPONSE_FOR(sip, "SUBSCRIBE"))
    {
      eXosip_delete_early_dialog(jd);
      if (jd->d_dialog==NULL)
	jd->d_STATE = JD_REDIRECTED;
    }

}

static void cb_rcv4xx(int type, osip_transaction_t *tr,osip_message_t *sip)
{
  eXosip_dialog_t *jd;
  eXosip_call_t *jc;
  eXosip_subscribe_t *js;
  jinfo_t *jinfo =  (jinfo_t *)osip_transaction_get_your_instance(tr);
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcv4xx (id=%i)\r\n", tr->transactionid));


  if (MSG_IS_RESPONSE_FOR(sip, "REGISTER"))
    {
      rcvregister_failure(type, tr, sip);
      return;
    }

  if (jinfo==NULL)
    return;
  jd = jinfo->jd;
  jc = jinfo->jc;
  js = jinfo->js;

  if (MSG_IS_RESPONSE_FOR(sip, "OPTIONS"))
    {

      if (jc==NULL)
	{
	  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcv4xx (id=%i) Error: no call or transaction info for INFO transaction\r\n", tr->transactionid));
	  return;
	}
      else if (jc->c_out_options_tr==NULL)
	{
	  /* options is within a call */
	}

      eXosip_report_call_event_with_status(EXOSIP_OPTIONS_REQUESTFAILURE, jc, jd, sip);
      return;
    }

  if (sip->status_code == 401 || sip->status_code == 407 )
    {
	if (eXosip_retry_with_auth_info(tr,sip) == 0)
	  {
	  if (jd != 0)
	    {
	      jd->d_STATE = JD_AUTH_REQUIRED;
	    }
	  return;
	  }
    }

  /*
     in the case when the status code is not auth related or
     the call to eXosip_retry_with_auth_info failed for some reason
     (for example because we've already tried sending a request with our credentials)
     we deliver the error event to application
  */

  if (MSG_IS_RESPONSE_FOR(sip, "INVITE"))
  {
    eXosip_report_call_event_with_status(EXOSIP_CALL_REQUESTFAILURE, jc, jd, sip);
  }
  else if (MSG_IS_RESPONSE_FOR(sip, "REFER"))
  {
    eXosip_report_call_event_with_status(EXOSIP_CALL_REFER_FAILURE, jc, jd, sip);
  }
  else if (MSG_IS_RESPONSE_FOR(sip, "MESSAGE"))
  {
    eXosip_event_t *je;
    je = eXosip_event_init_for_message(EXOSIP_MESSAGE_FAILURE, tr, sip);
    if (je!=NULL)
      report_event_with_status(je, sip);
    return;
  }
  else if (MSG_IS_RESPONSE_FOR(sip, "SUBSCRIBE"))
  {
    eXosip_event_t *je;
    je = eXosip_event_init_for_subscribe(EXOSIP_SUBSCRIPTION_REQUESTFAILURE, js, jd);
	if (je!=NULL) {

		//<MINHPQ>
		char *tmp;
		if (sip->from != NULL)
			{
			osip_from_to_str(sip->to, &tmp);
			if (tmp!=NULL)	{
				snprintf(je->remote_uri, 255, "%s", tmp);
				osip_free(tmp);
				}
			}
		if (sip->to != NULL) {
			osip_to_to_str(sip->from, &tmp);
			if (tmp!=NULL) {
				snprintf(je->local_uri, 255, "%s", tmp);
				osip_free(tmp);
				}
			}
		//</MINHPQ>
        report_event_with_status(je, sip);
	    }
  }
  if (jd==NULL) return;
  if (MSG_IS_RESPONSE_FOR(sip, "INVITE")
      || MSG_IS_RESPONSE_FOR(sip, "SUBSCRIBE"))
    {
      eXosip_delete_early_dialog(jd);
      if (MSG_TEST_CODE(sip, 401) || MSG_TEST_CODE(sip, 407))
	jd->d_STATE = JD_AUTH_REQUIRED;
      else
	jd->d_STATE = JD_CLIENTERROR;
    }

}

static void cb_rcv5xx(int type, osip_transaction_t *tr,osip_message_t *sip)
{
  eXosip_dialog_t *jd;
  eXosip_call_t *jc;
  eXosip_subscribe_t *js;
  jinfo_t *jinfo =  (jinfo_t *)osip_transaction_get_your_instance(tr);
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcv5xx (id=%i)\r\n", tr->transactionid));

  if (MSG_IS_RESPONSE_FOR(sip, "REGISTER"))
    {
      rcvregister_failure(type, tr, sip);
      return;
    }

  if (jinfo==NULL)
    return;
  jd = jinfo->jd;
  jc = jinfo->jc;
  js = jinfo->js;

  if (MSG_IS_RESPONSE_FOR(sip, "OPTIONS"))
    {
      if (jc==NULL)
	{
	  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcv5xx (id=%i) Error: no call or transaction info for INFO transaction\r\n", tr->transactionid));
	  return;
	}
      else if (jc->c_out_options_tr==NULL)
	{
	  /* options is within a call */
	}

      eXosip_report_call_event_with_status(EXOSIP_OPTIONS_SERVERFAILURE, jc, jd, sip);
      return;
    }

  if (MSG_IS_RESPONSE_FOR(sip, "INVITE"))
  {
    eXosip_report_call_event_with_status(EXOSIP_CALL_SERVERFAILURE, jc, jd, sip);
  }
  else if (MSG_IS_RESPONSE_FOR(sip, "REFER"))
  {
    eXosip_report_call_event_with_status(EXOSIP_CALL_REFER_FAILURE, jc, jd, sip);
  }
  else if (MSG_IS_RESPONSE_FOR(sip, "MESSAGE"))
  {
      eXosip_event_t *je;
      je = eXosip_event_init_for_message(EXOSIP_MESSAGE_FAILURE, tr, sip);
      if (je!=NULL)
	report_event_with_status(je, sip);
      return;
  }
  else if (MSG_IS_RESPONSE_FOR(sip, "SUBSCRIBE"))
  {
    eXosip_event_t *je;
    je = eXosip_event_init_for_subscribe(EXOSIP_SUBSCRIPTION_SERVERFAILURE, js, jd);
    if (je!=NULL)
      report_event_with_status(je, sip);
  }

  if (jd==NULL) return;
  if (MSG_IS_RESPONSE_FOR(sip, "INVITE")
      || MSG_IS_RESPONSE_FOR(sip, "SUBSCRIBE"))
    {
      eXosip_delete_early_dialog(jd);
      jd->d_STATE = JD_SERVERERROR;
    }

}

static void cb_rcv6xx(int type, osip_transaction_t *tr,osip_message_t *sip)
{
  eXosip_dialog_t *jd;
  eXosip_call_t *jc;
  eXosip_subscribe_t *js;
  jinfo_t *jinfo =  (jinfo_t *)osip_transaction_get_your_instance(tr);
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcv6xx (id=%i)\r\n", tr->transactionid));

  if (MSG_IS_RESPONSE_FOR(sip, "REGISTER"))
    {
      rcvregister_failure(type, tr, sip);
      return;
    }

  if (jinfo==NULL)
    return;
  jd = jinfo->jd;
  jc = jinfo->jc;
  js = jinfo->js;

  if (MSG_IS_RESPONSE_FOR(sip, "OPTIONS"))
    {

      if (jc==NULL)
	{
	  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcv6xx (id=%i) Error: no call or transaction info for INFO transaction\r\n", tr->transactionid));
	  return;
	}
      else if (jc->c_out_options_tr==NULL)
	{
	  /* options is within a call */
	}
      eXosip_report_call_event_with_status(EXOSIP_OPTIONS_GLOBALFAILURE, jc, jd, sip);
      return;
    }

  if (MSG_IS_RESPONSE_FOR(sip, "INVITE"))
  {
      eXosip_report_call_event_with_status(EXOSIP_CALL_GLOBALFAILURE, jc, jd, sip);
  }
  else if (MSG_IS_RESPONSE_FOR(sip, "REFER"))
  {
    eXosip_report_call_event_with_status(EXOSIP_CALL_REFER_FAILURE, jc, jd, sip);
  }
  else if (MSG_IS_RESPONSE_FOR(sip, "MESSAGE"))
  {
      eXosip_event_t *je;
      je = eXosip_event_init_for_message(EXOSIP_MESSAGE_FAILURE, tr, sip);
      if (je!=NULL)
	report_event_with_status(je, sip);
      return;
  }
  else if (MSG_IS_RESPONSE_FOR(sip, "SUBSCRIBE"))
  {
    eXosip_event_t *je;
    je = eXosip_event_init_for_subscribe(EXOSIP_SUBSCRIPTION_GLOBALFAILURE, js, jd);
    if (je!=NULL)
      report_event_with_status(je, sip);
  }

  if (jd==NULL) return;
  if (MSG_IS_RESPONSE_FOR(sip, "INVITE")
      || MSG_IS_RESPONSE_FOR(sip, "SUBSCRIBE"))
    {
      eXosip_delete_early_dialog(jd);
      jd->d_STATE = JD_GLOBALFAILURE;
    }

}

static void cb_snd1xx(int type, osip_transaction_t *tr,osip_message_t *sip)
{
  eXosip_dialog_t *jd;
  eXosip_call_t *jc;
  jinfo_t *jinfo =  (jinfo_t *)osip_transaction_get_your_instance(tr);
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_snd1xx (id=%i)\r\n", tr->transactionid));

  if (jinfo==NULL)
    return;
  jd = jinfo->jd;
  jc = jinfo->jc;
  if (jd==NULL) return;
  jd->d_STATE = JD_TRYING;
}

static void cb_snd2xx(int type, osip_transaction_t *tr,osip_message_t *sip)
{
  eXosip_dialog_t *jd;
  eXosip_call_t *jc;
  jinfo_t *jinfo =  (jinfo_t *)osip_transaction_get_your_instance(tr);
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_snd2xx (id=%i)\r\n", tr->transactionid));
  if (jinfo==NULL)
    return;
  jd = jinfo->jd;
  jc = jinfo->jc;
  if (jd==NULL) return;
  if (MSG_IS_RESPONSE_FOR(sip, "INVITE")
      || MSG_IS_RESPONSE_FOR(sip, "SUBSCRIBE"))
    {
      jd->d_STATE = JD_ESTABLISHED;
      return;
    }
  jd->d_STATE = JD_ESTABLISHED;
}

static void cb_snd3xx(int type, osip_transaction_t *tr,osip_message_t *sip)
{
  eXosip_dialog_t *jd;
  eXosip_call_t *jc;
  jinfo_t *jinfo =  (jinfo_t *)osip_transaction_get_your_instance(tr);
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_snd3xx (id=%i)\r\n", tr->transactionid));
  if (jinfo==NULL)
    return;
  jd = jinfo->jd;
  jc = jinfo->jc;
  if (jd==NULL) return;
  if (MSG_IS_RESPONSE_FOR(sip, "INVITE")
      || MSG_IS_RESPONSE_FOR(sip, "SUBSCRIBE"))
    {
      eXosip_delete_early_dialog(jd);
    }
  jd->d_STATE = JD_REDIRECTED;

  if (MSG_IS_RESPONSE_FOR(sip, "INVITE"))
  {
    eXosip_report_call_event_with_status(EXOSIP_CALL_CLOSED, jc, jd, sip);
  }
}

static void cb_snd4xx(int type, osip_transaction_t *tr,osip_message_t *sip)
{
  eXosip_dialog_t *jd;
  eXosip_call_t *jc;
  jinfo_t *jinfo =  (jinfo_t *)osip_transaction_get_your_instance(tr);
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_snd4xx (id=%i)\r\n", tr->transactionid));
  if (jinfo==NULL)
    return;
  jd = jinfo->jd;
  jc = jinfo->jc;
  if (jd==NULL) return;
  if (MSG_IS_RESPONSE_FOR(sip, "INVITE")
      || MSG_IS_RESPONSE_FOR(sip, "SUBSCRIBE"))
    {
      eXosip_delete_early_dialog(jd);
    }
  jd->d_STATE = JD_CLIENTERROR;

  if (MSG_IS_RESPONSE_FOR(sip, "INVITE"))
  {
    eXosip_report_call_event_with_status(EXOSIP_CALL_CLOSED, jc, jd, sip);
  }

}

static void cb_snd5xx(int type, osip_transaction_t *tr,osip_message_t *sip)
{
  eXosip_dialog_t *jd;
  eXosip_call_t *jc;
  jinfo_t *jinfo =  (jinfo_t *)osip_transaction_get_your_instance(tr);
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_snd5xx (id=%i)\r\n", tr->transactionid));
  if (jinfo==NULL)
    return;
  jd = jinfo->jd;
  jc = jinfo->jc;
  if (jd==NULL) return;
  if (MSG_IS_RESPONSE_FOR(sip, "INVITE")
      || MSG_IS_RESPONSE_FOR(sip, "SUBSCRIBE"))
    {
      eXosip_delete_early_dialog(jd);
    }
  jd->d_STATE = JD_SERVERERROR;

  if (MSG_IS_RESPONSE_FOR(sip, "INVITE"))
  {
    eXosip_report_call_event_with_status(EXOSIP_CALL_CLOSED, jc, jd, sip);
  }

}

static void cb_snd6xx(int type, osip_transaction_t *tr,osip_message_t *sip)
{
  eXosip_dialog_t *jd;
  eXosip_call_t *jc;
  jinfo_t *jinfo =  (jinfo_t *)osip_transaction_get_your_instance(tr);
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_snd6xx (id=%i)\r\n", tr->transactionid));
  if (jinfo==NULL)
    return;
  jd = jinfo->jd;
  jc = jinfo->jc;
  if (jd==NULL) return;
  if (MSG_IS_RESPONSE_FOR(sip, "INVITE")
      || MSG_IS_RESPONSE_FOR(sip, "SUBSCRIBE"))
    {
      eXosip_delete_early_dialog(jd);
    }
  jd->d_STATE = JD_GLOBALFAILURE;

  if (MSG_IS_RESPONSE_FOR(sip, "INVITE"))
  {
    eXosip_report_call_event_with_status(EXOSIP_CALL_CLOSED, jc, jd, sip);
  }

}

static void cb_rcvresp_retransmission(int type, osip_transaction_t *tr, osip_message_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcvresp_retransmission (id=%i)\r\n", tr->transactionid));
}

static void cb_sndreq_retransmission(int type, osip_transaction_t *tr, osip_message_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_sndreq_retransmission (id=%i)\r\n", tr->transactionid));
}

static void cb_sndresp_retransmission(int type, osip_transaction_t *tr, osip_message_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_sndresp_retransmission (id=%i)\r\n", tr->transactionid));
}

static void cb_rcvreq_retransmission(int type, osip_transaction_t *tr, osip_message_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcvreq_retransmission (id=%i)\r\n", tr->transactionid));
}

static void cb_transport_error(int type, osip_transaction_t *tr, int error)
{
  eXosip_dialog_t    *jd;
  eXosip_call_t      *jc;
  eXosip_subscribe_t *js;
  eXosip_notify_t    *jn;
  jinfo_t *jinfo =  (jinfo_t *)osip_transaction_get_your_instance(tr);


  if (type==OSIP_NICT_TRANSPORT_ERROR)
    return;

  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_transport_error (id=%i)\r\n", tr->transactionid));
  if (jinfo==NULL)
    return;
  jd = jinfo->jd;
  jc = jinfo->jc;
  jn = jinfo->jn;
  js = jinfo->js;

  if (jn==NULL && js==NULL)
    return;

  if (MSG_IS_NOTIFY(tr->orig_request)
      && type==OSIP_NICT_TRANSPORT_ERROR)
    {
      /* delete the dialog! */
      REMOVE_ELEMENT(eXosip.j_notifies, jn);
      eXosip_notify_free(jn);
    }

  if (MSG_IS_SUBSCRIBE(tr->orig_request)
      && type==OSIP_NICT_TRANSPORT_ERROR)
    {
      /* delete the dialog! */
      REMOVE_ELEMENT(eXosip.j_subscribes, js);
      eXosip_subscribe_free(js);
    }

  if (MSG_IS_OPTIONS(tr->orig_request) && jc->c_dialogs==NULL
      && type==OSIP_NICT_TRANSPORT_ERROR)
    {
      /* delete the dialog! */
      REMOVE_ELEMENT(eXosip.j_calls, jc);
      eXosip_call_free(jc);
    }
}



int
eXosip_set_callbacks(osip_t *osip)
{
  /* register all callbacks */

  osip_set_cb_send_message(osip, &cb_udp_snd_message);

  osip_set_kill_transaction_callback(osip ,OSIP_ICT_KILL_TRANSACTION,
				 &cb_ict_kill_transaction);
  osip_set_kill_transaction_callback(osip ,OSIP_IST_KILL_TRANSACTION,
				 &cb_ist_kill_transaction);
  osip_set_kill_transaction_callback(osip ,OSIP_NICT_KILL_TRANSACTION,
				 &cb_nict_kill_transaction);
  osip_set_kill_transaction_callback(osip ,OSIP_NIST_KILL_TRANSACTION,
				 &cb_nist_kill_transaction);

  osip_set_message_callback(osip ,OSIP_ICT_STATUS_2XX_RECEIVED_AGAIN,
			&cb_rcvresp_retransmission);
  osip_set_message_callback(osip ,OSIP_ICT_STATUS_3456XX_RECEIVED_AGAIN,
			&cb_rcvresp_retransmission);
  osip_set_message_callback(osip ,OSIP_ICT_INVITE_SENT_AGAIN,
			&cb_sndreq_retransmission);
  osip_set_message_callback(osip ,OSIP_IST_STATUS_2XX_SENT_AGAIN,
			&cb_sndresp_retransmission);
  osip_set_message_callback(osip ,OSIP_IST_STATUS_3456XX_SENT_AGAIN,
			&cb_sndresp_retransmission);
  osip_set_message_callback(osip ,OSIP_IST_INVITE_RECEIVED_AGAIN,
			&cb_rcvreq_retransmission);
  osip_set_message_callback(osip ,OSIP_NICT_STATUS_2XX_RECEIVED_AGAIN,
			&cb_rcvresp_retransmission);
  osip_set_message_callback(osip ,OSIP_NICT_STATUS_3456XX_RECEIVED_AGAIN,
			&cb_rcvresp_retransmission);
  osip_set_message_callback(osip ,OSIP_NICT_REQUEST_SENT_AGAIN,
			&cb_sndreq_retransmission);
  osip_set_message_callback(osip ,OSIP_NIST_STATUS_2XX_SENT_AGAIN,
			&cb_sndresp_retransmission);
  osip_set_message_callback(osip ,OSIP_NIST_STATUS_3456XX_SENT_AGAIN,
			&cb_sndresp_retransmission);
  osip_set_message_callback(osip ,OSIP_NIST_REQUEST_RECEIVED_AGAIN,
			&cb_rcvreq_retransmission);

  osip_set_transport_error_callback(osip ,OSIP_ICT_TRANSPORT_ERROR,
				    &cb_transport_error);
  osip_set_transport_error_callback(osip ,OSIP_IST_TRANSPORT_ERROR,
				    &cb_transport_error);
  osip_set_transport_error_callback(osip ,OSIP_NICT_TRANSPORT_ERROR,
				    &cb_transport_error);
  osip_set_transport_error_callback(osip ,OSIP_NIST_TRANSPORT_ERROR,
				    &cb_transport_error);

  osip_set_message_callback(osip ,OSIP_ICT_INVITE_SENT,     &cb_sndinvite);
  osip_set_message_callback(osip ,OSIP_ICT_ACK_SENT,        &cb_sndack);
  osip_set_message_callback(osip ,OSIP_NICT_REGISTER_SENT,  &cb_sndregister);
  osip_set_message_callback(osip ,OSIP_NICT_BYE_SENT,       &cb_sndbye);
  osip_set_message_callback(osip ,OSIP_NICT_CANCEL_SENT,    &cb_sndcancel);
  osip_set_message_callback(osip ,OSIP_NICT_INFO_SENT,      &cb_sndinfo);
  osip_set_message_callback(osip ,OSIP_NICT_OPTIONS_SENT,   &cb_sndoptions);
  osip_set_message_callback(osip ,OSIP_NICT_SUBSCRIBE_SENT, &cb_sndsubscribe);
  osip_set_message_callback(osip ,OSIP_NICT_NOTIFY_SENT,    &cb_sndnotify);
  /*  osip_set_cb_nict_sndprack   (osip,&cb_sndprack); */
  osip_set_message_callback(osip ,OSIP_NICT_UNKNOWN_REQUEST_SENT, &cb_sndunkrequest);

  osip_set_message_callback(osip ,OSIP_ICT_STATUS_1XX_RECEIVED, &cb_rcv1xx);
  osip_set_message_callback(osip ,OSIP_ICT_STATUS_2XX_RECEIVED, &cb_rcv2xx);
  osip_set_message_callback(osip ,OSIP_ICT_STATUS_3XX_RECEIVED, &cb_rcv3xx);
  osip_set_message_callback(osip ,OSIP_ICT_STATUS_4XX_RECEIVED, &cb_rcv4xx);
  osip_set_message_callback(osip ,OSIP_ICT_STATUS_5XX_RECEIVED, &cb_rcv5xx);
  osip_set_message_callback(osip ,OSIP_ICT_STATUS_6XX_RECEIVED, &cb_rcv6xx);

  osip_set_message_callback(osip ,OSIP_IST_STATUS_1XX_SENT, &cb_snd1xx);
  osip_set_message_callback(osip ,OSIP_IST_STATUS_2XX_SENT, &cb_snd2xx);
  osip_set_message_callback(osip ,OSIP_IST_STATUS_3XX_SENT, &cb_snd3xx);
  osip_set_message_callback(osip ,OSIP_IST_STATUS_4XX_SENT, &cb_snd4xx);
  osip_set_message_callback(osip ,OSIP_IST_STATUS_5XX_SENT, &cb_snd5xx);
  osip_set_message_callback(osip ,OSIP_IST_STATUS_6XX_SENT, &cb_snd6xx);

  osip_set_message_callback(osip ,OSIP_NICT_STATUS_1XX_RECEIVED, &cb_rcv1xx);
  osip_set_message_callback(osip ,OSIP_NICT_STATUS_2XX_RECEIVED, &cb_rcv2xx);
  osip_set_message_callback(osip ,OSIP_NICT_STATUS_3XX_RECEIVED, &cb_rcv3xx);
  osip_set_message_callback(osip ,OSIP_NICT_STATUS_4XX_RECEIVED, &cb_rcv4xx);
  osip_set_message_callback(osip ,OSIP_NICT_STATUS_5XX_RECEIVED, &cb_rcv5xx);
  osip_set_message_callback(osip ,OSIP_NICT_STATUS_6XX_RECEIVED, &cb_rcv6xx);

  osip_set_message_callback(osip ,OSIP_NIST_STATUS_1XX_SENT, &cb_snd1xx);
  osip_set_message_callback(osip ,OSIP_NIST_STATUS_2XX_SENT, &cb_snd2xx);
  osip_set_message_callback(osip ,OSIP_NIST_STATUS_3XX_SENT, &cb_snd3xx);
  osip_set_message_callback(osip ,OSIP_NIST_STATUS_4XX_SENT, &cb_snd4xx);
  osip_set_message_callback(osip ,OSIP_NIST_STATUS_5XX_SENT, &cb_snd5xx);
  osip_set_message_callback(osip ,OSIP_NIST_STATUS_6XX_SENT, &cb_snd6xx);

  osip_set_message_callback(osip ,OSIP_IST_INVITE_RECEIVED,     &cb_rcvinvite);
  osip_set_message_callback(osip ,OSIP_IST_ACK_RECEIVED,        &cb_rcvack);
  osip_set_message_callback(osip ,OSIP_IST_ACK_RECEIVED_AGAIN,  &cb_rcvack2);
  osip_set_message_callback(osip ,OSIP_NIST_REGISTER_RECEIVED,  &cb_rcvregister);
  osip_set_message_callback(osip ,OSIP_NIST_BYE_RECEIVED,       &cb_rcvbye);
  osip_set_message_callback(osip ,OSIP_NIST_CANCEL_RECEIVED,    &cb_rcvcancel);
  osip_set_message_callback(osip ,OSIP_NIST_INFO_RECEIVED,      &cb_rcvinfo);
  osip_set_message_callback(osip ,OSIP_NIST_OPTIONS_RECEIVED,   &cb_rcvoptions);
  osip_set_message_callback(osip ,OSIP_NIST_SUBSCRIBE_RECEIVED, &cb_rcvsubscribe);
  osip_set_message_callback(osip ,OSIP_NIST_NOTIFY_RECEIVED,    &cb_rcvnotify);
  osip_set_message_callback(osip ,OSIP_NIST_UNKNOWN_REQUEST_RECEIVED, &cb_rcvunkrequest);


  return 0;
}
