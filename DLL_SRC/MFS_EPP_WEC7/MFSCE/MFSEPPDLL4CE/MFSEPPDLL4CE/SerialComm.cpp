#include "StdAfx.h"
#include "SerialComm.h"

// ASCII code definition
#define ASCII_LF				0x0A
#define ASCII_CR				0x0D
#define ASCII_XON				0x11
#define ASCII_XOFF				0x13

const UINT PTM_INTERVAL = 30;


CSerialComm::CSerialComm(void)
{
	m_bConnected = FALSE;
}

CSerialComm::~CSerialComm(void)
{
	if(	m_bConnected)
		ClosePort();
}

int CSerialComm::OpenPort(const TCHAR* szPortNum, DWORD dwBaudRate, BYTE byByteSize, BYTE byParity)
{
//	m_bConnected = FALSE;
	if(	m_bConnected)
		return 0;

#ifdef WIN32
	// Overlapped structure 변수 초기화
	m_osRead.Offset = 0;
	m_osRead.OffsetHigh = 0;
	m_osRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if(!m_osRead.hEvent)
	{
		return FALSE;
	}

	m_osWrite.Offset = 0;
	m_osWrite.OffsetHigh = 0;
	m_osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if(!m_osWrite.hEvent)
	{
		return FALSE;
	}
	
	// COM포트 열기
	m_hComm = CreateFile(szPortNum, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
#else
	m_hComm = CreateFile(szPortNum, GENERIC_READ|GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
#endif
	if(m_hComm == (HANDLE)-1) 	
	{
		DWORD dwErr = GetLastError();
 		return (int)dwErr;//0xCC00;
	}

	// COM 포트에서 캐치할 이벤트 설정
//	SetCommMask(m_hComm, EV_RXCHAR | EV_BREAK | EV_ERR);
	SetCommMask(m_hComm, EV_CTS | EV_DSR | EV_BREAK | EV_ERR);


	// Serial 포트 장치의 InQueue, OutQueue의 크기 설정
	SetupComm(m_hComm, 4096, 4096);
	
	// 포트 InQueue, OutQueue 초기화
	PurgeComm(m_hComm, PURGE_TXABORT | PURGE_RXABORT | 
					   PURGE_TXCLEAR | PURGE_RXCLEAR );

	// Save time-out parameters for COMx 
	COMMTIMEOUTS timeouts;
	BOOL bRet = FALSE;
	bRet = GetCommTimeouts(m_hComm, &timeouts);
	if(!bRet)
		return 0xCC11;

	// Set our time-outs. 
	// When ReadIntervalTimeout and ReadTotalTimeoutMultiplier are
	// set to MAXDWORD and ReadTotalTimeoutConstant is a value 
	// greater than 0 and less than MAXWORD, the following occurs
	// when ReadFile() is called:
	//
	// - If there are any characters in the input buffer, ReadFile()
	// returns immediately with the characters in the buffer, instead
	// of waiting until the end of the transmission. 
	//
	// - If there are no characters in the input buffer, ReadFile() 
	// waits until a character arrives and then returns immediately.
	// (Again without waiting until the end of the transmission.)
	//
	// - If no character arrives within the time specified by 
	// ReadTotalTimeoutConstant, ReadFile times out.
	//
	// Information obtained from the "COMMTIMEOUTS" section of the 
	// MSDN Library.

	timeouts.ReadIntervalTimeout         = MAXDWORD; 
	timeouts.ReadTotalTimeoutMultiplier  = MAXDWORD; 
	timeouts.ReadTotalTimeoutConstant    = TIMEOUT_CONSTANT;
	timeouts.WriteTotalTimeoutMultiplier = TIMEOUT_CONSTANT;
	timeouts.WriteTotalTimeoutConstant   = TIMEOUT_CONSTANT;
	bRet = SetCommTimeouts(m_hComm, &timeouts);
	if(!bRet)
		return 0xCC12;

	// Set Port parameters.
	// We make a call to GetCommState() first in order to fill
	// the comSettings structure with all the necessary values.
	// Then we change the ones we want and call SetCommState().

	DCB comSettings;
	bRet = GetCommState(m_hComm, &comSettings);
	if(!bRet)
		return 0xCC13;
#if 1
	comSettings.BaudRate = dwBaudRate;
	comSettings.StopBits = ONESTOPBIT;
	comSettings.ByteSize = byByteSize;
	comSettings.Parity   = byParity;
	comSettings.fParity  = FALSE;
	comSettings.fOutxCtsFlow = FALSE;				/* CTS handshaking on output       */
	comSettings.fOutxDsrFlow = FALSE;				/* DSR handshaking on output       */
	comSettings.fDtrControl = DTR_CONTROL_DISABLE;  /* DTR Flow control                */
	comSettings.fRtsControl = RTS_CONTROL_DISABLE;
	comSettings.fInX = FALSE;
	comSettings.fOutX = FALSE;
#else
	comSettings.BaudRate = 115200;
	comSettings.fBinary = TRUE;
	comSettings.fParity = NOPARITY;
	comSettings.fOutxCtsFlow = 0;
	comSettings.fOutxDsrFlow = 0;
	comSettings.fDtrControl = DTR_CONTROL_DISABLE;
	comSettings.fDsrSensitivity = 0;
	comSettings.fTXContinueOnXoff = 0;
	comSettings.fOutX = 0;
	comSettings.fInX = 0;
	comSettings.fErrorChar = 0;
	comSettings.fNull = 0;
	comSettings.fRtsControl = RTS_CONTROL_DISABLE;
	comSettings.fAbortOnError = 0;
	comSettings.fDummy2 = 0;
	comSettings.wReserved = 0;
	comSettings.XonLim = 64;
	comSettings.XoffLim = 64;
//	comSettings.XonLim = 0;
//	comSettings.XoffLim = 0;
	comSettings.ByteSize = 8;
	comSettings.Parity = 0;
	comSettings.StopBits = ONESTOPBIT;
	comSettings.XonChar = 0x11;
	comSettings.XoffChar = 0x13;
//	comSettings.XonChar = 0;
//	comSettings.XoffChar = 0;
	comSettings.ErrorChar = 0;
	comSettings.EofChar = 0;
	comSettings.EvtChar = 0;
#endif

	if(!SetCommState(m_hComm, &comSettings))
	{
		return 0xCC01;
	}

	m_hMutex = ::CreateMutex(NULL, FALSE, _T("Mutex4COMPort"));
	if(!m_hMutex)
	{
		return 0xCC02;
	}
	m_bConnected = TRUE;

	return 0;
}

int CSerialComm::ClosePort(void)
{
	m_bConnected = FALSE;
	SetCommMask(m_hComm, 0);
	// 통신 큐 초기화
	PurgeComm(m_hComm, PURGE_TXABORT | PURGE_RXABORT | 
					   PURGE_TXCLEAR | PURGE_RXCLEAR );

	CloseHandle(m_hComm);
	CloseHandle(m_hMutex);
	return 0;
}

int CSerialComm::WriteComm(LPBYTE pBuff, DWORD nToWrite, DWORD &dwWritten)
{
	DWORD dwLeng = 0;
	DWORD dwWaitResult = WAIT_OBJECT_0;

	dwWaitResult = WaitForSingleObject(m_hMutex, 5000);
	if(dwWaitResult == WAIT_OBJECT_0)
	{
		__try
		{
			BOOL bWrite = TRUE;
#ifdef WIN32
			if(!WriteFile(m_hComm, pBuff, nToWrite, &dwLeng, &m_osWrite))
			{
				COMSTAT comstate;
				DWORD dwError;
				DWORD dwErrorFlags;
				if(GetLastError() == ERROR_IO_PENDING)
				{
					while(!GetOverlappedResult(m_hComm, &m_osWrite, &dwLeng, TRUE))
					{
						dwError = GetLastError();
						if(dwError != ERROR_IO_INCOMPLETE)
						{
							if(dwError == ERROR_OPERATION_ABORTED)
								continue;
							else
							{
							ClearCommError(m_hComm, &dwErrorFlags, &comstate);
							bWrite = FALSE;
							break;
							}
						}
					}
				}
				else
				{
					dwWaitResult = GetLastError();
					dwWritten = 0;
					ClearCommError(m_hComm, &dwErrorFlags, &comstate);
					//bWrite= FALSE;
				}
			}
#else
			if(!WriteFile(m_hComm, pBuff, nToWrite, &dwLeng, NULL))
			{
				COMSTAT comstate;
				DWORD dwErrorFlags;
				if(GetLastError() == ERROR_IO_PENDING)
				{
					dwWaitResult = 0xEEF1;
					ClearCommError(m_hComm, &dwErrorFlags, &comstate);
					bWrite = FALSE;
				}
				else
				{
					dwWaitResult = GetLastError();
					dwWritten = 0;
					ClearCommError(m_hComm, &dwErrorFlags, &comstate);
				}
			}
			//				if(bWrite)
			//					pDBG->WriteTrace(pBuff, nToWrite, 0); //성공시에 trace write
#endif
		}
		__finally
		{
			if(!::ReleaseMutex(m_hMutex))
			{
			}
		}
		//Sleep(5);
	}
	else
	{
		dwWritten = 0;
		return (int)dwWaitResult;
	}

	dwWritten = dwLeng;
	return dwWaitResult;
}

int CSerialComm::ReadComm(BYTE* pBuff, DWORD nToRead, DWORD &dwBeRead)
{
	DWORD dwRead, dwError = 0, dwErrorFlags;
	COMSTAT comstate;

	// system queue에 도착한 byte수만 미리 읽는다.
	ClearCommError(m_hComm, &dwErrorFlags, &comstate);
	// input queue에 들어있는 데이타의 길이
	dwRead = comstate.cbInQue;

	if(dwRead > 0)
	{
#ifdef WIN32
		if(!ReadFile(m_hComm, pBuff, nToRead, &dwRead, &m_osRead))
		{
			if(GetLastError() == ERROR_IO_PENDING)
			{
				//입출력 중인 데이타가 있는 경우 timeouts에 정해준 시간만큼 기다려준다.
				while(!GetOverlappedResult(m_hComm, &m_osRead, &dwRead, TRUE))
				{
					dwError = GetLastError();
					if(dwError != ERROR_IO_INCOMPLETE)
					{
						ClearCommError(m_hComm, &dwErrorFlags, &comstate);
						break;
					}
				}
			}
			else
			{
				dwRead = 0;
				ClearCommError(m_hComm, &dwErrorFlags, &comstate);
			}
		}
#else
		if(!ReadFile(m_hComm, pBuff, nToRead, &dwRead, NULL))
		{
			dwError = GetLastError();
/*			if(dwError == ERROR_IO_PENDING)
			{
				dwError = GetLastError();
				if(dwError != ERROR_IO_INCOMPLETE)
				{
					ClearCommError(m_hComm, &dwErrorFlags, &comstate);
					dwError = 0;
				}
			}
			else
			{
				dwRead = 0;
				ClearCommError(m_hComm, &dwErrorFlags, &comstate);
			}*/
		}
		else
		{
			dwError = GetLastError();
		}
#endif
	}

	dwBeRead = dwRead;
	return dwError;
}



