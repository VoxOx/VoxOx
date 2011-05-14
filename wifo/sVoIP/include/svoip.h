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

#ifndef _VOIP_H_
#define _VOIP_H_


#define __DEBUG__

#include "error.h"

#define CID unsigned int
#include <sessionsManager.h>
/*
	API initialize
*/
extern sVoIP_ErrorCode sVoIP_init();

/**
* entry point for a given session, prepare the session for a _cipher type
* @param _cid the id of session
* @param _cipher the cipher mode
* @return SOK on succes
*/
extern sVoIP_ErrorCode sVoIP_preCreateSession(CID _cid, EVRB_CIPHER_MODE _cipher);
/**
* call it to modify output invite sip packs
* @param _cid the id of session
* @param _packetSIP the sip pack as a string
* @param _len _packetSIP size
* @return SOK on succes
*/
extern sVoIP_ErrorCode sVoIP_SIPAugmentINVITE(CID _cid, const char** packetSIP, size_t* _len);
/**
* call it to modify output ok sip packs
* @param _cid the id of session
* @param _packetSIP the sip pack as a string
* @param _len _packetSIP size
* @return SOK on succes
*/
extern sVoIP_ErrorCode sVoIP_SIPAugmentOK(CID _cid, const char** packetSIP, size_t* _len);
/**
* call it to handle input invite sip packs
* @param _cid the id of session
* @param _packetSIP the sip pack as a string
* @param _len _packetSIP size
* @return SOK on succes
*/
extern sVoIP_ErrorCode sVoIP_SIPHandleINVITE(CID _cid, const char* _packetSIP, size_t _len);
/**
* call it to handle input ok sip packs
* @param _cid the id of session
* @param _packetSIP the sip pack as a string
* @param _len _packetSIP size
* @return SOK on succes
*/
extern sVoIP_ErrorCode sVoIP_SIPHandleOK(CID _cid, const char* _packetSIP, size_t _len);
/**
* call it to handle input by sip packs
* @param _cid the id of session
* @param _packetSIP the sip pack as a string
* @param _len _packetSIP size
* @return SOK on succes
*/
extern sVoIP_ErrorCode sVoIP_SIPHandleBYE(CID _cid, const char* _packetSIP, size_t _len);

/* ************************ Simple API for wengo ****************************** */

/**
* call it to modify INVITE sdp body
* @param _cid the id of session
* @param sdp the sdp message
* @param len message size
* @return SOK on succes
*/
extern sVoIP_ErrorCode sVoIP_SIPAugmentINVITE2(CID _cid, const char** sdp, size_t* len);
/**
* call it to modify OK sdp body
* @param _cid the id of session
* @param sdp the sdp message
* @param len message size
* @return SOK on succes
*/
extern sVoIP_ErrorCode sVoIP_SIPAugmentOK2(CID _cid, const char** sdp, size_t* len);
/**
* call it to handle INVITE sdp body
* @param _cid the id of session
* @param sdp the sdp message
* @param len message suze
* @return SOK on succes
*/
extern sVoIP_ErrorCode sVoIP_SIPHandleINVITE2(CID _cid, const char *sdp, size_t len);
/**
* call it to handle INVITE OK sdp body
* @param _cid the id of session
* @param sdp the sdp message
* @param len message size
* @return SOK on succes
*/
extern sVoIP_ErrorCode sVoIP_SIPHandleOK2(CID _cid, const char *sdp, size_t len);

/**
* cipher the rtp payload
* @param _cid the id of session
* @param _data rtp payload
* @param _len the cipher mode
* @return SOK on succes
*/
extern int sVoIP_RTPsend(CID _cid,void* _data, size_t* _len);
/**
* uncipher the rtp payload
* @param _cid the id of session
* @param _data rtp payload
* @param _len _data size
* @return SOK on succes
*/
extern int sVoIP_RTPrecv(CID _cid, void* _data, size_t* _len);

#endif
