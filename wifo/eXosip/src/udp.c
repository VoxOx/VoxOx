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

#include <stdlib.h>
#include <errno.h>
#include <time.h>

#ifdef ENABLE_MPATROL
#include <mpatrol.h>
#endif

#include "eXosip2.h"
#include <eXosip/eXosip.h>
#include <eXosip/eXosip_cfg.h>

#if ! (defined(_WIN32_WCE) || defined(WIN32))
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#ifdef __APPLE_CC__
#include <unistd.h>
#endif
#else
#include <windows.h>
#endif

extern eXosip_t eXosip;
#if defined(WIN32) || defined(_WIN32_WCE)
#define strcasecmp stricmp
#endif

/* Private functions */
static void eXosip_send_default_answer(eXosip_dialog_t *jd,
				       osip_transaction_t *transaction,
				       osip_event_t *evt,
				       int status,
				       char *reason_phrase,
				       char *warning,
				       int line);
static void eXosip_process_info(eXosip_call_t *jc, eXosip_dialog_t *jd,
			        osip_transaction_t *transaction, osip_event_t *evt);
static void eXosip_process_options(eXosip_call_t *jc, eXosip_dialog_t *jd,
				   osip_transaction_t *transaction, osip_event_t *evt);
static void eXosip_process_bye(eXosip_call_t *jc, eXosip_dialog_t *jd,
			       osip_transaction_t *transaction, osip_event_t *evt);
static void eXosip_process_refer(eXosip_call_t *jc, eXosip_dialog_t *jd,
				 osip_transaction_t *transaction, osip_event_t *evt);
static void eXosip_process_ack(eXosip_call_t *jc, eXosip_dialog_t *jd, osip_event_t *evt);
static int cancel_match_invite(osip_transaction_t *invite, osip_message_t *cancel);
static void eXosip_process_cancel(osip_transaction_t *transaction, osip_event_t *evt);
static osip_event_t *eXosip_process_reinvite(eXosip_call_t *jc, eXosip_dialog_t *jd,
					     osip_transaction_t *transaction,
					     osip_event_t *evt, sdp_message_t *remote_sdp);
static void eXosip_process_invite_on_hold(eXosip_call_t *jc, eXosip_dialog_t *jd,
					  osip_transaction_t *transaction,
					  osip_event_t *evt, sdp_message_t *sdp);
static void eXosip_process_invite_off_hold(eXosip_call_t *jc, eXosip_dialog_t *jd,
					   osip_transaction_t *transaction,
					   osip_event_t *evt, sdp_message_t *sdp);
static void eXosip_process_new_options(osip_transaction_t *transaction, osip_event_t *evt);
static void eXosip_process_new_invite(osip_transaction_t *transaction, osip_event_t *evt);
static void eXosip_process_invite_within_call(eXosip_call_t *jc, eXosip_dialog_t *jd,
					      osip_transaction_t *transaction, osip_event_t *evt);
static int eXosip_event_package_is_supported(osip_transaction_t *transaction,
					     osip_event_t *evt);
static void eXosip_process_new_subscribe(osip_transaction_t *transaction,
					 osip_event_t *evt);
static void eXosip_process_subscribe_within_call(eXosip_notify_t *jn,
						 eXosip_dialog_t *jd,
						 osip_transaction_t *transaction,
						 osip_event_t *evt);
static void eXosip_process_notify_within_dialog(eXosip_subscribe_t *js,
						eXosip_dialog_t *jd,
						osip_transaction_t *transaction,
						osip_event_t *evt);
static int eXosip_match_notify_for_subscribe(eXosip_subscribe_t *js, osip_message_t *notify);
static void eXosip_process_newrequest(osip_event_t *evt);
static void eXosip_process_response_out_of_transaction(osip_event_t *evt);
static int eXosip_pendingosip_transaction_exist(eXosip_call_t *jc, eXosip_dialog_t *jd);
static int eXosip_release_finished_calls(eXosip_call_t *jc, eXosip_dialog_t *jd);
static int eXosip_release_aborted_calls(eXosip_call_t *jc, eXosip_dialog_t *jd);

// <ncouturier>
osip_content_type_t * copy_content_type(osip_content_type_t * ctt_src);
// </ncouturier>


static void eXosip_send_default_answer(eXosip_dialog_t *jd,
				       osip_transaction_t *transaction,
				       osip_event_t *evt,
				       int status,
				       char *reason_phrase,
				       char *warning,
				       int line)
{
  osip_event_t *evt_answer;
  osip_message_t *answer;
  int i;
  
  /*owsip_list_add_nodup(eXosip.j_transactions, transaction, 0); */

  /* tmonjalo: don't know why set this to NULL ?
     commented because it was erasing account of transaction */
  /* osip_transaction_set_your_instance(transaction, NULL); */
  
  /* THIS METHOD DOES NOT ACCEPT STATUS CODE BETWEEN 101 and 299 */
  if (status>100 && status<299 && MSG_IS_INVITE(evt->sip))
    return ;
  
  if (jd!=NULL)
    i = _eXosip_build_response_default(&answer, jd->d_dialog, status, evt->sip);
  else
    i = _eXosip_build_response_default(&answer, NULL, status, evt->sip);

  if (i!=0 || answer==NULL)
    {
      return ;
    }

  if (reason_phrase!=NULL)
    {
      char *_reason;
      _reason = osip_message_get_reason_phrase(answer);
      if (_reason!=NULL)
	osip_free(_reason);
      _reason = osip_strdup(reason_phrase);
      osip_message_set_reason_phrase(answer, _reason);
    }

  if (jd && jd->d_localcontact)
    osip_message_set_contact(answer, jd->d_localcontact);

  osip_message_set_content_length(answer, "0");
  
  if (status==500)
    osip_message_set_retry_after(answer, "10");
  
  evt_answer = osip_new_outgoing_sipmessage(answer);
  evt_answer->transactionid =  transaction->transactionid;
  osip_transaction_add_event(transaction,evt_answer);
  __eXosip_wakeup();
  
}

static void send_default_answer(eXosip_dialog_t *jd,
				       osip_transaction_t *transaction,
				       osip_event_t *evt,
				       int status,
				       char *reason_phrase,
				       char *warning,
				       int line)
{

      owsip_list_add_nodup(eXosip.j_transactions, transaction, 0);
      eXosip_send_default_answer(jd, transaction, evt, status, reason_phrase, warning, line);

}

static void
__eXosip_report_event(int evt, eXosip_call_t *jc, eXosip_dialog_t *jd, eXosip_event_t *je )
{
  if (!je)
    je = eXosip_event_init_for_call(evt, jc, jd);

  if (eXosip.j_call_callbacks[evt]!=NULL)
    eXosip.j_call_callbacks[evt](evt, je);
  else if (eXosip.j_runtime_mode==EVENT_MODE)
    eXosip_event_add(je);
}


static void eXosip_process_options(eXosip_call_t *jc, eXosip_dialog_t *jd,
				   osip_transaction_t *transaction, osip_event_t *evt)
{
  osip_event_t *evt_answer;
  osip_message_t *answer;
  int i;

  osip_transaction_set_your_instance(transaction,
	  __eXosip_new_jinfo(
	                        owsip_dialog_account_get (jd),
	                        jc,
							NULL /*jd */,
							NULL,
							NULL));
  i = _eXosip_build_response_default(&answer, jd->d_dialog, 200, evt->sip);
  if (i!=0)
    {
      owsip_list_add_nodup(eXosip.j_transactions, transaction, 0);
      return ;
    }
    
  evt_answer = osip_new_outgoing_sipmessage(answer);
  evt_answer->transactionid =  transaction->transactionid;

  osip_list_add(jd->d_inc_trs, transaction , 0);

  osip_transaction_add_event(transaction,evt_answer);
  __eXosip_wakeup();
}

static void eXosip_process_info(eXosip_call_t *jc, eXosip_dialog_t *jd,
 		    osip_transaction_t *transaction, osip_event_t *evt)
{
  osip_event_t *evt_answer;
  osip_message_t *answer;
  int i;
  
  osip_transaction_set_your_instance
	(
		transaction,
		__eXosip_new_jinfo
		(
			owsip_dialog_account_get (jd),
			jc,
			jd,
			NULL,
			NULL
		)
	) ;
  /* for now, send default response of 200ok.  eventually, application should
      be deciding how to answer INFO messages */
  i = _eXosip_build_response_default(&answer, jd->d_dialog, 200, evt->sip);
  if (i!=0)
    {
      owsip_list_add_nodup(eXosip.j_transactions, transaction, 0);
       return ;
    }

  evt_answer = osip_new_outgoing_sipmessage(answer);
  evt_answer->transactionid =  transaction->transactionid;
  
  osip_list_add(jd->d_inc_trs, transaction , 0);
  
  osip_transaction_add_event(transaction,evt_answer);
  __eXosip_wakeup();
}
 

static void eXosip_process_bye(eXosip_call_t *jc, eXosip_dialog_t *jd,
			       osip_transaction_t *transaction, osip_event_t *evt)
{
  osip_event_t *evt_answer;
  osip_message_t *answer;
  int i;

  osip_transaction_set_your_instance
	(
		transaction,
		__eXosip_new_jinfo
		(
			owsip_dialog_account_get (jd),
			jc,
			NULL /*jd */,
			NULL,
			NULL
		)
	) ;

  i = _eXosip_build_response_default(&answer, jd->d_dialog, 200, evt->sip);
  if (i!=0)
    {
      owsip_list_add_nodup(eXosip.j_transactions, transaction, 0);
      return ;
    }
  osip_message_set_content_length(answer, "0");
    
  evt_answer = osip_new_outgoing_sipmessage(answer);
  evt_answer->transactionid =  transaction->transactionid;

  osip_list_add(jd->d_inc_trs, transaction , 0);

  /* Release the eXosip_dialog */
  osip_dialog_free(jd->d_dialog);
  jd->d_dialog = NULL;
  __eXosip_report_event(EXOSIP_CALL_CLOSED, jc, jd, NULL);

  osip_transaction_add_event(transaction,evt_answer);
  __eXosip_wakeup();
}


static void eXosip_post_outgoing_msg(osip_list_t *list, osip_transaction_t *tr, osip_message_t *sip)
{
  osip_event_t *evt;


  evt = osip_new_outgoing_sipmessage(sip);
  evt->transactionid =  tr->transactionid;

  if (list)
    osip_list_add(list, tr , 0);
  
  osip_transaction_add_event(tr, evt);
  __eXosip_wakeup();

}

static void eXosip_process_refer(eXosip_call_t *jc, eXosip_dialog_t *jd,
				 osip_transaction_t *transaction, osip_event_t *evt)
{
  osip_message_t *answer;
  osip_header_t *referto_head = NULL;
  osip_contact_t *referto;
  int i;

  /* check if the refer is valid */
  osip_message_header_get_byname(evt->sip, "refer-to", 0, &referto_head);
  if (referto_head==NULL || referto_head->hvalue==NULL)
    {
      send_default_answer(jd, transaction, evt, 400, "Missing Refer-To header", "Missing Refer-To header", __LINE__);
      return;
    }
  /* check if refer-to is well-formed */
  osip_contact_init(&referto);
  i = osip_contact_parse(referto, referto_head->hvalue);
  if (i!=0)
    {
      osip_contact_free(referto);
      send_default_answer(jd, transaction, evt, 400, "Non valid Refer-To header", "Non valid Refer-To header", __LINE__);
      return;
    }
  if (0!=osip_strcasecmp(referto->url->scheme, "sip"))
    {
      osip_contact_free(referto);
      send_default_answer(jd, transaction, evt, 501, "Scheme Not Implemented", "Scheme Not Implemented", __LINE__);
      return;
    }

  osip_contact_free(referto);

  /* check policy so we can decline immediatly the refer */

  osip_transaction_set_your_instance
	(
		transaction,
		__eXosip_new_jinfo
		(
			owsip_dialog_account_get (jd),
			jc,
			jd,
			NULL,
			NULL
		)
	) ;


  i = _eXosip_build_response_default(&answer, jd->d_dialog, 100, evt->sip);
  if (i!=0)
    {
      owsip_list_add_nodup(eXosip.j_transactions, transaction, 0);
       return ;
    }


#if 0  
  i = complete_answer_that_establish_a_dialog(answer, evt->sip);
#endif

  eXosip_post_outgoing_msg(jd->d_inc_trs, transaction, answer);






#if 0
  _eXosip_transfer_send_notify(jc, jd, EXOSIP_SUBCRSTATE_ACTIVE, "SIP/2.0 100 Trying");  
#endif
}

