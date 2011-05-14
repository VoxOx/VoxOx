#include <time.h>
#include <assert.h>

#ifdef WIN32

#include <winsock2.h>
#include <stdlib.h>
#include <io.h>

#else

#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#endif

#include <string.h>

#include "udp.h"

#define bool int
#define false 0
#define true 1

#define DBG(x)

Socket
openPort( unsigned short port, unsigned int interfaceIp, bool verbose )
{
   Socket fd;
   struct sockaddr_in addr;
    
   fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
   if ( fd == INVALID_SOCKET )
   {
      int err = getErrno();
      DBG(printf("Could not create a UDP socket: %d\n", err));
      return INVALID_SOCKET;
   }
    
   memset((char*) &(addr),0, sizeof((addr)));
   addr.sin_family = AF_INET;
   addr.sin_addr.s_addr = htonl(INADDR_ANY);
   addr.sin_port = htons(port);
    
   if ( (interfaceIp != 0) && 
        ( interfaceIp != 0x100007f ) )
   {
      addr.sin_addr.s_addr = htonl(interfaceIp);
      if (verbose )
      {
#if 0
         clog << "Binding to interface " 
              << hex << "0x" << htonl(interfaceIp) << dec << endl;
#endif
      }
   }
	
   if ( bind( fd,(struct sockaddr*)&addr, sizeof(addr)) != 0 )
   {
      int e = getErrno();
        
      switch (e)
      {
         case 0:
         {
            DBG(printf("Could not bind socket\n"));
            return INVALID_SOCKET;
         }
         case EADDRINUSE:
         {
            DBG(printf("Port %d receiving UDP is in use\n", port));
            return INVALID_SOCKET;
         }
         break;
         case EADDRNOTAVAIL:
         {
            if ( verbose ) 
            {
               DBG(printf("Cannot assign requested address\n"));
            }
            return INVALID_SOCKET;
         }
         break;
         default:
         {
            DBG(printf("Could not bind UDP receive port, err=%s\n", strerror(e)));
            return INVALID_SOCKET;
         }
         break;
      }
   }
   if ( verbose )
   {
      DBG(printf("Opened port %d with fd %d\n", port, fd));
   }
   
   assert( fd != INVALID_SOCKET  );
	
   return fd;
}





int 
getMessageWithTimeout( Socket fd, char* buf, int* len,
            unsigned int* srcIp, unsigned short* srcPort,
            int verbose,  long sec,  long usec)

{

    struct timeval tv;
    fd_set rset;
	  int retVal;

   int originalSize = *len;
   struct sockaddr_in from;
   int fromLen = sizeof(from);
   

   assert( fd != INVALID_SOCKET );	
   assert( originalSize > 0 );

   // add select

   
	                         
#ifndef WIN32
#define ioctlsocket ioctl
#endif
   {
    unsigned long non_blocking = 1;
    if (ioctlsocket(fd, FIONBIO, &non_blocking))
        return SOCKET_ERROR;
   }

	
   

    // select will return upon timeout, error or received message
    FD_ZERO(&rset);
    FD_SET(fd, &rset);
    tv.tv_sec = sec;
    tv.tv_usec = usec;
	#ifdef WIN32
    retVal = select(0, &rset, NULL, NULL, &tv);
	#else
	 retVal = select(fd + 1, &rset, NULL, NULL, &tv);
	#endif
    // we don't care about no stinking error
    if (retVal <= 0)
        return retVal;


   *len = recvfrom(fd,
                   buf,
                   originalSize,
                   0,
                   (struct sockaddr *)&from,
                   (socklen_t*)&fromLen);
   
	
   if ( *len == SOCKET_ERROR )
   {
      int err = getErrno();
		
      switch (err)
      {
         case ENOTSOCK:
            DBG(printf("Error fd not a socket\n"));
            break;
         case ECONNRESET:
            DBG(printf("Error connection reset - host not reachable\n"));
            break;
				
         default:
            DBG(printf("Socket Error=%d\n", err));
            break;
      }
		
      return false;
   }
	
   if ( *len < 0 )
   {
      DBG(printf("socket closed? negative len\n"));
      return false;
   }
    
   if ( *len == 0 )
   {
      DBG(printf("socket closed? zero len\n"));
      return false;
   }
    
   *srcPort = ntohs(from.sin_port);
   *srcIp = ntohl(from.sin_addr.s_addr);
	
   if ( (*len)+1 >= originalSize )
   {
      if (verbose)
      {
         DBG(printf("Received a message that was too large\n"));
      }
      return false;
   }
   buf[*len]=0;
    
   return true;

}


