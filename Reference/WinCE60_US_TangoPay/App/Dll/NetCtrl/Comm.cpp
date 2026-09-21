// Comm.cpp: implementation of the CComm class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Comm.h"

#include ".\Common\NHDbgApi.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern int		put_data(char *recv_data, int recv_size);
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CComm::CComm()
{
	m_hComm = NULL;
	m_hReadThread = NULL;
	m_hCloseReadEvent = NULL;

	m_dwEvtMask = 0;
	memset(&m_CommTimeouts, 0, sizeof(m_CommTimeouts));
	memset(&m_dcb, 0, sizeof(m_dcb));
}

CComm::~CComm()
{
	ClosePort();
}


BOOL CComm::OpenPort(HANDLE hComm)
{
	NHDEBUG(1, (TEXT("CComm::OpenPort()\n")));

	CString			strError;

	if (m_hComm != NULL)		return TRUE;

	DWORD			dwErrorFlags;
	COMSTAT			comstate;

	if (hComm == NULL)
	{
		NHDEBUG(0, (TEXT("CComm ERROR:0")));
		return FALSE;
	}

	// only clear error state.
	ClearCommError(hComm, &dwErrorFlags , &comstate);

	if (::GetCommMask(hComm, &m_dwEvtMask) == FALSE)
	{
		NHDEBUG(0, (TEXT("CComm ERROR:1")));
		ClosePort();
		return FALSE;
	}

	if (::GetCommState(hComm, &m_dcb) == FALSE)
	{
		NHDEBUG(0, (TEXT("CComm ERROR:2")));
		ClosePort();
		return FALSE;
	}

	if (::GetCommTimeouts(hComm, &m_CommTimeouts) == FALSE)
	{
		NHDEBUG(0, (TEXT("CComm ERROR:3")));
		ClosePort();
		return FALSE;
	}

	// Set the comm mask so we receive error signals.
	if ( ::SetCommMask(hComm, EV_RXCHAR | EV_ERR |EV_BREAK |EV_TXEMPTY) == FALSE)
	{
		NHDEBUG(0, (TEXT("CComm ERROR:4")));
		ClosePort();
		return FALSE;
	}

	m_dcb.DCBlength		= sizeof(DCB);
	m_dcb.fBinary		= TRUE;
	m_dcb.fParity		= TRUE;
	m_dcb.fErrorChar	= FALSE;
	m_dcb.fNull			= FALSE;
	m_dcb.fAbortOnError = FALSE;

	// RMS position
	m_dcb.ByteSize		= 8;
	m_dcb.Parity		= NOPARITY;
	m_dcb.StopBits		= ONESTOPBIT;

	if (::SetCommState(hComm, &m_dcb) == FALSE)
	{
		NHDEBUG(0, (TEXT("CComm ERROR:5")));
		ClosePort();
		return FALSE;
	}

	//SPR 대책 내용과 동일 적용
	// adjust ReadTimeouts to make ReadFile() non-blocking. It will return immediately when it received no data. (i.e. AT response when modem not attached)
	//m_CommTimeouts.ReadIntervalTimeout = 100;
	//m_CommTimeouts.ReadTotalTimeoutMultiplier = 3;
	//m_CommTimeouts.ReadTotalTimeoutConstant = 30;
	m_CommTimeouts.ReadIntervalTimeout = 0xFFFFFFFF;
	m_CommTimeouts.ReadTotalTimeoutMultiplier = 0;
	m_CommTimeouts.ReadTotalTimeoutConstant = 0;
	
	m_CommTimeouts.WriteTotalTimeoutMultiplier = 50;
	m_CommTimeouts.WriteTotalTimeoutConstant = 150;

	if (!::SetCommTimeouts(hComm, &m_CommTimeouts))
	{
		NHDEBUG(0, (TEXT("CComm ERROR:8")));
		ClosePort();
		return FALSE;
	}

	if (::PurgeComm(hComm,
		PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_RXCLEAR) == FALSE)
	{
		NHDEBUG(0, (TEXT("CComm ERROR:6")));
		ClosePort();
		return FALSE;
	}

	m_hComm = hComm;

	if (StartReadThread() == FALSE)
	{
		NHDEBUG(0, (TEXT("CComm ERROR:7")));
		ClosePort();
		return FALSE;
	}

	return TRUE;
}

void CComm::ClosePort()
{
	NHDEBUG(1, (TEXT("CComm::ClosePort()\n")));
	StopReadThread();

	if (m_hComm != NULL)
	{
		::PurgeComm(m_hComm,
					PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_RXCLEAR);
		::SetCommMask(m_hComm, m_dwEvtMask);
		::SetCommState(m_hComm, &m_dcb);
		::SetCommTimeouts(m_hComm, &m_CommTimeouts);
		::CloseHandle(m_hComm);
		m_hComm = NULL;
	}
}

