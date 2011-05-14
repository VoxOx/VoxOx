
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
* CLASS DESCRIPTION 
* @author Rolando 
* @date 2009.09.01
*/


#ifndef OWUMITEMINFO_H
#define OWUMITEMINFO_H

#include <imwrapper/EnumPresenceState.h>
#include <model/contactlist/EnumGroupType.h>

#include <QtGui/QTreeWidget>			//VOXOX - JRT - 2009.09.09

class QTreeWidgetItem;


/**
 * Store Item informations for sorting
 *
 * @author Mr K
 */
class QtUMItemInfo {
public:

	QtUMItemInfo( QTreeWidgetItem * item, QTreeWidgetItem * parentItem, const QString & id, 
				   const QString& userName, int index, bool isGroup);

	QtUMItemInfo(const QtUMItemInfo & other);

	~QtUMItemInfo();

	QTreeWidgetItem * getItem() const			{ return _item;			}
	QString getItemId() const				{ return _id;}
	QString getUserName()  const				{ return _userName;	}
	int		getRandomKey() const				{ return _randomKey;}
	bool	isGroup()		const				{ return _isGroup;	}
	int getIndex()   const						{ return _index;	}
	bool			operator<(const QtUMItemInfo & other) const;
	QtUMItemInfo&	operator=(const QtUMItemInfo & other);

private:
	void copy(const QtUMItemInfo & other);
	

	QTreeWidgetItem * _item;
	bool	_isGroup;
	QString _id;
	QString	_userName;
	int		_randomKey; 
	int		_normalizedPresence;
	int		_index;	
};

#endif	//OWUMITEMINFO_H
