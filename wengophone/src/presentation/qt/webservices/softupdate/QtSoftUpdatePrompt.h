/*
 * VoxOx!!!
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

#ifndef VOX_QTSOFTUPDATE_PROMPT_H
#define VOX_QTSOFTUPDATE_PROMPT_H

#include <model/webservices/softupdate/WsSoftUpdate.h>

#include <QtGui/QDialog>
#include <QtCore/QMutex>

#include <util/NonCopyable.h>

#include <string>

namespace Ui { class SoftUpdatePromptWindow; }

/**
 * Prompt/Inform user about new version
 *
 * @author Jeff Theinert
 */
class QtSoftUpdatePrompt : public QDialog, NonCopyable 
{
	Q_OBJECT
public:
	QtSoftUpdatePrompt( QWidget* parent = NULL );

	void display( const SoftUpdateInfo& suInfo );
	void updateDownloadStats( double bytesDone, double bytesTotal, unsigned int downloadSpeed );
	void reportDownloadStats();

	~QtSoftUpdatePrompt();

Q_SIGNALS:
	void signalInstall();
	void signalCancel( bool ignore );

private Q_SLOTS:
	void handleInstall();
	void handleCancel();

private:
//	void updateDisplay( double bytesTotal, double bytesDone, const QString& timeRemaining );
	void refreshDisplay();
	void timerEvent(QTimerEvent * eventIn);
	void killRefreshTimer();

	QString calcTimeRemaining();

private:
	Ui::SoftUpdatePromptWindow*	_ui;

	static QString s_defRecommended;
	static QString s_defMandatory;
	static QString s_duringDownloadText;
	
	int				_refreshTimerId;
	double			_bytesTotal;
	double			_bytesDone;
	unsigned int	_downloadSpeed;

	QMutex			_mutex;
};

#endif	//VOX_QTSOFTUPDATE_PROMPT_H
