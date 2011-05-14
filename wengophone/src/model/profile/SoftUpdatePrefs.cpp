/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2007 Wengo
 *
 *	VOXOX!!!!!
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

#include "stdafx.h"
#include <model/profile/SoftUpdatePrefs.h>
#include <tinyxml.h>

//=============================================================================

SoftUpdatePrefs::SoftUpdatePrefs()
{
	initVars();
}

//-----------------------------------------------------------------------------

void SoftUpdatePrefs::initVars()
{
	setVersionToIgnore( "" );
	setDateLastIgnored( Date(1,1,2000) );
	setAutoCheck	  ( true );
}

//-----------------------------------------------------------------------------

SoftUpdatePrefs& SoftUpdatePrefs::operator=( const SoftUpdatePrefs& src )
{
	if ( this != &src )
	{
		setVersionToIgnore( src.getVersionToIgnore() );
		setDateLastIgnored( src.getDateLastIgnored() );
		setAutoCheck	  ( src.autoCheck()			 );
	}

	return *this;
}

//=============================================================================


//=============================================================================

SoftUpdatePrefsXMLSerializer::SoftUpdatePrefsXMLSerializer( SoftUpdatePrefs& prefs )
	: _prefs( prefs ) 
{
}

//-----------------------------------------------------------------------------

std::string SoftUpdatePrefsXMLSerializer::serialize()
{
	std::string result = "";

	result += openTag( "softUpdate" );

	result += serializeTag( "versionToIgnore", _prefs.getVersionToIgnore(), false );
	result += serializeTag( "dateLastIgnored", _prefs.getDateLastIgnored(), false );
	result += serializeTag( "autoCheck",	   _prefs.autoCheck(),			false );

	result += closeTag();

	return result;
}

//-----------------------------------------------------------------------------

bool SoftUpdatePrefsXMLSerializer::unserialize( const std::string& xml )
{
	TiXmlDocument doc;

	doc.Parse(xml.c_str());

	TiXmlHandle docHandle(&doc);
	TiXmlNode*  node = docHandle.FirstChild("softUpdate").Node();

	return unserialize( node );
}

//-----------------------------------------------------------------------------

bool SoftUpdatePrefsXMLSerializer::unserialize( TiXmlNode* node )
{
	bool result = true;

	if (node) 
	{
		_prefs.setVersionToIgnore ( 	    fromNode( node, "versionToIgnore"   ) );
		_prefs.setDateLastIgnored(	toDate( fromNode( node, "dateLastIgnored" ) ) );
		_prefs.setAutoCheck       ( toBool( fromNode( node, "autoCheck"		  ) ) );
	}

	return result;
}

//-----------------------------------------------------------------------------
