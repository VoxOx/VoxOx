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
* @date 2010.01.11
*/


#include "stdafx.h"		//VOXOX - JRT - 2009.04.05
#include "QtUMTranslation.h"
#include "translation/QtTranslationManager.h"
#include <util/SafeDelete.h>
#include <util/Logger.h>
#include <control/profile/CUserProfile.h>
#include <control/contactlist/CContactList.h>
#include <presentation/qt/messagebox/QtVoxMessageBox.h>

#include <QtGui/QtGui>
QtUMTranslation::QtUMTranslation(CChatHandler & cChatHandler):
_cChatHandler(cChatHandler)
{		
		//VOXOX - CJC - 2010.01.18 TODO LOAD TRANSLATION VARIABLES FOR EACH CONTACT
		_mode = QtEnumTranslationMode::None;
		_wsTranslation = _cChatHandler.getCUserProfile().getWsTranslation();
		_wsTranslation->translationDoneEvent += boost::bind(&QtUMTranslation::messageTranslated, this,_1,_2,_3,_4,_5);
		_contactId = "";
}



QtUMTranslation::~QtUMTranslation() {
	
}

void QtUMTranslation::initTranslationSettings(const QString & contactId){

	_contactId = contactId;
	const MessageTranslation & translation = _cChatHandler.getMessageTranslationSettings(_contactId.toStdString());
	setTranslationSourceLocale(getLanguage(QString::fromStdString(translation.getSourceMessageTranslationLanguage())));
	setTranslationDestLocale(getLanguage(QString::fromStdString(translation.getDestMessageTranslationLanguage())));
	setTranslationMode(getMode(QString::fromStdString(translation.getMessageTranslationMode())));

}

void QtUMTranslation::saveTranslationSettings(){

	if(_contactId!= ""){

		MessageTranslation translation;
		translation.setSourceMessageTranslationLanguage(getTranslationSourceLocale().getTranslationName().toStdString());
		translation.setDestMessageTranslationLanguage(getTranslationDestLocale().getTranslationName().toStdString());
		translation.setMessageTranslationMode(QtEnumTranslationMode::toString(getTranslationMode()).toStdString());

		_cChatHandler.setMessageTranslationSettings(_contactId.toStdString(),translation);

	}

}


bool QtUMTranslation::isTranslationOn(){

	if(_mode == QtEnumTranslationMode::None){
		return false;
	}else{
		return true;
	}
}

bool QtUMTranslation::isTranslationForSendingMessages(){

	if(_mode == QtEnumTranslationMode::Outgoing || _mode == QtEnumTranslationMode::Both ){
		return true;
	}else{
		return false;
	}
}

bool QtUMTranslation::isTranslationForReceivingMessages(){

	if(_mode == QtEnumTranslationMode::Incoming || _mode == QtEnumTranslationMode::Both ){
		return true;
	}else{
		return false;
	}
}

bool QtUMTranslation::validateTranslationSettings(){

	if(_translationSourceLocate.isNull() && _translationDestLocate.isNull()){
		
		QtVoxMessageBox box(0);
		box.setWindowTitle(tr("VoxOx - No language selected"));
		box.setText(tr("You need to select the translation languages, please select your languange and your contact's language."));
		box.setStandardButtons(QMessageBox::Ok);
		box.exec();
		return false;
		
	}else if(_translationSourceLocate.isNull() && !_translationDestLocate.isNull()){

		QtVoxMessageBox box(0);
		box.setWindowTitle(tr("VoxOx - Language problem"));
		box.setText(tr("Please select your language."));
		box.setStandardButtons(QMessageBox::Ok);
		box.exec();
		return false;

	}else if(!_translationSourceLocate.isNull() && _translationDestLocate.isNull()){

		QtVoxMessageBox box(0);
		box.setWindowTitle(tr("VoxOx - Language problem"));
		box.setText(tr("Please select your contact's language."));
		box.setStandardButtons(QMessageBox::Ok);
		box.exec();
		return false;

	}else{
		return true;
	}

}

void QtUMTranslation::setTranslationDestLocale(QtTranslationLanguage  destLocale){
	_translationDestLocate = destLocale;
}

void QtUMTranslation::setTranslationSourceLocale(QtTranslationLanguage sourceLocale){

	_translationSourceLocate = sourceLocale;
}

QtTranslationLanguage QtUMTranslation::getTranslationDestLocale(){
	return _translationDestLocate;
}

QtTranslationLanguage QtUMTranslation::getTranslationSourceLocale(){
	return _translationSourceLocate;
}

QtEnumTranslationMode::Mode QtUMTranslation::getTranslationMode(){
	return _mode;
}

void QtUMTranslation::setTranslationMode(QtEnumTranslationMode::Mode mode){
	_mode = mode;
}


