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

extern eXosip_t eXosip;

#if 0
int eXosip_notify_find(int sid, eXosip_notify_t **jn)
{
  for (*jn=eXosip.j_notifies; *jn!=NULL; *jn=(*jn)->next)
    {
      if ((*jn)->n_id==sid)
	return 0;
    }
  *jn = NULL;
  return -1;
}
#endif

osip_transaction_t *
eXosip_find_last_inc_subscribe(eXosip_notify_t *jn, eXosip_dialog_t *jd )
{
  osip_transaction_t *inc_tr;
  int pos;
  inc_tr = NULL;
  pos=0;
  if (jd!=NULL)
    {
      while (!osip_list_eol(jd->d_inc_trs, pos))
	{
	  inc_tr = osip_list_get(jd->d_inc_trs, pos);
	  if (0==strcmp(inc_tr->cseq->method, "SUBSCRIBE"))
	    break;
	  else inc_tr = NULL;
	  pos++;
	}
    }
  else
    inc_tr = NULL;

  if (inc_tr==NULL)
    return jn->n_inc_tr; /* can be NULL */

  return inc_tr;
}


osip_transaction_t *
eXosip_find_last_out_notify(eXosip_notify_t *jn, eXosip_dialog_t *jd )
{
  osip_transaction_t *out_tr;
  int pos;
  out_tr = NULL;
  pos=0;
  if (jd!=NULL)
    {
      while (!osip_list_eol(jd->d_out_trs, pos))
	{
	  out_tr = osip_list_get(jd->d_out_trs, pos);
	  if (0==strcmp(out_tr->cseq->method, "NOTIFY"))
	    return out_tr;
	  pos++;
	}
    }

  return NULL;
}

int
eXosip_notify_init(eXosip_notify_t **jn, OWSIPAccount account, osip_message_t *inc_subscribe)
{
  osip_contact_t *co;
  char *uri;
  int i;

  if (inc_subscribe==NULL
      ||inc_subscribe->to==NULL
      ||inc_subscribe->to->url==NULL)
    return -1;
  co = (osip_contact_t *) osip_list_get(&inc_subscribe->contacts, 0);
  if (co==NULL || co->url==NULL)
    return -1;

  *jn = (eXosip_notify_t *)osip_malloc(sizeof(eXosip_notify_t));
  if (*jn == NULL) return -1;
  memset(*jn, 0, sizeof(eXosip_notify_t));

  i = osip_uri_to_str(co->url, &uri);
  if (i!=0)
    {
      osip_free(*jn);
      *jn=NULL;
      return -1;
    }
  osip_strncpy((*jn)->n_uri, uri, 254);
  osip_free(uri);

  {
    char tmp[256];
    char *lt = tmp;
    char *gt;

    if (!owsip_account_contact_get(account, tmp, sizeof(tmp)))
      return -1;

    lt = strchr(tmp, '<');
    if (lt)
      {
	gt = strchr(++lt, '>');
	if (gt)
	  *gt = 0;

      }

    (*jn)->n_contact_info = osip_strdup(lt);
  }
  return 0;

}

#if 0
void
__eXosip_notify_remove_dialog_reference_in_notify(eXosip_notify_t *jn, eXosip_dialog_t *jd)
{
  eXosip_dialog_t *_jd;
  jinfo_t *ji;
  if (jn==NULL) return;
  if (jd==NULL) return;

  for (_jd = jn->n_dialogs; _jd!=NULL; _jd=jn->n_dialogs)
    {
      if (jd==_jd)
	break;
    }
  if (_jd==NULL)
    {
      /* dialog not found??? */
    }

  ji = osip_transaction_get_your_instance(jn->n_inc_tr);
  if (ji!=NULL && ji->jd==jd)
    ji->jd=NULL;
  ji = osip_transaction_get_your_instance(jn->n_out_tr);
  if (ji!=NULL && ji->jd==jd)
    ji->jd=NULL;
}
#endif

void
eXosip_notify_free(eXosip_notify_t *jn)
{
  /* ... */

  eXosip_dialog_t *jd;

  for (jd = jn->n_dialogs; jd!=NULL; jd=jn->n_dialogs)
    {
      REMOVE_ELEMENT(jn->n_dialogs, jd);
      eXosip_dialog_free(jd);
    }

  __eXosip_delete_jinfo(jn->n_inc_tr);
  __eXosip_delete_jinfo(jn->n_out_tr);
  if (jn->n_inc_tr!=NULL)
    owsip_list_add_nodup(eXosip.j_transactions, jn->n_inc_tr, 0);
  if (jn->n_out_tr!=NULL)
    owsip_list_add_nodup(eXosip.j_transactions, jn->n_out_tr, 0);
  if (jn->n_contact_info!=NULL) osip_free(jn->n_contact_info);
  osip_free(jn);
}