static int ctype_is(const osip_content_type_t *ctype, const char *type, const char *subtype)
{
  return !osip_strcasecmp(ctype->type, type) && !osip_strcasecmp(ctype->subtype, subtype);
}


static void eXosip_process_notify_for_refer(eXosip_call_t *jc, eXosip_dialog_t *jd,
					    osip_transaction_t *transaction, osip_event_t *evt)
{
  osip_event_t *evt_answer;
  osip_message_t *answer;
  int i;
  osip_transaction_t *ref;
  osip_header_t *event_hdr;
  osip_header_t *sub_state;
  osip_content_type_t *ctype;
  osip_body_t *body = NULL;

  /* get the event type and return "489 Bad Event". */
  osip_message_header_get_byname(evt->sip, "event", 0, &event_hdr);
  if (event_hdr==NULL || event_hdr->hvalue==NULL)
    {
        send_default_answer(jd, transaction, evt, 481, "Missing Event header in Notify", "Missing Event header in Notify", __LINE__);
      return ;
    }
  if (0!=osip_strncasecmp(event_hdr->hvalue, "refer", 5))
    {
      send_default_answer(jd, transaction, evt, 501, "Unsupported Event header", "Unsupported Event header in Notify", __LINE__);
      return ;
    }
  osip_message_header_get_byname(evt->sip, "subscription-state",
				 0,
				 &sub_state);

  if (sub_state==NULL||sub_state->hvalue==NULL)
    {
      send_default_answer(jd, transaction, evt, 400, NULL, NULL, __LINE__);
      return;
    }

  /* we are going to accept bodyless responses */
  ctype = osip_message_get_content_type(evt->sip);

  if ( ( ctype == NULL ) || ( ctype->type == NULL ) || ( ctype->subtype == NULL) ) {
      send_default_answer(jd, transaction, evt, 400, "Missing Header", "Missing Content-Type Header", __LINE__);
      return ;      
  }

  if ( ! ( ctype_is(ctype, "message", "sipfrag") ) && ! ( ctype_is(ctype, "application", "sip") ) ) {
      send_default_answer(jd, transaction, evt, 501, "Unsupported body type", "Unsupported body type", __LINE__);
      return ;
  }

  osip_message_get_body(evt->sip, 0, &body);
  if ( ( body == NULL ) || ( body->body == NULL ) ) {
      send_default_answer(jd, transaction, evt, 400, "Missing Body", "Missing Body", __LINE__);
      return ;
  }

  
  {
    eXosip_event_t *je;

    je = eXosip_event_init_for_call(EXOSIP_CALL_REFER_STATUS, jc, jd);
    if (je==NULL) return;
    
    if (body != 0 && body->body != 0)
      {
        if (je->msg_body)
            osip_free(je->msg_body);
       
	je->msg_body = osip_strdup(body->body);
        if (je->msg_body == NULL) 
	  {
            eXosip_event_free(je);
            return;
	  }
      }

    if (sub_state && sub_state->hvalue != 0)
      {
	if (!osip_strncasecmp(sub_state->hvalue, "terminated", 10))
	  je->ss_status = EXOSIP_SUBCRSTATE_TERMINATED;
	else
	  je->ss_status = EXOSIP_SUBCRSTATE_ACTIVE;
      }
    else
      je->ss_status = EXOSIP_SUBCRSTATE_TERMINATED;

    __eXosip_report_event(EXOSIP_CALL_REFER_STATUS, NULL, NULL, je);
  }

  /* check if a refer was sent previously! */
  ref = eXosip_find_last_out_refer(jc, jd);
  if (ref==NULL)
    {
      send_default_answer(jd, transaction, evt, 481, NULL, "Not associated refer", __LINE__);
      return ;
    }

  osip_transaction_set_your_instance
	(
		transaction,
		__eXosip_new_jinfo
		(
			owsip_dialog_account_get (jd),
			jc,
			jd,
			NULL,
			NULL
		)
	);
  /* for now, send default response of 200ok.  eventually, application should
      be deciding how to answer INFO messages */
  i = _eXosip_build_response_default(&answer, jd->d_dialog, 200, evt->sip);
  if (i!=0)
    {
      owsip_list_add_nodup(eXosip.j_transactions, transaction, 0);
      return ;
    }
  
  i = complete_answer_that_establish_a_dialog2(answer, evt->sip, jd->d_localcontact);

  evt_answer = osip_new_outgoing_sipmessage(answer);
  evt_answer->transactionid = transaction->transactionid;
  
  osip_list_add(jd->d_inc_trs, transaction , 0);
  
  osip_transaction_add_event(transaction,evt_answer);
  __eXosip_wakeup();
}

static void eXosip_process_ack(eXosip_call_t *jc, eXosip_dialog_t *jd, osip_event_t *evt)
{
  eXosip_event_t *je;

  /* stop ACK retransmission, in case there is any */
  jd->d_count=0;
  osip_message_free(jd->d_200Ok);
  jd->d_200Ok=NULL;

  je = eXosip_event_init_for_call(EXOSIP_CALL_ACK, jc, jd);
  if (je==NULL) return;

  /* MBW - for SDP in ACK used in alternate SDP offer-response model */
  eXosip_event_add_sdp_info(je, evt->sip);
  __eXosip_report_event(EXOSIP_CALL_ACK, NULL, NULL, je);

  osip_event_free(evt);
}

static int cancel_match_invite(osip_transaction_t *invite, osip_message_t *cancel)
{
	osip_generic_param_t *br;
	osip_generic_param_t *br2;
	osip_via_t *via;

	osip_via_param_get_byname (invite->topvia, "branch", &br);
	via = osip_list_get(&cancel->vias, 0);
	if (via==NULL) 
		return -1; /* request without via??? */
	osip_via_param_get_byname (via, "branch", &br2);
	if (br != NULL && br2 == NULL)
		return -1;
	if (br2 != NULL && br == NULL)
		return -1;
	if (br2 != NULL && br != NULL) /* compliant UA  :) */
	{
		if (br->gvalue != NULL && br2->gvalue != NULL &&
			0 == strcmp(br->gvalue, br2->gvalue))
			return 0;
		return -1;
	}
	/* old backward compatibility mechanism */
	if (0 != osip_call_id_match (invite->callid, cancel->call_id))
		return -1;
	if (0 != osip_to_tag_match (invite->to, cancel->to))
		return -1;
	if (0 != osip_from_tag_match (invite->from, cancel->from))
		return -1;
	if (0 != osip_via_match (invite->topvia, via))
		return -1;
	return 0;
}

static void eXosip_process_cancel(osip_transaction_t *transaction, osip_event_t *evt)
{
	osip_transaction_t *tr;
	osip_event_t *evt_answer;
	osip_message_t *answer;
	int i;

	eXosip_call_t *jc;
	eXosip_dialog_t *jd;

	tr = NULL;
	jd = NULL;
	/* first, look for a Dialog in the map of element */
	for (jc = eXosip.j_calls; jc!= NULL ; jc=jc->next)
	{
		if (jc->c_inc_tr!=NULL)
		{
			i = cancel_match_invite(jc->c_inc_tr, evt->sip);
			if (i==0) {
				tr = jc->c_inc_tr;
				break;
			}
		}
		tr=NULL;
		for (jd = jc->c_dialogs; jd!= NULL ; jd=jd->next)
		{
			int pos=0;
			while (!osip_list_eol(jd->d_inc_trs, pos))
			{
				tr = osip_list_get(jd->d_inc_trs, pos);
				i = cancel_match_invite(tr, evt->sip);
				if (i==0)
					break;
				tr = NULL;
				pos++;
			}
		}
		if (jd!=NULL) break; /* tr has just been found! */
	}

	if (tr==NULL) /* we didn't found the transaction to cancel */
	{
		i = _eXosip_build_response_default(&answer, NULL, 481, evt->sip);
		if (i!=0)
		{
			OSIP_TRACE (osip_trace
				(__FILE__, __LINE__, OSIP_ERROR, NULL,
				"eXosip: cannot cancel transaction.\n"));
			owsip_list_add_nodup(eXosip.j_transactions, tr, 0);
			osip_transaction_set_your_instance(tr, NULL);
			return ;
		}
		osip_message_set_content_length(answer, "0");
		evt_answer = osip_new_outgoing_sipmessage(answer);
		evt_answer->transactionid =  transaction->transactionid;
		osip_transaction_add_event(transaction,evt_answer);

		owsip_list_add_nodup(eXosip.j_transactions, transaction, 0);
		//osip_transaction_set_your_instance(transaction, NULL);
		__eXosip_wakeup();
		return;
	}

	if (tr->state==IST_TERMINATED || tr->state==IST_CONFIRMED
		|| tr->state==IST_COMPLETED)
	{
		/* I can't find the status code in the rfc?
		(I read I must answer 200? wich I found strange)
		I probably misunderstood it... and prefer to send 481
		as the transaction has been answered. */
		if (jd==NULL)
			i = _eXosip_build_response_default(&answer, NULL, 481, evt->sip);
		else
			i = _eXosip_build_response_default(&answer, jd->d_dialog, 481, evt->sip);
		if (i!=0)
		{
			OSIP_TRACE (osip_trace
				(__FILE__, __LINE__, OSIP_ERROR, NULL,
				"eXosip: cannot cancel transaction.\n"));
			owsip_list_add_nodup(eXosip.j_transactions, tr, 0);
			osip_transaction_set_your_instance(tr, NULL);
			return ;
		}
		osip_message_set_content_length(answer, "0");
		evt_answer = osip_new_outgoing_sipmessage(answer);
		evt_answer->transactionid =  transaction->transactionid;
		osip_transaction_add_event(transaction,evt_answer);

		if (jd!=NULL)
			osip_list_add(jd->d_inc_trs, transaction , 0);
		else
			owsip_list_add_nodup(eXosip.j_transactions, transaction, 0);
		//osip_transaction_set_your_instance(transaction, NULL);
		__eXosip_wakeup();

		return ;
	}

	{
		if (jd==NULL)
			i = _eXosip_build_response_default(&answer, NULL, 200, evt->sip);
		else
			i = _eXosip_build_response_default(&answer, jd->d_dialog, 200, evt->sip);
		if (i!=0)
		{
			OSIP_TRACE (osip_trace
				(__FILE__, __LINE__, OSIP_ERROR, NULL,
				"eXosip: cannot cancel transaction.\n"));
			owsip_list_add_nodup(eXosip.j_transactions, tr, 0);
			osip_transaction_set_your_instance(tr, NULL);
			return ;
		}
		osip_message_set_content_length(answer, "0");
		evt_answer = osip_new_outgoing_sipmessage(answer);
		evt_answer->transactionid =  transaction->transactionid;
		osip_transaction_add_event(transaction,evt_answer);
		__eXosip_wakeup();

		if (jd!=NULL)
			osip_list_add(jd->d_inc_trs, transaction , 0);
		else
			owsip_list_add_nodup(eXosip.j_transactions, transaction, 0);
		//osip_transaction_set_your_instance(transaction, NULL);

		/* answer transaction to cancel */
		if (jd==NULL)
			i = _eXosip_build_response_default(&answer, NULL, 487,
			tr->orig_request);
		else
			i = _eXosip_build_response_default(&answer, jd->d_dialog, 487,
			tr->orig_request);
		if (i!=0)
		{
			OSIP_TRACE (osip_trace
				(__FILE__, __LINE__, OSIP_ERROR, NULL,
				"eXosip: cannot cancel transaction.\n"));
			owsip_list_add_nodup(eXosip.j_transactions, tr, 0);
			osip_transaction_set_your_instance(tr, NULL);
			return ;
		}
		osip_message_set_content_length(answer, "0");
		evt_answer = osip_new_outgoing_sipmessage(answer);
		evt_answer->transactionid =  tr->transactionid;
		__eXosip_report_event(EXOSIP_CALL_CANCELLED, jc, jd, NULL);

		osip_transaction_add_event(tr,evt_answer);
		__eXosip_wakeup();
	}
}

