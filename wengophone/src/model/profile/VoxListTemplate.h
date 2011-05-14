#pragma once

//Based on work by J. R. Theinert

#include <list>
#include <map>
#include <iterator>

#include <thread/RecursiveMutex.h>
//#include <util/logger.h>		//Debugging

template <class T> class VoxListTemplate;			//Fwd declaration.
template <class k, class T> class VoxMapTemplate;	//Fwd declaration.

//NOTE: contains VoxListTemplate and VoxMapTemplate.

//=============================================================================
template <class T>
class VoxListTemplate : public std::list< T >
{
public:
			 VoxListTemplate()									{ Init();	}
			 VoxListTemplate( const VoxListTemplate& src );
	virtual ~VoxListTemplate()									{ DeleteAll( m_bAutoDelete );	}

	T*	getPreferred();

	bool setPreferred    ( const T& item  );
	bool setPreferredType( const char* type );
	std::string	getPreferredType()	const			{ return m_preferredType;	}

	T*			findByType		  ( const char* type ) const;
	T&			findOrCreateByType( const char* type );

	void		Merge( const VoxListTemplate& src );

	T*			Find( T* pSource );
	T*			GetByID( int nID );

	//Add/Delete methods
	void		SetAutoDelete( bool bVal )			{ m_bAutoDelete = bVal;	}
	bool		GetAutoDelete()						{ return m_bAutoDelete;	}

	long		GetCount()							{ return (long)this->size();	}
	long		GetValidCount();

	T*			Add   ( T* pData, bool bSetID = false );
	T*			AddOrUpdate( T* pData, bool bSetID = false );	//VOXOX - JRT - 2009.09.24 
	int			Append( VoxListTemplate<T>* pList );
//	bool		InsertBefore( T* pItemBefore, T* pNewItem );

//	bool		Delete( T* pData, bool bDelete = true );
	bool		Delete( int nID, bool bDelete = true );
	void		DeleteAll( bool bDelete = true );

	bool		EntryExists( T* pData )			{ return (Find(pData) != NULL);		}

	void		SetNextID( int nVal )			{ m_nNextID = nVal;	}
	int			GetNextID()						{ return m_nNextID;	}

	bool		hasValid()						{ return (GetValidCount() > 0);	}

	//Critical section
//	void SetDesc( LPCTSTR str )					{ m_cs.SetDesc( str );	}
	void Lock()									{ _mutex.lock();	}
	void Unlock()								{ _mutex.unlock();	}

	void lock()									{ Lock();	}
	void unlock()								{ Unlock();	}

	VoxListTemplate<T>& operator= ( const VoxListTemplate<T>& src );

protected:
	void	Init();

	mutable RecursiveMutex _mutex;

	bool		m_bAutoDelete;
	int			m_nNextID;

	std::string	m_preferredType;
};

//=============================================================================




//=============================================================================
template <class T>
VoxListTemplate<T>::VoxListTemplate( const VoxListTemplate& src )
{
	*this = src;
}

//-----------------------------------------------------------------------------

template <class T> VoxListTemplate<T>& VoxListTemplate<T>::operator= (const VoxListTemplate<T>& src)
{
	if (&src != this )
	{
		Lock();

		//Clear existing entries.
		this->DeleteAll(m_bAutoDelete);

		//Now replicate and add from src.
//		src.Lock();
		for ( typename VoxListTemplate<T>::const_iterator iter = src.begin(); iter != src.end(); iter++ )
		{
			this->Add( const_cast<T*>(&(*iter)) );
		}
//		src.Unlock();

		Unlock();
	}

	return *this;
}

//-----------------------------------------------------------------------------

template <class T> void VoxListTemplate<T>::Init()
{
	m_bAutoDelete   = true;
	m_nNextID	    = 0;
	m_preferredType = "";
}

//-----------------------------------------------------------------------------

template <class T> T* VoxListTemplate<T>::Add( T* pItem, bool bSetID )
{ 
	Lock();

	T* pRet  = NULL;
	T* pTemp = Find( pItem );

	//Avoid exact duplicates.
	if ( pTemp )
	{
		pRet = pTemp;
	}
	else
	{
	//To avoid scoping issues, we clone the input.
	T* pNew = new T;
	*pNew = *pItem;
	push_back( *pNew );

		pRet = pNew;
	}

	Unlock();

	return pRet;
}

