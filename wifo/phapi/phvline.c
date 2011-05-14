#include <assert.h>
#include <eXosip/eXosip.h>

#include <phvline.h>
#include <phapi-old.h>
#include <owsl.h>


#if defined(WIN32) || defined(_WIN32_WCE)
#define snprintf _snprintf
#define strncasecmp strnicmp
#define strcasecmp stricmp
#define usleep(usecs) Sleep((usecs)/1000)
#endif


phVLine ph_vlines[PH_MAX_VLINES];

static osip_from_t *ph_parse_from(const char *userid);


phVLine* ph_vlid2vline(int vlid) 
{
	if (vlid < 1 || vlid > PH_MAX_VLINES)
	{
		return NULL ;
	}
	return ph_vlines + (vlid) - 1;
}

int ph_vline2vlid(phVLine* vl)
{
	return ((vl) - ph_vlines + 1);
}

phVLine *
ph_valid_vlid2(int vlid)
{
	phVLine *vl = 0;

	if (vlid > 0 && vlid <= PH_MAX_VLINES)
	{
		vl = ph_vlid2vline(vlid);
		if (!vl->used)
		{
			vl = 0;
		}
	}
	return vl;
}

phVLine *
ph_valid_vlid(int vlid)
{
	phVLine *vl = 0;

	if (vlid > 0 && vlid <= PH_MAX_VLINES)
	{
		vl = ph_vlid2vline(vlid);
		if (!vl->used || vl->LineState == LINESTATE_DELETING)
		{
			vl = 0;
		}
	}
	return vl;
}

// Allocate memory for a new phVLine structure
phVLine * vline_alloc()
{
  int i;

  for(i = 0; i < PH_MAX_VLINES; i++)
    {
      phVLine *vl = ph_vlines + i;

      if (!vl->used)
	{
	  memset(vl, 0, sizeof(*vl));
	  vl->used = 1;
	  return vl;
	}

    }
  return 0;
}

// Free resource used by the specified phVLine pointer
void vline_free(phVLine *vl)
{
	if (vl->used)
	{
		vl->used = 0;
	}
}

phVLine *
ph_find_vline_by_rid(int rid)
{
  int i;

  for(i = 0; i < PH_MAX_VLINES; i++)
    {
      phVLine *vl = ph_vlines + i;

      if (vl->used && rid == vl->rid)
	return vl;
    }
  return 0;
}

int ph_vline_get_id_from_event(struct eXosip_event * je)
{
	return owplLineGetFromSipAccount (je->account) ;
}

phVLine *
ph_find_matching_vline(const char *userid, int ignore)
{
  osip_from_t *from;
  phVLine *vl;
  char *host;
  char hostport[256];
  char *uname;

  hostport[0] = 0;
  from = ph_parse_from(userid);

  if (!from)
    return 0;

  if (from->url)
    uname = from->url->username;
  else
    uname = hostport;


  if (!(ignore & PHM_IGNORE_PORT) && from->url && from->url->port)
    {
      snprintf(hostport, sizeof(hostport), "%s:%s", from->url->host, from->url->port);
      host = hostport;
    }
  else if (ignore & PHM_IGNORE_HOST)
    host = 0;
  else if (from->url)
    {
      host = from->url->host;
    }
  else
    host = hostport;

  vl = ph_find_matching_vline2(uname, host, ignore);

  osip_from_free(from);

  return vl;

}

phVLine *
ph_find_matching_vline2(const char *username, const char* host, int ignore)
{
	char srvbuf[256];
	int port;
	
	ph_split_host_port(srvbuf, sizeof(srvbuf), host, &port);

	//return ph_find_matching_vline3(username, host, 0, ignore);
	return ph_find_matching_vline3(username, srvbuf, port, ignore);
}

