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

#include "stdafx.h"	//VOXOX - JRT - 2009.04.01
#include "ProfileXMLSerializer.h"

#include "Profile.h"

#include <model/profile/StreetAddressXMLSerializer.h>
#include <model/profile/StreetAddress.h>

#include <serializer/DateXMLSerializer.h>

#include <util/Base64.h>

#include <tinyxml.h>

using namespace std;

//-----------------------------------------------------------------------------

ProfileXMLSerializer::ProfileXMLSerializer(Profile & profile)
	: _profile(profile) 
{
}

//-----------------------------------------------------------------------------

string ProfileXMLSerializer::serialize() 
{
	string result = "";

	result += serializeTag( "wengoid", _profile._wengoPhoneId );

	// Serializing names
	result += openTag( "name" );
	result += serializeTag( "first", _profile._firstName, true, true );
	result += serializeTag( "last",  _profile._lastName,  true, true );
	result += closeTag();

	//Serializing sex
	if (_profile._sex != EnumSex::SexUnknown) 
	{
		result += serializeTag( "sex", EnumSex::toString(_profile._sex) );
	}

	result += serializeTag( "alias",			_profile._alias,		  true, true );
	result += serializeTag( "statusMessage",	_profile._statusMessage, true, true );
	result += serializeTag( "smssignature type=\"other\"", _profile._smsSignature );	//TODO: why the attribute?
	result += serializeTag( "notes",			_profile._notes, true, true ); 
	result += serializeTag( "organization",		_profile.getCompany() );
	result += serializeTag( "title",			_profile.getTitle() );
	result += serializeTag( "timestamp",		_profile.getTimestamp() );		//VOXOX - JRT - 2009.08.14 

	// Serializing photo
	if (!_profile._icon.getData().empty()) 	//TODO: revisit
	{
		result += ("<photo filename=\"" + _profile._icon.getFilename()
			+ "\"><![CDATA[" + Base64::encode(_profile._icon.getData()) + "]]></photo>");
	}

	// Serializing birthday
	result += "<birthday>\n";
	DateXMLSerializer serializer(_profile._birthdate);
	result += serializer.serialize();
	result += "</birthday>\n";
	result += serializeMessageTranslation()+"\n";//VOXOX - CJC - 2010.01.22 

	//Serializing phone numbers	//VOXOX - JRT - 2009.03.31
	Telephones& rPhones = _profile.getTelephones();

	for ( Telephones::iterator it = rPhones.begin(); it != rPhones.end(); it++ )
	{
		if ( (*it).isValid() )
		{
			result += serializeTagAndType( "tel", (*it).getType().c_str(), (*it).getNumber().c_str(),  true, false );
		}
	}

	// Serializing street address	//VOXOX - JRT - 2009.03.31 - 
	StreetAddresses& rAddrs = _profile.getStreetAddresses();

	for ( StreetAddresses::iterator it = rAddrs.begin(); it != rAddrs.end(); it++ )
	{
		if ( (*it).isValid() )
		{
			StreetAddressXMLSerializer streetSerializer( (*it));
			result += streetSerializer.serialize();
		}
	}

	// Serializing email 
	EmailAddresses& rEmails = _profile.getEmailAddresses();

	for ( EmailAddresses::iterator it = rEmails.begin(); it != rEmails.end(); it++ )
	{
		if ( (*it).isValid() )
		{
			result += serializeTagAndType( "email", (*it).getType().c_str(), (*it).getAddress().c_str(),  true, false );
		}
	}


	// Serializing urls
	Urls& rUrls= _profile.getUrls();

	for ( Urls::iterator it = rUrls.begin(); it != rUrls.end(); it++ )
	{
		if ( (*it).isValid() )
		{
			result += serializeTagAndType( "url", (*it).getType().c_str(), _profile.getWebsite(), true, true );
		}
	}

	return result;
}

std::string ProfileXMLSerializer::serializeMessageTranslation()//VOXOX - CJC - 2010.01.21 
{
	std::string result = "";

	const MessageTranslation & messageTranslation = _profile.getMessageTranslation();

	if ( !messageTranslation.isValid() )
		return result;

	result += openTag( "MESSAGETRANSLATION" );

	result += serializeTag( "MODE",  messageTranslation.getMessageTranslationMode().c_str()  );
	result += serializeTag( "SRCLANGUAGE", messageTranslation.getSourceMessageTranslationLanguage().c_str()	);
	result += serializeTag( "DESTLANGUAGE", messageTranslation.getDestMessageTranslationLanguage().c_str()	);


	result += closeTag();



	return result;
}
//-----------------------------------------------------------------------------

bool ProfileXMLSerializer::unserialize(const std::string & data) {
	return false;
}

//-----------------------------------------------------------------------------

