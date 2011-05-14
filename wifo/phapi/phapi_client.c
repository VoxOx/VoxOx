/*
  The phapi_client module implements simple RPC interface to phApi server
  application based on eXosip stack
  Copyright (C) 2004  Vadim Lebedev  vadim@mbdsys.com
  
  This library is free software; you can redistribute it and/or
  modify it under any terms you wish provided this copyright message is included.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.


*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#ifndef _WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <pthread.h>

#define SOCKET int
#define closesocket close
#define ioctlsocket ioctl

#else /* !WIN32 */
#include <process.h>
#include <winsock2.h>
#define snprintf _snprintf
typedef unsigned int pthread_t;
#endif   /* !_WIN32 */

#define USETHREADS
#define FILE "phapi_client.c"

#include "phapi.h"
#include "phrpc.h"

/**
 * socket on wich requests are sent and immediate retcode returned
 */
static SOCKET ph_reqsock;

/**
 * socket on which callback messages are delivered
 */
static SOCKET ph_cbksock;

/**
 * phApi server address
 */
static struct sockaddr_in ph_serveraddr;



phCallbacks_t *phcb;

unsigned short phServerPort = PH_SERVER_PORT;
unsigned short phCallBackPort = PH_CALLBACK_PORT; 

int phDebugLevel;
char *phLogFileName;

static void ph_fatal_error(char *msg,  char *data);
static void ph_event_get();

void (*phFatalError)(char *msg,  char *data) =  ph_fatal_error;


phConfig_t phcfg = { "10600", "", "5060" };

#ifdef USETHREADS
#ifndef _WIN32
pthread_t 
ph_thread_create (int stacksize, void *(*func) (void *), void *arg)
{
  int i;
  pthread_t thread;

  i = pthread_create (&thread, NULL, func, (void *) arg);
  if (i != 0)
    {
      perror("phapi_client error creating thread:");
      exit(2);
      return 0;
    }
  return thread;
}
#else

pthread_t
ph_thread_create (int stacksize, void *(*func) (void *), void *arg)
{
  pthread_t thread;
  typedef void (*thrfun)(void *);
  

  thread = (pthread_t) _beginthread ((thrfun)func, 0, arg);

  if (thread == 0)
    {
      fprintf(stderr, "phapi_client: error creating thread");
      exit(2);
      return NULL;
    }
  return thread;
}
#endif
#endif


static void ph_fatal_error(char *msg,  char *data)
{
  printf("phapi_client: %s %s\n", msg, data);
}



static int 
ph_rpc2(const void *req, int len)
{
  int i = sendto(ph_reqsock, req, len, 0, (struct sockaddr *)&ph_serveraddr, sizeof(ph_serveraddr));
  char resp[32];

  if (i < 0)
    {
      phFatalError("Error sending packet to remote", "");
      return -PH_RPCERR;
    }


  do {
    i = recv(ph_reqsock, resp, sizeof(resp), 0);

  } while ((i < 0) && (errno == EAGAIN));

  if (i < 0)
    return -PH_RPCERR;

  sscanf(resp, "RET %d", &i);

  return i;

}

static int 
ph_rpc(const char *req)
{
  return ph_rpc2(req, strlen(req)+1);

}
    

#define stringize(v) #v
int 
phGetVersion()
{
  if (!ph_reqsock)
    {
      /* phInit was not called yet */
      static const char versn[] = stringize(PHAPI_VERSION);
      char *subv = strstr(versn, ".");
      int v,s,r;

      v = atoi(versn);
      s = atoi(subv+1);
      r = atoi(strstr(subv+1, "."));

      return (v << 16) | (s << 8) | r;
    }

  return ph_rpc("VER");

}  
#undef stringize  


int 
phPlaceCall2(const char *from, const char *uri, void *userData, int rcid)
{
  char reqbuf[1000];

  snprintf(reqbuf, sizeof(reqbuf), "CALL UDATA=%08lx RCID=%d FROM=%s TO=%s", (unsigned long) userData, rcid, from, uri);
  return ph_rpc(reqbuf);
}


