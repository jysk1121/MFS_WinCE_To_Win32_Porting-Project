// Modem.h: interface for the CModem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MODEM_H__9424A229_0F8A_44A2_ACC8_6B45BAF7DC35__INCLUDED_)
#define AFX_MODEM_H__9424A229_0F8A_44A2_ACC8_6B45BAF7DC35__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include ".\Net\LineCtrl.h"
#include "Comm.h"
#include <tapi.h>

class CModem : public CLineCtrl
{
public:
	CModem();
	virtual ~CModem();

public:
	virtual int	LineOpen(LPCTSTR Dest_Info,		/*phone number*/
						 LPCTSTR dest_port=L"", /*reserved*/
						 LPCTSTR option=L"",	/*call origin mode: "1" - Rms, "0" - host*/
						 LPCTSTR mode=L"");		/*modem mode: "1" - ready for Rms, "0" - host connect mode*/
	virtual int	LineClose();
	virtual int	LineSendData(BYTE *send_data, int len);
	virtual int LineAccept(BYTE opt=0);
	virtual int LineCallState();

private:
	LONG	Initialize();
	LONG	Open();
	LONG	Answer();
	LONG	Dial(LPCSTR pszPhoneNo);
	LONG	Close();
	LONG	Shutdown();
	BOOL	HangUp();		// [#525] US KSK 2009.05.25


	friend void CALLBACK LineCallbackFunc(DWORD hDevice,  
										  DWORD dwMsg,
										  DWORD dwCallbackInstance,
										  DWORD dwParam1, 
										  DWORD dwParam2,
										  DWORD dwParam3);	

};

#if (_WIN32_WCE < 0x600)
static CModem theDialLine;
#endif

#endif // !defined(AFX_MODEM_H__9424A229_0F8A_44A2_ACC8_6B45BAF7DC35__INCLUDED_)