phVLine *
ph_find_matching_vline3(const char *username, const char* host, int port, int ignore)
{
	int i;
	int hostlen;
	int unamelen;
	phVLine *vl, *defaultvl = 0;

	if (!username) {
		username = "";
	}

	hostlen = host ? strlen(host) : 0;
	unamelen =  strlen(username);

	if (port == 0) {
		port = 5060;
	}

	for( i = 0; i < PH_MAX_VLINES; i++)
	{
		int len;
		vl = ph_vlines + i;

		if (!vl->used) {
			continue;
		}

		if (! owsip_account_domain_get (vl->sipAccount))
		{
			defaultvl = vl;
			continue;
		}

		len = strlen(owsip_account_domain_get (vl->sipAccount));

		if ((ignore & PHM_IGNORE_HOST) || 
			((len == hostlen) && !strcasecmp(host, owsip_account_domain_get (vl->sipAccount))))
		{
			len = strlen(owsip_account_user_get (vl->sipAccount));

			if ((len == unamelen) && !strcasecmp(username, owsip_account_user_get (vl->sipAccount)))
			{
				if (ignore & PHM_IGNORE_PORT)
				{
					defaultvl = vl;
					break;
				}
				if (port == owsip_account_port_get (vl->sipAccount))
				{
					defaultvl = vl;
					break;
				}
			}
		}
	}
	return defaultvl;
}

static time_t last_vline_refresh;
static time_t last_nat_refresh;

static void
ph_nat_refresh(phVLine *vl)
{
	char buf[128];
	char to[128];
	assert(vl);

	ph_vline_get_from(buf, sizeof(buf), vl);
	snprintf(to, sizeof(to), "sip:ping@%s", owsip_account_domain_get (vl->sipAccount));
	phSendOptions(ph_vline2vlid (vl), buf, to);
}

void ph_refresh_vlines()
{
	int i;
	phVLine *vl;

	time_t now = time(NULL);

	if (now - last_vline_refresh > 5)
	{
		for (i = 0; i < PH_MAX_VLINES; i++)
		{
			vl = ph_vlines + i;
			if (!vl->used)
				continue;

			if (vl->LineState == LINESTATE_REGISTERED)
			{
				if (vl->regTimeout > 0)
				{
					if ((now - vl->lastRegTime) > (vl->regTimeout - 15))
					{
						phvlRegister(ph_vline2vlid(vl));
					}
				}
				if (vl->publishInfo.publishTimeout > 0)
				{
					if ((now - vl->publishInfo.lastPublishTime) > (vl->publishInfo.publishTimeout - 5))
					{
						owplPresencePublish(ph_vline2vlid(vl), vl->publishInfo.onlineState, vl->publishInfo.szStatus, vl->publishInfo.hPub);
					}
				}
			}
		}
		last_vline_refresh = time(0);
	}

	if (phcfg.nat_refresh_time > 0)
	{
		// This is to support a deprecated mode when using the function owplConfigSetNat.
		// This if block is to be removed (but keep the else block)

		if (now - last_nat_refresh > phcfg.nat_refresh_time)
		{
			for( i = 0; i < PH_MAX_VLINES; i++)
			{
				vl = ph_vlines + i;
				if (!vl->used) {
					continue;
				}

				if (vl->keepAliveInfo.enabled && vl->regTimeout > 0)
				{
					ph_nat_refresh(vl);
				}
			}
			last_nat_refresh = time(0);
		}
	}
	else 
	{
		for( i = 0; i < PH_MAX_VLINES; i++)
		{
			vl = ph_vlines + i;
			if (!vl->used)
			{
				continue;
			}
			if (vl->keepAliveInfo.enabled &&
				(now - (time_t) vl->keepAliveInfo.lastSentTime) > ((time_t) vl->keepAliveInfo.period))
			{
				if (vl->LineState == LINESTATE_REGISTERED)
				{
					char to[128];

					snprintf(to, sizeof(to), "sip:%s@%s",
						owsip_account_user_get(vl->sipAccount),
						owsip_account_domain_get(vl->sipAccount));
					phLineSendOptions (ph_vline2vlid(vl), to);
					vl->keepAliveInfo.lastSentTime = time(0);
				}
			}
		}
	}
}

