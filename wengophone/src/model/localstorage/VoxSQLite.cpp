#include "stdafx.h"	//VOXOX - JRT - 2009.04.01
#include "VoxSQLite.h"
#include "../contactlist/Contact.h"
#include <assert.h>

//-----------------------------------------------------------------------------

CVoxSQLite::CVoxSQLite()
{
}

//-----------------------------------------------------------------------------

CVoxSQLite::CVoxSQLite( const char* dbPathName )
{
	m_db.SetPathName( dbPathName );
	CreateDb();
}

//-----------------------------------------------------------------------------

CVoxSQLite::~CVoxSQLite()
{
}

//-----------------------------------------------------------------------------
 
bool CVoxSQLite::CreateDb( bool bDeleteExisting )
{
	return m_db.Create( bDeleteExisting );
}

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//Contact related
//-----------------------------------------------------------------------------

void CVoxSQLite::AddContact( Contact& rContact )
{
//	if ( ContactExists( rContact.getName().c_str() ) )
//	{
//		UpdateContact( rContact );
//	}
//	else
//	{
//		InsertContact( rContact );
//	}
//
//	AddProfile( rContact.getName().c_str(), rContact );
}

//-----------------------------------------------------------------------------

void CVoxSQLite::InsertContact( Contact& rContact )
{
	//TODO: compile as a member var?
	std::string	strSql = "INSERT INTO Contact "
						 "( [username], [nickname], [merged_parent_username]) "
						 "VALUES( ?, ?, ? ); ";

	try
	{
		CppSQLite3Statement stmt = m_db.compileStatement( strSql.c_str() );

//		stmt.bind( 1, rContact.getName().c_str()	);
//		stmt.bind( 2, rContact.getNickname().c_str());
//		stmt.bind( 3, rContact.getMergedContact().c_str() );

		stmt.execDML();		//We expect a return value == 1 (Number of rows changed);
		stmt.reset();

		int nContactId = (int)m_db.lastRowId();

//		InsertGroups	    ( rContact.getName().c_str(), rContact.getGroups()			);
//		InsertMergedContacts( rContact.getName().c_str(), rContact.getMergedContacts()	);
	}

	catch (CppSQLite3Exception& e)
	{
		e.errorCode();
	}
}

//-----------------------------------------------------------------------------

void CVoxSQLite::GetContact( const char* username, Contact& c )
{
	CppSQLite3Buffer buf;
	buf.format( "SELECT * from Contact WHERE username = %Q;", username );

	try
	{
		CppSQLite3Statement stmt = m_db.compileStatement( (const char*)buf );

		int	nContactId = 0;
		CppSQLite3Query q = stmt.execQuery();

		//Process record set.
        while (!q.eof())
        {
			nContactId = q.getIntField(0);

//			c.setName		( q.getStringField(1) );
//			c.setNickname	( q.getStringField(2) );
//			c.setBirthday	( q.getStringField(3) );
//			c.setMergedContact( q.getStringField(3) );

			GetMergedContacts( username, c.getMergedContacts() );
			GetGroups		 ( username, c.getGroups() );
			GetProfile		 ( username, c );

			q.nextRow();
        }

		stmt.reset();
	}

	catch (CppSQLite3Exception& e)
	{
		e.errorCode();
	}
}

//-----------------------------------------------------------------------------

void CVoxSQLite::UpdateContact( Contact& rContact )
{
	assert(false);
}

//-----------------------------------------------------------------------------

bool CVoxSQLite::ContactExists( const char* username )
{
	int nCount = 0;

	try
	{
		CppSQLite3Buffer buf;
		buf.format( "SELECT count(*) from Contact WHERE username = %Q", username );
		nCount = m_db.execScalar( (const char*)buf );
	}

	catch (CppSQLite3Exception& e)
	{
		e.errorCode();
	}

	return (nCount > 0);
}

//-----------------------------------------------------------------------------
//End contact related
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//MergedContact
//-----------------------------------------------------------------------------

void CVoxSQLite::AddMergedContact( const char* strParentUsername, const char* strUsername )
{
	if ( MergedContactExists( strParentUsername, strUsername ) )
	{
		UpdateMergedContact( strParentUsername, strUsername );
	}
	else
	{
		InsertMergedContact( strParentUsername, strUsername );
	}
}

//------------------------------------------------------------------------------

