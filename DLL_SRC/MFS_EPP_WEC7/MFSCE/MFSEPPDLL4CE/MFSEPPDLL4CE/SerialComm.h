#pragma once

#include "SerialCommDef.h"

class CSerialComm
{
public:
	CSerialComm(void);
	~CSerialComm(void);

	int OpenPort(const TCHAR* szPortNum, DWORD dwBaudRate, BYTE byByteSize=8, BYTE byParity=NOPARITY);
	int ClosePort(void);

	int WriteComm(LPBYTE pBuff, DWORD dwToWrite, DWORD &dwWritten);
	int ReadComm(LPBYTE pBuff, DWORD dwToRead, DWORD &dwBeRead);

public:
	HANDLE m_hComm;
	BOOL m_bConnected;
	OVERLAPPED m_osRead, m_osWrite;

protected:
	HANDLE m_hMutex;
	BYTE m_RecvBuff[MAX_RECV_BUFFER_SIZE];
	int m_nFrameLength;
	BYTE m_bySendFlag;
	BYTE m_byRecvFlag;
};
