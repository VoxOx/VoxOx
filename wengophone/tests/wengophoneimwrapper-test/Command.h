/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2005  Wengo
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
 
#ifndef COMMAND_H
#define COMMAND_H

#include <string>

class WengoPhone;

class Command {
public:

	Command(WengoPhone & wengoPhone) 
		: _wengoPhone(wengoPhone) {}

	virtual ~Command() {}

	/**
	 * @return true if this Command class can handle the command.
	 */
	virtual bool canHandle(const std::string & command) const {
		return (command == name());
	}

	/**
	 * Execute the command.
	 */
	virtual void execute() = 0;

	/**
	 * @return the Command name.
	 */
	virtual const std::string & name() const = 0;

	/**
	 * @return a description.
	 */
	virtual const std::string & desc() const = 0;

protected:

	WengoPhone & _wengoPhone;

};

#endif /*COMMAND_H*/
