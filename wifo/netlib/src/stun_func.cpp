/* ====================================================================
 * The Vovida Software License, Version 1.0 
 * 
 * Copyright (c) 2000 Vovida Networks, Inc.  All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 
 * 3. The names "VOCAL", "Vovida Open Communication Application Library",
 *    and "Vovida Open Communication Application Library (VOCAL)" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact vocal@vovida.org.
 *
 * 4. Products derived from this software may not be called "VOCAL", nor
 *    may "VOCAL" appear in their name, without prior written
 *    permission of Vovida Networks, Inc.
 * 
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE AND
 * NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL VOVIDA
 * NETWORKS, INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT DAMAGES
 * IN EXCESS OF $1,000, NOR FOR ANY INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 * 
 * ====================================================================
 * 
 * This software consists of voluntary contributions made by Vovida
 * Networks, Inc. and many individuals on behalf of Vovida Networks,
 * Inc.  For more information on Vovida Networks, Inc., please see
 * <http://www.vovida.org/>.
 *
 */

#include <cassert>
#include <cstring>
#include <iostream>
#include <cstdlib>
#include <errno.h>

#ifdef WIN32
#include <winsock2.h>
#include <stdlib.h>
#include <io.h>
#include <time.h>
#else
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <net/if.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#if defined(__sparc__) || defined(WIN32)
#define NOSSL
#endif
#define NOSSL

#include "udp_func.h"
#include "stun_func.h"

//#define DEBUG_STUN
#ifdef DEBUG_STUN
#define LOG(x) clog << __FILE__ << ":" << __LINE__ << ": " << x
#else
#define LOG(x)
#endif

using namespace std;

static void
computeHmac(char* hmac, const char* input, int length, const char* key, int keySize);

// A GARDER
static bool
stunParseAtrAddress( char* body, unsigned int hdrLen,  StunAtrAddress4& result )
{
   if ( hdrLen != 8 )
   {
      LOG("hdrLen wrong for Address" <<endl);
      return false;
   }
   result.pad = *body++;
   result.family = *body++;
   if (result.family == IPv4Family)
   {
      UInt16 nport;
      memcpy(&nport, body, 2); body+=2;
      result.ipv4.port = ntohs(nport);

      UInt32 naddr;
      memcpy(&naddr, body, 4); body+=4;
      result.ipv4.addr = ntohl(naddr);
      return true;
   }
   else if (result.family == IPv6Family)
   {
      LOG("ipv6 not supported" << endl);
   }
   else
   {
      LOG("bad address family: " << result.family << endl);
   }

   return false;
}

static bool
stunParseAtrChangeRequest( char* body, unsigned int hdrLen,  StunAtrChangeRequest& result )
{
   if ( hdrLen != 4 )
   {
      LOG("hdr length = " << hdrLen << " expecting " << sizeof(result) << endl);

      LOG("Incorrect size for ChangeRequest" << endl);
      return false;
   }
   else
   {
      memcpy(&result.value, body, 4);
      result.value = ntohl(result.value);
      return true;
   }
}

static bool
stunParseAtrError( char* body, unsigned int hdrLen,  StunAtrError& result )
{
   if ( hdrLen >= sizeof(result) )
   {
      LOG("head on Error too large" << endl);
      return false;
   }
   else
   {
      memcpy(&result.pad, body, 2); body+=2;
      result.pad = ntohs(result.pad);
      result.errorClass = *body++;
      result.number = *body++;

      result.sizeReason = hdrLen - 4;
      memcpy(&result.reason, body, result.sizeReason);
      result.reason[result.sizeReason] = 0;
      return true;
   }
}

static bool
stunParseAtrUnknown( char* body, unsigned int hdrLen,  StunAtrUnknown& result )
{
   if ( hdrLen >= sizeof(result) )
   {
      return false;
   }
   else
   {
      if (hdrLen % 4 != 0) return false;
      result.numAttributes = hdrLen / 4;
      for (int i=0; i<result.numAttributes; i++)
      {
         memcpy(&result.attrType[i], body, 2); body+=2;
         result.attrType[i] = ntohs(result.attrType[i]);
      }
      return true;
   }
}

// A GARDER
static bool
stunParseAtrString( char* body, unsigned int hdrLen,  StunAtrString& result )
{
   if ( hdrLen >= STUN_MAX_STRING )
   {
      LOG("String is too large" << endl);
      return false;
   }
   else
   {
      if (hdrLen % 4 != 0)
      {
         LOG("Bad length string " << hdrLen << endl);
         return false;
      }

      result.sizeValue = hdrLen;
      memcpy(&result.value, body, hdrLen);
      result.value[hdrLen] = 0;
      return true;
   }
}


static bool
stunParseAtrIntegrity( char* body, unsigned int hdrLen,  StunAtrIntegrity& result )
{
   if ( hdrLen != 20)
   {
      LOG("MessageIntegrity must be 20 bytes" << endl);
      return false;
   }
   else
   {
      memcpy(&result.hash, body, hdrLen);
      return true;
   }
}


