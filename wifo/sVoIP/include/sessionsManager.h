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

#ifndef _SESSIONSMANAGER_H_
#define _SESSIONSMANAGER_H_

#define PH_MAX_CALLS  32

#include "error.h"

#define MAX_CALLS  PH_MAX_CALLS

typedef enum
{
	EVRB_NOTINITIALIZE = -1 ,
	EVRB_NOCRYPTO = 0,
	EVRB_SRTP,EVRB_AUTO 
} EVRB_CIPHER_MODE;

typedef struct s_session
{
	EVRB_SESSION session;
	EVRB_CIPHER_MODE cipher;
} tSession;

extern tSession sessions[MAX_CALLS];

/**
 * initialize array sessions.
 *
 * @return              0  on succes
 */
extern sVoIP_ErrorCode smInit();

/**
 * get session at id. 
 *
 * @param id            the id of session
 * @param session       a pointer to the session stored at id.
 * @param ciphermode    (out) the cipher mode for this session.
 *
 * @return              SOK on success
 */
extern sVoIP_ErrorCode smSession(int id, EVRB_SESSION** session, int* ciphermode);

/**
 * precreate a session at id with cipher mode  (O for no cipher, 1 evb..
 *
 * @param id            the id of session
 * @param cipher        the cipher type to use
 *
 * @return              return 0 on succes , -3 if a session already exist at id
 */
extern sVoIP_ErrorCode smPreCreate(int id, EVRB_CIPHER_MODE cipher);

/**
* close
*
 * @param id            the id of session to close
*/
extern sVoIP_ErrorCode smClose(int id);

/**
 * update the precreated session at id according to informations.
 *
 * @param id            the id of session to close
 * @param msg_type      type of the message
 * @param incoming	message is incoming
 */
extern sVoIP_ErrorCode smUpdate(int id, int msg_type, int incoming);

/**
 * update the cipher mode of all opened sessions
 *
 * @param cipherMode	cipherMode
 */
extern sVoIP_ErrorCode smSetAllCipherMode(int cipherMode);

#endif