static osip_event_t *
eXosip_process_reinvite(eXosip_call_t *jc, eXosip_dialog_t *jd,
			osip_transaction_t *transaction,
			osip_event_t *evt, sdp_message_t *remote_sdp)
{
  sdp_message_t *local_sdp;
  osip_message_t *answer;
  osip_event_t *sipevent;
  int i;



  /* We must negociate... */
  local_sdp = NULL;
  if (remote_sdp!=NULL) {

    i =  eXosip_sdp_negotiate(jc->c_ctx, remote_sdp);

    if (i!=200)
      {
	send_default_answer(jd, transaction, evt, i, NULL, NULL, 0);
	return NULL;
      }
    local_sdp = osip_negotiation_ctx_get_local_sdp(jc->c_ctx);
  }

#if 0
  if (remote_sdp==NULL)

    {
      sdp_message_t *local_sdp;
      osip_negotiation_sdp_build_offer(eXosip.osip_negotiation, NULL, &local_sdp, "25563", NULL);
      osip_negotiation_ctx_set_local_sdp(jc->c_ctx, local_sdp);

      if (local_sdp==NULL)
	{
	  send_default_answer(jd, transaction, evt, 500, NULL, NULL, __LINE__);
	  return NULL;
	}
    }
#endif

  i = _eXosip_build_response_default(&answer, jd->d_dialog, 200, evt->sip);
  if (i!=0)
    {
      send_default_answer(jd, transaction, evt, 500, "Internal SIP Error", "Failed to build Answer for INVITE within call", __LINE__);
      return NULL;
    }

  complete_answer_that_establish_a_dialog2(answer, evt->sip, jd->d_localcontact);

  if (local_sdp!=NULL)
    {
      char *local_body;
      char *size;


      i = sdp_message_to_str(local_sdp, &local_body);

      if (i!=0) {
	eXosip_send_default_answer(jd, transaction, evt, 500, "Internal SDP Error", "SDP packet is corrupted", __LINE__);
	osip_message_free(answer);
	return NULL;
      }
      i = osip_message_set_body(answer, local_body, strlen(local_body));
      if (i!=0) {
	send_default_answer(jd, transaction, evt, 500, "Internal SDP Error", "SDP cannot be added in message", __LINE__);
	osip_free(local_body);
	osip_message_free(answer);
	return NULL;
      }
      size = (char *) osip_malloc(6*sizeof(char));
#ifdef __APPLE_CC__
      sprintf(size,"%li",strlen(local_body));
#else
      sprintf(size,"%i",strlen(local_body));
#endif
      osip_free(local_body);
      osip_message_set_content_length(answer, size);
      osip_free(size);
      i = osip_message_set_content_type(answer, "application/sdp");
      if (i!=0) {
	send_default_answer(jd, transaction, evt, 500, "Internal SIP Error", "Content-Type cannot be added in message", __LINE__);
	osip_message_free(answer);
	return NULL;
      }

    }
  
  osip_transaction_set_your_instance(transaction, __eXosip_new_jinfo(owsip_dialog_account_get (jd), jc, jd, NULL, NULL));
  sipevent = osip_new_outgoing_sipmessage(answer);
  sipevent->transactionid =  transaction->transactionid;

  osip_list_add(jd->d_inc_trs, transaction, 0);
  return sipevent;
}


static void eXosip_process_invite_on_hold(eXosip_call_t *jc, eXosip_dialog_t *jd,
					  osip_transaction_t *transaction,
					  osip_event_t *evt, sdp_message_t *sdp)
{
  osip_event_t *sipevent;
	osip_message_t * tmp_msg = NULL;

  sipevent = eXosip_process_reinvite(jc, jd, transaction, evt, sdp);
  if (sipevent==NULL)
    return; /* ERROR */

  if(jd != NULL && sipevent->sip!= NULL && sipevent->sip->status_code == 200) {
	  osip_message_clone(sipevent->sip, &(tmp_msg));
  }

  eXosip_report_call_event_with_status(EXOSIP_CALL_HOLD, jc, jd, evt->sip);

  osip_transaction_add_event(transaction, sipevent);
  __eXosip_wakeup();

	jd->d_200Ok = tmp_msg;

}

static void eXosip_process_invite_off_hold(eXosip_call_t *jc, eXosip_dialog_t *jd,
					   osip_transaction_t *transaction,
					   osip_event_t *evt, sdp_message_t *sdp)
{
  osip_event_t *sipevent;
	osip_message_t * tmp_msg = NULL;

  sipevent = eXosip_process_reinvite(jc, jd, transaction, evt, sdp);
  if (sipevent==NULL)
    return; /* ERROR */

  if(jd != NULL && sipevent->sip!= NULL && sipevent->sip->status_code == 200) {
	  osip_message_clone(sipevent->sip, &(tmp_msg));
  }

  eXosip_report_call_event_with_status(EXOSIP_CALL_OFFHOLD, jc, jd, evt->sip);
  osip_transaction_add_event(transaction, sipevent);
  __eXosip_wakeup();
  
	jd->d_200Ok = tmp_msg;
}

static void eXosip_process_new_options(osip_transaction_t *transaction, osip_event_t *evt)
{
  osip_event_t *evt_answer;
  osip_message_t *answer;
  int i;

  i = _eXosip_build_response_default(&answer, NULL, 200, evt->sip);
  if (i!=0)
    {
      return ;
    }

  evt_answer = osip_new_outgoing_sipmessage(answer);
  evt_answer->transactionid =  transaction->transactionid;

  osip_transaction_add_event(transaction,evt_answer);
  __eXosip_wakeup();
}

/**
* This function is called on receive of a new (out of any previous transaction) INVITE.
* Sends provisional answers.
* Puts an event in the eXosip event list.
*
* @param	[in] transaction	SIP messages part of the same Command Sequence (CSeq)
* @param	[in] evt			? TODO
*/
static void eXosip_process_new_invite(osip_transaction_t *transaction, osip_event_t *evt)
{
#ifdef AUTO_RING
	osip_event_t *evt_answer;
#endif
	int i;
	eXosip_call_t *jc;
	eXosip_dialog_t *jd;
	osip_message_t *answer;
	// <ncouturier>
	osip_body_t * body = NULL;
	// </ncouturier>
	int oldcid = 0;


	/* if we're recieving a new INVITE with tag in To: header,  ignore it */
	if (transaction->to)
	{
		osip_generic_param_t *tag;

		osip_from_param_get_byname (transaction->to, "tag", &tag);

		if (tag)
		{
			send_default_answer(NULL, transaction, evt, 481, NULL, NULL, __LINE__);
			return ;
		}
	}

	eXosip_call_init(&jc);
	/* eXosip_call_set_subect... */

	ADD_ELEMENT(eXosip.j_calls, jc);

	i = _eXosip_build_response_default(&answer, NULL, 101, evt->sip);
	if (i!=0)
	{
		OSIP_TRACE (osip_trace
			(__FILE__, __LINE__, OSIP_ERROR, NULL,
			"eXosip: cannot create dialog."));
		owsip_list_add_nodup(eXosip.j_transactions, transaction, 0);
		osip_transaction_set_your_instance(transaction, NULL);
		OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_ERROR,NULL,"ERROR: Could not create response for invite\n"));
		return;
	}
	osip_message_set_content_length(answer, "0");
	i = complete_answer_that_establish_a_dialog(answer, evt->sip);
	if (i!=0)
	{
		OSIP_TRACE (osip_trace
			(__FILE__, __LINE__, OSIP_ERROR, NULL,
			"eXosip: cannot complete answer!\n"));
		owsip_list_add_nodup(eXosip.j_transactions, transaction, 0);
		osip_transaction_set_your_instance(transaction, NULL);
		osip_message_free(answer);
		return ;
	}

	i = eXosip_dialog_init_as_uas(&jd, owsip_transaction_account_get (transaction), evt->sip, answer);
	if (i!=0)
	{
		OSIP_TRACE (osip_trace
			(__FILE__, __LINE__, OSIP_ERROR, NULL,
			"eXosip: cannot create dialog!\n"));
		owsip_list_add_nodup(eXosip.j_transactions, transaction, 0);
		osip_transaction_set_your_instance(transaction, NULL);
		osip_message_free(answer);
		return ;
	}
	ADD_ELEMENT(jc->c_dialogs, jd);

	osip_transaction_set_your_instance(transaction, __eXosip_new_jinfo(owsip_dialog_account_get (jd), jc, jd, NULL, NULL));
	//TODO: This is a hack in order not to send 101 dialog establisment message???
	//  evt_answer = osip_new_outgoing_sipmessage(answer);
	//  evt_answer->transactionid = transaction->transactionid;
#ifdef AUTO_RING /* default is now to not send a 180 Ringing */
  osip_transaction_add_event(transaction, evt_answer);
  __eXosip_wakeup();
#endif

#ifdef AUTO_RING /* default is now to not send a 180 Ringing */
  i = _eXosip_build_response_default(&answer, jd->d_dialog, 180, evt->sip);

  if (i!=0)
    {
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: cannot send ringback."));
      owsip_list_add_nodup(eXosip.j_transactions, transaction, 0);
      osip_transaction_set_your_instance(transaction, NULL);
      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_ERROR,NULL,"ERROR: Could not create response for invite\n"));
      return;
    }
  i = complete_answer_that_establish_a_dialog(answer, evt->sip);
  if (i!=0)
    {
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: cannot complete answer!\n"));
      owsip_list_add_nodup(eXosip.j_transactions, transaction, 0);
      osip_transaction_set_your_instance(transaction, NULL);
      osip_message_free(answer);
      return ;
    }

  osip_message_set_content_length(answer, "0");
  /*  send message to transaction layer */

  evt_answer = osip_new_outgoing_sipmessage(answer);
  evt_answer->transactionid = transaction->transactionid;
#endif

	eXosip_update();
	jc->c_inc_tr = transaction;
	//TODO: This is a hack in order not to send 101 dialog establisment message???
	//  osip_transaction_add_event(transaction, evt_answer);

	/* be sure the invite will be processed
	before any API call on this dialog*/
	osip_ist_execute(eXosip.j_osip);

	if (transaction->orig_request!=NULL)
	{
		eXosip_event_t *je;
		int evtCode;

		evtCode = (oldcid) ? EXOSIP_CALL_REPLACES : EXOSIP_CALL_NEW; 

		je = eXosip_event_init_for_call(evtCode, jc, jd);
		if (je!=NULL)
		{
			osip_header_t *subject;
			char *tmp;
			osip_message_get_subject(transaction->orig_request, 0, &subject);
			if (subject!=NULL && subject->hvalue!=NULL && subject->hvalue[0]!='\0')
				snprintf(je->subject, 255, "%s", subject->hvalue);
			osip_uri_to_str(transaction->orig_request->req_uri, &tmp);
			if (tmp!=NULL)
			{
				snprintf(je->req_uri, 255, "%s", tmp);
				osip_free(tmp);
			}
			// <ncouturier>
			if(transaction->orig_request->content_type != NULL){
				if (osip_strcasecmp(transaction->orig_request->content_type->type, "application") == 0 &&
					osip_strcasecmp(transaction->orig_request->content_type->subtype, "sdp") == 0)
				{
					eXosip_event_add_sdp_info(je, transaction->orig_request);
					// fill in the content type
					je->i_ctt = copy_content_type(transaction->orig_request->content_type);
				}
				else
				{
					// fill in the content type
					je->i_ctt = copy_content_type(transaction->orig_request->content_type);

					// fill in the body; just get the first body
					// TODO get all the bodies?
					if(!osip_list_eol(&transaction->orig_request->bodies, 0)){
						body = (osip_body_t *)osip_list_get(&transaction->orig_request->bodies, 0);

						if (je->msg_body) osip_free(je->msg_body);

						je->msg_body = osip_strdup(body->body);
						if (!je->msg_body){
							eXosip_event_free(je);
							return;
						}
					}
				}
			}
			// </ncouturier>
			eXosip_event_add_status(je, answer);
		}

		je->replacedcid = oldcid;

		__eXosip_report_event(evtCode, NULL, NULL, je);
	}

	__eXosip_wakeup();
}