bool
stunParseMessage( char* buf, unsigned int bufLen, StunMessage& msg, bool verbose)
{
   if (verbose) LOG("Received stun message: " << bufLen << " bytes" << endl);
   memset(&msg, 0, sizeof(msg));

   if (sizeof(StunMsgHdr) > bufLen)
   {
      LOG("Bad message" << endl);
      return false;
   }

   memcpy(&msg.msgHdr, buf, sizeof(StunMsgHdr));
   msg.msgHdr.msgType = ntohs(msg.msgHdr.msgType);
   msg.msgHdr.msgLength = ntohs(msg.msgHdr.msgLength);

   if (msg.msgHdr.msgLength + sizeof(StunMsgHdr) != bufLen)
   {
      LOG("Message header length doesn't match message size: " << msg.msgHdr.msgLength << " - " << bufLen << endl);
      return false;
   }

   char* body = buf + sizeof(StunMsgHdr);
   unsigned int size = msg.msgHdr.msgLength;

   //LOG("bytes after header = " << size << endl);

   while ( size > 0 )
   {
      // !jf! should check that there are enough bytes left in the buffer

      StunAtrHdr* attr = reinterpret_cast<StunAtrHdr*>(body);

      unsigned int attrLen = ntohs(attr->length);
      int atrType = ntohs(attr->type);

      //if (verbose) LOG("Found attribute type=" << AttrNames[atrType] << " length=" << attrLen << endl);
      if ( attrLen+4 > size )
      {
         LOG("claims attribute is larger than size of message " <<"(attribute type="<<atrType<<")"<< endl);
         return false;
      }

      body += 4; // skip the length and type in attribute header
      size -= 4;

      switch ( atrType )
      {
         case MappedAddress:
            msg.hasMappedAddress = true;
            if ( stunParseAtrAddress(  body,  attrLen,  msg.mappedAddress )== false )
            {
               LOG("problem parsing MappedAddress" << endl);
               return false;
            }
            else
            {
               if (verbose) LOG("MappedAddress = " << msg.mappedAddress.ipv4 << endl);
            }

            break;

         case ResponseAddress:
            msg.hasResponseAddress = true;
            if ( stunParseAtrAddress(  body,  attrLen,  msg.responseAddress )== false )
            {
               LOG("problem parsing ResponseAddress" << endl);
               return false;
            }
            else
            {
               if (verbose) LOG("ResponseAddress = " << msg.responseAddress.ipv4 << endl);
            }
            break;

         case ChangeRequest:
            msg.hasChangeRequest = true;
            if (stunParseAtrChangeRequest( body, attrLen, msg.changeRequest) == false)
            {
               LOG("problem parsing ChangeRequest" << endl);
               return false;
            }
            else
            {
               if (verbose) LOG("ChangeRequest = " << msg.changeRequest.value << endl);
            }
            break;

         case SourceAddress:
            msg.hasSourceAddress = true;
            if ( stunParseAtrAddress(  body,  attrLen,  msg.sourceAddress )== false )
            {
               LOG("problem parsing SourceAddress" << endl);
               return false;
            }
            else
            {
               if (verbose) LOG("SourceAddress = " << msg.sourceAddress.ipv4 << endl);
            }
            break;

         case ChangedAddress:
            msg.hasChangedAddress = true;
            if ( stunParseAtrAddress(  body,  attrLen,  msg.changedAddress )== false )
            {
               LOG("problem parsing ChangedAddress" << endl);
               return false;
            }
            else
            {
               if (verbose) LOG("ChangedAddress = " << msg.changedAddress.ipv4 << endl);
            }
            break;

         case Username:
            msg.hasUsername = true;
            if (stunParseAtrString( body, attrLen, msg.username) == false)
            {
               LOG("problem parsing Username" << endl);
               return false;
            }
            else
            {
               if (verbose) LOG("Username = " << msg.username.value << endl);
            }

            break;

         case Password:
            msg.hasPassword = true;
            if (stunParseAtrString( body, attrLen, msg.password) == false)
            {
               LOG("problem parsing Password" << endl);
               return false;
            }
            else
            {
               if (verbose) LOG("Password = " << msg.password.value << endl);
            }
            break;

         case MessageIntegrity:
            msg.hasMessageIntegrity = true;
            if (stunParseAtrIntegrity( body, attrLen, msg.messageIntegrity) == false)
            {
               LOG("problem parsing MessageIntegrity" << endl);
               return false;
            }
            else
            {
               //if (verbose) LOG("MessageIntegrity = " << msg.messageIntegrity.hash << endl);
            }

            // read the current HMAC
            // look up the password given the user of given the transaction id
            // compute the HMAC on the buffer
            // decide if they match or not
            break;

         case ErrorCode:
            msg.hasErrorCode = true;
            if (stunParseAtrError(body, attrLen, msg.errorCode) == false)
            {
               LOG("problem parsing ErrorCode" << endl);
               return false;
            }
            else
            {
               if (verbose) clog << "ErrorCode = " << int(msg.errorCode.errorClass)
                                 << " " << int(msg.errorCode.number)
                                 << " " << msg.errorCode.reason << endl;
            }

            break;

         case UnknownAttribute:
            msg.hasUnknownAttributes = true;
            if (stunParseAtrUnknown(body, attrLen, msg.unknownAttributes) == false)
            {
               LOG("problem parsing UnknownAttribute" << endl);
               return false;
            }
            break;

         case ReflectedFrom:
            msg.hasReflectedFrom = true;
            if ( stunParseAtrAddress(  body,  attrLen,  msg.reflectedFrom ) == false )
            {
               LOG("problem parsing ReflectedFrom" << endl);
               return false;
            }
            break;

         case XorMappedAddress:
            msg.hasXorMappedAddress = true;
            if ( stunParseAtrAddress(  body,  attrLen,  msg.xorMappedAddress ) == false )
            {
               LOG("problem parsing XorMappedAddress" << endl);
               return false;
            }
            else
            {
               if (verbose) LOG("XorMappedAddress = " << msg.mappedAddress.ipv4 << endl);
            }
            break;

         case XorOnly:
            msg.xorOnly = true;
            if (verbose)
            {
               LOG("xorOnly = true" << endl);
            }
            break;

         case ServerName:
            msg.hasServerName = true;
            if (stunParseAtrString( body, attrLen, msg.serverName) == false)
            {
               LOG("problem parsing ServerName" << endl);
               return false;
            }
            else
            {
               if (verbose) LOG("ServerName = " << msg.serverName.value << endl);
            }
            break;

         case SecondaryAddress:
            msg.hasSecondaryAddress = true;
            if ( stunParseAtrAddress(  body,  attrLen,  msg.secondaryAddress ) == false )
            {
               LOG("problem parsing secondaryAddress" << endl);
               return false;
            }
            else
            {
               if (verbose) LOG("SecondaryAddress = " << msg.secondaryAddress.ipv4 << endl);
            }
            break;

         default:
            if (verbose) LOG("Unknown attribute: " << atrType << endl);
            if ( atrType <= 0x7FFF )
            {
               return false;
            }
      }

      body += attrLen;
      size -= attrLen;
   }

   return true;
}


