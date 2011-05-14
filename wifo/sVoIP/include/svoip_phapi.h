 /*
	Zenbow Secure VoIP.
	Copyright (C) 2006 Everbee Networks 

	* This program is free software; you can redistribute it and/or
	* modify it under the terms of the GNU General Public License
	* as published by the Free Software Foundation; either version 2
	* of the License, or (at your option) any later version.

	* This program is distributed in the hope that it will be useful,
	* but WITHOUT ANY WARRANTY; without even the implied warranty of
	* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	* GNU General Public License for more details.

	* You should have received a copy of the GNU General Public License
	* along with this program; if not, write to the Free Software
	* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

	* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
	* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
	* COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
	* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
	* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
	* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
	* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
	* OF THE POSSIBILITY OF SUCH DAMAGE.

*/

/*

The use of sVoIP is :
call smPreCreate( the cid of session, 0 for uncipher, 1 for everbee cipher)
or call phLinePlaceCall(vlid, uri, cipher, 0) with cipher to 1 to cipher or to 0 if not
a smPreCreate call overwrite a phLinePlaceCall choice.
To (pre)create a session it must be close, and btw,
at the end u may call smClose(the cid of session) (this is called on cancel and by sip message)

*/

#ifndef _VOIPPHAPI_H_
#define _VOIPPHAPI_H_

#include <osip2/osip.h>
#include "error.h"

/*
* Enable/disable use of ciphering on calls
* @param cipherMode 0 = No crypto, 1 = Crypted
*/
void sVoIP_phapi_setCipherMode(int cipherMode);

/*
* Get the status of ciphering on calls
* @return  0 = No crypto, 1 = Crypted
*/
int sVoIP_phapi_getCipherMode();

/*
* Indicate if the call is crypted or not
* @param cid CID of the call
* @return 1 if the call is crypted, 0 if the call is not cryted
*/
int sVoIP_phapi_isCrypted(int cid);

/*
* (precreate a session) 'on incoming invite' call evb crypto api
* @param cid CID
* @param sip the invite sip packet
* @return 0 on success
*/
extern int sVoIP_phapi_handle_invite_in(int cid, osip_message_t *sip);
/*
* 'on Ok invite' call evb crypto api only for ok reply to invite
* @param cid CID
* @param sip the ok sip packet
* @return 0 on success
*/
extern int sVoIP_phapi_handle_ok_in(int cid, osip_message_t *sip);
/*
* (close a session) 'on bye invite' call evb crypto api
* @param cid CID
* @param sip the bye sip packet
* @return 0 on success
*/
extern int sVoIP_phapi_handle_bye_in(int cid, osip_message_t *sip);
/*
* call it when a session is cancelled
* @param cid CID
* @param sip the cancel sip packet
* @return 0 on success
*/
extern int sVoIP_phapi_handle_cancel_in(int cid, osip_message_t *sip);
/*
* (precreate a session) modify outgoing 'invite' sip message 
* @param cid CID
* @param sip the ok sip packet
* @return 0 on success
*/
extern int sVoIP_phapi_handle_invite_out(int cid, osip_message_t *sip);
/*
* modify outgoing 'ok' sip message 
* @param cid CID
* @param sip the ok sip packet
* @return 0 on success
*/
extern int sVoIP_phapi_handle_ok_out(int cid, osip_message_t *sip);
/*
* (close session) handle 'bye' outgoing sip message
* @param cid CID
* @param sip the ok sip packet
* @return 0 on success
*/
extern int sVoIP_phapi_handle_bye_out(int cid, osip_message_t *sip);

#if 0
/*
* call when a rtp is reiceived in phapi
* @param cid CID
* @param error error code
* @param buffer buffer to receive data
* @param len buffer size
*/
extern void sVoIP_phapi_recvRtp(int cid, void *error, void *buffer, int *len);

/*
* call before a rtp is send in phapi
* @param cid CID
* @param error error code
* @param buffer buffer to send
* @param len bytes to send
*/
extern void sVoIP_phapi_sendRtp(int cid, void *error, void *buffer, int *len);
#endif

#endif
