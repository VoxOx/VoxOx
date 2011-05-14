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

#include "stdafx.h"	//VOXOX - JRT - 2009.04.01
#include "HistoryMementoCollection.h"

#include <util/String.h>
#include <util/Logger.h>

unsigned int HistoryMementoCollection::_historyId = 0;

HistoryMementoCollection::HistoryMementoCollection() {
}

HistoryMementoCollection::~HistoryMementoCollection() {
}

unsigned int HistoryMementoCollection::addMemento(HistoryMemento * memento) {
	_privateCollection[_historyId] = memento;
	_historyId++;
	return _historyId - 1;
}

HistoryMemento * HistoryMementoCollection::getMementoByCallId(int callId) {
	if( callId != -1 ) {
		HistoryMap::iterator it;
		for(it = _privateCollection.begin(); it != _privateCollection.end(); it++) {
			if( ((*it).second->isCallMemento()) &&
				((*it).second->getCallId() == callId) ) {
				return (*it).second;
				}
		}
	}
	return NULL;
}

HistoryMemento * HistoryMementoCollection::getMementoBySMSId(int callId) {
		if( callId != -1 ) {
		HistoryMap::iterator it;
		for(it = _privateCollection.begin(); it != _privateCollection.end(); it++) {
			if( ((*it).second->isSMSMemento()) &&
				((*it).second->getCallId() == callId)) {
				return (*it).second;
			}
		}
	}
	return NULL;
}

unsigned int HistoryMementoCollection::size() {
	return _privateCollection.size();
}

HistoryMemento * HistoryMementoCollection::getMemento(unsigned int id) {
	HistoryMap::iterator it = _privateCollection.find(id);
	if (it != _privateCollection.end()) {
		return it->second;
	} else {
		return NULL;
	}
}

void HistoryMementoCollection::removeMemento(unsigned int id) {
	_privateCollection.erase(_privateCollection.find(id));
}

void HistoryMementoCollection::clear(HistoryMemento::State state) {

	int id = -1;
	HistoryMemento * ref = NULL;

	HistoryMap::iterator it;
	for(it = _privateCollection.begin(); it != _privateCollection.end(); it++) {

		if( ref ) {
			delete ref;
			_privateCollection.erase(id);
		}

		if( ((*it).second->getState() == state) || (state == HistoryMemento::Any)) {

			id = (*it).first;
			ref = (*it).second;

		} else {
			ref = NULL;
		}
	}

	if(ref) {
		delete ref;
		_privateCollection.erase(id);
	}
}

void HistoryMementoCollection::setMaxEntries(unsigned int max) {
	LOG_WARN("Not implemented yet");
}

void HistoryMementoCollection::privateAdd(unsigned int id, HistoryMemento * memento) {
	_privateCollection[id] = memento;
}

HistoryMementoCollection * HistoryMementoCollection::getMementos(HistoryMemento::State state, int count, bool noDuplicates) {

	int c = 0;
	HistoryMementoCollection * toReturn = new HistoryMementoCollection();

	HistoryMap::reverse_iterator it;
	for (it = _privateCollection.rbegin(); it != _privateCollection.rend(); it++) {

		if ((state == HistoryMemento::Any) || ((*it).second->getState() == state)) {

			if (noDuplicates) {
				if (HistoryMementoCollection::isPeerInCollection(toReturn, ((*it).second)->getPeer())) {
					continue;
				}
			}

			toReturn->privateAdd((*it).first, (*it).second);
			c++;
		}

		if (c >= count) {
			break;
		}
	}
	return toReturn;
}

HistoryMap::iterator HistoryMementoCollection::begin() {
	return _privateCollection.begin();
}

HistoryMap::iterator HistoryMementoCollection::end() {
	return _privateCollection.end();
}

std::string HistoryMementoCollection::toString() {
	std::string toReturn = "_historyId: " + String::fromNumber(_historyId) + "\n";

	HistoryMap::iterator it;
	for (it = _privateCollection.begin(); it != _privateCollection.end(); it++) {
		toReturn += ( "id: " + String::fromNumber((*it).first) + "/" + (*it).second->toString() + "\n");
	}
	return toReturn;
}

unsigned int HistoryMementoCollection::getMementoId(HistoryMemento * memento) {
	HistoryMap::iterator it;
	for(it = _privateCollection.begin(); it != _privateCollection.end(); it++) {
		if( (*it).second == memento ) {
			return (*it).first;
		}
	}
	return 0;
}

bool HistoryMementoCollection::isPeerInCollection(HistoryMementoCollection * collection, const std::string & peer) {
	HistoryMap::iterator it;
	for (it = collection->begin(); it != collection->end(); it++) {
		HistoryMemento * memento = (*it).second;
		if ((memento->getPeer() == peer)) {
			return true;
		}
	}
	return false;
}