static void eXosip_process_invite_within_call(eXosip_call_t *jc, eXosip_dialog_t *jd,
					      osip_transaction_t *transaction, osip_event_t *evt)
{
	sdp_message_t *sdp;
	int i;
	int pos;
	int pos_media;
	char *sndrcv;
	char *ipaddr;
	osip_body_t *body;

	/* Is this a "on hold" message? */
	sdp = NULL;
	pos = 0;
	i = 500;
	while (!osip_list_eol(&evt->sip->bodies,pos)){		
		body = (osip_body_t *)osip_list_get(&evt->sip->bodies,pos);
		pos++;

		// <ncouturier>
		if(evt->sip != NULL && evt->sip->content_type != NULL && osip_strcasecmp(evt->sip->content_type->type, "application") == 0 && osip_strcasecmp(evt->sip->content_type->subtype, "sdp") == 0){

			i = sdp_message_init(&sdp);
			if (i!=0){
				break;
			}

			/* WE ASSUME IT IS A SDP BODY AND THAT    */
			/* IT IS THE ONLY ONE, OF COURSE, THIS IS */
			/* NOT TRUE */
			if (body->body!=NULL){
				i = sdp_message_parse(sdp,body->body);
				if (i==0) {
					i = 200;
					break;
				}
			}
			sdp_message_free(sdp);
			sdp = NULL;

		}else{
			if(body != NULL && body->body != NULL && (osip_strcasecmp(body->body, "holdon") == 0 || osip_strcasecmp(body->body, "holdoff") == 0)){
				i = 200;
			}
		}
		// </ncouturier>
	}

	if (pos!=0 && i!=200){
		send_default_answer(jd, transaction, evt, 400, NULL, NULL, __LINE__);
		return;
	}

	// <ncouturier>

	/* TODO: we should verify the content-type */

	if(evt->sip != NULL && evt->sip->content_type != NULL && osip_strcasecmp(evt->sip->content_type->type, "application") == 0 && osip_strcasecmp(evt->sip->content_type->subtype, "sdp") == 0){
		if (pos!=0){
			pos_media=-1;
			pos = 0;
			ipaddr = NULL;
			while (!sdp_message_endof_media(sdp, pos_media)){
				ipaddr = sdp_message_c_addr_get(sdp, pos_media, pos);
				while (ipaddr!=NULL){ /* only one is allowed here? */			
					if (pos==1 && pos_media==-1){
						break;
					}
					if (0==osip_strcasecmp("0.0.0.0", ipaddr)){
						break;
					}
					pos++;
					ipaddr = sdp_message_c_addr_get(sdp, pos_media, pos);
				}
				if (pos==1 && pos_media==-1){
					ipaddr=NULL;
				}
				if (ipaddr!=NULL){
					break;
				}
				pos = 0;
				pos_media++;
			}

			if (ipaddr==NULL){
				sndrcv = NULL;
				pos_media=-1;
				pos = 0;
				while (!sdp_message_endof_media(sdp, pos_media)){
					const char *mtype = sdp_message_m_media_get(sdp, pos_media);

					/* take only audio streams in account */
					if ((mtype != 0) && !osip_strcasecmp(mtype, "audio")){
						sndrcv = sdp_message_a_att_field_get(sdp, pos_media, pos);
						while (sndrcv!=NULL){
							if (0==osip_strcasecmp("inactive", sndrcv) || 0==osip_strcasecmp("sendonly", sndrcv)){
								break;
							}
							pos++;
							sndrcv = sdp_message_a_att_field_get(sdp, pos_media, pos);
						}
					}

					if (sndrcv!=NULL){
						break;
					}
					pos = 0;
					pos_media++;
				}
			}

			if (ipaddr!=NULL || (sndrcv!=NULL && (0==osip_strcasecmp("inactive", sndrcv) || 0==osip_strcasecmp("sendonly", sndrcv)))){
				/*  We received an INVITE to put on hold the other party. */
				eXosip_process_invite_on_hold(jc, jd, transaction, evt, sdp);
				return;
			}else{
				/* This is a call modification, probably for taking it of hold */
				eXosip_process_invite_off_hold(jc, jd, transaction, evt, sdp);
				return;
			}
		}

	}else{
		if (pos!=0){
			if(body != NULL && body->body != NULL && osip_strcasecmp(body->body, "holdon") == 0){
				/*  We received an INVITE to put on hold the other party. */
				eXosip_process_invite_on_hold(jc, jd, transaction, evt, sdp);
				return;
			}else if(body != NULL && body->body != NULL && osip_strcasecmp(body->body, "holdoff") == 0){
				/* This is a call modification, probably for taking it of hold */
				eXosip_process_invite_off_hold(jc, jd, transaction, evt, sdp);
				return;
			}
		}
	}

	// </ncouturier>
	eXosip_process_invite_off_hold(jc, jd, transaction, evt, NULL);
	return;
}

static int eXosip_event_package_is_supported(osip_transaction_t *transaction,
					     osip_event_t *evt)
{
  osip_header_t *event_hdr;
  int code;

  /* get the event type and return "489 Bad Event". */
  osip_message_header_get_byname(evt->sip, "event", 0, &event_hdr);
  if (event_hdr==NULL || event_hdr->hvalue==NULL)
    {
      code = 400;      /* Bad Request */
    }
  else if (0!=osip_strcasecmp(event_hdr->hvalue, "presence"))
    code = 489;
  else code = 200;
  if (code!=200)
    {
      send_default_answer(NULL, transaction, evt, code, NULL, NULL, __LINE__);
      return 0;
    }
  return -1;
}

static void eXosip_process_new_subscribe(osip_transaction_t *transaction,
					 osip_event_t *evt)
{
  osip_event_t *evt_answer;
  eXosip_notify_t *jn;
  eXosip_dialog_t *jd;
  osip_message_t *answer;
  int code;
  int i;

  eXosip_notify_init(&jn, owsip_transaction_account_get (transaction), evt->sip);
  _eXosip_notify_set_refresh_interval(jn, evt->sip);

  i = _eXosip_build_response_default(&answer, NULL, 101, evt->sip);
  if (i!=0)
    {
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: cannot create dialog."));
      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_ERROR,NULL,"ERROR: Could not create response for invite\n"));
      owsip_list_add_nodup(eXosip.j_transactions, transaction, 0);
      eXosip_notify_free(jn);
      return;
    }
  i = complete_answer_that_establish_a_dialog(answer, evt->sip);
  if (i!=0)
    {
      osip_message_free(answer);
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: cannot complete answer!\n"));
      owsip_list_add_nodup(eXosip.j_transactions, transaction, 0);
      eXosip_notify_free(jn);
      return ;
    }

  i = eXosip_dialog_init_as_uas(&jd, owsip_transaction_account_get (transaction), evt->sip, answer);
  if (i!=0)
    {
      osip_message_free(answer);
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: cannot create dialog!\n"));
      owsip_list_add_nodup(eXosip.j_transactions, transaction, 0);
      eXosip_notify_free(jn);
      return ;
    }
  ADD_ELEMENT(jn->n_dialogs, jd);
  owsip_list_add_nodup(jd->d_inc_trs, transaction, -1);
  jn->n_inc_tr = transaction;

  osip_transaction_set_your_instance(transaction, __eXosip_new_jinfo(owsip_dialog_account_get (jd), NULL, jd, NULL, jn));

  evt_answer = osip_new_outgoing_sipmessage(answer);
  evt_answer->transactionid = transaction->transactionid;
  osip_transaction_add_event(transaction, evt_answer);

  ADD_ELEMENT(eXosip.j_notifies, jn);
  eXosip_update();
  __eXosip_wakeup();

#if 0
  /* There should be a list of already accepted freinds for which we
     have already accepted the subscription. */
  if (0==_eXosip_notify_is_a_known_subscriber(evt->sip))
    code = 200;
  else
    code = 202;

  i = _eXosip_build_response_default(&answer, jd->d_dialog, code, evt->sip);
  if (i!=0)
    {
      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"ERROR: Could not create response for subscribe\n"));
      owsip_list_add_nodup(eXosip.j_transactions, transaction, 0);
      return;
    }

  _eXosip_notify_add_expires_in_2XX_for_subscribe(jn, answer);

  {
    
    i = complete_answer_that_establish_a_dialog(answer, evt->sip);
    if (i!=0)
      {
	owsip_list_add_nodup(eXosip.j_transactions, transaction, 0);
	osip_message_free(answer);
	return;
      }
  }

  jn->n_inc_tr = transaction;

  /* eXosip_dialog_set_200ok(jd, answer); */
  evt_answer = osip_new_outgoing_sipmessage(answer);
  evt_answer->transactionid = transaction->transactionid;

  osip_transaction_add_event(transaction, evt_answer);

  osip_dialog_set_state(jd->d_dialog, DIALOG_CONFIRMED);

  eXosip_update();
  __eXosip_wakeup();
#endif
}

static void eXosip_process_subscribe_within_call(eXosip_notify_t *jn,
						 eXosip_dialog_t *jd,
						 osip_transaction_t *transaction,
						 osip_event_t *evt)
{
  osip_message_t *answer;
  osip_event_t *sipevent;
  int i;

  _eXosip_notify_set_refresh_interval(jn, evt->sip);
  i = _eXosip_build_response_default(&answer, jd->d_dialog, 200, evt->sip);
  if (i!=0)
    {
      send_default_answer(jd, transaction, evt, 500, "Internal SIP Error", "Failed to build Answer for SUBSCRIBE", __LINE__);
      return ;
    }
  
  _eXosip_notify_add_expires_in_2XX_for_subscribe(jn, answer);

  {
    
    i = complete_answer_that_establish_a_dialog2(answer, evt->sip, jd->d_localcontact);
    if (i!=0)
      {
	/* this info is yet known by the remote UA,
	   so we don't have to exit here */
      }
  }

  osip_transaction_set_your_instance(transaction, __eXosip_new_jinfo(owsip_dialog_account_get (jd), NULL, jd, NULL, jn));
  sipevent = osip_new_outgoing_sipmessage(answer);
  sipevent->transactionid =  transaction->transactionid;
  osip_transaction_add_event(transaction, sipevent);
  __eXosip_wakeup();

  /* if subscribe request contains expires="0", close the subscription */
  {
    int now = time(NULL);
    if (jn->n_ss_expires-now<=0)
      {
	jn->n_ss_status=EXOSIP_SUBCRSTATE_TERMINATED;
	jn->n_ss_reason=TIMEOUT;
      }
  }

  osip_list_add(jd->d_inc_trs, transaction, 0);

  eXosip_notify_send_notify(jn, jd, jn->n_ss_status,
			    jn->n_online_status);
  return;
}

static void
eXosip_process_notify_out_of_dialog(osip_transaction_t *transaction, osip_event_t *evt)
{
	osip_message_t	*answer;
	osip_event_t	*sipevent;
	osip_header_t	*event_hdr;
	osip_body_t		*body = NULL;
	int				i;

	i = _eXosip_build_response_default(&answer, NULL, 200, evt->sip);
	if (i != 0 || answer == NULL)
	{
		return;
	}
	
	sipevent = osip_new_outgoing_sipmessage(answer);
	sipevent->transactionid = transaction->transactionid;
	osip_transaction_add_event(transaction, sipevent);
	
	{
		eXosip_event_t *je;

		eXosip_event_init(&je, EXOSIP_SUBSCRIPTION_NOTIFY);
		if (je!=NULL)
		{
			eXosip_event_add_status(je, answer);
		}
		else
		{
			return;
		}
		
		osip_message_header_get_byname(evt->sip, "event", 0, &event_hdr);
		if (event_hdr==NULL || event_hdr->hvalue == NULL)
		{
			send_default_answer(NULL, transaction, evt, 481,
								"Missing Event header in Notify",
								"Missing Event header in Notify",
								__LINE__);
			
			return;
		}
		
		strncpy(je->sip_event, event_hdr->hvalue, sizeof(je->sip_event));
		if (je->msg_body)
			osip_free(je->msg_body);
		
		osip_message_get_body(evt->sip, 0, &body);
		if (body && body->body)
			je->msg_body = osip_strdup(body->body);
		
		if (je->msg_body == NULL)
		{
			eXosip_event_free(je);
			return;
		}
			
		if (eXosip.j_call_callbacks[EXOSIP_SUBSCRIPTION_NOTIFY]!=NULL)
			eXosip.j_call_callbacks[EXOSIP_SUBSCRIPTION_NOTIFY](EXOSIP_SUBSCRIPTION_NOTIFY, je);
		else if (eXosip.j_runtime_mode==EVENT_MODE)
			eXosip_event_add(je);
	}
	
	__eXosip_wakeup();
	return;
}

