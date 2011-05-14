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
#include <svoip.h>


static int initialized = 0;

sVoIP_ErrorCode sVoIP_init() {
	int i;
	if (initialized == 1)
		return SOK;
	if (evrb_crypto_init() != 0)
		return CIPHER_INIT;
	if (SOK != (i = smInit()) )
	{
		SVOIP_PERROR(i,stdout,"sVoIP_init");
		return SESSION_MANAGER_INIT;
	}
	initialized = 1;
	return SOK;
}

sVoIP_ErrorCode sVoIP_preCreateSession(CID _cid, EVRB_CIPHER_MODE _cipher)
{
	//do we crypt the CID session? cipher
	return smPreCreate(_cid,_cipher);
}

/* ********************************* NEW API *************************** */

/* Message: INVITE incoming */
sVoIP_ErrorCode sVoIP_SIPHandleINVITE2(CID _cid,const char* sdp,size_t len)
{
	int			i = 0;
	int			cipherMode = 0;
	EVRB_SESSION*		sion = 0;
	char*			rtp_ip;
	unsigned short 		rtp_port;
	char*			crypt_key;

	if (SOK != ( i = smSession(_cid,&sion,&cipherMode)))
	{
		SVOIP_PERROR(i,stdout,"sVoIP_SIPHandleINVITE");
		smClose(_cid);
		return BAD_SESSION;
	}
	if (cipherMode <= EVRB_NOCRYPTO)
	{
		smClose(_cid);
		return BAD_SESSION;
	}
	//update appropriate session
	if (SOK != ( i = smUpdate(_cid, INVITE, 1)))
	{
		SVOIP_PERROR(i,stdout,"sVoIP_SIPHandleINVITE update");
		return BAD_SESSION;
	}
	//check for crypto style and keying
	sdp_parse(sdp, len, &rtp_ip, &rtp_port, &crypt_key);

	if (crypt_key == 0 /*|| strnlen(crypt_key,128)<32*/ )
	  {
		SVOIP_PERROR(BAD_SIP_MESSAGE,stdout,"sVoIP_SIPHandleINVITE parse sdp");
		smClose(_cid);
	  }
	else
	{
		evrb_cryptokey_set(crypt_key,&sion->distant.crypto);
		sion->distant.ip.s_addr = inet_addr (rtp_ip);
		sion->distant.port = rtp_port;
	}
	return SOK;
};

/* Message: OK incoming */
sVoIP_ErrorCode sVoIP_SIPHandleOK2(CID _cid, const char* sdp, size_t len)
{
	EVRB_SESSION*		sion= 0;
	int			i = 0;
	char*			rtp_ip;
	unsigned short 		rtp_port;
	char*			crypt_key;
	int			cipherMode;

	//check_session(session must have been created during a SIPAugmentINVITE)
	if (SOK != ( i = smSession(_cid,&sion,&cipherMode)) )
	{
		SVOIP_PERROR(i,stdout,"sVoIP_SIPHandleOK");
		return BAD_SESSION;
	}
	//update appropriate session
	if (SOK != ( i = smUpdate(_cid, OK, 1)) )
	{
		SVOIP_PERROR(i,stdout,"sVoIP_SIPHandleOK");
		return BAD_SESSION;
	}
	//check for crypto style and keying
	sdp_parse(sdp, len, &rtp_ip, &rtp_port, &crypt_key);

	if (crypt_key == 0 /*|| 0!=strncmp(crypt_key,evrb_cryptokey_get(sion->local.crypto),128)*/ )
	  {
		SVOIP_PERROR(BAD_SIP_MESSAGE,stdout,"sVoIP_SIPHandleINVITE parse sdp");
		smClose(_cid);
	  }
	else
	{
		evrb_cryptokey_set(crypt_key,&sion->distant.crypto);
		sion->distant.ip.s_addr = inet_addr (rtp_ip);
		sion->distant.port = rtp_port;
		//finalize session
		i = evrb_crypto_keys_compute(sion->local.crypto, sion->distant.crypto, 1);
		if( i != 0)
		{
			smClose(_cid);
			return CANNOT_COMPUTE_KEY;
		}
		fprintf(stdout,"---KEY IS OK!!!\n");
		//evrb_crypto_clean_exchanged_key(sion->local.crypto);
		//evrb_crypto_clean_exchanged_key(sion->distant.crypto);
	}
	return SOK;
}

