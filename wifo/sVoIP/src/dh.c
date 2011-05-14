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
#include <stdlib.h>
#include <string.h>

#include <openssl/crypto.h>
#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/rand.h>
#include <openssl/dh.h>
#include <openssl/err.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wincrypt.h>
#endif

#define _DEBUG_

void	dh_init()
{
  FILE *f;
  char	rnd_seed[128];

#ifdef _DEBUG_
  CRYPTO_malloc_debug_init();
  CRYPTO_dbg_set_options(V_CRYPTO_MDEBUG_ALL);
  CRYPTO_mem_ctrl(CRYPTO_MEM_CHECK_ON);
#endif

#ifdef WIN32
  CRYPTO_malloc_init();
#endif

  /* Get enough entropy */
#ifndef WIN32
  f = fopen("/dev/urandom", "r");
  fread(rnd_seed, 1, 128, f);
  fclose(f);
#else
  {
    HCRYPTPROV hProv=0;
    CryptAcquireContext(&hProv, NULL, NULL, PROV_DH_SCHANNEL, CRYPT_VERIFYCONTEXT);
    CryptGenRandom(hProv, 128, rnd_seed);
    CryptReleaseContext(hProv,0);
  }
#endif

  RAND_seed(rnd_seed, 128);
}

DH *dh_create_default()
{
  /* Use Oakley 5 by default (192 bits large) */
  DH *dh;
  const char *P =
    "FFFFFFFF" "FFFFFFFF" "C90FDAA2" "2168C234" "C4C6628B" "80DC1CD1"
    "29024E08" "8A67CC74" "020BBEA6" "3B139B22" "514A0879" "8E3404DD"
    "EF9519B3" "CD3A431B" "302B0A6D" "F25F1437" "4FE1356D" "6D51C245"
    "E485B576" "625E7EC6" "F44C42E9" "A637ED6B" "0BFF5CB6" "F406B7ED"
    "EE386BFB" "5A899FA5" "AE9F2411" "7C4B1FE6" "49286651" "ECE65381"
    "FFFFFFFF" "FFFFFFFF";
  const char *G = "2";
  const char *Q =
    "7FFFFFFF" "FFFFFFFF" "E487ED51" "10B4611A" "62633145" "C06E0E68"
    "94812704" "4533E63A" "0105DF53" "1D89CD91" "28A5043C" "C71A026E"
    "F7CA8CD9" "E69D218D" "98158536" "F92F8A1B" "A7F09AB6" "B6A8E122"
    "F242DABB" "312F3F63" "7A262174" "D31BF6B5" "85FFAE5B" "7A035BF6"
    "F71C35FD" "AD44CFD2" "D74F9208" "BE258FF3" "24943328" "F67329C0"
    "FFFFFFFF" "FFFFFFFF";

  dh = DH_new();
  if (dh == NULL) 
    {
      return NULL;
    }
  if (!BN_hex2bn(&dh->p, P)) 
    {
      DH_free(dh);
      return NULL;
    }
  if (!BN_hex2bn(&dh->g, G)) 
    {
      DH_free(dh);
      return NULL;
    }
  if (!BN_hex2bn(&dh->q, Q)) 
    {
      DH_free(dh);
      return NULL;
    }
  return dh;
}

DH *dh_create()
{
  DH	*a;

  a = DH_generate_parameters(64, DH_GENERATOR_5, NULL, NULL);
  if (a == NULL)
    return NULL;

#ifdef DH_CHECK
  if (!DH_check(a, &i)) 
    printf("DH: couldn't check\n");
  else
    {
      if (i & DH_CHECK_P_NOT_PRIME)
	printf("DH: p value is not prime\n");
      if (i & DH_CHECK_P_NOT_SAFE_PRIME)
	printf("DH: p value is not a safe prime\n");
      if (i & DH_UNABLE_TO_CHECK_GENERATOR)
	printf("DH: unable to check the generator value\n");
      if (i & DH_NOT_SUITABLE_GENERATOR)
	printf("DH: the g value is not a generator\n");
      DH_free(a);
      return NULL;
    }
#endif

  return a;
}

