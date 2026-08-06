/** ***************************************************************
*	@file LineModem.h
*	@date 2017/10/20
*	@author MFS
*	@brief 실질적인 통신을 수행하는 모뎀라인을 구현한 헤더파일입니다. 
*******************************************************************/

#ifndef __LINE_MODEM_H__
#define __LINE_MODEM_H__

#include "../../Template/MFSCtrlThread.h"
#include "LineCtrl.h"
#include <Tapi.h>

#ifndef _WIN32_WCE
// Win32 desktop: TAPI is exported from tapi32.dll (separate import lib),
// unlike WinCE where these symbols come from the core CE import libraries.
#pragma comment(lib, "tapi32.lib")
#endif

#include "../../EagleCE_Framework/EagleCE_Framework/EagleConfig.h"

typedef struct tagLINEINFO
{
  HLINE hLine;              // Line handle returned by lineOpen
  BOOL  bVoiceLine;         // Indicates if the line is a voice line
  DWORD dwAPIVersion;       // API version that the line supports
  DWORD dwNumOfAddress;     // Number of available addresses on the line
  DWORD dwPermanentLineID;  // Permanent line identifier
  TCHAR szLineName[256];    // Name of the line
} LINEINFO, *LPLINEINFO;

/** ************************************************************
*	@class CLineModem 
*	@date 2017/10/20
*	@author MFS
*	@brief 서버 또는 호스트와의 모뎀통신을 구현한 클래스입니다.
****************************************************************/
class CLineModem : public CLineCtrl, public CMFSCtrlThread
{
public:
	CLineModem();
	~CLineModem();

	/* LINECTRL INTERFACE */
public:
	virtual int	LineOpen(LPCTSTR Dest_Info,				// phone number
						 LPCTSTR dest_port=_T(""),		// not used
						 LPCTSTR option=_T(""),			// option: "1" - ams init string, "0" - host init string 
						 LPCTSTR mode=_T(""));			// mode: "1" - ready for Rms, "0" - host connect mode
	virtual int	LineClose();
	virtual int	LineSendData(BYTE *send_data, int len);
	virtual int LineAccept(BYTE opt);					/*accept option: '0x01' - call accept, '0x00' - call deny*/
	virtual int LineCallState(int nMode=0);

	virtual int TestPing(LPCTSTR sHostInfo, ICMP_ECHO_REPLY &icmpEchoReply) { return 0; };	// Modem은 Ping 미지원

private:
	BOOL	m_bConnectStatus;

	CEagleConfig	m_EagleConfig;

	/* THREAD */
protected:
	unsigned ThreadHandlerProc(void);		// CommPort Read Check Thread.
	unsigned LineEventThread(void);			// Modem Event Check Thread

	/* INTER USE */
private:
	VOID	SetModemInitString(BOOL bRmsInitString);
	BOOL	InitializeTAPI();
	BOOL	IsNeedLineDev(DWORD dwLineID, LPLINEINFO lpLineInfo);
	DWORD	OpenPhoneLine();
	VOID	ClosePhoneLine();
	long	MakePhoneCall(LPCTSTR lpszPhoneNum);
	HANDLE	GetComportHandle(DWORD *lpRet);
	BOOL	SetupComPort(HANDLE hComm, BOOL bOutgoingCall);
	DWORD	Answer();

	static DWORD _stdcall CLineModem::LineEventThreadProc( void *pThreadHandler );

private:
	BOOL			m_bExitThread;			// Control flag to exit for all Thread.

	/* CHECK SERIAL THREAD CONTROL */
	CMFSEvent		m_eStartCheckSerial;	// Start signal for Check Serial Thread.
	BOOL			m_bCheckSerial;			// Control flag to check for Check Serial Thread.
	HANDLE			m_hComm;				// Comport Handle of modem serial port

	DWORD			m_dwEvtMask;
	DCB				m_dcb;
	COMMTIMEOUTS	m_CommTimeouts;

	/* CHECK LINE THREAD CONTROL*/
	HANDLE			m_hLineEventThread;		// Hanlde of Check Line Thread.
	DWORD			m_nLineEventThread;		// ID of Check Line Thread.
	CMFSEvent		m_eStartCheckLine;		// Start signal for Check Line Thread.
	BOOL			m_bCheckLine;			// Control flag to check for Check Line Thread.
	HANDLE			m_hLineEvent;			// Event Handle of Modem Line

	/* TAPI CONTROL */
	HLINEAPP		m_hLineApp;				// Application's use handle for TAPI
	DWORD			m_nLineID;				// device identifier to use
	HCALL			m_hCall;				// Handle to the open line device on 
	LINEINFO		m_CurLineInfo;			// Line Information of current Line.
	LINEDEVCAPS		m_LineDevCaps;			// Device Information of current Device
	LINEINFO		m_TmpLineInfo;
	
	/* USE INFORMATION */
	BOOL			m_bOutgoingCall;		// Communication call type. (outgoing or incoming)
	BOOL			m_bRMSMode;
	LONG			m_nMakeCallRequestID;   // Request identifier returned by 
	BOOL			m_bNewCall;				// incoming call flag.

	CTime			m_CheckRingEventTime;
	int				m_nRingCount;
};

static CLineModem theCLineModem;

#endif __LINE_MODEM_H__