int 
phLinePlaceCall(int vlid, const char *uri, void *userData, int rcid)
{
  char reqbuf[1000];

  snprintf(reqbuf, sizeof(reqbuf), "CALL VLID=%d UDATA=%08lx RCID=%d TO=%s", (unsigned long) userData, rcid,  uri);
  return ph_rpc(reqbuf);
}


int 
phPlaceCall(const char *from, const char *uri, void *userData)
{
  return phPlaceCall2(from, uri, userData, 0);
}




int 
phSendOptions(const char *from, const char *to)
{
  char reqbuf[1000];

  snprintf(reqbuf, sizeof(reqbuf), "OPT FROM=%s TO=%s",  from, to);
  return ph_rpc(reqbuf);
}


int 
phAcceptCall2(int cid, void *udata)
{
  char reqbuf[64];

  snprintf(reqbuf, sizeof(reqbuf), "ACCEPT CID=%d UDATA=%08lx", cid, (unsigned long)udata);
  return ph_rpc(reqbuf);
}

int 
phRejectCall(int cid, int reason)
{
  char reqbuf[64];

  snprintf(reqbuf, sizeof(reqbuf), "REJECT CID=%d REASON=%d", cid, reason);
  return ph_rpc(reqbuf);

}

int 
phRingingCall(int cid)
{
  char reqbuf[64];

  snprintf(reqbuf, sizeof(reqbuf), "RINGING CID=%d", cid);
  return ph_rpc(reqbuf);

}



int 
phCloseCall(int cid)
{
  char reqbuf[64];

  snprintf(reqbuf, sizeof(reqbuf), "CLOSE CID=%d", cid);
  return ph_rpc(reqbuf);

}



int 
phBlindTransferCall(int cid, const char *uri)
{
  char reqbuf[1000];

  snprintf(reqbuf, sizeof(reqbuf), "BTXCALL CID=%d TO=%s", cid, uri);
  return ph_rpc(reqbuf);

}


int 
phTransferCall(int cid, int tcid)
{
  char reqbuf[1000];

  snprintf(reqbuf, sizeof(reqbuf), "TXCALL CID=%d CIDT=%d", cid, tcid);
  return ph_rpc(reqbuf);

}

MY_DLLEXPORT  int 
phSetContact(int vlid, const char *uri)
{
  char reqbuf[1000];

  snprintf(reqbuf, sizeof(reqbuf), "CONTACT VLID=%d FROM=%s", vlid, uri);
  return ph_rpc(reqbuf);

}

int 
phConf(int cid1, int cid2)
{
  char reqbuf[1000];

  snprintf(reqbuf, sizeof(reqbuf), "CONF CID=%d CIDT=%d", cid1, cid2);
  return ph_rpc(reqbuf);

}
int 
phStopConf(int cid1, int cid2)
{
  char reqbuf[1000];

  snprintf(reqbuf, sizeof(reqbuf), "NOCONF CID=%d CIDT=%d", cid1, cid2);
  return ph_rpc(reqbuf);

}

int 
phResumeCall(int cid)
{
  char reqbuf[64];

  snprintf(reqbuf, sizeof(reqbuf), "RESUME CID=%d", cid);
  return ph_rpc(reqbuf);


}


int 
phHoldCall(int cid)
{
  char reqbuf[64];

  snprintf(reqbuf, sizeof(reqbuf), "HOLD CID=%d", cid);
  return ph_rpc(reqbuf);


}


int 
phSetFollowMe(const char *uri)
{
  char reqbuf[1000];

  snprintf(reqbuf, sizeof(reqbuf), "FOLLOWME TO=%s", uri);
  return ph_rpc(reqbuf);

}

int 
phSetBusy(int busyFlag)
{
  char reqbuf[64];

  snprintf(reqbuf, sizeof(reqbuf), "SETBUSY %d", busyFlag);
  return ph_rpc(reqbuf);


}

