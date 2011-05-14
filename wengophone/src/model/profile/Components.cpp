
#include "stdafx.h"	//VOXOX - JRT - 2009.04.01
#include "Components.h"
#include "assert.h"

#ifndef WIN32
  #define _stricmp strcasecmp
#endif

//=============================================================================

ImContact::ImContact()
{
	init();
}

//-----------------------------------------------------------------------------

ImContact::~ImContact()
{
}

//-----------------------------------------------------------------------------

void ImContact::init()
{
	setUserId( "" );
	setProtocol( "" );	//Enum?	Jabber, AIM, MSN, Yahoo!, etc

	setAlias   ( "" );
	setAccount ( "" );	//uuid?
	setPhoto   ( "" );	//base-64 encoded
}

//=============================================================================


//=============================================================================

void ImContacts::setImContact( const char* userId, const char* protocol, const char* alias,
							   const char* account, const char* photo )
{
}

//-----------------------------------------------------------------------------

void ImContacts::setImContact( ImContact& imContact )
{
}

//-----------------------------------------------------------------------------

ImContact* ImContacts::findByProtocol( const char* protocol )
{
	ImContact* pRet = NULL;

	for ( ImContacts::iterator iter = begin(); iter != end(); iter++ )
	{
		if ( _stricmp( protocol, iter->getProtocol() ) == 0 )
		{
			pRet = &(*iter);
			break;
		}
	}

	return pRet;
}

//-----------------------------------------------------------------------------

ImContact* ImContacts::findByUserId( const char* userId   )
{
	ImContact* pRet = NULL;

	for ( ImContacts::iterator iter = begin(); iter != end(); iter++ )
	{
		if ( _stricmp( userId, iter->getUserId() ) == 0 )
		{
			pRet = &(*iter);
			break;
		}
	}

	return pRet;
}

//=============================================================================


//=============================================================================

Group::Group()
{
	init();
}

//-----------------------------------------------------------------------------
	
Group::Group( const std::string& name, EnumGroupType::GroupType type )
{
	init();
	setName( name.c_str() );
	setType( type );
}

//-----------------------------------------------------------------------------

Group::~Group()
{
}

//-----------------------------------------------------------------------------

void Group::init()
{
	setUsername ("");
	setName     ("");
	setType     (EnumGroupType::GroupType_User);
	setIsDefault( false );
}

//-----------------------------------------------------------------------------

void Group::updateKey()
{
	_key = getName() + typeToString();
}

//-----------------------------------------------------------------------------

std::string Group::typeToString()
{
	return EnumGroupType::toString( _type );
}

//-----------------------------------------------------------------------------

bool Group::operator==( const Group& src )
{
	bool bEqual = false;

	if (   _username == src.getUsername() 
		&& _name     == src.getName()
		&& getType() == src.getType()   )
	{
		bEqual = true;
	}

	return bEqual;
}

//=============================================================================


//=============================================================================

Group* Groups::findByName( const char* name )
{
	Group* pRet = NULL;

	for ( Groups::iterator iter = begin(); iter != end(); iter++ )
	{
		if ( _stricmp( name, iter->getName() ) == 0 )
		{
			pRet = &(*iter);
			break;
		}
	}

	return pRet;
}

//-----------------------------------------------------------------------------
//Maintains backward compatibility with Profile::groupId
std::string Groups::getFirst() const
{
	std::string grp = "";

	for ( Groups::const_iterator iter = begin(); iter != end(); iter++ )
	{
		grp = (*iter).getName();
		break;
	}

	return grp;
}

//-----------------------------------------------------------------------------

void Groups::setGroup( const char* name )
{
	Group* pGroup = findByName( name );

	if ( pGroup )
	{
		assert( _stricmp( pGroup->getName(), name ) == 0 );
	}
	else
	{
		Group g;
		g.setName( name );

		Add( &g );
	}
}

//-----------------------------------------------------------------------------

