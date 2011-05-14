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

#include <time.h>

#include "eXosip2.h"

extern eXosip_t eXosip;

void eXosip_dialog_set_state(eXosip_dialog_t *jd, int state)
{
  jd->d_STATE = state;
}

int eXosip_call_dialog_find(int jid, eXosip_call_t **jc, eXosip_dialog_t **jd)
{
  for (*jc=eXosip.j_calls; *jc!=NULL; *jc=(*jc)->next)
    {
      for (*jd=(*jc)->c_dialogs; *jd!=NULL; *jd=(*jd)->next)
	{
	  if ((*jd)->d_id==jid)
	    return 0;
	}
    }
  *jd = NULL;
  *jc = NULL;
  return -1;
}

/*
int eXosip_find_replaced_dialog(osip_message_t *sip, eXosip_call_t **jc, eXosip_dialog_t **jd)
{
 *jd = NULL;
 *jc = NULL;

 if (!sip->replaces)
   return -1;

  for (*jc=eXosip.j_calls; *jc!=NULL; *jc=(*jc)->next)
    {
      for (*jd=(*jc)->c_dialogs; *jd!=NULL; *jd=(*jd)->next)
	{
	  if (osip_dialog_match_with_replaces((*jd)->d_dialog, sip->replaces))
	    return 0;
	}
    }
 return -1;
}
*/


int eXosip_notify_dialog_find(int nid, eXosip_notify_t **jn, eXosip_dialog_t **jd)
{
  for (*jn=eXosip.j_notifies; *jn!=NULL; *jn=(*jn)->next)
    {
      for (*jd=(*jn)->n_dialogs; *jd!=NULL; *jd=(*jd)->next)
	{
	  if ((*jd)->d_id==nid)
	      return 0;
	}
    }
  *jd = NULL;
  *jn = NULL;
  return -1;
}

int eXosip_subscribe_dialog_find(int sid, eXosip_subscribe_t **js, eXosip_dialog_t **jd)
{
  for (*js=eXosip.j_subscribes; *js!=NULL; *js=(*js)->next)
    {
      *jd=NULL;
	  if ((*js)->s_id==sid) {
	    *jd = (*js)->s_dialogs;
	    return 0;
	  }
      for (*jd=(*js)->s_dialogs; *jd!=NULL; *jd=(*jd)->next)
	{
	  if ((*jd)->d_id==sid)
	      return 0;
	}
    }
  *jd = NULL;
  *js = NULL;
  return -1;
}

eXosip_dialog_t *
owsip_dialog_get
(
	osip_dialog_t * odialog
)
{
	eXosip_dialog_t * dialog ;
	eXosip_subscribe_t * subscribe ;
	eXosip_notify_t * notify ;
	eXosip_call_t * call ;

	for (subscribe = eXosip.j_subscribes ; subscribe != NULL; subscribe = subscribe->next)
	{
		for (dialog = subscribe->s_dialogs ; dialog != NULL ; dialog = dialog->next)
		{
			if (dialog->d_dialog == odialog)
			{
				return dialog ;
			}
		}
	}

	for (notify = eXosip.j_notifies ; notify != NULL; notify = notify->next)
	{
		for (dialog = notify->n_dialogs ; dialog != NULL ; dialog = dialog->next)
		{
			if (dialog->d_dialog == odialog)
			{
				return dialog ;
			}
		}
	}

	for (call = eXosip.j_calls ; call != NULL; call = call->next)
	{
		for (dialog = call->c_dialogs ; dialog != NULL ; dialog = dialog->next)
		{
			if (dialog->d_dialog == odialog)
			{
				return dialog ;
			}
		}
	}

	return NULL ;
}

int eXosip_dialog_set_200ok(eXosip_dialog_t *jd, osip_message_t *_200Ok)
{
	int i;

	if (jd==NULL) 
		return -1;
	
	if (jd->d_200Ok!=NULL)
		osip_message_free(jd->d_200Ok);
	jd->d_timer = time (NULL) + 2;
	jd->d_count = 0;
	i = osip_message_clone(_200Ok, &(jd->d_200Ok));
	
	if (i!=0) {
		return -1;
	}
	return 0;
}

static char *
get_contact_str(osip_message_t *sip)
{
  osip_contact_t *ct = (osip_contact_t *) osip_list_get(&sip->contacts, 0);
  char *str;

  if (!ct)
    return 0;

  osip_contact_to_str(ct, &str);
  return str;
}