void CVoxSQLite::InsertMergedContacts( const char* strContactUsername, MergedContacts& rList )
{
	for ( MergedContacts::iterator iter = rList.begin(); iter != rList.end(); iter++ )
	{
		InsertMergedContact( strContactUsername, (*iter) );
	}
}

//-----------------------------------------------------------------------------

void CVoxSQLite::InsertMergedContact( const char* strContactUsername, MergedContact& mc )
{
//	InsertMergedContact( strContactUsername, mc.getUsername() );
}

//-----------------------------------------------------------------------------

void CVoxSQLite::InsertMergedContact( const char* strContactUsername, const char* strUsername )
{
	//TODO: compile as a member var?
	std::string	strSql = "INSERT INTO [MergedContact] "
						 "( [parent_username], [username] ) "
						 "VALUES( ?, ? ); ";

	try
	{
		CppSQLite3Statement stmt = m_db.compileStatement( strSql.c_str() );

		stmt.bind( 1, strContactUsername );
		stmt.bind( 2, strUsername );

		stmt.execDML();		//We expect a return value == 1 (Number of rows changed);
		stmt.reset();

		int nId = (int)m_db.lastRowId();
	}

	catch (CppSQLite3Exception& e)
	{
		e.errorCode();
	}
}

//-----------------------------------------------------------------------------

void CVoxSQLite::UpdateMergedContact( const char* strContactUsername, const char* strUsername )
{
	assert(false);
}

//-----------------------------------------------------------------------------

bool CVoxSQLite::MergedContactExists( const char* strParentUsername, const char* strUsername )
{
	int nCount = 0;

	try
	{
		CppSQLite3Buffer buf;
		buf.format( "SELECT count(*) from [MergedContact] WHERE [parent_username] = %Q AND [username] = %Q", strParentUsername, strUsername );

		nCount = m_db.execScalar( (const char*)buf );
	}

	catch (CppSQLite3Exception& e)
	{
		e.errorCode();
	}

	return (nCount > 0);
}

//-----------------------------------------------------------------------------

void CVoxSQLite::GetMergedContacts( const char* parent_username, MergedContacts& rList )
{
	CppSQLite3Buffer buf;
	buf.format( "SELECT * from [MergedContact] WHERE [parent_username] = %Q;", parent_username );

	try
	{
		CppSQLite3Statement stmt = m_db.compileStatement( (const char*)buf );

		int				nContactId	= 0;
		std::string		type		= "";
		CppSQLite3Query q			= stmt.execQuery();
		MergedContact	mc;

		//Process record set.
        while (!q.eof())
        {
			mc.setParentUsername( q.getStringField( 0 ) );
			mc.setUsername      ( q.getStringField( 1 ) );

			rList.Add( &mc );

			q.nextRow();
        }

		stmt.reset();
	}

	catch (CppSQLite3Exception& e)
	{
		e.errorCode();
	}
}

//-----------------------------------------------------------------------------
//End MergedContact
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
//Group related
//------------------------------------------------------------------------------

void CVoxSQLite::InsertGroups( const char* username, Groups& rList )
{
	for ( Groups::iterator iter = rList.begin(); iter != rList.end(); iter++ )
	{
		InsertGroup( username, (*iter) );
	}
}

//-----------------------------------------------------------------------------

void CVoxSQLite::InsertGroup( const char* username, Group& rGroup )
{
	//TODO: compile as a member var?
	std::string	strSql = "INSERT INTO [Group] "
						 "( [username], [name] ) "
						 "VALUES( ?, ? ); ";

	try
	{
		CppSQLite3Statement stmt = m_db.compileStatement( strSql.c_str() );

		stmt.bind( 1, username );
		stmt.bind( 2, rGroup.getName() );

		stmt.execDML();		//We expect a return value == 1 (Number of rows changed);
		stmt.reset();

		int nId = (int)m_db.lastRowId();
	}

	catch (CppSQLite3Exception& e)
	{
		e.errorCode();
	}
}

//-----------------------------------------------------------------------------

void CVoxSQLite::UpdateGroup( const char* strContactUsername, const char* strUsername )
{
	assert(false);
}

//-----------------------------------------------------------------------------

