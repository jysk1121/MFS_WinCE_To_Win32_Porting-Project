
#pragma warning(disable:4430) // nonstandard extensions warning

////////////////////////////
//	Linked List
////////////////////////////

/*-------------------------------------------------------------------
 CLASS    NAME: CNHLinkedList
 FUNCTION NAME: CNHLinkedList()
 ACCESS LEVEL : PUBLIC
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 생성자.
-------------------------------------------------------------------*/
template <class T> 
CNHLinkedList<T>::CNHLinkedList() : m_cNotEmpty(TRUE, FALSE), m_cCritSec()
{
	// 마스터와 프리 노트가 모두 자신을 가리키는 것은
	// 이 리스트가 비었다는  사실을 나타낸다.
	
	m_MasterNode.m_pNext = m_MasterNode.m_pPrev = (CListNode*)&m_MasterNode;
	m_FreeNode.m_pNext = m_FreeNode.m_pPrev = (CListNode*)&m_FreeNode;
	m_pCursor = NULL;
	m_nDataCount = 0;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CNHLinkedList
 FUNCTION NAME: ~CNHLinkedList()
 ACCESS LEVEL : PUBLIC
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 소멸자.
-------------------------------------------------------------------*/
template <class T> 
CNHLinkedList<T>::~CNHLinkedList()
{
	Cleanup();
}

/*-------------------------------------------------------------------
 CLASS    NAME: CNHLinkedList
 FUNCTION NAME: operator= ()
 ACCESS LEVEL : PUBLIC
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : = 연산자 정의
-------------------------------------------------------------------*/
template <class T> 
CNHLinkedList<T>& CNHLinkedList<T>::operator= (const CNHLinkedList<T> &rList)
{
	T	Data;
	CListNode *pNode = NULL;

	DeleteAll();
	
	// 처음 노드로 이동한다.
	pNode = rList.m_MasterNode.m_pNext;
	
	while ( pNode != (CListNode*)&rList.m_MasterNode )
	{
		pNode->GetData(Data);
		PutOnTailOfList(pNode->GetKey(), Data);
		
		// Next
		pNode = pNode->m_pNext;
	}

	m_pCursor = NULL;
	m_nDataCount = rList.m_nDataCount;
	
	return *this;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CNHLinkedList
 FUNCTION NAME: PutOnHeadOfList ()
 ACCESS LEVEL : PUBLIC
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Master List의 맨 앞에 데이터를 추가한다.
-------------------------------------------------------------------*/
template <class T> 
BOOL CNHLinkedList<T>::PutOnHeadOfList( CString Key, T &rData )
{
	CListNode *pNewNode = NULL;

	//리스트의 임계 영역 락을 획득한다.
	CNHAutoLock autoLock( m_cCritSec );
	
	// 같은 Key의 데이터가 있는지 검사 한다.
	pNewNode = (CListNode*) FindNodeOfList(Key);
	if ( pNewNode != NULL )
	{
		// 새로운 값으로 변경한다.
		pNewNode->SetData(rData);
		return TRUE;
	}
	
	// 새로운 데이터를 위한 Node를 요청한다.
	pNewNode = (CListNode*) AllocateListNode( Key, rData );
	if ( pNewNode == NULL )
	{
		// 메모리 할당이 실패하는 경우이다.
		return FALSE;
	}
	
	// 새로운 데이터 Node를 리스트의 맨 앞에 삽입.
	InsertBetween((CListNode*)&m_MasterNode, m_MasterNode.m_pNext, pNewNode);
	
	m_nDataCount++;
	m_cNotEmpty.Set();
	
	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CNHLinkedList
 FUNCTION NAME: PutOnTailOfList ()
 ACCESS LEVEL : PUBLIC
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Master List의 맨 뒤에 데이터를 추가한다.
-------------------------------------------------------------------*/
template <class T> 
BOOL CNHLinkedList<T>::PutOnTailOfList( CString Key, T &rData )
{
	CListNode *pNewNode = NULL;
	
	// 리스트의 임계 영역 락을 획득한다.
	CNHAutoLock autoLock( m_cCritSec );
	
	// 같은 Key의 데이터가 있는지 검사 한다.
	pNewNode = (CListNode*) FindNodeOfList(Key);
	if ( pNewNode != NULL )
	{
		// 새로운 값으로 변경한다.
		pNewNode->SetData(rData);
		return TRUE;
	}
	
	// 새로운 데이터를 위한 Node를 요청한다.
	pNewNode = (CListNode*) AllocateListNode( Key, rData );
	if( pNewNode == NULL )
	{
		// 메모리 할당을 실패하는 경우이다.
		return FALSE;
	}
	
	// 새로운 데이터 Node를 리스트의 맨 뒤에 삽입.
	InsertBetween(m_MasterNode.m_pPrev, (CListNode*)&m_MasterNode, pNewNode);

	m_nDataCount++;
	m_cNotEmpty.Set();
	
	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CNHLinkedList
 FUNCTION NAME: GetFromHeadOfList ()
 ACCESS LEVEL : PUBLIC
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Master List의 맨 앞에서 데이터를 가져온다.
-------------------------------------------------------------------*/
template <class T> 
BOOL CNHLinkedList<T>::GetFromHeadOfList( T &rData, DWORD dwTimeout )
{
	CListNode	*pNode = NULL;
	CListNode	*pPrevNode = NULL;
	CListNode	*pNextNode = NULL;
	
	// 리스트에 원소가 생길 때까지 대기한다.
	if (m_cNotEmpty.Wait(dwTimeout) != WAIT_OBJECT_0)
		return FALSE;
	
	// 리스트의 임계 영역 락을 획득한다.
	CNHAutoLock autoLock( m_cCritSec );
	
	// 노드를 찾는다.
	pNode = m_MasterNode.m_pNext;
	
	// 리스트에 데이터가 있는지 검사.
	if( pNode == (CListNode*)&m_MasterNode )
		return FALSE;
	
	// Master 리스트의 맨 앞 노드를 떼어낸다.
	pPrevNode = pNode->m_pPrev;
	pNextNode = pNode->m_pNext;
	pPrevNode->m_pNext = pNextNode;
	pNextNode->m_pPrev = pPrevNode;
	
	// 리스트가 비게 되면 not empty 이벤트를 리셋한다.
	if( m_MasterNode.m_pNext == (CListNode*)&m_MasterNode )
		m_cNotEmpty.Reset();			
	
	// 이 리스트 노드를 프리 리스트에 추가한다.
	AddToFreeList( pNode );	
	
	// 리스트 노드의 데이터를 복사한다.
	pNode->GetData( rData );
	m_nDataCount--;
	
	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CNHLinkedList
 FUNCTION NAME: GetFromTailOfList ()
 ACCESS LEVEL : PUBLIC
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Master List의 맨 뒤에서 데이터를 가져온다.
-------------------------------------------------------------------*/
template <class T> 
BOOL CNHLinkedList<T>::GetFromTailOfList( T &rData, DWORD dwTimeout )
{
	CListNode	*pNode = NULL;
	CListNode	*pPrevNode = NULL;
	CListNode	*pNextNode = NULL;

	// 리스트에 원소가 생길 때까지 대기한다.
	if (m_cNotEmpty.Wait(dwTimeout) != WAIT_OBJECT_0)
		return FALSE;
	
	// 리스트의 임계 영역 락을 획득한다.
	CNHAutoLock autoLock( m_cCritSec );
	
	// 노드를 찾는다.
	pNode = m_MasterNode.m_pPrev;
	
	// 리스트에 데이터가 있는지 검사.
	if( pNode == (CListNode*)&m_MasterNode )
		return FALSE;
	
	// Master 리스트의 맨 뒤 노드를 떼어낸다.
	pPrevNode = pNode->m_pPrev;
	pNextNode = pNode->m_pNext;
	pPrevNode->m_pNext = pNextNode;
	pNextNode->m_pPrev = pPrevNode;
	
	// 리스트가 비게 되면 not empty 이벤트를 리셋한다.
	if( m_MasterNode.m_pNext == &m_MasterNode )
		m_cNotEmpty.Reset();			
	
	// 이 리스트 노드를 프리 리스트에 추가한다.
	AddToFreeList( pNode );	
	
	// 리스트 노드의 데이터를 복사한다.
	pNode->GetData( rData );
	m_nDataCount--;
	
	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CNHLinkedList
 FUNCTION NAME: GetDataCount()
 ACCESS LEVEL : PUBLIC
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : List의 데이터 개수를 구한다..
-------------------------------------------------------------------*/
template <class T> 
int CNHLinkedList<T>::GetDataCount()
{
	return m_nDataCount;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CNHLinkedList
 FUNCTION NAME: FindDataOfList()
 ACCESS LEVEL : PUBLIC
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Key에 맞는 데이터를 찾는다.
				Cursor는 bRemove가 FALSE 일때 갱신된다.
-------------------------------------------------------------------*/
template <class T> 
BOOL CNHLinkedList<T>::FindDataOfList(CString Key, T &rData, BOOL bRemove)
{
	CListNode	*pNode = NULL;

	// 리스트에 원소가 생길 때까지 대기한다.
	if (m_cNotEmpty.Wait(0) != WAIT_OBJECT_0)
		return FALSE;
	
	// 리스트의 임계 영역 락을 획득한다.
	CNHAutoLock autoLock( m_cCritSec );
	
	pNode = (CListNode*)FindNodeOfList(Key);
	if (pNode == NULL)
		return FALSE;
	
	// Get Data
	pNode->GetData(rData);
	
	if (bRemove == TRUE)
	{
		CListNode *pPrevNode = NULL, *pNextNode = NULL;

		// pNode를 떼어 낸다.
		pPrevNode = pNode->m_pPrev;
		pNextNode = pNode->m_pNext;
		pPrevNode->m_pNext = pNextNode;
		pNextNode->m_pPrev = pPrevNode;

		AddToFreeList(pNode);
		m_nDataCount--;
	}
	else
		m_pCursor = pNode;
	
	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CNHLinkedList
 FUNCTION NAME: FindDataOneOfList()
 ACCESS LEVEL : PUBLIC
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Key의 크기만큼 일치하는 데이터를 찾는다.
-------------------------------------------------------------------*/
template <class T> 
BOOL CNHLinkedList<T>::FindDataOneOfList(CString Key, T &rData)
{
	CListNode	*pNode = NULL;
	CString		strNodeKey;
	int			nNumData = Key.GetLength();

	// 리스트에 원소가 생길 때까지 대기한다.
	if (m_cNotEmpty.Wait(0) != WAIT_OBJECT_0)
		return FALSE;
	
	// 리스트의 임계 영역 락을 획득한다.
	CNHAutoLock autoLock( m_cCritSec );
	
	if (Key.IsEmpty())
		return FALSE;

	pNode = m_MasterNode.m_pNext;

	// 일치하는 Key 검색.
	while ( pNode != (CListNode*)&m_MasterNode )
	{
		strNodeKey = pNode->GetKey();
		if (Key.Left(nNumData) == strNodeKey.Left(nNumData))
			break;
		
		// Next
		pNode = pNode->m_pNext;
	}
	
	// 성공 했는지 검사.
	if( pNode == (CListNode*)&m_MasterNode )
		return FALSE;

	// Get Data
	pNode->GetData(rData);
	
	m_pCursor = pNode;
	
	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CNHLinkedList
 FUNCTION NAME: MoveFirst()
 ACCESS LEVEL : PUBLIC
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Master 리스트의 처음 데이터를 가져온다.
				Cursor의 위치는 Master의 처음 데이터이다.
-------------------------------------------------------------------*/
template <class T> 
BOOL CNHLinkedList<T>::MoveFirst(T &rData)
{
	// 리스트에 원소가 생길 때까지 대기한다.
	if (m_cNotEmpty.Wait(0) != WAIT_OBJECT_0)
		return FALSE;
	
	// 리스트의 임계 영역 락을 획득한다.
	CNHAutoLock autoLock( m_cCritSec );
	
	m_pCursor = m_MasterNode.m_pNext;
	if (m_pCursor == (CListNode*)&m_MasterNode)
	{
		m_pCursor = NULL;
		return FALSE;
	}
	
	// Get Data
	m_pCursor->GetData(rData);
	
	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CNHLinkedList
 FUNCTION NAME: MoveLast()
 ACCESS LEVEL : PUBLIC
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Master 리스트의 마지막 데이터를 가져온다.
				Cursor의 위치는 Master의 마지막 데이터이다.
-------------------------------------------------------------------*/
template <class T> 
BOOL CNHLinkedList<T>::MoveLast(T &rData)
{
	// 리스트에 원소가 생길 때까지 대기한다.
	if (m_cNotEmpty.Wait(0) != WAIT_OBJECT_0)
		return FALSE;
	
	// 리스트의 임계 영역 락을 획득한다.
	CNHAutoLock autoLock( m_cCritSec );
	
	m_pCursor = m_MasterNode.m_pPrev;
	if (m_pCursor == (CListNode*)&m_MasterNode)
	{
		m_pCursor = NULL;
		return FALSE;
	}
	
	// Get Data
	m_pCursor->GetData(rData);

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CNHLinkedList
 FUNCTION NAME: MovePrev()
 ACCESS LEVEL : PUBLIC
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Cursor가 위치한 데이터의 이전 데이터를 가져온다.
				Cursor의 위치는 이전 데이터가 된다.
-------------------------------------------------------------------*/
template <class T> 
BOOL CNHLinkedList<T>::MovePrev(T &rData, int nSize)
{
	// 리스트에 원소가 생길 때까지 대기한다.
	if (m_cNotEmpty.Wait(0) != WAIT_OBJECT_0)
		return FALSE;
	
	// 리스트의 임계 영역 락을 획득한다.
	CNHAutoLock autoLock( m_cCritSec );
	
	// NULL 이거나 Master 이면 마지막 이니 Move할 필요가 없다.
	if (m_pCursor == NULL || nSize <=0 ||
		m_pCursor == (CListNode*)&m_MasterNode ||
		m_pCursor->m_pPrev == (CListNode*)&m_MasterNode) 
	{
		return FALSE;
	}
	
	// 이동 중 처음이라면 거기까지 이동한다.
	while(nSize > 0)
	{
		if (m_pCursor->m_pPrev == (CListNode*)&m_MasterNode)
			break;
		
		// 다음 노드.
		m_pCursor = m_pCursor->m_pPrev;
		nSize--;
	}
	
	// 혹시나 Error Check.
	if (m_pCursor == (CListNode*)&m_MasterNode)
		return FALSE;
	
	m_pCursor->GetData(rData);
	
	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CNHLinkedList
 FUNCTION NAME: MoveNext()
 ACCESS LEVEL : PUBLIC
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Cursor가 위차한 데이터의 다음 데이터를 가져온다.
				Cursor의 위치는 다음 데이터가 된다.
-------------------------------------------------------------------*/
template <class T> 
BOOL CNHLinkedList<T>::MoveNext(T &rData, int nSize)
{
	// 리스트에 원소가 생길 때까지 대기한다.
	if (m_cNotEmpty.Wait(0) != WAIT_OBJECT_0)
		return FALSE;
	
	// 리스트의 임계 영역 락을 획득한다.
	CNHAutoLock autoLock( m_cCritSec );
	
	// NULL 이거나 Master 이면 마지막 이다.
	if (m_pCursor == NULL || nSize <= 0 ||
		m_pCursor == (CListNode*)&m_MasterNode || 
		m_pCursor->m_pNext == (CListNode*)&m_MasterNode)
	{
		return FALSE;
	}
	
	// 이동 중 마지막 이라면 거기까지 이동한다.
	while(nSize > 0)
	{
		if (m_pCursor->m_pNext == &m_MasterNode)
			break;
		
		// 다음 노드.
		m_pCursor = m_pCursor->m_pNext;
		nSize--;
	}
	
	// 혹시나 Error Check.
	if (m_pCursor == &m_MasterNode)
		return FALSE;
	
	m_pCursor->GetData(rData);
	
	return TRUE;	
}

/*-------------------------------------------------------------------
 CLASS    NAME: CNHLinkedList
 FUNCTION NAME: Sort()
 ACCESS LEVEL : PUBLIC
 RETURN TYPE  : 
 PARAMETER    : bAsc : TRUE -  오름차순
					   FALSE - 내림차순
 DESCRIPTION  : Master 리스트의 데이터를 정렬한다.
-------------------------------------------------------------------*/
template <class T> 
void CNHLinkedList<T>::Sort(BOOL bAsc)
{
	// 리스트에 원소가 생길 때까지 대기한다.
	if (m_cNotEmpty.Wait(0) != WAIT_OBJECT_0)
		return;
	
	// 리스트의 임계 영역 락을 획득한다.
	CNHAutoLock autoLock( m_cCritSec );

	T Data;
	BOOL bSwitch;
	CListDataNode	TempNode;
	CListNode *pNode = NULL;
	CListNode *pNextNode = NULL;
	CString	strKey1, strKey2;			// [#475] NH AIREAT 2008.12.31 - SORT 버그수정
	
	for (pNode = m_MasterNode.m_pNext; pNode != &m_MasterNode; pNode = pNode->m_pNext)
	{
		for (pNextNode = pNode; pNextNode != &m_MasterNode; pNextNode = pNextNode->m_pNext)
		{
			bSwitch = FALSE;
			
			// [#475] NH AIREAT 2008.12.31 - SORT 버그수정
/*
			if (bAsc && (pNode->GetKey() > pNextNode->GetKey()))
				bSwitch = TRUE;
			else if (!bAsc && (pNode->GetKey() < pNextNode->GetKey()))
				bSwitch = TRUE;
*/

			strKey1 = pNode->GetKey();
			strKey2 = pNextNode->GetKey();

			// 오름차순.
			if (bAsc && (NHStringCompare(strKey1, strKey2) > 0))
				bSwitch = TRUE;
			else if (!bAsc && (NHStringCompare(strKey1, strKey2) < 0))
				bSwitch = TRUE;
			// end of [#475]
			
			if (bSwitch)
			{
				pNode->GetData(TempNode.m_data);
				TempNode.m_strKey = pNode->GetKey();
				
				pNextNode->GetData(Data);
				pNode->SetData(Data);
				pNode->SetKey(pNextNode->GetKey());
				
				pNextNode->SetData(TempNode.m_data);
				pNextNode->SetKey(TempNode.m_strKey);
				
			}
		}
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: CNHLinkedList
 FUNCTION NAME: DeleteAll()
 ACCESS LEVEL : PUBLIC
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Master 리스트의 데이터를 삭제한다.
-------------------------------------------------------------------*/
template <class T> 
void CNHLinkedList<T>::DeleteAll( void )
{
	CListNode	*pNode = NULL;
	CListNode	*pPrevNode = NULL;
	CListNode	*pNextNode = NULL;

	//리스트의 임계 영역 락을 획득한다.
	CNHAutoLock autoLock( m_cCritSec );
	
	// 마스터 리스트의 모든 노드를 삭제한다.
	pNode = m_MasterNode.m_pNext;
	
	while( pNode != (CListNode*)&m_MasterNode )
	{
		// pNode를 떼어 낸다.
		pPrevNode = pNode->m_pPrev;
		pNextNode = pNode->m_pNext;
		pPrevNode->m_pNext = pNextNode;
		pNextNode->m_pPrev = pPrevNode;

		AddToFreeList(pNode);

		pNode = pNextNode;
	}

	// 데이터 초기화.
	m_MasterNode.m_pNext = (CListNode*)&m_MasterNode;
	m_MasterNode.m_pPrev = (CListNode*)&m_MasterNode;
	m_pCursor = NULL;
	m_nDataCount = 0;
	
	// 노드에 데이터가 없다.
	m_cNotEmpty.Reset();
}

/*-------------------------------------------------------------------
 CLASS    NAME: CNHLinkedList
 FUNCTION NAME: CleanFreeNode()
 ACCESS LEVEL : PUBLIC
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Free 리스트의 데이터를 삭제한다.
-------------------------------------------------------------------*/
template <class T> 
void CNHLinkedList<T>::CleanFreeNode( void )
{
	//리스트의 임계 영역 락을 획득한다.
	CNHAutoLock autoLock( m_cCritSec );
	
	// 프리 리스트의 모든 노드를 삭제한다.
	CListNode *pNode = m_FreeNode.m_pNext;
	
	while( pNode != (CListNode*)&m_FreeNode )
	{
		CListDataNode *pOldNode = (CListDataNode*)pNode;
		pNode = pNode->m_pNext;
		delete pOldNode;
	}
	
	// 데이터 초기화
	m_FreeNode.m_pNext = &m_FreeNode;
	m_FreeNode.m_pPrev = &m_FreeNode;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CNHLinkedList
 FUNCTION NAME: Cleanup()
 ACCESS LEVEL : PUBLIC
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Master, Free 리스트의 데이터를 삭제한다.
-------------------------------------------------------------------*/
template <class T> 
void CNHLinkedList<T>::Cleanup( void )
{
	// Master List 삭제.
	DeleteAll();

	// Free List 삭제.
	CleanFreeNode();
}


/*-------------------------------------------------------------------
 CLASS    NAME: CNHLinkedList
 FUNCTION NAME: FindNodeOfList()
 ACCESS LEVEL : PROTECTED
 RETURN TYPE  : Key와 일치하는 데이터
 PARAMETER    : Key : 검색할 Key
 DESCRIPTION  : Master List에 Key와 일치하는 데이터가 있는지 검색한다.
-------------------------------------------------------------------*/
#if 0	// [2ND] NH AIREAT 2008.11.10 : .h로 구현 옮김.
template <class T> 
CNHLinkedList<T>::CListDataNode* CNHLinkedList<T>::FindNodeOfList( CString Key )
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
}
#endif

/*-------------------------------------------------------------------
 CLASS    NAME: CNHLinkedList
 FUNCTION NAME: IsHasDataOfList()
 ACCESS LEVEL : PROTECTED
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Master List에 데이터가 있는지 검사한다.
-------------------------------------------------------------------*/
template <class T> 
BOOL CNHLinkedList<T>::IsHasDataOfList( DWORD dwTimeout )
{
	// 리스트에 원소가 생길 때까지 대기한다.
	if (m_cNotEmpty.Wait(dwTimeout) != WAIT_OBJECT_0)
		return FALSE;
	
	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CNHLinkedList
 FUNCTION NAME: AddToFreeList()
 ACCESS LEVEL : PROTECTED
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Free List에 Node를 추가한다.
-------------------------------------------------------------------*/
template <class T> 
void CNHLinkedList<T>::AddToFreeList( CListNode* pNewNode )
{
	// Data Node를 Free List에 끝에 추가한다.
	InsertBetween(m_FreeNode.m_pPrev, (CListNode*)&m_FreeNode, pNewNode);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CNHLinkedList
 FUNCTION NAME: InsertBetween()
 ACCESS LEVEL : PROTECTED
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : NewNode를 PrevNode와 NextNode 사이에 끼워 넣는다.
-------------------------------------------------------------------*/
template <class T> 
void CNHLinkedList<T>::InsertBetween(CListNode *pPrevNode, CListNode *pNextNode, CListNode *pNewNode)
{
	pPrevNode->m_pNext = pNewNode;
	pNewNode->m_pPrev = pPrevNode;
	
	pNewNode->m_pNext = pNextNode;
	pNextNode->m_pPrev = pNewNode;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CNHLinkedList
 FUNCTION NAME: AllocateListNode()
 ACCESS LEVEL : PROTECTED
 RETURN TYPE  : 추가되는 Data Node의 주소.
 PARAMETER    : Key : 데이터를 위한 Key
				rData : 추가할 데이터
 DESCRIPTION  : 새로운 Data를 위한 List Node를 얻는다.
				Free List에 남는 Node가 있으면 재활용을 하고 
				그렇지 않으면 새로 생성한다.
-------------------------------------------------------------------*/
#if 0	// [2ND] NH AIREAT 2008.11.10 : .h로 구현 옮김.
template <class T> 
CNHLinkedList<T>::CListDataNode* CNHLinkedList<T>::AllocateListNode( CString Key, T &rData )
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
}
#endif
