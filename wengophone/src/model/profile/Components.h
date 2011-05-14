#ifndef _VOXOX_COMPONENTS_H_
#define _VOXOX_COMPONENTS_H_

#include "VoxListTemplate.h"
#include "EnumVisibility.h"
#include "contactlist/EnumGroupType.h"
#include <string>
#include <list>

#include "util/String.h"

//=============================================================================
//These classes are used primarily with Contacts:
//	ImContact, Group, MergedContact
//	They all do NOT contain Visibility as a data member.
//=============================================================================

class ImContact
{
public:
	ImContact();
	virtual ~ImContact();

	const char*	getUserId()		const			{ return _userId.c_str();	}
	const char* getProtocol()	const			{ return _protocol.c_str();	}
	const char*	getAlias()		const			{ return _alias.c_str();	}
	const char* getAccount()	const			{ return _account.c_str();	}
	const char*	getPhoto()		const			{ return _photo.c_str();	}

	void setUserId  ( const char* str )			{ _userId   = str;	}
	void setProtocol( const char* str )			{ _protocol = str;	}
	void setAlias   ( const char* str )			{ _alias    = str;	}
	void setAccount ( const char* str )			{ _account  = str;	}
	void setPhoto   ( const char* str )			{ _photo    = str;	}


protected:
	void init();

private:
	std::string	_userId;
	std::string	_protocol;	//Enum?	Jabber, AIM, MSN, Yahoo!, etc

	std::string	_alias;
	std::string _account;	//uuid?
	std::string	_photo;		//base-64 encoded

};

//=============================================================================


//=============================================================================

class ImContacts : public VoxListTemplate<ImContact>
{
public:
	void		setImContact( const char* userId, const char* protocol, const char* alias,
							  const char* account, const char* photo );
	void		setImContact( ImContact& imContact );

	ImContact* findByProtocol( const char* protocol );
	ImContact* findByUserId  ( const char* userId   );
};

//=============================================================================


//=============================================================================

class Group
{
public:
	Group();
	Group( const std::string& name, EnumGroupType::GroupType type );
	virtual ~Group();

	const char*		getKey()		const				{ return _key.c_str();		}

	const char*		getUsername()	const				{ return _username.c_str();	}
	const char*		getName()		const				{ return _name.c_str();		}
	bool			isDefault()		const				{ return _isDefault;		}
	EnumGroupType::GroupType	getType()	const		{ return _type;				}

	void setUsername ( const char* str )				{ _username  = str; updateKey();	}
	void setName	 ( const char* str )				{ _name		 = str; updateKey();	}
	void setType     ( EnumGroupType::GroupType val )	{ _type		 = val; updateKey();	}
	void setIsDefault( bool val )						{ _isDefault = val;					}

	bool isUser()	const								{ return getType() == EnumGroupType::GroupType_User;	}

	std::string typeToString();
	static std::string typeToString( EnumGroupType::GroupType eType );

	bool operator==( const Group& src );

protected:
	void init();
	void updateKey();

private:
	std::string	_username;
	std::string	_name;
	std::string _key;
	bool		_isDefault;

	EnumGroupType::GroupType _type;
};

//============================================================================

class Groups : public VoxListTemplate<Group>
{
public:
	void		setGroup( const char* name );
	std::string	getGroup( const char* name ) const;	//JRT - 2009.03.38 - Kind of worthless.
	std::string	getFirst() const;						//JRT - 2009.03.38 - Kind of worthless.

	Group* findByName( const char* name );
	Group* findFirstUserGroupInfo() const;
	std::string getFirstUserGroupKey() const;
};

//=============================================================================


//=============================================================================

class MergedContact
{
public:
	MergedContact();
	virtual ~MergedContact();

	std::string getParentUsername()		const			{ return _parentUsername;	}
	int			getParentNetworkId()	const			{ return _parentNetworkId;	}

	std::string getUsername()			const			{ return _username;			}
	int			getNetworkId()			const			{ return _networkId;		}

	void setParentUsername ( const std::string& str )	{ _parentUsername  = str;	}
	void setParentNetworkId( int val )					{ _parentNetworkId = val;	}

	void setUsername	   ( const std::string& str )	{ _username		   = str;	}
	void setNetworkId	   ( int val )					{ _networkId	   = val;	}

	bool isValid() const;

	bool operator==( const MergedContact& src );

protected:
	void init();

private:
	std::string	_parentUsername;
	int			_parentNetworkId;

	std::string	_username;
	int			_networkId;
};

//============================================================================

class MergedContacts : public VoxListTemplate<MergedContact>
{
public:
	void	set( const char* parentUsername, int parentNetworkId, const char* username, int networkId );
	void	set( const MergedContact& rMC );

	void	deleteByUsernameAndNetworkId( const char* username, int networkId );

	MergedContact* findByUsernameAndNetworkId( const char* username, int networkId );
};

//=============================================================================




//=============================================================================
//These classes are used primarily with Profiles:
//	EmailAddress, Telephone, Url (and StreetAddress) in a separate file.
//	They all contain Visibility as a data member.
//	They all represent a means of contact.
//=============================================================================

//=============================================================================

class EmailAddress
{
public:
	EmailAddress();
	virtual ~EmailAddress();

	const std::string&	getType()		const					{ return _type;			}
	const std::string&	getAddress()	const					{ return _address;		}
	bool				isPreferred()	const					{ return _isPreferred;	}
	EnumVisibility::Visibility	getVisibility() const			{ return _visibility;	}

