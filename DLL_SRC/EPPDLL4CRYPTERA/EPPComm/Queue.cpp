#include "StdAfx.h"
#include "Queue.h"

CQueue::CQueue(void)
{
	::InitializeCriticalSection(&m_Critical);
	Clear();
}

CQueue::~CQueue(void)
{
	::DeleteCriticalSection(&m_Critical);
}

void CQueue::Clear()
{
	::EnterCriticalSection(&m_Critical);
	m_iHead = m_iTail = 0;
	memset(buff, 0, QBUFF_SIZE);
	::LeaveCriticalSection(&m_Critical);
}

int CQueue::GetSize()
{
	::EnterCriticalSection(&m_Critical);
	int nSize = (m_iHead - m_iTail + QBUFF_SIZE) % QBUFF_SIZE;
	::LeaveCriticalSection(&m_Critical);
	return nSize;
//	return (m_iHead - m_iTail + QBUFF_SIZE) % QBUFF_SIZE;
}

BOOL CQueue::PutByte(BYTE b)
{
	::EnterCriticalSection(&m_Critical);
	if(GetSize() == (QBUFF_SIZE-1))
	{
		::LeaveCriticalSection(&m_Critical);
		return FALSE;
	}
	buff[m_iHead++] = b;
	m_iHead %= QBUFF_SIZE;
	::LeaveCriticalSection(&m_Critical);
	return TRUE;
}

BOOL CQueue::GetByte(BYTE* pb)
{
	::EnterCriticalSection(&m_Critical);
	if (GetSize() == 0) 
	{
		::LeaveCriticalSection(&m_Critical);
		return FALSE;
	}
	*pb = buff[m_iTail++];
	m_iTail %= QBUFF_SIZE;
	::LeaveCriticalSection(&m_Critical);
	return TRUE;
}


/**************************************************************************
// FUNCTION NAME : LookByte
// DESCRIPTION	 : Queue에서 데이터를 꺼내지 않고 buffer 내용만 복사해간다.
// PARAMETERS 
//               : pDest - Destination buffer, nSize-읽어갈 버퍼의 길이
//               : 
// RETURN VALUE 
//               : pDest에 복사된 길이
// DATE          : 2003-03-29 오후 2:17:22
// SEE ALSO      : 
**************************************************************************/
int CQueue::LookByte(LPBYTE pDest, int nSize)
{
	int nCurrSize = GetSize();
	BYTE QueueBuff[QBUFF_SIZE];
	memset(QueueBuff, NULL, QBUFF_SIZE);
	if(nSize == 0)	//읽어갈 버퍼 길이 nSize를 지정하지 않고 호출한 경우
	{
		memcpy((void*)pDest, (const void*)&buff[m_iTail], nCurrSize);
		return nCurrSize;
	}
	else		//읽어갈 버퍼 길이 nSize를 지정한 경우
	{
		if(nCurrSize >= nSize)
		{
			memcpy((void*)pDest, (const void*)&buff[m_iTail], nSize);
			return nSize;
		}
		else 
			return 0;
	}
}