BOOL CComm::StartReadThread(void)
{
	DWORD		m_dwReadThreadID;

	m_hCloseReadEvent = CreateEvent(NULL,		// Security Attributes
									  TRUE,		// Reset Type : Manual reset
									  FALSE,	// Initial State : non-signaled
									  NULL);	// Object Name
	if (m_hCloseReadEvent == NULL)
		return FALSE;

	m_hReadThread = ::CreateThread(NULL,				// Security Attributes
								   8192,				// Stack Size : 8 Kb
								   ReadThreadProc,		// Start Function
								   this,				// Thread Parameter
								   0,					// Creation Flag
								   &m_dwReadThreadID);	// ThreadId
	if (m_hReadThread == NULL)
		return FALSE;
	
	return TRUE;
}

void CComm::StopReadThread(void)
{
	if (m_hReadThread == NULL)
	{
		return;
	}

	if (m_hCloseReadEvent != NULL)		// [#2022] NH KSK 2011.02.22
	{
		SetEvent(m_hCloseReadEvent);
		NHDEBUG(1, (TEXT("m_hCloseReadEvent Set\n")));
	}

	if (::WaitForSingleObject(m_hReadThread, 10 * 1000) == WAIT_TIMEOUT)
	{
		::TerminateThread(m_hReadThread, 0);
	}

	if (::CloseHandle(m_hReadThread))
	{
		m_hReadThread = NULL;
	}

	// [#2022] NH KSK 2011.02.22
	if (m_hCloseReadEvent != NULL && ::CloseHandle(m_hCloseReadEvent))
	{
		m_hCloseReadEvent = NULL;
	}
	// end of [#2022]
}

BOOL CComm::WriteData(LPBYTE lpBufferToWrite, DWORD dwSizeToWrite)
{
	DWORD	dwNumberOfBytesWritten = 0;
	DWORD	dwWhereToStartWriting = 0;		// Start at the beginning.
	DWORD	dwErrorFlags;
	COMSTAT	comstate;
	DWORD	dwEvent;

	if (m_hComm == NULL)	return FALSE;

	::PurgeComm(m_hComm, PURGE_TXABORT | PURGE_TXCLEAR);

   	ClearCommError(m_hComm, &dwErrorFlags , &comstate);

	if (::WriteFile(m_hComm,
		&lpBufferToWrite[dwWhereToStartWriting],
		dwSizeToWrite,
		&dwNumberOfBytesWritten,
			NULL))
		{
			if(dwSizeToWrite == dwNumberOfBytesWritten)
			{
				if(WaitCommEvent(m_hComm, &dwEvent, NULL))
				{
					if (dwEvent & EV_TXEMPTY)		return TRUE;
				} 
				ClearCommError(m_hComm, &dwErrorFlags , &comstate);
			}
			else
			{
				ClearCommError(m_hComm, &dwErrorFlags , &comstate);
				return FALSE;
			}
		}
		
	return FALSE;
}

DWORD WINAPI ReadThreadProc(LPVOID lpvParam)
{
	CComm	*pComm = (CComm *)lpvParam;
	BYTE	szInputBuffer[INPUTBUFFERSIZE];
	DWORD	dwNumberOfBytesRead;

	while(TRUE)
	{
		// exit this loop when 'CloseReadEvent' is set
		DWORD dwResult = WaitForSingleObject(pComm->m_hCloseReadEvent, 0);
		NHDEBUG(1, (TEXT("dwResult = WaitForSingleObject: %d\n"), dwResult));

		if(dwResult != WAIT_TIMEOUT)
		{
			NHDEBUG(1, (TEXT("WaitResult:m_hCloseReadEvent Set, Terminating this thread.\n")));
			break;
		}

		// otherwise, read data from COM port.
		pComm->SetupReadEvent(NULL, szInputBuffer, INPUTBUFFERSIZE, &dwNumberOfBytesRead);
		Sleep(100);
	}

	NHDEBUG(1, (TEXT("ReadThreadProc()::return\n")));
	return 0;
}

BOOL CComm::SetupReadEvent(LPOVERLAPPED lpOverlappedRead,
						   LPBYTE lpszInputBuffer, DWORD dwSizeofBuffer,
						   LPDWORD lpdwNumberOfBytesRead)
{
	// timeout of ReadFile() depends on m_CommTimeouts set by SetCommTimeouts()
	BOOL bReadRet = ::ReadFile(m_hComm,
								lpszInputBuffer,
								dwSizeofBuffer,
								lpdwNumberOfBytesRead,
								lpOverlappedRead);
	
	if (bReadRet)
	{
		if(*lpdwNumberOfBytesRead == 0)
			return TRUE;

		put_data((char *)lpszInputBuffer, *lpdwNumberOfBytesRead);
	}

	return TRUE;
}
