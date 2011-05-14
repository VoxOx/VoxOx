#ifndef _VOX_CLIENT_DB_H_
#define _VOX_CLIENT_DB_H_

#include <CppSQLite3.h>	
#include <string>

class CVoxClientDb : public CppSQLite3DB
{
public:
	CVoxClientDb();
	CVoxClientDb( const char* strDbPathName );
	virtual ~CVoxClientDb();

	bool Create( bool bDeleteExisting = false );
	void SetPathName( const char* str )		{ m_strDbPathName = str;	}

	const char* getSQLiteVersion()			{ return SQLiteVersion();	}
	const char* getVersion();
	
	//Some convenience methods.
	int  GetTableRecordCount( const char* strTableName );
	int	 DeleteAllFromTable ( const char* strTableName );

protected:
	//Tables
	void CreateTables();
	void CreateIndexes();
	void CreateTriggers();

	void CreateTable_Contact();

	void CreateTable_MergedContact();
	void CreateTable_ImContact();
	void CreateTable_Group();

	void CreateTable_Profile();
	void CreateTable_EmailAddress();
	void CreateTable_Telephone();
	void CreateTable_StreetAddress();
	void CreateTable_Url();

	void CreateTable_History();
	void CreateTable_Settings();

	//Helpers
	void CreateTable		( const char* strTableName, const char* strFields );

private:
	std::string	m_strDbPathName;
};

#endif