bool ProfileXMLSerializer::unserializeContent(TiXmlHandle & rootElt) 
{
	// Here we need to call setWengoPhoneId because each class inherited
	// from Profile handles the wengophone ID differently.
	_profile.setWengoPhoneId( fromNode( rootElt, "wengoid" ) );

	// Retrieving names
	_profile.setFirstName( fromNode( rootElt.FirstChild("name").FirstChild("first").FirstChild().Node() ) );
	_profile.setLastName ( fromNode( rootElt.FirstChild("name").FirstChild("last").FirstChild().Node()  ) );

	_profile.setAlias		 ( fromNode( rootElt, "alias"		  ) );
	_profile.setStatusMessage( fromNode( rootElt, "statusMessage" ) );
	_profile.setSmsSignature ( fromNode( rootElt, "smssignature"  ) );
	_profile.setNotes		 ( fromNode( rootElt, "notes"		  ) );
	_profile.setCompany		 ( fromNode( rootElt, "company"		  ) );
	_profile.setTitle		 ( fromNode( rootElt, "title"		  ) );
	_profile.setTimestamp	 ( toInt( fromNode( rootElt, "timestamp"	  ) ) );		//VOXOX - JRT - 2009.08.14 
	_profile.setSex			 ( EnumSex::toSex( fromNode( rootElt, "sex" ) ) );

	// Retrieving URLs
	if (rootElt.Node()) {
		TiXmlNode * url = NULL;
		while ((url = rootElt.Node()->IterateChildren("url", url))) {
			TiXmlElement * urlElt = url->ToElement();
			string typeAttr = string(urlElt->Attribute("type"));
			if (typeAttr == "website") {
				TiXmlNode * website = url->FirstChild();
				if (website) {
					_profile.setWebsite( website->Value() );	//VOXOX - JRT - 2009.03.31 - Use accessor!
				}
			}
		}
	}
	////

	// Retrieving birthday
	TiXmlNode * birthday = rootElt.FirstChild("birthday").FirstChild("date").Node();
	if (birthday) {
		TiXmlElement * birthdayElt = birthday->ToElement();
		string birthdayData;
		birthdayData << *birthdayElt;
		Date date;
		DateXMLSerializer dateSerializer(date);
		dateSerializer.unserialize(birthdayData);
		_profile._birthdate = date;
	}
	unserializeMessageTranslation(rootElt);//VOXOX - CJC - 2010.01.22 
	/////

	// Retrieving organization
	////


	// Retrieving address	//VOXOX - JRT - 2009.06.13 - TODO: iterate once more than one is supported in UI.
	TiXmlNode * address = rootElt.FirstChild("address").Node();
	if (address) 
	{
		TiXmlElement * addressElt = address->ToElement();
		string addressData;
		addressData << *addressElt;
		StreetAddress streetAddress;
		StreetAddressXMLSerializer addressSerializer(streetAddress);
		addressSerializer.unserialize(addressData);
//		_profile._streetAddress = streetAddress;

		streetAddress.verifyType();						//VOXOX - JRT - 2009.04.05
		_profile.setStreetAddress( streetAddress );		//VOXOX - JRT - 2009.03.31 - Use accessor!
	}
	////

	// Retrieving phone numbers
	TiXmlNode* telNode   = NULL;
	Telephone phone;

	while ((rootElt.Node()) && (telNode = rootElt.Node()->IterateChildren("tel", telNode))) 
	{
		TiXmlElement* telElt    = telNode->ToElement();
		string        typeAttr  = string(telElt->Attribute("type"));
		TiXmlNode *   phoneNode = telNode->FirstChild();

		phone.setType  ( typeAttr );
		phone.setNumber( phoneNode->Value() );

		if ( phone.isValid() )
		{
			_profile.getTelephones().Add( &phone );
		}
	}

	// Retrieving emails
	TiXmlNode*   emailNode = NULL;
	EmailAddress email;

	while ((rootElt.Node()) && (emailNode = rootElt.Node()->IterateChildren("email", emailNode))) 
	{
		TiXmlElement* emailElt = emailNode->ToElement();
		string		  typeAttr = string(emailElt->Attribute("type"));
		TiXmlNode*    node	   = emailNode->FirstChild();

		email.setType   ( typeAttr );
		email.setAddress( node->Value() );

		if ( email.isValid() )
		{
			_profile.getEmailAddresses().Add( &email );
		}
	}

	// Retrieving URLs
	TiXmlNode*  urlNode = NULL;
	Url			url;

	while ((rootElt.Node()) && (urlNode = rootElt.Node()->IterateChildren("url", urlNode))) 
	{
		TiXmlElement* urlElt   = urlNode->ToElement();
		string		  typeAttr = string(urlElt->Attribute("type"));
		TiXmlNode*    node	   = urlNode->FirstChild();

		url.setType( typeAttr );
		url.setUrl ( node->Value() );

		if ( url.isValid() )
		{
			_profile.getUrls().Add( &url );
		}
	}

	// Retrieving icon
	TiXmlNode * photo = rootElt.FirstChild("photo").Node();
	if (photo) {
		TiXmlElement * photoElt = photo->ToElement();
		string filename;
		const char * filenameAttr = photoElt->Attribute("filename");
		if (filenameAttr) {
			filename = string(filenameAttr);
		}

		TiXmlNode * photoNode = photo->FirstChild();
		if (photoNode) {
			OWPicture picture = OWPicture::pictureFromData(Base64::decode(photoNode->Value()));
			picture.setFilename(filename);
			_profile._icon = picture;
		}
	}
	////

	return true;
}

//-----------------------------------------------------------------------------

void ProfileXMLSerializer::unserializeMessageTranslation(TiXmlHandle & rootElt )
{
	TiXmlHandle messageTranslationElt = rootElt.FirstChild("MESSAGETRANSLATION");
	MessageTranslation messageTranslation;
	messageTranslation.setMessageTranslationMode(fromNode( messageTranslationElt, "MODE" ));
	messageTranslation.setSourceMessageTranslationLanguage(fromNode( messageTranslationElt, "SRCLANGUAGE" ));
	messageTranslation.setDestMessageTranslationLanguage(fromNode( messageTranslationElt, "DESTLANGUAGE" ));
	_profile.setMessageTranslation(messageTranslation);

}
