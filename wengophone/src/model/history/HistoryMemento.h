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

#ifndef OWHISTORYMEMENTO_H
#define OWHISTORYMEMENTO_H

#include <sipwrapper/EnumPhoneCallState.h>

#include <util/Date.h>
#include <util/Time.h>
#include <serializer/DateXMLSerializer.h>
#include <serializer/TimeXMLSerializer.h>

#include <string>

#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/archive_exception.hpp>

/**
 * History Memento (Memento in the Memento pattern)
 *
 * @author Mathieu Stute
 */
class HistoryMemento {
	friend class HistoryMementoCollection;
	friend class History;
public:

	static const unsigned int SERIALIZATION_VERSION = 1;

	/**
	 * State of a memento.
	 */
	enum State {
		IncomingCall,
		OutgoingCall,
		MissedCall,
		RejectedCall,
		OutgoingSmsOk,
		OutgoingSmsNok,
		ChatSession,
		FileTransferUploaded,
		FileTransferDownloaded,
		FileTransferInProgress,
		FileTransferCancelled,
		FileTransferPaused,
		None,
		Any,
	};

	/**
	 * Default constructor.
	 */
	HistoryMemento();

	/**
	 * Complete constructor.
	 */
	HistoryMemento(State state, const std::string & peer, int callId = -1, const std::string & data = "");

	/**
	 * a more complete constructor
	 */
	HistoryMemento(State state, Date date, Time time, const std::string & peer, int callId = -1, const std::string & data = "");

	/**
	 * Default destructor.
	 */
	~HistoryMemento();

	/**
	 * Returns the state.
	 *
	 * @return the state
	 */
	HistoryMemento::State getState() const;

	/**
	 * Returns the peer.
	 *
	 * @return the peer
	 */
	std::string getPeer() const;

	/**
	 * Returns the duration.
	 *
	 * @return the duration
	 */
	int getDuration() const;

	/**
	 * Returns the date.
	 *
	 * @return the date
	 */
	Date getDate() const;

	/**
	 * Returns the time.
	 *
	 * @return the time
	 */
	Time getTime() const;

	/**
	 * Returns data.
	 *
	 * @return data
	 */
	std::string getData() const;

	/**
	 * Returns true if the Memento can be replayed.
	 *
	 * @return true if the Memento can be replayed
	 */
	bool canReplay() const;

	/**
	 * update the duration.
	 *
	 * @param duration the duration of the call
	 */
	void updateDuration(int duration);

	/**
	 * Returns a string representing the memento.
	 *
	 * @return return a string representing the memento
	 */
	std::string toString() const;

private:

	/**
	 * Returns true if the memento is for a call.
	 *
	 * @return true if the memento is for a call
	 */
	bool isCallMemento() const;

	/**
	 * Returns true if the memento is for a SMS.
	 *
	 * @return true if the memento is for a SMS
	 */
	bool isSMSMemento() const;

	/**
	 * Returns true if the memento is for a chat session.
	 *
	 * @return true if the memento is for a chat session
	 */
	bool isChatSessionMemento() const;

	/**
	 * Updates state.
	 *
	 * @param state new state
	 */
	void updateState(State state);

	/**
	 * Returns the callId associated to the memento.
	 *
	 * @return return the callId associated to the memento
	 */
	int getCallId() const;

	/**
	 * Returns a string representing a memento state.
	 *
	 * @return return a string representing a memento state
	 */
	static std::string stateToString(State state);

	/**
	 * the peer.
	 * TODO: replace by a set of peers (multi SMS, audio conf)
	 */
	std::string _peer;

	/** State of the Memento. */
	State _state;

	/** Date associated to the memento. */
	Date _date;

	/** Time associated to the memento. */
	Time _time;

	/**
	 * meaning:
	 * - text for SMS
	 * - filename for file transfer
	 */
	std::string _data;

	/** data: meaningfull only for calls. */
	int _duration;

	/** data: meaningfull only for calls from this instance of the application. */
	int _callId;

	friend class boost::serialization::access;

	/** Serialialization load method. */
	template < class Archive >
	void load(Archive & ar, const unsigned int version) {
		if (version == SERIALIZATION_VERSION) {
			ar >> BOOST_SERIALIZATION_NVP(_peer);
			DateXMLSerializer d(_date);
			std::string date;
			ar >> BOOST_SERIALIZATION_NVP(date);
			d.unserialize(date);
			TimeXMLSerializer t(_time);
			std::string time;
			ar >> BOOST_SERIALIZATION_NVP(time);
			t.unserialize(time);
			ar >> BOOST_SERIALIZATION_NVP(_duration);
			ar >> BOOST_SERIALIZATION_NVP(_state);
			ar >> BOOST_SERIALIZATION_NVP(_data);
		} else {
			//LOG_DEBUG("Bad Serialization Version");
		}
	}

	/** Serialialization save method. */
	template < class Archive >
	void save(Archive & ar, const unsigned int version) const {
		ar << BOOST_SERIALIZATION_NVP(_peer);
		DateXMLSerializer d((Date &)_date);
		std::string date = d.serialize();
		ar << BOOST_SERIALIZATION_NVP(date);
		TimeXMLSerializer t((Time &)_time);
		std::string time = t.serialize();
		ar << BOOST_SERIALIZATION_NVP(time);
		ar << BOOST_SERIALIZATION_NVP(_duration);
		ar << BOOST_SERIALIZATION_NVP(_state);
		ar << BOOST_SERIALIZATION_NVP(_data);
	}

	BOOST_SERIALIZATION_SPLIT_MEMBER()
};

BOOST_CLASS_VERSION(HistoryMemento, HistoryMemento::SERIALIZATION_VERSION)

#endif	//OWHISTORYMEMENTO_H

