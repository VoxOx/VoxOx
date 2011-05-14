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
* CLASS DESCRIPTION 
* @author Chris Jimenez C 
* @date 2009.06.01
*/


#ifndef QTWIZARDSELECTNETWORKS_H
#define QTWIZARDSELECTNETWORKS_H

#include "QtIWizard.h"

#include <QtGui/QWidget>

class CWengoPhone;
namespace Ui { class WizardSelectNetworks; }

/**
 * General WengoPhone configuration panel.
 *
 * @author Tanguy Krotoff
 */
class QtWizardSelectNetworks : public QWidget, public QtIWizard {
	Q_OBJECT
public:

	QtWizardSelectNetworks(CWengoPhone & cWengoPhone, QWidget * parent);

	virtual ~QtWizardSelectNetworks();

	virtual QString getName() const;

	virtual QString getTitle() const;

	virtual QString getDescription() const;

	virtual int getStepNumber() const;

	virtual void saveConfig();


	virtual QWidget *getWidget() const {
		return (QWidget*)this;
	}

	virtual QString getPixmapPath() const{return "";}

	bool getAddAIM();
	
	bool getAddFacebook();

	bool getAddGoogleTalk();

	bool getAddICQ();

	bool getAddJabber();

	bool getAddMSN();

	bool getAddMySpace();

	bool getAddSkype();

	bool getAddTwitter();

	bool getAddYahoo();

private:

	CWengoPhone & _cWengoPhone;

	virtual void readConfig();

	Ui::WizardSelectNetworks * _ui;

	bool _aimAdded;
	bool _facebookAdded;
	bool _googleAdded;
	bool _icqAdded;
	bool _jabberAdded;
	bool _msnAdded;
	bool _myspaceAdded;
	bool _skypeAdded;
	bool _twitterAdded;
	bool _yahooAdded;
};

#endif	//QtWizardSelectNetworks_H
