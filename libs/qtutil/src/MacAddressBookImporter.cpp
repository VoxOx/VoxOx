/*
 * VoxOx, Take Control
 * Copyright (C) 2004-2009  VoxOx
 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version
 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/**
 * Represent Email types
 * @author Alexander Solis V 
 * @date 2009.08.21
 */

#include <QtGui/QMessageBox>
#include <QtCore/QVariant>
#include <QtCore/QDebug>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

#include <util/Path.h>

#include <util/Logger.h>
//#include <QSqlTableModel>
//#include <QtSql>
//#include <QSqlDatabase>
//#include <QSqlError>


#include <qtutil/MacAddressBookImporter.h>

bool MacAddressBookImporter::connectToDB()
{
	db = QSqlDatabase::addDatabase("QSQLITE", QString("AddressBook"));
	QString home = QString::fromStdString(Path::getHomeDirPath());
	db.setDatabaseName("/Users/alex/Library/Application Support/AddressBook/AddressBook-v22.abcddb");
	//db.setDatabaseName(home+"/Library/Application Support/AddressBook/AddressBook-v22.abcddb");
    
	QSqlError err;
	
	if (db.open()) {
		return true;		
    }
	else {
		err = db.lastError();
		QString strErr = err.databaseText();
		strErr = err.driverText();
		strErr = strErr + err.text();
        db = QSqlDatabase();
        QSqlDatabase::removeDatabase(QString("AddressBook"));
    	QMessageBox::critical(0, QString("Cannot open the database"),
							  strErr + "\nClick Ok to exit.", QMessageBox::Ok);
        return false;		
	}
}

QStringList MacAddressBookImporter::extractAddressBookContacts()
{
	QSqlQuery *query = new QSqlQuery("SELECT pn.zfullnumber, r.zfirstname, r.zlastname FROM ZABCDPHONENUMBER pn JOIN ZABCDRECORD r ON pn.zowner = r.z_pk ORDER by r.zfirstname, r.zlastname", db);
	QStringList dataList;
	std::string temp = "";
	// Loops thought all the selected records and loads them in a QStringList
	while (query->next()) {
		for (int iter=0; iter<=2; iter++) {
			temp = query->value(iter).toString().toStdString();
			dataList << query->value(iter).toString();
		}
	}

	QSqlDatabase::removeDatabase("QSQLITE");
	
	return dataList;
}
