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

#include <stdio.h>
#include <session.h>
#include <crypto.h>
#include <srtp.h>
#include <string.h>

#include <dh.h>

#define EVRB_KEY_SIZE SRTP_MASTER_KEY_LEN

typedef struct evrb_crypto_t {
	srtp_t srtp_session;
	char* key;
	size_t key_len;
	DH *dh;
} EVRB_CRYPTO;

// read-only template
const srtp_policy_t default_policy = {
	{ssrc_undefined,  0},
	{AES_128_ICM, EVRB_KEY_SIZE, NULL_AUTH, 0, 0, sec_serv_conf},
	{AES_128_ICM, EVRB_KEY_SIZE, NULL_AUTH, 0, 0, sec_serv_conf},
	NULL,
	NULL
};

int evrb_crypto_init()
{
#ifdef USE_SRTP
	int		i=0;
	if(i=srtp_init()) {
		fprintf(stderr,"srtp_init() return %i\n",i);
		return -1;
	}
#endif
	dh_init();
	return 0;
}

int
evrb_encrypt(void* local_ctx, void* data, size_t* len)
{
#ifdef USE_SRTP
	int i;
	if (local_ctx == NULL || ((EVRB_CRYPTO*)local_ctx)->srtp_session == NULL)
	  return -1;
	if(i=srtp_protect(((EVRB_CRYPTO*)local_ctx)->srtp_session, data, (int*)len)) {
#ifndef NDEBUG	
		fprintf(stdout,"-srtp_protect-%i--\n",i);fflush(stdout);
#endif
		return -1;
	}
#endif
	return 0;
}

int
evrb_decrypt(void* distant_ctx, void* data, size_t* len)
{
#ifdef USE_SRTP
	if (distant_ctx == NULL || ((EVRB_CRYPTO*)distant_ctx)->srtp_session == NULL)
	  return -1;
	if(srtp_unprotect(((EVRB_CRYPTO*)distant_ctx)->srtp_session, data, (int*)len))
		return -1;
#endif
	return 0;
}

char*
evrb_cryptokey_get(void* ctx)
{
	if(!ctx)
		return NULL;
	return ((EVRB_CRYPTO*)ctx)->key;
}



int
evrb_cryptokey_set_gen(void** ctx)
{
	EVRB_CRYPTO* crypto;

	if(!ctx)
		return -1;

	crypto = (EVRB_CRYPTO*)calloc(1, sizeof(EVRB_CRYPTO));
	if(!crypto)
		return -1;

	crypto->dh = dh_create_default();
	crypto->key = dh_generate_key(crypto->dh);
	crypto->key_len = strlen(crypto->key)/2;

	*((EVRB_CRYPTO**)ctx) = crypto;
	return 0;
}

int
evrb_cryptokey_set(char *key, void** ctx)
{
	EVRB_CRYPTO* crypto;

	if(!ctx)
		return -1;

	crypto = (EVRB_CRYPTO*)calloc(1, sizeof(EVRB_CRYPTO));
	if(!crypto)
		return -1;

	crypto->dh = NULL;
	crypto->key = strdup(key);
	crypto->key_len = strlen(crypto->key)/2;

	*((EVRB_CRYPTO**)ctx) = crypto;
	return 0;
}


void evrb_crypto_free(void* ctx)
{
	EVRB_CRYPTO* evb_crypto=ctx;
	if(evb_crypto) {
		srtp_dealloc(evb_crypto->srtp_session);
		if (evb_crypto->key) free(evb_crypto->key);
		evb_crypto->key=0;
		if (evb_crypto->dh)
		  dh_free(evb_crypto->dh);
		free(ctx);
		ctx=0;
	}
}

void evrb_crypto_clean_exchanged_key(void* ctx)
{
	EVRB_CRYPTO* evb_crypto=ctx;
	if(evb_crypto) {
		if (evb_crypto->key) free(evb_crypto->key);
		evb_crypto->key=0;
	}
}

int
evrb_crypto_keys_compute(void* local_ctx, void* distant_ctx, int caller)
{
	srtp_policy_t* loc_policy;
	srtp_policy_t* dist_policy;
	EVRB_CRYPTO* loc = (EVRB_CRYPTO*)local_ctx;
	EVRB_CRYPTO* dist = (EVRB_CRYPTO*)distant_ctx;
	unsigned char *master_key;
	int	master_key_len;

	if(!local_ctx || !distant_ctx)
		return -1;

	loc_policy = (srtp_policy_t*)malloc(sizeof(srtp_policy_t));
	dist_policy = (srtp_policy_t*)malloc(sizeof(srtp_policy_t));
	if(!loc_policy || !dist_policy)
		return -1;

	memcpy(loc_policy, &default_policy, sizeof(srtp_policy_t));
	memcpy(dist_policy, &default_policy, sizeof(srtp_policy_t));
	loc_policy->ssrc.type = ssrc_any_outbound;
	dist_policy->ssrc.type = ssrc_any_inbound;

	if (loc->dh == NULL)
	  return -1;

	master_key = dh_compute_key(loc->dh, dist->key, &master_key_len);

#ifndef NDEBUG
	{
	  int i;
	printf("key1=%s\nkey2=%s\n", loc->key, dist->key);
	printf("master_key(%d)=", master_key_len);
	for (i = 0; i < master_key_len; i++)
	  printf("%02x", master_key[i]);
	printf("\n");
	}
#endif
	loc_policy->key = (uint8_t*)malloc(EVRB_KEY_SIZE);
	dist_policy->key = (uint8_t*)malloc(EVRB_KEY_SIZE);

	// TODO: better key derivation
	/* Simmetric keys */
	if (caller)
	  {
	    memcpy(loc_policy->key, master_key, EVRB_KEY_SIZE);
	    memcpy(dist_policy->key, master_key+EVRB_KEY_SIZE, EVRB_KEY_SIZE);
	  }
	else
	  {
	    memcpy(loc_policy->key, master_key+EVRB_KEY_SIZE, EVRB_KEY_SIZE);
	    memcpy(dist_policy->key, master_key, EVRB_KEY_SIZE);
	  }

	if(srtp_create(&loc->srtp_session, loc_policy))
		return -1;

	if(srtp_create(&dist->srtp_session, dist_policy))
		return -1;

	return 0;
}
