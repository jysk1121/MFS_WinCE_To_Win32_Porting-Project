#ifndef __NH_LINKED_LIST_H__
#define __NH_LINKED_LIST_H__

#include ".\Dll\NHSyncObject.h"

////////////////////////////
//	Linked List
////////////////////////////

template <class T>
class CNHLinkedList
{
protected:
	// 추상화 노드 클래스
	class CListNode
	{
	public:
		CListNode *m_pNext;
		CListNode *m_pPrev;
	public:
		virtual void	SetKey( CString strKey ) = 0;
		virtual CString GetKey( ) = 0;
		virtual void	SetData( T &rData ) = 0;
		virtual void	GetData( T &rData ) = 0;
	};

	// 마스터(Master)와 프리(Free) 리스트 노드를 위해서 상속한 클래스
	class CListSentinelNode : public CListNode
	{
	public:
		// 아래 함수는 호출되면 안된다.
		virtual void	SetKey( CString strKey)	{ASSERT(0);};
		virtual CString GetKey( )				{ASSERT(0); return _T("");};
		virtual void	SetData( T &rData )	{ASSERT(0);};
		virtual void	GetData( T &rData )	{ASSERT(0);};
	};

	// 데이터를 위해서 상속한 클래스
	class CListDataNode : public CListNode
	{
	public:
		T		m_data;
		CString	m_strKey;
	public:
		CListDataNode()	{};
		CListDataNode( CString Key, T &rData ) : m_strKey(Key), m_data(rData) {};
		virtual void	SetKey( CString strKey)	{m_strKey=strKey;};
		virtual CString GetKey( )				{return m_strKey;};
		virtual void	SetData( T &rData )		{m_data=rData;};
		virtual void	GetData( T &rData )		{rData=m_data;};
	};


	CListSentinelNode	m_MasterNode;
	CListSentinelNode	m_FreeNode;
	
	CNHCritSec			m_cCritSec;
	CNHEvent			m_cNotEmpty;

	CListNode			*m_pCursor;
	int					m_nDataCount;
public:
	//////////////////////////////////////////////////////
	//  생성자.
	/////////////////////////////////////////////////////

	CNHLinkedList();
	~CNHLinkedList();
	
	CNHLinkedList& operator= (const CNHLinkedList &ErrDesc);

	//////////////////////////////////////////////////////
	//  추가.
	/////////////////////////////////////////////////////

	BOOL PutOnHeadOfList( CString Key, T &rData);
	BOOL PutOnTailOfList( CString Key, T &rData);

	//////////////////////////////////////////////////////
	//  삭제.
	/////////////////////////////////////////////////////

	BOOL GetFromHeadOfList( T &rData, DWORD dwTimeout = 0 );
	BOOL GetFromTailOfList( T &rData, DWORD dwTimeout = 0 );

	//////////////////////////////////////////////////////
	//  검색 및 이동.
	/////////////////////////////////////////////////////

	int	 GetDataCount();
	BOOL IsHasDataOfList( DWORD dwTimeout = INFINITE );
	BOOL FindDataOfList(CString Key, T &rData, BOOL bRemove=FALSE);
	BOOL FindDataOneOfList(CString Key, T &rData);

	BOOL MoveFirst(T &rData);
	BOOL MoveLast(T &rData);
	BOOL MovePrev(T &rData, int nSize);
	BOOL MoveNext(T &rData, int nSize);

	void Sort(BOOL bAsc = TRUE);

	///////////////////////////////////////////////////////
	//  삭제.
	//////////////////////////////////////////////////////

	void DeleteAll( void );

protected :
	void InsertBetween(CListNode *pPrevNode, CListNode *pNextNode, CListNode *pNewNode);
	void AddToFreeList( CListNode *pNewNode );
	// [2ND] NH AIREAT 2008.11.10 : .h로 구현 옮김.
	CListDataNode* FindNodeOfList( CString Key )
	{
		CListNode	*pNode = NULL;
		CString		strNodeKey;
		int			nNumData = Key.GetLength();

		// Key의 데이터가 없으면 검색을 하지 않는다.
		if (nNumData <= 0)
			return NULL;

		pNode = m_MasterNode.m_pNext;

		// 일치하는 Key 검색.
		while ( pNode != (CListNode*)&m_MasterNode )
		{
			strNodeKey = pNode->GetKey();
			if (Key == strNodeKey)
				break;

			// Next
			pNode = pNode->m_pNext;
		}

		// 성공 했는지 검사.
		if( pNode == (CListNode*)&m_MasterNode )
			return NULL;

		return (CListDataNode*)pNode;
	};
	// [2ND] NH AIREAT 2008.11.10 : .h로 구현 옮김.
	CListDataNode* AllocateListNode( CString Key, T &rData )
	{
		CListNode *pNode = m_FreeNode.m_pNext;

		if( pNode != (CListNode*)&m_FreeNode )
		{
			// Free List에 사용가능한 Node가 있으면 그걸 사용한다.

			CListNode *pPrevNode = pNode->m_pPrev;
			CListNode *pNextNode = pNode->m_pNext;

			pPrevNode->m_pNext = pNextNode;
			pNextNode->m_pPrev = pPrevNode;

			pNode->SetKey( Key );
			pNode->SetData( rData );
		}
		else
		{
			// 새로운 메모리를 생성한다.
			pNode = (CListNode*) new CListDataNode( Key, rData );
		}

		if (pNode != NULL)
		{
			pNode->m_pPrev = pNode;
			pNode->m_pNext = pNode;
		}

		return (CListDataNode*)pNode;
	};

	void CleanFreeNode( void );
	void Cleanup( void );

};

#include ".\NHLinkedList.hpp"

#endif //__NH_LINKED_LIST_H__