//-----------------------------------------------------------------------------

template <class T> T* VoxListTemplate<T>::AddOrUpdate( T* pItem, bool bSetID )
{
	//If we have an 'invalid' entry with same type as input, we will update that entry.
	//	Otherwise, we just Add() the input.

	Lock();

	T* pRet  = NULL;
	T* pTemp = findByType( pItem->getType().c_str() );

	bool bAdd = true;
	
	if ( pTemp )
	{
		bAdd = pTemp->isValid();
	}

	if ( bAdd )
	{
		pRet = Add( pItem );
	}
	else
	{
		*pTemp = *pItem;
		pRet = pTemp;
	}

	Unlock();

	return pRet;
}

//-----------------------------------------------------------------------------

template <class T> T& VoxListTemplate<T>::findOrCreateByType( const char* type ) 
{
	T* pRet = findByType( type );

	//We assume caller needs to have this type, so we create it if it does not exist.
	//We will validate it before serialization.
	if ( pRet == NULL )
	{
		T temp;
		temp.setType( type );

		if ( !temp.isValid() )
			int xxx = 1;

		Add( &temp );
	
		pRet = findByType( type );
	}

	return *pRet;
}

//-----------------------------------------------------------------------------

template <class T> T* VoxListTemplate<T>::findByType( const char* type ) const
{
	T* pRet = NULL;

	for ( typename VoxListTemplate<T>::const_iterator iter = std::list< T >::begin(); iter != std::list< T >::end(); iter++ )
	{
		if ( iter->getType() == type )
		{
			pRet = const_cast<T*>(&(*iter));
			break;
		}
	}

	return pRet;
}

//-----------------------------------------------------------------------------

template <class T> void VoxListTemplate<T>::Merge( const VoxListTemplate& src )
{
//	T* pTemp = NULL;
//	LOG_INFO( "src count: " + String::fromNumber( const_cast<VoxListTemplate&>(src).GetValidCount() ) );

	const_cast<VoxListTemplate&>(src).Lock();	//VOXOX - JRT - 2009.09.11 

	for ( typename VoxListTemplate<T>::const_iterator it = src.begin(); it != src.end(); it++ )
	{
		//VOXOX - JRT - 2009.09.11 - We now allow multiple of same type.
		if ( (*it).isValid() )
		{
			if ( EntryExists(const_cast<T*>( &(*it) ) ) )
			{
//				LOG_INFO( "Skipped: " + (*it).getType() );
			}
			else
			{
				//The list may include "invalid" entries, so let's find them and if they exist,
				// we will update them to be valid.  Otherwise, we will add.
				// Since these 'invalid' entries should only exist if no valid entry for that 'type' exist,
				//  we should never have an invalid and a valid entry for same 'type'.
				AddOrUpdate( const_cast<T*>(&(*it)) );		//VOXOX - JRT - 2009.09.24 
//				LOG_INFO( "Added: " + (*it).getType() );
			}
		}
		else
		{
//			LOG_INFO( "Invalid: " + (*it).getType() );
		}
	}

	const_cast<VoxListTemplate&>(src).Unlock();	//VOXOX - JRT - 2009.09.11 
}

//-----------------------------------------------------------------------------

template <class T> int VoxListTemplate<T>::Append( VoxListTemplate<T>* pList )
{
	int nCount = 0;

	pList->Lock();

	T* pTemp = NULL;

	for ( typename VoxListTemplate<T>::const_iterator iter = pList->begin(); iter != pList->end(); iter++ )
	{
		pTemp = Find( const_cast<T*>(&(*iter)) );	//VOXOX - JRT - 2009.09.11 

		if ( pTemp == NULL )
		{
			Add( pTemp );	//Add() handles locking.
		}
	}
	
	pList->Unlock();

	return nCount;
}

//-----------------------------------------------------------------------------

