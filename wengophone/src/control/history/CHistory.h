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

#ifndef OWCHISTORY_H
#define OWCHISTORY_H

#include <model/history/History.h>
#include <model/chat/ChatHandler.h>

#include <control/Control.h>

#include <imwrapper/IMChatSession.h>

#include <string>
#include <map>

class Presentation;
class CWengoPhone;
class PHistory;
class CUserProfile;

/**
 * Control for History.
 *
 * @ingroup control
 * @author Mathieu Stute
 */
class CHistory : public Control {
public:

	/**
	 * Default constructor.
	 */
	CHistory(History & history, CWengoPhone & cWengoPhone, CUserProfile & cUserProfile);

	~CHistory();

	Presentation * getPresentation() const;

	CWengoPhone & getCWengoPhone() const;

	/**
	 * @see History::removeMemento
	 */
	void removeHistoryMemento(unsigned id);

	/**
	 * @see History::clear
	 */
	void clear(HistoryMemento::State state = HistoryMemento::Any);

	/**
	 * @see History::replay
	 */
	void replay(unsigned id);

	/**
	 * @see History::getMementos
	 */
	 HistoryMementoCollection * getMementos(HistoryMemento::State state, int count = -1);

	/**
	 * Gets the memento data.
	 *
	 * @param id the memento id
	 * @return the memento data (the sms text)
	 */
	std::string getMementoData(unsigned id) const;

	/**
	 * Gets the memento peer.
	 *
	 * @param id the memento id
	 * @return the memento peer
	 */
	std::string getMementoPeer(unsigned id) const;

	/**
	 * Resets unseen missed calls.
	 */
	void resetUnseenMissedCalls();

	/**
	 * Gets the unseen missed calls count.
	 *
	 * @return unseen missed calls count
	 */
	int getUnseenMissedCalls();

	/**
	 * FIXME to remove
	 * Retreives the History.
	 *
	 * @return the History
	 */
	History & getHistory();

private:

	void initPresentationThreadSafe();

	void historyMementoAddedEventHandler(History & sender, unsigned id);
	void historyMementoAddedEventHandlerThreadSafe(unsigned id);

	void historyMementoUpdatedEventHandler(History & sender, unsigned id);
	void historyMementoUpdatedEventHandlerThreadSafe(unsigned id);

	void historyMementoRemovedEventHandler(History & sender, unsigned id);
	void historyMementoRemovedEventHandlerThreadSafe(unsigned id);

	void historyLoadedEventHandler(History & sender);
	void historyLoadedEventHandlerThreadSafe();

	void unseenMissedCallsChangedEventhandler(History & sender, int count);
	void unseenMissedCallsChangedEventHandlerThreadSafe(int count);

	void newIMChatSessionCreatedEventHandler(ChatHandler & sender, IMChatSession & imChatSession);

	void imChatSessionWillDieEventHandler(IMChatSession & sender);

	void messageReceivedEventHandler(IMChatSession & sender);

	void messageSentEventHandler(IMChatSession & sender, std::string message);

	void removeHistoryMementoThreadSafe(unsigned id);

	void clearThreadSafe(HistoryMemento::State state);

	void replayThreadSafe(unsigned id);

	void resetUnseenMissedCallsThreadSafe();

	/** Link to the model. */
	History & _history;

	/** Link to the CWengoPhone. */
	CWengoPhone & _cWengoPhone;

	/** Link to the CUserProfile **/
	CUserProfile & _cUserProfile;

	/** Link to the presentation via an interface. */
	PHistory * _pHistory;

	/** Index of the last message recorded in the History for each sessionID **/
	std::map<int, int> _lastReceivedMesssageIndex;
};

#endif	//OWCHISTORY_H