bool CVoxSQLite::GroupExists( const char* strContactUsername, const char* strName )
{
	int nCount = 0;

	try
	{
		CppSQLite3Buffer buf;
		buf.format( "SELECT count(*) from [Group] WHERE [username] = %Q AND [name] = %Q", strContactUsername, strName );

		nCount = m_db.execScalar( (const char*)buf );
	}

	catch (CppSQLite3Exception& e)
	{
		e.errorCode();
	}

	return (nCount > 0);
}

//-----------------------------------------------------------------------------

void CVoxSQLite::GetGroups( const char* username, Groups& rList )
{
	CppSQLite3Buffer buf;
	buf.format( "SELECT * from [Group] WHERE [username] = %Q;", username );

	try
	{
		CppSQLite3Statement stmt = m_db.compileStatement( (const char*)buf );

		int				nContactId	= 0;
		std::string		type		= "";
		CppSQLite3Query q			= stmt.execQuery();
		Group			grp;

		//Process record set.
        while (!q.eof())
        {
			grp.setUsername( q.getStringField( 0 ) );
			grp.setName    ( q.getStringField( 1 ) );

			rList.Add( &grp );

			q.nextRow();
        }

		stmt.reset();
	}

	catch (CppSQLite3Exception& e)
	{
		e.errorCode();
	}
}

//-----------------------------------------------------------------------------
//End Group related
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
//Profile related
//-----------------------------------------------------------------------------

void CVoxSQLite::AddProfile( const char* username, Profile& rProfile )
{
	if ( ProfileExists( username ) )
	{
		UpdateProfile( username, rProfile );
	}
	else
	{
		InsertProfile( username, rProfile );
	}
}

//-----------------------------------------------------------------------------

void CVoxSQLite::InsertProfile( const char* username, Profile& rProfile )
{
	//TODO: compile as a member var?
	std::string	strSql = "INSERT INTO Profile "
						 "( [username], [first_name], [last_name], [alias], [sms_signature], [company], [notes], [sex] ) "
//						 "[birthday], [photo] ) "		//TODO
						 "VALUES( ?, ?, ?, ?, ?, ?, ?, ? ); ";

	try
	{
		CppSQLite3Statement stmt = m_db.compileStatement( strSql.c_str() );

		stmt.bind( 1, username	);
		stmt.bind( 2, rProfile.getFirstName().c_str()	);
		stmt.bind( 3, rProfile.getLastName().c_str() 	);
		stmt.bind( 4, rProfile.getAlias().c_str()		);
		stmt.bind( 5, rProfile.getSmsSignature().c_str());
		stmt.bind( 6, rProfile.getCompany().c_str()		);
		stmt.bind( 7, rProfile.getNotes().c_str()		);
		stmt.bind( 8, rProfile.getSex()					);
//		stmt.bind( 10, rProfile.getBirthday()			);
//		stmt.bind( 11, rProfile.getPhoto()				);

		stmt.execDML();		//We expect a return value == 1 (Number of rows changed);
		stmt.reset();

		int nId = (int)m_db.lastRowId();

		InsertUrls			 ( nId, rProfile.getUrls()			  );
		InsertStreetAddresses( nId, rProfile.getStreetAddresses() );
		InsertEmailAddresses ( nId, rProfile.getEmailAddresses()  );
		InsertTelephones	 ( nId, rProfile.getTelephones()	  );
	}

	catch (CppSQLite3Exception& e)
	{
		e.errorCode();
	}
}

//-----------------------------------------------------------------------------

void CVoxSQLite::UpdateProfile( const char* username, Profile& rProfile )
{
	assert(false);
}

//-----------------------------------------------------------------------------

bool CVoxSQLite::ProfileExists( const char* username )
{
	int nCount = 0;

	try
	{
		CppSQLite3Buffer buf;
		buf.format( "SELECT count(*) from Profile WHERE username = %Q", username );

		nCount = m_db.execScalar( (const char*)buf );
	}

	catch (CppSQLite3Exception& e)
	{
		e.errorCode();
	}

	return (nCount > 0);
}

//-----------------------------------------------------------------------------

