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



extern eXosip_t eXosip;

int eXosip_msg_find(int mid, eXosip_msg_t **jm)
{
  for (*jm=eXosip.j_msgs; *jm!=NULL; *jm=(*jm)->next)
    {
      if ((*jm)->m_id==mid)
	{
	  return 0;
	}
    }
  *jm = NULL;
  return -1;
}


int eXosip_msg_init(eXosip_msg_t **jm)
{
  static int m_id;

  *jm = (eXosip_msg_t*) osip_malloc(sizeof(eXosip_msg_t));
  if (*jm==NULL) return -1;

  if (m_id > 1000000)			/* keep it non-negative */
  	m_id = 0;

  (*jm)->m_id         = ++m_id;


  (*jm)->next   = NULL;
  (*jm)->m_last_tr   = NULL;

  return 0;
}

void eXosip_msg_free(eXosip_msg_t *jm)
{
  __eXosip_delete_jinfo(jm->m_last_tr);
  if (jm->m_last_tr!=NULL)
    owsip_list_add_nodup(eXosip.j_transactions, jm->m_last_tr, 0);

  osip_free(jm);
}
