/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2005  Wengo
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "SipXCallbacks.h"

#include "model/sipwrapper/SipCallbacks.h"
#include "model/WengoPhoneLogger.h"

#include <StringList.h>

#include <tapi/sipXtapi.h>

#include <windows.h>

#include <iostream>
using namespace std;

void SipXCallbacks::sipLineCallbackProc(SIPX_LINE lineId, SIPX_LINE_EVENT_TYPE_MAJOR eMajor, void * pUser) {
	SipCallbacks * callbacks = (SipCallbacks *) pUser;

	cout << "SipXCallbacks::sipLineCallbackProc=" << eMajor << " lineId=" << lineId << endl;

	switch (eMajor) {

	//This is the initial Line event state.
	case SIPX_LINE_EVENT_UNKNOWN:
		break;

	//The REGISTERING event is fired when sipXtapi
	//has successfully sent a REGISTER message,
	//but has not yet received a success response from the
	//registrar server.
	case SIPX_LINE_EVENT_REGISTERING:
		break;

	//The REGISTERED event is fired after sipXtapi has received
	//a response from the registrar server, indicating a successful
	//registration.
	case SIPX_LINE_EVENT_REGISTERED:
		//FIXME this event is not fired with sipXtapi
		callbacks->registerProgress(lineId, SipCallbacks::LINE_OK);
		break;

	//The UNREGISTERING event is fired when sipXtapi
	//has successfully sent a REGISTER message with an expires=0 parameter,
	//but has not yet received a success response from the
	//registrar server.
	case SIPX_LINE_EVENT_UNREGISTERING:
		break;

	//The UNREGISTERED event is fired after sipXtapi has received
	//a response from the registrar server, indicating a successful
	//un-registration.
	case SIPX_LINE_EVENT_UNREGISTERED:
		break;

	//The REGISTER_FAILED event is fired to indicate a failure of REGISTRATION.
	//It is fired in the following cases:
	//The client could not connect to the registrar server.
	//The registrar server challenged the client for authentication credentials,
	//and the client failed to supply valid credentials.
	//The registrar server did not generate a success response (status code == 200)
	//within a timeout period.
	case SIPX_LINE_EVENT_REGISTER_FAILED:
		callbacks->registerProgress(lineId, SipCallbacks::SERVER_ERROR);
		break;

	//The UNREGISTER_FAILED event is fired to indicate a failure of un-REGISTRATION.
	//It is fired in the following cases:
	//The client could not connect to the registrar server.
	//The registrar server challenged the client for authentication credentials,
	//and the client failed to supply valid credentials.
	//The registrar server did not generate a success response (status code == 200)
	//within a timeout period.
	case SIPX_LINE_EVENT_UNREGISTER_FAILED:
		break;

	//The PROVISIONED event is fired when a sipXtapi Line is added, and Registration is not
	//requested (i.e. - sipxLineAdd is called with a bRegister parameter of false.
	case SIPX_LINE_EVENT_PROVISIONED:
		break;

	default:
		LOG_FATAL("unknown sipX event=" + String::fromNumber(eMajor));
	}

	/*char szLineURI[1024];
	size_t theSize;
	sipxLineGetURI(hLine, szLineURI, 1024, theSize);
	UtlString logMsg;*/

	/*char * eventDesc = sipxLineEventToString(eMajor);
	printf("<-> Received Event: %s\n", eventDesc);
	free(eventDesc);*/
}

