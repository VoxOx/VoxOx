
/*VOXOX CHANGE CJC CLASS TO HANDLE ITEM LIST STYLES OF VOXOX */
#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtUMItemListStyle.h"

#include <QtCore/QFile>

#include <QtCore/QSettings>
#include <util/Logger.h>

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <model/config/ConfigManager.h>
#include <model/config/Config.h>

static const QString ITEM_LIST_STYLE_FOLDER = "chat/umtreewidget/";
static const QString CURRENT_STYLE = "dark";

QtUMItemListStyle * QtUMItemListStyle::instance = NULL;


QtUMItemListStyle::QtUMItemListStyle(){
}


QtUMItemListStyle *QtUMItemListStyle::getInstance() {

	if (!instance) {
		instance = new QtUMItemListStyle();
	}

	return instance;
}

QString QtUMItemListStyle::getStyleFolder(){

	return ITEM_LIST_STYLE_FOLDER;
}

QString QtUMItemListStyle::getCurrentStyle(){

	return CURRENT_STYLE;
}

QStringList QtUMItemListStyle::getStyleList() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	QDir dir(QString::fromStdString(config.getResourcesDir()) + ITEM_LIST_STYLE_FOLDER);

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


void QtUMItemListStyle::loadStyleConfig(QString currentStylePath) {
	_currentStylePath = currentStylePath;

	QSettings s(_currentStylePath + "/style.dat", QSettings::IniFormat);
	s.beginGroup("Style");

	_groupBlinkingBackgroundColor = QColor(s.value("GroupBlinkingBackgroundColor", "#FFFFFF").toString());
	_groupBlinkingFontColor = QColor(s.value("GroupBlinkingFontColor", "#FFFFFF").toString());

	_groupSelectedFontColor = QColor(s.value("GroupSelectedFontColor", "#FFFFFF").toString());
	_groupNonSelectedFontColor = QColor(s.value("GroupNonSelectedFontColor", "#FFFFFF").toString());

	_groupSelectedBackgroundColorTop = QColor(s.value("GroupSelectedBackgroundColorTop", "#FFFFFF").toString());
	_groupSelectedBackgroundColorBottom = QColor(s.value("GroupSelectedBackgroundColorBottom", "#FFFFFF").toString());
	_groupNonSelectedBackgroundColorTop = QColor(s.value("GroupNonSelectedBackgroundColorTop", "#FFFFFF").toString());
	_groupNonSelectedBackgroundColorBottom = QColor(s.value("GroupNonSelectedBackgroundColorBottom", "#FFFFFF").toString());

	_groupSelectedBorderColor = QColor(s.value("GroupSelectedBorderColor", "#FFFFFF").toString());
	_groupNonSelectedBorderColor = QColor(s.value("GroupNonSelectedBorderColor", "#FFFFFF").toString());

	_groupPixmapClose = QPixmap(s.value("GroupPixmapClose", "").toString());
	_groupPixmapOpen = QPixmap(s.value("GroupPixmapOpen", "").toString());
	_itemNonSelectedBackgroundColor = QColor(s.value("ItemNonSelectedBackgroundColor", "#FFFFFF").toString());
	_itemSelectedBackgroundColorTop = QColor(s.value("ItemSelectedBackgroundColorTop", "#FFFFFF").toString());
	_itemSelectedBackgroundColorBottom = QColor(s.value("ItemSelectedBackgroundColorBottom", "#FFFFFF").toString());

	_itemUserNameNonSelectedFontColor = QColor(s.value("ItemUserNameNonSelectedFontColor", "#FFFFFF").toString());
	_itemUserNameSelectedFontColor = QColor(s.value("ItemUserNameSelectedFontColor", "#FFFFFF").toString());

	_itemBlinkingBackgroundColor = QColor(s.value("ItemBlinkingBackgroundColor", "#FFFFFF").toString());
	_itemBlinkingFontColor = QColor(s.value("ItemBlinkingFontColor", "#FFFFFF").toString());

}