void CVoxSQLite::GetProfile( const char* username, Profile& rProfile )
{
	CppSQLite3Buffer buf;
	buf.format( "SELECT * from Profile WHERE username = %Q;", username );

	try
	{
		CppSQLite3Statement stmt = m_db.compileStatement( (const char*)buf );

		int	nProfileId	  = 0;
		CppSQLite3Query q = stmt.execQuery();

		//Process record set.
        while (!q.eof())
        {
			nProfileId = q.getIntField(0);

			rProfile.setFirstName	( q.getStringField(2) );
			rProfile.setLastName	( q.getStringField(3) );
			rProfile.setAlias		( q.getStringField(4) );
			rProfile.setSmsSignature( q.getStringField(5) );
			rProfile.setCompany		( q.getStringField(6) );
			rProfile.setSex			( (EnumSex::Sex)q.getIntField(7)	  );
			rProfile.setNotes		( q.getStringField(10) );
//			rProfile.setBirthday( q.getStringField(8) );
//			photo (filename)
//TODO		rProfile.setPhoto( q.getBlobField(9) );
			
			//Address, email, phone numbers, and URLs will be in separate tables to allow for easy
			GetStreetAddresses( nProfileId, rProfile.getStreetAddresses() );
			GetEmailAddresses ( nProfileId, rProfile.getEmailAddresses()  );
			GetTelephones	  ( nProfileId, rProfile.getTelephones()	  );
			GetUrls			  ( nProfileId, rProfile.getUrls()			  );

			q.nextRow();
        }

		stmt.reset();
	}

	catch (CppSQLite3Exception& e)
	{
		e.errorCode();
	}
}

//-----------------------------------------------------------------------------
//End profile related
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//StreetAddresses
//-----------------------------------------------------------------------------

void CVoxSQLite::InsertStreetAddresses( int nId, StreetAddresses& rList )
{
	for ( StreetAddresses::iterator iter = rList.begin(); iter != rList.end(); iter++ )
	{
		InsertStreetAddress( nId, (*iter) );
	}
}

//-----------------------------------------------------------------------------

void CVoxSQLite::InsertStreetAddress( int nId, StreetAddress& addr )
{
	std::string	strSql = "INSERT INTO StreetAddress "
		"( [profile_id], [type], [street], [locality], [region], [postcode], [country], [visibility] ) "
						 "VALUES( ?, ?, ?, ?, ?, ?, ?, ? ); ";

	try
	{
		//TODO: compile as a member var?
		CppSQLite3Statement stmt = m_db.compileStatement( strSql.c_str() );

		stmt.bind( 1, nId	);
		stmt.bind( 2, addr.getType().c_str()		);
		stmt.bind( 3, addr.getStreet1().c_str() 	);
		stmt.bind( 4, addr.getCity().c_str()		);
		stmt.bind( 5, addr.getStateProvince().c_str());
		stmt.bind( 6, addr.getPostalCode().c_str()	);
		stmt.bind( 7, addr.getCountry().c_str()		);
		stmt.bind( 8, (int) addr.getVisibility()	);

		stmt.execDML();		//We expect a return value == 1 (Number of rows changed);
		stmt.reset();
	}

	catch (CppSQLite3Exception& e)
	{
		e.errorCode();
	}
}

//-----------------------------------------------------------------------------

void CVoxSQLite::GetStreetAddresses( int nProfileId, StreetAddresses& rAddrs )
{
	CppSQLite3Buffer buf;
	buf.format( "SELECT * from StreetAddress WHERE profile_id = %d;", nProfileId );

	try
	{
		CppSQLite3Statement stmt = m_db.compileStatement( (const char*)buf );

		int				nId		= 0;
		CppSQLite3Query q		= stmt.execQuery();

		StreetAddress	addr;

		//Process record set.
        while (!q.eof())
        {
//			"( [profile_id], [type], [street], [locality], [region], [postcode], [country], [visibility] ) "
			nId  = q.getIntField(0);			//Not part of Profile.

			addr.setType		 ( q.getStringField(1) );
			addr.setStreet1		 ( q.getStringField(2) );
			addr.setCity		 ( q.getStringField(3) );
			addr.setStateProvince( q.getStringField(4) );
			addr.setPostalCode	 ( q.getStringField(5) );
			addr.setCountry		 ( q.getStringField(6) );
			addr.setVisibility   ( (EnumVisibility::Visibility)q.getIntField(7) );

			rAddrs.Add( &addr );

			q.nextRow();
        }

		stmt.reset();
	}

	catch (CppSQLite3Exception& e)
	{
		e.errorCode();
	}
}

//-----------------------------------------------------------------------------
//End StreetAddresses
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//EmailAddresses
//-----------------------------------------------------------------------------

