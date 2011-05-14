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

#include "stdafx.h"	//VOXOX - JRT - 2009.04.01
#include "History.h"

#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/contactlist/Contact.h>
#include <model/contactlist/ContactList.h>
#include <model/profile/UserProfile.h>
#include <model/phoneline/IPhoneLine.h>

#include <util/Logger.h>
#include <util/File.h>
#include <util/Path.h>
#include <util/Date.h>
#include <util/Time.h>
#include <util/SafeDelete.h>

#include <sstream>
#include <exception>
#include <tinyxml.h>


History::History(UserProfile & userProfile)
	: _userProfile(userProfile) {

	_collection = new HistoryMementoCollection();
	_missedCallCount = 0;
}

History::~History() {
}

HistoryMementoCollection * History::getHistoryMementoCollection() {
	return _collection;
}

unsigned int History::addMemento(HistoryMemento * memento) {
	unsigned id = _collection->addMemento(memento);
	mementoAddedEvent(*this, id);
	return id;
}

void History::updateCallDuration(int callId, int duration) {
	HistoryMemento * memento = _collection->getMementoByCallId(callId);
	if (memento) {
		memento->updateDuration(duration);
		unsigned id = _collection->getMementoId(memento);
		mementoUpdatedEvent(*this, id);
	}
}

void History::updateSMSState(int callId, HistoryMemento::State state) {
	HistoryMemento * memento = _collection->getMementoBySMSId(callId);
	if (memento) {
		memento->updateState(state);
		unsigned id = _collection->getMementoId(memento);
		mementoUpdatedEvent(*this, id);
	}
}

void History::updateCallState(int callId, HistoryMemento::State state) {
	HistoryMemento * memento = _collection->getMementoByCallId(callId);
	if (memento) {
		memento->updateState(state);
		unsigned id = _collection->getMementoId(memento);
		mementoUpdatedEvent(*this, id);

		if (state == HistoryMemento::MissedCall) {
			_missedCallCount++;
			unseenMissedCallsChangedEvent(*this, _missedCallCount);
		}
	}
}

unsigned int History::size() const {
	return _collection->size();
}

std::string History::serialize() {
	std::stringstream ss;
	try {
		boost::archive::xml_oarchive oa(ss);
		oa << BOOST_SERIALIZATION_NVP(_collection);
		oa << BOOST_SERIALIZATION_NVP(_missedCallCount);
	} catch (boost::archive::archive_exception & e) {
		LOG_DEBUG(e.what());
		return String::null;
	}
	return ss.str();
}

bool History::unserialize(const std::string & data) {
	std::stringstream ss(data);
	try {
		boost::archive::xml_iarchive ia(ss);
		ia >> BOOST_SERIALIZATION_NVP(_collection);
		ia >> BOOST_SERIALIZATION_NVP(_missedCallCount);
	} catch (boost::archive::archive_exception & e) {
		LOG_DEBUG(e.what());
		return false;
	}
	return true;
}

void History::clear(HistoryMemento::State state, bool notify) {
	_collection->clear(state);
	if (notify) {
		mementoRemovedEvent(*this, 0);
	}
}

std::string History::toString() const {
	return _collection->toString();
}

void History::removeMemento(unsigned id) {
	_collection->removeMemento(id);
	mementoRemovedEvent(*this, id);
}

HistoryMemento * History::getMemento(unsigned id) {
	Mutex::ScopedLock scopedLock(_mutex);

	return _collection->getMemento(id);
}

HistoryMementoCollection * History::getMementos(HistoryMemento::State state, int count) {
	Mutex::ScopedLock scopedLock(_mutex);

	return _collection->getMementos(state, count);
}

bool History::load(const std::string & url) {
	FileReader file(url);

	if (file.open()) {
		std::string data = file.read();

		//clear & unserialize the history
		clear(HistoryMemento::Any, false);
		bool toReturn = unserialize(data);
		historyLoadedEvent(*this);

		return toReturn;
	}

	return true;
}

bool History::save(const std::string & url) {
	FileWriter file(url);

	file.write(serialize());

	historySavedEvent(*this);

	return true;
}

void History::replay(unsigned id) {

	//replay only outgoing call
	if (getMemento(id)->isCallMemento()) {
		std::string phoneNumber = getMemento(id)->getPeer();
		_userProfile.getActivePhoneLine()->makeCall(phoneNumber);
	}
}

void History::resetUnseenMissedCalls() {
	_missedCallCount = 0;
	unseenMissedCallsChangedEvent(*this, _missedCallCount);
}

