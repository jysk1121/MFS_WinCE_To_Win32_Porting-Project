#ifndef __COLLECTION_H__
#define __COLLECTION_H__

#include "LinkedList.h"
#include "PtrLinkedList.h"

////////////////////////////////////////////////////////////////////
//
//	Queue
//

template <class T>
class CMFSQueue : private CMFSLinkedList<T>
{
public:
	BOOL Enqueue( T &rData )
	{
		return PutOnTailOfList( _T(""), rData );
	};

	BOOL Dequeue( T &rData, DWORD dwTimeout = INFINITE )
	{
		return GetFromHeadOfList( rData, dwTimeout );
	};

	BOOL IsHasData( DWORD dwTimeout = INFINITE )
	{
		return IsHasDataOfList( dwTimeout );
	};

	BOOL Peekqueue( T &rData, DWORD dwTimeout = INFINITE)
	{
		if (!IsHaveDataOfList(dwTimeout))
			return FALSE;
		
		return MoveFirst(rData);
	}

	void Clear()
	{
		DeleteAll();
	};
};

////////////////////////////////////////////////////////////////////
//
//	Stack
//

template <class T>
class CMFSStack : private CMFSLinkedList<T>
{
public :
	BOOL Push( T &rData )
	{
		return PutOnHeadOfList( rData );
	};

	BOOL Pop( T &rData, DWORD dwTimeout = INFINITE )
	{
		return GetFromHeadOfList( rData, dwTimeout );
	};

	BOOL IsHaveData( DWORD dwTimeout = INFINITE )
	{
		return IsHaveDataOfList( dwTimeout );
	};

	BOOL Peekpop( T &rData, DWORD dwTimeout = INFINITE)
	{
		if (!IsHasDataOfList(dwTimeout))
			return FALSE;

		return MoveFirst(rData);
	}

	void Clear()
	{
		DeleteAll();
	};
};

#endif // __COLLECTION_H__