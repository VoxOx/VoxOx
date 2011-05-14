#ifndef _OWPL_VLINE_HEADER_
#define _OWPL_VLINE_HEADER_

#include "phapi.h"
#include "phevents.h"
#include "owpl_adapter.h"
/* TODO: remove eXosip.h dependency */
#include <eXosip.h> /* for OWSIPAccount */

struct eXosip_event;

typedef struct phVLinePublishInfo_T
{
	unsigned int onlineState ;
	char * szStatus ;
	OWPL_PUB * hPub ;
	long lastPublishTime ;
	long publishTimeout ;
} phVLinePublishInfo ;

typedef struct phVLineKeepAliveInfo_T
{
	unsigned int enabled ;
	unsigned int period ;
	long lastSentTime ;
	char * sip_method ;
	char * destination ;
} phVLineKeepAliveInfo ;

typedef struct phVLine_T
{
	 /* Because we use a static array to store all vline, this property indicate if 
	the specified vline in the array is free to use or is already being used */
	int used ;

	/* line parameters */
	OWPL_LINE_CONFIGURATION_HOOK configurationHook ;
	void * configurationHookUserData ;
	OWPL_LINESTATE_EVENT LineState ;
	int busy ;

	/* sip parameters */
	OWSIPAccount sipAccount ;
	long regTimeout ;
	long lastRegTime ;
	int rid ;
	phVLinePublishInfo publishInfo ;
	phVLineKeepAliveInfo keepAliveInfo ;
} phVLine ;

#define PH_MAX_VLINES 16

#define PHM_IGNORE_PORT 1
#define PHM_IGNORE_HOST 2


//Initialize all data related to vline functions
void ph_vlines_init();

// ph_vlid2vline: Get phVline structure from line id
phVLine* ph_vlid2vline(int vlid);

// ph_vline2vlid: Get line id from phVline structure
int ph_vline2vlid(phVLine* vl);

//Get a phVline from a line id. Return 0 if vlid is not inuse or being deleted
phVLine * ph_valid_vlid(int vlid);

//Get a phVline from a line id. Return 0 if vlid is not inuse
phVLine * ph_valid_vlid2(int vlid);

// Allocate memory for a new phVLine structure
phVLine* vline_alloc();

// Free resource used by the specified phVLine pointer
void vline_free(phVLine *vl);

phVLine * ph_find_vline_by_rid(int rid);

int ph_vline_get_id_from_event(struct eXosip_event * je);

phVLine * ph_find_matching_vline(const char *userid, int ignore);

phVLine * ph_find_matching_vline2(const char *username, const char* host, int ignore);

phVLine * ph_find_matching_vline3(const char *username, const char* host, int port, int ignore);

int phvlRegisterNoLock(int vlid);
int phvlRegister(int vlid);

int phvlUnregister(int vlid);

void ph_refresh_vlines();

void ph_vline_get_from(char *buf, int bufSize, phVLine *vl);
void ph_vline_get_user_domain(char *buf, int bufSize, phVLine *vl);

void _owplLineSetState(OWPL_LINE hLine, OWPL_LINESTATE_EVENT LineState, OWPL_LINESTATE_CAUSE LineStateCause);

void owplLinesCheck (void) ;

OWSIPAccount 
owplLineSipAccountGet
(
	const OWPL_LINE line
) ;

OWPL_LINE
owplLineGetFromSipAccount
(
	const OWSIPAccount sipAccount
) ;

#endif