int History::getUnseenMissedCalls() {
	Mutex::ScopedLock scopedLock(_mutex);

	return _missedCallCount;
}
bool History::loadChatLog(std::string chatlog, HistoryMementoCollection * hmc, std::string * userlogin, StringList * cuuidList) {

	// complete filename
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	std::string filename = File::convertPathSeparators( 
		config.getConfigDir() + "chatlogs" + File::getPathSeparator() 
		+ _userProfile.getName() + File::getPathSeparator()
		+ chatlog + ".xml"
	);
	////

	//open and read chat log
	FileReader file( filename );
	if(!file.open()) {
		return false;
	}
	std::string lu = file.read();
	std::stringstream ss(lu);
	boost::archive::xml_iarchive ia(ss);
	////

	// contact
	int nbcontact = 0;
	ia >> BOOST_SERIALIZATION_NVP(nbcontact);
	for( int ic = 0; ic < nbcontact; ++ic) {
		std::string cuuid;
		ia >> BOOST_SERIALIZATION_NVP(cuuid);
		cuuidList->push_back(cuuid);
	}

	//user login
	ia >> BOOST_SERIALIZATION_NVP(*userlogin);

	//number of message
	int size = 0;
	ia >> BOOST_SERIALIZATION_NVP(size);

	//load every message
	std::string date, peer, data;
	for(int i = 0; i < size; ++i) {

		//
		//	more compact serialization
		//
		//ia >> BOOST_SERIALIZATION_NVP(date);
		//ia >> BOOST_SERIALIZATION_NVP(peer);
		//ia >> BOOST_SERIALIZATION_NVP(data);

		//Date fdate(	String(date.substr(8,2)).toInteger(), 
		//			String(date.substr(5,2)).toInteger(), 
		//			String(date.substr(0,4)).toInteger());

		//Time ftime(	String(date.substr(11,2)).toInteger(), 
		//			String(date.substr(14,2)).toInteger(), 
		//			String(date.substr(17,4)).toInteger());

		//HistoryMemento* hm = new HistoryMemento(HistoryMemento::ChatSession, fdate, ftime, peer, -1, data);

		HistoryMemento* hm = new HistoryMemento();
		hm->load(ia,HistoryMemento::SERIALIZATION_VERSION);

		/** duration -1 means it is an history message */
		hm->updateDuration(-1);

		hmc->addMemento(hm);
	}
	return true;
}

bool History::addChatMementoSession(IMChatSession * imchatSession) {
	int chatSessionID = imchatSession->getId();
	if(_chatSessionsMementos[chatSessionID] == NULL){
		_chatSessionsMementos[chatSessionID] = new HistoryMementoCollection();

/**	reload chat log	(lost feature)	***/
//
//		//load last chat log
//		IMContactSet contactSet = imchatSession->getIMContactSet();
//
//		// TO DO loop on all contacts ?? for now it could only be 1 contact...
//		IMContactSet::const_iterator itc = contactSet.begin();
//		////
//		Contact * thecontact = _userProfile.getContactList().findContactThatOwns(*itc);
//		if(!thecontact) {
//			return true;
//		}
//		std::string repload = File::convertPathSeparators( 
//			Path::getWengoConfigurationDirPath() + "chatlogs" + File::getPathSeparator() 
//			+ _userProfile.getName() + File::getPathSeparator()
//		);
//		FileReader filec( repload + thecontact->getUUID() + ".xml" );
//		if(!filec.open()) {
//			return true;
//		}
//		std::string readdatas = filec.read();
//
//		//parse datas
//		TiXmlDocument doc;
//		doc.Parse(readdatas.c_str());
//		TiXmlHandle docHandle(&doc);
//
//		TiXmlNode * lastChat= NULL;
//		while ((lastChat = docHandle.ToNode()->IterateChildren("chat", lastChat))) {
//			std::string filechat(lastChat->FirstChild("id")->ToElement()->GetText());
//
//			std::string username;
//			StringList cuuidList;
//			if (!loadChatLog(filechat,_chatSessionsMementos[chatSessionID], &username, &cuuidList)) {
//				continue;
//			}
//		}
/**********/
		
		return true;
	}
	return false;
}

