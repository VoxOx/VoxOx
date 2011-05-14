#include "phapi.h"
#include "phcall.h"
#include <eXosip/eXosip.h>

phcall_t *
ph_locate_call_by_cid(phConfig_t *cfg, int cid)
{
  phcall_t *ca;

  for(ca = cfg->ph_calls; ca < &cfg->ph_calls[PH_MAX_CALLS];  ca++)
    {
      if (ca->cid == cid)
	  {
		return ca;
	  }
    }

  return 0;
}

void phReleaseTerminatedCalls(phConfig_t *cfg)
{
	phcall_t *ca;
	for(ca = cfg->ph_calls; ca < &cfg->ph_calls[PH_MAX_CALLS];  ca++)
    {
		if ((ca->cid != -1) && (ph_media_is_stream_stopped(ca) == 1))
		{
			ph_release_call(cfg, ca);
		}
    }
}

phcall_t *
ph_locate_call_by_rcid(phConfig_t *cfg, int cid)
{
  phcall_t *ca;

  for(ca = cfg->ph_calls; ca < &cfg->ph_calls[PH_MAX_CALLS];  ca++)
    {
      if (ca->rcid == cid)
	  {
		return ca;
	  }
    }

  return 0;
}

phcall_t *
ph_locate_call_by_rdid(phConfig_t *cfg, int did)
{
  phcall_t *ca;


  for(ca = cfg->ph_calls; ca < &cfg->ph_calls[PH_MAX_CALLS];  ca++)
    {
      if (ca->rdid == did)
	  {
		return ca;
	  }
    }

  return 0;
}


phcall_t *
ph_allocate_call(phConfig_t *cfg, int cid)
{
  phcall_t *ca = ph_locate_call_by_cid(cfg, -1);

  if (!ca)
  {
    return 0;
  }

  ca->cid = cid;
  ca->cfg = cfg;
  return ca;
}


phcall_t *
ph_locate_call(phConfig_t *cfg, eXosip_event_t *je, int creatit)
{
  phcall_t *ca, *found = 0, *newca = 0;


  /* lookup matching call descriptor */
  for(ca = cfg->ph_calls; ca < &cfg->ph_calls[PH_MAX_CALLS];  ca++)
    {
      if (ca->cid == -1 && !newca)
	  {
		newca = ca;
	  }

      if (ca->cid == je->cid)
	{
	  found  = ca;
	  break;
	}
    }


  ca = found;

  if (!ca)   /* we didn't find a matching call descriptor */
    {
    if (creatit)   
      {
	/* allocate a new one */
	if (!newca)
	{
	  return 0; /* !!! BUG !!! */
	}
	ca = newca;
	memset(ca, 0, sizeof(*ca));
	ca->cid = -1;
      }
    }

 
  
  if (!ca)
  {
    return 0;
  }


  /* update the call information */

  if (!ca->localrefer)
    {
      ca->cid = je->cid;
      ca->did = je->did;
    }

  if (je->remote_sdp_audio_ip[0])
    {
      strncpy(ca->remote_sdp_audio_ip, je->remote_sdp_audio_ip, sizeof(ca->remote_sdp_audio_ip));
      ca->remote_sdp_audio_port = je->remote_sdp_audio_port;
      strncpy(ca->audio_payload_name, je->payload_name, sizeof(ca->audio_payload_name));
      ca->audio_payload = je->payload;
    }

  if (je->remote_sdp_video_ip[0])
    {
      strncpy(ca->remote_sdp_video_ip, je->remote_sdp_video_ip, sizeof(ca->remote_sdp_video_ip));
      ca->remote_sdp_video_port = je->remote_sdp_video_port;
      strncpy(ca->video_payload_name, je->payload_name, sizeof(ca->video_payload_name));
      ca->video_payload = je->payload;
    }


  return ca;
}


void ph_release_call(phcall_t *ca)
{
  if (ca->hasaudio)
    {
      ph_media_stop(ca);
      ca->cid = -1;
    }
}
