// TCPIP.h: interface for the CTCPIP class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TCPIP_H__7A81CB8E_188E_4FC6_8661_2AFBB6581CC0__INCLUDED_)
#define AFX_TCPIP_H__7A81CB8E_188E_4FC6_8661_2AFBB6581CC0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include ".\Net\LineCtrl.h"
#include "CESocket.h"

class CTCPIP : public CLineCtrl, public CCESocket
{
public:
	CTCPIP();
	virtual ~CTCPIP();

	int m_callCount;
	int m_connectMode;
	SOCKET m_serviceSocket;		// 2008.04.24 PSC linstener 수정

public:
	virtual int	LineOpen(LPCTSTR Dest_Info,			/*ip:[xxx.xxx.xxx.xxx]*/
						 LPCTSTR dest_port=L"",		/*port:[xxxxx]*/ 
						 LPCTSTR option=L"",		/*SSL mode: "1" - enable, "0" - disable*/	 
						 LPCTSTR mode=L"");			/*socket mode: "1" - ready for Rms, "0" - host connect mode*/
	virtual int	LineClose();
	virtual int	LineSendData(BYTE *send_data, int len);
	virtual int LineAccept(BYTE opt);				/*accept option: '0x01' - call accept, '0x00' - call deny*/
	virtual int LineCallState();

	virtual bool OnReceive(char* buf, int len);
	virtual void OnClose(int closeEvent);
	//virtual bool OnAccept();
	virtual bool OnAccept(SOCKET serviceSocket);
};

#if (_WIN32_WCE < 0x600)
static CTCPIP theTcpLine;
#endif

#endif // !defined(AFX_TCPIP_H__7A81CB8E_188E_4FC6_8661_2AFBB6581CC0__INCLUDED_)