int
_eXosip_notify_set_refresh_interval(eXosip_notify_t *jn,
				    osip_message_t *inc_subscribe)
{
  osip_header_t *exp;
  int now;
  now = time(NULL);
  if (jn==NULL || inc_subscribe==NULL)
    return -1;
  
  osip_message_get_expires(inc_subscribe, 0, &exp);
  if (exp==NULL || exp->hvalue==NULL)
    jn->n_ss_expires = now + 600;
  else
    {
      jn->n_ss_expires = osip_atoi(exp->hvalue);
      if (jn->n_ss_expires!=-1)
	jn->n_ss_expires = now + jn->n_ss_expires;
      else /* on error, set it to default */
	jn->n_ss_expires = now + 600;
    }

  return 0;
}

int
eXosip_notify_add_allowed_subscriber(char *sip_url)
{
  /* TODO */
  return -1;
}

int
_eXosip_notify_is_a_known_subscriber(osip_message_t *sip)
{
  /* */
  return -1;
}


int
_eXosip_notify_add_body(eXosip_notify_t *jn, osip_message_t *notify)
{
  char buf[1000];
  if (jn->n_ss_status!=EXOSIP_SUBCRSTATE_ACTIVE
      || jn->n_contact_info==NULL || jn->n_contact_info=='\0') /* mandatory! */
    return 0; /* don't need a body? */

  if (jn->n_online_status==EXOSIP_NOTIFY_ONLINE)
    {
      sprintf(buf, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\
<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" entity=\"%s\">\n\
<tuple id=\"sg89ae\">\n\
<status>\n\
<basic>open</basic>\n\
</status>\n\
<contact priority=\"0.8\">%s</contact>\n\
</tuple>\n\
</presence>",
	      jn->n_contact_info, jn->n_contact_info);
    }
  else
    {
      sprintf(buf, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\
<presence xmlns=\"urn:ietf:params:xml:ns:pidf\"\n\
xmlns:es=\"urn:ietf:params:xml:ns:pidf:status:rpid-status\"\n\
xmlns:et=\"urn:ietf:params:xml:ns:pidf:rpid-tuple\"\n\
xmlns:ci=\"urn:ietf:params:xml:ns:pidf:cipid\"\n\
entity=\"%s\">\n%s%s",
	      jn->n_contact_info,
"<tuple id=\"sg89ae\">\n\
<status>\n\
<basic>closed</basic>\n\
</status>\n\
</tuple>\n\
\n"
,
"<tuple id=\"c8dqui\">\n\
<status>\n\
<basic>open</basic>\n\
</status>\n\
<et:class>assistant</et:class>\n\
<ci:homepage>http://partysip.org/</ci:homepage>\n\
<ci:icon>http://partysip.org/</ci:icon>\n\
<ci:card>http://partysip.org/</ci:card>\n\
<et:relationship>assistant</et:relationship>\n\
<et:contact-type>presentity</et:contact-type>\n\
<contact>sip:secretary@partysip.org</contact>\n\
<note>My secretary</note>\n\
</tuple>\n\
\
</presence>");
    }
  osip_message_set_body(notify, buf, strlen(buf));
  osip_message_set_content_type(notify, "application/pidf+xml");

  return 0;
}

void
_eXosip_notify_add_expires_in_2XX_for_subscribe(eXosip_notify_t *jn, osip_message_t *answer)
{
  char tmp[20];
  int now;
  now = time(NULL);

  if (jn->n_ss_expires-now<0)
    {
      tmp[0] = '0';
      tmp[1] = '\0';
    }
  else
    {
      sprintf(tmp, "%i", jn->n_ss_expires-now);
    }
  owsip_message_set_expires(answer, tmp);
}



int
eXosip_answer_subscribe(int did, int status)
{
  eXosip_notify_t *jn = 0;
  eXosip_dialog_t *jd = 0;
  osip_message_t *answer;
  int i;
  osip_transaction_t *tr;
  osip_event_t *evt_answer;

  if (did>0)
    {
      eXosip_notify_dialog_find(did, &jn, &jd);
    }


  if (!jd || !jn)
    return -1;

  tr = jn->n_inc_tr;
  if (!tr)
    return -1;
  
  i = _eXosip_build_response_default(&answer, NULL, status, tr->orig_request);
  i = complete_answer_that_establish_a_dialog(answer, tr->orig_request);
  if (200 <= status && status < 300)
    eXosip_dialog_set_200ok(jd, answer);


  evt_answer = osip_new_outgoing_sipmessage(answer);
  evt_answer->transactionid = tr->transactionid;
  osip_transaction_add_event(tr, evt_answer);
  osip_dialog_set_state(jd->d_dialog, DIALOG_CONFIRMED);


  __eXosip_wakeup();

  return 0;
}
