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


#include "eXosip2.h"
#include <eXosip/eXosip_cfg.h>
#include <time.h>
#include <string.h>
#include <assert.h>

#ifdef _MSC_VER
#define snprintf _snprintf
#define strncasecmp strnicmp
#define strcasecmp stricmp
#endif

extern eXosip_t eXosip;

#if 0
int eXosip_subscribe_find(int sid, eXosip_subscribe_t **js)
{
  for (*js=eXosip.j_subscribes; *js!=NULL; *js=(*js)->next)
    {
      if ((*js)->s_id==sid)
	return 0;
    }
  *js = NULL;
  return -1;
}
#endif


static osip_transaction_t *
eXosip_find_last_transaction_in_list(osip_list_t *list, const char *method)
{
  osip_transaction_t *result = 0, *tmp;
  int pos;


  pos = 0;
  while (!osip_list_eol(list, pos))
    {
      tmp = osip_list_get(list, pos);
      if (tmp->cseq && tmp->cseq->method && !strcasecmp(tmp->cseq->method, method))
	result = tmp;
      pos++;
    }

  return result;
}


osip_transaction_t *
eXosip_find_last_out_subscribe(eXosip_subscribe_t *js, eXosip_dialog_t *jd )
{
  osip_transaction_t  *tmp = 0;
  
  if (jd)
    tmp =  eXosip_find_last_transaction_in_list(jd->d_out_trs, "SUBSCRIBE");

  if (!tmp && js)
    tmp = js->s_out_tr;

  return tmp;

}

osip_transaction_t *
eXosip_find_last_inc_notify(eXosip_subscribe_t *js, eXosip_dialog_t *jd )
{
  osip_transaction_t  *tmp;
  
  if (!jd)
    return 0;

  tmp =  eXosip_find_last_transaction_in_list(jd->d_inc_trs, "NOTIFY");

  if (!tmp && js)
    tmp = js->s_inc_tr;

  return tmp;
}  

 

#if 0
void
__eXosip_subscribe_remove_dialog_reference_in_subscribe(eXosip_subscribe_t *js, eXosip_dialog_t *jd)
{
  eXosip_dialog_t *_jd;
  jinfo_t *ji;
  if (js==NULL) return;
  if (jd==NULL) return;


  for (_jd = js->s_dialogs; _jd!=NULL; _jd=js->s_dialogs)
    {
      if (jd==_jd)
	break;
    }
  if (_jd==NULL)
    {
      /* dialog not found??? */
    }

  ji = osip_transaction_get_your_instance(js->s_inc_tr);
  if (ji!=NULL && ji->jd==jd)
    ji->jd=NULL;
  ji = osip_transaction_get_your_instance(js->s_out_tr);
  if (ji!=NULL && ji->jd==jd)
    ji->jd=NULL;
}
#endif

int
eXosip_subscribe_init(eXosip_subscribe_t **js, char *uri)
{
  if (uri==NULL) return -1;
  *js = (eXosip_subscribe_t *)osip_malloc(sizeof(eXosip_subscribe_t));
  if (*js == NULL) return -1;
  memset(*js, 0, sizeof(eXosip_subscribe_t));
  osip_strncpy((*js)->s_uri, uri, strlen(uri));
  return 0;
}

void
eXosip_subscribe_free(eXosip_subscribe_t *js)
{
  /* ... */

  eXosip_dialog_t *jd;

  for (jd = js->s_dialogs; jd!=NULL; jd=js->s_dialogs)
    {
      REMOVE_ELEMENT(js->s_dialogs, jd);
      eXosip_dialog_free(jd);
    }

  __eXosip_delete_jinfo(js->s_inc_tr);
  __eXosip_delete_jinfo(js->s_out_tr);
  if (js->s_inc_tr!=NULL)
    owsip_list_add_nodup(eXosip.j_transactions, js->s_inc_tr, 0);
  if (js->s_out_tr!=NULL)
    owsip_list_add_nodup(eXosip.j_transactions, js->s_out_tr, 0);

  osip_free(js);
}

int
_eXosip_subscribe_set_refresh_interval(eXosip_subscribe_t *js,
				       osip_message_t *out_subscribe)
{
	osip_header_t *exp;
	time_t now = time(NULL);
	if (js==NULL || out_subscribe==NULL)
		return -1;
  
	osip_message_get_expires(out_subscribe, 0, &exp);
	if (exp==NULL || exp->hvalue==NULL)
		js->s_ss_expires = now + atoi(eXosip.subscribe_timeout);
	else
    {
		js->s_ss_expires = osip_atoi(exp->hvalue);
		if (js->s_ss_expires!=-1)
			js->s_ss_expires = now + js->s_ss_expires;
		else /* on error, set it to default */
			js->s_ss_expires = now + atoi(eXosip.subscribe_timeout);
	}
	return 0;
}

int  eXosip_subscribe_need_refresh(eXosip_subscribe_t *js, time_t now)
{
	if (now + 60 - js->s_ss_expires>0)
		return 0;
	return -1;
}

int eXosip_subscribe_send_subscribe(eXosip_subscribe_t *js,
				     eXosip_dialog_t *jd, const char *expires)
{
  osip_transaction_t *transaction;
  osip_message_t *subscribe;
  osip_event_t *sipevent;
  int i;
  
	if (!jd || !jd->d_dialog)
	{
		// JULIEN : Maybe the first SUBSCRIBE didn't work 
		// that's why there's no more dialog
		return -1;
	}

  transaction = eXosip_find_last_out_subscribe(js, jd);

  if (transaction!=NULL)
    {
      if (transaction->state!=NICT_TERMINATED && transaction->state!=NIST_TERMINATED)
	return -1;
      //<MINHPQ>		
      // Remove the transaction from dialog structure
      if (jd)
	owsip_list_remove_element (jd->d_out_trs, transaction); // Remove the transaction from jd->d_out_trs list
      if (js && (js->s_out_tr == transaction)) 
	{
	  js->s_out_tr = NULL; // Remove the transaction from js->s_out_tr
	}

      eXosip_transaction_free(transaction);
      transaction = NULL;
      //</MINHPQ>
    }		

  i = _eXosip_build_request_within_dialog(&subscribe, "SUBSCRIBE", jd->d_dialog);
  if (i!=0)
    return -2;
  //<MINHPQ>
  if (js->winfo) 
    {
      owsip_message_set_header(subscribe, "Event", "presence.winfo");
      owsip_message_set_header(subscribe, "Accept", "application/watcherinfo+xml");
    }
  //</MINHPQ>
  
  owsip_message_set_expires(subscribe, expires);

  i = osip_transaction_init(&transaction,
			    NICT,
			    eXosip.j_osip,
			    subscribe);
  if (i!=0)
    {
      /* TODO: release the j_call.. */
      osip_message_free(subscribe);
      return -1;
    }
  
  _eXosip_subscribe_set_refresh_interval(js, subscribe);
  osip_list_add(jd->d_out_trs, transaction, 0);
  
  sipevent = osip_new_outgoing_sipmessage(subscribe);
  sipevent->transactionid =  transaction->transactionid;
  
  osip_transaction_add_event(transaction, sipevent);

  osip_transaction_set_your_instance(transaction, __eXosip_new_jinfo(owsip_dialog_account_get (jd), NULL, jd, js, NULL));
  __eXosip_wakeup();
  return 0;
}