static void
eXosip_process_notify_within_dialog(eXosip_subscribe_t *js,
				    eXosip_dialog_t *jd,
				    osip_transaction_t *transaction,
				    osip_event_t *evt)
{
  osip_message_t *answer;
  osip_event_t   *sipevent;
  osip_header_t  *sub_state;
  osip_header_t	 *event_hdr;
  osip_body_t *body = NULL;
  int i;

  if (jd==NULL)
    {
      send_default_answer(jd, transaction, evt, 500, "Internal SIP Error", "No dialog for this NOTIFY", __LINE__);
      return ;
    }

  /* if subscription-state has a reason state set to terminated,
     we close the dialog */
  osip_message_header_get_byname(evt->sip, "subscription-state", 0, &sub_state);
  if (sub_state==NULL||sub_state->hvalue==NULL)
    {
      send_default_answer(jd, transaction, evt, 400, NULL, NULL, __LINE__);
      return;
    }

    i = _eXosip_build_response_default(&answer, jd->d_dialog, 200, evt->sip);
  if (i!=0)
    {
      send_default_answer(jd, transaction, evt, 500, "Internal SIP Error", "Failed to build Answer for NOTIFY", __LINE__);
      return ;
    }

  /* modify the status of user */
  if (0==osip_strncasecmp(sub_state->hvalue, "active", 6))
    {
      osip_content_type_t *ctype;
      js->s_ss_status = EXOSIP_SUBCRSTATE_ACTIVE;
      js->s_online_status = EXOSIP_NOTIFY_UNKNOWN; /* default value */

      /* if there is a body which we understand, analyse it */
      ctype = osip_message_get_content_type(evt->sip);
      if (ctype!=NULL && ctype->type!=NULL && ctype->subtype!=NULL)
	{
	  if (0==osip_strcasecmp(ctype->type, "application")
	      && ((0==osip_strcasecmp(ctype->subtype, "pidf+xml")) || (0==osip_strcasecmp(ctype->subtype, "watcherinfo+xml"))
		      ||(0==osip_strcasecmp(ctype->subtype, "cpim-pidf+xml"))))
	    osip_message_get_body(evt->sip, 0, &body);
	  else
	    {
	      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_ERROR,NULL,
				    "Unknown body: %s/%s\n",
				    ctype->type, ctype->subtype));
	    }
	}

      if (body!=NULL && 0==osip_strcasecmp(ctype->subtype, "pidf+xml"))
	{
	  /* search for the open string */
	  char *status = 0, *basic = 0, *estatus = 0, *ebasic = 0;
	  char *cv, *ov;

	  status = strstr(body->body, "<status>");
	  if (status)
	      estatus = strstr(status, "</status>");

	  if (estatus)
	    basic = strstr(status, "<basic>");
	    
	  if (basic)
	    ebasic = strstr(basic, "</basic>");
	  

	  if (ebasic && (ebasic < estatus))
	    {
	      ov = strstr(basic, "open");
	      cv = strstr(basic, "closed");
	      if (ov  && (ov < ebasic))
		js->s_online_status = EXOSIP_NOTIFY_ONLINE;
	      else if (cv && (cv < ebasic))
		js->s_online_status = EXOSIP_NOTIFY_CLOSED;
	    }
		       
	}
    }
  else if (0==osip_strncasecmp(sub_state->hvalue, "pending", 7))
    {
      js->s_ss_status = EXOSIP_SUBCRSTATE_PENDING;
      js->s_online_status = EXOSIP_NOTIFY_PENDING;
    }

	if (0==osip_strncasecmp(sub_state->hvalue, "terminated", 10))
	{
		sipevent = osip_new_outgoing_sipmessage(answer);
		sipevent->transactionid =  transaction->transactionid;
		osip_transaction_add_event(transaction, sipevent);

		owsip_list_add_nodup(eXosip.j_transactions, transaction, 0);

		__eXosip_wakeup();
		return;
	}
	else
    {
		osip_transaction_set_your_instance(transaction, __eXosip_new_jinfo(owsip_dialog_account_get (jd), NULL, jd, js, NULL));
	}

  osip_list_add(jd->d_inc_trs, transaction, 0);

  sipevent = osip_new_outgoing_sipmessage(answer);
  sipevent->transactionid =  transaction->transactionid;
  osip_transaction_add_event(transaction, sipevent);

  {
    eXosip_event_t *je;
    je = eXosip_event_init_for_subscribe(EXOSIP_SUBSCRIPTION_NOTIFY, js, jd);
    if (je!=NULL)
      {
	eXosip_event_add_status(je, answer);
      }

    osip_message_header_get_byname(evt->sip, "event", 0, &event_hdr);
    if ( event_hdr==NULL || event_hdr->hvalue==NULL || 
	 (strcmp(event_hdr->hvalue,"presence") && strcmp(event_hdr->hvalue,"presence.winfo")))
      {
	send_default_answer(jd, transaction, evt, 481, "Missing or Bad Event header in Notify", "Missing Event header in Notify", __LINE__);
	return ;
      }

    strncpy(je->sip_event,event_hdr->hvalue,sizeof(je->sip_event));
    if (je->msg_body)
      osip_free(je->msg_body);

    if (body)
      je->msg_body = osip_strdup(body->body);
    else
      je->msg_body = osip_strdup("");
    
    if (je->msg_body == NULL) {
      eXosip_event_free(je);
      return;
    }	
	


    if (eXosip.j_call_callbacks[EXOSIP_SUBSCRIPTION_NOTIFY]!=NULL)
      eXosip.j_call_callbacks[EXOSIP_SUBSCRIPTION_NOTIFY](EXOSIP_SUBSCRIPTION_NOTIFY, je);
    else if (eXosip.j_runtime_mode==EVENT_MODE)
      eXosip_event_add(je);
  }

  __eXosip_wakeup();
  return;
}

static int
eXosip_match_notify_for_subscribe(eXosip_subscribe_t *js, osip_message_t *notify)
{
  osip_transaction_t *out_sub;
  
  if (js==NULL)
    return -1;
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,
			"Trying to match notify with subscribe\n"));
  
  out_sub = eXosip_find_last_out_subscribe(js, NULL);
  if (out_sub==NULL || out_sub->orig_request==NULL)
    return -1;
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,
			"subscribe transaction found\n"));
  
  /* some checks to avoid crashing on bad requests */
  if (notify == NULL || notify->cseq == NULL
      || notify->cseq->method == NULL || notify->to == NULL)
    return -1;
  
  if (0 != osip_call_id_match (out_sub->callid, notify->call_id))
    return -1;
  
  {
    /* The From tag of outgoing request must match
       the To tag of incoming notify:
    */
    osip_generic_param_t *tag_from;
    osip_generic_param_t *tag_to;
    
    osip_from_param_get_byname (out_sub->from, "tag", &tag_from);
    osip_from_param_get_byname (notify->to, "tag", &tag_to);
    if (tag_to == NULL || tag_to->gvalue==NULL)
      {
	OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_ERROR,NULL,
			      "Uncompliant user agent: no tag in from of outgoing request\n"));
	return -1;
      }
    if (tag_from == NULL || tag_to->gvalue==NULL)
      {
	OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_ERROR,NULL,
			      "Uncompliant user agent: no tag in to of incoming request\n"));
	return -1;
      }
    
    if (0 != strcmp (tag_from->gvalue, tag_to->gvalue))
      return -1;
  }
  
  return 0;
}

static void
eXosip_process_message_outside_of_dialog(osip_transaction_t *transaction,
					 osip_event_t *evt)
{
  osip_message_t *answer;
  osip_event_t   *evt_answer;
  osip_header_t  *expires, *date;
  int i;
  
  /* Check whether the Expire time has been reached */
  
  /* the Expires header is present ? */
  if (osip_message_get_expires(evt->sip, 0, &expires) != -1  && expires->hvalue!=NULL)
    {
      
      unsigned long expires_ul;
      time_t now_ts, date_ts;
      
      expires_ul = strtoul(expires->hvalue, (char**)NULL, 10);
      now_ts = time(NULL);
      
      /*
      ** A MESSAGE request is said to be expired if its expiration time has
      ** passed. The expiration time is determined by examining the Expires
      ** header field, if present.  MESSAGE requests without an Expires header
      ** field do not expire.  If the MESSAGE request containing an Expires
      ** header field also contains a Date header field, the UAS SHOULD
      ** interpret the Expires header field value as delta time from the Date
      ** header field value.  If the request does not contain a Date header
      ** field, the UAS SHOULD interpret the Expires header value as delta
      ** time from the time the UAS received the request.
      */
      
      /* Does the message also contain a Date header ? */
      if (osip_message_get_date(evt->sip, 0, &date) != -1 && date->hvalue != NULL)
	{
	  
	  /*
	    20.17 Date
	    The Date header field contains the date and time.  Unlike HTTP/1.1,
	    SIP only supports the most recent RFC 1123 [20] format for dates. 
	    As in [H3.3], SIP restricts the time zone in SIP-date to "GMT", while
	    RFC 1123 allows any time zone.  An RFC 1123 date is case-sensitive.
	    The Date header field reflects the time when the request or
	    response is first sent.

	    The Date header field can be used by simple end systems without a
	    battery-backed clock to acquire a notion of current time.
	    However, in its GMT form, it requires clients to know their offset
	    from GMT.
	    
	    Example:
	    
	    Date: Sat, 13 Nov 2010 23:29:00 GMT
	    
	    20.19 Expires
	    
	    The Expires header field gives the relative time after which the
	    message (or content) expires.
	    
	    The precise meaning of this is method dependent.
	    
	    The expiration time in an INVITE does not affect the duration of
	    the actual session that may result from the invitation.  Session
	    description protocols may offer the ability to express time limits
	    on the session duration, however.
	    
	    The value of this field is an integral number of seconds (in decimal)
	    between 0 and (2**32)-1, measured from the receipt of the request.
	    
	    Example:
	    
	    Expires: 5
	    
	  */
	  
	  /* TODO: 
	     date_ts = RECUPERE_TIMESTAMP(date->hvalue);
	  */
	  
	  date_ts = time(NULL);
	  
	  /* Has the message expired ? */
	  if ((time_t) (date_ts + expires_ul) < now_ts)
	    {
	      /* discard old data. */
	      return;
	    }
	}
      else if (expires_ul != 0)
	{
	  /* No date header, but a not null Expires one:
	  ** TODO: Compare to the UAS reception time
	  */
	  date_ts = transaction->birth_time;
	}
      /* The message has not expired */
    }
  
  /* We must create the event */
  {
    eXosip_event_t *je;    
    je = eXosip_event_init_for_message(EXOSIP_MESSAGE_NEW, transaction, evt->sip);    
    i = _eXosip_build_response_default(&answer, NULL, SIP_OK, evt->sip);
    if (i!=0)
      {
	owsip_list_add_nodup(eXosip.j_transactions, transaction, 0);
	return ;
      }
    
    if (je!=NULL)
      {
	int pos = 0;
	osip_body_t *oldbody;


	osip_content_type_clone(evt->sip->content_type,&je->i_ctt);

	
	eXosip_event_add_status(je, answer);
	
	while (!osip_list_eol(&evt->sip->bodies, pos))
	  {
	    oldbody = (osip_body_t *)osip_list_get(&evt->sip->bodies, pos);
	    pos++;
	  
	    if (je->msg_body)
	      osip_free(je->msg_body);
		
		
	    je->msg_body = osip_strdup(oldbody->body);
	    if ( ! je->msg_body )
	      {
		eXosip_event_free(je);
		return;
	      }
          }
      }
    
    if (eXosip.j_call_callbacks[EXOSIP_MESSAGE_NEW]!=NULL)
      eXosip.j_call_callbacks[EXOSIP_MESSAGE_NEW](EXOSIP_MESSAGE_NEW, je);
    else if (eXosip.j_runtime_mode==EVENT_MODE)
      eXosip_event_add(je);
  }
  
  /* finally, send the 200 OK response */
  
  evt_answer = osip_new_outgoing_sipmessage(answer);
  evt_answer->transactionid = transaction->transactionid;
  
  osip_transaction_add_event(transaction, evt_answer);
  
#ifdef NEW_TIMER
  __eXosip_wakeup();
#endif
  return;
}


