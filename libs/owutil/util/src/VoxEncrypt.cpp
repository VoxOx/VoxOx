/*
	VoxOx!!!

 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2006  Wengo
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

//#include "stdafx.h"
//VOXOX - JRT - 2009.09.26 - A couple typedefs to get this to compile.
#if defined (_WIN32)
	#define ssize_t int

	#ifndef snprintf
		#define snprintf _snprintf
	#endif
#endif


typedef int pid_t;

//Keep the gcrypt.h before the VoxEncrypt.h
#include <gcrypt.h>

#include <util/VoxEncrypt.h>
#include <assert.h>

//-----------------------------------------------------------------------------

std::string getErrorMsg( gcry_error_t err )
{
	std::string errMsg = "";

	errMsg += gcry_strsource ( err );
	errMsg += " - ";
	errMsg += gcry_strerror ( err );

	return errMsg;
}

//-----------------------------------------------------------------------------

VoxEncrypt::VoxEncrypt()
{
	_initializationVector = "";

	gcry_check_version ( NULL );
	gcry_control ( GCRYCTL_DISABLE_SECMEM_WARN );
	gcry_control ( GCRYCTL_INIT_SECMEM, 16384, 0 );
}

//-----------------------------------------------------------------------------

bool VoxEncrypt::encrypt( const char* input, int inputLen, const std::string& key, char** result, std::string& errMsg )
{
	return process( true, input, inputLen, key, result, errMsg );
}

//-----------------------------------------------------------------------------

bool VoxEncrypt::decrypt( const char* input, int inputLen, const std::string& key, char** result, std::string& errMsg )
{
	return process( false, input, inputLen, key, result, errMsg );
}

//-----------------------------------------------------------------------------

bool VoxEncrypt::process( bool encrypting, const char* input, int inputLen, const std::string& key, char** result, std::string& errMsg )
{
	*result = NULL;
	errMsg  = "";
	
	bool success = false;

//	size_t inLen  = input.size() * sizeof(char);		//+1?
	size_t outLen = (inputLen * 10) + 1;				//Not sure why.
	size_t keyLen = key.size();

//	char* plain_text  = new char[inputLen+1];	//VOXOX - JRT - 2009.11.04 
	char* out		  = new char[outLen];

//	memset( plain_text, 0, inputLen+1 );		//VOXOX - JRT - 2009.11.04 
	memset( out,        0, outLen  );

//	assert ( plain_text );
	assert ( out );

//	strncpy ( plain_text, input, inputLen );	//VOXOX - JRT - 2009.11.04 

	gcry_cipher_hd_t handle = NULL;
	gcry_error_t	 err    = 0;

	if ( err == 0 )
	{
		err = gcry_cipher_open( &handle, GCRY_CIPHER_AES, GCRY_CIPHER_MODE_CBC, 0 );

		if ( err == 0 )
		{
			if ( _initializationVector != "" )
			{
				err = gcry_cipher_setiv( handle, (void*)_initializationVector.c_str(), _initializationVector.size() );
			}

			if ( err == 0 )
			{
				err = gcry_cipher_setkey( handle,  key.c_str(), keyLen );
				
				if ( err == 0 )
				{
					if ( encrypting )
					{
//						err =  gcry_cipher_encrypt( handle, ( unsigned char * ) out, outLen, ( const unsigned char * ) plain_text, inputLen );
						err =  gcry_cipher_encrypt( handle, ( unsigned char * ) out, outLen, ( const unsigned char * ) input, inputLen );	//VOXOX - JRT - 2009.11.04 
					}
					else
					{
//						err =  gcry_cipher_decrypt( handle, ( unsigned char * ) out, outLen, ( const unsigned char * ) plain_text, inputLen );
						err =  gcry_cipher_decrypt( handle, ( unsigned char * ) out, outLen, ( const unsigned char * ) input, inputLen );	//VOXOX - JRT - 2009.11.04 
					}
				}
			}
		}
	}

	if ( err == 0 )
	{
		*result = out;
		success = true;
	}
	else
	{
		delete[]  out;
		*result = NULL;
		errMsg  = getErrorMsg( err );
		success = false;
	}

	//Clean up
//	delete[]  plain_text;	//VOXOX - JRT - 2009.11.04 

	gcry_cipher_close( handle );

	return success;
}

//-----------------------------------------------------------------------------

std::string VoxEncrypt::Base16_Encode( const char* data, int len )
{
	std::string result = "";

	if ( data != "" )
	{
		int   i		 = 0;
		int   buflen = (len * 2) + 1;
		char* ascii  = new char[buflen];

		memset( ascii, 0, len );

		for (i = 0; i < len; i++)
		{
			snprintf(&ascii[i * 2], 3, "%02hhx", (unsigned char)data[i]);
		}

		result = ascii;
	}

	return result;
}

//-----------------------------------------------------------------------------

char* VoxEncrypt::Base16_Decode( const char *str, int& retLen )
{
	char* result = NULL;

	if ( str )
	{
		int len = strlen(str);

		if ( len > 0 )
		{
			int i			= 0;
			int accumulator = 0;
			int buflen		= (len/2) + 1;

			unsigned char* data = new unsigned char[buflen];

			memset( data, 0, buflen );

			for (i = 0; i < len; i++)
			{
				if ((i % 2) == 0)
				{
					accumulator = 0;
				}
				else
				{
					accumulator <<= 4;
				}

				if (isdigit(str[i]))
				{
					accumulator |= str[i] - 48;
				}
				else
				{
					switch(tolower(str[i]))
					{
						case 'a':  accumulator |= 10;  break;
						case 'b':  accumulator |= 11;  break;
						case 'c':  accumulator |= 12;  break;
						case 'd':  accumulator |= 13;  break;
						case 'e':  accumulator |= 14;  break;
						case 'f':  accumulator |= 15;  break;
					}
				}

				if (i % 2)
				{
					data[(i - 1) / 2] = accumulator;
				}
			}

			result = (char*)data;
			retLen = len / 2;

//			delete[] data;	//Don't delete because 'result' is this ptr, just recast.
		}
	}

	return result;
}

//-----------------------------------------------------------------------------