bool 
getMessage( Socket fd, char* buf, int* len,
            unsigned int* srcIp, unsigned short* srcPort,
            bool verbose)
{
   int originalSize = *len;
   struct sockaddr_in from;
   int fromLen = sizeof(from);

   assert( fd != INVALID_SOCKET );	
   assert( originalSize > 0 );
   
	
   *len = recvfrom(fd,
                   buf,
                   originalSize,
                   0,
                   (struct sockaddr *)&from,
                   (socklen_t*)&fromLen);
	
   if ( *len == SOCKET_ERROR )
   {
      int err = getErrno();
		
      switch (err)
      {
         case ENOTSOCK:
            DBG(printf("Error fd not a socket\n"));
            break;
         case ECONNRESET:
            DBG(printf("Error connection reset - host not reachable\n"));
            break;
				
         default:
            DBG(printf("Socket Error=%d\n", err));
            break;
      }
		
      return false;
   }
	
   if ( *len < 0 )
   {
      DBG(printf("socket closed? negative len\n"));
      return false;
   }
    
   if ( *len == 0 )
   {
      DBG(printf("socket closed? zero len\n"));
      return false;
   }
    
   *srcPort = ntohs(from.sin_port);
   *srcIp = ntohl(from.sin_addr.s_addr);
	
   if ( (*len)+1 >= originalSize )
   {
      if (verbose)
      {
         DBG(printf("Received a message that was too large\n"));
      }
      return false;
   }
   buf[*len]=0;
    
   return true;
}


bool 
sendMessage( Socket fd, char* buf, int l, 
             unsigned int dstIp, unsigned short dstPort,
             bool verbose)
{
   int s;
   assert( fd != INVALID_SOCKET );
    
   if ( dstPort == 0 )
   {
      // sending on a connected port 
      assert( dstIp == 0 );
		
      s = send(fd,buf,l,0);
   }
   else
   {
      struct sockaddr_in to;
      int toLen = sizeof(to);

	  assert( dstIp != 0 );
      assert( dstPort != 0 );
        
      memset(&to,0,toLen);
        
      to.sin_family = AF_INET;
      to.sin_port = htons(dstPort);
      to.sin_addr.s_addr = htonl(dstIp);
        
      s = sendto(fd, buf, l, 0,(struct sockaddr*)&to, toLen);
   }
    
   if ( s == SOCKET_ERROR )
   {
      int e = getErrno();
      switch (e)
      {
         case ECONNREFUSED:
         case EHOSTDOWN:
         case EHOSTUNREACH:
         {
            // quietly ignore this 
         }
         break;
         case EAFNOSUPPORT:
         {
            DBG(printf("err EAFNOSUPPORT in send\n"));
         }
         break;
         default:
         {
            DBG(printf("err %s %s in send\n", e strerror(e)));
         }
      }
      return false;
   }
    
   if ( s == 0 )
   {
      DBG(printf("no data sent in send\n"));
      return false;
   }
    
   if ( s != l )
   {
      if (verbose)
      {
         DBG(printf("only %d out of %d bytes sent\n", s, l));
      }
      return false;
   }
    
   return true;
}


void
initNetwork()
{
#ifdef WIN32 
   WORD wVersionRequested = MAKEWORD( 2, 2 );
   WSADATA wsaData;
   int err;
	
   err = WSAStartup( wVersionRequested, &wsaData );
   if ( err != 0 ) 
   {
      // could not find a usable WinSock DLL
//      cerr << "Could not load winsock" << endl;
      assert(0); // is this is failing, try a different version that 2.2, 1.0 or later will likely work 
      exit(1);
   }
    
   /* Confirm that the WinSock DLL supports 2.2.*/
   /* Note that if the DLL supports versions greater    */
   /* than 2.2 in addition to 2.2, it will still return */
   /* 2.2 in wVersion since that is the version we      */
   /* requested.                                        */
    
   if ( LOBYTE( wsaData.wVersion ) != 2 ||
        HIBYTE( wsaData.wVersion ) != 2 ) 
   {
      /* Tell the user that we could not find a usable */
      /* WinSock DLL.                                  */
      WSACleanup( );
//      cerr << "Bad winsock verion" << endl;
      assert(0); // is this is failing, try a different version that 2.2, 1.0 or later will likely work 
      exit(1);
   }    
#endif
}


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

// Local Variables:
// mode:c++
// c-file-style:"ellemtel"
// c-file-offsets:((case-label . +))
// indent-tabs-mode:nil
// End:
