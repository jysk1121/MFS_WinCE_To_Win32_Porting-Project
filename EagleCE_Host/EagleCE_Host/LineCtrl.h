/** **********************************************************
*	@file LineCtrl.h
*	@date 2017/10/20	
*	@author MFS
*	@brief TCP라인과 모뎀라인을 구현할 수 있게 만든 틀의 소스파일입니다.
**************************************************************/

// LineCtrl.h: interface for the CLineCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LINECTRL_H__583BD1E5_F01B_4DA0_A103_F2FAEFE4CEA4__INCLUDED_)
#define AFX_LINECTRL_H__583BD1E5_F01B_4DA0_A103_F2FAEFE4CEA4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Ping Test 관련 정보
#include <Icmpapi.h>
#pragma comment(lib, "Iphlpapi.lib")


typedef enum TypeOfLine
{
	LINE_TCP=0,
	LINE_DIALUP,
	LINE_CURL,		///< TCP/IP over libcurl - client side only, see CLineCurl
	MAX_LINETYPE=3 
};
/** *************************************************************************************************
@class CLineCtrl
@date 2017/10/19
@author MFS
@brief 라인과 관련된 클래스를 상속하기 위해 만든 틀입니다. 
*****************************************************************************************************/
class CLineCtrl  
{
public:
	CLineCtrl();
	virtual ~CLineCtrl();

	static void			Register(int nLineType, CLineCtrl *pLine);
	static CLineCtrl*	Instance(int LineType);

private:
	static CLineCtrl *m_pNetLines[MAX_LINETYPE]; ///< 라인 타입을 저장하는 배열입니다. 

public:
	virtual int	LineOpen(LPCTSTR Dest_Info, LPCTSTR dest_port= _T(""), LPCTSTR option=_T(""), LPCTSTR mode=_T("0"))=0;
	virtual int	LineClose()=0;
	virtual int	LineSendData(BYTE *send_data, int len)=0;

	virtual int LineAccept(BYTE opt=0)=0;
	virtual int LineCallState(int nMode=0)=0;

	virtual int TestPing(LPCTSTR sHostInfo, ICMP_ECHO_REPLY &icmpEchoReply) = 0;

protected:
//	static char		m_RecvBuf[2048];
	static char		m_RecvBuf[4096];
};

#endif // !defined(AFX_LINECTRL_H__583BD1E5_F01B_4DA0_A103_F2FAEFE4CEA4__INCLUDED_)