int 
phLineSetFollowMe(int vlid, const char *uri)
{
  char reqbuf[1000];

  snprintf(reqbuf, sizeof(reqbuf), "FOLLOWME VLID=%d TO=%s", vlid, uri);
  return ph_rpc(reqbuf);

}

int 
phLineSetBusy(int vlid, int busyFlag)
{
  char reqbuf[64];

  snprintf(reqbuf, sizeof(reqbuf), "SETBUSY VLID=%d F=%d", vlid, busyFlag);
  return ph_rpc(reqbuf);


}


int
phAddAuthInfo(const char *username, const char *userid,
			       const char *passwd, const char *ha1,
	      const char *realm)
{
  char reqbuf[1000];

  snprintf(reqbuf, sizeof(reqbuf), "AUTH U=%s ID=%s P=%s R=%s", username, userid, passwd, realm);
  return ph_rpc(reqbuf);


}

int 
phRegister2(const char *username, const char *server, int timeout)
{
  char reqbuf[1000];

  snprintf(reqbuf, sizeof(reqbuf), "REG T=%d U=%s S=%s", timeout, username, server);
  return ph_rpc(reqbuf);
}



int 
phSendDtmf(int cid, int dtmfEvent, int mode)
{
  char reqbuf[64];

  snprintf(reqbuf, sizeof(reqbuf), "DTMF CID=%d E=%d MODE=%d", cid, dtmfEvent, mode);
  return ph_rpc(reqbuf);

}





int 
phPlaySoundFile(const char *fileName , int loop)
{
  char reqbuf[256];

  snprintf(reqbuf, sizeof(reqbuf), "PLAY LOOP=%d FILE=%s", loop, fileName);
  return ph_rpc(reqbuf);

}



int 
phSendSoundFile(int cid, const char *fileName)
{
  char reqbuf[256];

  snprintf(reqbuf, sizeof(reqbuf), "SENDSF CID=%d FILE=%s", cid, fileName);
  return ph_rpc(reqbuf);

}





int phStopSoundFile()
{
  char reqbuf[16];

  snprintf(reqbuf, sizeof(reqbuf), "PLSTOP");
  return ph_rpc(reqbuf);
  
}


int phSetSpeakerVolume(int cid,  int volume)
{
  char reqbuf[32];

  snprintf(reqbuf, sizeof(reqbuf), "SETVOL CID=%d V=%d", cid, volume);
  return ph_rpc(reqbuf);
  
}


int phSetRecLevel(int cid,  int level)
{
  char reqbuf[32];

  snprintf(reqbuf, sizeof(reqbuf), "SETLVL CID=%d L=%d", cid, level);
  return ph_rpc(reqbuf);
  
}


int phDelVline(int vlid)
{
  char reqbuf[32];

  snprintf(reqbuf, sizeof(reqbuf), "DELVL VLID=%d", vlid);
  return ph_rpc(reqbuf);

}

int phAddVline(const char* username, const char *server, const char*  proxy,  int regTimeout)
{
  char reqbuf[1000];


  snprintf(reqbuf, sizeof(reqbuf), "ADDVL U=%s S=%s P=%s T=%d", username, server ? server : "", proxy ? proxy : "", regTimeout);
  return ph_rpc(reqbuf);



}


int phAddVline2(const char *displayname, const char* username, const char *server, const char*  proxy,  int regTimeout)
{
  char reqbuf[1000];


  snprintf(reqbuf, sizeof(reqbuf), "ADDVL DN=<%s> U=%s S=%s P=%s T=%d", 
	   displayname ? displayname : "", 
	   username, 
	   server ? server : "", 
	   proxy ? proxy : "", 
	   regTimeout);
  return ph_rpc(reqbuf);

}


int phChangeAudioDevices(const char *devstr)
{
 char reqbuf[521];


 snprintf(reqbuf, sizeof(reqbuf), "AUDIO DEV=%s", devstr ? devstr : "");

 return ph_rpc(reqbuf);

}

