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
* WS TO SEND Personal Assistand Data
* @author Chris Jimenez C 
* @date 2009.06.27
*/


#ifndef OWPADATA_H
#define OWPADATA_H

#include <string>

/**
 * Wengo SMS web service.
 *
 * Permits to send SMS.
 *
 * @author Mathieu Stute
 */
class PAData {
public:

	
	PAData();

	~PAData() {}

	void setFullName(std::string fullName){_fullName = fullName;}

	void setGender(std::string gender){_gender = gender;}

	void setVoiceGender(std::string voiceGender){_voiceGender = voiceGender;}

	std::string getFullName(){return _fullName;}

	std::string getGender(){return _gender;}

	std::string getVoiceGender(){return _voiceGender;}
	

private:

	std::string _fullName;

	std::string _gender;

	std::string _voiceGender;

	
};

#endif //OWPAData_H