static char*
encode16(char* buf, UInt16 data)
{
   UInt16 ndata = htons(data);
   memcpy(buf, reinterpret_cast<void*>(&ndata), sizeof(UInt16));
   return buf + sizeof(UInt16);
}

static char*
encode32(char* buf, UInt32 data)
{
   UInt32 ndata = htonl(data);
   memcpy(buf, reinterpret_cast<void*>(&ndata), sizeof(UInt32));
   return buf + sizeof(UInt32);
}


static char*
encode(char* buf, const char* data, unsigned int length)
{
   memcpy(buf, data, length);
   return buf + length;
}


static char*
encodeAtrAddress4(char* ptr, UInt16 type, const StunAtrAddress4& atr)
{
   ptr = encode16(ptr, type);
   ptr = encode16(ptr, 8);
   *ptr++ = atr.pad;
   *ptr++ = IPv4Family;
   ptr = encode16(ptr, atr.ipv4.port);
   ptr = encode32(ptr, atr.ipv4.addr);

   return ptr;
}

static char*
encodeAtrChangeRequest(char* ptr, const StunAtrChangeRequest& atr)
{
   ptr = encode16(ptr, ChangeRequest);
   ptr = encode16(ptr, 4);
   ptr = encode32(ptr, atr.value);
   return ptr;
}

static char*
encodeAtrError(char* ptr, const StunAtrError& atr)
{
   ptr = encode16(ptr, ErrorCode);
   ptr = encode16(ptr, 6 + atr.sizeReason);
   ptr = encode16(ptr, atr.pad);
   *ptr++ = atr.errorClass;
   *ptr++ = atr.number;
   ptr = encode(ptr, atr.reason, atr.sizeReason);
   return ptr;
}


static char*
encodeAtrUnknown(char* ptr, const StunAtrUnknown& atr)
{
   ptr = encode16(ptr, UnknownAttribute);
   ptr = encode16(ptr, 2+2*atr.numAttributes);
   for (int i=0; i<atr.numAttributes; i++)
   {
      ptr = encode16(ptr, atr.attrType[i]);
   }
   return ptr;
}


static char*
encodeXorOnly(char* ptr)
{
   ptr = encode16(ptr, XorOnly );
   return ptr;
}


static char*
encodeAtrString(char* ptr, UInt16 type, const StunAtrString& atr)
{
   assert(atr.sizeValue % 4 == 0);

   ptr = encode16(ptr, type);
   ptr = encode16(ptr, atr.sizeValue);
   ptr = encode(ptr, atr.value, atr.sizeValue);
   return ptr;
}


static char*
encodeAtrIntegrity(char* ptr, const StunAtrIntegrity& atr)
{
   ptr = encode16(ptr, MessageIntegrity);
   ptr = encode16(ptr, 20);
   ptr = encode(ptr, atr.hash, sizeof(atr.hash));
   return ptr;
}