Group* Groups::findFirstUserGroupInfo() const
{
	Group* pRet = NULL;

	for ( const_iterator it = begin(); it != end(); it++ )
	{
		if ( (*it).isUser() )
		{
			pRet = &(const_cast<Group&>((*it)));
			break;
		}
	}

	return pRet;
}

//-----------------------------------------------------------------------------

std::string Groups::getFirstUserGroupKey() const	//Mostly for backward compatibility.
{
	Group* pGroup = findFirstUserGroupInfo();
	std::string  result = (pGroup ? pGroup->getKey() : "");

	return result;
}

//=============================================================================





//=============================================================================

MergedContact::MergedContact()
{
	init();
}

//-----------------------------------------------------------------------------

MergedContact::~MergedContact()
{
}

//-----------------------------------------------------------------------------

void MergedContact::init()
{
	setParentUsername( "" );
	setUsername      ( "" );
}

//-----------------------------------------------------------------------------

bool MergedContact::isValid() const
{
	return !getUsername().empty();
}

//-----------------------------------------------------------------------------

bool MergedContact::operator==( const MergedContact& src )
{
	bool bEqual = false;

	if (   _username        == src.getUsername() 
		&& _parentUsername  == src.getParentUsername() )
	{
		bEqual = true;
	}

	return bEqual;
}

//=============================================================================


//=============================================================================

MergedContact* MergedContacts::findByUsernameAndNetworkId( const char* username, int networkId )
{
	MergedContact* pRet = NULL;

	for ( MergedContacts::iterator it = begin(); it != end(); it++ )
	{
		if ( it->getNetworkId() == networkId )
		{
			if ( username == it->getUsername() ) 
			{
				pRet = &(*it);
				break;
			}
		}
	}

	return pRet;
}

//-----------------------------------------------------------------------------
//ParentUsername is not really needed since all entries in list will/should be for a single parent Username.
//  We retain it though because we will most likely create a Master List of merged contacts.
void MergedContacts::set( const char* parentUsername, int parentNetworkId, const char* username, int networkId )
{
	MergedContact* pMC = findByUsernameAndNetworkId( username, networkId );

	if ( pMC )
	{
		assert( pMC->getUsername() == username );
		assert( pMC->getNetworkId() == networkId );
	}
	else
	{
		MergedContact mc;

		mc.setParentUsername ( parentUsername );
		mc.setParentNetworkId( parentNetworkId );
		mc.setUsername       ( username  );
		mc.setNetworkId		 ( networkId );

		Add( &mc );
	}
}

//=============================================================================



//=============================================================================

EmailAddress::EmailAddress()
{
	init();
}

//-----------------------------------------------------------------------------

EmailAddress::~EmailAddress()
{
}

//-----------------------------------------------------------------------------

void EmailAddress::init()
{
	setType		  ( "" );
	setAddress	  ( "" );
	setIsPreferred( false );
	setVisibility ( EnumVisibility::VisibilityUnknown );
}

//-----------------------------------------------------------------------------

EmailAddress& EmailAddress::operator=( const EmailAddress& src )
{
	if ( &src != this )
	{
		setType		  ( src.getType()		);
		setAddress	  ( src.getAddress()	);
		setIsPreferred( src.isPreferred()	);
		setVisibility ( src.getVisibility() );
	}

	return *this;
}

//-----------------------------------------------------------------------------

bool EmailAddress::operator==( const EmailAddress& src )
{
	bool bEqual = false;

	if (   _type		== src._type 
		&& _address		== src._address
		&& _isPreferred == src._isPreferred
		&& _visibility  == src._visibility )
	{
		bEqual = true;
	}

	return bEqual;
}

//-----------------------------------------------------------------------------

bool EmailAddress::isValid() const
{
	return !getAddress().empty();
}

//=============================================================================


//=============================================================================
//VOXOX - JRT - 2009.07.26 
void EmailAddresses::addPersonal( const std::string& address )
{
	setEmailAddress( "personal", address.c_str() );
}

//-----------------------------------------------------------------------------

void EmailAddresses::addWork( const std::string& address )
{
	setEmailAddress( "work", address.c_str() );
}