template <class T> void VoxListTemplate<T>::DeleteAll( bool bDelete )
{
	T* pItem = NULL;

	Lock();

	while ( GetCount() > 0 )
	{
		typename VoxListTemplate<T>::iterator iter = std::list< T >::begin();

		if ( bDelete )
		{
			delete pItem;
		}

		this->erase( iter );
	}
	
	Unlock();

	m_nNextID = 0;
}

//-----------------------------------------------------------------------------

template <class T> T* VoxListTemplate<T>::Find( T* pItem )
{
	T* pRet = NULL;

	Lock();

	for ( typename VoxListTemplate<T>::const_iterator iter = std::list< T >::begin(); iter != std::list< T >::end(); iter++ )
	{
		if ( *pItem == const_cast<T&>(*iter) )	//VOXOX - JRT - 2009.09.11 
		{
			pRet = const_cast<T*>(&(*iter));	//VOXOX - JRT - 2009.09.11 
			break;
		}
	}
	
	Unlock();

	return pRet;
}

//-----------------------------------------------------------------------------

template <class T> T* VoxListTemplate<T>::getPreferred()
{
	T* pRet = NULL;
	
	if ( GetCount() > 0 )
	{
		pRet = findByType( m_preferredType.c_str() );

		if ( pRet == NULL )
		{
			typename VoxListTemplate<T>::iterator iter = std::list< T >::begin();
			pRet  = &(*iter);
		}
	}

	return pRet;
}

//-----------------------------------------------------------------------------

template <class T> bool VoxListTemplate<T>::setPreferred( const T& item  )
{
	Lock();

	int nSet = 0;

	for ( typename VoxListTemplate<T>::const_iterator iter = std::list< T >::begin(); iter != std::list< T >::end(); iter++ )
	{

		(*iter).setPreferred( false );

		if ( (*iter) == item )
		{
			(*iter).setPreferred( true );
			nSet++;
		}
	}
	
	Unlock();

	return (nSet > 0);
 }

//-----------------------------------------------------------------------------

template <class T> bool VoxListTemplate<T>::setPreferredType( const char* type )
{
	bool bSet = false;

	//Let's make sure it exists.
	if		( type == NULL )
	{
		m_preferredType = "";
		bSet		    = true;
	}
	else if ( strlen(type) == 0 )
	{
		m_preferredType = "";
		bSet		    = true;
	}
	else
	{
		if ( findByType( type ) != NULL )
		{
			m_preferredType = type;
			bSet		    = true;
		}
	}

	return bSet;
}

//-----------------------------------------------------------------------------

template <class T> long VoxListTemplate<T>::GetValidCount()
{
	Lock();

	long nCount = 0;

	for ( typename VoxListTemplate<T>::const_iterator iter = std::list< T >::begin(); iter != std::list< T >::end(); iter++ )
	{
		if ( (*iter).isValid() )
		{
			nCount++;
		}
	}
	
	Unlock();

	return nCount;
}

//=============================================================================


//=============================================================================
template <class K, class T>
class VoxMapTemplate : public std::map<K, T>
{
public:
			 VoxMapTemplate()								{ Init();	}
			 VoxMapTemplate( const VoxMapTemplate& src );
	virtual ~VoxMapTemplate()								{ DeleteAll();	}

	//Retrieval methods
	T*			Lookup( const K& rKkey );

	//Add/Delete methods
	long		GetCount()								{ return (long)std::map<K,T>::size();	}

	T*			Add   ( const T* pItem, bool* pAdded = NULL );					//This requires that class T provide a GetKey() method.
	T*			Add   ( const T& rItem, bool* pAdded = NULL );					//This requires that class T provide a GetKey() method.
	T*			Add   ( const K& rKey, const T* pData, bool* pAdded = NULL );
	T*			Add   ( const K& rKey, const T& rData, bool* pAdded = NULL );
//	int			Append( VoxMapTemplate<K,T>* pMap, bool bClone = false );

	bool		Delete( const T& rItem )				{ return Delete( const_cast<T&>(rItem).getKey());	}
	bool		Delete( const K& rKey  );
	void		DeleteAll();
//	bool		EntryExists( T* pData )			{ return (Find(pData) != NULL);		}