unsigned int
stunEncodeMessage( const StunMessage& msg,
                   char* buf,
                   unsigned int bufLen,
                   const StunAtrString& password,
                   bool verbose)
{
   assert(bufLen >= sizeof(StunMsgHdr));
   char* ptr = buf;

   ptr = encode16(ptr, msg.msgHdr.msgType);
   char* lengthp = ptr;
   ptr = encode16(ptr, 0);
   ptr = encode(ptr, reinterpret_cast<const char*>(msg.msgHdr.id.octet), sizeof(msg.msgHdr.id));

   if (verbose) LOG("Encoding stun message: " << endl);
   if (msg.hasMappedAddress)
   {
      if (verbose) LOG("Encoding MappedAddress: " << msg.mappedAddress.ipv4 << endl);
      ptr = encodeAtrAddress4 (ptr, MappedAddress, msg.mappedAddress);
   }
   if (msg.hasResponseAddress)
   {
      if (verbose) LOG("Encoding ResponseAddress: " << msg.responseAddress.ipv4 << endl);
      ptr = encodeAtrAddress4(ptr, ResponseAddress, msg.responseAddress);
   }
   if (msg.hasChangeRequest)
   {
      if (verbose) LOG("Encoding ChangeRequest: " << msg.changeRequest.value << endl);
      ptr = encodeAtrChangeRequest(ptr, msg.changeRequest);
   }
   if (msg.hasSourceAddress)
   {
      if (verbose) LOG("Encoding SourceAddress: " << msg.sourceAddress.ipv4 << endl);
      ptr = encodeAtrAddress4(ptr, SourceAddress, msg.sourceAddress);
   }
   if (msg.hasChangedAddress)
   {
      if (verbose) LOG("Encoding ChangedAddress: " << msg.changedAddress.ipv4 << endl);
      ptr = encodeAtrAddress4(ptr, ChangedAddress, msg.changedAddress);
   }
   if (msg.hasUsername)
   {
      if (verbose) LOG("Encoding Username: " << msg.username.value << endl);
      ptr = encodeAtrString(ptr, Username, msg.username);
   }
   if (msg.hasPassword)
   {
      if (verbose) LOG("Encoding Password: " << msg.password.value << endl);
      ptr = encodeAtrString(ptr, Password, msg.password);
   }
   if (msg.hasErrorCode)
   {
      if (verbose) clog << "Encoding ErrorCode: class="
			<< int(msg.errorCode.errorClass)
			<< " number=" << int(msg.errorCode.number)
			<< " reason="
			<< msg.errorCode.reason
			<< endl;

      ptr = encodeAtrError(ptr, msg.errorCode);
   }
   if (msg.hasUnknownAttributes)
   {
      if (verbose) LOG("Encoding UnknownAttribute: ???" << endl);
      ptr = encodeAtrUnknown(ptr, msg.unknownAttributes);
   }
   if (msg.hasReflectedFrom)
   {
      if (verbose) LOG("Encoding ReflectedFrom: " << msg.reflectedFrom.ipv4 << endl);
      ptr = encodeAtrAddress4(ptr, ReflectedFrom, msg.reflectedFrom);
   }
   if (msg.hasXorMappedAddress)
   {
      if (verbose) LOG("Encoding XorMappedAddress: " << msg.xorMappedAddress.ipv4 << endl);
      ptr = encodeAtrAddress4 (ptr, XorMappedAddress, msg.xorMappedAddress);
   }
   if (msg.xorOnly)
   {
      if (verbose) LOG("Encoding xorOnly: " << endl);
      ptr = encodeXorOnly( ptr );
   }
   if (msg.hasServerName)
   {
      if (verbose) LOG("Encoding ServerName: " << msg.serverName.value << endl);
      ptr = encodeAtrString(ptr, ServerName, msg.serverName);
   }
   if (msg.hasSecondaryAddress)
   {
      if (verbose) LOG("Encoding SecondaryAddress: " << msg.secondaryAddress.ipv4 << endl);
      ptr = encodeAtrAddress4 (ptr, SecondaryAddress, msg.secondaryAddress);
   }

   if (password.sizeValue > 0)
   {
      if (verbose) LOG("HMAC with password: " << password.value << endl);

      StunAtrIntegrity integrity;
      computeHmac(integrity.hash, buf, int(ptr-buf) , password.value, password.sizeValue);
      ptr = encodeAtrIntegrity(ptr, integrity);
   }
   if (verbose) LOG(endl);

   encode16(lengthp, UInt16(ptr - buf - sizeof(StunMsgHdr)));
   return int(ptr - buf);
}

int
stunRand()
{
   // return 32 bits of random stuff
   assert( sizeof(int) == 4 );
   static bool init=false;
   if ( !init )
   {
      init = true;

      UInt64 tick;

#if defined(WIN32) && defined(CC_MSVC)
      volatile unsigned int lowtick=0,hightick=0;
      __asm
         {
            rdtsc
               mov lowtick, eax
               mov hightick, edx
               }
      tick = hightick;
      tick <<= 32;
      tick |= lowtick;
#elif defined(__GNUC__) && ( defined(__i686__) || defined(__i386__) || defined(__x86_64__) )
      asm("rdtsc" : "=A" (tick));
#else
	  int fd=open("/dev/random",O_RDONLY);
	  if (fd < 0)
		  fd=open("/dev/urandom",O_RDONLY);

	  if (fd >= 0)
	  {
		  read(fd,&tick,sizeof(tick));
		  close(fd);
	  }
	  else
		  tick = (UInt64) ::time(0);
#endif
      int seed = int(tick);
#ifdef WIN32
      srand(seed);
#else
      srandom(seed);
#endif
   }

#ifdef WIN32
   assert( RAND_MAX == 0x7fff );
   int r1 = rand();
   int r2 = rand();

   int ret = (r1<<16) + r2;

   return ret;
#else
   return random();
#endif
}


/// return a random number to use as a port
static int
randomPort()
{
   int min=0x4000;
   int max=0x7FFF;

   int ret = stunRand();
   ret = ret|min;
   ret = ret&max;

   return ret;
}


#ifdef NOSSL
static void
computeHmac(char* hmac, const char* input, int length, const char* key, int sizeKey)
{
   strncpy(hmac,"hmac-not-implemented",20);
}
#else
#include <openssl/hmac.h>

static void
computeHmac(char* hmac, const char* input, int length, const char* key, int sizeKey)
{
   unsigned int resultSize=0;
   HMAC(EVP_sha1(),
        key, sizeKey,
        reinterpret_cast<const unsigned char*>(input), length,
        reinterpret_cast<unsigned char*>(hmac), &resultSize);
   assert(resultSize == 20);
}
#endif


static void
toHex(const char* buffer, int bufferSize, char* output)
{
   static char hexmap[] = "0123456789abcdef";

   const char* p = buffer;
   char* r = output;
   for (int i=0; i < bufferSize; i++)
   {
      unsigned char temp = *p++;

      int hi = (temp & 0xf0)>>4;
      int low = (temp & 0xf);

      *r++ = hexmap[hi];
      *r++ = hexmap[low];
   }
   *r = 0;
}

void
stunCreateUserName(const StunAddress4& source, StunAtrString* username)
{
   UInt64 time = stunGetSystemTimeSecs();
   time -= (time % 20*60);
   //UInt64 hitime = time >> 32;
   UInt64 lotime = time & 0xFFFFFFFF;

   char buffer[1024];
   sprintf(buffer,
           "%08x:%08x:%08x:",
           UInt32(source.addr),
           UInt32(stunRand()),
           UInt32(lotime));
   assert( strlen(buffer) < 1024 );

   assert(strlen(buffer) + 41 < STUN_MAX_STRING);

   char hmac[20];
   char key[] = "Jason";
   computeHmac(hmac, buffer, strlen(buffer), key, strlen(key) );
   char hmacHex[41];
   toHex(hmac, 20, hmacHex );
   hmacHex[40] =0;

   strcat(buffer,hmacHex);

   int l = strlen(buffer);
   assert( l+1 < STUN_MAX_STRING );
   assert( l%4 == 0 );

   username->sizeValue = l;
   memcpy(username->value,buffer,l);
   username->value[l]=0;

   //if (verbose) LOG("computed username=" << username.value << endl);
}