void *
ph_api_thread(void *arg)
{

  //  while(1)
    ph_event_get();

  return 0;
}


/**
 * Initialize the phoneapi module
 * @param cbk          pointer to callback descriptor
 * @param asyncmode    when != 0 a thread will be created to deliver
 *
 */
int 
phInit(phCallbacks_t *cbk, char *server, int asyncmode)
{
  struct sockaddr_in service;
  struct
  {
    char msgcode[8];
    phConfig_t cfg;
  }  cfgmsg;
  int one = 1;
  int i;
#ifdef WIN32
  WORD wVersionRequested;
  WSADATA wsaData;

  wVersionRequested = MAKEWORD(1,1);
  if(i = WSAStartup(wVersionRequested,  &wsaData))
       {
		   perror(FILE);
		   exit(1);
       }
#endif /* !WIN32 */
#ifndef USETHREADS
  if (asyncmode)
    return -1;
#endif
    
  phcb = cbk;

  ph_reqsock = socket(AF_INET, SOCK_DGRAM, 0);
  ph_cbksock = socket(AF_INET, SOCK_DGRAM, 0);

  ph_serveraddr.sin_family = AF_INET;
  ph_serveraddr.sin_port = htons(phServerPort);
  ph_serveraddr.sin_addr.s_addr = inet_addr(server);


  /* set socket to non-blocking mode */
  ioctlsocket(ph_cbksock, FIONBIO, &one);

  service.sin_family = AF_INET;
  service.sin_port = htons(phCallBackPort);
  service.sin_addr.s_addr = 0;

  if (bind(ph_cbksock, (struct sockaddr *)&service, sizeof(service)))
    {
      phFatalError("Error binding socket", "");
      return -1;
    }

  connect(ph_reqsock, (struct sockaddr *) &ph_serveraddr, sizeof(ph_serveraddr));

  cfgmsg.msgcode[0] = 'P'; 
  cfgmsg.msgcode[1] = 'H'; 
  cfgmsg.msgcode[2] = 'I'; 
  cfgmsg.msgcode[3] = 'N';
  cfgmsg.msgcode[4] = 'I';
  cfgmsg.msgcode[5] = 'T';
  cfgmsg.msgcode[6] = '0';
  cfgmsg.msgcode[7] = ' ';
  

  cfgmsg.cfg = phcfg;

  i = ph_rpc2(&cfgmsg, sizeof(cfgmsg));

#ifdef USETHREADS
  if (i == 0 && asyncmode)
    ph_thread_create(20000, ph_api_thread, 0);
#endif

  return i;
    

}


void 
phTerminate()
{
  ph_rpc("TERM");
  closesocket(ph_reqsock);
  closesocket(ph_cbksock);
  ph_reqsock = 0;

}


static void 
ph_event_get();

/**
 * poll for phApi events
 */
void phPoll()
{

  if (!phcfg.asyncmode)
    ph_event_get();
}






static char *
ph_parseCallProgressUri(char* data)
{
  char *tag = strstr(data, "URI=");

  if (!tag)
    return 0;

  return tag+4;
}


static char *
ph_parseCallProgressUri2(char* data)
{
  char *tag = strstr(data, "URI2=");
  char *eos;

  if (!tag)
    return 0;

  eos = tag+4;
  while(*eos && *eos != ' ')
    *eos++;

  *eos = 0;

  return tag+4;
}




static int 
ph_parseCallProgressCid(char* data)
{
  char *tag = strstr(data, "CID=");

  if (!tag)
    return 0;

  return atoi(tag+4);
}


static int 
ph_parseCallProgressNewCid(char* data)
{
  char *tag = strstr(data, "CIDN=");

  if (!tag)
    return 0;

  return atoi(tag+5);
}


static int 
ph_parseCallProgressVlId(char* data)
{
  char *tag = strstr(data, "VLID=");

  if (!tag)
    return 0;

  return atoi(tag+5);
}


