#ifndef __CMN_COLLECTION_H__
#define __CMN_COLLECTION_H__

#include ".\NHLinkedList.h"
#include ".\NHPtrLinkedList.h"

////////////////////////////////////////////////////////////////////
//
//	Queue
//

template <class T>
class CNHQueue : private CNHLinkedList<T>
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
class CNHStack : private CNHLinkedList<T>
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

#endif // __CMN_COLLECTION_H__