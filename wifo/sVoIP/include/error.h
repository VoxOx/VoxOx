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

#ifndef _VOIPERROR_H_
#define _VOIPERROR_H_

#include <stdio.h>

typedef enum e_error_code
{
		SOK  = 0,
		BAD_CIPHER_CODE,
		SESSION_INDEX_OUT_OF_BOUND,
		SESSION_NOT_CLOSE,
		SESSION_NOT_INITIALIZE,
		SESSION_STATE_HACKED,
		SESSION_MANAGER_INIT,
		SESSION_NOT_READY,
		CIPHER_INIT,
		CIPHER_ERROR,
		BAD_SESSION,
		BAD_SIP_MESSAGE,
		CANNOT_COMPUTE_KEY,
		INIT_ERROR,
		MAX_ERROR_CODE
} sVoIP_ErrorCode;

extern char* sVoIP_error_message[];

extern int svoip_perror(sVoIP_ErrorCode _code, FILE* _out, char* _custom);

#ifdef _DEBUG_
#define SVOIP_PERROR(_code,_out,_custom) svoip_perror(_code,_out,_custom)
#else
#define SVOIP_PERROR(a,b,c)
#endif


#endif
