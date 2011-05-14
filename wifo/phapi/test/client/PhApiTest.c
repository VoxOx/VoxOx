#include "wlm_password.h"

#include <string.h>
#include <owlibc.h>
#include <phapi.h>
#include <wlm.h>
#include <nortel.h>

#define ACCOUNT_MAX 1
OWPL_LINE line [ACCOUNT_MAX] ;
OWPL_CALL call [ACCOUNT_MAX] ;
int isRegistered [ACCOUNT_MAX] ;
int isConnected [ACCOUNT_MAX] ;
int hasError [ACCOUNT_MAX] ;

#define WLM_ADAPTER_NAME "wlm"
#define NORTEL_ADAPTER_NAME "nortel"

static void printStep (const char * step)
{
	printf ("\n%s...\n", step) ;
}

static void printStepAccount (const char * step, int account)
{
	if (ACCOUNT_MAX == 1)
	{
		printf ("\n%s...\n", step) ;
	}
	else
	{
		printf ("\n%s [account %d / %d]...\n", step, account + 1, ACCOUNT_MAX) ;
	}
}

static void userWait (const char * nextStep)
{
	printf ("\npress return to %s...\n", nextStep) ;
	getchar () ;
}

static void userWaitAccount (const char * nextStep, int account)
{
	if (ACCOUNT_MAX == 1)
	{
		printf ("\npress return to %s...\n", nextStep) ;
	}
	else
	{
		printf ("\npress return to %s [account %d / %d]...\n", nextStep, account + 1, ACCOUNT_MAX) ;
	}
	getchar () ;
}

static char * getStringResult (OWPL_RESULT result)
{
	static char stringResult [20] ;
	switch (result)
	{
		case OWPL_RESULT_SUCCESS :
			strcpy (stringResult, "SUCCESS") ; 
			break ;
		case OWPL_RESULT_FAILURE :
			strcpy (stringResult, "FAILURE") ;
			break ;
		case OWPL_RESULT_NOT_IMPLEMENTED :
			strcpy (stringResult, "NOT_IMPLEMENTED") ;
			break ;
		case OWPL_RESULT_OUT_OF_MEMORY :
			strcpy (stringResult, "OUT_OF_MEMORY") ;
			break ;
		case OWPL_RESULT_INVALID_ARGS :
			strcpy (stringResult, "INVALID_ARGS") ;
			break ;
		case OWPL_RESULT_BAD_ADDRESS :
			strcpy (stringResult, "BAD_ADDRESS") ;
			break ;
		case OWPL_RESULT_OUT_OF_RESOURCES :
			strcpy (stringResult, "OUT_OF_RESOURCES") ;
			break ;
		case OWPL_RESULT_INSUFFICIENT_BUFFER :
			strcpy (stringResult, "INSUFFICIENT_BUFFER") ;
			break ;
		case OWPL_RESULT_BUSY :
			strcpy (stringResult, "BUSY") ;
			break ;
		case OWPL_RESULT_INVALID_STATE :
			strcpy (stringResult, "INVALID_STATE") ;
			break ;
		default :
			strcpy (stringResult, "UNEXPECTED_RESULT") ;
	}
	return stringResult ;
}

#define CHECK_SUCCESS(statement) checkSuccessOrExit (statement, __FILE__, __LINE__)

static void checkSuccessOrExit (OWPL_RESULT result, const char * sourceFileName, int sourceLineNumber)
{
	if (result != OWPL_RESULT_SUCCESS)
	{
		char * shortFileName ;
		char * separator ;
		const char separatorArray [3] = {'/', '\\', ':'} ;
		int separatorIndex ;
		shortFileName = (char *) sourceFileName ;
		for (separatorIndex = 0 ; separatorIndex < 3 ; separatorIndex ++)
		{
			separator = strrchr (shortFileName, separatorArray [separatorIndex]) ;
			if (separator != NULL && separator [1] != 0)
			{
				shortFileName = separator + 1 ;
			}
		}
		printf ("%s in file %s at line %d\n", getStringResult (result), shortFileName, sourceLineNumber) ;
		printf ("\npress return to exit\n") ;
		getchar () ;
		exit (result) ;
	}
}

