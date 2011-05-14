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

#include <sdp.h>
#include <stdlib.h>

#include <string.h>
//4 vc8
/*
#ifdef WIN32
#define strncpy(dest, sizedest, source, sizesource) strncpy_s(dest,sizedest,  source, sizesource)
#define strncat(dest, sizedest, source, sizesource) strncat_s(dest, source, sizesource)
#define sscanf sscanf_s
#define strtok(data,token,context) strtok_s(data,token,context)
#else*/
#if 1
#define strncpy(dest, sizedest, source, sizesource) strncpy (dest, source,sizedest)
#define strncat(dest, sizedest, source, sizesource) strncat (dest, source, sizedest)
#endif


int sdp_parse(char *buf, size_t len, char **rtp_ip, unsigned short *rtp_port, char **crypt_key)
{
	char *tmpbuf;
	char *tmpline;
	char *tmpchr1;
	char *tmpchr2;
	char *ipc = NULL;
	char *ipo = NULL;
	char *port = NULL;
	char *key = NULL;
	int  cfound = 0;
	int  ofound = 0;
	int  mfound = 0;
	int  afound = 0;

	*rtp_ip = NULL;
	*rtp_port = 0;
	if(crypt_key)
		*crypt_key = NULL;

	// TODO avoid alloc and strtok
	tmpbuf = (char*)malloc(len+1);
	memcpy ( tmpbuf, buf, len );
	tmpbuf[len] = '\0';

	tmpline = strtok(tmpbuf,"\n");

	while((tmpline != NULL) && (tmpline[1]=='=')) {
		switch(tmpline[0]) {
			case 'c':
			/* We try to find <connection address> in the c= line (optional)*/
			/* c=<network type> <address type> <connection address>*/
				tmpchr1=strchr(tmpline,' ');
				tmpchr1=strchr(tmpchr1+1,' ');
				tmpchr2=strchr(tmpchr1+1,'\r');
				ipc = (char*)malloc(tmpchr2-tmpchr1);
				if (!ipc) return -1;
				strncpy (ipc,tmpchr2-tmpchr1,tmpchr1+1,tmpchr2-tmpchr1-1);
				ipc[tmpchr2-tmpchr1-1] = '\0';
				cfound = 1;
				if (ofound) {free (ipo); ofound = 0;}
			break;
			case 'o':
			/* If not found, we look at <address> in the o= line */
			/* o=<username> <session id> <version> <network type> <address type> <address> */
			if (!cfound) {
				tmpchr1=strchr(tmpline,' ');
				tmpchr1=strchr(tmpchr1+1,' ');
				tmpchr1=strchr(tmpchr1+1,' ');
				tmpchr1=strchr(tmpchr1+1,' ');
				tmpchr1=strchr(tmpchr1+1,' ');
				tmpchr2=strchr(tmpchr1+1,'\r');
				ipo = (char*)malloc(tmpchr2-tmpchr1);
				if (!ipo) return -1;
				strncpy (ipo,tmpchr2-tmpchr1,tmpchr1+1,tmpchr2-tmpchr1-1);
				ipo[tmpchr2-tmpchr1-1] = '\0';
				ofound = 1;
			}
			break;
			case 'm':
			/* Then we check the <port> in the m=audio line */
			/* m=audio <port> <transport> <fmt list>*/
			if ( strncmp (tmpline, "m=audio", 7 ) == 0) {
				tmpchr1=strchr(tmpline,' ');
				tmpchr2=strchr(tmpchr1+1,' ');
				port = (char*)malloc(tmpchr2-tmpchr1);
				if (!port) return -1;
				strncpy (port,tmpchr2-tmpchr1,tmpchr1+1,tmpchr2-tmpchr1-1);
				port[tmpchr2-tmpchr1-1] = '\0';
				mfound = 1;
			}
			break;
			case 'a':
			/* Is there any Crypto key ? a=evrb_key:<key>*/
			if ((crypt_key != NULL) && (strncmp (tmpline, EVRB_KEY, strlen(EVRB_KEY) ) == 0)) {
				tmpchr1=strchr(tmpline,':');
				tmpchr2=strchr(tmpchr1+1,'\r');
				key = (char*)malloc(tmpchr2-tmpchr1);
				if (!key) return -1;
				strncpy (key,tmpchr2-tmpchr1,tmpchr1+1,tmpchr2-tmpchr1-1);
				key[tmpchr2-tmpchr1-1] = '\0';
				afound = 1;
			}
			break;
			default:
			/* Other line type ... */
			break;
		}
		tmpline = strtok(NULL,"\n");
	}
	if (cfound) {*rtp_ip = ipc;}
	else if (ofound) {*rtp_ip = ipo;}
	else {
		if (afound) free (key);
		if (mfound) free (port);
		return -2; /* No IP found ! */
	}
	if (mfound) {*rtp_port = (unsigned short)atoi (port);}
	else {
		if (cfound) free (ipc);
		if (ofound) free (ipo);
		if (afound) free (key);
		return -2; /* No Port found */
	}
	if ((crypt_key != NULL) && (afound)) {
		*crypt_key = key;
	}
	return 0;

}

int sdp_create(char **buf, size_t* len, char *crypt_key)
{
  char *ptr;

  if (!crypt_key)
    return -1;
  *len = strlen(crypt_key); //+ strlen(EVRB_KEY) + 2;
  ptr = malloc(*len+1);
  if (ptr == NULL)
    return -1;
  *ptr = 0;
  //strcat(ptr, EVRB_KEY);
  strcat(ptr, crypt_key);
  //strcat(ptr, "\r\n");
  *buf = ptr;
  return 0;
}


int sdp_modify(char *buf, size_t len, char **newbuf, size_t* newlen, char *crypt_key)
{
	char 	*tmpchr1 = NULL;
	char 	*tmpchr2 = NULL;
	char 	*locbuf = NULL;
	size_t 	loclen = 0;

	if (!crypt_key) return -1;
	/* We add a a=evrb_key:<key> line after the s=<session name> line */
	loclen = len + strlen(crypt_key) + strlen(EVRB_KEY) +2 ;
	locbuf = (char*)malloc (loclen + 1);
	if (!locbuf) return -1;
	memcpy ( locbuf, buf, len );
	locbuf[len] = '\0';
	tmpchr1 = strstr (locbuf, "s=");
	if (!tmpchr1) {
		free(locbuf);
		return -1;
	}
	tmpchr1 = strstr (tmpchr1, "\r\n");
	if (!tmpchr1) {
		free(locbuf);
		return -1;
	}
	tmpchr1 = tmpchr1 +2;
	tmpchr2 = tmpchr1;
	memcpy ( tmpchr1, EVRB_KEY, strlen(EVRB_KEY));
	tmpchr1 = tmpchr1 + strlen(EVRB_KEY);
	memcpy ( tmpchr1, crypt_key, strlen(crypt_key) );
	tmpchr1 = tmpchr1 + strlen(crypt_key);
	memcpy ( tmpchr1, "\r\n", 2 );
	tmpchr1 = tmpchr1 +2;
	memcpy ( tmpchr1, buf + (tmpchr2 - locbuf), len - (tmpchr2 - locbuf) );
	locbuf[loclen] = '\0';
	*newbuf = locbuf;
	*newlen = loclen;
	return 0;
}
