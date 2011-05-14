#ifndef _VOXSQLLITE_H_
#define _VOXSQLLITE_H_

#include "VoxClientDb.h"

#include "../contactlist/ContactList.h"
#include "../profile/Profile.h"
#include "../profile/Components.h"
#include "../profile/StreetAddress.h"

#include <string>

class CVoxSQLite
{
public:
	CVoxSQLite();
	CVoxSQLite( const char* strDbPathName );
	virtual ~CVoxSQLite();

	bool CreateDb( bool bDeleteExisting = false );
	void SetPathName( const char* str )		{ m_db.SetPathName( str );	}

	const char* getVersion()				{ return m_db.SQLiteVersion();	}

	//Contact
	void AddContact( Contact& rContact );
	void GetContact( const char* uuid, Contact& c );
	int  DeleteAllContacts()				{ return m_db.DeleteAllFromTable ( "Contact" ); }
	int	 GetContactCount()					{ return m_db.GetTableRecordCount( "Contact" ); };

	//Profile
	void AddProfile( const char* uuid, Profile& rProfile );
	void GetProfile( const char* uuid, Profile& rProfile );
	int  DeleteAllProfiles()				{ return m_db.DeleteAllFromTable ( "Profile" ); }
	int	 GetProfileCount()					{ return m_db.GetTableRecordCount( "Profile" ); };

	//MergedContact
	void AddMergedContact   ( const char* strParentUuid, const char* strName );

	//Lists
	int	 GetAddressCount()					{ return m_db.GetTableRecordCount( "StreetAddress"   ); };
	int	 GetEmailCount()					{ return m_db.GetTableRecordCount( "EmailAddress"	); };
	int	 GetTelephoneCount()				{ return m_db.GetTableRecordCount( "Telephone"		); };
	int	 GetUrlCount()						{ return m_db.GetTableRecordCount( "Url"				); };

protected:
	//Contact related
	bool ContactExists( const char* uuid );
	void InsertContact( Contact& rContact );
	void UpdateContact( Contact& rContact );

	//-------------------------------------------------------------------------
	//These are part of Contact
	//-------------------------------------------------------------------------
	//MergedContact related
	bool MergedContactExists ( const char* username, const char* strName );
	void InsertMergedContacts( const char* username, MergedContacts& rList );
	void InsertMergedContact ( const char* username, MergedContact&  mc  );
	void InsertMergedContact ( const char* username, const char* strName );
	void UpdateMergedContact ( const char* username, const char* strName );
	void GetMergedContacts   ( const char* username, MergedContacts& rList );

	//ImContact related
	bool ImContactExists ( const char* username );
	void InsertImcontacts( int nId, ImContacts& rList );
	void InsertImContact ( const char* username, ImContact& rImcontact );
	void UpdateImContact ( const char* username, ImContact& rImContact );

	//Group related
	bool GroupExists ( const char* username, const char* name );
	void InsertGroups( const char* username, Groups& grp );
	void InsertGroup ( const char* username, Group&  grp );
	void UpdateGroup ( const char* username, const char* name );
	void GetGroups   ( const char* parent_username, Groups& rList );

	//Profile related - DO we want Contact_id (vs. uuid)?
	bool ProfileExists( const char* username );
	void InsertProfile( const char* username, Profile& rProfile );
	void UpdateProfile( const char* username, Profile& rProfile );


	//---------------------------------------------------------------------
	//These are part of profile
	//---------------------------------------------------------------------
	//Address related
	void InsertStreetAddresses( int nId, StreetAddresses& rAddrs );
	void InsertStreetAddress  ( int nId, StreetAddress&   rAddr  );
	void GetStreetAddresses   ( int nId, StreetAddresses& rAddrs );

	//Email related
	void InsertEmailAddresses( int nId, EmailAddresses& rEmailAddrs );
	void InsertEmailAddress  ( int nId, EmailAddress& rAddr );
	void InsertEmailAddress  ( int nId, const std::string& type, const std::string& email, EnumVisibility::Visibility vis );
	bool IsValidEmailAddress ( const char* email );
	void GetEmailAddresses   ( int nId, EmailAddresses& rEmailAddrs );

	//Telephone related
	void InsertTelephones( int nId, Telephones& rTelephones );
	void InsertTelephone ( int nId, const Telephone rTel );
	void InsertTelephone ( int nId, const std::string& type, const std::string& telephone, EnumVisibility::Visibility vis );
	bool IsValidTelephone( const char* telephone );
	void GetTelephones   ( int nId, Telephones& rTelephones );

	//URL related related
	void InsertUrls( int nId, const Urls& rUrls );
	void InsertUrl ( int nId, const Url&  rUrl  );
	void InsertUrl ( int nId, const std::string& type, const std::string& url, EnumVisibility::Visibility vis );
	bool IsValidUrl( const char* url );
	void GetUrls   ( int nId, Urls& rUrls );

private:
	CVoxClientDb m_db;
};

#endif