int
phvlRegisterNoLock(int vlid)
{
	phVLine *vl = ph_vlid2vline(vlid);
	int ret = -1;
	char utmp[256];
	char stmp[256];
	char *server;
	char *user = NULL;
	char *domain = NULL;

	assert(vl);

	user = owsip_account_user_get (vl->sipAccount);
	domain = owsip_account_domain_get (vl->sipAccount);

	if (user && domain)
	{
		snprintf(utmp, sizeof(utmp), "sip:%s@%s", owsip_account_user_get (vl->sipAccount), owsip_account_domain_get (vl->sipAccount));

		server = stmp;
		if (owsip_account_port_get (vl->sipAccount) && owsip_account_port_get (vl->sipAccount) != 5060)
		{
			/* snprintf(stmp, sizeof(stmp), "sip:%s@%s:%d", vl->username, vl->server, vl->port); */
			snprintf(stmp, sizeof(stmp), "sip:%s:%d", owsip_account_domain_get (vl->sipAccount), owsip_account_port_get (vl->sipAccount));
		}
		else
		{
			/* snprintf(stmp, sizeof(stmp), "sip:%s@%s:%d", vl->username, vl->server, vl->port); */
			snprintf(stmp, sizeof(stmp), "sip:%s", owsip_account_domain_get (vl->sipAccount)); 
		}

		vl->rid = eXosip_register_init(vl->sipAccount, utmp, server, owsip_account_proxy_get (vl->sipAccount));

		if (vl->rid >= 0)
		{
			if (vl->regTimeout > 0)
			{
				if (vl->LineState != LINESTATE_REGISTERED && vl->LineState != LINESTATE_REGISTERING)
				{
					_owplLineSetState(vlid, LINESTATE_REGISTERING, LINESTATE_CAUSE_NORMAL);
				}
			}
			else
			{
				if (vl->LineState == LINESTATE_REGISTERED)
				{
					_owplLineSetState(vlid, LINESTATE_UNREGISTERING, LINESTATE_CAUSE_NORMAL);
				}
			}

			ret = eXosip_register(vl->rid, vl->regTimeout);

			if (ret == 0)
			{
				ret = vl->rid;
				vl->lastRegTime = time(0);
			}
		}
	} else  {
		ret = vl->rid;
		vl->lastRegTime = time(0);
	}

	return ret;
}

int
phvlRegister(int vlid)
{
	int ret;

	eXosip_lock();
	ret = phvlRegisterNoLock(vlid);
	eXosip_unlock();

	return ret;
}

int
phvlUnregister(int vlid)
{
	phVLine *vl = ph_vlid2vline(vlid);

	if (vl) {
		int oldTimeout = vl->regTimeout;
		int ret = -1;

		//_owplLineSetState(vlid, LINESTATE_UNREGISTERING, LINESTATE_CAUSE_NORMAL);
		vl->regTimeout = 0; // For backward compatibility
		ret = phvlRegister(vlid);
		vl->regTimeout = oldTimeout;
		return ret;
	}
	return -1;
}

void ph_vlines_init()
{
	memset(ph_vlines, 0, sizeof(ph_vlines));
}

static osip_from_t *ph_parse_from(const char *userid)
{
	osip_from_t *from;

	osip_from_init(&from);
	if (!from) {
		return 0;
	}

	osip_from_parse(from, userid); 
	if (from->url && from->url->port && !strcmp(from->url->port, "5060"))
	{
		osip_free(from->url->port);
		from->url->port = NULL;
	}

	return from;
}

