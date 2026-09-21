// Comm.h: interface for the CComm class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COMM_H__D8079A33_8F9A_4100_998E_D5F059259501__INCLUDED_)
#define AFX_COMM_H__D8079A33_8F9A_4100_998E_D5F059259501__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define	INPUTBUFFERSIZE		4096

class CComm
{
public:
	CComm();
	virtual ~CComm();

	BOOL	OpenPort(HANDLE hComm);
	void	ClosePort(void);
	BOOL	WriteData(LPBYTE lpBufferToWrite, DWORD dwSizeToWrite);

protected:
	BOOL	StartReadThread(void);
	void	StopReadThread(void);
	BOOL	SetupReadEvent(LPOVERLAPPED lpOverlappedRead,
		LPBYTE lpInputBuffer, DWORD dwSizeofBuffer,
		LPDWORD lpdwNmberOfBytesRead);

	friend	DWORD WINAPI ReadThreadProc(LPVOID lpvParam);

private:
	DWORD			m_dwEvtMask;
	COMMTIMEOUTS	m_CommTimeouts;
	DCB				m_dcb;

	HANDLE			m_hComm;

	HANDLE			m_hReadThread;
	HANDLE			m_hCloseReadEvent;

	int				m_nCallOrigin;	// [#527] US KSK 2009.5.29
};

#endif // !defined(AFX_COMM_H__D8079A33_8F9A_4100_998E_D5F059259501__INCLUDED_)