	void setType	   ( const std::string& type )				{ _type		   = String::toLowerCase(type); }
	void setAddress	   ( const std::string& str  )				{ _address	   = str;	}
	void setIsPreferred( bool val )								{ _isPreferred = val;	}
	void setVisibility ( EnumVisibility::Visibility vis )		{ _visibility  = vis;	}

	EmailAddress&	operator= ( const EmailAddress& src );
	bool			operator==( const EmailAddress& src );
	bool			operator!=( const EmailAddress& src )		{ return !(operator==(src) );	}

	bool isValid() const;

protected:
	void init();

private:
	std::string		_type;
	std::string		_address;
	bool			_isPreferred;

	EnumVisibility::Visibility	_visibility;
};

//============================================================================


//=============================================================================

class EmailAddresses : public VoxListTemplate<EmailAddress>
{
public:
	void		setEmailAddress( const char* type, const char* address, EnumVisibility::Visibility vis = EnumVisibility::VisibilityUnknown );	//TODO - Remove default param during code integration.
	std::string	getEmailAddress( const char* type  );

	void addPersonal( const std::string& address );
	void addWork    ( const std::string& address );
	void addOther   ( const std::string& address );
};


//=============================================================================




//=============================================================================

class Telephone
{
public:
	Telephone();
	virtual ~Telephone();

	const std::string&	getType()		const					{ return _type;			}
	const std::string&	getNumber()		const					{ return _number;		}
	bool				isPreferred()	const					{ return _isPreferred;	}
	EnumVisibility::Visibility	getVisibility() const			{ return _visibility;	}

	void setType	   ( const std::string& type )				{ _type		   = String::toLowerCase( type ); 	}
	void setNumber	   ( const std::string& str  )				{ _number	   = str;	}
	void setIsPreferred( bool val )								{ _isPreferred = val;	}
	void setVisibility ( EnumVisibility::Visibility vis )		{ _visibility  = vis;	}

	Telephone& operator= ( const Telephone& src );
	bool	   operator==( const Telephone& src );
	bool	   operator!=( const Telephone& src )				{ return !(operator==(src) );	}

	bool isValid() const;

protected:
	void init();

private:
	std::string		_type;
	std::string		_number;
	bool			_isPreferred;

	EnumVisibility::Visibility	_visibility;
};

//============================================================================


//=============================================================================

class Telephones : public VoxListTemplate<Telephone>
{
public:
	void		setNumber( const char* type, const char* number, EnumVisibility::Visibility vis = EnumVisibility::VisibilityUnknown );	//TODO - Remove default param during code integration.
	std::string	getNumber( const char* type  );

	void addVoxOxNumber ( const std::string& number );
	void addMobileNumber( const std::string& number );
	void addWorkNumber  ( const std::string& number );
	void addHomeNumber  ( const std::string& number );
	void addFaxNumber   ( const std::string& number );
	void addOtherNumber ( const std::string& number );

	std::string getVoxOxNumber ();
	std::string getMobileNumber();
	std::string getWorkNumber  ();
	std::string getHomeNumber  ();
	std::string getFaxNumber   ();
	std::string getOtherNumber ();
};


//=============================================================================


//=============================================================================

class Url
{
public:
	Url();
	virtual ~Url();

	const std::string&	getType()		const					{ return _type;	}
	const std::string&	getUrl()		const					{ return _url;	}
	bool				isPreferred()	const					{ return _isPreferred;	}
	EnumVisibility::Visibility	getVisibility() const			{ return _visibility;	}

	void setType	   ( const std::string& type )				{ _type		  = String::toLowerCase( type );	}
	void setUrl		   ( const std::string& str )				{ _url		  = str;	}
	void setIsPreferred( bool val )								{ _isPreferred = val;	}
	void setVisibility ( EnumVisibility::Visibility vis )		{ _visibility = vis;	}

	Url& operator= ( const Url& src );
	bool operator==( const Url& src );
	bool operator!=( const Url& src )		{ return !(operator==(src) );	}

	bool isValid() const;

protected:
	void init();

private:
	std::string		_type;
	std::string		_url;
	bool			_isPreferred;

	EnumVisibility::Visibility	_visibility;
};

//============================================================================



//=============================================================================

class Urls : public VoxListTemplate<Url>
{
public:
	void		setUrl( const char* type, const char* url, EnumVisibility::Visibility vis = EnumVisibility::VisibilityUnknown );	//TODO - Remove default param during code integration. );
	std::string	getUrl( const char* type  );
};


//=============================================================================

//VOXOX - CJC - 2010.01.21 
class MessageTranslation
{
public:
	MessageTranslation();
	virtual ~MessageTranslation();

	const std::string&	getMessageTranslationMode()		const		{ return _MessageTranslationMode;			}
	const std::string&	getSourceMessageTranslationLanguage()	const	{ return _sourceMessageTranslationLanguage;		}
	const std::string&	getDestMessageTranslationLanguage()	const	{ return _destMessageTranslationLanguage;	}

	void setMessageTranslationMode( const std::string& MessageTranslationMode ){ _MessageTranslationMode = MessageTranslationMode; }
	void setSourceMessageTranslationLanguage( const std::string& str  ){ _sourceMessageTranslationLanguage	   = str;	}
	void setDestMessageTranslationLanguage( const std::string& str  ){ _destMessageTranslationLanguage	   = str;	}

	MessageTranslation&	operator= ( const MessageTranslation& src );
	bool			operator==( const MessageTranslation& src );
	bool			operator!=( const MessageTranslation& src )		{ return !(operator==(src) );	}

	bool isValid() const;

protected:
	void init();

private:
	std::string		_MessageTranslationMode;
	std::string		_sourceMessageTranslationLanguage;
	std::string		_destMessageTranslationLanguage;

};
#endif
