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

#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtSoftUpdate.h"
#include "QtSoftUpdatePrompt.h"	//VOXOX - JRT - 2009.10.28 

//#include "ui_SoftUpdateWindow.h"

#include "QtBrowserSoftUpdate.h"

#include <presentation/qt/QtWengoPhone.h>
#include <presentation/qt/messagebox/QtVoxMessageBox.h>		//VOXOX - CJC - 2009.06.10 

#include <model/webservices/softupdate/WsSoftUpdate.h>		//VOXOX - JRT - 2009.10.05 - For SoftUpdateInfo

#include <control/CWengoPhone.h>
#include <control/webservices/softupdate/CSoftUpdate.h>

#include <softupdater/SoftUpdater.h>

#include <util/String.h>
#include <util/Logger.h>
#include <util/SafeDelete.h>
#include <util/Path.h>			//VOXOX - JRT - 2009.12.10 
#include <cutil/global.h>

#include <qtutil/SafeConnect.h>

#include <QtGui/QtGui>

//VOXOX - ASV - 2009.12.02
#ifdef OS_WINDOWS
static const char * UPDATE_PROGRAM = "VoxOxUpdate.exe";
#endif
//VOXOX - ASV - 2009.12.02
#ifdef OS_MACOSX
#include <unistd.h>
static const char * UPDATE_PROGRAM = "VoxOxUpdate.dmg";
#endif

QtSoftUpdate::QtSoftUpdate(CSoftUpdate & cSoftUpdate)
	: QObjectThreadSafe(NULL),
	_cSoftUpdate(cSoftUpdate) 
{
//	_ui				  = NULL;
	_softUpdater	  = NULL;
	_promptWindow	  = NULL;		//VOXOX - JRT - 2009.10.28 
	_msgBox			  = NULL;
	_downloading	  = false;		//VOXOX - JRT - 2009.10.30 

	_updateCmdLine	    = "";		//VOXOX - JRT - 2009.10.26 
	_updateCmdLineParms.clear();	//VOXOX - JRT - 2009.10.26 

	_qtWengoPhone = (QtWengoPhone *) _cSoftUpdate.getCWengoPhone().getPresentation();

	updateWengoPhoneEvent  += boost::bind(&QtSoftUpdate::updateWengoPhoneEventHandler,  this, _1 );	//VOXOX - JRT - 2009.10.05 
	noAvailableUpdateEvent += boost::bind(&QtSoftUpdate::noAvailableUpdateEventHandler, this, _1 );	//VOXOX - JRT - 2009.10.05 


	typedef PostEvent0<void ()> MyPostEvent;
	MyPostEvent * event = new MyPostEvent(boost::bind(&QtSoftUpdate::initThreadSafe, this));
	postEvent(event);
}

void QtSoftUpdate::initThreadSafe() 
{
}

QtSoftUpdate::~QtSoftUpdate() 
{
//	OWSAFE_DELETE(_ui);
	OWSAFE_DELETE(_softUpdater);
	OWSAFE_DELETE(_msgBox);

	OWSAFE_DELETE( _promptWindow	 );
}


void QtSoftUpdate::noAvailableUpdateEventHandler( const SoftUpdateInfo& suInfo )		//VOXOX - JRT - 2009.10.27 
{
	typedef PostEvent1<void ( SoftUpdateInfo ), SoftUpdateInfo > MyPostEvent;
	MyPostEvent * event = new MyPostEvent(boost::bind(&QtSoftUpdate::noAvailableUpdateEventHandlerThreadSafe, this, _1 ), suInfo );
	postEvent(event);
}

void QtSoftUpdate::noAvailableUpdateEventHandlerThreadSafe( SoftUpdateInfo suInfo )		//VOXOX - JRT - 2009.10.27 
{
	if ( _msgBox == NULL )
	{
		_msgBox = new QtVoxMessageBox(_qtWengoPhone->getWidget());

		_msgBox->setModal( false );
		_msgBox->setWindowTitle("No New Updates");
		_msgBox->setText(tr("You are running the latest version of VoxOx."));
		_msgBox->setStandardButtons(QMessageBox::Ok);
	}

	_msgBox->show();		//Modeless
}