	//Critical section
//	void SetDesc( LPCTSTR str )					{ m_cs.SetDesc( str );	}
	void Lock()	  const							{ _mutex.lock();	}
	void Unlock() const							{ _mutex.unlock();	}

	void lock()	  const							{ Lock();			}
	void unlock() const							{ Unlock();			}

	VoxMapTemplate<K,T>& operator=( const VoxMapTemplate<K,T>& src);

protected:
	void	Init();

	mutable RecursiveMutex _mutex;
};

//=============================================================================



//=============================================================================
template <class K, class T>
VoxMapTemplate<K, T>::VoxMapTemplate( const VoxMapTemplate& src )
{
	*this = src;
}

//-----------------------------------------------------------------------------

template <class K, class T>
void VoxMapTemplate<K, T>::Init()
{
//	m_bAutoDelete = true;
}

//-----------------------------------------------------------------------------

template <class K, class T>
T* VoxMapTemplate<K, T>::Add( const K& rKey, const T& rItem, bool* pAdded )
{ 
	int nOrigCount = GetCount();

	Lock();
	insert( std::pair<K,T>(rKey, rItem ) );
	Unlock();

	if ( pAdded )
	{
		*pAdded = (nOrigCount != GetCount());
	}

	return &const_cast<T&>(rItem);
}

//-----------------------------------------------------------------------------

template <class K, class T>
T* VoxMapTemplate<K, T>::Add( const T& rItem, bool* pAdded )
{ 
	return Add( const_cast<T&>(rItem).getKey(), rItem, pAdded );
}

//-----------------------------------------------------------------------------

template <class K, class T>
T* VoxMapTemplate<K, T>::Add( const T* pItem, bool* pAdded )
{ 
	if ( pItem )
	{
		return Add( *pItem, pAdded );
	}
	else
	{
		return NULL;
	}
}

//-----------------------------------------------------------------------------

template <class K, class T>
bool VoxMapTemplate<K, T>::Delete( const K& rKey )
{
	Lock();

	int nOrigSize = GetCount();
	std::map<K,T>::erase( rKey );
	

	Unlock();

	return (nOrigSize < GetCount());
}

//-----------------------------------------------------------------------------

template <class K, class T>
void VoxMapTemplate<K, T>::DeleteAll()
{
	Lock();
	std::map<K,T>::clear();
	Unlock();
}

//-----------------------------------------------------------------------------

template <class K, class T>
T* VoxMapTemplate<K, T>::Lookup( const K& rKey )
{
	T*	pRet = NULL;

//	Lock();

	typename VoxMapTemplate<K,T>::iterator it = std::map<K,T>::find( rKey );

	if( it != std::map<K,T>::end() )
	{
		pRet = &((*it).second);
	}

//	Unlock();

	return pRet;
}

//-----------------------------------------------------------------------------

template <class K, class T>
VoxMapTemplate<K,T>& VoxMapTemplate<K,T>::operator=( const VoxMapTemplate<K,T>& src )
{
	if (&src != this )
	{
		Lock();

		//Clear existing entries.
		this->DeleteAll();

		//Now replicate and add from src.
//		src.Lock();

		for ( typename VoxMapTemplate<K,T>::const_iterator iter = src.begin(); iter != src.end(); iter++ )
		{
			this->Add( (*iter).second );
		}

//		src.Unlock();

		Unlock();
	}

	return *this;
}

//-----------------------------------------------------------------------------

//template <class K, class T>
//int VoxMapTemplate<K, T>::Append( VoxMapTemplate<K, T>* pMap, bool bClone )
//{
//	//Locking is handled in Add().
//	int nCount = 0;
//
//	T* pData = pMap->GetFirst();
//	T* pNew  = NULL;
//
//	while ( pData )
//	{
//		if ( bClone )
//		{
//			pNew  = new T;
//			*pNew = *pData;
//		}
//		else
//		{
//			pNew = pData;
//		}
//
//		if ( this->Add( pNew ) )	//Add() handles locking.
//		{
//			nCount++;
//		}
//
//		pData = pMap->GetNext();
//	}
//
//	return nCount;
//}

//=============================================================================
