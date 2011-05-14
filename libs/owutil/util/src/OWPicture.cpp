/*
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

//#include "stdafx.h"		//VOXOX - JRT - 2009.04.03 - For precompiled headers.
#include <util/OWPicture.h>

#include <util/File.h>
#include <util/VoxMd5.h>	//VOXOX - JRT - 2009.08.14 
#include <util/Base64.h>	//VOXOX - JRT - 2009.08.14 

//#ifdef _WIN32
//#include <windows.h>		//JRT-XXX
//#endif

using namespace std;

OWPicture::OWPicture() 
{
}

OWPicture::OWPicture(const OWPicture & picture)
{
	copy(picture);
}

OWPicture & OWPicture::operator=(const OWPicture & picture) 
{
	if ( this != &picture )
	{
		copy(picture);
	}

	return *this;
}

void OWPicture::copy(const OWPicture & picture) 
{
	_pictureData = picture._pictureData;
	_filename	 = picture._filename;
	_type		 = picture._type;
	_md5		 = picture._md5;
	_providedMd5 = picture._providedMd5;

//#ifdef _WIN32		//JRT-XXX
//	static int count     = 0;
//	static int zeroCount = 0;
//	static int oneCount  = 0;
//
//	if ( getData().size() == 0 )
//		zeroCount++;
//	else
//		oneCount++;
//
//	count++;
//
//	char msg[200];
//	sprintf_s( msg, 200, "OWPicture::copy: times called: %d, empty: %d, valid: %d\n", count, zeroCount, oneCount );
//	OutputDebugString( msg );
//#endif

	//if ( getData().size() < 12000 )	//JRT-XXX
	//	int xxx = 1;
}

OWPicture OWPicture::pictureFromData(const std::string & data) 
{
	OWPicture result;

	result._pictureData = data;
	result.calcMd5();			//VOXOX - JRT - 2009.06.07 

	return result;
}

OWPicture OWPicture::pictureFromFile(const std::string & filename) 
{
	string data;
	FileReader file(filename);

	if (file.open()) 
	{
		data = file.read();
		file.close();
	}

	OWPicture result = pictureFromData(data);
	result.setFilename(filename);

	return result;
}

string OWPicture::getData() const 
{
	return _pictureData;
}

string OWPicture::getFilename() const 
{
	return _filename;
}

void OWPicture::setFilename(const string & filename) 
{
	string path = filename;
	path = File::convertPathSeparators(path);
	string::size_type pos = path.rfind(File::getPathSeparator());

	_filename = path.substr(pos + 1);	//VOXOX - JRT - 2009.08.14 - TODO: why aren't we saving fullpath name?

	File file(filename);
	_type = file.getExtension();
}

void OWPicture::calcMd5()
{
	VoxMd5 md5;

	//VOXOX - JRT - 2009.08.14 
	std::string temp = Base64::encode( getData() );
	_md5 = md5.toString( temp.c_str(), temp.size() );
}

bool OWPicture::isValid() const
{
	return (getData().size() > 0);
}