void
stunCreatePassword(const StunAtrString& username, StunAtrString* password)
{
   char hmac[20];
   char key[] = "Fluffy";
   //char buffer[STUN_MAX_STRING];
   computeHmac(hmac, username.value, strlen(username.value), key, strlen(key));
   toHex(hmac, 20, password->value);
   password->sizeValue = 40;
   password->value[40]=0;

   //LOG("password=" << password->value << endl);
}


UInt64
stunGetSystemTimeSecs()
{
   UInt64 time=0;
#if defined(WIN32)
   SYSTEMTIME t;
   // CJ TODO - this probably has bug on wrap around every 24 hours
   GetSystemTime( &t );
   time = (t.wHour*60+t.wMinute)*60+t.wSecond;
#else
   struct timeval now;
   gettimeofday( &now , NULL );
   //assert( now );
   time = now.tv_sec;
#endif
   return time;
}


ostream& operator<< ( ostream& strm, const UInt128& r )
{
   strm << int(r.octet[0]);
   for ( int i=1; i<16; i++ )
   {
      strm << ':' << int(r.octet[i]);
   }

   return strm;
}

ostream&
operator<<( ostream& strm, const StunAddress4& addr)
{
   UInt32 ip = addr.addr;
   strm << ((int)(ip>>24)&0xFF) << ".";
   strm << ((int)(ip>>16)&0xFF) << ".";
   strm << ((int)(ip>> 8)&0xFF) << ".";
   strm << ((int)(ip>> 0)&0xFF) ;

   strm << ":" << addr.port;

   return strm;
}


// returns true if it scucceeded
bool
stunParseHostName( char* peerName,
               UInt32& ip,
               UInt16& portVal,
               UInt16 defaultPort )
{
   in_addr sin_addr;

   char host[512];
   strncpy(host,peerName,512);
   host[512-1]='\0';
   char* port = NULL;

   int portNum = defaultPort;

   // pull out the port part if present.
   char* sep = strchr(host,':');

   if ( sep == NULL )
   {
      portNum = defaultPort;
   }
   else
   {
      *sep = '\0';
      port = sep + 1;
      // set port part

      char* endPtr=NULL;

      portNum = strtol(port,&endPtr,10);

      if ( endPtr != NULL )
      {
         if ( *endPtr != '\0' )
         {
            portNum = defaultPort;
         }
      }
   }

   if ( portNum < 1024 ) return false;
   if ( portNum >= 0xFFFF ) return false;

   // figure out the host part
   struct hostent* h;

#ifdef WIN32
   assert( strlen(host) >= 1 );
   if ( isdigit( host[0] ) )
   {
      // assume it is a ip address
      unsigned long a = inet_addr(host);
      //LOG("a=0x" << hex << a << dec << endl);

      ip = ntohl( a );
   }
   else
   {
      // assume it is a host name
      h = gethostbyname( host );

      if ( h == NULL )
      {
         int err = getErrno();
         LOG("error was " << err << std::endl);
         assert( err != WSANOTINITIALISED );

         ip = ntohl( 0x7F000001L );

         return false;
      }
      else
      {
         sin_addr = *(struct in_addr*)h->h_addr;
         ip = ntohl( sin_addr.s_addr );
      }
   }

#else
   h = gethostbyname( host );
   if ( h == NULL )
   {
      LOG("error was " << getErrno() << std::endl);
      ip = ntohl( 0x7F000001L );
      return false;
   }
   else
   {
      sin_addr = *(struct in_addr*)h->h_addr;
      ip = ntohl( sin_addr.s_addr );
   }
#endif

   portVal = portNum;

   return true;
}

// A GARDER
bool
stunParseServerName( char* name, StunAddress4& addr, const int port)
{
   assert(name);

   // TODO - put in DNS SRV stuff.

   bool ret = stunParseHostName( name, addr.addr, addr.port, port);
   if ( ret != true )
   {
       addr.port=0xFFFF;
   }
   return ret;
}


static void
stunCreateErrorResponse(StunMessage& response, int cl, int number, const char* msg)
{
   response.msgHdr.msgType = BindErrorResponseMsg;
   response.hasErrorCode = true;
   response.errorCode.errorClass = cl;
   response.errorCode.number = number;
   strcpy(response.errorCode.reason, msg);
}

#if 0
static void
stunCreateSharedSecretErrorResponse(StunMessage& response, int cl, int number, const char* msg)
{
   response.msgHdr.msgType = SharedSecretErrorResponseMsg;
   response.hasErrorCode = true;
   response.errorCode.errorClass = cl;
   response.errorCode.number = number;
   strcpy(response.errorCode.reason, msg);
}
#endif

static void
stunCreateSharedSecretResponse(const StunMessage& request, const StunAddress4& source, StunMessage& response)
{
   response.msgHdr.msgType = SharedSecretResponseMsg;
   response.msgHdr.id = request.msgHdr.id;

   response.hasUsername = true;
   stunCreateUserName( source, &response.username);

   response.hasPassword = true;
   stunCreatePassword( response.username, &response.password);
}