void SipXCallbacks::sipCallbackProc(SIPX_CALL callId, SIPX_LINE lineId, SIPX_CALLSTATE_MAJOR eMajor,
			SIPX_CALLSTATE_MINOR eMinor, void * pUser) {

	lineId += 1;	//FIXME Bug fixed for sipXtapi

	cout << "SipXCallbacks::sipCallbackProc=" << sipxEventToString(eMajor, eMinor) << " lineId=" << lineId << " callId=" << callId << endl;

	SipCallbacks * callbacks = (SipCallbacks *) pUser;

	//char * szEventDesc = sipxEventToString(eMajor, eMinor);
	/*char * eventDesc = sipxEventToString(eMajor);
	printf("<-> Received Event: %s\n", eventDesc);
	free(eventDesc);*/

	switch (eMajor) {

	//An UNKNOWN event is generated when the state for a call
	//is no longer known. This is generally an error
	//condition; see the minor event for specific causes.
	case UNKNOWN:
		break;

	//The NEWCALL event indicates that a new call has been
	//created automatically by the sipXtapi. This event is
	//most frequently generated in response to an inbound
	//call request.
	case NEWCALL:
		/*if (pCallInfo->cause == CALLSTATE_NEW_CALL_TRANSFERRED) {
			mTransferInProgress = true;
			m_hTransferInProgress = pCallInfo->hCall;
			m_hCallController = m_hCall;
			m_hCall = m_hTransferInProgress;
		}*/
		break;

	//The DIALTONE event indicates that a new call has been
	//created for the purpose of placing an outbound call.
	//The application layer should determine if it needs to
	//simulate dial tone for the end user.
	case DIALTONE:
		break;

	//The REMOTE_OFFERING event indicates that a call setup
	//invitation has been sent to the remote party. The
	//invitation may or may not every receive a response. If
	//a response is not received in a timely manor, sipXtapi
	//will move the call into a disconnected state. If
	//calling another sipXtapi user agent, the reciprocate
	//state is OFFER.
	case REMOTE_OFFERING:
		//callbacks->callProgress(lineId, callId, SipCallbacks::CALL_DIALING, String::null);
		break;

	//The REMOTE_ALERTING event indicates that a call setup
	//invitation has been accepted and the end user is in the
	//alerting state (ringing). Depending on the SIP
	//configuration, end points, and proxy servers involved,
	//this event should only last for 3 minutes.  Afterwards,
	//the state will automatically move to DISCONNECTED. If
	//calling another sipXtapi user agent, the reciprocate
	//state is ALERTING.
	case REMOTE_ALERTING:
		//callbacks->callProgress(lineId, callId, SipCallbacks::CALL_RINGING, String::null);
		break;

	//The CONNECTED state indicates that call has been setup
	//between the local and remote party. Audio should be
	//flowing provided and the microphone and speakers should
	//be engaged.
	case CONNECTED:
		//callbacks->callProgress(lineId, callId, SipCallbacks::CALL_OK, String::null);
		/*if (pCallInfo->cause == CALLSTATE_CONNECTED_ACTIVE) {
			PhoneStateMachine::getInstance().OnConnected();
		}
		else if (pCallInfo->cause == CALLSTATE_CONNECTED_ACTIVE_HELD) {
			thePhoneApp->addLogMessage("Remote HOLD\n");
		}
		else if (pCallInfo->cause == CALLSTATE_CONNECTED_INACTIVE) {
			PhoneStateMachine::getInstance().OnConnectedInactive();
		}*/
		break;

	//The DISCONNECTED state indicates that a call was
	//disconnected or failed to connect. A call may move
	//into the DISCONNECTED states from almost every other
	//state. Please review the DISCONNECTED minor events to
	//understand the cause.
	case DISCONNECTED:
		//callbacks->callProgress(lineId, callId, SipCallbacks::CALL_CLOSED, String::null);
		/*if (CALLSTATE_DISCONNECTED_BUSY == pCallInfo->cause) {
			PhoneStateMachine::getInstance().OnRemoteBusy();
		}
		else if (mTransferInProgress && m_hTransferInProgress && pCallInfo->hCall == m_hCallController) {
			m_hTransferInProgress = 0;
			mTransferInProgress = false;
			m_hCallController = 0;
		}
		else {
			PhoneStateMachine::getInstance().OnDisconnected(hCall);
		}*/
		break;

	//An OFFERING state indicates that a new call invitation
	//has been extended this user agent. Application
	//developers should invoke sipxCallAccept(),
	//sipxCallReject() or sipxCallRedirect() in response.
	//Not responding will result in an implicit call
	//sipXcallReject().
	case OFFERING:
		sipxCallAccept(callId);
		break;

	//An ALERTING state indicates that an inbound call has
	//been accepted and the application layer should alert
	//the end user. The alerting state is limited to 3
	//minutes in most configurations; afterwards the call
	//will be canceled. Applications will generally play
	//some sort of ringing tone in response to this event.
	case ALERTING:
		//Sleep(5000);
		sipxCallAnswer(callId);
		//callbacks->callProgress(lineId, callId, SipCallbacks::CALL_INCOMING, String::null);
		break;

	//The DESTROYED event indicates the underlying resources
	//have been removed for a call. This is the last event
	//that the application will receive for any call. The
	//call handle is invalid after this event is received.
	case DESTROYED:
		sipxCallDestroy(callId);
		break;

	default:
		LOG_FATAL("unknown sipX event=" + String::fromNumber(eMajor));
	}
}
