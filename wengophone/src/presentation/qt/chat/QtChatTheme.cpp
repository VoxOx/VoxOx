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
#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtChatTheme.h"

#include <model/config/ConfigManager.h>
#include <model/config/Config.h>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QStringList>
#include <QtCore/QSettings>

#include <util/Logger.h>

static const char KEYWORD_CHAR = '%';

static const QString CONTENTS_PATH = "Contents/";
static const QString RESOURCES_PATH = "Resources/";
static const QString VARIANTS_PATH = "Variants/";

QtChatTheme::QtChatTheme(QWidget * parent): QObject(parent){

}


QtChatTheme::~QtChatTheme() {
	
}
QString QtChatTheme::getFooter() const {
	return _footer;
}

QString QtChatTheme::getStatusMessage() const {
	return _statusMessage;
}

QString QtChatTheme::getContent(Direction direction, Position position) const {
	QString html;
	if (direction == Incoming && position == First) {
		html = _incomingContent;
	} else if (direction == Outgoing && position == First) {
		//VOXOX HCANGE CJC If Outgoing/Content.html isn't found, Incoming/Content.html will be used 
		if(_outgoingContent!=""){
			html = _outgoingContent;
		}else
		{
			html = _incomingContent;
		}
	} else if (direction == Incoming && position == Next) {
		//VOXOX CHANGE CHECK IF WE HAVE A INCOMINNEXTCONTENT
		if(_incomingNextContent !=""){
			html = _incomingNextContent;
		}else{
			//Use Incoming content
			html = _incomingContent;
		}
	} else if (direction == Outgoing && position == Next) {
		//VOXOX HCANGE CJC If Outgoing/NextContent.html isn't found, Incoming/Content.html will be used 
		if(_outgoingNextContent!=""){
			html = _outgoingNextContent;
		}else
		{
			html = _incomingContent;
		}
	} else {
		LOG_FATAL("Unknown combination of direction and position");
	}

	return html;
}



QString QtChatTheme::getThemePath() const {
	return _themePath;
}

QString QtChatTheme::getHeader() const {
	return _header;
}


void QtChatTheme::initHeader(const QString & header){
	_header = header;
}

void QtChatTheme::setVariantCss(const QString & cssFile){
	
	_selectedVariantPath = _themePath + VARIANTS_PATH + cssFile +".css";

}

QString QtChatTheme::getSelectedVariantPath() const {
	return _selectedVariantPath;
}

QString QtChatTheme::getTemplate() const {//VOXOX - CJC - 2009.08.24 
	return _template;
}


QString QtChatTheme::loadFile(const QString& path) {
	QFile file(path);
	file.open(QIODevice::ReadOnly);
	QString data = QString::fromUtf8(file.readAll());
	return data.trimmed();
}

QString QtChatTheme::themeBaseDir() {
	static QString baseDir;
	if (baseDir.isEmpty()) {
		Config & config = ConfigManager::getInstance().getCurrentConfig();
		baseDir = QString::fromUtf8(config.getResourcesDir().c_str())
			+ "chat/themes/";
	}
	return baseDir;
}

