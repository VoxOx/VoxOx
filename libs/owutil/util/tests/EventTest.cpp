/**
 * This program is a simple and small example of what can be achieved
 * with custom events.
 *
 * These events are wrappers around boost signals, they are defined in
 * Event.h.
 *
 * Commented lines are here to show the old syntax.
 *
 * You can build this example on GNU/Linux with the following command line in this directory:
 * g++ -I ../include/ -lboost_signals EventTest.cpp -o event_test
 */

#include <iostream>
#include <util/Event.h>
#include <util/Trackable.h>
#include <string>

class MyTimer {
public:

	Event<void (MyTimer *, const std::string &)> secondEvent;
	//boost::signal<void (MyTimer *, const std::string &)> secondEvent;

	Event<void (MyTimer *, const std::string &)> minuteEvent;
	//boost::signal<void (MyTimer *, const std::string &)> minuteEvent;

	void tick() {
		//Sends the signal, updates the observers
		secondEvent(this, "Second Event");
		minuteEvent(this, "Minute Event");
	}
};

void showTime(MyTimer * sender, const std::string & time) {
	std::cout << time << std::endl;
}

class Clock : public Trackable {
public:

	Clock() {
		_crashVariable = 1;
	}

	~Clock() {
		_crashVariable = -1;
	}

	void showTime(MyTimer * sender, const std::string & time) const {
		std::cout << "Clock: " << time << _crashVariable << std::endl;
	}

private:

	int _crashVariable;
};


int main(void) {
	MyTimer timer;
	Clock * clock = new Clock();

	timer.secondEvent += &showTime;
	timer.secondEvent += &showTime;
	//timer.secondEvent.connect(&showTime);
	timer.secondEvent += boost::bind(&Clock::showTime, clock, _1, _2);
	timer.secondEvent += boost::bind(&Clock::showTime, clock, _1, _2);
	//timer.secondEvent.connect(boost::bind(&Clock::showTime, clock, _1, _2));
	timer.minuteEvent += boost::bind(&Clock::showTime, clock, _1, _2);
	//timer.minuteEvent.connect(boost::bind(&Clock::showTime, clock, _1, _2));

	timer.tick();

	//delete clock;

	//Print: Second Event
	//       Clock: Second Event
	//       Clock: Minute Event

	timer.minuteEvent -= boost::bind(&Clock::showTime, clock, _1, _2);
	//timer.minuteEvent.disconnect(&Clock::showTime);
	timer.tick();
	//Print: Second Event
	//       Clock: Second Event

	timer.secondEvent += timer.minuteEvent;
}
