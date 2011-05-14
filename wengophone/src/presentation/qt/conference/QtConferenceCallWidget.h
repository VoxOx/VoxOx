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

#ifndef OWQTCONFERENCECALLWIDGET_H
#define OWQTCONFERENCECALLWIDGET_H

#include <model/phoneline/EnumMakeCallError.h>

#include <thread/ThreadEvent.h>
#include <util/Trackable.h>

#include <QtGui/QWidget>
#include <map>

class IPhoneLine;
class CWengoPhone;
class QtVoxOxCallBarFrame;
class StringList;

namespace Ui { class ConferenceCallWidget; }

/**
 *
 * @author Mathieu Stute
 */
class QtConferenceCallWidget : public QWidget, public Trackable  {
	Q_OBJECT

public:

	/**
	 * Constructor.
	 */
	QtConferenceCallWidget(QWidget * parent, CWengoPhone & cWengoPhone, IPhoneLine * phoneLine , bool startConference = true);

	/**
	 * Destructor.
	 */
	virtual ~QtConferenceCallWidget();

	void populateVoxOxContactsMap();

private Q_SLOTS:

	/**
	 * The user has clicked the start button.
	 */
	void startClicked();

	void cancel();

Q_SIGNALS:
	void startClickedSignal();
	void cancelClicked();

	void cancelConferenceCall(std::string phoneNumber);//VOXOX CHANGE by Rolando - 2009.05.29 

private:
	
	void makeConferenceCallErrorEventHandler(EnumMakeCallError::MakeCallError, std::string);

	void makeConferenceCallErrorEventHandlerThreadSafe(EnumMakeCallError::MakeCallError error, std::string phoneNumber);

	bool _startConference;

	IPhoneLine * _phoneLine;

	CWengoPhone & _cWengoPhone;

	Ui::ConferenceCallWidget * _ui;

	QtVoxOxCallBarFrame *_qtVoxOxCallBarFrame1;

	QtVoxOxCallBarFrame *_qtVoxOxCallBarFrame2;

	typedef std::map<std::string,std::string> VoxOxContactsMap;//VOXOX CHANGE Rolando 04-28-09
	VoxOxContactsMap _voxOxContactsMap;//VOXOX CHANGE Rolando 04-28-09

};

#endif	//OWQTCONFERENCECALLWIDGET_H