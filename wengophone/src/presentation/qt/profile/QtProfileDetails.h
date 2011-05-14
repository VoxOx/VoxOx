/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2007  Wengo
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

#ifndef OWQTPROFILEDETAILS_H
#define OWQTPROFILEDETAILS_H

#include <util/NonCopyable.h>

#include <QtCore/QObject>
#include <QtGui/QDialog>

class UserProfile;
class ContactProfile;
class CUserProfile;
class Profile;
class QtIMContactManagerInterface;

class QImage;
class QWidget;
#include <presentation/qt/QtVoxWindowInfo.h>
class QtVoxOxCallBarFrame;
class QShortcut;

namespace Ui { class ProfileDetails; }

/**
 * Profile/Contact/UserProfile details window.
 *
 * @see Profile
 * @see UserProfile
 * @see Contact
 * @author Tanguy Krotoff
 */
class QtProfileDetails :public QtVoxWindowInfo, NonCopyable {
	Q_OBJECT
public:

	//QtProfileDetails(CUserProfile & cUserProfile, ContactProfile & contactProfile, QWidget * parent, const QString & windowTitle);

	QtProfileDetails(CUserProfile & cUserProfile, UserProfile & userProfile, QWidget * parent, const QString & windowTitle);

	~QtProfileDetails();

	virtual QWidget * getWidget() const;

	virtual QString getKey() const;

	virtual QString getDescription() const;

	virtual int getAllowedInstances() const;

	virtual int getCurrentTab() const ;

	virtual void setCurrentTab(QString tabName);

	virtual void showWindow();

	//QDialog * getWidget() const {
	//	return this;
	//}

	void setWengoName(const QString & wengoName);

	void setGroup(const QString & group);

	void setFirstName(const QString & firstName);

	void setLastName(const QString & lastName);

	void setCountry(const QString & country);

	void setCity(const QString & city);

	void setState(const QString & state);

	void setWebsite(const QString & website);

	void setHomePhone(const QString & homePhone);

	void setSipAddress(const QString & sipAddress);

public Q_SLOTS:

	void closeEvent();//VOXOX - CJC - 2009.07.09 

	void changeUserProfileAvatar();

	void openMyNetworks();

private Q_SLOTS:

	void updateSaveButtonState();

	void saveContact();

	void saveUserProfile();

	void cancelButtonClicked();

	/**
	 * Switch from simple mode to advanced mode.
	 *
	 * The button advanced from QtSimpleIMContactManager has been clicked.
	 * Changes _ui->imStackedWidget widget.
	 */
	void advancedButtonClicked();

	/**
	 * Shows the contact web page.
	 */
	void websiteButtonClicked();

	/**
	 * Shows the contact email.
	 */
	void emailButtonClicked();

private:


	Ui::ProfileDetails * _ui;
	//ContactProfile & fillAndGetContactProfile();

	void init(QWidget * parent);

	/** Populates the country list widget. */
	void populateCountryList();

	void readProfile();

	void updateAvatarButton();

	void saveProfile();

	void setAvatarImage(const QString& path);
	void setAvatarImage(QImage image);							//VOXOX CHANGE by Rolando - 2009.06.01  - needed when using PictureFlow class
	void setAvatarImage(const QString& path, QImage& image );	//VOXOX - JRT - 2009.06.07 - Share common code

	//QDialog * _profileDetailsWindow;

	CUserProfile & _cUserProfile;

//	Profile & _profile;
	UserProfile & _profile;

	QtIMContactManagerInterface * _qtIMContactManager;

	QtVoxOxCallBarFrame * _callBarFrameMobile;
	QtVoxOxCallBarFrame * _callBarFrameHome;
	QtVoxOxCallBarFrame * _callBarFrameWork;
	QtVoxOxCallBarFrame * _callBarFrameFax;
	QtVoxOxCallBarFrame * _callBarFrameOther;

	/**
	 * QtProfileDetails shows a Contact rather than a UserProfile if true.
	 */
	bool _showContact;
	
	#if defined(OS_MACOSX)
		QShortcut *_closeWindowShortCut;//VOXOX -ASV- 08-25-2009: we close the window when CMD + Shift + W is pressed on Mac
	#endif
};

#endif	//QTPROFILEDETAILS_H
