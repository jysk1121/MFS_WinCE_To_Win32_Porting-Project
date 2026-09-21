// TCPIP.cpp: implementation of the CTCPIP class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TCPIP.h"

#include "Queue.h"
//#define NH_DEBUG
#include ".\Common\NHDbgApi.h"
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
extern int		put_data(char *recv_data, int recv_size);
extern int		put_event(char recv_evt);

CTCPIP::CTCPIP() : CCESocket()
{
	CLineCtrl::Register(LINE_TCP, this);
	m_callCount = 0;
	m_connectMode = 0;
	m_serviceSocket = INVALID_SOCKET;	// 2008.04.24 PSC linstener 수정
}

CTCPIP::~CTCPIP()
{
	Disconnect();
	if(m_serviceSocket != INVALID_SOCKET)
	{
		shutdown(m_serviceSocket, SD_BOTH);
		closesocket(m_serviceSocket);
		m_serviceSocket = INVALID_SOCKET;
	}
}

int	CTCPIP::LineOpen(LPCTSTR Dest_Info, LPCTSTR dest_port, LPCTSTR option, LPCTSTR mode)
{
NHDEBUG(1, (_T("LineOpen start[%d][%d]\n"), s, ssl));
	if(wcslen(Dest_Info)<1 || wcslen(dest_port)<1 || wcslen(option)<1)
		return TRUE;

	int temp_port = _ttoi(dest_port);
	bool temp_ssloption = (_ttoi(option)>0) ? TRUE : FALSE;
	int temp_mode = _ttoi(mode);

	int ret = Create(SOCK_STREAM, TCPBUFFERSIZE);
	if(ret<1)		return TRUE;
	
	if(NULL == temp_mode)
	{
		CString strIP2 = Dest_Info;
NHDEBUG(1, (_T("Line Connect start strIP2[%s] temp_port[%d] temp_ssloption[%d]\n"), strIP2, temp_port, temp_ssloption));
		ret = Connect(strIP2, (UINT)temp_port, temp_ssloption);
		m_connectMode = 0;		// client mode
	}
	else						// server mode
	{
		ret = Listen((UINT)temp_port);
		m_connectMode = 1;		// rms(listener) mode
	}

	if(ret<1)	// connect or listen fail
		return TRUE;

	if(NULL == temp_mode && ret>0)	// if client(connect) mode and function success
		put_event(CONNECT_EVT);
NHDEBUG(1, (_T("LineOpen end[%d][%d]\n"),s, ssl));
	return FALSE;
}

int	CTCPIP::LineClose()
{
NHDEBUG(1, (_T("line close start[%d][%d]\n"),s, ssl));
	Disconnect();
	if(m_serviceSocket != INVALID_SOCKET)
	{
		shutdown(m_serviceSocket, SD_BOTH);
		closesocket(m_serviceSocket);
		m_serviceSocket = INVALID_SOCKET;
	}
	m_callCount = 0;
	m_connectMode = 0;
NHDEBUG(1, (_T("line close end[%d][%d]\n"), s, ssl));
	return FALSE;
}

int	CTCPIP::LineSendData(BYTE *send_data, int len)
{
NHDEBUG(1, (_T("LineSendData[%d][%d]\n"),s, ssl));
	int sentBytes = 0;

	if(s > 1 && len > 0)
		sentBytes = Send((char *)send_data, len);

NHDEBUG(1, (_T("LineSendData[%d][%d]\n"),s, ssl));
	return sentBytes;
}

int CTCPIP::LineAccept(BYTE opt)
{
	bool ret = FALSE;
	
	//if(opt && m_callCount>0)
	if(opt && m_callCount > 0 && m_serviceSocket != INVALID_SOCKET)		/*accept option: '0x01' - call accept, '0x00' - call deny*/
		ret = AcceptServiceSocket(m_serviceSocket);

	return (int)ret;
}

int CTCPIP::LineCallState()
{
	//if(m_callCount)
	if(m_callCount > 0 && m_serviceSocket != INVALID_SOCKET)
		return TRUE;
	
	return FALSE;
}

bool CTCPIP::OnReceive(char* buf, int len)
{
	if(len <= 0)	return TRUE;
	
	put_data(buf, len);
NHDEBUG(1, (_T("receive Data(put_data): [len: %d]\n"), len));

	return TRUE;
}

void CTCPIP::OnClose(int closeEvent)
{
NHDEBUG(1, (_T("OnClose[%d]\n"),closeEvent));
	put_event(DISCONNECT_EVT);
}

bool CTCPIP::OnAccept(SOCKET serviceSocket)
{
NHDEBUG(1, (_T("OnAccept[%d]\n"),m_callCount));
	bool ret = false;

	if (m_callCount == 0)		// 무조건 1개의 connect만 허용하도록 한다.
	{
		m_callCount += 1;
		m_serviceSocket = serviceSocket;
		ret = true;
	}

	return ret;
}