int eXosip_dialog_init_as_uac(eXosip_dialog_t **_jd, OWSIPAccount account, osip_message_t *_200Ok)
{
  int i;
  eXosip_dialog_t *jd;

  *_jd = NULL;
  jd = (eXosip_dialog_t *) osip_malloc(sizeof(eXosip_dialog_t));
  jd->d_id  = -1; /* not yet available to user */
  jd->d_STATE = JD_EMPTY;
  jd->d_localcontact = 0;

  i = owsip_dialog_account_set (jd, account) ;
  if (i != 0)
  {
	osip_free (jd) ;
	return -1 ;
  }

  if (MSG_IS_REQUEST(_200Ok))
    {
      i = osip_dialog_init_as_uac_with_remote_request(&(jd->d_dialog), _200Ok, -1);
      jd->d_localcontact =  get_contact_str(_200Ok);
    }
  else
    { /* normal usage with response */
      i = osip_dialog_init_as_uac(&(jd->d_dialog), _200Ok);
    }
  if (i!=0)
    {
      osip_free(jd);
      return -1;
    }

  jd->media_lines = (osip_list_t*) osip_malloc(sizeof(osip_list_t));
  osip_list_init(jd->media_lines);

  jd->d_count = 0;
  jd->d_timer = time(NULL);
  jd->d_200Ok = NULL;
  jd->d_ack   = NULL;
  jd->next    = NULL;
  jd->parent  = NULL;
  jd->d_out_trs = (osip_list_t*) osip_malloc(sizeof(osip_list_t));
  osip_list_init(jd->d_out_trs);
  jd->d_inc_trs = (osip_list_t*) osip_malloc(sizeof(osip_list_t));
  osip_list_init(jd->d_inc_trs);

  /* jd->d_bh = sdp_handler_new(); */
  *_jd = jd;
  return 0;
}

int eXosip_dialog_init_as_uas(eXosip_dialog_t **_jd, OWSIPAccount account, osip_message_t *_invite, osip_message_t *_200Ok)
{
  int i;
  eXosip_dialog_t *jd;

  *_jd = NULL;
  jd = (eXosip_dialog_t *) osip_malloc(sizeof(eXosip_dialog_t));
  jd->d_id  = -1; /* not yet available to user */
  jd->d_STATE = JD_EMPTY;
  i = owsip_dialog_account_set (jd, account) ;
  if (i != 0)
  {
	osip_free (jd) ;
	return -1 ;
  }

  i = osip_dialog_init_as_uas(&(jd->d_dialog), _invite, _200Ok);
  if (i!=0)
    {
      osip_free(jd);
      return -1;
    }

  jd->d_localcontact = get_contact_str(_200Ok);
  jd->media_lines = (osip_list_t*) osip_malloc(sizeof(osip_list_t));
  osip_list_init(jd->media_lines);

  jd->d_count = 0;
  jd->d_timer = time(NULL);
  jd->d_200Ok = NULL;
  jd->d_ack   = NULL;
  jd->next    = NULL;
  jd->parent  = NULL;
  jd->d_out_trs = (osip_list_t*) osip_malloc(sizeof(osip_list_t));
  osip_list_init(jd->d_out_trs);
  jd->d_inc_trs = (osip_list_t*) osip_malloc(sizeof(osip_list_t));
  osip_list_init(jd->d_inc_trs);

  /* jd->d_bh = sdp_handler_new(); */
  *_jd = jd;
  return 0;
}


static void 
clean_transaction_list(osip_list_t *tlist)
{
  while (!osip_list_eol(tlist, 0))
    {
      osip_transaction_t *tr;
      tr = (osip_transaction_t*) osip_list_get(tlist, 0);
      osip_list_remove(tlist, 0);
      __eXosip_delete_jinfo(tr);
      owsip_list_add_nodup(eXosip.j_transactions, tr, 0);
    }
}

void eXosip_dialog_free(eXosip_dialog_t *jd)
{
  clean_transaction_list(jd->d_inc_trs);
  clean_transaction_list(jd->d_out_trs);

  osip_message_free(jd->d_200Ok);
  osip_message_free(jd->d_ack);

  osip_dialog_free(jd->d_dialog);

  while (!osip_list_eol(jd->media_lines, 0))
    {
      char *tmp = osip_list_get(jd->media_lines, 0);
      osip_list_remove(jd->media_lines, 0);
      osip_free(tmp);
    }

  osip_free(jd->media_lines);
  osip_free(jd->d_out_trs);
  osip_free(jd->d_inc_trs);
  osip_free(jd->d_localcontact);
  osip_free(jd);
}

OWSIPAccount
owsip_dialog_account_get
(
	eXosip_dialog_t * dialog
)
{
	OWSIPAccount account = -1 ;

	if (dialog != NULL)
	{
		account = dialog->account ;
	}

	return account ;
}

int
owsip_dialog_account_set
(
	eXosip_dialog_t * dialog,
	OWSIPAccount account
)
{
	if (dialog == NULL)
	{
		return -1 ;
	}

	dialog->account = account ;

	return 0 ;
}