void QtSoftUpdate::updateWengoPhoneEventHandler( const SoftUpdateInfo& suInfo )		//VOXOX - JRT - 2009.10.05 
{
	typedef PostEvent1<void ( SoftUpdateInfo ), SoftUpdateInfo > MyPostEvent;
	MyPostEvent * event = new MyPostEvent(boost::bind(&QtSoftUpdate::updateWengoPhoneEventHandlerThreadSafe, this, _1 ), suInfo );
	postEvent(event);
}

void QtSoftUpdate::updateWengoPhoneEventHandlerThreadSafe( SoftUpdateInfo suInfo ) 
{
	_suInfo = suInfo;

	_promptWindow = new QtSoftUpdatePrompt();

	SAFE_CONNECT( _promptWindow, SIGNAL(signalInstall()),		SLOT(doUpdate()		     ));
	SAFE_CONNECT( _promptWindow, SIGNAL(signalCancel ( bool )), SLOT(abortUpdate( bool ) ));

	_promptWindow->display( suInfo );
}

//-----------------------------------------------------------------------------

void QtSoftUpdate::abortUpdate( bool ignore ) 
{
	if ( _downloading )
	{
		_softUpdater->dataReadProgressEvent -= boost::bind(&QtSoftUpdate::dataReadProgressEventHandler, this, _1, _2, _3);
		_softUpdater->downloadFinishedEvent -= boost::bind(&QtSoftUpdate::downloadFinishedEventHandler, this, _1);

		OWSAFE_DELETE(_promptWindow);
		abortDownload();
	}
	else
	{
		if ( ignore )
		{
			_cSoftUpdate.ignoreVersion( _suInfo );
		}

		OWSAFE_DELETE(_promptWindow);

		handleAbortUpdate();
	}
}

//-----------------------------------------------------------------------------

void QtSoftUpdate::doUpdate()
{
//VOXOX - ASV - 2009.12.02: we save the file on a tmp folder on Mac so this is not necessary
//VOXOX - JRT - 2009.12.10 - TODO: any Mac specific pathing should be moved to getDownloadTgtFilePath().
#ifdef OS_WINDOWS
	//Deletes previous update program
	QString filePath = getDownloadTgtFilePath( UPDATE_PROGRAM );
	QFile file( filePath );
	file.remove();
	file.close();

	_updateCmdLine = filePath;
#else
	_updateCmdLine = UPDATE_PROGRAM;
#endif

	LOG_INFO( "AutoUpdater local file: " + _updateCmdLine.toStdString() );
	
	_downloading   = true;

	QString parms = _suInfo.getCmdLineParms().c_str();
	_updateCmdLineParms = parms.split( "|" );

	_softUpdater = new SoftUpdater( _suInfo.getDownloadUrl(), _updateCmdLine.toStdString() );		//VOXOX - JRT - 2009.10.05 
	
	_softUpdater->dataReadProgressEvent += boost::bind(&QtSoftUpdate::dataReadProgressEventHandler, this, _1, _2, _3);
	_softUpdater->downloadFinishedEvent += boost::bind(&QtSoftUpdate::downloadFinishedEventHandler, this, _1);
	_softUpdater->start();
}

//VOXOX - JRT - 2009.12.16 - Let's not needlessly use the Application event loop when a UI timer will do.
void QtSoftUpdate::dataReadProgressEventHandler(double bytesDone, double bytesTotal, unsigned downloadSpeed) 
{
	static int count = 0;

	count++;

	_promptWindow->updateDownloadStats( bytesDone, bytesTotal, downloadSpeed );
}

//void QtSoftUpdate::dataReadProgressEventHandler(double bytesDone, double bytesTotal, unsigned downloadSpeed) 
//{
//	typedef PostEvent3<void (double, double, unsigned), double, double, unsigned> MyPostEvent;
//	MyPostEvent * event = new MyPostEvent(boost::bind(&QtSoftUpdate::dataReadProgressEventHandlerThreadSafe, this, _1, _2, _3), bytesDone, bytesTotal, downloadSpeed);
//	postEvent(event);

//}