//-----------------------------------------------------------------------------

void EmailAddresses::addOther( const std::string& address )
{
	setEmailAddress( "other", address.c_str() );
}

//-----------------------------------------------------------------------------

void EmailAddresses::setEmailAddress( const char* type, const char* address, EnumVisibility::Visibility vis )
{
	EmailAddress* pEmail = findByType( type );

	if ( pEmail )
	{
		pEmail->setAddress( address );
		pEmail->setVisibility( vis );
	}
	else
	{
		EmailAddress email;
		email.setType	( type );
		email.setAddress( address );
		email.setVisibility( vis );

		Add( &email );
	}
}

//-----------------------------------------------------------------------------

std::string EmailAddresses::getEmailAddress( const char* type )
{
	std::string address = "";

	EmailAddress& rEmail = findOrCreateByType( type );

//	if ( rEmail )
//	{
		address = rEmail.getAddress();
//	}

	return address;
}

//=============================================================================



//=============================================================================

Telephone::Telephone()
{
	init();
}

//-----------------------------------------------------------------------------

Telephone::~Telephone()
{
}

//-----------------------------------------------------------------------------

void Telephone::init()
{
	setType		  ( "" );
	setNumber	  ( "" );
	setIsPreferred( false );
	setVisibility ( EnumVisibility::VisibilityUnknown );
}

//-----------------------------------------------------------------------------

Telephone& Telephone::operator=( const Telephone& src ) 
{
	if ( &src != this )
	{
		setType		  ( src.getType() );
		setNumber	  ( src.getNumber() );
		setIsPreferred( src.isPreferred() );
		setVisibility ( src.getVisibility() );
	}

	return *this;
}

//-----------------------------------------------------------------------------

bool Telephone::operator==( const Telephone& src )
{
	bool bEqual = false;

	if (   _type		== src._type 
		&& _number		== src._number
		&& _isPreferred == src._isPreferred
		&& _visibility  == src._visibility )
	{
		bEqual = true;
	}

	return bEqual;
}

//-----------------------------------------------------------------------------

bool Telephone::isValid() const
{
	return !getNumber().empty();
}

//=============================================================================


//=============================================================================

void Telephones::addVoxOxNumber( const std::string& number )
{
	setNumber( "voxox", number.c_str() );
}

//-----------------------------------------------------------------------------

void Telephones::addMobileNumber( const std::string& number )
{
	setNumber( "mobile", number.c_str() );
}

//-----------------------------------------------------------------------------

void Telephones::addWorkNumber( const std::string& number )
{
	setNumber( "work", number.c_str() );
}

//-----------------------------------------------------------------------------

void Telephones::addHomeNumber( const std::string& number )
{
	setNumber( "home", number.c_str() );
}

//-----------------------------------------------------------------------------

void Telephones::addFaxNumber( const std::string& number )
{
	setNumber( "fax", number.c_str() );
}

//-----------------------------------------------------------------------------

void Telephones::addOtherNumber( const std::string& number )
{
	setNumber( "other", number.c_str() );
}

//-----------------------------------------------------------------------------

std::string Telephones::getVoxOxNumber()
{
	return getNumber( "voxox" );
}

//-----------------------------------------------------------------------------

std::string Telephones::getMobileNumber()
{
	return getNumber( "mobile" );
}

//-----------------------------------------------------------------------------

std::string Telephones::getWorkNumber()
{
	return getNumber( "work" );
}

//-----------------------------------------------------------------------------

std::string Telephones::getHomeNumber()
{
	return getNumber( "home" );
}

//-----------------------------------------------------------------------------

std::string Telephones::getFaxNumber()
{
	return getNumber( "fax" );
}

//-----------------------------------------------------------------------------

std::string Telephones::getOtherNumber()
{
	return getNumber( "other" );
}

//-----------------------------------------------------------------------------