// A GARDER
void
stunBuildReqSimple( StunMessage* msg,
                    const StunAtrString& username,
                    bool changePort, bool changeIp, unsigned int id )
{
   assert( msg );
   memset( msg , 0 , sizeof(*msg) );

   msg->msgHdr.msgType = BindRequestMsg;

   for ( int i=0; i<16; i=i+4 )
   {
      assert(i+3<16);
      int r = stunRand();
      msg->msgHdr.id.octet[i+0]= r>>0;
      msg->msgHdr.id.octet[i+1]= r>>8;
      msg->msgHdr.id.octet[i+2]= r>>16;
      msg->msgHdr.id.octet[i+3]= r>>24;
   }

   if ( id != 0 )
   {
      msg->msgHdr.id.octet[0] = id;
   }

   msg->hasChangeRequest = true;
   msg->changeRequest.value =(changeIp?ChangeIpFlag:0) |
      (changePort?ChangePortFlag:0);

   if ( username.sizeValue > 0 )
   {
      msg->hasUsername = true;
      msg->username = username;
   }
}

// A GARDER
static void
stunSendTest( Socket myFd, StunAddress4& dest,
              const StunAtrString& username, const StunAtrString& password,
              int testNum, bool verbose )
{
   assert( dest.addr != 0 );
   assert( dest.port != 0 );

   bool changePort=false;
   bool changeIP=false;
   bool discard=false;

   switch (testNum)
   {
      case 1:
      case 10:
      case 11:
         break;
      case 2:
         //changePort=true;
         changeIP=true;
         break;
      case 3:
         changePort=true;
         break;
      case 4:
         changeIP=true;
         break;
      case 5:
         discard=true;
         break;
      default:
         LOG("Test " << testNum <<" is unkown\n");
         assert(0);
   }

   StunMessage req;
   memset(&req, 0, sizeof(StunMessage));

   stunBuildReqSimple( &req, username,
                       changePort , changeIP ,
                       testNum );

   char buf[STUN_MAX_MESSAGE_SIZE];
   int len = STUN_MAX_MESSAGE_SIZE;

   len = stunEncodeMessage( req, buf, len, password,verbose );

   if ( verbose )
   {
      LOG("About to send msg of len " << len << " to " << dest << endl);
   }

   sendMessage( myFd, buf, len, dest.addr, dest.port, verbose );

   // add some delay so the packets don't get sent too quickly
#ifdef WIN32 // !cj! TODO - should fix this up in windows
		 clock_t now = clock();
		 assert( CLOCKS_PER_SEC == 1000 );
		 while ( clock() <= now+10 ) { };
#else
		 usleep(10*1000);
#endif

}


void
stunGetUserNameAndPassword(  const StunAddress4& dest,
                             StunAtrString* username,
                             StunAtrString* password)
{
   // !cj! This is totally bogus - need to make TLS connection to dest and get a
   // username and password to use
   stunCreateUserName(dest, username);
   stunCreatePassword(*username, password);
}


void
stunTest( StunAddress4& dest, int testNum, bool verbose, StunAddress4* sAddr )
{
   assert( dest.addr != 0 );
   assert( dest.port != 0 );

   int port = randomPort();
   UInt32 interfaceIp=0;
   if (sAddr)
   {
      interfaceIp = sAddr->addr;
      if ( sAddr->port != 0 )
      {
        port = sAddr->port;
      }
   }
   Socket myFd = openPort(port,interfaceIp,verbose);

   StunAtrString username;
   StunAtrString password;

   username.sizeValue = 0;
   password.sizeValue = 0;

#ifdef USE_TLS
   stunGetUserNameAndPassword( dest, username, password );
#endif

   stunSendTest( myFd, dest, username, password, testNum, verbose );

   char msg[STUN_MAX_MESSAGE_SIZE];
   int msgLen = STUN_MAX_MESSAGE_SIZE;

   StunAddress4 from;
   getMessage( myFd,
               msg,
               &msgLen,
               &from.addr,
               &from.port,verbose );

   StunMessage resp;
   memset(&resp, 0, sizeof(StunMessage));

   if ( verbose ) LOG("Got a response" << endl);
   bool ok = stunParseMessage( msg,msgLen, resp,verbose );

   if ( verbose )
   {
      LOG("\t ok=" << ok << endl);
      LOG("\t id=" << resp.msgHdr.id << endl);
      LOG("\t mappedAddr=" << resp.mappedAddress.ipv4 << endl);
      LOG("\t changedAddr=" << resp.changedAddress.ipv4 << endl);
      LOG(endl);
   }

   if (sAddr)
   {
      sAddr->port = resp.mappedAddress.ipv4.port;
      sAddr->addr = resp.mappedAddress.ipv4.addr;
   }
}