static void eXosip_process_newrequest (osip_event_t *evt)
{
  OWSIPAccount account = 0 ;
  osip_transaction_t *transaction;
  osip_event_t *evt_answer;
  osip_message_t *answer;
  int i;
  int ctx_type;
  eXosip_call_t *jc;
  eXosip_subscribe_t *js;
  eXosip_notify_t *jn;
  eXosip_dialog_t *jd;

  if (MSG_IS_INVITE(evt->sip))
    {
      ctx_type = IST;
    }
  else if (MSG_IS_ACK(evt->sip))
    { /* this should be a ACK for 2xx (but could be a late ACK!) */
      ctx_type = -1;
#if 0 /* ACK must be announced */
      osip_event_free(evt);
      return ;
#endif
    }
  else if (MSG_IS_REQUEST(evt->sip))
    {
      ctx_type = NIST;
    }
  else
    { /* We should handle late response and 200 OK before coming here. */
      ctx_type = -1;
      osip_event_free(evt);
      return ;
    }

  if (evt->sip != NULL && evt->sip->to != NULL)
  {
	account = owsip_account_get_from_received_request (evt->sip) ;
  }

  transaction=NULL;
  if (ctx_type != -1)
    {
      i = osip_transaction_init(&transaction,
			   (osip_fsm_type_t)ctx_type,
			   eXosip.j_osip,
			   evt->sip);
      if (i!=0)
	{
	  osip_event_free(evt);
	  return ;
	}

      evt->transactionid =  transaction->transactionid;
      osip_transaction_set_your_instance(transaction, __eXosip_new_jinfo (account, NULL, NULL, NULL, NULL));


      i = _eXosip_build_response_default(&answer, NULL, 100, evt->sip);
      if (i!=0)
	{
	  __eXosip_delete_jinfo(transaction);
	  osip_transaction_free(transaction);
	  osip_event_free(evt);
	  return ;
	}
	
      osip_message_set_content_length(answer, "0");
      /*  send message to transaction layer */


      evt_answer = osip_new_outgoing_sipmessage(answer);
      evt_answer->transactionid = transaction->transactionid;
	
      /* add the REQUEST & the 100 Trying */
      osip_transaction_add_event(transaction, evt);
      osip_transaction_add_event(transaction, evt_answer);
      __eXosip_wakeup();
    }

  if (MSG_IS_CANCEL(evt->sip))
    {
      /* special handling for CANCEL */
      /* in the new spec, if the CANCEL has a Via branch, then it
	 is the same as the one in the original INVITE */
      eXosip_process_cancel(transaction, evt);
      return ;
    }

  jd = NULL;
  /* first, look for a Dialog in the map of element */
  for (jc = eXosip.j_calls; jc!= NULL ; jc=jc->next)
    {
      for (jd = jc->c_dialogs; jd!= NULL ; jd=jd->next)
	{
	  if (jd->d_dialog!=NULL)
	    {
	      if (osip_dialog_match_as_uas(jd->d_dialog, evt->sip)==0)
		break;
	    }
	}
      if (jd!=NULL) break;
    }


  if (jd!=NULL)
    {
      osip_transaction_t *old_trn;
      /* it can be:
	 1: a new INVITE offer.
	 2: a REFER request from one of the party.
	 2: a BYE request from one of the party.
	 3: a REQUEST with a wrong CSeq.
	 4: a NOT-SUPPORTED method with a wrong CSeq.
      */
      
      if (!MSG_IS_BYE(evt->sip))
	{
	  /* reject all requests for a closed dialog */
	  old_trn = eXosip_find_last_inc_bye(jc, jd);
	  if (old_trn == NULL)
	    old_trn = eXosip_find_last_out_bye(jc, jd);
	  
	  if (old_trn!=NULL)
	    {
	      send_default_answer(jd, transaction, evt, 481, NULL, NULL, __LINE__);
	      return ;
	    }

	}

      if (MSG_IS_INVITE(evt->sip))
	{
	  /* the previous transaction MUST be freed */
	  old_trn = eXosip_find_last_inc_invite(jc, jd);
	    
	  if (old_trn!=NULL && old_trn->state!=IST_TERMINATED)
	    {
	      send_default_answer(jd, transaction, evt, 500, "Retry Later", "An INVITE is not terminated", __LINE__);
	      return ;
	    }

	  old_trn = eXosip_find_last_out_invite(jc, jd);
	  if (old_trn!=NULL && old_trn->state!=ICT_TERMINATED)
	    {
	      send_default_answer(jd, transaction, evt, 491, NULL, NULL, __LINE__);
	      return ;
	    }

	  osip_dialog_update_osip_cseq_as_uas(jd->d_dialog, evt->sip);
	  osip_dialog_update_route_set_as_uas(jd->d_dialog, evt->sip);

	  eXosip_process_invite_within_call(jc, jd, transaction, evt);
	}
      else if (MSG_IS_BYE(evt->sip))
	{
	  old_trn = eXosip_find_last_inc_bye(jc, jd);
	    
	  if (old_trn!=NULL) /* && old_trn->state!=NIST_TERMINATED) */
	    { /* this situation should NEVER occur?? (we can't receive
		 two different BYE for one call! */
	      send_default_answer(jd, transaction, evt, 500, "Call Already Terminated", "A pending BYE has already terminate this call", __LINE__);
	      return;
	    }
	  /* osip_transaction_free(old_trn); */
	  eXosip_process_bye(jc, jd, transaction, evt);
	}
      else if (MSG_IS_ACK(evt->sip))
	{
	  eXosip_process_ack(jc, jd, evt);
	}
      else if (MSG_IS_REFER(evt->sip))
	{
	  eXosip_process_refer(jc, jd, transaction, evt);
	}
      else if (MSG_IS_OPTIONS(evt->sip))
	{
	  eXosip_process_options(jc, jd, transaction, evt);
	}
      else if (MSG_IS_INFO(evt->sip))
	{
	  eXosip_process_info(jc, jd, transaction, evt);
	}
      else if (MSG_IS_NOTIFY(evt->sip))
	{
	  eXosip_process_notify_for_refer(jc, jd, transaction, evt);
	}
      else
	{
	  send_default_answer(jd, transaction, evt, 405, NULL, "Method Not Allowed", __LINE__);
	}
      return ;
    }

  if (MSG_IS_ACK(evt->sip))
    {
      /* no transaction has been found for this ACK! */
      osip_event_free(evt);
      return;
    }

  if (MSG_IS_INFO(evt->sip))
    {
      send_default_answer(jd, transaction, evt, 481, NULL, NULL, __LINE__);
      return; /* fixed */
    }
  if (MSG_IS_OPTIONS(evt->sip))
    {
      eXosip_process_new_options(transaction, evt);
      return;
    }
  else if (MSG_IS_INVITE(evt->sip))
    {
      eXosip_process_new_invite(transaction, evt);
      return;
    }
  else if (MSG_IS_BYE(evt->sip))
    {
      send_default_answer(jd, transaction, evt, 481, NULL, NULL, __LINE__);
      return;
    }

  js = NULL;
  /* first, look for a Dialog in the map of element */
  for (js = eXosip.j_subscribes; js!= NULL ; js=js->next)
    {
      for (jd = js->s_dialogs; jd!= NULL ; jd=jd->next)
	{
	  if (jd->d_dialog!=NULL)
	    {
	      if (osip_dialog_match_as_uas(jd->d_dialog, evt->sip)==0)
		break;
	    }
	}
      if (jd!=NULL) break;
    }

  if (js!=NULL)
    {
      /* dialog found */
      osip_transaction_t *old_trn;
      /* it can be:
	 1: a new INVITE offer.
	 2: a REFER request from one of the party.
	 2: a BYE request from one of the party.
	 3: a REQUEST with a wrong CSeq.
	 4: a NOT-SUPPORTED method with a wrong CSeq.
      */
      if (MSG_IS_MESSAGE(evt->sip))
	{
	  /* eXosip_process_imessage_within_subscribe_dialog(transaction, evt); */
	  send_default_answer(jd, transaction, evt, SIP_NOT_IMPLEMENTED,
				     NULL, "MESSAGEs within dialogs are not implemented.", __LINE__);
	  return;
	}
      else if (MSG_IS_NOTIFY(evt->sip))
	{
	  /* the previous transaction MUST be freed */
	  old_trn = eXosip_find_last_inc_notify(js, jd);
	  
	  /* shouldn't we wait for the COMPLETED state? */
		if (old_trn!=NULL) {
			if (old_trn->state!=NIST_TERMINATED && old_trn->state!=NIST_COMPLETED)
	    {
	      /* retry later? */
	      send_default_answer(jd, transaction, evt, 500, "Retry Later", "A pending NOTIFY is not terminated", __LINE__);
	      return ;
			} else {
				__osip_transaction_set_state(old_trn, NIST_TERMINATED);
				owsip_list_remove_element(jd->d_inc_trs, old_trn);
				owsip_list_add_nodup(eXosip.j_transactions, old_trn, 0);
			}
	    }

	  osip_dialog_update_osip_cseq_as_uas(jd->d_dialog, evt->sip);
	  osip_dialog_update_route_set_as_uas(jd->d_dialog, evt->sip);
	  eXosip_process_notify_within_dialog(js, jd, transaction, evt);
	}
      else
	{
	  send_default_answer(jd, transaction, evt, 501, NULL, "Just Not Implemented", __LINE__);
	}
      return ;
    }

  if (MSG_IS_NOTIFY(evt->sip))
    {
      /* I should probably initiate a dialog with info from NOTIFY...
	 By now, I prefer to discard the message until an answer for
	 the subscribe is received, then I'll be able to answer
	 the NOTIFY retransmission. */

      /* let's try to check if the NOTIFY is related to an existing
	 subscribe */
      js = NULL;
      /* first, look for a Dialog in the map of element */
      for (js = eXosip.j_subscribes; js!= NULL ; js=js->next)
	{
	  if (eXosip_match_notify_for_subscribe(js, evt->sip)==0)
	    {
	      i = eXosip_dialog_init_as_uac(&jd, account, evt->sip);
	      if (i!=0)
		{
        OSIP_TRACE (osip_trace
	       (__FILE__, __LINE__, OSIP_ERROR, NULL,
		     "eXosip: cannot establish a dialog\n"));
		  return;
		}
	      ADD_ELEMENT(js->s_dialogs, jd);
	      eXosip_update();

	      eXosip_process_notify_within_dialog(js, jd, transaction, evt);
	      return;
	    }
	}
      
      //send_default_answer(NULL, transaction, evt, 200, NULL, NULL, __LINE__);
	  owsip_list_add_nodup(eXosip.j_transactions, transaction, 0);
	  eXosip_process_notify_out_of_dialog(transaction, evt);
      return;
    }

  jn = NULL;
  /* first, look for a Dialog in the map of element */
  for (jn = eXosip.j_notifies; jn!= NULL ; jn=jn->next)
    {
      for (jd = jn->n_dialogs; jd!= NULL ; jd=jd->next)
	{
	  if (jd->d_dialog!=NULL)
	    {
	      if (osip_dialog_match_as_uas(jd->d_dialog, evt->sip)==0)
		break;
	    }
	}
      if (jd!=NULL) break;
    }

  if (jn!=NULL)
    {
      /* dialog found */
      osip_transaction_t *old_trn;
      /* it can be:
	 1: a new INVITE offer.
	 2: a REFER request from one of the party.
	 2: a BYE request from one of the party.
	 3: a REQUEST with a wrong CSeq.
	 4: a NOT-SUPPORTED method with a wrong CSeq.
      */
      if (MSG_IS_MESSAGE(evt->sip))
	{
	  send_default_answer(jd, transaction, evt, SIP_NOT_IMPLEMENTED,
				     NULL, "MESSAGEs within dialogs are not implemented.", __LINE__);
	  return;
	}
      else if (MSG_IS_SUBSCRIBE(evt->sip))
	{
	  /* the previous transaction MUST be freed */
	  old_trn = eXosip_find_last_inc_subscribe(jn, jd);
	  
	  /* shouldn't we wait for the COMPLETED state? */
	  if (old_trn!=NULL && old_trn->state!=NIST_TERMINATED
	      && old_trn->state!=NIST_COMPLETED)
	    {
	      /* retry later? */
	      send_default_answer(jd, transaction, evt, 500, "Retry Later", "A SUBSCRIBE is not terminated", __LINE__);
	      return ;
	    }

	  osip_dialog_update_osip_cseq_as_uas(jd->d_dialog, evt->sip);
	  osip_dialog_update_route_set_as_uas(jd->d_dialog, evt->sip);

	  eXosip_process_subscribe_within_call(jn, jd, transaction, evt);
	}
      else
	{
	  send_default_answer(jd, transaction, evt, 501, NULL, NULL, __LINE__);
	}
      return ;
    }

  if (MSG_IS_MESSAGE(evt->sip))
    {
      eXosip_process_message_outside_of_dialog(transaction , evt);
      owsip_list_add_nodup(eXosip.j_transactions, transaction, 0);
      return;
    }

  if (MSG_IS_SUBSCRIBE(evt->sip))
    {

      if (0==eXosip_event_package_is_supported(transaction, evt))
	{
	  return;
	}
      eXosip_process_new_subscribe(transaction, evt);
      return;
    }

  /* default answer */
  eXosip_send_default_answer(NULL, transaction, evt, 501, NULL, NULL, __LINE__);
}

