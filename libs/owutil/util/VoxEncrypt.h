
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

#ifndef VOX_ENCRYPT_H
#define VOX_ENCRYPT_H

#include <util/owutildll.h>		//Access libgcrypt-11.dll

#include <string>

class OWUTIL_API VoxEncrypt
{
public:
	VoxEncrypt();

	void setInitializationVector( const std::string& val )		{ _initializationVector = val;	}

	bool encrypt( const char* input, int inputLen, const std::string& key, char** result, std::string& errMsg );
	bool decrypt( const char* input, int inputLen, const std::string& key, char** result, std::string& errMsg );

	std::string Base16_Encode( const char* data, int len );
	char* Base16_Decode( const char *str, int& retLen );

private:
	bool process( bool encrypting, const char* input, int inputLen, const std::string& key, char** result, std::string& errMsg );

	std::string _initializationVector;
};

#endif //VOX_ENCRYPT_H