void Telephones::setNumber( const char* type, const char* number, EnumVisibility::Visibility vis )
{
	Telephone tel;
	tel.setType( type );
	tel.setNumber( number );
	tel.setVisibility( vis );

	AddOrUpdate( &tel );	//VOXOX - JRT - 2009.09.24 

//	Telephone* pTel = findByType( type );
//
//	if ( pTel )
//		{
////		if ( tel.isValid )
////		{
//			pTel->setNumber( number );
//			pTel->setVisibility( vis );
////		}
////		else
////		{
////			DeleteByType( type );
////		}
//		}
//		else
//		{
////		if ( tel.isValid )
////		{
//			Add( &tel );
////		}
//		}
}

//-----------------------------------------------------------------------------

std::string Telephones::getNumber( const char* type )
{
	std::string number = "";

	Telephone& rTel = findOrCreateByType( type );

//	if ( rTel )
	{
		number = rTel.getNumber();
	}

	return number;
}

//=============================================================================


//=============================================================================

Url::Url()
{
	init();
}

//-----------------------------------------------------------------------------

Url::~Url()
{
}

//-----------------------------------------------------------------------------

void Url::init()
{
	setType( "" );
	setUrl ( "" );
	setIsPreferred( false );
	setVisibility( EnumVisibility::VisibilityUnknown );
}

//-----------------------------------------------------------------------------

Url& Url::operator=( const Url& src )
{
	if ( &src != this )
	{
		setType( src.getType() );
		setUrl ( src.getUrl()  );
		setIsPreferred( src.isPreferred() );
		setVisibility( src.getVisibility() );
	}

	return *this;
}

//-----------------------------------------------------------------------------

bool Url::operator==( const Url& src )
{
	bool bEqual = false;

	if (   _type	   == src._type 
		&& _url		   == src._url 
		&& _visibility == src._visibility )
	{
		bEqual = true;
	}

	return bEqual;
}

//-----------------------------------------------------------------------------

bool Url::isValid() const
{
	return !getUrl().empty();
}

//=============================================================================


//=============================================================================

void Urls::setUrl( const char* type, const char* url, EnumVisibility::Visibility vis )
{
	Url* pUrl = findByType( type );

	if ( pUrl )
	{
		pUrl->setUrl( url );
		pUrl->setVisibility( vis );
	}
	else
	{
		Url u;
		u.setType( type );
		u.setUrl ( url );
		u.setVisibility( vis );

		Add( &u );
	}
}

//-----------------------------------------------------------------------------

std::string Urls::getUrl( const char* type )
{
	std::string url = "";

	Url& rUrl = findOrCreateByType( type );

//	if ( rUrl )
	{
		url = rUrl.getUrl();
	}

	return url;
}

//=============================================================================
MessageTranslation::MessageTranslation()
{
	init();
}

//-----------------------------------------------------------------------------

MessageTranslation::~MessageTranslation()
{
}

//-----------------------------------------------------------------------------

void MessageTranslation::init()
{
	setMessageTranslationMode ("None");
	setSourceMessageTranslationLanguage("");
	setDestMessageTranslationLanguage("");
}

//-----------------------------------------------------------------------------

MessageTranslation& MessageTranslation::operator=( const MessageTranslation& src ) 
{
	if ( &src != this )
	{
		setMessageTranslationMode( src.getMessageTranslationMode() );
		setSourceMessageTranslationLanguage( src.getSourceMessageTranslationLanguage() );
		setDestMessageTranslationLanguage( src.getDestMessageTranslationLanguage() );
	}

	return *this;
}

//-----------------------------------------------------------------------------

bool MessageTranslation::operator==( const MessageTranslation& src )
{
	bool bEqual = false;

	if ( _MessageTranslationMode	== src._MessageTranslationMode
		&& _sourceMessageTranslationLanguage == src._sourceMessageTranslationLanguage
		&& _destMessageTranslationLanguage  == src._destMessageTranslationLanguage )
	{
		bEqual = true;
	}

	return bEqual;
}

//-----------------------------------------------------------------------------

bool MessageTranslation::isValid() const
{
	return !getMessageTranslationMode().empty();
}