//void QtSoftUpdate::dataReadProgressEventHandlerThreadSafe(double bytesDone, double bytesTotal, unsigned downloadSpeed) 
//{
//	int totalSeconds = (bytesTotal - bytesDone) / (downloadSpeed * 1000);
//	int minutes		 = (int)(totalSeconds / 60);
//	int seconds		 = totalSeconds - (minutes * 60);
//
//	double pctDone	 = bytesDone / bytesTotal;
//
//	//VOXOX - JRT - 2009.12.11 - Smooth out time calculations
//	QString timeRemaining = "Calculating...";
//
//	if ( pctDone >= 0.02 )
//	{
//		timeRemaining = QString( "%1:%2").arg(minutes).arg(seconds, 2, 10, QChar('0'));
//	}
//
//	_promptWindow->updateDisplay( bytesTotal, bytesDone, timeRemaining );
//
//	QApplication::processEvents();
//}

void QtSoftUpdate::downloadFinishedEventHandler(HttpRequest::Error error) 
{
	typedef PostEvent1<void (HttpRequest::Error), HttpRequest::Error> MyPostEvent;
	MyPostEvent * event = new MyPostEvent(boost::bind(&QtSoftUpdate::downloadFinishedEventHandlerThreadSafe, this, _1), error);
	postEvent(event);
}

void QtSoftUpdate::downloadFinishedEventHandlerThreadSafe(HttpRequest::Error error) 
{
//	LOG_DEBUG("download finished");
	LOG_INFO("download finished. Result code: " + String::fromNumber( error ) );

	_promptWindow->reportDownloadStats();

	if (error == HttpRequest::NoError) 
	{
		//Terminates the thread from the model
		_cSoftUpdate.getCWengoPhone().terminate();

		launchUpdateProcess();
	} 
	else 
	{
		LOG_FATAL( "download failed!" );
		handleAbortUpdate();		//VOXOX - JRT - 2009.10.30 
		OWSAFE_DELETE(_promptWindow);
	}
}

void QtSoftUpdate::abortDownload() 
{
	_downloading = false;		//VOXOX - JRT - 2009.12.29 
	_softUpdater->abort();

	handleAbortUpdate();
}

void QtSoftUpdate::launchUpdateProcess() 
{
#ifdef OS_WINDOWS
	QProcess * updateProcess = new QProcess();
	SAFE_CONNECT(updateProcess, SIGNAL(error(QProcess::ProcessError)), SLOT(updateProcessError(QProcess::ProcessError)));

	updateProcess->start( getUpdateCmdLine(), getUpdateCmdLineParms() );
#endif

//VOXOX - ASV - 2009.12.02
#ifdef OS_MACOSX
	QString fileName = QString::fromStdString( getenv("TMPDIR") );
	fileName += "/autoupdater.sh";
	QFile file( fileName );
	file.open(QIODevice::WriteOnly | QIODevice::Text);

	QTextStream out(&file);
	out << "#!/bin/bash\n" << "open `echo $TMPDIR`VoxOxUpdate.dmg";
	
	file.close();
	
	std::string pathToScript = getenv("TMPDIR");
	pathToScript += "/autoupdater.sh &";
	std::string addingPermissions = "chmod +x "+pathToScript;
	system(addingPermissions.c_str());
	system(pathToScript.c_str());
#endif
}

void QtSoftUpdate::updateProcessError(QProcess::ProcessError error) 
{
	QtVoxMessageBox box(_qtWengoPhone->getWidget());
	box.setWindowTitle("VoxOx - Update failed");
	box.setText(tr("VoxOx update failed to start: try to update @product@ manually"));
	box.setStandardButtons(QMessageBox::Ok);
	box.exec();
}

void QtSoftUpdate::handleAbortUpdate()
{
	//If user cancels a mandatory update, then all accounts are to be disconnected and app exited.
	if ( _suInfo.isMandatory() )
	{
		_cSoftUpdate.handleUserCanceledMandatory();
	}
}

//VOXOX - JRT - 2009.12.10 
#ifdef OS_WINDOWS
QString QtSoftUpdate::getDownloadTgtFilePath( const QString& fileName )
{
	QString result = QString( Path::getVoxOxConfigurationDirPath().c_str() ) + fileName;

	return result;
}
#endif