void CVoxSQLite::InsertEmailAddresses( int nId, EmailAddresses& rList )
{
	for ( EmailAddresses::iterator iter = rList.begin(); iter != rList.end(); iter++ )
	{
		InsertEmailAddress( nId, (*iter) );
	}
}

//-----------------------------------------------------------------------------

void CVoxSQLite::InsertEmailAddress( int nId, EmailAddress& rAddr )
{
	InsertEmailAddress( nId, rAddr.getType(), rAddr.getAddress(), rAddr.getVisibility() );
}

//-----------------------------------------------------------------------------

void CVoxSQLite::InsertEmailAddress( int nId, const std::string& type, const std::string& email, EnumVisibility::Visibility vis )
{
	if ( IsValidEmailAddress( email.c_str() ) )
	{
		std::string	strSql = "INSERT INTO EmailAddress "
			"( [profile_id], [type], [email], [visibility] ) "
							 "VALUES( ?, ?, ?, ? ); ";
		try
		{
			//TODO: compile as a member var?
			CppSQLite3Statement stmt = m_db.compileStatement( strSql.c_str() );

			stmt.bind( 1, nId			);
			stmt.bind( 2, type.c_str()	);
			stmt.bind( 3, email.c_str()	);
			stmt.bind( 4, (int)vis );

			stmt.execDML();		//We expect a return value == 1 (Number of rows changed);
			stmt.reset();
		}

		catch (CppSQLite3Exception& e)
		{
			e.errorCode();
		}
	}
}

//-----------------------------------------------------------------------------

bool CVoxSQLite::IsValidEmailAddress( const char* email )
{
	bool bValid = false;

	std::string str = email;

	if ( str.length() > 0 )
	{
		bValid = true;
	}

	return bValid;
}

//-----------------------------------------------------------------------------

void CVoxSQLite::GetEmailAddresses( int nProfileId, EmailAddresses& rList )
{
	CppSQLite3Buffer buf;
	buf.format( "SELECT * from EmailAddress WHERE profile_id = %d;", nProfileId );

	try
	{
		CppSQLite3Statement stmt = m_db.compileStatement( (const char*)buf );

		int				nProfileId	= 0;
		std::string		type		= "";
		EmailAddress	addr;
		CppSQLite3Query q			= stmt.execQuery();

		//Process record set.
        while (!q.eof())
        {
//			"( [profile_id], [type], [email], [visiblity]
			nProfileId = q.getIntField(0);		//Not part of Profile.
			type = q.getStringField( 1 );		//Use to determine which address to update.

			addr.setType	  ( q.getStringField(1) );
			addr.setAddress	  ( q.getStringField(2) );
			addr.setVisibility( (EnumVisibility::Visibility) q.getIntField(3)	);

			rList.Add( &addr );

			q.nextRow();
        }

		stmt.reset();
	}

	catch (CppSQLite3Exception& e)
	{
		e.errorCode();
	}
}

//-----------------------------------------------------------------------------
//End EmailAddresses
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//Telephones
//-----------------------------------------------------------------------------

void CVoxSQLite::InsertTelephones( int nId, Telephones& rList )
{
	for ( Telephones::iterator iter = rList.begin(); iter != rList.end(); iter++ )
	{
		InsertTelephone( nId, (*iter) );
	}
}

//-----------------------------------------------------------------------------

void CVoxSQLite::InsertTelephone( int nId, const Telephone rTel)
{
	InsertTelephone( nId, rTel.getType(), rTel.getNumber(), rTel.getVisibility() );
}

//-----------------------------------------------------------------------------

void CVoxSQLite::InsertTelephone ( int nId, const std::string& type, const std::string& telephone, EnumVisibility::Visibility vis )
{
	if ( IsValidTelephone( telephone.c_str() ) )
	{
		std::string	strSql = "INSERT INTO Telephone "
			"( [profile_id], [type], [number], [visibility] ) "
							 "VALUES( ?, ?, ?, ? ); ";

		try
		{
			//TODO: compile as a member var?
			CppSQLite3Statement stmt = m_db.compileStatement( strSql.c_str() );

			stmt.bind( 1, nId			);
			stmt.bind( 2, type.c_str()	);
			stmt.bind( 3, telephone.c_str() );
			stmt.bind( 4, (int)vis  );

			stmt.execDML();		//We expect a return value == 1 (Number of rows changed);
			stmt.reset();
		}

		catch (CppSQLite3Exception& e)
		{
			e.errorCode();
		}
	}
}