NatType stunNatType(StunAddress4& dest,
					bool verbose,
					bool *preservePort, // if set, is return for if NAT preservers ports or not
					bool *hairpin,  // if set, is the return for if NAT will hairpin packets
					int port, // port to use for the test, 0 to choose random port
					StunAddress4* sAddr) // NIC to use
{
   assert( dest.addr != 0 );
   assert( dest.port != 0 );

   if ( hairpin )
   {
      *hairpin = false;
   }

   if ( port == 0 )
   {
      port = randomPort();
   }
   UInt32 interfaceIp=0;
   if (sAddr)
   {
      interfaceIp = sAddr->addr;
   }
   Socket myFd1 = openPort(port,interfaceIp,verbose);
   Socket myFd2 = openPort(port+1,interfaceIp,verbose);

   if ( ( myFd1 == INVALID_SOCKET) || ( myFd2 == INVALID_SOCKET) )
   {
        LOG("Some problem opening port/interface to send on" << endl);
       return StunTypeFailure;
   }

   assert( myFd1 != INVALID_SOCKET );
   assert( myFd2 != INVALID_SOCKET );

   bool respTestI=false;
   bool isNat=true;
   StunAddress4 testIchangedAddr;
   StunAddress4 testImappedAddr;
   bool respTestI2=false;
   bool mappedIpSame = true;
   StunAddress4 testI2mappedAddr;
   StunAddress4 testI2dest=dest;
   bool respTestII=false;
   bool respTestIII=false;

   bool respTestHairpin=false;

   memset(&testImappedAddr,0,sizeof(testImappedAddr));

   StunAtrString username;
   StunAtrString password;

   username.sizeValue = 0;
   password.sizeValue = 0;

#ifdef USE_TLS
   stunGetUserNameAndPassword( dest, username, password );
#endif

   //stunSendTest( myFd1, dest, username, password, 1, verbose );
   int count=0;
   while ( count < 7 )
   {
      struct timeval tv;
      fd_set fdSet;
#ifdef WIN32
      unsigned int fdSetSize;
#else
      int fdSetSize;
#endif
      FD_ZERO(&fdSet); fdSetSize=0;
      FD_SET(myFd1,&fdSet); fdSetSize = (myFd1+1>fdSetSize) ? myFd1+1 : fdSetSize;
      FD_SET(myFd2,&fdSet); fdSetSize = (myFd2+1>fdSetSize) ? myFd2+1 : fdSetSize;
      tv.tv_sec=0;
      tv.tv_usec=150*1000; // 150 ms
      if ( count == 0 ) tv.tv_usec=0;

      int err = select(fdSetSize, &fdSet, NULL, NULL, &tv);
      if ( err == SOCKET_ERROR )
      {
        // error occured
        int e = getErrno();
        LOG("Error " << e << " " << strerror(e) << " in select" << endl);
        return StunTypeFailure;
      }
      else if ( err == 0 )
      {
         // timeout occured
         count++;

         if ( !respTestI )
         {
            stunSendTest( myFd1, dest, username, password, 1 ,verbose );
         }

         if ( (!respTestI2) && respTestI )
         {
            // check the address to send to if valid
            if (  ( testI2dest.addr != 0 ) &&
                  ( testI2dest.port != 0 ) )
            {
               stunSendTest( myFd1, testI2dest, username, password, 10  ,verbose);
            }
         }

         if ( !respTestII )
         {
            stunSendTest( myFd2, dest, username, password, 2 ,verbose );
         }

         if ( !respTestIII )
         {
            stunSendTest( myFd2, dest, username, password, 3 ,verbose );
         }

         if ( respTestI && (!respTestHairpin) )
         {
            if (  ( testImappedAddr.addr != 0 ) &&
                  ( testImappedAddr.port != 0 ) )
            {
               stunSendTest( myFd1, testImappedAddr, username, password, 11 ,verbose );
            }
         }
      }
      else
      {
         //if (verbose) LOG("-----------------------------------------" << endl);
         assert( err>0 );
         // data is avialbe on some fd

         for ( int i=0; i<2; i++)
         {
            Socket myFd;
            if ( i==0 )
            {
               myFd=myFd1;
            }
            else
            {
               myFd=myFd2;
            }

            if ( myFd!=INVALID_SOCKET )
            {
               if ( FD_ISSET(myFd,&fdSet) )
               {
                  char msg[STUN_MAX_MESSAGE_SIZE];
                  int msgLen = sizeof(msg);

                  StunAddress4 from;

                  getMessage( myFd,
                              msg,
                              &msgLen,
                              &from.addr,
                              &from.port,verbose );

                  StunMessage resp;
                  memset(&resp, 0, sizeof(StunMessage));

                  stunParseMessage( msg,msgLen, resp,verbose );

                  if ( verbose )
                  {
                     clog << "Received message of type " << resp.msgHdr.msgType
                          << "  id=" << (int)(resp.msgHdr.id.octet[0]) << endl;
                  }

                  switch( resp.msgHdr.id.octet[0] )
                  {
                     case 1:
                     {
                        if ( !respTestI )
                        {

                           testIchangedAddr.addr = resp.changedAddress.ipv4.addr;
                           testIchangedAddr.port = resp.changedAddress.ipv4.port;
                           testImappedAddr.addr = resp.mappedAddress.ipv4.addr;
                           testImappedAddr.port = resp.mappedAddress.ipv4.port;

                           if ( preservePort )
                           {
                              *preservePort = ( testImappedAddr.port == port );
                           }

                           testI2dest.addr = resp.changedAddress.ipv4.addr;

                           if (sAddr)
                           {
                              sAddr->port = testImappedAddr.port;
                              sAddr->addr = testImappedAddr.addr;
                           }

                           count = 0;
                        }
                        respTestI=true;
                     }
                     break;
                     case 2:
                     {
                        respTestII=true;
                     }
                     break;
                     case 3:
                     {
                        respTestIII=true;
                     }
                     break;
                     case 10:
                     {
                        if ( !respTestI2 )
                        {
                           testI2mappedAddr.addr = resp.mappedAddress.ipv4.addr;
                           testI2mappedAddr.port = resp.mappedAddress.ipv4.port;

                           mappedIpSame = false;
                           if ( (testI2mappedAddr.addr  == testImappedAddr.addr ) &&
                                (testI2mappedAddr.port == testImappedAddr.port ))
                           {
                              mappedIpSame = true;
                           }


                        }
                        respTestI2=true;
                     }
                     break;
                     case 11:
                     {

                        if ( hairpin )
                        {
                           *hairpin = true;
                        }
                        respTestHairpin = true;
                     }
                     break;
                  }
               }
            }
         }
      }
   }

   // see if we can bind to this address
   //LOG("try binding to " << testImappedAddr << endl);
   Socket s = openPort( 0/*use ephemeral*/, testImappedAddr.addr, false );
   if ( s != INVALID_SOCKET )
   {
      closesocket(s);
      isNat = false;
      //LOG("binding worked" << endl);
   }
   else
   {
      isNat = true;
      //LOG("binding failed" << endl);
   }

   if (verbose)
   {
      LOG("test I = " << respTestI << endl);
      LOG("test II = " << respTestII << endl);
      LOG("test III = " << respTestIII << endl);
      LOG("test I(2) = " << respTestI2 << endl);
      LOG("is nat  = " << isNat <<endl);
      LOG("mapped IP same = " << mappedIpSame << endl);
   }

   // implement logic flow chart from draft RFC
   if ( respTestI )
   {
      if ( isNat )
      {
         if (respTestII)
         {
            return StunTypeConeNat;
         }
         else
         {
            if ( mappedIpSame )
            {
               if ( respTestIII )
               {
                  return StunTypeRestrictedNat;
               }
               else
               {
                  return StunTypePortRestrictedNat;
               }
            }
            else
            {
               return StunTypeSymNat;
            }
         }
      }
      else
      {
         if (respTestII)
         {
            return StunTypeOpen;
         }
         else
         {
            return StunTypeSymFirewall;
         }
      }
   }
   else
   {
      return StunTypeBlocked;
   }

   return StunTypeUnknown;
}