static void eXosip_process_response_out_of_transaction (osip_event_t *evt)
{
	eXosip_call_t *jc = NULL;
	eXosip_dialog_t *jd = NULL;
	time_t now;

	now = time (NULL);
	if (evt->sip==NULL
		|| evt->sip->cseq==NULL
		|| evt->sip->cseq->number==NULL
		|| evt->sip->to==NULL
		|| evt->sip->from==NULL)
	{
		osip_event_free (evt);
		return;
	}

	/* search for existing dialog: match branch & to tag */
	for (jc = eXosip.j_calls; jc != NULL; jc = jc->next)
	{
		/* search for calls with only ONE outgoing transaction */
		if (jc->c_id >= 1 && jc->c_dialogs != NULL && jc->c_out_tr!=NULL)
		{
			for (jd = jc->c_dialogs; jd != NULL; jd = jd->next)
			{
				/* only initial request are concerned with this */
				if (jd->d_id >=1 && jd->d_dialog != NULL)
				{
					/* match answer with dialog */
					osip_generic_param_t *tag;

					osip_from_get_tag (evt->sip->to, &tag);

					if (jd->d_dialog->remote_tag == NULL || tag == NULL)
						continue;
					if (jd->d_dialog->remote_tag != NULL && tag != NULL
						&& tag->gvalue != NULL
						&& 0 == strcmp (jd->d_dialog->remote_tag, tag->gvalue))
						break;
				}
			}
			if (jd!=NULL)
				break; /* found a matching dialog! */

			/* check if the transaction match this from tag */
			if (jc->c_out_tr->orig_request!=NULL && jc->c_out_tr->orig_request->from!=NULL)
			{
				osip_generic_param_t *tag_invite;
				osip_generic_param_t *tag;
				osip_from_get_tag (jc->c_out_tr->orig_request->from, &tag_invite);
				osip_from_get_tag (evt->sip->from, &tag);

				if (tag_invite == NULL || tag == NULL)
					continue;
				if (tag_invite->gvalue != NULL && tag->gvalue != NULL
					&& 0 == strcmp (tag_invite->gvalue, tag->gvalue))
					break;
			}
		}
	}

	if (jc==NULL)
	{
		OSIP_TRACE (osip_trace
			(__FILE__, __LINE__, OSIP_INFO1, NULL,
			"Incoming 2xx has no relations with current calls: Message discarded.\r\n"));
		osip_event_free (evt);
		return;
	}

	if (jc!=NULL && jd!=NULL)
	{
		/* we have to restransmit the ACK (if already available) */
		OSIP_TRACE (osip_trace
			(__FILE__, __LINE__, OSIP_INFO1, NULL,
			"2xx restransmission receveid.\r\n"));
		/* check if the 2xx is for the same ACK */ 
		if (jd->d_ack!=NULL && jd->d_ack->cseq!=NULL
			&& jd->d_ack->cseq->number!=NULL)
		{
			if (0==osip_strcasecmp(jd->d_ack->cseq->number, evt->sip->cseq->number))
			{
				cb_snd_message (NULL, jd->d_ack, NULL,0, -1);
				OSIP_TRACE (osip_trace
					(__FILE__, __LINE__, OSIP_INFO1, NULL,
					"ACK restransmission sent.\r\n"));
			}
		}

		osip_event_free (evt);
		return;
	}

	if (jc!=NULL)
	{
		/* match answer with dialog */
		osip_dialog_t *dlg;
		osip_transaction_t *last_tr;
		int i;

		/* we match an existing dialog: send a retransmission of ACK */
		i = osip_dialog_init_as_uac (&dlg, evt->sip);
		if (i != 0 || dlg==NULL)
		{
			OSIP_TRACE (osip_trace
				(__FILE__, __LINE__, OSIP_ERROR, NULL,
				"Cannot build dialog for 200ok.\r\n"));
			osip_event_free (evt);
			return;
		}
		OSIP_TRACE (osip_trace
			(__FILE__, __LINE__, OSIP_INFO1, NULL,
			"sending ACK for 2xx out of transaction.\r\n"));

		{
			osip_message_t *ack;
			osip_message_t *bye;

			i = _eXosip_build_request_within_dialog (&ack, "ACK", dlg);
			if (i != 0)
			{
				osip_dialog_free(dlg);
				osip_event_free (evt);
				return;
			}
			/* copy all credentials from INVITE! */
			last_tr = jc->c_out_tr;
			if (last_tr!=NULL)
			{
				int pos = 0;
				int i;
				osip_proxy_authorization_t *pa = NULL;

				i = osip_message_get_proxy_authorization (last_tr->orig_request, pos, &pa);
				while (i == 0 && pa != NULL)
				{
					osip_proxy_authorization_t *pa2;

					i = osip_proxy_authorization_clone (pa, &pa2);
					if (i != 0)
					{
						OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL,
							"Error in credential from INVITE\n"));
						break;
					}
					osip_list_add (&ack->proxy_authorizations, pa2, -1);
					pa = NULL;
					pos++;
					i = osip_message_get_proxy_authorization (last_tr->orig_request, pos, &pa);
				}
			}
			cb_snd_message (NULL, ack, NULL,0, -1);
			osip_message_free(ack);

			/* ready to send a BYE */
			i = generating_bye (&bye, dlg);
			cb_snd_message (NULL, bye, NULL, 0, -1);
			osip_message_free(bye);
		}

		osip_dialog_free(dlg);
		osip_event_free (evt);
		return;
	}

	osip_event_free(evt);
}

/* if second==-1 && useconds==-1  -> wait for ever
   if max_message_nb<=0  -> infinite loop....  */
int eXosip_read_message( int max_message_nb, int sec_max, int usec_max )
{
	fd_set osip_fdset;
	struct timeval tv;
	char *buf;
	  
	tv.tv_sec = sec_max;
	tv.tv_usec = usec_max;
	  
	buf = (char *)osip_malloc(SIP_MESSAGE_MAX_LENGTH*sizeof(char)+1);
	while (max_message_nb!=0 && eXosip.j_stop_ua==0)
    {
		int i,sock = 0;
		int max;
		int wakeup_socket = jpipe_get_read_descr(eXosip.j_socketctl);
		FD_ZERO(&osip_fdset);

		max = sock;

#if defined (WIN32) || defined (_WIN32_WCE)
		FD_SET((unsigned int)wakeup_socket, &osip_fdset);
#else
		FD_SET(wakeup_socket, &osip_fdset);
#endif
		if (wakeup_socket>max)
		{
			max = wakeup_socket;
		}
		if ((sec_max==-1)||(usec_max==-1))
			i = select(max+1, &osip_fdset, NULL, NULL, NULL);
		else
			i = select(max+1, &osip_fdset, NULL, NULL, &tv);
	      
		if ((i == -1) && (errno == EINTR || errno == EAGAIN))
			continue;
	      
		if ((i > 0) && FD_ISSET (wakeup_socket, &osip_fdset))
		{
			char buf2[500];
			jpipe_read (eXosip.j_socketctl, buf2, 499);
		}

		if (0==i || eXosip.j_stop_ua!=0)
		{
		}
		else if (-1==i)
		{
			osip_free(buf);
			return -2; /* error */
		}
		max_message_nb--;
    }
	osip_free(buf);
	return 0;
}

void
eXosip_recv (OWSLSocket socket)
{
	int i ;
	char * buffer ;
	OWSLAddress address ;
	int address_length = OWSL_ADDRESS_SIZE ;

	buffer = osip_malloc (SIP_MESSAGE_MAX_LENGTH * sizeof (char) + 1) ;

	i = owsl_recvfrom
	(
		socket,
		buffer,
		SIP_MESSAGE_MAX_LENGTH,
		0,
		(struct sockaddr *) & address,
		& address_length
	) ;

	if( i > 5 ) /* we expect at least one byte, otherwise there's no doubt that it is not a sip message !*/
	{
		/* Message might not end with a "\0" but we know the number of */
		/* char received! */
		osip_event_t * sipevent ;
		buffer[i] = '\0' ;
		OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,
			"Received message: \n%s\n", buffer));
#ifdef WIN32
		if (strlen (buffer) > 412)
		{
			OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,
				"Message suite: \n%s\n", buffer + 412));
		}
#endif

		/* for debug use */
		/*printf ("RECEIVED:\n%s\n", buffer) ;*/

		sipevent = osip_parse (buffer, i) ;
		if (sipevent != NULL && sipevent->sip != NULL)
		{
			osip_transaction_t * transaction ;
			OWSIPAccount account ;

			/* TODO: fix it for IPv6 */
			osip_message_fix_last_via_header (sipevent->sip, inet_ntoa (((struct sockaddr_in *) & address)->sin_addr), ntohs (((struct sockaddr_in *) & address)->sin_port)) ;

			transaction = __osip_find_transaction (eXosip.j_osip, sipevent, 1) ;

			if (transaction != NULL)
			{
				account = owsip_transaction_account_get (transaction) ;
			}
			else
			{
				account = owsip_account_get_from_received_message (sipevent->sip) ;
			}
			if (account <= 0)
			{
				OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL,
					"no matching SIP account found\n")) ;
				osip_free (buffer);
				return ;
			}

			owsip_account_idle_time_refresh (account) ;

			owsip_account_message_filter (account, sipevent->sip, OWSIP_IN, transaction) ;

			if (transaction == NULL)
			{
				/* this event has no transaction, */
				OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,
					"This is a request\n", buffer));
				eXosip_lock () ;
				if (MSG_IS_REQUEST (sipevent->sip))
				{
					eXosip_process_newrequest (sipevent) ;
				}
				else if (MSG_IS_RESPONSE (sipevent->sip))
				{
					eXosip_process_response_out_of_transaction(sipevent) ;
				}
				eXosip_unlock () ;
			}
			else
			{
				/* handled by oSIP !*/
			}

			/* signal read event */
			__eXosip_wakeup () ;
		}
		else
		{
			OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_ERROR,NULL,
				"Could not parse SIP message\n"));
			osip_event_free(sipevent);
		}
	}
	else if (i < 0)
	{
		OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_ERROR,NULL,
			"Could not read socket\n"));
	}
	else 
	{
		OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,
			"Dummy SIP message received\n"));
	}

	osip_free (buffer);
}

