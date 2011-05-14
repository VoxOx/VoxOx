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

//#include "stdafx.h"		//VOXOX - JRT - 2009.04.03 - For precompiled headers.
#include <util/Date.h>

#include <util/Logger.h>
#include <util/String.h>

//#include <stdio.h>
#include <time.h>

Date::Date() {
	std::time_t curTime = time(NULL);
	struct std::tm * timeinfo = std::localtime(&curTime);
	setDay(timeinfo->tm_mday);
	setMonth(timeinfo->tm_mon + 1);
	setYear(timeinfo->tm_year + 1900);
}

Date::Date(const Date & date) {
	setDay(date._day);
	setMonth(date._month);
	setYear(date._year);
}

Date::Date(unsigned day, unsigned month, unsigned year) {
	setDay(day);
	setMonth(month);
	setYear(year);
}

Date::~Date() {
}

bool Date::operator==(const Date & date) const {
	return ((_day == date._day)
		&& (_month == date._month)
		&& (_year == date._year));
}

//VOXOX - JRT - 2009.10.27 
bool Date::operator> (const Date & date) const 
{
	bool result = false;

	if ( ! (*this == date ) )
	{
		if ( getYear() > date.getYear() )
		{
			result = true;
		}
		else if ( getYear() == date.getYear() )
		{
			if ( getMonth() > date.getMonth() )
			{
				result = true;
			}
			else 
			{
				if ( getDay() > date.getDay() )
				{
					result = true;
				}
			}
		}
	}

	return result;
}

unsigned Date::getDay() const {
	return _day;
}

void Date::setDay(unsigned day) {
	if (day < 1) {
		LOG_FATAL("day cannot be < 1");
	} else if (day > 31) {
		LOG_FATAL("day cannot be > 31");
	}

	_day = day;
}

unsigned Date::getMonth() const {
	return _month;
}

void Date::setMonth(unsigned month) {
	if (month < 1) {
		LOG_FATAL("month cannot be < 1");
	} else if (month > 12) {
		LOG_FATAL("month cannot be > 12");
	}

	_month = month;
}

unsigned Date::getYear() const {
	return _year;
}

void Date::setYear(unsigned year) {
	_year = year;
}

std::string Date::toString() const {
	std::string month = String::fromNumber(_month);
	std::string day = String::fromNumber(_day);

	if (month.size() == 1) {
		month = "0" + month;
	}

	if (day.size() == 1) {
		day = "0" + day;
	}

	return String::fromNumber(_year) + "-" + month + "-" + day;
}

//static -  date1 - date2
int Date::daysDiff( const Date& date1, const Date& date2 )
{
	int	 days = 0;

	struct std::tm a = {0,0,0,0,0,0};
	struct std::tm b = {0,0,0,0,0,0};

	a.tm_year  = date1.getYear() - 1900;
	a.tm_mon   = date1.getMonth();
	a.tm_mday  = date1.getDay();

	b.tm_year  = date2.getYear() - 1900;
	b.tm_mon   = date2.getMonth();
	b.tm_mday  = date2.getDay();


	std::time_t x = std::mktime(&a);
	std::time_t y = std::mktime(&b);

	if ( x != (std::time_t)(-1) && y != (std::time_t)(-1) )
	{
		days = (int) (std::difftime(x, y) / (60 * 60 * 24));
	}

	return days; 
}