void QtChatTheme::load(const QString & themeDir,const QString & themeVariant) {
	_themePath = themeBaseDir() + themeDir + "/" + CONTENTS_PATH + RESOURCES_PATH;
	//VOXOX CHANGE CJC Fix path
	_themePath = _themePath.replace("\\","/");
	if(themeVariant!=""){
		setVariantCss(themeVariant);
	}else{
		_selectedVariantPath = _themePath+"main.css";
	}
	//VOXOX - CJC - 2009.08.24 CJC Check if template exist for a specific theme
	//QFile file = _themePath + "Template.html";
	//VOXOX - ASV - 2009.09.09: fix a compilation issue on Mac with the above commented line.
	// Not sure why it works on Windows because on Mac there is no operator '=' in the documentation
	QFile file(_themePath + "Template.html");
	
	if(file.exists()){
		_template = loadFile(_themePath + "Template.html");
	}else{
		QString strThemeBaseDir = themeBaseDir();
		strThemeBaseDir = strThemeBaseDir.replace("\\","/");
		//Load default template
		_template =  loadFile(strThemeBaseDir + "Template.html");
	}

	_incomingContent = removeBreaklines(loadFile(_themePath + "Incoming/Content.html"));
	_incomingNextContent = removeBreaklines(loadFile(_themePath + "Incoming/Nextcontent.html"));
	_outgoingContent = removeBreaklines(loadFile(_themePath + "Outgoing/Content.html"));
	_outgoingNextContent = removeBreaklines(loadFile(_themePath + "Outgoing/Nextcontent.html"));
	_footer = loadFile(_themePath + "Footer.html");
	_header = loadFile(_themePath + "Header.html");
	_statusMessage = removeBreaklines(loadFile(_themePath + "Status.html"));
	//_styleSheet = loadFile(_themePath + "main.css");
	//VOXOX CHANGE CJC
	//Load stuff from style.dat
	//QSettings s(_themePath + "style.dat", QSettings::IniFormat);
	//s.beginGroup("Style");
	//QString value = s.value("nextContentInsideContent", "N").toString();
	//	if(value=="Y"){
	//		_nextContentInsideContent = true;
	//	}else{
	//		_nextContentInsideContent = false;
	//	}



}

void QtChatTheme::initTemplate(){

	  QStringList templateList = _template.split("%@");
		/* There is 2 formats for Template.html: The old one has 4 parameters,
		+ * the new one has 5 parameters. */
	  if(templateList.count()==5){
		  templateList[0].append(_themePath);	//	<base href="%@">
		  templateList[1].append(getSelectedVariantPath());	//	<style id="mainStyle" type="text/css" media="screen,print">	@import url( "%@" ); </style>
		  templateList[2].append(_header);	//	<body onload="alignchat(true);" style="==bodyBackground==">%@
		  templateList[3].append(_footer);	//	%@</body>
	  }else if(templateList.count()==6){
		  QString mainCSS = loadFile(_themePath+"main.css");
		  templateList[0].append(_themePath);	//	<base href="%@">
		  templateList[1].append(mainCSS);	//<style id="baseStyle" type="text/css" media="screen,print">%@</style>
		  templateList[2].append(getSelectedVariantPath());	//	<style id="mainStyle" type="text/css" media="screen,print">	@import url( "%@" ); </style>
		  templateList[3].append(_header);	//	<body onload="alignchat(true);" style="==bodyBackground==">%@
		  templateList[4].append(_footer);	//	%@</body>
	  }

	  _template = templateList.join("");
	
}

void QtChatTheme::setKeywordValue(QString & html, const QString & keyword, const QString & value) {
	html.replace(KEYWORD_CHAR + keyword + KEYWORD_CHAR, value);
}


QString QtChatTheme::removeBreaklines(const QString & html){
	QString htmlToReturn = html;
	htmlToReturn = htmlToReturn.replace("\n","");
	htmlToReturn = htmlToReturn.replace("\r","");
	htmlToReturn = htmlToReturn.replace("\t","");
	return htmlToReturn;
}


QStringList QtChatTheme::getThemeList() {
	QDir dir(themeBaseDir());

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

QStringList QtChatTheme::getSelectedThemeVariantList(QString selectedTheme) {
	QString selectedThemePath = themeBaseDir() + selectedTheme + "/" + CONTENTS_PATH + RESOURCES_PATH;
	selectedThemePath = selectedThemePath.replace("\\","/");
	QDir dir(selectedThemePath + VARIANTS_PATH);

	QStringList list;
	Q_FOREACH(QFileInfo info, dir.entryInfoList()) {
		if (info.isFile()) {
			QString name = info.fileName();
			if (name[0] != '.' && name.endsWith(".css")) {
				//Remove css extrantion from name
				name = name.replace(".css","");
				list << name;
			}
		}
	}

	return list;
}