char *dh_generate_key(DH *dh)
{
  BIGNUM	*bn;
  char		*buf;
  char		*tmp;

  if (!DH_generate_key(dh)) 
    return NULL;

  bn = dh->pub_key;
  tmp = BN_bn2hex(bn);
  buf = strdup(tmp);
  OPENSSL_free(tmp);

  return buf;
}

unsigned char *dh_compute_key(DH *dh, char *pub_key, int *len)
{
  unsigned char	*buf;
  int		dh_len;
  BIGNUM	*bn = NULL;

  dh_len = DH_size(dh);
  buf = malloc(dh_len);
  if (buf == NULL)
    return NULL;

  BN_hex2bn(&bn, pub_key);

  if (!DH_compute_key(buf, bn, dh))
    {
      free(buf);
      return NULL;
    }

  *len = dh_len;
  return buf;
}

void dh_free(DH *dh)
{
  DH_free(dh);
}

#if 0
int main(int argc, char *argv[])
{
  DH *a;
  DH *b=NULL;
  char buf[12];
  unsigned char *abuf=NULL,*bbuf=NULL;
  int i,alen,blen,aout,bout,ret=1;
  BIO *out;
  char	*akey, *bkey;

  dh_init();
  
  out=BIO_new(BIO_s_file());
  if (out == NULL) exit(1);
  BIO_set_fp(out,stdout,BIO_NOCLOSE);
	
  a = dh_create_default();
  
  if (!DH_check(a, &i)) goto err;
  if (i & DH_CHECK_P_NOT_PRIME)
    BIO_puts(out, "p value is not prime\n");
  if (i & DH_CHECK_P_NOT_SAFE_PRIME)
    BIO_puts(out, "p value is not a safe prime\n");
  if (i & DH_UNABLE_TO_CHECK_GENERATOR)
    BIO_puts(out, "unable to check the generator value\n");
  if (i & DH_NOT_SUITABLE_GENERATOR)
    BIO_puts(out, "the g value is not a generator\n");
  
  BIO_puts(out,"\np    =");
  BN_print(out,a->p);
  BIO_puts(out,"\ng    =");
  BN_print(out,a->g);
  BIO_puts(out,"\n");
  
  b= dh_create_default();
  if (b == NULL) goto err;
  
  akey = dh_generate_key(a);
  
  printf("akey=%s\n", akey);
  
  bkey = dh_generate_key(b);
  
  printf("bkey=%s\n", bkey);
  
  
  abuf = dh_compute_key(a, bkey, &aout);

  BIO_puts(out,"key1 =");
  for (i=0; i<aout; i++)
    {
      sprintf(buf,"%02X",abuf[i]);
      BIO_puts(out,buf);
    }
  BIO_puts(out,"\n");
  
  bbuf = dh_compute_key(b, akey, &bout);
  
  BIO_puts(out,"key2 =");
  for (i=0; i<bout; i++)
    {
      sprintf(buf,"%02X",bbuf[i]);
      BIO_puts(out,buf);
    }
  BIO_puts(out,"\n");
  
  printf("len = %d %d\n", aout, bout);

  if ((aout < 4) || (bout != aout) || (memcmp(abuf,bbuf,aout) != 0))
    {
      fprintf(stderr,"Error in DH routines\n");
      ret=1;
    }
  else
    ret=0;
err:
  ERR_print_errors_fp(stderr);
  
  if(b != NULL) DH_free(b);
  if(a != NULL) DH_free(a);
  BIO_free(out);
  CRYPTO_cleanup_all_ex_data();
  ERR_remove_state(0);
  CRYPTO_mem_leaks_fp(stderr);
  exit(ret);
  return(ret);
}
#endif
