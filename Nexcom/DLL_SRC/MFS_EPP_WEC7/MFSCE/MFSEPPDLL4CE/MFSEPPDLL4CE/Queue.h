#pragma once

#define QBUFF_SIZE 8192 //디바디스마다 조정 가능

class CQueue
{
public:
	CQueue(void);
	~CQueue(void);

	void Clear();
	int GetSize();
	BOOL PutByte(BYTE b);
	BOOL GetByte(BYTE *pb);
	int LookByte(LPBYTE pDest, int nSize=0);
	
	long GetMaxQSize()	{	return QBUFF_SIZE;	};

private:
	BYTE buff[QBUFF_SIZE];
	int m_iHead, m_iTail;
	CRITICAL_SECTION m_Critical;
};
