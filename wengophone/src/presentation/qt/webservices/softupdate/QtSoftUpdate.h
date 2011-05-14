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

#ifndef OWQTSOFTUPDATE_H
#define OWQTSOFTUPDATE_H

#include <presentation/PSoftUpdate.h>
#include <model/webservices/softupdate/WsSoftUpdate.h>

#include <http/HttpRequest.h>

#include <util/Trackable.h>

#include <qtutil/QObjectThreadSafe.h>

#include <QtCore/QString>
#include <QtCore/QProcess>

class CWengoPhone;
class SoftUpdater;
class CSoftUpdate;
class QtWengoPhone;

class QDialog;
class QtVoxMessageBox;
class QtSoftUpdatePrompt;	//VOXOX - JRT - 2009.10.28 

//namespace Ui { class SoftUpdateWindow; }

/**
 * Shows a Qt progress dialog when downloading a WengoPhone update.
 *
 * Starts the update.exe process (the NSIS installer under Windows).
 *
 * @author Tanguy Krotoff
 */
class QtSoftUpdate : public QObjectThreadSafe, public PSoftUpdate, public Trackable {
	Q_OBJECT
public:

	QtSoftUpdate(CSoftUpdate & cSoftUpdate);

	virtual ~QtSoftUpdate();

	void updatePresentation() { }

private Q_SLOTS:
	void abortDownload();
	void doUpdate();
	void abortUpdate( bool ignore );

	/**
	 * An error occured while starting the update process.
	 *
	 * @param error update process launching error
	 */
	void updateProcessError(QProcess::ProcessError error);

private:
	void initThreadSafe();

	void updatePresentationThreadSafe() { }

	void noAvailableUpdateEventHandler			( const SoftUpdateInfo& suInfo );		//VOXOX - JRT - 2009.10.27 
	void noAvailableUpdateEventHandlerThreadSafe( SoftUpdateInfo suInfo );				//VOXOX - JRT - 2009.10.27 

	void updateWengoPhoneEventHandler		   ( const SoftUpdateInfo& suInfo );		//VOXOX - JRT - 2009.10.05 
	void updateWengoPhoneEventHandlerThreadSafe( SoftUpdateInfo suInfo );				//VOXOX - JRT - 2009.10.27 

	/**
	 * @see SoftUpdater::dataReadProgressEvent
	 */
	void dataReadProgressEventHandler		   (double bytesDone, double bytesTotal, unsigned downloadSpeed);
	void dataReadProgressEventHandlerThreadSafe(double bytesDone, double bytesTotal, unsigned downloadSpeed);

	/**
	 * @see SoftUpdater::downloadFinishedEvent
	 */
	void downloadFinishedEventHandler		   (HttpRequest::Error error);
	void downloadFinishedEventHandlerThreadSafe(HttpRequest::Error error);

	/**
	 * Launches the update process (the NSIS installer under Windows).
	 */
	void launchUpdateProcess();
	void handleAbortUpdate();

#ifdef OS_WINDOWS
	QString		getDownloadTgtFilePath( const QString& fileName );
#endif

	QString		getUpdateCmdLine()		const		{ return _updateCmdLine;		}
	QStringList getUpdateCmdLineParms() const		{ return _updateCmdLineParms;	}

private:
	QString				_updateCmdLine;
	QStringList			_updateCmdLineParms;

	QtWengoPhone*		_qtWengoPhone;

	CSoftUpdate&		_cSoftUpdate;

	/** SoftUpdater. */
	SoftUpdater *		_softUpdater;

	/** Original progress dialog label text. */
	QString				_originalLabelText;

//	Ui::SoftUpdateWindow * _ui;

	QtSoftUpdatePrompt*	 _promptWindow;

	QtVoxMessageBox*	_msgBox;
	SoftUpdateInfo		_suInfo;
	bool				_downloading;
};

#endif	//OWQTSOFTUPDATE_H
