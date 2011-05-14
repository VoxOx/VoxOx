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

#include <sip.h>
#include <sdp.h>
#include <session.h>
#include <crypto.h>
#include <sessionsManager.h>

#pragma warning(disable:4100)

tSession sessions[MAX_CALLS];

sVoIP_ErrorCode smInit()
{
	int i;
	for (i = 0;i < MAX_CALLS;i++)
	{
		memset(&(sessions[i].session),0,sizeof(EVRB_SESSION));
		sessions[i].cipher = EVRB_NOTINITIALIZE;
		sessions[i].session.state = -1;
	}
	return SOK ;
}

sVoIP_ErrorCode smPreCreate(int id, EVRB_CIPHER_MODE _cipher)
{
	if (id < 0 || id >= MAX_CALLS)
		return SESSION_INDEX_OUT_OF_BOUND;
	if (_cipher < 0)
		return BAD_CIPHER_CODE;
	if (sessions[id].cipher == EVRB_NOTINITIALIZE && sessions[id].session.state == -1)
	{
		memset(&(sessions[id].session),0,sizeof(EVRB_SESSION));
		sessions[id].cipher = _cipher;
		return SOK ;
	} else
		return SESSION_NOT_CLOSE;
}


sVoIP_ErrorCode smUpdate(int id, int msg_type, int incoming)
{
	
	if (id < 0 || id >= MAX_CALLS)
		return SESSION_INDEX_OUT_OF_BOUND;
	if (sessions[id].cipher == EVRB_NOTINITIALIZE || sessions[id].session.state == -1)
		return SESSION_NOT_INITIALIZE;
	if (sessions[id].cipher == /*EVRB_CIPHER_MODE::*/EVRB_NOCRYPTO)
	{
		smClose(id);
#ifndef NDEBUG
		fprintf(stdout,"smUpdate() return SOK::EVRB_NOCRYPTO"); fflush(stdout);
#endif
		return SOK;
	}
	switch (sessions[id].session.state)
	{
		case 0:
			if (msg_type == INVITE && incoming)
			{
				sessions[id].session.state = 1;
				//receive initiator key, done in sVoip.C
			} else
			if (msg_type == INVITE && !incoming)
			{
				sessions[id].session.state = 1;
				//we r initiator , create our key., done in sVoip.C
			}
			return SOK;
		case 1:
			if (msg_type == OK && incoming)
			{
				sessions[id].session.state = 2;
				//finalize session //we r initiator, receive other key, done in sVoip.C
			} else
			if (msg_type == OK && !incoming)
			{
				sessions[id].session.state = 2;
				//finalize session // we have inititor key , create local one (and send it), done in sVoip.C
			}
		return SOK;
		case 2:
			//session is fine...
			break;
		default:
			return SESSION_STATE_HACKED;
	}
	return SOK;
}

sVoIP_ErrorCode smSession(int id,EVRB_SESSION** session,int* ciphermode)
{
	if (id < 0 || id >= MAX_CALLS)
		return SESSION_INDEX_OUT_OF_BOUND;
	if (sessions[id].cipher == EVRB_NOTINITIALIZE && sessions[id].session.state == -1)
		return SESSION_NOT_INITIALIZE;
	*session = &(sessions[id].session);
	*ciphermode = sessions[id].cipher;
	return SOK;
}

sVoIP_ErrorCode smSetAllCipherMode(int ciphermode)
{
	int i;
	for (i = 0;i < MAX_CALLS;i++)
	{
	  if (sessions[i].cipher != EVRB_NOTINITIALIZE)
	    sessions[i].cipher = ciphermode;
	}
	return SOK ;
}

sVoIP_ErrorCode smClose(int id)
{
	if (id < 0 || id >= MAX_CALLS)
		return SESSION_INDEX_OUT_OF_BOUND;
	evrb_crypto_free(sessions[id].session.local.crypto);
	evrb_crypto_free(sessions[id].session.distant.crypto);
	sessions[id].cipher = EVRB_NOTINITIALIZE;
	sessions[id].session.state = -1;
	memset(&(sessions[id].session),0,sizeof(EVRB_SESSION));
	return SOK;
}
