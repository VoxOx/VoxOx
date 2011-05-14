
/*VOXOX CHANGE CJC CLASS TO HANDLE CONTACT LIST STYLES OF VOXOX */
#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtContactListStyle.h"

#include <QtCore/QFile>
#include <QtCore/QSettings>
#include <util/Logger.h>

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <model/config/ConfigManager.h>
#include <model/config/Config.h>

static const QString CONTACT_LIST_STYLE_FOLDER = "contactList/";

QtContactListStyle * QtContactListStyle::instance = NULL;


QtContactListStyle::QtContactListStyle(){

	

}


QtContactListStyle *QtContactListStyle::getInstance() {


if (!instance) {
	instance = new QtContactListStyle();
}

	return instance;
}

QString QtContactListStyle::getContactStyleFolder(){

	return CONTACT_LIST_STYLE_FOLDER;
}

QStringList QtContactListStyle::getStyleList() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	QDir dir(QString::fromStdString(config.getResourcesDir()) + "contactList/");

	QStringList list;
	Q_FOREACH(QFileInfo info, dir.entryInfoList()) {
		if (info.isDir()) {
			QString name = info.fileName();
			if (name[0] != '.') {
				list << name;
			}
		}
	}

	return list;
}


void QtContactListStyle::loadStyleConfig(QString currentStylePath) {
	_currentStylePath = currentStylePath;

	QSettings s(_currentStylePath + "/style.dat", QSettings::IniFormat);
	s.beginGroup("Style");
	_groupFontColor = QColor(s.value("GroupFontColor", "#FFFFFF").toString());
	_groupBackgroundColorTop = QColor(s.value("GroupBackgroundColorTop", "#FFFFFF").toString());
	_groupBackgroundColorBottom = QColor(s.value("GroupBackgroundColorBottom", "#FFFFFF").toString());
	_groupBorderColor = QColor(s.value("GroupBorderColor", "#FFFFFF").toString());
	_groupPixmapClose = QPixmap(s.value("GroupPixmapClose", "").toString());
	_groupPixmapOpen = QPixmap(s.value("GroupPixmapOpen", "").toString());
	_contactNonSelectedBackgroundColor = QColor(s.value("ContactNonSelectedBackgroundColor", "#FFFFFF").toString());
	_contactSelectedBackgroundColorTop = QColor(s.value("ContactSelectedBackgroundColorTop", "#FFFFFF").toString());
	_contactSelectedBackgroundColorBottom = QColor(s.value("ContactSelectedBackgroundColorBottom", "#FFFFFF").toString());
	_contactUserNameNonSelectedFontColor = QColor(s.value("ContactUserNameNonSelectedFontColor", "#FFFFFF").toString());
	_contactSocialStatusNonSelectedFontColor = QColor(s.value("ContactSocialStatusNonSelectedFontColor", "#FFFFFF").toString());
	_contactUserNameSelectedFontColor = QColor(s.value("ContactUserNameSelectedFontColor", "#FFFFFF").toString());
	_contactSocialStatusSelectedFontColor = QColor(s.value("ContactSocialStatusSelectedFontColor", "#FFFFFF").toString());

}