void QtUMTranslation::translateRecievedMessage(const QString & contactId, const QString & senderName, const QString & protocolName,const QString & message){

	QtUMTranslationRequest * request = new QtUMTranslationRequest(contactId,senderName,protocolName,message,QtUMTranslationRequest::RecieveMessage);
	_umRequestMap[request->getKey()] = request;
	std::string utf8Message = QString(message.toUtf8()).toStdString();//VOXOX - CJC - 2010.02.17 //VOXOX -ASV- 2010.02.17 fixed a compilation issue on mac where we can't assign an QByteArray to an std::string
	if(_wsTranslation){
		_wsTranslation->translate(request->getKey().toStdString(),utf8Message.c_str(),_translationDestLocate.getTranslationAbbreviation().toStdString(),_translationSourceLocate.getTranslationAbbreviation().toStdString());
	}
	
}

void QtUMTranslation::translateAndSendMessage(const QString & contactId, const QString & senderName, const QString & protocolName, const QString & message){
	
	QtUMTranslationRequest * request = new QtUMTranslationRequest(contactId,senderName,protocolName,message,QtUMTranslationRequest::SendMessage);
	_umRequestMap[request->getKey()] = request;
	std::string utf8Message = QString(message.toUtf8()).toStdString();//VOXOX - CJC - 2010.02.17 //VOXOX -ASV- 2010.02.17 fixed a compilation issue on mac where we can't assign an QByteArray to an std::string
	if(_wsTranslation){
		_wsTranslation->translate(request->getKey().toStdString(),utf8Message.c_str(),_translationSourceLocate.getTranslationAbbreviation().toStdString(),_translationDestLocate.getTranslationAbbreviation().toStdString());//VOXOX - CJC - 2010.02.17 Fix translation issue when translating languages like Rusian or Japanese(With not "regular" characters)
	}

}

//VOXOX - CJC - 2010.01.18 This is the return from the webservice
void QtUMTranslation::messageTranslated(WsTranslation & sender,const std::string id,const std::string originalText,
			const std::string & translatedText,bool success)
{
	processTranlatedMessage(id,originalText,translatedText,success);
}


void QtUMTranslation::processTranlatedMessage(const std::string id,const std::string originalText,const std::string & translatedText,bool success){

	QtUMTranslationRequest * request = getTranslationRequest(QString::fromStdString(id));
	if(request){

		QString contactId = request->getContactId();
		QString senderName =  request->getSenderName();
		QString protocolName =  request->getProtocolName();
		QString originalMessage = request->getOriginalMessage();//VOXOX - CJC - 2010.01.18 We could also get this from the parameter, lets try it like this for now
		QString translatedMessage = QString::fromUtf8(translatedText.c_str());
		bool sendMessageToSession;
		
		if(request->getTranslationRequestMode() == QtUMTranslationRequest::SendMessage){
			
			sendMessageToSession =  true;

		}else if(request->getTranslationRequestMode() == QtUMTranslationRequest::RecieveMessage){

			sendMessageToSession =  false;
		}

		translateMessageSignal(contactId,senderName,protocolName,originalMessage,translatedMessage,success,sendMessageToSession);
							   
		removeTranslationRequest(QString::fromStdString(id));//VOXOX - CJC - 2010.01.18 Clean request after proccesing it

	}else{
		LOG_DEBUG("CANT FIND TRASLATION REQUEST,MAYBE ANOTHER CHAT PRODUCE THIS?");
		if(!success && id == "" ){
			LOG_DEBUG("WEBSERVICE RETURN IS EMPTY NOTIFYING UI?");
			translateErrorSignal();
		}
	}

}


QtUMTranslationRequest * QtUMTranslation::getTranslationRequest(const QString & key){
	UMTranslationRequestMap::iterator iter = _umRequestMap.find(key);

	if (iter != _umRequestMap.end()) 
	{
		 return iter->second;
	}

	return NULL;
}

void QtUMTranslation::removeTranslationRequest(const QString & key){

	UMTranslationRequestMap::iterator iter = _umRequestMap.find(key);//Check if item exists

	if (iter != _umRequestMap.end()) {

		OWSAFE_DELETE(iter->second);
		_umRequestMap.erase(iter);
	}

}

QtTranslationLanguage QtUMTranslation::getLanguage(const QString & text){

	QtTranslationManager * qtTranslationManager = QtTranslationManager::getInstance(QString::fromStdString(_cChatHandler.getTranslationLanguageXML()));//VOXOX - CJC - 2010.01.28 
	QtTranslationLanguage language = qtTranslationManager->getTranslationLanguage(text);
	return language;

}

QtEnumTranslationMode::Mode QtUMTranslation::getMode(const QString & text){

	QtEnumTranslationMode::Mode mode = QtEnumTranslationMode::toTranslationMode(text);
	return mode;

}