//-----------------------------------------------------------------------------

bool CVoxSQLite::IsValidTelephone( const char* telephone )
{
	bool bValid = false;

	std::string str = telephone;

	if ( str.length() > 0 )
	{
		bValid = true;
	}

	return bValid;
}

//-----------------------------------------------------------------------------

void CVoxSQLite::GetTelephones( int nProfileId, Telephones& rList )
{
	CppSQLite3Buffer buf;
	buf.format( "SELECT * from Telephone WHERE profile_id = %d;", nProfileId );

	try
	{
		CppSQLite3Statement stmt = m_db.compileStatement( (const char*)buf );

		int				nProfileId	= 0;
		std::string		type		= "";
		CppSQLite3Query q			= stmt.execQuery();
		Telephone		tel;

		//Process record set.
        while (!q.eof())
        {
//			"( [profile_id], [type], [number]
			nProfileId = q.getIntField(0);		//Not part of Profile.

			tel.setType  ( q.getStringField( 1 ) );
			tel.setNumber( q.getStringField( 2 ) );

			rList.Add( &tel );

			q.nextRow();
        }

		stmt.reset();
	}

	catch (CppSQLite3Exception& e)
	{
		e.errorCode();
	}
}

//-----------------------------------------------------------------------------
//End Telephones
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
//Urls
//-----------------------------------------------------------------------------

void CVoxSQLite::InsertUrls( int nId, const Urls& rList )
{
	for ( Urls::const_iterator iter = rList.begin(); iter != rList.end(); iter++ )
	{
		InsertUrl( nId, (*iter) );
	}
}

//-----------------------------------------------------------------------------

void CVoxSQLite::InsertUrl( int nId, const Url& rUrl )
{
	InsertUrl( nId, rUrl.getType(), rUrl.getUrl(), rUrl.getVisibility() );
}

//-----------------------------------------------------------------------------

void CVoxSQLite::InsertUrl ( int nId, const std::string& type, const std::string& url, EnumVisibility::Visibility vis )
{
	if ( IsValidUrl( url.c_str() ) )
	{
		std::string	strSql = "INSERT INTO Url "
			"( [profile_id], [type], [url], [visibility] ) "
							 "VALUES( ?, ?, ?, ? ); ";

		try
		{
			//TODO: compile as a member var?
			CppSQLite3Statement stmt = m_db.compileStatement( strSql.c_str() );

			stmt.bind( 1, nId			);
			stmt.bind( 2, type.c_str()	);
			stmt.bind( 3, url.c_str()   );
			stmt.bind( 4, (int)vis		);

			stmt.execDML();		//We expect a return value == 1 (Number of rows changed);
			stmt.reset();
		}

		catch (CppSQLite3Exception& e)
		{
			e.errorCode();
		}
	}
}

//-----------------------------------------------------------------------------

bool CVoxSQLite::IsValidUrl( const char* url )
{
	bool bValid = false;

	std::string str = url;

	if ( str.length() > 0 )
	{
		bValid = true;
	}

	return bValid;
}

//-----------------------------------------------------------------------------

void CVoxSQLite::GetUrls( int nProfileId, Urls& rList )
{
	CppSQLite3Buffer buf;
	buf.format( "SELECT * from Url WHERE profile_id = %d;", nProfileId );

	try
	{
		CppSQLite3Statement stmt = m_db.compileStatement( (const char*)buf );

		int				nProfileId	= 0;
		std::string		type		= "";
		CppSQLite3Query q			= stmt.execQuery();
		Url				url;

		//Process record set.
        while (!q.eof())
        {
//			"( [profile_id], [type], [address]
			nProfileId = q.getIntField(0);		//Not part of Profile.

			url.setType   ( q.getStringField( 1 ) );
			url.setUrl    ( q.getStringField( 2 ) );
			url.setVisibility( (EnumVisibility::Visibility)q.getIntField(3) );

			rList.Add( &url );

			q.nextRow();
        }

		stmt.reset();
	}

	catch (CppSQLite3Exception& e)
	{
		e.errorCode();
	}
}

//-----------------------------------------------------------------------------
//End URLs
//-----------------------------------------------------------------------------