#define PRINT_EVENT(eventString, event, cause) printf ("EVENT %d / %d (%s)\n", eventString, event, cause)

static int accountGetFromLine (OWPL_LINE _line)
{
	int account ;
	for (account = 0 ; account < ACCOUNT_MAX ; account ++)
	{
		if (line [account] == _line)
		{
			break ;
		}
	}
	return account ;
}

static int accountGetFromLineInfo (OWPL_LINESTATE_INFO * info)
{
	return accountGetFromLine (info->hLine) ;
}

static int accountGetFromCallInfo (OWPL_CALLSTATE_INFO * info)
{
	return accountGetFromLine (info->hLine) ;
}

static int eventHandler (OWPL_EVENT_CATEGORY category, void * info, void * userData)
{
	/* unused parameter */
	userData = NULL ;
	switch (category)
	{
		case EVENT_CATEGORY_LINESTATE :
		{
			OWPL_LINESTATE_INFO * lineInfo = (OWPL_LINESTATE_INFO *) info ;
			switch (lineInfo->event)
			{
				case LINESTATE_REGISTERING :
					PRINT_EVENT (lineInfo->event, lineInfo->cause, "registering") ;
					break ;
				case LINESTATE_REGISTERED :
					PRINT_EVENT (lineInfo->event, lineInfo->cause, "registered") ;
					isRegistered [accountGetFromLineInfo (lineInfo)] = 1 ;
					break ;
				case LINESTATE_REGISTER_FAILED :
					PRINT_EVENT (lineInfo->event, lineInfo->cause, "register failed") ;
					isRegistered [accountGetFromLineInfo (lineInfo)] = 0 ;
					hasError [accountGetFromLineInfo (lineInfo)] = 1 ;
					break ;
				case LINESTATE_UNREGISTERING :
					PRINT_EVENT (lineInfo->event, lineInfo->cause, "unregistering") ;
					break ;
				case LINESTATE_UNREGISTERED :
					PRINT_EVENT (lineInfo->event, lineInfo->cause, "unregistered") ;
					isRegistered [accountGetFromLineInfo (lineInfo)] = 0 ;
					break ;
				case LINESTATE_UNREGISTER_FAILED :
					PRINT_EVENT (lineInfo->event, lineInfo->cause, "unregister failed") ;
					hasError [accountGetFromLineInfo (lineInfo)] = 1 ;
					break ;
				case LINESTATE_PROVISIONED :
					PRINT_EVENT (lineInfo->event, lineInfo->cause, "provisioned") ;
					break ;
				default :
					PRINT_EVENT (lineInfo->event, lineInfo->cause, "unknown linestate event") ;
					hasError [accountGetFromLineInfo (lineInfo)] = 1 ;
			}
			break ;
		}
		case EVENT_CATEGORY_CALLSTATE :
		{
			OWPL_CALLSTATE_INFO * callInfo = (OWPL_CALLSTATE_INFO *) info ;
			switch (callInfo->event)
			{
				case CALLSTATE_NEWCALL :
					PRINT_EVENT (callInfo->event, callInfo->cause, "new call") ;
					break;
				case CALLSTATE_REMOTE_OFFERING :
					PRINT_EVENT (callInfo->event, callInfo->cause, "remote offering") ;
					break;
				case CALLSTATE_REMOTE_ALERTING :
					PRINT_EVENT (callInfo->event, callInfo->cause, "remote alerting") ;
					break;
				case CALLSTATE_CONNECTED :
					PRINT_EVENT (callInfo->event, callInfo->cause, "connected") ;
					isConnected [accountGetFromCallInfo (callInfo)] = 1 ;
					break;
				case CALLSTATE_DISCONNECTED :
					PRINT_EVENT (callInfo->event, callInfo->cause, "disconnected") ;
					if (isConnected [accountGetFromCallInfo (callInfo)])
					{
						isConnected [accountGetFromCallInfo (callInfo)] = 0 ;
					}
					else
					{
						hasError [accountGetFromCallInfo (callInfo)] = 1 ;
					}
					break;
				case CALLSTATE_OFFERING :
					PRINT_EVENT (callInfo->event, callInfo->cause, "offering") ;
					 /* Accept the call. The answer 180 Ringing will be sent */
					owplCallAccept (callInfo->hCall, 0) ;
					break;
				case CALLSTATE_ALERTING :
					PRINT_EVENT (callInfo->event, callInfo->cause, "alerting") ;
					 /* Answer the call right away when it is in ringing state */
					owplCallAnswer (callInfo->hCall, OWPL_STREAM_AUDIO) ;
					break;
				case CALLSTATE_DESTROYED :
					PRINT_EVENT (callInfo->event, callInfo->cause, "destroyed") ;
					if (isConnected [accountGetFromCallInfo (callInfo)])
					{
						isConnected [accountGetFromCallInfo (callInfo)] = 0 ;
					}
					else
					{
						hasError [accountGetFromCallInfo (callInfo)] = 1 ;
					}
					break;
				case CALLSTATE_AUDIO_EVENT :
					PRINT_EVENT (callInfo->event, callInfo->cause, "audio event") ;
					break;
				case CALLSTATE_VIDEO_EVENT :
					PRINT_EVENT (callInfo->event, callInfo->cause, "video event") ;
					break;
				case CALLSTATE_TRANSFER :
					PRINT_EVENT (callInfo->event, callInfo->cause, "transfer") ;
					break;
				case CALLSTATE_REDIRECTED :
					PRINT_EVENT (callInfo->event, callInfo->cause, "redirected") ;
					break;
				case CALLSTATE_HOLD :
					PRINT_EVENT (callInfo->event, callInfo->cause, "hold") ;
					break;
				case CALLSTATE_SECURITY_EVENT :
					PRINT_EVENT (callInfo->event, callInfo->cause, "security event") ;
					break;
				case CALLSTATE_IDENTITY_CHANGE :
					PRINT_EVENT (callInfo->event, callInfo->cause, "identity change") ;
					break;
				default :
					PRINT_EVENT (callInfo->event, callInfo->cause, "unknown callstate event") ;
					hasError [accountGetFromCallInfo (callInfo)] = 1 ;
			}
			break ;
		}
		case EVENT_CATEGORY_NOTIFY :
		{
			/* TODO: NOTIFY */
			/*OWPL_NOTIFICATION_INFO * callInfo = (OWPL_NOTIFICATION_INFO *) info ;*/
			break ;
		}
		case EVENT_CATEGORY_SUB_STATUS :
		{
			/* TODO: SUB_STATUS */
			/*OWPL_SUBSTATUS_INFO * callInfo = (OWPL_SUBSTATUS_INFO *) info ;*/
			break ;
		}
		case EVENT_CATEGORY_ERROR :
		{
			/* TODO: ERROR */
			/*OWPL_ERROR_INFO * callInfo = (OWPL_ERROR_INFO *) info ;*/
			break ;
		}
		default :
		{
		}
	}
	return 0 ;
}