/* Message: INVITE outgoing */
sVoIP_ErrorCode sVoIP_SIPAugmentINVITE2(CID cid, const char** sdp, size_t*	len)
{
	EVRB_SESSION*	session = 0;
	int		i = 0;
	int		cipherMode = 0;

	//check_session
	if (SOK != (i = smSession(cid,&session,&cipherMode)))
	{
		SVOIP_PERROR(i,stdout,"sVoIP_SIPAugmentInvite getSession");
		return BAD_SESSION;
	}
	if (cipherMode <= EVRB_NOCRYPTO)
	{
		smClose(cid);
		return BAD_SESSION;
	}

	//update appropriate session
	if (SOK != (i = smUpdate(cid, INVITE, 0)) ) {
		SVOIP_PERROR(i,stdout,"sVoIP_SIPAugmentInvite update");
		return BAD_SESSION;
	}

	session->status = EVRB_SES_LOCAL;
	//give a g(remote) if agree for crypto

	if (evrb_cryptokey_get(session->local.crypto) == 0)
		evrb_cryptokey_set_gen(&session->local.crypto);

	i = sdp_create(sdp, len, evrb_cryptokey_get(session->local.crypto));
	if (i!= SOK)
	  return i;

	return SOK;
}

/* Message: OK outgoing */
sVoIP_ErrorCode sVoIP_SIPAugmentOK2(CID cid, const char** sdp, size_t*	len)
{
	EVRB_SESSION*	session = 0;
	int		i = 0;
	int		cipherMode = 0;

	//check_session
	if (SOK != (i = smSession(cid,&session,&cipherMode)))
	{
		SVOIP_PERROR(i,stdout,"sVoIP_SIPAugmentOK getSession");
		return BAD_SESSION;
	}

	//update appropriate session
	if (SOK != (i = smUpdate(cid, OK, 0)) ) {
		SVOIP_PERROR(i,stdout,"sVoIP_SIPAugmentOK update");
		return BAD_SESSION;
	}

	session->status = EVRB_SES_DIST;
	//give a g(remote) if agree for crypto

	if (evrb_cryptokey_get(session->local.crypto) == 0)
		evrb_cryptokey_set_gen(&session->local.crypto);

	i = sdp_create(sdp, len, evrb_cryptokey_get(session->local.crypto));
	if (i!= SOK)
	  return i;

	//finalize session
	i = evrb_crypto_keys_compute(session->local.crypto, session->distant.crypto, 0);
	if( i != 0)
	{
		smClose(cid);
		return CANNOT_COMPUTE_KEY;
	}
	fprintf(stdout,"---KEY IS OK!!!\n");
	return SOK;
}

int sVoIP_RTPsend(CID _cid, void* _data, size_t* _len)
{
	EVRB_SESSION* sion = 0;
	int   cipherMode;
	int   i;
	//check_session
	if (SOK != (i=smSession(_cid,&sion,&cipherMode)))
	{
		if (i!=SESSION_NOT_INITIALIZE)
		{
			SVOIP_PERROR(i,stdout,"sVoIP_RTPsend");
			return -1;
		} else
			return SOK;
	}
	if (cipherMode == EVRB_NOCRYPTO || cipherMode == EVRB_NOTINITIALIZE)
		return SOK;
	if (sion->state < 2)
		return SESSION_NOT_READY;
	//check_data
#ifndef NDEBUG
	fprintf(stdout,"---Packet Ciphered--\n");
	i = evrb_encrypt(sion->local.crypto,_data,_len);
	if (i != 0)
		fprintf(stdout,"---sVoIP_RTP_bad_send--\n");fflush(stdout);
	return i;
#else
	return evrb_encrypt(sion->local.crypto,_data,_len);
#endif
};

int sVoIP_RTPrecv(CID _cid, void* _data, size_t* _len)
{
	EVRB_SESSION*	sion = 0;
	int		cipherMode;
	int		i;

	//check_session
	if (SOK != ( i = smSession(_cid,&sion,&cipherMode)))
	{
		if (i != SESSION_NOT_INITIALIZE)
		{
			SVOIP_PERROR(i,stdout,"sVoIP_RTPrecv");
			return -1;
		} else
			return SOK;
	}
	if (cipherMode == EVRB_NOCRYPTO || cipherMode == EVRB_NOTINITIALIZE)
		return SOK;
	if (sion->state < 2)
		return SESSION_NOT_READY;

#ifndef NDEBUG
	fprintf(stdout,"---Packet deciphered--\n");
	i = evrb_decrypt(sion->distant.crypto,_data,_len);
	if (i != 0)
		fprintf(stdout,"---sVoIP_RTP_bad_recv--\n");fflush(stdout);
	return i;
#else
	return evrb_decrypt(sion->distant.crypto,_data,_len);
#endif
};