int
stunOpenSocket( StunAddress4& dest, StunAddress4* mapAddr,
                int port, StunAddress4* srcAddr,
                bool verbose )
{
   assert( dest.addr != 0 );
   assert( dest.port != 0 );
   assert( mapAddr );

   if ( port == 0 )
   {
      port = randomPort();
   }
   unsigned int interfaceIp = 0;
   if ( srcAddr )
   {
      interfaceIp = srcAddr->addr;
   }

   Socket myFd = openPort(port,interfaceIp,verbose);
   if (myFd == INVALID_SOCKET)
   {
      return myFd;
   }

   char msg[STUN_MAX_MESSAGE_SIZE];
   int msgLen = sizeof(msg);

   StunAtrString username;
   StunAtrString password;

   username.sizeValue = 0;
   password.sizeValue = 0;

#ifdef USE_TLS
   stunGetUserNameAndPassword( dest, username, password );
#endif

   stunSendTest(myFd, dest, username, password, 1, 0/*false*/ );

   StunAddress4 from;

   getMessage( myFd, msg, &msgLen, &from.addr, &from.port,verbose );

   StunMessage resp;
   memset(&resp, 0, sizeof(StunMessage));

   bool ok = stunParseMessage( msg, msgLen, resp,verbose );
   if (!ok)
   {
      return -1;
   }

   StunAddress4 mappedAddr = resp.mappedAddress.ipv4;
   StunAddress4 changedAddr = resp.changedAddress.ipv4;

   //LOG("--- stunOpenSocket --- " << endl);
   //LOG("\treq  id=" << req.id << endl);
   //LOG("\tresp id=" << id << endl);
   //LOG("\tmappedAddr=" << mappedAddr << endl);

   *mapAddr = mappedAddr;

   return myFd;
}


bool
stunOpenSocketPair( StunAddress4& dest, StunAddress4* mapAddr,
                    int* fd1, int* fd2,
                    int port, StunAddress4* srcAddr,
                    bool verbose )
{
   assert( dest.addr!= 0 );
   assert( dest.port != 0 );
   assert( mapAddr );

   const int NUM=3;

   if ( port == 0 )
   {
      port = randomPort();
   }

   *fd1=-1;
   *fd2=-1;

   char msg[STUN_MAX_MESSAGE_SIZE];
   int msgLen =sizeof(msg);

   StunAddress4 from;
   int fd[NUM];
   int i;

   unsigned int interfaceIp = 0;
   if ( srcAddr )
   {
      interfaceIp = srcAddr->addr;
   }

   for( i=0; i<NUM; i++)
   {
      fd[i] = openPort( (port == 0) ? 0 : (port + i),
                        interfaceIp, verbose);
      if (fd[i] < 0)
      {
         while (i > 0)
         {
            closesocket(fd[--i]);
         }
         return false;
      }
   }

   StunAtrString username;
   StunAtrString password;

   username.sizeValue = 0;
   password.sizeValue = 0;

#ifdef USE_TLS
   stunGetUserNameAndPassword( dest, username, password );
#endif

   for( i=0; i<NUM; i++)
   {
      stunSendTest(fd[i], dest, username, password, 1/*testNum*/, verbose );
   }

   StunAddress4 mappedAddr[NUM];
   for( i=0; i<NUM; i++)
   {
      msgLen = sizeof(msg)/sizeof(*msg);
      getMessage( fd[i],
                  msg,
                  &msgLen,
                  &from.addr,
                  &from.port ,verbose);

      StunMessage resp;
      memset(&resp, 0, sizeof(StunMessage));

      bool ok = stunParseMessage( msg, msgLen, resp, verbose );
      if (!ok)
      {
         return false;
      }

      mappedAddr[i] = resp.mappedAddress.ipv4;
      StunAddress4 changedAddr = resp.changedAddress.ipv4;
   }

   if (verbose)
   {
      LOG("--- stunOpenSocketPair --- " << endl);
      for( i=0; i<NUM; i++)
      {
         LOG("\t mappedAddr=" << mappedAddr[i] << endl);
      }
   }

   if ( mappedAddr[0].port %2 == 0 )
   {
      if (  mappedAddr[0].port+1 ==  mappedAddr[1].port )
      {
         *mapAddr = mappedAddr[0];
         *fd1 = fd[0];
         *fd2 = fd[1];
         closesocket( fd[2] );
         return true;
      }
   }
   else
   {
      if (( mappedAddr[1].port %2 == 0 )
          && (  mappedAddr[1].port+1 ==  mappedAddr[2].port ))
      {
         *mapAddr = mappedAddr[1];
         *fd1 = fd[1];
         *fd2 = fd[2];
         closesocket( fd[0] );
         return true;
      }
   }

   // something failed, close all and return error
   for( i=0; i<NUM; i++)
   {
      closesocket( fd[i] );
   }

   return false;
}
