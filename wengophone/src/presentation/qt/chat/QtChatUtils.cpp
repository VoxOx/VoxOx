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
#include "QtChatUtils.h"

#include "emoticons/QtEmoticonsManager.h"
#include <imwrapper/QtEnumIMProtocol.h>

#include <QtGui/QTextDocument>

static QString CHAT_TO_EMAIL_KEY = "aa733115-828c-102c-8ced-001ec9b74ff9";
static QString CHAT_TO_SMS_KEY = "d0f1e09b-9b9a-102c-af39-001ec9b74ff9";


const QString QtChatUtils::replaceUrls(const QString & str, const QString & htmlstr) {
	int beginPos = 0;
	int find = 0;
	QString tmp = htmlstr;
	int endPos;
	int repCount = 0;

	//VOXOX - CJC - 2009.06.25 
	//Check if string contains HREF, if so, get out, this is not needed
	find = htmlstr.indexOf(QRegExp("<a[\\s]+[^>]*href[\\s]?=.*[^<]+</a>", Qt::CaseInsensitive), beginPos);
	if (find != -1) {
		return htmlstr;
	}


	QStringList urls;
	QStringList reps;
	while (true) {

		

		beginPos = str.indexOf(QRegExp("(http://|https://|ftp://|^www.)", Qt::CaseInsensitive), beginPos);
		if (beginPos == -1) {
			break;
		}

		for (endPos = beginPos; endPos < str.size(); endPos++) {
			if ((str[endPos] == ' ') || (str[endPos] == '\r') || (str[endPos] == '\n')) {
				break;
			}
		}
		QString url = str.mid(beginPos, endPos - beginPos);
		urls << url;
		QString r = QString("$$_$$*_VOXOXTRUTILS_|KB|%1").arg(repCount);
		reps << r;
		repCount++;
		tmp.replace(url, r);
		beginPos = endPos;
	}
	for (int i = 0; i < reps.size(); i++) {
		if(urls[i].startsWith("www.")){//VOXOX CHANGE by Rolando - 2009.10.21 
			urls[i].insert(0,"http://");//VOXOX CHANGE by Rolando - 2009.10.21 
		}
		QString url = QString("<a href='" + urls[i]+ "'>" + urls[i] + "</a>");
		tmp.replace(reps[i], url);
	}
	return tmp;
}

//VOXOX CHANGE CJC CATCH IMAGES AND SHOW THEM PROPERLY
const QString QtChatUtils::replaceImgUrls(const QString & htmlstr) {

	QString returnURL = htmlstr;
	QString textToReplace;

	//This expresion will only work if images come inside href tags (VoxOx, MSN only tested that does)
	QRegExp * regExp = new QRegExp("http\\:\\/\\/[^<|>]+\\.(?:jpg|jpeg|gif|png)");
	
	if(regExp->indexIn(htmlstr) != -1){

		QString findImgUrl = regExp->cap();	

		textToReplace = checkIfHTMLIsInHREF(returnURL,findImgUrl);

		returnURL = returnURL.replace(textToReplace,QString("<img src=\"%1\" />").arg(findImgUrl));
	
	}

	return returnURL;
}

const QString QtChatUtils::checkIfHTMLIsInHREF(const QString & fullHtml,const QString & concatHtml)
{	
		QString textToReplace = concatHtml; 
		QRegExp * findHref = new QRegExp("<a[\\s]+[^>]*href[\\s]?=.*[^<]+</a>");

		if(findHref->isValid()){
		//Check if we have the stuff inside and href link
		if(findHref->indexIn(fullHtml) != -1){

			QString href = findHref->cap();
			//Check if we have the image inside an href link
			if(href.contains(concatHtml)){
				textToReplace = href;
			}
		}
		}
		return textToReplace;
}



//VOXOX CHANGE CJC CATCH YOUTUBE AND SHOW THEM PROPERLY
const QString QtChatUtils::replaceYoutubeAddress(const QString & htmlstr) {

	QString returnURL = htmlstr;

	QString stringToReplace;

	//FULL YOUTUBE ADDRESS
	QRegExp * regExpFullYouTube = new QRegExp("http:\\/\\/www\\.youtube\\.com\\/watch\\?v=([A-Za-z0-9._%-]*)[&\\w;=\\+_\\-]*");
	//YOUTUBE ADDRESS WITH NO STUFF AFTER YOUTUBE VIDEO CODE
	QRegExp * regExpConcatYouTube = new QRegExp("http:\\/\\/www\\.youtube\\.com\\/watch\\?v=([A-Za-z0-9._%-]*)");

	if(regExpFullYouTube->indexIn(htmlstr) != -1){
		if(regExpConcatYouTube->indexIn(htmlstr) != -1){

			QString youTubeCode = regExpConcatYouTube->cap();	

			QString youTubeFullCode= regExpFullYouTube->cap();

			stringToReplace = checkIfHTMLIsInHREF(returnURL,youTubeFullCode);

			//Youtube address
			youTubeCode = youTubeCode.replace("watch?v=","v/");

		
			QString youTubeEndendedVideo = QString("<object width=\"480\" height=\"295\"><param name=\"movie\" value=\"%1&hl=en&fs=1\"></param><param name=\"allowFullScreen\" value=\"true\"></param><param name=\"allowscriptaccess\" value=\"always\"></param><embed src=\"%1&hl=en&fs=1\" type=\"application/x-shockwave-flash\" allowscriptaccess=\"always\" allowfullscreen=\"true\" width=\"480\" height=\"295\"></embed></object>").arg(youTubeCode);

			returnURL = returnURL.replace(stringToReplace,youTubeEndendedVideo);
		}
	
	}

	return returnURL;
}