int main (void)
{
	int account ;

	char *            adapter         [ACCOUNT_MAX] ;
	char *            domainName      [ACCOUNT_MAX] ;
	char *            proxyIp         [ACCOUNT_MAX] ;
	OWPL_TRANSPORT_PROTOCOL transport [ACCOUNT_MAX] ;
	char *            userId          [ACCOUNT_MAX] ;
	char *            userPassword    [ACCOUNT_MAX] ;
	char *            userDisplayName [ACCOUNT_MAX] ;
	char *            userStatus      [ACCOUNT_MAX] ;
	char *            calleeURI       [ACCOUNT_MAX] ;

	adapter         [0] = NULL ;
	domainName      [0] = "voip.wengo.fr" ;
	proxyIp         [0] = "213.91.9.206" ;
	transport       [0] = OWPL_TRANSPORT_UDP ;
	userId          [0] = "user" ;
	userPassword    [0] = "sip_password" ;
	userDisplayName [0] = "name" ;
	userStatus      [0] = "online" ;
	calleeURI       [0] = "sip:333@voip.wengo.fr" ;

	for (account = 0 ; account < ACCOUNT_MAX ; account ++)
	{
		isRegistered [account] = 0 ;
		isConnected [account] = 0 ;
	}

	printStep ("Initializing PhAPI") ;
	CHECK_SUCCESS (owplConfigAddAudioCodecByName ("PCMU")) ;
	CHECK_SUCCESS (owplConfigAddAudioCodecByName ("PCMA")) ;
	CHECK_SUCCESS (owplAudioSetConfigString ("pa:")) ;
	/*CHECK_SUCCESS (owplAudioSetConfigString ("alsa:IN=default OUT=default")) ;*/

	CHECK_SUCCESS (owplConfigSetHttpTunnel ("80.118.99.31", 80, 0, 3)) ;		//VOXOX - JRT - 2009.07.01 
	CHECK_SUCCESS (owplConfigEnableHttpTunnel (0, 0)) ;							//VOXOX - JRT - 2009.07.01 
	
	CHECK_SUCCESS (owplInit (1, 5060, 5060, 5061, NULL, 0)) ;
	CHECK_SUCCESS (owplEventListenerAdd(eventHandler, 0)) ;
	CHECK_SUCCESS (owplAdapterWlmInitialize (WLM_ADAPTER_NAME)) ;
	CHECK_SUCCESS (owplAdapterNortelInitialize (NORTEL_ADAPTER_NAME)) ;

	for (account = 0 ; account < ACCOUNT_MAX ; account ++)
	{
		printStepAccount ("Opening a line", account) ;
		CHECK_SUCCESS (owplLineAdd (userDisplayName [account], userId [account], domainName [account], proxyIp [account], transport [account], 3500, & line [account])) ;
		if (adapter [account] != NULL)
		{
			CHECK_SUCCESS (owplLineSetAdapter (line [account], adapter [account], NULL, & line [account])) ;
		}
		if (adapter [account] != NULL && strcmp (adapter [account], "wlm") == 0)
		{
			char wlm_sip_password [1000] ;
			userPassword [account] = wlm_sip_password_get
			(
				userId [account],
				domainName [account],
				userPassword [account],
				wlm_sip_password,
				sizeof (wlm_sip_password)
			) ;
		}
		CHECK_SUCCESS (owplLineAddCredential (line [account], userId [account], userPassword [account], domainName [account])) ;
		CHECK_SUCCESS (owplLineRegister (line [account], 1)) ;
		while (! isRegistered [account])
		{
			if (hasError [account])
			{
				goto end ;
			}
			msleep (500) ;
		}
		CHECK_SUCCESS (owplPresencePublish (line [account], 1, userStatus [account], NULL)) ;
		msleep (500) ;
	}

	for (account = 0 ; account < ACCOUNT_MAX ; account ++)
	{
		printStepAccount ("Calling", account) ;
		CHECK_SUCCESS (owplCallCreate (line [account], & call [account])) ;
		CHECK_SUCCESS (owplCallConnect (call [account], calleeURI [account], OWPL_STREAM_AUDIO)) ;
		while (! isConnected [account])
		{
			if (hasError [account])
			{
				goto end ;
			}
			msleep (500) ;
		}
	}

	for (account = 0 ; account < ACCOUNT_MAX ; account ++)
	{
		if (isConnected [account])
		{
			userWaitAccount ("close the call", account) ;
			printStepAccount ("Ending the call", account) ;
			if (isConnected [account])
			{
				CHECK_SUCCESS (owplCallDisconnect (call [account])) ;
			}
			while (isConnected [account])
			{
				if (hasError [account])
				{
					goto end ;
				}
				msleep (500) ;
			}
		}
	}

end :

	for (account = 0 ; account < ACCOUNT_MAX ; account ++)
	{
		hasError [account] = 0 ;
		printStepAccount ("Closing the line", account) ;
		if (isRegistered [account])
		{
			CHECK_SUCCESS (owplPresencePublish (line [account], 0, NULL, NULL)) ;
			CHECK_SUCCESS (owplLineRegister (line [account], 0)) ;
			while (isRegistered [account])
			{
				if (hasError [account])
				{
					break ;
				}
				msleep (500) ;
			}
		}
		CHECK_SUCCESS (owplLineDelete (line [account], 1)) ;
	}

	printStep ("Cleaning PhAPI") ;
	CHECK_SUCCESS (owplShutdown ()) ;

	userWait ("close the application") ;
	return 0 ;
}
