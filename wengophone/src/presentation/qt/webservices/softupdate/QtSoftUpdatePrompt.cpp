/*
 * VOXOX

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
#include "QtSoftUpdatePrompt.h"

#include "ui_SoftUpdatePromptWindow.h"

#include <util/SafeDelete.h>
#include <util/Logger.h>		//VOXOX - JRT - 2009.12.23 

#include <qtutil/SafeConnect.h>

#include <QtGui/QtGui>

//#include <windows.h>

QString QtSoftUpdatePrompt::s_defRecommended	 = "A new version of VoxOx (%1) is available.  <p>Would you like to install it now?";
QString QtSoftUpdatePrompt::s_defMandatory		 = "A new version of VoxOx (%1) is available.  <p><b>In order to continue using VoxOx, you will need to install the new version.</b>  <p>Would you like to install it now?";
QString QtSoftUpdatePrompt::s_duringDownloadText = "Once download is complete the new version of VoxOx will be installed.  <p>The installation process may take up to two minutes, once it's complete VoxOx will restart.";

//-----------------------------------------------------------------------------

QtSoftUpdatePrompt::QtSoftUpdatePrompt( QWidget* parent ) : QDialog(parent) 
{
	_ui = new Ui::SoftUpdatePromptWindow();
	_ui->setupUi(this);

	_bytesTotal		= 0;
	_bytesDone		= 0;
	_downloadSpeed	= 1;

	refreshDisplay();
}

//-----------------------------------------------------------------------------

QtSoftUpdatePrompt::~QtSoftUpdatePrompt() 
{
	killRefreshTimer();
	OWSAFE_DELETE(_ui);
}

//-----------------------------------------------------------------------------

void QtSoftUpdatePrompt::killRefreshTimer()
{
	if ( _refreshTimerId > 0 )
	{
		killTimer( _refreshTimerId );
	}
}

//-----------------------------------------------------------------------------

void QtSoftUpdatePrompt::display( const SoftUpdateInfo& suInfo )
{
	_refreshTimerId = 0;

	//Set prompt.
	QString labelText = "";
	QString imagePath = ":pics/auto_updater_recommended.png";

	//Determine text.
	if ( suInfo.getDescription() == "" )
	{
		if ( suInfo.isMandatory() )
		{
			labelText = s_defMandatory;
		}
		else if ( suInfo.isRecommended() )
		{
			labelText = s_defRecommended;
		}
		else
		{
			assert(false);		//New option?
			labelText = s_defRecommended;
		}
	}
	else
	{
		labelText = suInfo.getDescription().c_str();
	}

	//Determine Image
	if ( suInfo.isMandatory() )
	{
		imagePath = ":pics/auto_updater_mandatory.png";
	}
	else if ( suInfo.isRecommended() )
	{
		imagePath = ":pics/auto_updater_recommended.png";
	}
	else
	{
		assert(false);		//New option?
		imagePath = ":pics/auto_updater_recommended.png";
	}
			
	//Update UI.
	QString labelText2 = labelText.arg( suInfo.getVersion().c_str() );

	_ui->lblPrompt->setText(labelText2);
	_ui->lblIcon->setPixmap( QPixmap(imagePath) );

	//Hide checkbox as needed.
	if ( suInfo.isMandatory() )
	{
		_ui->checkBox->setEnabled( false );
		_ui->checkBox->hide();
	}

	//Hide download text and progress bar until user clicks Install Now.
	_ui->lblDownloading->hide();
	_ui->progressBar->hide();

	SAFE_CONNECT(_ui->installButton, SIGNAL(clicked()), SLOT(handleInstall()) );
	SAFE_CONNECT(_ui->cancelButton,  SIGNAL(clicked()), SLOT(handleCancel() ) );

	//Modal for mandatory, modeless for recommended.
	if ( suInfo.isMandatory() )
	{
		exec();
	}
	else
	{
		show();
	}
}

//-----------------------------------------------------------------------------

void QtSoftUpdatePrompt::handleInstall()
{
	_ui->lblPrompt->setText(s_duringDownloadText);
	_ui->lblPrompt->repaint();

	_ui->checkBox->setEnabled( false );
	_ui->checkBox->hide();

	_ui->lblDownloading->show();
	_ui->progressBar->show();

	_ui->installButton->setEnabled( false );
	_ui->installButton->hide();

	repaint();

	signalInstall();

	_refreshTimerId = startTimer( 500 );
}

//-----------------------------------------------------------------------------

void QtSoftUpdatePrompt::handleCancel()
{
	bool ignore = _ui->checkBox->isChecked();

	killRefreshTimer();		//VOXOX - JRT - 2009.12.29 

	signalCancel( ignore );
}

//-----------------------------------------------------------------------------

void QtSoftUpdatePrompt::updateDownloadStats( double bytesDone, double bytesTotal, unsigned int downloadSpeed )
{
	_mutex.lock();

	_bytesDone		= bytesDone;
	_bytesTotal		= bytesTotal;
	_downloadSpeed	= downloadSpeed;
	
	_mutex.unlock();
}

//-----------------------------------------------------------------------------

QString QtSoftUpdatePrompt::calcTimeRemaining()
{
	int totalSeconds = (_bytesTotal - _bytesDone) / (_downloadSpeed * 1000);
	int minutes		 = (int)(totalSeconds / 60);
	int seconds		 = totalSeconds - (minutes * 60);

	double pctDone	 = _bytesDone / _bytesTotal;

	QString timeRemaining = "Calculating...";

	if ( pctDone >= 0.02 )
	{
		timeRemaining = QString( "%1:%2").arg(minutes).arg(seconds, 2, 10, QChar('0'));
	}

	return timeRemaining;
}

//-----------------------------------------------------------------------------

void QtSoftUpdatePrompt::refreshDisplay()
{
	_mutex.lock();

	QString timeRemaining = calcTimeRemaining();

	_ui->progressBar->setRange( 0, (int)_bytesTotal);
	_ui->progressBar->setValue( (int)_bytesDone);

	QString prompt = QString( "Downloading - Time Remaining:  %1").arg(timeRemaining);

	_ui->lblDownloading->setText( prompt );

	update();		//Force repaint.
	
	_mutex.unlock();
}

//-----------------------------------------------------------------------------

void QtSoftUpdatePrompt::timerEvent( QTimerEvent * eventIn ) 
{
	if (eventIn->timerId() == _refreshTimerId) 
	{
		refreshDisplay();
	}
	else
	{
		QObject::timerEvent(eventIn);
	}
}

//-----------------------------------------------------------------------------

void QtSoftUpdatePrompt::reportDownloadStats() 
{
	char msg[200];

	sprintf( msg, "Download Stats - Total Bytes: %10.0f, Bytes Downloaded: %10.0f, Speed: %u kbps.\n", _bytesTotal, _bytesDone, _downloadSpeed );
	LOG_INFO( msg );
}

//-----------------------------------------------------------------------------