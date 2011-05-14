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

#ifndef OWHISTORY_H
#define OWHISTORY_H

#include "HistoryMemento.h"
#include "HistoryMementoCollection.h"

#include <util/Event.h>
#include <serialization/Serializable.h>
#include <thread/Mutex.h>
#include <imwrapper/IMChatSession.h>

#include <map>
#include <string>

class UserProfile;
class IMContactSet;

/**
 * History (Care Taker in the Memento pattern)
 *
 * example: serialization example
 *	<pre>
 *	std::ofstream ofs("history.log");
 *	ofs << History::history.serialize();
 *	ofs.close();
 *	</pre>
 *
 * example: unserialize a History
 *	<pre>
 *	std::string lines;
 *
 *	// assum you have read the save file
 *	// and you have the content of this file
 *	// in the std::string lines
 *
 *	History::history.unserialize(lines);
 *	</pre>
 *
 * @author Mathieu Stute
 */
class History : public Serializable {
public:

	/**
	 * The history has been loaded.
	 */
	Event<void (History &)> historyLoadedEvent;

	/**
	 * The history has been saved.
	 */
	Event<void (History &)> historySavedEvent;

	/**
	 * A memento has been added.
	 */
	Event<void (History &, unsigned int id)> mementoAddedEvent;

	/**
	 * A chat memento has been added.
	 */
	Event<void (History &, unsigned int id)> chatMementoAddedEvent;

	/**
	 * A memento has been updated.
	 */
	Event<void (History &, unsigned int id)> mementoUpdatedEvent;

	/**
	 * A memento has been removed.
	 */
	Event<void (History &, unsigned int id)> mementoRemovedEvent;

	/**
	 * Unseen missed calls changed event.
	 */
	Event<void (History &, int count)> unseenMissedCallsChangedEvent;

	/**
	 * Default constructor
	 *
	 * @param wengophone a pointer to WengoPhone
	 */
	History(UserProfile & userProfile);

	/**
	 * destructor.
	 */
	~History();

	bool load(const std::string & url);

	bool save(const std::string & url);

	/**
	 * get a HistoryMemento by its id.
	 *
	 * @param id the id
	 * @return the HistoryMemento with the given id
	 */
	HistoryMemento * getMemento(unsigned id);

	/**
	 * get a HistoryMementoCollection containing all mementos
	 * that match state.
	 *
	 * @param state filter memento by State
	 * @param count specify number of mementos to be returned
	 * @return a pointer to a HistoryMementoCollection
	 */
	HistoryMementoCollection * getMementos(HistoryMemento::State state, int count = -1);

	/**
	 * remove a memento by its id.
	 *
	 * @param id the id
	 */
	void removeMemento(unsigned id);

	/**
	 * Clear entries of the history
	 * @param state type of entries to remove
	 */
	void clear(HistoryMemento::State state = HistoryMemento::Any, bool notify = true);

	/**
	 * return the number of HistoryMemento's.
	 *
	 * @return the number of HistoryMemento's
	 */
	unsigned size() const;

	/**
	 * add a history memento to the history.
	 *
	 * @param memento the memento to add
	 * @return the id of the memento
	 */
	unsigned addMemento(HistoryMemento * memento);

	/**
	 * Return a string representing this object.
	 *
	 * @return a string representing this object
	 */
	std::string toString() const;

	/**
	 * return a pointer to the HistoryMementoCollection.
	 *
	 * @return a pointer to the HistoryMementoCollection
	 */
	HistoryMementoCollection * getHistoryMementoCollection();

	/**
	 * update a memento duration (for incoming/outgoing calls).
	 *
	 * @param callId callId associated to the memento
	 * @param duration duration
	 */
	void updateCallDuration(int callId, int duration);

	/**
	 * update a memento state (for incoming/outgoing calls).
	 *
	 * @param callId callId associated to the memento
	 * @param state the new state
	 */
	void updateCallState(int callId, HistoryMemento::State state);

	/**
	 * update a memento internal state (for SMS).
	 *
	 * @param callId callId associated to the memento
	 * @param state new state
	 */
	void updateSMSState(int callId, HistoryMemento::State state);

	/**
	 * update a memento internal state (for SMS).
	 *
	 * @param id memento to replay
	 */
	void replay(unsigned id);

	/**
	 * Reset unseen missed calls.
	 *
	 */
	void resetUnseenMissedCalls();

	/**
	 * Returns the unseen missed calls count.
	 *
	 * @return unseen missed calls count
	 */
	int getUnseenMissedCalls();
	
	/**
	 * Loads a collection of History from a saved chat log
	 *
	 *@param chatlog name of the file where the chat log is saved
	 *@param hmc HistoryMementoCollection to complete with chat messages
	 *@param userlogin login used by the user
	 *@return <code>true</code> if chat log has been loaded
	 */
	 bool loadChatLog(std::string chatlog, HistoryMementoCollection * hmc, std::string * userlogin, StringList * cuuidList);
	
	/**
	 * Creates a new collection of History Memento for new a chat session
	 *
	 * @param	chatSessionID : the ID of the chat sesssion
	 * @return	true if the session has been added; false else
	 */
	bool addChatMementoSession(IMChatSession * imchatSession);

	/**
	 * Removes a collection of History Memento of a closing chat session
	 *
	 * @param	chatSessionID : the ID of the chat sesssion
	 */
	void removeChatMementoSession(IMChatSession * imchatSession);

	/**
	 * add memento into the right chat memento collection
	 *
	 * @param memento the memento to add
	 * @param chatSessionID the ID of the chat sesssion to which the memento belongs
	 * @return the id of the memento
	 */
	unsigned addChatMemento(HistoryMemento * memento, int chatSessionID);

	/**
	 * Access to the collection corresponding to chatSessionID
	 *
	 * @param chatSessionID : the ID of the chat sesssion
	 * @return pointor to the HistoryMementoCollection of te chat session
	 */
	HistoryMementoCollection * getSessionCollection(int chatSessionID);

private:

	/**
	 * Serialize the history.
	 */
	std::string serialize();

	/**
	 * Serialize the history.
	 */
	bool unserialize(const std::string & data);

	/**
	 * Private HistoryMementoList that store
	 * all HistoryMemento objects.
	 */
	HistoryMementoCollection *_collection;

	/**
	* Set of History Memento Collection that store every message
	* of a chat session.
	*/
	std::map<int, HistoryMementoCollection *> _chatSessionsMementos;

	/** A ref to UserProfile */
	UserProfile & _userProfile;

	/** Unseen missed calls count. */
	int _missedCallCount;

	/** Mutex for thread-safe. */
	mutable Mutex _mutex;
};

#endif //OWHISTORY_H