static int 
ph_parseRegProgressRid(char* data)
{
  char *tag = strstr(data, "RID=");

  if (!tag)
    return 0;

  return atoi(tag+4);
}

static int
ph_parseCallProgressErrCode(char *data, int *errcode)
{
  char *tag = strstr(data, "ERR=");

  if (!tag)
    return -1;

  *errcode = atoi(tag+4);
  return 0;
}




static unsigned long
ph_parseCallProgressUdata(char *data)
{
  char *tag = strstr(data, "UDATA=");
  unsigned long ret;

  if (!tag)
    return (unsigned long)-1;

  sscanf(tag+6, "%08lx", &ret);
  return ret;
}

  

#define xx(n) #n
static const char *ph_call_event_names[] =
{
  xx(DIALING), xx(RINGING), xx(NOANSWER), xx(CALLBUSY), xx(CALLREDIRECTED), xx(CALLOK), 
  xx(CALLHELD), xx(CALLRESUMED), xx(HOLDOK), xx(RESUMEOK), 
  xx(INCALL), xx(CALLCLOSED), xx(CALLERROR), xx(DTMF),
  xx(XFERPROGRESS), xx(XFEROK), xx(XFERFAIL), xx(XFERREQ), xx(CALLREPLACED)
};
#undef xx
  

static int
ph_parseCallProgressEvent(char *data)
{
  char *tag;
  int i;
  const int last = sizeof(ph_call_event_names)/sizeof(ph_call_event_names[0]);


  tag = strstr(data, "EVT=");

  if (!tag)
    return -1;

  tag += 4;

  for( i = 0; i < last; i++)
    {
    const char *name = ph_call_event_names[i];

    if (!strncmp(name, tag, strlen(name)))
	return i;
    }

  return -1;
}




void ph_handleCallProgress(char *data)
{
  phCallStateInfo_t cinfo;
  int errCode;
  char *uri = NULL;
  int cid;
  int event;
  int newcid;
  int vlid;


  cid = ph_parseCallProgressCid(data);
  newcid = ph_parseCallProgressNewCid(data);
  vlid  = ph_parseCallProgressVlId(data);
  if (cid == -1)
    {
      phFatalError("Invalid CID in message", data);
      return;
    }
    

  event = ph_parseCallProgressEvent(data);
  if (event == -1)
    {
      phFatalError("Invalid event in message", data);
      return;
    }

  cinfo.event = event;

  if (0 != ph_parseCallProgressErrCode(data, &errCode))
    cinfo.u.remoteUri = ph_parseCallProgressUri(data);
  else
    cinfo.u.errorCode = errCode;


  cinfo.userData = (void *) ph_parseCallProgressUdata(data);

  cinfo.localUri = ph_parseCallProgressUri2(data);

  cinfo.newcid = newcid;
  cinfo.vlid = vlid;

  phcb->callProgress(cid, &cinfo);
     
}


void ph_handleConfProgress(char *data)
{
}

void ph_handleTransferProgress(char *data)
{
}


void ph_handleRegProgress(char *data)
{
  int rid;
  int status;

  rid = ph_parseRegProgressRid(data);
  if (rid == -1)
    {
      phFatalError("Invalid RID in message", data);
      return;
    }
    
  ph_parseCallProgressErrCode(data, &status);
  phcb->regProgress(rid, status);

}




void ph_event_get()
{
  int i;
  char data[1024];
  

  while( 1 )
    {
      do {
	i = recv(ph_cbksock, data, sizeof(data), 0);
      } while ((i < 0) && (errno == EAGAIN));

      if (i <= 0)
	return;

      if (!strncmp(data, "CL", 2))
	ph_handleCallProgress(data);
      else if (!strncmp(data, "TX", 2))
	ph_handleTransferProgress(data);
      else if (!strncmp(data, "CF", 2))
	ph_handleConfProgress(data);
      else if (!strncmp(data, "RG", 2))
	ph_handleRegProgress(data);
      else if (!strncmp(data, "PING", 4))
	{
	}
    }
}






