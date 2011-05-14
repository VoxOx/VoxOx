/*
	VOXOX!!!
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

#ifndef VOX_QTENUM_SORT_OPTION_H
#define VOX_QTENUM_SORT_OPTION_H

class QtEnumSortOption
{
public:
	enum SortOption 
	{
		SortAlpha		 = 0,	//Default
		SortPresence	 = 1,
		SortRandom		 = 2,

		SortGroupAlpha	 = 10,	//Default
		SortGroupManual  = 11,

		GroupByUserGroup = 20,	//Default
		GroupByMostComm  = 21,
		GroupByNetwork	 = 22,
		GroupByNone		 = 23,

		ViewUnavailable  = 30,
//		ViewGroups		 = 31,

		AddContact		 = 40,
	};

	static bool isContactSort  ( int nOption )		{ return (nOption >= SortAlpha		  && nOption <= SortRandom	    );	}
	static bool isGroupSort    ( int nOption )		{ return (nOption >= SortGroupAlpha   && nOption <= SortGroupManual );	}
	static bool isGrouping     ( int nOption )		{ return (nOption >= GroupByUserGroup && nOption <= GroupByNone     );	}
//	static bool isView         ( int nOption )		{ return (nOption >= ViewUnavailable  && nOption <= ViewGroups	    );	}
	static bool isView		   ( int nOption )		{ return (nOption >= ViewUnavailable  && nOption <= ViewUnavailable	);	}
	static bool groupsAreHidden( int nOption )		{ return (nOption == GroupByNone);									}
};

#endif	//VOX_QTENUM_SORT_OPTION_H