const QString QtChatUtils::text2Emoticon(const QString & htmlstr) {
	QtEmoticonsManager * emoticonsManager = QtEmoticonsManager::getInstance();
	return emoticonsManager->text2Emoticon(htmlstr);
}

const QString QtChatUtils::emoticons2Text(const QString & htmlstr) {
	QtEmoticonsManager * emoticonsManager = QtEmoticonsManager::getInstance();
	return emoticonsManager->emoticons2Text(htmlstr);
}

const QString QtChatUtils::removeFontTags(const QString & htmlstr) {
	
	int beginPos = 0;
	QString tmp = htmlstr;
	beginPos = tmp.indexOf(QRegExp("(#){1}([a-fA-F0-9]){6}", Qt::CaseInsensitive), beginPos);
	if (beginPos == -1) {
		return htmlstr;
	}

	QString returnValue = tmp.replace(beginPos,7,"");
	return returnValue;
}

//const QString QtChatUtils::encodeMessage(EnumIMProtocol::IMProtocol protocol, const QString & message) 
const QString QtChatUtils::encodeMessage( const QString & message )		//VOXOX - JRT - OK - 2010.01.06 - protocol param is not used.
{
//	return emoticons2Text(message, getProtocol(protocol));
//	QString strProtocol = QtEnumIMProtocolMap::getInstance().toString( protocol ).c_str();	//VOXOX - JRT - OK - 2010.01.06 - Not used.
	return emoticons2Text(message);
}

const QString QtChatUtils::encodeMessageChatToEmail(const QString & toEmail,const QString & from, const QString & subject, const QString & message) 
{
	QString finalMessage = "";
	finalMessage += "-a c2e.Rest.index -p ";
	finalMessage += "\"key=%1&";
	finalMessage += "method=sendMessage&to=%2&from=%3&";
	finalMessage += "subject=%4&message=%5\"";
	finalMessage = finalMessage.arg(CHAT_TO_EMAIL_KEY).arg(toEmail).arg(from).arg(subject).arg(message);

	return finalMessage;
}
//VOXOX - CJC - 2009.06.15 
const QString QtChatUtils::encodeMessageChatToSMS(const QString & toNumber,const QString & from, const QString & message) 
{
	QString finalMessage = "";
	finalMessage += "-a vsms.Rest.index -p ";
	finalMessage += "\"key=%1&";
	finalMessage += "method=sendSMS&userId=%2&to=%3&";
	finalMessage += "body=%4\"";
	finalMessage = finalMessage.arg(CHAT_TO_SMS_KEY).arg(from).arg(toNumber).arg(message);

	return finalMessage;
}





const QString QtChatUtils::decodeMessage(const QString & message) {
	QTextDocument tmp;
	tmp.setHtml(message);

	
	// Replace URLS
	QString toReturn = replaceUrls(tmp.toPlainText(), message);

	//Replace Images
	toReturn = replaceImgUrls(toReturn);

	//toReturn = replaceYoutubeAddress(toReturn);
	//VOXOX CHANCE CJC We only want to use the font color from the selected style.
	toReturn = removeFontTags(toReturn);
	
	toReturn = text2Emoticon( toReturn );
	//End VoxOx

	return toReturn;
}


const QString QtChatUtils::fixHTMLforJavascript(const QString & message) {
	
	QString toReturn = message;

	toReturn = toReturn.replace("\\\'", "'");//VOXOX - CJC - 2009.10.07 If we have \' (Formated apostrofee) Allready on the message, lets set it as a normal one, (This is faster the changing and validating everyone independently).
	
	toReturn = toReturn.replace("'", "\\\'");

	toReturn = removeBreaklines(toReturn);//VOXOX - CJC - 2009.09.17 We need this because we are sending this to javascript and the string cant contain any breaklines.

	return toReturn;
}


const QString QtChatUtils::removeBreaklines(const QString & html){
	QString htmlToReturn = html;
	htmlToReturn = htmlToReturn.replace("\n","");
	htmlToReturn = htmlToReturn.replace("\r","");
	htmlToReturn = htmlToReturn.replace("\t","");
	return htmlToReturn;
}