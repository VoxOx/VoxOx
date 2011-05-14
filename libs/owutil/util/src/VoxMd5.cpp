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
#include <util/VoxMd5.h>
#include <util/md5.h>
#include <assert.h>

//-----------------------------------------------------------------------------

VoxMd5::VoxMd5()
{
}

//-----------------------------------------------------------------------------

void VoxMd5::toRaw( const char* input, int inputLen, unsigned char* md5 )
{
	//assert( strlen( (const char*)md5) >= 16 );

	MD5Context context;

	MD5Init  ( &context );	//Init context
	MD5Update( &context, (const unsigned char *)input, inputLen );	
	MD5Final ( md5, &context );
}

//-----------------------------------------------------------------------------

std::string VoxMd5::toString( const char* input, int inputLen, bool bUpper )
{
	std::string result = "";
	unsigned char md5[16];

	toRaw( input, inputLen, md5 );

	for ( int x = 0; x < 16; x++ )
	{
		result += toHex( md5[x], bUpper );
	}

	return result;
}

//-----------------------------------------------------------------------------

std::string VoxMd5::toHex( unsigned char c, bool bUpper )
{
	std::string result = "";

	unsigned char temp;

	temp = c & 0xf0;
	temp = temp >> 4;
	result += toHexPrivate( temp, bUpper );

	temp = c & 0x0f;
	result += toHexPrivate( temp, bUpper );

	return result;
}

//-----------------------------------------------------------------------------

unsigned char VoxMd5::toHexPrivate( unsigned char c, bool bUpper )
{
	unsigned char result = 0;
	if		( c >= 0 && c <= 9 )
	{
		result = '0' + c;
	}
	else if ( c >= 10 && c <= 15 )
	{
		if ( bUpper )
		{
			result = 'A' + (c- 10);
		}
		else
		{
			result = 'a' + (c- 10);
		}
	}
	else
	{
		result = 0;
	}

	return result;
}

//-----------------------------------------------------------------------------