static int eXosip_pendingosip_transaction_exist ( eXosip_call_t *jc, eXosip_dialog_t *jd )
{
  osip_transaction_t *tr;
  int now = time(NULL);

  tr = eXosip_find_last_inc_bye(jc, jd);
  if (tr!=NULL && tr->state!=NIST_TERMINATED)
    { /* Don't want to wait forever on broken transaction!! */
      if (tr->birth_time+180<now) /* Wait a max of 2 minutes */
	{
	  /* remove the transaction from oSIP: */
	  // osip_remove_transaction(eXosip.j_osip, tr);
	  eXosip_remove_transaction_from_call(tr, jc);
	  eXosip_transaction_free(tr);
	}
      else
	return 0;
    }

  tr = eXosip_find_last_out_bye(jc, jd);
  if (tr!=NULL && tr->state!=NICT_TERMINATED)
    { /* Don't want to wait forever on broken transaction!! */
      if (tr->birth_time+180<now) /* Wait a max of 2 minutes */
	{
	  /* remove the transaction from oSIP: */
	  // osip_remove_transaction(eXosip.j_osip, tr);
	  eXosip_remove_transaction_from_call(tr, jc);
	  eXosip_transaction_free(tr);
	}
      else
	return 0;
    }

  tr = eXosip_find_last_inc_invite(jc, jd);
  if (tr!=NULL && tr->state!=IST_TERMINATED)
    { /* Don't want to wait forever on broken transaction!! */
      if (tr->birth_time+180<now) /* Wait a max of 2 minutes */
	{
	  /* remove the transaction from oSIP: */
	  /* osip_remove_transaction(eXosip.j_osip, tr);
	     eXosip_remove_transaction_from_call(tr, jc);
	     osip_transaction_free(tr); */
	}
      else
	return 0;
    }

  tr = eXosip_find_last_out_invite(jc, jd);
  if (tr!=NULL && tr->state!=ICT_TERMINATED)
    { /* Don't want to wait forever on broken transaction!! */
      if (tr->birth_time+180<now) /* Wait a max of 2 minutes */
	{
	  /* remove the transaction from oSIP: */
	  /* osip_remove_transaction(eXosip.j_osip, tr);
	     eXosip_remove_transaction_from_call(tr, jc);
	     osip_transaction_free(tr); */
	}
      else
	return 0;
    }

  tr = eXosip_find_last_inc_refer(jc, jd);
  if (tr!=NULL && tr->state!=IST_TERMINATED)
    { /* Don't want to wait forever on broken transaction!! */
      if (tr->birth_time+180<now) /* Wait a max of 2 minutes */
	{
	  /* remove the transaction from oSIP: */
	  // osip_remove_transaction(eXosip.j_osip, tr);
	  eXosip_remove_transaction_from_call(tr, jc);
	  eXosip_transaction_free(tr);
	}
      else
	return 0;
    }

  tr = eXosip_find_last_out_refer(jc, jd);
  if (tr!=NULL && tr->state!=NICT_TERMINATED)
    { /* Don't want to wait forever on broken transaction!! */
      if (tr->birth_time+180<now) /* Wait a max of 2 minutes */
	{
	  /* remove the transaction from oSIP: */
	  // osip_remove_transaction(eXosip.j_osip, tr);
	  eXosip_remove_transaction_from_call(tr, jc);
	  eXosip_transaction_free(tr);
	}
      else
	return 0;
    }

  return -1;
}

static int eXosip_release_finished_calls ( eXosip_call_t *jc, eXosip_dialog_t *jd )
{
  osip_transaction_t *tr;
  tr = eXosip_find_last_inc_bye(jc, jd);
  if (tr==NULL)
    tr = eXosip_find_last_out_bye(jc, jd);

  if (tr!=NULL &&
      ( tr->state==NIST_TERMINATED || tr->state==NICT_TERMINATED ))
    {
      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO2,NULL,
			    "eXosip: eXosip_release_finished_calls remove a dialog\n"));
      /* Remove existing reference to the dialog from transactions! */
      __eXosip_call_remove_dialog_reference_in_call(jc, jd);
      REMOVE_ELEMENT(jc->c_dialogs, jd);
      eXosip_dialog_free(jd);
      return 0;
    }
  return -1;
}



static void
__eXosip_release_call(eXosip_call_t *jc, eXosip_dialog_t *jd )
{
  REMOVE_ELEMENT(eXosip.j_calls, jc);
  __eXosip_report_event(EXOSIP_CALL_RELEASED, jc, jd, NULL);
  eXosip_call_free(jc);
  __eXosip_wakeup();
}


static int eXosip_release_aborted_calls ( eXosip_call_t *jc, eXosip_dialog_t *jd )
{
  int now = time(NULL);
  osip_transaction_t *tr;
  tr = eXosip_find_last_inc_invite(jc, jd);
  if (tr==NULL)
    tr = eXosip_find_last_out_invite(jc, jd);

  if (tr==NULL)
    {
      if (jd!=NULL)
	{
	  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO2,NULL,
				"eXosip: eXosip_release_aborted_calls remove an empty dialog\n"));
	  __eXosip_call_remove_dialog_reference_in_call(jc, jd);
	  REMOVE_ELEMENT(jc->c_dialogs, jd);
	  eXosip_dialog_free(jd);
	  return 0;
	}
      return -1;
    }

  if (tr!=NULL
      && tr->state!=IST_TERMINATED
      && tr->state!=ICT_TERMINATED
      && tr->birth_time+180<now) /* Wait a max of 2 minutes */
    {
      if (jd!=NULL)
	{
	  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO2,NULL,
				"eXosip: eXosip_release_aborted_calls remove a dialog for an unfinished transaction\n"));
	  __eXosip_call_remove_dialog_reference_in_call(jc, jd);
	  REMOVE_ELEMENT(jc->c_dialogs, jd);
	  __eXosip_report_event(EXOSIP_CALL_NOANSWER, jc, jd, NULL);
	  eXosip_dialog_free(jd);
	  __eXosip_wakeup();
	  return 0;
	}
    }

  if (tr!=NULL
      && (tr->state==IST_TERMINATED
	  || tr->state==ICT_TERMINATED))
    {
      if (tr==jc->c_inc_tr)
	{
	  if (jc->c_inc_tr->last_response==NULL)
	    {
	      /* OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO2,NULL,
		 "eXosip: eXosip_release_aborted_calls transaction with no answer\n")); */
	    }
	  else if (MSG_IS_STATUS_3XX(jc->c_inc_tr->last_response))
	    {
	      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO2,NULL,
				    "eXosip: eXosip_release_aborted_calls answered with a 3xx\n"));
	      __eXosip_release_call(jc, jd);
	      return 0;
	    }
	  else if (MSG_IS_STATUS_4XX(jc->c_inc_tr->last_response))
	    {
	      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO2,NULL,
				    "eXosip: eXosip_release_aborted_calls answered with a 4xx\n"));
	      __eXosip_release_call(jc, jd);
	      return 0;
	    }
	  else if (MSG_IS_STATUS_5XX(jc->c_inc_tr->last_response))
	    {
	      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO2,NULL,
				    "eXosip: eXosip_release_aborted_calls answered with a 5xx\n"));
	      __eXosip_release_call(jc, jd);
	      return 0;
	    }
	  else if (MSG_IS_STATUS_6XX(jc->c_inc_tr->last_response))
	    {
	      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO2,NULL,
				    "eXosip: eXosip_release_aborted_calls answered with a 6xx\n"));
	      __eXosip_release_call(jc, jd);
	      return 0;
	    }
	}
      else if (tr==jc->c_out_tr)
	{
	  if (jc->c_out_tr->last_response==NULL)
	    {
	      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO2,NULL,
				    "eXosip: eXosip_release_aborted_calls completed with no answer\n"));
	      __eXosip_release_call(jc, jd);
	      return 0;
	    }
	  else if (MSG_IS_STATUS_3XX(jc->c_out_tr->last_response))
	    {
	      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO2,NULL,
				    "eXosip: eXosip_release_aborted_calls completed answered with 3xx\n"));
	      __eXosip_release_call(jc, jd);
	      return 0;
	    }
	  else if (MSG_IS_STATUS_4XX(jc->c_out_tr->last_response))
	    {
	      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO2,NULL,
				    "eXosip: eXosip_release_aborted_calls completed answered with 4xx\n"));
	      __eXosip_release_call(jc, jd);
	      return 0;
	    }
	  else if (MSG_IS_STATUS_5XX(jc->c_out_tr->last_response))
	    {
	      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO2,NULL,
				    "eXosip: eXosip_release_aborted_calls completed answered with 5xx\n"));
	      __eXosip_release_call(jc, jd);
	      return 0;
	    }
	  else if (MSG_IS_STATUS_6XX(jc->c_out_tr->last_response))
	    {
	      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO2,NULL,
				    "eXosip: eXosip_release_aborted_calls completed answered with 6xx\n"));
	      __eXosip_release_call(jc, jd);
	      return 0;
	    }
	}
    }

  return -1;
}


void eXosip_release_terminated_calls ( void )
{
  eXosip_dialog_t *jd;
  eXosip_dialog_t *jdnext;
  eXosip_call_t *jc;
  eXosip_call_t *jcnext;
  int now = time(NULL);
  int pos;


  for (jc = eXosip.j_calls ; jc != NULL; )
    {
      jcnext=jc->next;
      /* free call terminated with a BYE */
      for (jd = jc->c_dialogs ; jd != NULL; )
	{
	  jdnext=jd->next;
	  if (0==eXosip_pendingosip_transaction_exist(jc, jd))
	    { }
	  else if (0==eXosip_release_finished_calls(jc, jd))
	    { jd = jc->c_dialogs; }
	  else if (0==eXosip_release_aborted_calls(jc, jd))
	    { jdnext = NULL; }
	  else
	    { }
	  jd=jdnext;
	}
      jc=jcnext;
    }

  for (jc = eXosip.j_calls ; jc != NULL; )
    {
      jcnext=jc->next;
      if (jc->c_dialogs==NULL)
	{
	  /* release call for options requests */
	  if (jc->c_inc_options_tr!=NULL)
	    {
	      if (jc->c_inc_options_tr->state==NIST_TERMINATED)
		{
		  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,
					"eXosip: remove an incoming OPTIONS with no final answer\n"));
		  __eXosip_release_call(jc, NULL);
		}
	      else if (jc->c_inc_options_tr->state!=NIST_TERMINATED
		       && jc->c_inc_options_tr->birth_time+180<now)
		{
		  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,
					"eXosip: remove an incoming OPTIONS with no final answer\n"));
		  __eXosip_release_call(jc, NULL);
		}
	    }
	  else if (jc->c_out_options_tr!=NULL)
	    {
	      if (jc->c_out_options_tr->state==NICT_TERMINATED)
		{
		  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,
					"eXosip: remove an outgoing OPTIONS with no final answer\n"));
		  __eXosip_release_call(jc, NULL);
		}
	      else if (jc->c_out_options_tr->state!=NIST_TERMINATED
		       && jc->c_out_options_tr->birth_time+180<now)
		{
		  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,
					"eXosip: remove an outgoing OPTIONS with no final answer\n"));
		  __eXosip_release_call(jc, NULL);
		}
	    }
	  else if (jc->c_inc_tr!=NULL && jc->c_inc_tr->state!=IST_TERMINATED
	      && jc->c_inc_tr->birth_time+180<now)
	    {
	      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,
				    "eXosip: remove an incoming call with no final answer\n"));
	      __eXosip_release_call(jc, NULL);
	    }
	  else if (jc->c_out_tr!=NULL && jc->c_out_tr->state!=ICT_TERMINATED
		   && jc->c_out_tr->birth_time+180<now)
	    {
	      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,
				    "eXosip: remove an outgoing call with no final answer\n"));
	      __eXosip_release_call(jc, NULL);
	    }
	  else if (jc->c_inc_tr!=NULL && jc->c_inc_tr->state!=IST_TERMINATED)
	    {  }
	  else if (jc->c_out_tr!=NULL && jc->c_out_tr->state!=ICT_TERMINATED)
	    {  }
	  else /* no active pending transaction */
	    {
	      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,
				    "eXosip: remove a call\n"));
	      __eXosip_release_call(jc, NULL);
	    }
	}
      jc = jcnext;
    }

  pos = 0;
  while (!osip_list_eol(eXosip.j_transactions, pos))
    {
      osip_transaction_t *tr = (osip_transaction_t*) osip_list_get(eXosip.j_transactions, pos);
      if (tr->state==IST_TERMINATED || tr->state==ICT_TERMINATED
	  || tr->state== NICT_TERMINATED || tr->state==NIST_TERMINATED)

	{ /* free (transaction is already removed from the oSIP stack) */
	  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,
		      "Release a terminated transaction\n"));

	  eXosip_transaction_free(tr);
	}
      else if (tr->birth_time+180<now) /* Wait a max of 2 minutes */
	{
	  eXosip_transaction_free(tr);
	}
      else
	pos++;
    }
}

// <ncouturier>
osip_content_type_t * copy_content_type(osip_content_type_t * ctt_src){
	osip_content_type_t * ctt_dst = (osip_content_type_t *)malloc(sizeof(osip_content_type_t));
	ctt_dst->type = strdup(ctt_src->type);
	ctt_dst->subtype = strdup(ctt_src->subtype);
	memset(&ctt_dst->gen_params, 0, sizeof(ctt_dst->gen_params));

	return ctt_dst;
}
// </ncouturier>
