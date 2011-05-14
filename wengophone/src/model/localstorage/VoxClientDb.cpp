#include "stdafx.h"	//VOXOX - JRT - 2009.04.01
#include "VoxClientDb.h"
#include <assert.h>

//-----------------------------------------------------------------------------

CVoxClientDb::CVoxClientDb()
{
	m_strDbPathName = "";
}

//-----------------------------------------------------------------------------

CVoxClientDb::CVoxClientDb( const char* dbPathName )
{
	m_strDbPathName = dbPathName;
	Create();
}

//-----------------------------------------------------------------------------

CVoxClientDb::~CVoxClientDb()
{
}

//-----------------------------------------------------------------------------
 
bool CVoxClientDb::Create( bool bDeleteExisting )
{
	bool bSuccess = false;

	if ( bDeleteExisting )
	{
		remove( m_strDbPathName.c_str() );		//TODO: how to handle re-creation/destruction.
	}

	open( m_strDbPathName.c_str() );

	CreateTables();
	CreateIndexes();
	CreateTriggers();

	return bSuccess;
}

//-----------------------------------------------------------------------------

void CVoxClientDb::CreateTables()
{
	CreateTable_Contact();
	CreateTable_ImContact();
	CreateTable_MergedContact();
	CreateTable_Group();

	CreateTable_Profile();
	CreateTable_StreetAddress();
	CreateTable_EmailAddress();
	CreateTable_Telephone();
	CreateTable_Url();

	CreateTable_History();
	CreateTable_Settings();
}

//-----------------------------------------------------------------------------

void CVoxClientDb::CreateIndexes()
{
	std::string strSql = "";
	
	try
	{
		//Contact
		strSql = "CREATE UNIQUE INDEX Contact_username on [Contact] ( [username] )";
		execDML( strSql.c_str() );

		//--------------------------------------
		//These associate with Contact
		//--------------------------------------
		//Profile
		strSql = "CREATE UNIQUE INDEX Profile_username on [Profile] ( [username] )";
		execDML( strSql.c_str() );

		//MergedContact
		strSql = "CREATE UNIQUE INDEX MergedContact_parent_username on [MergedContact] ( [parent_username], [username] )";
		execDML( strSql.c_str() );

		//Group
		strSql = "CREATE UNIQUE INDEX Group_parent_username on [Group] ( [username], [name] )";
		execDML( strSql.c_str() );

		//ImContact
		strSql = "CREATE INDEX ImContact_username on [ImContact] ( [username] )";
		execDML( strSql.c_str() );

		//-------------------------------------
		//These associate with Profile
		//-------------------------------------
		//StreetAddress
		strSql = "CREATE INDEX Address_id_type on [StreetAddress] ( [profile_id], [type] )";
		execDML( strSql.c_str() );

		//EmailAddress
		strSql = "CREATE INDEX Email_id_type on [EmailAddress] ( [profile_id], [type] )";
		execDML( strSql.c_str() );

		//Telephone
		strSql = "CREATE INDEX Telephone_id_type on [Telephone] ( [profile_id], [type] )";
		execDML( strSql.c_str() );

		//Url
		strSql = "CREATE INDEX Url_id_type on [Url] ( [profile_id], [type] )";
		execDML( strSql.c_str() );
	}

	catch (CppSQLite3Exception& e)
	{
		e.errorCode();
	}
}

//-----------------------------------------------------------------------------