void History::removeChatMementoSession(IMChatSession * imchatSession) {
	int chatSessionID = imchatSession->getId();

	HistoryMementoCollection * collection = NULL;
	if((collection = _chatSessionsMementos[chatSessionID]) != NULL){
		//seek for history chat
		int nbhistory = 0;
		HistoryMap::iterator ithm;
		for (ithm = collection->begin(); ithm != collection->end(); ++ithm) {
			HistoryMemento* hm = ithm->second;

			// duration -1 means it is an history message
			if( hm->getDuration() != -1 ) {
				break;
			}
			++nbhistory;
		}
		////

		// don't save empty chat history
		int size = collection->size() - nbhistory;
		if(size>0) {
			//save chat log
			Date saveDate;
			Time saveTime;
			std::string peer = "";
			std::string filechat =	String::fromNumber(saveDate.getYear(), 2) + String::fromNumber(saveDate.getMonth(), 2) + 
									String::fromNumber(saveDate.getDay(), 2) + String::fromNumber(saveTime.getHour(), 2) + 
									String::fromNumber(saveTime.getMinute(), 2) + String::fromNumber(saveTime.getSecond(), 2)+
									"_" + String::fromNumber(chatSessionID);
			Config & config = ConfigManager::getInstance().getCurrentConfig();
			std::string saverep = File::convertPathSeparators( 
				config.getConfigDir() + "chatlogs" + File::getPathSeparator() 
				+ _userProfile.getName() + File::getPathSeparator() 
			);
			File::createPath(saverep);
			//save file should be unique
			while(File::exists(saverep + filechat + ".xml")) {
				filechat += "_f";
			}
			////
			FileWriter file(saverep + filechat+".xml");
			std::stringstream ss;
			bool serializedSuccessfully = false;
			try {
				boost::archive::xml_oarchive oa(ss);

				//constructs list of login per peer
				std::map<std::string, std::vector<std::string>*> aliasMap;
				IMContactSet contactSet = imchatSession->getIMContactSet();
				for (IMContactSet::const_iterator itc = contactSet.begin(); itc != contactSet.end(); ++itc) {
					Contact * thecontact = _userProfile.getContactList().findContactThatOwns(*itc);
					std::string cuuid = "unrecognized";
					if(thecontact) {
//						cuuid = thecontact->getUUID();
						cuuid = thecontact->getKey();	//VOXOX - JRT - 2009.04.28 
					}	

					if(aliasMap[cuuid] == NULL) {
						aliasMap[cuuid] = new std::vector<std::string>;
					}
//					aliasMap[cuuid]->push_back(itc->cleanContactId());	//VOXOX - JRT - 2009.04.10 
					aliasMap[cuuid]->push_back(itc->getCleanContactId());
				}
				////

				// saves number of peer in this chat
				int nbcontact = aliasMap.size();
				oa << BOOST_SERIALIZATION_NVP(nbcontact);
				////

				//links all peers to this chat
				for(std::map<std::string, std::vector<std::string>*>::const_iterator itam = aliasMap.begin();
					itam != aliasMap.end(); ++itam) {
				
					/** links peer -> chat */

					//filechat
					std::string tobewritten = "<chat>\n\t<id>"+filechat+"</id>\n";

					//different login used by this peer during this chat
					for(std::vector<std::string>::const_iterator itv = itam->second->begin(); itv != itam->second->end(); ++itv) {
						tobewritten += "\t<alias>" + (*itv) + "</alias>\n";
						
						peer += "," + (*itv);
					}
					
					////

					tobewritten += "</chat>\n";
					/////
	
					std::string cuuid = itam->first;
					FileWriter contactFile( saverep + cuuid + ".xml" );
					contactFile.setAppendMode(true);
					contactFile.write(tobewritten);

					/** links chat -> peer */
					oa << BOOST_SERIALIZATION_NVP(cuuid);
				}
				////

				// saves user login for this chat
				IMAccount * imAccount =
					_userProfile.getIMAccountManager().getIMAccount(imchatSession->getIMChat().getIMAccountId());
				std::string userlogin;
				if (imAccount) {
					userlogin = imAccount->getLogin();
					OWSAFE_DELETE(imAccount);
				} else {
					LOG_ERROR("cannot find the IMAccount");
				}
				oa << BOOST_SERIALIZATION_NVP(userlogin);
				////

				// saves size
				oa << BOOST_SERIALIZATION_NVP(size);

				// save all historymementos i.e. all message
				for (;ithm != collection->end(); ++ithm) {
					HistoryMemento* hm = ithm->second;

					//
					//	more compact serialization
					//
					//std::string date = hm->getDate().toString() + " " + hm->getTime().toString();
					//oa << BOOST_SERIALIZATION_NVP(date);
					//std::string peer = hm->getPeer();
					//oa << BOOST_SERIALIZATION_NVP(peer);
					//std::string data = hm->getData();
					//oa << BOOST_SERIALIZATION_NVP(data);

					hm->save(oa,HistoryMemento::SERIALIZATION_VERSION);

				}
				serializedSuccessfully = true;
			} catch (boost::archive::archive_exception & e) {
				LOG_DEBUG(e.what());
				file.write(String::null);
			}

			if (serializedSuccessfully) {
				// xml_oarchive write the end of the xml in its destructor.
				// This is why we do not write inside the try {} catch block
				// because we would write before the xml_oarchive object is
				// deleted.
				file.write(ss.str());
			}
			
			if (peer.size() > 1) {
				peer = peer.substr(1);
			}
			
			HistoryMemento * historyMemento = new HistoryMemento(HistoryMemento::ChatSession, saveDate, saveTime, peer, -1, filechat);
			historyMemento->updateDuration(0);
			_userProfile.getHistory().addMemento(historyMemento);
		}
		_chatSessionsMementos.erase(chatSessionID);
		delete collection;
	}
}

unsigned History::addChatMemento(HistoryMemento * memento, int chatSessionID) {
	unsigned id = _chatSessionsMementos[chatSessionID]->addMemento(memento);
	chatMementoAddedEvent(*this, id);
	return id;
}

HistoryMementoCollection * History::getSessionCollection(int chatSessionID) {
	return _chatSessionsMementos[chatSessionID];
}
