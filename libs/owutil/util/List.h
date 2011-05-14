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

#ifndef OWLIST_H
#define OWLIST_H

#include <vector>
#include <algorithm>
#include <thread/RecursiveMutex.h>	//VOXOX - JRT - 2009.06.24 

/**
 * List.
 *
 * @see java.util.List<E>
 * @see QList
 * @author Tanguy Krotoff
 */
template<typename T>
class List : public std::vector<T> 
{
public:
	//VOXOX - JRT - 2009.06.24 - required to use mutex (non-copyable!).	
	//List()		{}

	//List( const List& src )
	//{
	//	*this = src;
	//}

	//List<T>& operator=(const List<T>& src)
	//{ 
	//	if ( this != &src )
	//	{
	//		(std::vector<T>)(*this) = (std::vector<T>)src;
	//	}

	//	return *this;	
	//}
	//End VoxOx

	/**
	 * Appends the specified element to the end of this list.
	 *
	 * @param element element to be appended to this list
	 */
	void operator+=(const T & element)			
	{ 
		lock();
		push_back(element);	
		unlock();
	}

	/**
	 * Removes the first occurrence in this list of the specified element.
	 *
	 * Does not delete the element, just remove it from the list.
	 *
	 * @param element to remove from the list
	 * @return true if the element was removed; false otherwise
	 */
	bool remove(const T & element) 
	{
		bool result = false;

		lock();

		typename std::vector<T>::iterator it = std::find(this->begin(), this->end(), element);

		if (it != this->end()) 
		{
			this->erase(it);
			result = true;
		}

		unlock();

		return result;
	}

	/**
	 * @see remove()
	 */
	bool operator-=(const T & element)		{ return remove(element);	}

	/**
	 * Gets the number of occurrences of an element contained in this list.
	 *
	 * @param element element to find inside this list
	 * @return number of occurrences of the specified element contained in this list
	 */
	unsigned contains(const T & element) const 
	{
		lock();		//VOXOX - JRT - 2009.06.24 
		unsigned j = 0;
		for (unsigned i = 0; i < this->size(); i++) 
		{
			if ((*this)[i] == element) 
			{
				j++;
			}
		}

		unlock();	//VOXOX - JRT - 2009.06.24 

		return j;
	}

	//VOXOX - JRT - 2009.06.24
	void lock()	  const					{}	//	{ _mutex.lock();	}
	void unlock() const					{}	//	{ _mutex.unlock();	}

private:
//	mutable RecursiveMutex _mutex;
};

//=============================================================================

//=============================================================================
//VOXOX - JRT - 2009.07.12 
//Extend List class with locking.  We don't want to just add to List class
//  because it may cause unnecessary locking for other derived classes.

template<typename T>
class ListEx : public std::vector<T> 
{
public:
	//VOXOX - JRT - 2009.06.24 - required to use mutex (non-copyable!).	
	ListEx()		{}

	ListEx( const ListEx& src )
	{
		*this = src;
	}

	ListEx<T>& operator=(const ListEx<T>& src)
	{ 
		if ( this != &src )
		{
			RecursiveMutex::ScopedLock lock( _mutex );	//VOXOX - JRT - 2009.07.13 
			src.lock();

			(std::vector<T>)(*this) = (std::vector<T>)src;
			
			src.unlock();
		}

		return *this;	
	}
	//End VoxOx

	/**
	 * Appends the specified element to the end of this list.
	 *
	 * @param element element to be appended to this list
	 */
	void operator+=(const T & element)			
	{ 
		RecursiveMutex::ScopedLock lock( _mutex );	//VOXOX - JRT - 2009.07.13 
		push_back(element);	
	}

	/**
	 * Removes the first occurrence in this list of the specified element.
	 *
	 * Does not delete the element, just remove it from the list.
	 *
	 * @param element to remove from the list
	 * @return true if the element was removed; false otherwise
	 */
	bool remove(const T & element) 
	{
		bool result = false;

		RecursiveMutex::ScopedLock lock( _mutex );	//VOXOX - JRT - 2009.07.13 

		typename std::vector<T>::iterator it = std::find(this->begin(), this->end(), element);

		if (it != this->end()) 
		{
			this->erase(it);
			result = true;
		}

		return result;
	}

	/**
	 * @see remove()
	 */
	bool operator-=(const T & element)		{ return remove(element);	}

	/**
	 * Gets the number of occurrences of an element contained in this list.
	 *
	 * @param element element to find inside this list
	 * @return number of occurrences of the specified element contained in this list
	 */
	unsigned contains(const T & element) const 
	{
		RecursiveMutex::ScopedLock lock( _mutex );	//VOXOX - JRT - 2009.07.13 
		unsigned j = 0;
		for (unsigned i = 0; i < this->size(); i++) 
		{
			if ((*this)[i] == element) 
			{
				j++;
			}
		}

		return j;
	}

	void clear()
	{
		RecursiveMutex::ScopedLock lock( _mutex );	//VOXOX - JRT - 2009.07.13 
		std::vector<T>::clear();
	}

	void lock()	  const						{ _mutex.lock();	}
	void unlock() const						{ _mutex.unlock();	}

private:
	mutable RecursiveMutex _mutex;
};

#endif	//OWLIST_H