void
   ph_vline_get_user_domain(char *buf, int bufSize, phVLine *vl)
{
	char *un, *s;

	assert(buf);
	assert(vl);

	un = (owsip_account_user_get (vl->sipAccount) && owsip_account_user_get (vl->sipAccount)[0]) ? owsip_account_user_get (vl->sipAccount) : "unknown";
	s = (owsip_account_domain_get (vl->sipAccount) && owsip_account_domain_get (vl->sipAccount)[0]) ? owsip_account_domain_get (vl->sipAccount) : "localhost";
	snprintf(buf, bufSize, "sip:%s@%s", un, s);
}

void
   ph_vline_get_from(char *buf, int bufSize, phVLine *vl)
{
	char *un, *s;

	assert(buf);
	assert(vl);

	un = (owsip_account_user_get (vl->sipAccount) && owsip_account_user_get (vl->sipAccount)[0]) ? owsip_account_user_get (vl->sipAccount) : "unknown";
	s = (owsip_account_domain_get (vl->sipAccount) && owsip_account_domain_get (vl->sipAccount)[0]) ? owsip_account_domain_get (vl->sipAccount) : "localhost";

	if (owsip_account_name_get (vl->sipAccount) && owsip_account_name_get (vl->sipAccount)[0])
	{
		if (!strchr(owsip_account_name_get (vl->sipAccount), ' '))
		{
			snprintf(buf, bufSize, "%s <sip:%s@%s>", owsip_account_name_get (vl->sipAccount), un, s);
		}
		else
		{
			snprintf(buf, bufSize, "\"%s\" <sip:%s@%s>", owsip_account_name_get (vl->sipAccount), un, s);
		}
	}
	else
	{
		snprintf(buf, bufSize, "<sip:%s@%s>", un, s);
	}
}

void _owplLineSetState(OWPL_LINE hLine, OWPL_LINESTATE_EVENT LineState, OWPL_LINESTATE_CAUSE LineStateCause)
{
	phVLine *vl;
	vl = ph_vlid2vline(hLine);
	if (vl && vl->used && vl->LineState != LineState)
	{
		vl->LineState = LineState;
		owplFireLineEvent(hLine, LineState, LineStateCause, 0);
	}
}

void owplLinesCheck (void)
{
	phVLine * vline ;
	size_t vline_index ;
	time_t idle_time_max ;
	for (vline_index = 0 ; vline_index < PH_MAX_VLINES ; vline_index ++)
	{
		vline = ph_vlines + vline_index ;
		if (! vline->used)
		{
			continue ;
		}
		idle_time_max = owsip_account_idle_time_max_get (vline->sipAccount) ;
		if (idle_time_max > 0 && owsip_account_idle_time_get (vline->sipAccount) > idle_time_max)
		{
			// treat this as register failure
			if (owsip_account_idle_time_reset (vline->sipAccount) == 0)
			{
				if (phcb && phcb->regProgress)
				{
					phcb->regProgress (0, -1) ;
				}
				vline->LineState = LINESTATE_UNREGISTERED;
				owplFireLineEvent (ph_vline2vlid(vline),
					LINESTATE_REGISTER_FAILED, LINESTATE_CAUSE_COULD_NOT_CONNECT, 0) ;
			}
		}
	}
}

OWSIPAccount
owplLineSipAccountGet
(
	const OWPL_LINE line
)
{
	phVLine * vl = ph_valid_vlid (line) ;
	if (vl == NULL)
	{
		return 0 ;
	}
	return vl->sipAccount ;
}

OWPL_LINE
owplLineGetFromSipAccount
(
	const OWSIPAccount sipAccount
)
{
	phVLine * vline ;
	size_t vline_index ;
	for (vline_index = 0 ; vline_index < PH_MAX_VLINES ; vline_index ++)
	{
		vline = ph_vlines + vline_index ;
		if (! vline->used)
		{
			continue ;
		}
		if (vline->sipAccount == sipAccount)
		{
			return ph_vline2vlid (vline) ;
		}
	}
	return 0 ;
}
