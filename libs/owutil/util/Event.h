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

#ifndef OWEVENT_H
#define OWEVENT_H

#include <util/NonCopyable.h>

#include <boost/signal.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/visit_each.hpp>

#include <list>

/**
 * Delegates/Design Pattern Observer.
 *
 * This class helps to implement the subject part of the observer design pattern.
 *
 * Uses boost::function and boost::signal in order to simplify the use of the pattern.
 * Works a bit like the C# observer pattern that uses delegates and events.
 *
 * You can also connect an Event to another.
 *
 * An class Trackable is available for automatic disconnection when an object is being destroyed.
 * boost::signal are not thread safe:
 * http://boost.org/doc/html/signals/s04.html#id2738867
 *
 * Example (the boost::signal syntax is commented so that one can compare both):
 * <pre>
 * void showTime(MyTimer * sender, const std::string & time) {
 *     std::cout << time << std::endl;
 * }
 *
 * class Clock {
 * public:
 *     void showTime(MyTimer * sender, const std::string & time) const {
 *         std::cout << "Clock: " << time << std::endl;
 *     }
 * };
 *
 * class MyTimer {
 * public:
 *
 *     Event<void (MyTimer *, const std::string &)> secondEvent;
 *     //boost::signal<void (const std::string &)> secondEvent;
 *
 *     Event<void (MyTimer *, const std::string &)> minuteEvent;
 *     //boost::signal<void (const std::string &)> minuteEvent;
 *
 *     void tick() {
 *         //Sends the signal, updates the observers
 *         secondEvent(this, "Second Event");
 *         minuteEvent(this, "Minute Event");
 *     }
 * };
 *
 * MyTimer timer;
 * Clock clock;
 *
 * timer.secondEvent += &showTime;
 * //timer.secondEvent.connect(&showTime);
 * timer.secondEvent += boost::bind(&Clock::showTime, &clock, _1, _2);
 * //timer.secondEvent.connect(boost::bind(&Clock::showTime, &clock, _1);
 * timer.minuteEvent += boost::bind(&Clock::showTime, &clock, _1, _2);
 * //timer.minuteEvent.connect(boost::bind(&Clock::showTime, &clock, _1);
 * timer.tick();
 *    //Print: Second Event
 *    //       Clock: Second Event
 *    //       Clock: Minute Event
 *
 * timer.minuteEvent -= boost::bind(&Clock::showTime, &clock, _1, _2);
 * //timer.minuteEvent.disconnect(boost::bind(&Clock::showTime, &clock, _1));
 * timer.tick();
 *    //Print: Second Event
 *    //       Clock: Second Event
 * </pre>
 *
 * A good pratice is to always have the sender as the first parameter of the Event.
 *
 * @see Trackable
 * @author Tanguy Krotoff
 */
template<typename Signature>
class Event : NonCopyable, public boost::signal<Signature> {
public:

	/**
	 * Connects a slot to this signal (=event).
	 *
	 * Provides unicity when connecting a slot to a signal.
	 * Two identical slots cannot be connected, only one will be:
	 * this method checks first if the same slot was not connected already.
	 *
	 * @param slot callback function
	 * @return connection object
	 */
	template<typename Slot>
	boost::signals::connection operator+=(const Slot & slot) {
		boost::signals::connection c;
		if (!alreadyConnected(slot)) {
			//The slot is not connected to the signal
			c = this->connect(slot);
			SlotConnection sc;
			sc.connection = c;
			sc.slot = slot;
			_slotList.push_back(sc);
		}
		//The slot is already connected to the signal
		return c;
	}

	/**
	 * Connects a signal to another signal (=event).
	 *
	 * Does not check if the signal is already connected, does not
	 * provide unicity connection.
	 *
	 * @param event signal to connect
	 * @return connection object
	 */
	boost::signals::connection operator+=(const Event & event) {
		return this->connect(event);
	}

	/**
	 * Disconnects a slot from a signal (=event).
	 *
	 * Slot comparison does not always work properly,
	 * check http://boost.org/doc/html/function/faq.html#id2699084
	 *
	 * @param slot callback function to disconnect from the signal
	 */
	template<typename Slot>
	void operator-=(const Slot & slot) {
		typename SlotList::iterator it;
		for (it = _slotList.begin(); it != _slotList.end(); it++) {
			if ((*it).slot == slot)
				break;
		}
		if (it != _slotList.end()) {
			(*it).connection.disconnect();
			_slotList.erase(it);
		}
	}

private:

	/**
	 * Checks if a slot is already present inside the slot list.
	 *
	 * If a slot is already present inside the slot list this means the slot
	 * is already connected to the signal.
	 *
	 * @param slot callback function
	 * @return true if the slot is present inside the slot list; false otherwise
	 */
	template<typename Slot>
	bool alreadyConnected(const Slot & slot) {
		for (typename SlotList::iterator it = _slotList.begin(); it != _slotList.end();) {
			SlotConnection sc = *it;
			if (sc.slot == slot) {
				if (sc.connection.connected()) {
					return true;
				} else {
					typename SlotList::iterator it2 = it;
					it = _slotList.erase(it2);
				}
			} else {
				it++;
			}
		}
		return false;
	}

	/** Associates a slot with a connection. */
	struct SlotConnection {
		boost::function<Signature> slot;
		boost::signals::connection connection;
	};

	/** Type list of slot. */
	typedef std::list<SlotConnection> SlotList;

	/**
	 * The vector/collection/list of slot.
	 *
	 * Permits to provide unicity when connecting a slot to a signal.
	 */
	SlotList _slotList;
};

#endif	//OWEVENT_H