void CVoxClientDb::CreateTriggers()
{
	std::string strSql = "";
	
	try
	{
		//Contact
		//Delete profiles
		strSql = "CREATE TRIGGER Contact_Delete_Profile "
				"BEFORE DELETE ON [Contact] "
				 "FOR EACH ROW BEGIN "
				 "DELETE FROM [Profile] WHERE [username] = OLD.[username]; "
				 "END; ";

		execDML( strSql.c_str() );

		//Delete Groups								//TODO: Do we want groups on Contacts or Profiles?
		strSql = "CREATE TRIGGER Contact_Delete_VGroup "
			"BEFORE DELETE ON [Contact] "
				 "FOR EACH ROW BEGIN "
				 "DELETE FROM [Group] WHERE [username] = OLD.[username]; "
				 "END; ";

		execDML( strSql.c_str() );

		//Delete MergedContacts
		strSql = "CREATE TRIGGER Contact_Delete_MergedContact "
				 "BEFORE DELETE ON [Contact] "
				 "FOR EACH ROW BEGIN "
				 "DELETE FROM [MergedContact] WHERE [parent_username] = OLD.[username]; "
				 "END; ";

		execDML( strSql.c_str() );

		//ImContact
//		execDML( strSql.c_str() );


		//Profile
		//Delete Street Addresses
		strSql = "CREATE TRIGGER Profile_Delete_StreetAddress "
				 "BEFORE DELETE ON [Profile] "
				 "FOR EACH ROW BEGIN "
				 "DELETE FROM [StreetAddress] WHERE [profile_id] = OLD.[profile_id]; "
				 "END; ";

		execDML( strSql.c_str() );

		//Delete Email Addresses
		strSql = "CREATE TRIGGER Profile_Delete_EmailAddress "
				 "BEFORE DELETE ON [Profile] "
				 "FOR EACH ROW BEGIN "
				 "DELETE FROM [EmailAddress] WHERE [profile_id] = OLD.[profile_id]; "
				 "END; ";

		execDML( strSql.c_str() );

		//Delete telephones
		strSql = "CREATE TRIGGER Profile_Delete_Telephone "
				 "BEFORE DELETE ON [Profile] "
				 "FOR EACH ROW BEGIN "
				 "DELETE FROM [Telephone] WHERE [profile_id] = OLD.[profile_id]; "
				 "END; ";

		execDML( strSql.c_str() );

		//Delete URLs
		strSql = "CREATE TRIGGER Profile_Delete_Url "
				 "BEFORE DELETE ON [Profile] "
				 "FOR EACH ROW BEGIN "
				 "DELETE FROM [Url] WHERE [profile_id] = OLD.[profile_id]; "
				 "END; ";

		execDML( strSql.c_str() );


	}

	catch (CppSQLite3Exception& e)
	{
		e.errorCode();
	}
}

//-----------------------------------------------------------------------------

void CVoxClientDb::CreateTable_Contact()
{
	std::string strFields = "";

	strFields += "[contact_id] INTEGER PRIMARY KEY, ";	//Key into IMContact, Profile. name to Account?
//	strFields += "[uuid]     TEXT, ";		//Key into IMContact, Profile.

	strFields += "[username] TEXT, ";		//Key into IMContact, Profile.
	strFields += "[nickname] TEXT, ";		//Jabber standard, I think.
//	strFields += "[group]    TEXT, ";		//cdata - should this be blob/binary?  //Jabber standard allows for multiple groups, I think.
	strFields += "[merged_parent_username] TEXT ";	//This will be UserName)

	CreateTable( "Contact", strFields.c_str() );
}

//-----------------------------------------------------------------------------

void CVoxClientDb::CreateTable_MergedContact()
{
	std::string strFields = "";

	strFields += "[parent_username] TEXT, ";	//Unique key on parent_username + username
	strFields += "[username] TEXT ";						

	CreateTable( "MergedContact", strFields.c_str() );
}

//-----------------------------------------------------------------------------

void CVoxClientDb::CreateTable_ImContact()
{
	std::string strFields = "";

	strFields += "[username]		TEXT, ";	//Key into IMContact, Profile. name to Account?

	strFields += "[protocol] TEXT, ";
	strFields += "[id]		 TEXT, ";
	strFields += "[alias]	 TEXT, ";	//cdata - same as nickcname
	strFields += "[photo]	 BLOB  ";	//base-64 encoded, unlimited length.

	CreateTable( "ImContact", strFields.c_str() );
}

//-----------------------------------------------------------------------------

void CVoxClientDb::CreateTable_Group()
{
	std::string strFields = "";

	strFields += "[username] TEXT NOT NULL, ";		//Key back to Contact
	strFields += "[name]	 TEXT ";				//Unique key on username + name

	CreateTable( "Group", strFields.c_str() );
}

//-----------------------------------------------------------------------------

void CVoxClientDb::CreateTable_Profile()
{
	std::string strFields = "";

	strFields += "[profile_id]	INTEGER PRIMARY KEY, ";	//Key for Telephone, email, addresses.
	strFields += "[username]		TEXT, ";			//Foreign key to Contact.  (we may merge Profile and Contact)

	strFields += "[first_name]		TEXT, ";
	strFields += "[last_name]		TEXT, ";
	strFields += "[alias]			TEXT, ";
	strFields += "[sms_signature]	TEXT, ";	//Not std part of VCard.
	strFields += "[company]			TEXT, ";	//Not std part of VCard.
	strFields += "[sex]			 INTEGER, ";	//Enum
	strFields += "[birthday]	 TEXT, ";

//	photo (filename)
	strFields += "[photo]		BLOB, ";	//base-64 encoded, unlimited length.
											//Do we need filename as separate field?
	strFields += "[notes]		TEXT ";

	//Address, email phone numbers, and urls will be in separate tables to allow for easy
	//	addition of new 'type' without altering table.

	CreateTable( "Profile", strFields.c_str() );
}

//-----------------------------------------------------------------------------

void CVoxClientDb::CreateTable_StreetAddress()
{
	std::string strFields = "";

	strFields += "[profile_id] INTEGER NOT NULL, ";		//Key back to Profile
	strFields += "[type]		 TEXT NOT NULL, ";	//Profile_id + type create unique key.

	strFields += "[street]     TEXT, ";
	strFields += "[locality]   TEXT, ";
	strFields += "[region]     TEXT, ";
	strFields += "[postcode]   TEXT, ";
	strFields += "[country]	   TEXT,  ";

	strFields += "[visibility]	INTEGER ";

	CreateTable( "StreetAddress", strFields.c_str() );
}

//-----------------------------------------------------------------------------

void CVoxClientDb::CreateTable_EmailAddress()
{
	//email (home, work other)  user defined?
	std::string strFields = "";

	strFields += "[profile_id] INTEGER NOT NULL, ";	//Key back to Profile
	strFields += "[type]		 TEXT NOT NULL, ";		//Profile_id + type create unique key.

	strFields += "[email]	 TEXT, ";
	strFields += "[visibility]	INTEGER ";

	CreateTable( "EmailAddress", strFields.c_str() );
}

//-----------------------------------------------------------------------------

void CVoxClientDb::CreateTable_Telephone()
{
	//tel (home, work, cell, voxox) add sip? skype? user-defined
	std::string strFields = "";

	strFields += "[profile_id] INTEGER NOT NULL, ";	//Key back to Profile
	strFields += "[type]		 TEXT NOT NULL, ";	//Profile_id + type create unique key.

	strFields += "[number]	 TEXT, ";
	strFields += "[visibility]	INTEGER ";

	CreateTable( "Telephone", strFields.c_str() );
}

//-----------------------------------------------------------------------------

void CVoxClientDb::CreateTable_Url()
{
	std::string strFields = "";

	strFields += "[profile_id] INTEGER NOT NULL, ";	//Key back to Profile
	strFields += "[type]		 TEXT NOT NULL, ";	//Profile_id + type create unique key.

	strFields += "[url]			TEXT, ";
	strFields += "[visibility]	INTEGER ";

	CreateTable( "Url", strFields.c_str() );
}

//-----------------------------------------------------------------------------

void CVoxClientDb::CreateTable_History()
{
}

//-----------------------------------------------------------------------------

void CVoxClientDb::CreateTable_Settings()
{
}

//-----------------------------------------------------------------------------

void CVoxClientDb::CreateTable( const char* strTableName, const char* strFields )
{
	std::string strSql = "";

	strSql += "CREATE TABLE [";
	strSql += strTableName;
	strSql += "] ( ";
	strSql += strFields;
	strSql += " );";

	try
	{
		execDML( strSql.c_str() );
	}

	catch (CppSQLite3Exception& e)
	{
		e.errorCode();
	}
}

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//Helpers
//-----------------------------------------------------------------------------

int CVoxClientDb::GetTableRecordCount( const char* strTableName )
{
	int nCount = 0;

	try
	{
		CppSQLite3Buffer buf;
		buf.format( "SELECT count(*) from %Q", strTableName );

		nCount = execScalar( (const char*)buf );
	}

	catch (CppSQLite3Exception& e)
	{
		e.errorCode();
	}

	return nCount;
}

//-----------------------------------------------------------------------------

int CVoxClientDb::DeleteAllFromTable( const char* strTableName )
{
	//TODO: delete all, or drop and recreate table?
	//need to drop related profiles, addresses etc.  Use trigger.
	int nCount = 0;

	try
	{
		std::string strSql = "";
		
		strSql += "DELETE FROM ";
		strSql += strTableName;
		strSql += ";";

		nCount = execDML( strSql.c_str() );
	}

	catch (CppSQLite3Exception& e)
	{
		e.errorCode();
	}

	return nCount;
}

//-----------------------------------------------------------------------------
