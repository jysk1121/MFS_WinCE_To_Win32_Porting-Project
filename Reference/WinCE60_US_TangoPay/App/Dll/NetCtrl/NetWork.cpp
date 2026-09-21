// NetWork.cpp: implementation of the CNetWork class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include ".\Dev\DevDefine.h"
#include ".\Net\NetWork.h"

#include "VISAIICtrl.h"
#include "StandardCtrl.h"
#include "UseAckCtrl.h"
#include "RMSFlowCtrl.h"
#include "EventNoticeCtrl.h"	// [#2313] US Justin 2014.11.20 Add Event Notice
#include ".\Common\ScreenDef.h"	// [#257] [MX] KSK 2008.6.5

#include <fstream>

#if (APP_LIBERTYX)
#include ".\Common\LibertyXConfigurationManager.h"
#endif

// [#2358] NH Kook 2015.07.14 support 'PING' function.
#ifndef UNDER_CE
#  include "Ipexport.h"
#endif
#include "Icmpapi.h"
#include "winsock.h"
// #include <ntddndis.h>
// #include <winioctl.h>
#include <iphlpapi.h>				// 2015.10.14 it is also used to renewing IP.

#pragma comment (lib, "Iphlpapi.lib") 
// #pragma comment (lib, "Ws2.lib")
// end of [#2358]

#ifdef APP_AGING_MODE
	//#define NH_DEBUG
	#include ".\Common\NHDbgApi.h"
	#define DBG_INFO	0
	#define DBG_AGING	1
#else
	//#define NH_DEBUG
	#include ".\Common\NHDbgApi.h"
	#define DBG_INFO	1
	#define DBG_AGING	1
#endif

// [#GLDV-2683] NH Kook 2019.11.19 Modem Removal Detection
#include "Comm.h"

extern int	get_queue(char *ctl_code);
extern int	get_size();
// end of [#GLDV-2683]

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CNetWork::CNetWork(int LineType, int RMSClientLineType, int RMSServerLineType)
{
	m_bRMSConnectOpen = FALSE;
	strErrorCode = _T("");
	strErrorMsg  = _T("");
	m_nCurrentFlowLine = -1;
	m_nCurrentLine = LineType;
	m_nCurrentRMSClientLine = RMSClientLineType;
	m_nCurrentRMSServerLine = RMSServerLineType;

	CLineCtrl *line = CLineCtrl::Instance(LineType);
	CLineCtrl *clientLine = CLineCtrl::Instance(m_nCurrentRMSClientLine);
	CLineCtrl *serverLine = CLineCtrl::Instance(m_nCurrentRMSServerLine);

	for (int i = 0; i < MAX_FLOWLINE; i++)
	{
		m_FlowAndLine[i] = NULL;

		switch(i)
		{
		case VISAII_FLOW:
			// Dialup or TCPIP CHECK
			m_FlowAndLine[i] = (CFlowCtrl*) new CVISAIICtrl(line);
			break;
		case STANDARD_FLOW:
			m_FlowAndLine[i] = (CFlowCtrl*) new CStandardCtrl(line);
			break;
		case ACKCONTROLLED_FLOW:
			m_FlowAndLine[i] = (CFlowCtrl*) new CUseAckCtrl(line);
			break;			
		case NHRMS_FLOW:
			m_FlowAndLine[i] = (CFlowCtrl*) new CRMSFlowCtrl(clientLine, serverLine);
			break;
		// [#2313] US Justin 2014.11.20 
		case HTTP_POST_FLOW:			// [#2347] Justin 2015.05.27
			m_FlowAndLine[i] = (CFlowCtrl*) new CEventNoticeCtrl(line);
			break;
		// end of [#2313]
		default:
			break;
		}
	}
}

CNetWork::~CNetWork()
{
	for (int i = 0; i < MAX_FLOWLINE; i++)
	{
		if (m_FlowAndLine[i])	delete m_FlowAndLine[i];
	}
}

int	CNetWork::SetInformation(int CtrlType)
{
	if (CtrlType < VISAII_FLOW)		return 1;
	if (CtrlType > MAX_FLOWLINE)	return 2;
//	if (LineType < LINE_DIALUP)		return 3;
//	if (LineType > MAX_LINETYPE)	return 4;


	if (m_bRMSConnectOpen)
		RMSConnectClose();

	m_nCurrentFlowLine	= CtrlType;
//	m_nCurrentLine		= LineType;

	return 0;
}

// [#2012] NH KJW 2011.01.26 Lottery URL, SSL 지원
//int	CNetWork::SendData(BYTE *pSendBuf, int Length, int SendSec)
int	CNetWork::SendData(BYTE *pSendBuf, int Length, int SendSec, int n3rdTranType, LPCTSTR sHostInfo) // [#2412] NH Justin 2016.04.05 Add Extra Host Info
// end of [#2012]
{
	NHDEBUG(DBG_INFO, (_T("***Nerwork.cpp***CNetWork::SendData() \n")));

	int		rty_cnt = 0;
	int		max_rty_cnt = 0;	// [#2068] NZ KSK 2011.06.09 거래시간 단축을 위해 TCP/IP인 경우 retry count 줄임
	int		nreturn	= 0;
	CString	CurrentPhone = _T("");
	DWORD	Timeout = GetTickCount() + SendSec * 1000;

	// [#2068] NZ KSK 2011.06.09
#if (AU_VERSION)
	if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == 1)
		max_rty_cnt = 3;
	else
		max_rty_cnt = 1;
//[#2169] CA Justin 2012.12.18 Reduce Host connection retrial Number (Canada) : 3 => 1
//#elif (CA_VERSION)
//		max_rty_cnt = 1;
// End of [#2169]
#else
		// [#2169] NH KSK 2013.01.17 EMV가 Enable일 경우 TCP Connect Retry 횟수 변경 (3->1)
	    //							 IC 거래인지 MS 거래인지 network단에서는 조회가 안되므로 Enable 여부로만 판단하도록 함
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_EMV_ENABLE) == ENABLE)
			max_rty_cnt = 1;
		else
			max_rty_cnt = 3;
		// end of [#2169]
#endif
	// end of [#2068]

	// Invalid Check
	if (m_nCurrentFlowLine < VISAII_FLOW)		return 1;
	if (m_nCurrentFlowLine >= MAX_FLOWLINE)		return 2;

	if (m_bRMSConnectOpen)
		RMSConnectClose();

	// Wait timeout
	// 3회 RETRY를 해야한다.(INTERFACE를 맞추기 위해)
	// MAX TIMEOUT을 설정해야함(APHOLDING 방지)
	while(GetTickCount() < Timeout)
	{
		if (m_nCurrentLine == LINE_DIALUP)
		{
			NHDEBUG(DBG_INFO, (_T("***Nerwork.cpp***CNetWork::SendData() LINE_DIALUP \n")));

			if( n3rdTranType == TRANHOST_ATM ) // [#2051] NH KJW 2011.01.26 Lottery Dial-Up 지원	// [#2185] NH Justin 2013.05.06	Host Type Definition.
			{
				// [#458] [NH] KSK 2008.11.26 Backup Number가 NULL일 경우 Skip하도록 수정
				if (!(rty_cnt%2))		CurrentPhone = MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_PHONENODEFINE1);
				else
				{
					if (MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_PHONENODEFINE2).GetLength() != 0)
						CurrentPhone = MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_PHONENODEFINE2);
					else
						CurrentPhone = MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_PHONENODEFINE1);
				}
				// end of [#458]
				
				NHDEBUG(DBG_INFO, (_T("(%d)SEND PHONE NUMBER[%s][%x]\n"), m_nCurrentFlowLine, CurrentPhone, rty_cnt));

				nreturn = m_FlowAndLine[m_nCurrentFlowLine]->SendData(pSendBuf, Length, CurrentPhone, L"", L"0");
			}
			// [#2185] US Justin 2013.05.06 Add Multi Host 
			else
				nreturn = 3;	// ERROR GENERATION [NO ANSWER]
			// End of [#2185]

			if (!nreturn)
			{
				NHDEBUG(DBG_INFO, (_T("SEND SUCCESS[%x]\n"), nreturn));
				// send ok
				return 0;
			}
			else
			{
				NHDEBUG(DBG_INFO, (_T("SEND ERROR RETURN[%x][%x]\n"), nreturn, rty_cnt));
				NHDEBUG(DBG_AGING, (_T("SEND ERROR RETURN[%x][%x]\n"), nreturn, rty_cnt));
				
				// send error (control error or dial fail)
				// send complete시점에서 부터는 retry하지 않음
				if ((nreturn >= 6) && (nreturn <= 10))
				{
					NHDEBUG(DBG_INFO, (_T("SEND CONTROL ERROR[%x][%x]\n"), nreturn, rty_cnt));
					NHDEBUG(DBG_AGING, (_T("SEND CONTROL ERROR[%x][%x]\n"), nreturn, rty_cnt));
					goto net_error;
				}
			}
		}
		else if (m_nCurrentLine == LINE_TCP || m_nCurrentLine == LINE_TLS)
		{
			if( n3rdTranType == TRANHOST_ATM ) // [#2012] NH KJW 2011.01.26 Lottery URL, SSL 지원		// [#2185] NH Justin 2013.05.06	Host Type Definition.
			{
				// [#458] [NH] KSK 2008.11.26 Backup Number가 NULL일 경우 Skip하도록 수정	
				if (!(rty_cnt%2))
				{
					m_tcp.IpAddr1 = MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_HOST1NAME);
					m_tcp.port1.Format(L"%d", MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_HOST1PORTNO));
				}
				else
				{
					if (MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_HOST2NAME).GetLength() != 0)
					{
						m_tcp.IpAddr1 = MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_HOST2NAME);
						m_tcp.port1.Format(L"%d", MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_HOST2PORTNO));
					}
					else
					{
						m_tcp.IpAddr1 = MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_HOST1NAME);
						m_tcp.port1.Format(L"%d", MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_HOST1PORTNO));
					}
				}
				// end of [#458]

//	 			if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_SSLENABLE))
// 					m_tcp.SSLOption.Format(L"%d", MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SSLVERSION));
// 				else
// 					m_tcp.SSLOption = L"0";
			
				// [#2093] PCS 2011.10.28 "SSL Option설정"
				if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_SSLENABLE))
				{
					int nSSLVer = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SSLVERSION);
					int nCertEn = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SSL_CERTIFY);
					if( nSSLVer == CONN_UPTO_SSL_V30)
					{
						if (nCertEn == ENABLE)		m_tcp.SSLOption.Format(L"%d", CONN_UPTO_SSL_V30_USE_CERT);
						else						m_tcp.SSLOption.Format(L"%d", CONN_UPTO_SSL_V30);
					}
					else if( nSSLVer == CONN_UPTO_TLS_V10)
					{
						if (nCertEn == ENABLE)		m_tcp.SSLOption.Format(L"%d", CONN_UPTO_TLS_V10_USE_CERT);
						else						m_tcp.SSLOption.Format(L"%d", CONN_UPTO_TLS_V10);
					}
					else if( nSSLVer == CONN_UPTO_TLS_V11)
					{
						if (nCertEn == ENABLE)		m_tcp.SSLOption.Format(L"%d", CONN_UPTO_TLS_V11_USE_CERT);
						else						m_tcp.SSLOption.Format(L"%d", CONN_UPTO_TLS_V11);
					}
					else if( nSSLVer == CONN_UPTO_TLS_V12)
					{
						if (nCertEn == ENABLE)		m_tcp.SSLOption.Format(L"%d", CONN_UPTO_TLS_V12_USE_CERT);
						else						m_tcp.SSLOption.Format(L"%d", CONN_UPTO_TLS_V12);
					}
					else if( nSSLVer == CONN_UPTO_TLS_V13)
					{
						if (nCertEn == ENABLE)		m_tcp.SSLOption.Format(L"%d", CONN_UPTO_TLS_V13_USE_CERT);
						else						m_tcp.SSLOption.Format(L"%d", CONN_UPTO_TLS_V13);
					}
					// End of [#2320]
					// end of [#2310]
				}
				else
				{
					m_tcp.SSLOption = L"0";
				}
				// end of [#2093] PCS 2011.10.28 "SSL Option설정"

				m_FlowAndLine[m_nCurrentFlowLine]->SetLine(CLineCtrl::Instance(LINE_TLS));
			
				NHDEBUG(DBG_INFO, (_T("SEND IP INFORMATION[%s] [%s][%s][%s]\n"), pSendBuf+2, m_tcp.IpAddr1, m_tcp.port1, m_tcp.SSLOption));
			
				//[#2093] NH PCS 2011.10.26 "SSL CERT. 검증의 리턴값이 여기에서 올라오게 된다."
				nreturn = m_FlowAndLine[m_nCurrentFlowLine]->SendData(pSendBuf, Length, m_tcp.IpAddr1, m_tcp.port1, m_tcp.SSLOption);
			}
			// [#2185] NH Justin 2013.05.06	Add Dual Host
			else if( n3rdTranType == TRANHOST_DUALHOST )		
			{
				if (!(rty_cnt%2))
				{
					m_tcp.IpAddr1 = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_HOST1NAME);
					m_tcp.port1.Format(L"%d", MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_HOST1PORTNO));
				}
				else
				{
					if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_HOST2NAME).GetLength() != 0)
					{
						m_tcp.IpAddr1 = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_HOST2NAME);
						m_tcp.port1.Format(L"%d", MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_HOST2PORTNO));
					}
					else
					{
						m_tcp.IpAddr1 = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_HOST1NAME);
						m_tcp.port1.Format(L"%d", MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_HOST1PORTNO));
					}
				}
				// end of [#458]

				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_SSL_ENABLE))
				{
					m_tcp.SSLOption.Format(L"%d", MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_SSL_VERSION));
				}
				else
				{
					m_tcp.SSLOption = L"0";
				}

				m_FlowAndLine[VISAII_FLOW]->SetLine(CLineCtrl::Instance(LINE_TLS));

				NHDEBUG(DBG_INFO, (_T("SEND IP INFORMATION[%S] [%s][%s][%s]\n"), pSendBuf, m_tcp.IpAddr1, m_tcp.port1, m_tcp.SSLOption));
				nreturn = m_FlowAndLine[VISAII_FLOW]->SendData(pSendBuf, Length, m_tcp.IpAddr1, m_tcp.port1, m_tcp.SSLOption);
			}
#if (APP_LIBERTYX)
			else if( n3rdTranType == TRANHOST_LIBERTYX ) // [#RWC6-59] US William 2019.10.08 LibertyX	
			{
				// Get the configuration
				LXConfiguration config;
				CLibertyXConfigurationManager configManager;
				
				if (!configManager.GetConfiguration(config))
				{
					nreturn = -1; //IDK what to do here, yet...
					goto net_error;
				}

				int flowType = STANDARD_FLOW;
				switch (config.EnvelopeType)
				{
				case LX_VISAII:
					flowType = VISAII_FLOW;
					break;
				case LX_STANDARD:
					flowType = STANDARD_FLOW;
					break;
				case LX_ACK:
					flowType = ACKCONTROLLED_FLOW;
					break;
				}

				if (!(rty_cnt%2))
				{
					m_tcp.IpAddr1 = config.Host1Url;
					m_tcp.port1.Format(L"%d", config.Host1Port);
				}
				else
				{
					if (config.Host2Url.GetLength() != 0)
					{
						m_tcp.IpAddr1 = config.Host2Url;
						m_tcp.port1.Format(L"%d", config.Host2Port);
					}
					else
					{
						m_tcp.IpAddr1 = config.Host1Url;
						m_tcp.port1.Format(L"%d", config.Host1Port);
					}
				}

				if (config.TlsEnabled)
				{
					m_tcp.SSLOption.Format(L"%d", CONN_UPTO_TLS_V12_USE_CERT);
				}
				else
				{
					m_tcp.SSLOption = L"0";
				}

				m_FlowAndLine[flowType]->SetLine(CLineCtrl::Instance(LINE_TLS));

				NHDEBUG(DBG_INFO, (_T("SEND IP INFORMATION[%S] [%s][%s][%s]\n"), pSendBuf, m_tcp.IpAddr1, m_tcp.port1, m_tcp.SSLOption));
				nreturn = m_FlowAndLine[flowType]->SendData(pSendBuf, Length, m_tcp.IpAddr1, m_tcp.port1, m_tcp.SSLOption);
			}
			// End of [#2185]
#endif
			// [#2405] NH Justin 2016.03.16	HalCash Online
			else if( (n3rdTranType == TRANHOST_PIN4_TRANSACTION)||(n3rdTranType == TRANHOST_PIN4_REVERSAL) || (n3rdTranType ==TRANHOST_PIN4_GETTOKEN) )	// [#2515]
			{
				rty_cnt = max_rty_cnt;		// One time sending....
				// Parse Host Information....
				if( sHostInfo == NULL )
				{
					nreturn = 1;
					goto net_error;
				}
				CStringArray strTempArray;
				SplitString(sHostInfo, ",", strTempArray);			// IP, PORT, SSLOPTION, HOST CERT
				if(	strTempArray.GetSize() < 4 )
				{
					nreturn = 1;
					goto net_error;
				}				
				m_tcp.IpAddr1	= strTempArray.GetAt(0);
				m_tcp.port1		= strTempArray.GetAt(1);
				m_tcp.SSLOption.Format(L"%s:%s", strTempArray.GetAt(2), strTempArray.GetAt(3));

				m_FlowAndLine[HTTP_POST_FLOW]->SetLine(CLineCtrl::Instance(LINE_TLS));

				CString sConnectionOption = m_tcp.SSLOption;
				sConnectionOption += L",TERMINATE:";
				if(n3rdTranType == TRANHOST_PIN4_REVERSAL)		sConnectionOption += L":Envelope>";	
				else if(n3rdTranType == TRANHOST_PIN4_GETTOKEN) sConnectionOption += L"\"}";
				else											sConnectionOption += L":Envelope>";

				// NB: The payloads to MCCP are sometimes too large for the serial buffer, and it crashes the AP.
				//NHTRACE(DBG_INFO, (_T("SEND IP INFORMATION[%S] [%s][%s][%s]\n"), pSendBuf, m_tcp.IpAddr1, m_tcp.port1, sConnectionOption));
				nreturn = m_FlowAndLine[HTTP_POST_FLOW]->SendData(pSendBuf, Length, m_tcp.IpAddr1, m_tcp.port1, sConnectionOption);		
			}
			// End of [#2405]
			// [#2446] US Justin 2016.10.07 Paypay CCA
			else if( n3rdTranType == TRANHOST_PAYPAL_TRANSACTION )
			{
				rty_cnt = max_rty_cnt;		// One time sending....
				// Parse Host Information....
				if( sHostInfo == NULL )
				{
					nreturn = 1;
					goto net_error;
				}
				CStringArray strTempArray;
				SplitString(sHostInfo, ",", strTempArray);			// IP, PORT, SSLOPTION, HOST CERT
				if(	strTempArray.GetSize() < 4 )
				{
					nreturn = 1;
					goto net_error;
				}				
				m_tcp.IpAddr1	= strTempArray.GetAt(0);
				m_tcp.port1		= strTempArray.GetAt(1);
				m_tcp.SSLOption.Format(L"%s:%s", strTempArray.GetAt(2), strTempArray.GetAt(3));
				m_FlowAndLine[HTTP_POST_FLOW]->SetLine(CLineCtrl::Instance(LINE_TLS));

				CString sConnectionOption = m_tcp.SSLOption;
				sConnectionOption += L",TERMINATE:";
				sConnectionOption += L"Envelope>";	
				NHDEBUG(DBG_INFO, (_T("SEND IP INFORMATION[%S] [%s][%s][%s]\n"), pSendBuf, m_tcp.IpAddr1, m_tcp.port1, sConnectionOption));
				nreturn = m_FlowAndLine[HTTP_POST_FLOW]->SendData(pSendBuf, Length, m_tcp.IpAddr1, m_tcp.port1, sConnectionOption);		
			}
			// End of [#2446]
			// [#2448] US Justin Just.Cash
			else if( n3rdTranType == TRANHOST_JUSTCASH_TRANSACTION )
			{
				rty_cnt = max_rty_cnt;		// One time sending....
				// Parse Host Information....
				if( sHostInfo == NULL )
				{
					nreturn = 1;
					goto net_error;
				}
				CStringArray strTempArray;
				SplitString(sHostInfo, ",", strTempArray);			// IP, PORT, SSLOPTION, HOST CERT
				if(	strTempArray.GetSize() < 4 )
				{
					nreturn = 1;
					goto net_error;
				}				
				m_tcp.IpAddr1	= strTempArray.GetAt(0);
				m_tcp.port1		= strTempArray.GetAt(1);
				m_tcp.SSLOption.Format(L"%s:%s", strTempArray.GetAt(2), strTempArray.GetAt(3));
				m_FlowAndLine[HTTP_POST_FLOW]->SetLine(CLineCtrl::Instance(LINE_TLS));

				CString sConnectionOption = m_tcp.SSLOption;
				sConnectionOption += L",TERMINATE:";
				sConnectionOption += L"}}";	
				NHDEBUG(DBG_INFO, (_T("SEND IP INFORMATION[%S] [%s][%s][%s]\n"), pSendBuf, m_tcp.IpAddr1, m_tcp.port1, sConnectionOption));
				nreturn = m_FlowAndLine[HTTP_POST_FLOW]->SendData(pSendBuf, Length, m_tcp.IpAddr1, m_tcp.port1, sConnectionOption);		
			}
			// End of [#2448]
			// [#2513] US Justin 2017.11.02 GivePay Giftcard Purchase
			else if( n3rdTranType == TRANHOST_GIVEPAY_TRANSACTION )
			{
				rty_cnt = max_rty_cnt;		// One time sending....
				// Parse Host Information....
				if( sHostInfo == NULL )
				{
					nreturn = 1;
					goto net_error;
				}
				CStringArray strTempArray;
				SplitString(sHostInfo, ",", strTempArray);			// IP, PORT, SSLOPTION, HOST CERT
				if(	strTempArray.GetSize() < 4 )
				{
					nreturn = 1;
					goto net_error;
				}				
				m_tcp.IpAddr1	= strTempArray.GetAt(0);
				m_tcp.port1		= strTempArray.GetAt(1);
				m_tcp.SSLOption.Format(L"%s:%s", strTempArray.GetAt(2), strTempArray.GetAt(3));
				m_FlowAndLine[HTTP_POST_FLOW]->SetLine(CLineCtrl::Instance(LINE_TLS));

				CString sConnectionOption = m_tcp.SSLOption;
				if(strTempArray.GetSize() > 4 )
				{
					if(strTempArray.GetAt(4).GetLength()>1)
					{
						sConnectionOption += L",TERMINATE:";
						sConnectionOption += strTempArray.GetAt(4);	
					}
				}
				// [#2574] US Justin GivePay Enhancement3, Overwrite TimeOut
				if(strTempArray.GetSize() > 5 )
				{
					sConnectionOption += L",TIMEOUT_OVERWRITE:";
					sConnectionOption += strTempArray.GetAt(5);	
				}
				// End of [#2574]

				NHDEBUG(DBG_INFO, (_T("SEND IP INFORMATION[%S] [%s][%s][%s]\n"), pSendBuf, m_tcp.IpAddr1, m_tcp.port1, sConnectionOption));
				nreturn = m_FlowAndLine[HTTP_POST_FLOW]->SendData(pSendBuf, Length, m_tcp.IpAddr1, m_tcp.port1, sConnectionOption);		
			}
			// End of [#2513]
			// [#2497] NH woooZ 2017.09.20  ALPHI server
#if (APP_ALPHI)
			else if( n3rdTranType == TRANHOST_ALPHI )  
			{
				m_tcp.IpAddr1 = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ALPHI_SERVER_ADDRESS);
				m_tcp.port1.Format(L"%d", MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ALPHI_SERVER_PORT));

				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ALPHI_SSL_ENDISABLE) == ENABLE)
				{ // [#2497] NH woooZ 2018.01.19  ALPHI server
					int nCertEn = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ALPHI_SSL_CERT_ENDISABLE);
					if (nCertEn == ENABLE)
						m_tcp.SSLOption.Format(L"%d", CONN_UPTO_TLS_V12_USE_CERT_ALPHI);
					else
						m_tcp.SSLOption.Format(L"%d", CONN_UPTO_TLS_V12_ALPHI);
				} // end of [#2497]
				else
					m_tcp.SSLOption.Format(L"%d", CONN_UPTO_SSL_NONE);

				WriteAlphiLogToFile(L"SendData TRANHOST_ALPHI SERVER:"+m_tcp.IpAddr1+",PORT:"+m_tcp.port1+",SSL:"+m_tcp.SSLOption);
				NHDEBUG(DBG_INFO, (_T("SEND IP INFORMATION[%S] [%s][%s][%s]\n"), pSendBuf, m_tcp.IpAddr1, m_tcp.port1, m_tcp.SSLOption));
				
				nreturn = m_FlowAndLine[STANDARD_FLOW]->SendData(pSendBuf, Length, m_tcp.IpAddr1, m_tcp.port1, m_tcp.SSLOption);
			}
#endif
			// End of [#2497]

			if (!nreturn)
			{
				NHDEBUG(DBG_INFO, (_T("SEND SUCCESS[%x]\n"), nreturn));
				// send ok
				return 0;
			}
			else
			{
				NHDEBUG(DBG_INFO, (_T("SEND ERROR RETURN[%x][%x]\n"), nreturn, rty_cnt));
				NHDEBUG(DBG_AGING, (_T("SEND ERROR RETURN[%x][%x]\n"), nreturn, rty_cnt));

				// send error (control error or dial fail)
				if ((nreturn >= 6) && (nreturn <= 10))
				{
					NHDEBUG(DBG_INFO, (_T("SEND CONTROL ERROR[%x][%x]\n"), nreturn, rty_cnt));
					NHDEBUG(DBG_AGING, (_T("SEND CONTROL ERROR[%x][%x]\n"), nreturn, rty_cnt));
					goto net_error;
				}
			}
		}

		rty_cnt++;

//		if (rty_cnt > 3)
		if (rty_cnt > max_rty_cnt)	// [#2068] NZ KSK 2011.06.09
		{
			// error stack
net_error:
			NHDEBUG(DBG_INFO, (_T("SEND RETRY ERROR[%x][%x]\n"), nreturn, rty_cnt));

			// [#525] US KSK 2009.05.25 ErroString은 Text에서 Read하므로 필요없는 Logic 삭제
			switch(nreturn)
			{
			case  1:	strErrorCode = L"D1500";	break;		// L"Modem Dial Connection Timeout"	L"TCPIP Connection Timeout"
			case  2:	strErrorCode = L"D1800";	break;		// L"No Dial Tone";
			case  3:	strErrorCode = L"D1900";	break;		// L"No Answer";
			case  4:	strErrorCode = L"D2000";	break;		// L"Line Busy";
			case  5:	strErrorCode = L"D1701";	break;		// L"No Carrier(No ENQ Received)";
			case  6:	strErrorCode = L"D1704";	break;		// L"No ACK/NAK Received";
			case  7:	strErrorCode = L"D1706";	break;		// L"No Message Data Received";
			case  8:	strErrorCode = L"D2200";	break;		// L"No EOT received from HOST";
			case 10:	strErrorCode = L"D1707";	break;		// L"NAK retry exceed";
			case 11:	strErrorCode = L"D1708";	break;		// L"Send retry exceed";
			case 12:	strErrorCode = L"D1709";	break;		// L"Modem Initialize fail";
			case 13:	strErrorCode = L"D1710";	break;		// L"Modem open fail";
			case 14:	strErrorCode = L"D0300";	break;		// L"Modem is not responding";
			case 15:	strErrorCode = L"D0301";	break;		// L"Modem is not responding";
			case 16:	strErrorCode = L"D0302";	break;		// L"Modem is not responding";
			case 17:	strErrorCode = L"D0303";	break;		// L"Modem is not responding";
			case 18:	strErrorCode = L"D0304";	break;		// L"Modem is not responding";
			case 19:	strErrorCode = L"D0305";	break;		// L"Modem is not responding";
			case 20:	strErrorCode = L"D0306";	break;		// L"Modem is not responding";
			case 21:	strErrorCode = L"D0307";	break;		// L"Modem is not responding";
			case 22:	strErrorCode = L"D1501";	break;		// L"SSL Connection failure"; //[#2093] NH PCS 2011.10.26
			default:	strErrorCode = L"D1500";	break;		// L"Modem Dial Connection Timeout"	L"TCPIP Connection Timeout"
			}
			
			NHDEBUG(DBG_INFO, (L"NET ERROR(3) - (%s)\n", strErrorCode));
			NHDEBUG(DBG_AGING, (L"NET ERROR(3) - (%s)\n", strErrorCode));
			return 3;
		}
		
		// 3초 대기후 retry하도록 한다.
		Delay_Msg(3000);
	}

	strErrorCode = L"D1500";
	
	NHDEBUG(DBG_INFO, (L"NET ERROR(1) - (%s)\n", strErrorCode));
	NHDEBUG(DBG_AGING, (L"NET ERROR(1) - (%s)\n", strErrorCode));

	// send timeout
	return 1;
}
// [#2054] NH KJW 2011.05.03
/*
// [#2012] NH KJW 2011.01.26 Lottery URL, SSL 지원
//int	CNetWork::RecvData(BYTE *pRecvBuf, int *Length, int RecvSec)
int	CNetWork::RecvData(BYTE *pRecvBuf, int *Length, int RecvSec, int n3rdTranType)
// end of [#2012]
*/
int	CNetWork::RecvData(LPBYTE pRecvBuf, int *Length, int RecvSec, int n3rdTranType)
// end of [#2054]
{
	if (m_bRMSConnectOpen)
		RMSConnectClose();


	int nCurrentFlowLine = 0;

	switch(n3rdTranType)
	{
		case TRANHOST_DUALHOST:
			nCurrentFlowLine = VISAII_FLOW;
			break;
		case TRANHOST_PIN4_TRANSACTION:
		case TRANHOST_PIN4_REVERSAL:
		case TRANHOST_PIN4_GETTOKEN:							// [#2515] US Justin
		case TRANHOST_JUSTCASH_TRANSACTION:
		case TRANHOST_PAYPAL_TRANSACTION:
		case TRANHOST_GIVEPAY_TRANSACTION:						// [#2513] US Justin 2017.11.02 GivePay Giftcard Purchase
			nCurrentFlowLine = HTTP_POST_FLOW;
			break;

		default:
			nCurrentFlowLine = m_nCurrentFlowLine;
			break;
	}
	// end of [#2476]

	if (nCurrentFlowLine >= 0)
	{
		if (nCurrentFlowLine <= MAX_FLOWLINE)
		{
			if (!m_FlowAndLine[nCurrentFlowLine]->RecvData(pRecvBuf, Length, 120))
			{
				// receive ok
				return 0;
			}
			return 3; // m_RecvLen == 0 in CFlowCtrl::RecvData()
		}
		// set m_nCurrentFlowLine max error
		return 2;
	}
	// set m_nCurrentFlowLine min error
	return 1;
}

// [#525] US KSK 2009.05.25
int CNetWork::PreDialStart()
{
	int		nReturn	= FALSE;
	CString	CurrentPhone;
	
	CurrentPhone = MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_PHONENODEFINE1);
		
	if(CurrentPhone.GetLength())
		nReturn = ((CVISAIICtrl *)m_FlowAndLine[m_nCurrentFlowLine])->PreDialStart(CurrentPhone);
	
	return nReturn;
}

int CNetWork::PreDialCancelByUser()
{
	int		nReturn	= 0;
	
	nReturn = ((CVISAIICtrl *)m_FlowAndLine[m_nCurrentFlowLine])->PreDialCancelByUser();
	
	return nReturn;
}
// end of [#525]

int CNetWork::CloseLine()
{
	return (m_FlowAndLine[m_nCurrentFlowLine]->CloseLine());
}

void CNetWork::SetErrorCode(CString strErr)
{
	strErrorCode = strErr;
}

void CNetWork::SetErrorMsg(CString strMsg)
{
	strErrorMsg = strMsg;
}

void CNetWork::ClearErrorCode()
{
	strErrorCode = _T("");
	strErrorMsg = _T("");
}

CString	CNetWork::GetErrorCode()
{
	return strErrorCode;
}

CString CNetWork::GetErrorMsg()
{
	return strErrorMsg;
}

// [#15] NH KGS 2008.3.12 MODEM TEST
int CNetWork::TestModem(LPCTSTR szTestNum, int nTestOption)
{
	CLineCtrl *dialLine = CLineCtrl::Instance(LINE_DIALUP);
	CVISAIICtrl dialConnection(dialLine);

// [#2016] NH KSK 2011.02.09
	if (nTestOption == MODEM_HOST_DEV_TEST)
		return dialConnection.SendData((BYTE*)"DEVICETEST", 10, szTestNum, L"", Int2Asc(nTestOption));

	// 상위국과 INTERFACE를 맞추기 위해 성공 TRUE / 실패 FALSE
	// DATA는 의미가 없음(LINE 상태만 CHECK함)
	if (nTestOption == MODEM_HOST_LINE_TEST)
	{
		if (dialConnection.SendData((BYTE*)"TEST", 4, szTestNum, L"", Int2Asc(nTestOption)) == 0)
			return TRUE;
	}
// end of [#2016]

	return FALSE;
}
// end of [#15]

// [#GLDV-2683] NH Kook 2019.11.19 Modem Removal Detection
int CNetWork::TestModem2()
{
	CComm		m_ModemComm;
	HANDLE		m_hModemComPort = INVALID_HANDLE_VALUE;

	char ATSendRecvBuff[1024] = { 0, };
	char GetData = 0;
	int nSendSize = 0;
	int	nRecvSize = 0;
	CString strTemp;

	// Port Open
	if (m_hModemComPort == INVALID_HANDLE_VALUE)
		m_hModemComPort = CreateFile(_T("COM5:"), GENERIC_READ|GENERIC_WRITE,NULL,NULL,OPEN_EXISTING,NULL,NULL);

	if (m_hModemComPort == NULL || m_hModemComPort == INVALID_HANDLE_VALUE)
	{
		NHDEBUG(1, (_T("PORT OPEN FAILED\n")));
		return FALSE;
	}

	if (m_ModemComm.OpenPort(m_hModemComPort) == FALSE)
	{
		NHDEBUG(1, (_T("PORT OPEN FAILED 2\n")));
		CloseHandle(m_hModemComPort);
		m_hModemComPort = NULL;
		return FALSE;
	}

	//NHDEBUG(1, (_T("PORT OPEN SUCCESS\n")));


	// Check response for AT command and return its result. ('AT' -> 'ATOK'?)
	strcpy(ATSendRecvBuff, "AT");
	nSendSize = strlen(ATSendRecvBuff);
	m_ModemComm.WriteData((BYTE*)ATSendRecvBuff, nSendSize);
	m_ModemComm.WriteData((BYTE*)"\r\n", 2);
	NHDEBUG(1, (_T("SEND [%S]\n"), ATSendRecvBuff));

	memset(ATSendRecvBuff, 0, sizeof(ATSendRecvBuff));

	Delay_Msg(200);

	nRecvSize = get_size();
	for (int i=0; i<nRecvSize; i++)
	{
		get_queue(&GetData);

		if (GetData == 0x0d || GetData == 0x0a || GetData == 0x09)
		{
			i--;
			nRecvSize--;
			continue;
		}
		ATSendRecvBuff[i] = GetData;
	}

	NHDEBUG(1, (_T("RECEIVED [%S]\n"), ATSendRecvBuff));

	strTemp.Format(L"%S", ATSendRecvBuff);
	if (strTemp.Find(_T("OK")) != -1)		// expects "ATOK"
		return TRUE;

	return FALSE;
}
// end of [#GLDV-2683]

// [#585] NH KSK 2009.12.03
int CNetWork::TestNetwork(LPCTSTR szTestAddress, LPCTSTR szPort, LPCTSTR szSSLOption)
{
	CLineCtrl *tcpLine = CLineCtrl::Instance(LINE_TCP);
	CVISAIICtrl NetworkConnection(tcpLine);
	
// [#2016] NH KSK 2011.02.09
	// 상위국과 INTERFACE를 맞추기 위해 성공 TRUE / 실패 FALSE
	// DATA는 의미가 없음(LINE 상태만 CHECK함)
	if (NetworkConnection.SendData((BYTE*)"TEST", 4, szTestAddress, szPort, szSSLOption) == 0)
		return TRUE;
// end of [#2016]
	
	return FALSE;
}
// end of [#585]

// [#2358] NH Kook 2015.07.14 support 'PING' function.
// return : -1	- error in address 
//			 0	- ping FALSE
//			 1	- ping TRUE 
INT CNetWork::TestPing(LPCTSTR szTestAddress, int *pOutRTT)
{
	NVDump('O', 'A', "92", L"", L"TestPing");
	NVDump('O', 'A', "92", L"DST", szTestAddress);

	int iTtl = 128;
	int iWaitTime = 1000;

	IPAddr                ipAddress ;
	IP_OPTION_INFORMATION ipInfo ;
	ICMP_ECHO_REPLY       icmpEcho;	
	HANDLE                hFile;	
	char                  strHost  [ MAX_PATH ] ;
	TCHAR                 szPCName [ MAX_PATH ]  = { TEXT( "" ) } ;
	TCHAR                 szName [ MAX_PATH ]  = { TEXT( "" ) } ;
	int                   iRet = -1 ;
	struct in_addr iaDest;		// Internet address structure
	LPHOSTENT pHost;			// Pointer to host entry structure

	_tcscpy( szName , TEXT("") );
	WideCharToMultiByte( CP_ACP, 0, szTestAddress , -1, strHost, sizeof( strHost ), NULL, FALSE );
	ipAddress = inet_addr(strHost);
	iaDest.s_addr = inet_addr(strHost);
	if (iaDest.s_addr == INADDR_NONE)
	{
		pHost = gethostbyname(strHost);
		if( pHost )
		{
			char *pIP ;
			iaDest.S_un.S_addr = *(DWORD *)(*pHost->h_addr_list) ;
			pIP = inet_ntoa( iaDest ) ;
			if( strlen( pIP ) )
				MultiByteToWideChar( CP_ACP, 0, pIP , -1, szPCName, sizeof( szPCName ) );
			if( _tcslen( szPCName ) )
				_tcscpy( szName , szPCName );
		}
	}
	else
	{
		pHost = gethostbyaddr((const char *)&iaDest, 
			sizeof(struct in_addr), AF_INET);
		if( pHost )
		{
			MultiByteToWideChar( CP_ACP, 0, pHost->h_name , -1, szPCName, sizeof( szPCName ) );
			if( _tcslen( szPCName ) )
				_tcscpy( szName , szPCName );
		}
	}

	if( _tcslen( szPCName ) )
		NVDump('O', 'A', "92", L"DSTNAME", szName);

	if( pHost )
		ipAddress = *(DWORD *)(*pHost->h_addr_list) ;

	if (ipAddress != INADDR_NONE)
	{
		DWORD dwRet;

		iRet = 0 ;
		hFile = IcmpCreateFile();

		// Set some reasonable default values
		ipInfo.Ttl   = iTtl ;
		ipInfo.Tos   = 0;
		ipInfo.Flags = 0;
		ipInfo.OptionsSize = 0 ;
		ipInfo.OptionsData = NULL ;
		icmpEcho.Status    = IP_SUCCESS ;
		
		dwRet = IcmpSendEcho( hFile , ipAddress , NULL , 0 ,
			(PIP_OPTION_INFORMATION)&ipInfo,
			&icmpEcho , sizeof(ICMP_ECHO_REPLY) , iWaitTime );
		IcmpCloseHandle(hFile) ;

		if (dwRet != 0)
		{
			if ( icmpEcho.Status == IP_SUCCESS )
				iRet = 1 ;
		}

		*pOutRTT = icmpEcho.RoundTripTime ;
	}
	return iRet;

}
// end of [#2358]

// RMS FUNCTION
BOOL CNetWork::RMSConnectCheck()
{
	if (!m_bRMSConnectOpen)
		return FALSE;

	return m_FlowAndLine[NHRMS_FLOW]->RMSConnectCheck();
}

int CNetWork::RMSConnectAccept()
{
	NHDEBUG(DBG_INFO, (_T("CNetWork::RMSConnectAccept()\n")));

	if (!m_bRMSConnectOpen)
		return FALSE;

	return m_FlowAndLine[NHRMS_FLOW]->RMSConnectAccept();
}

int CNetWork::RMSConnectOpen(int OpenType)
{
	if (!m_bRMSConnectOpen)
	{
		if (m_nCurrentRMSClientLine == LINE_DIALUP)
		{
			if (OpenType == RMS_LISTEN)
			{
				if (m_FlowAndLine[NHRMS_FLOW]->RMSConnectOpen(L"", L"", L"1", L"LISTEN") == RMS_OK)
				{
					m_bRMSConnectOpen = TRUE;
					return RMS_OK;
				}
			}
			else if (OpenType == RMS_CONNECT)
			{
				CString	strPhoneNumber;

				for (int i = 0; i < 2; i++)
				{
					strPhoneNumber = MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSPHONENO1+i);

					// [#496] [NH] KSK 2009.2.9 RMS Phone2가 NULL일 경우 Phone1으로 하도록 수정
					if (strPhoneNumber.GetLength() <= 0)
					{
						strPhoneNumber = MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSPHONENO1);
					}
					// end of [#496]

					if (m_FlowAndLine[NHRMS_FLOW]->RMSConnectOpen(strPhoneNumber, L"", L"1", L"CONNECT") == RMS_OK)
					{
						m_bRMSConnectOpen = TRUE;
						return RMS_OK;
					}
				}
			}
		}
		else if (m_nCurrentRMSClientLine == LINE_TCP || m_nCurrentRMSClientLine == LINE_TLS)
		{
			if (OpenType == RMS_LISTEN)
			{
				CString strListenPort;
				strListenPort.Format(L"%d", MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_RMSSVRPORTNO));

				if (m_FlowAndLine[NHRMS_FLOW]->RMSConnectOpen(L"127.0.0.1", strListenPort, L"0", L"LISTEN") == RMS_OK)
				{
					m_bRMSConnectOpen = TRUE;
					return RMS_OK;
				}
			}
			else if (OpenType == RMS_CONNECT)
			{
				CString strIpAddr, strPort, strOptions;

				strIpAddr = MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_RMSNAME);

				// [#RWC6-67] US William 2019.10.25 MoniView TLS
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RMSTLSENABLE) == ENABLE)
				{
					strPort.Format(L"%d", MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RMSTLSPORT));
					strOptions = L"83:1";
				}
				else
				{
					strPort.Format(L"%d", MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_RMSPORTNO));
					strOptions = L"0";
				}

				if (strIpAddr.GetLength() > 0 && strPort.GetLength() > 0)
				{
					// WW: Option 83 is CONN_UPTO_TLS_V12_USE_CERT
					if (m_FlowAndLine[NHRMS_FLOW]->RMSConnectOpen(strIpAddr, strPort, strOptions, L"CONNECT") == RMS_OK)
					{
						m_bRMSConnectOpen = TRUE;
						return RMS_OK;
					}
				}
			}
		}

		return RMS_ERR;
	}

	return RMS_OK;
}

int CNetWork::RMSConnectClose()
{
	if (m_bRMSConnectOpen)
	{
		NHDEBUG(DBG_INFO, (_T("CNetWork::RMSConnectClose()\n")));
		m_FlowAndLine[NHRMS_FLOW]->RMSConnectClose();
		m_bRMSConnectOpen = FALSE;
	}

	return RMS_OK;
}

int	CNetWork::RMSSendData(BYTE *pSendBuf, int Length, int SendSec)
{
	NHDEBUG(DBG_INFO, (_T("CNetWork::RMSSendData()\n")));

	if (!m_bRMSConnectOpen)
		return RMS_ERR;

	return m_FlowAndLine[NHRMS_FLOW]->SendData(pSendBuf, Length, _T(""), _T(""), _T(""));
}

int CNetWork::RMSRecvData(BYTE *pRecvBuf, int *Length, int SendSec)
{
	NHDEBUG(DBG_INFO, (_T("CNetWork::RMSRecvData()\n")));

	if (!m_bRMSConnectOpen)
		return RMS_ERR;

	return m_FlowAndLine[NHRMS_FLOW]->RecvData(pRecvBuf, Length, SendSec);
}

// [#2075] NH KSK 2011.06.27
BOOL CNetWork::RKTConnectCheck()
{
	return m_FlowAndLine[m_nCurrentFlowLine]->RKTConnectCheck();
}

int CNetWork::RKTConnectOpen()
{
	if (RKTConnectCheck())	// 이전에 open이 되어져 있다면 close로 open을 시도한다.
		RKTConnectClose();

	if (m_nCurrentLine == LINE_DIALUP)
	{
		CString	strPhoneNumber;

		for (int i = 0; i < 2; i++)
		{
			strPhoneNumber = MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_PHONENODEFINE1+i);
			if (strPhoneNumber.GetLength() <= 0)
				strPhoneNumber = MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_PHONENODEFINE2);
			if (m_FlowAndLine[m_nCurrentFlowLine]->RKTConnectOpen(strPhoneNumber, L"", L"0") == 0)
				return 0;
		}
	}
	else if (m_nCurrentLine == LINE_TCP || m_nCurrentLine == LINE_TLS)
	{
		CString strIpAddr, strPort;

		strIpAddr = MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_HOST1NAME);
		strPort.Format(L"%d", MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_HOST1PORTNO));
		if (strIpAddr.GetLength() > 0 && strPort.GetLength() > 0)
		{
			if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_SSLENABLE))
				m_tcp.SSLOption.Format(L"%d", MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SSLVERSION));
			else
				m_tcp.SSLOption = L"0";

			if (m_FlowAndLine[m_nCurrentFlowLine]->RKTConnectOpen(strIpAddr, strPort, m_tcp.SSLOption) == 0)
				return 0;
		}
	}

	return 1;
}

int CNetWork::RKTConnectClose()
{
	return m_FlowAndLine[m_nCurrentFlowLine]->RKTConnectClose();
}

int	CNetWork::RKTSendData(BYTE *pSendBuf, int Length, int SendSec, LPCTSTR Option)
{
	NHDEBUG(1, (_T("CNetWork::RKTSendData()\n")));

	if (!RKTConnectCheck())
	{
		RKTConnectClose();
		return 1;
	}

	return m_FlowAndLine[m_nCurrentFlowLine]->SendData(pSendBuf, Length, _T(""), _T(""), Option);
}

int CNetWork::RKTRecvData(BYTE *pRecvBuf, int *Length, int RecvSec)
{
	NHDEBUG(1, (_T("CNetWork::RKTRecvData()\n")));

	return m_FlowAndLine[m_nCurrentFlowLine]->RecvData(pRecvBuf, Length, RecvSec);
}
// end of [#2075]

// [#2326] US Kook 2015.03.10 Support MoniAir
void CNetWork::RenewIPAddress()			// 2015.10.16 it is no use now but remain for later.
{
	// to fix MoniAir connection failure, try to renew LAN's IP.
	// from : https://msdn.microsoft.com/en-us/library/aa366057(v=VS.85).aspx
	NVDump('O', 'A', "92", L"MoniAir", L"# RENEW IP");

	/* Note: could also use malloc() and free() */
#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

	// Before calling IpReleaseAddress and IpRenewAddress we use
	// GetInterfaceInfo to retrieve a handle to the adapter

	PIP_INTERFACE_INFO pInfo;
	pInfo = (IP_INTERFACE_INFO *) MALLOC( sizeof(IP_INTERFACE_INFO) );
	ULONG ulOutBufLen = 0;
	DWORD dwRetVal = 0;

	// Make an initial call to GetInterfaceInfo to get
	// the necessary size into the ulOutBufLen variable
	if ( GetInterfaceInfo(pInfo, &ulOutBufLen) == ERROR_INSUFFICIENT_BUFFER) {
		FREE(pInfo);
		pInfo = (IP_INTERFACE_INFO *) MALLOC (ulOutBufLen);
	}

	// Make a second call to GetInterfaceInfo to get the
	// actual data we want
	if ((dwRetVal = GetInterfaceInfo(pInfo, &ulOutBufLen)) == NO_ERROR ) {
// 		printf("\tAdapter Name: %ws\n", pInfo->Adapter[0].Name);
// 		printf("\tAdapter Index: %ld\n", pInfo->Adapter[0].Index);
// 		printf("\tNum Adapters: %ld\n", pInfo->NumAdapters);
		NVDump('O', 'A', "92", L"MoniAir", pInfo->Adapter[0].Name);
	}
	else if (dwRetVal == ERROR_NO_DATA) {
// 		printf("There are no network adapters with IPv4 enabled on the local system\n");
		NVDump('O', 'A', "92", L"MoniAir", L"No NetAdapts");

		FREE(pInfo);
		pInfo = NULL;
		return;
	}
	else {
// 		printf("GetInterfaceInfo failed.\n");
		NVDump('O', 'A', "92", L"MoniAir", L"GetIf NG");

		LPVOID lpMsgBuf;

		if (FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dwRetVal,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,
			0,
			NULL )) {
				printf("\tError: %s", lpMsgBuf);
		}
		LocalFree( lpMsgBuf );
		return;
	}

	// Call IpReleaseAddress and IpRenewAddress to release and renew
	// the IP address on the first network adapter returned 
	// by the call to GetInterfaceInfo.
	if ((dwRetVal = IpReleaseAddress(&pInfo->Adapter[0])) == NO_ERROR) {
// 		printf("IP release succeeded.\n");
		NVDump('O', 'A', "92", L"MoniAir", L"IP release OK");

	}
	else {
// 		printf("IP release failed.\n");
		NVDump('O', 'A', "92", L"MoniAir", L"IP release NG");
	}

	if ((dwRetVal = IpRenewAddress(&pInfo->Adapter[0])) == NO_ERROR) {
// 		printf("IP renew succeeded.\n");
		NVDump('O', 'A', "92", L"MoniAir", L"IP renew OK");

	}
	else {
// 		printf("IP renew failed.\n");
		NVDump('O', 'A', "92", L"MoniAir", L"IP renew NG");
	}

	/* Free allocated memory no longer needed */
	if (pInfo) {
		FREE(pInfo);
		pInfo = NULL;
	}

}


CString	CNetWork::GetMACAddress() // [#RWC6-54] US William 2019.09.13 Add MAC Address to PAS
{
	// Get MAC Address
	CString strMac;
	unsigned long ulOutBufLen = sizeof(IP_ADAPTER_INFO);
	IP_ADAPTER_INFO *pAdapterInfos = (IP_ADAPTER_INFO *) malloc(sizeof(IP_ADAPTER_INFO));
	DWORD result = GetAdaptersInfo(pAdapterInfos, &ulOutBufLen);
	if (result != ERROR_SUCCESS) 
	{
		delete pAdapterInfos;
		pAdapterInfos = (IP_ADAPTER_INFO *) malloc(ulOutBufLen);
	}

	result = GetAdaptersInfo(pAdapterInfos, &ulOutBufLen);
	if (result == ERROR_SUCCESS) 
	{
		PIP_ADAPTER_INFO pAdapter = pAdapterInfos;

		for (unsigned int i = 0; i < pAdapter->AddressLength; i++)
		{
			if (i == pAdapter->AddressLength - 1)
			{
				strMac.Format(L"%s%.2x", strMac, (int)pAdapter->Address[i]);
			}
			else 
			{
				strMac.Format(L"%s%.2x:", strMac, (int)pAdapter->Address[i]);
			}
		}
	}

	delete pAdapterInfos;

	return strMac;
}

/**
* Combines all the known CA files into one for use by the networking functions
*/
bool CNetWork::InstallCAFiles()
{
	bool customerCertsExist = !!IsExistFile(CA_ROOT_CERTS_FILE_CUSTOMER);
	bool hyosungCertsExist = !!IsExistFile(CA_ROOT_CERTS_FILE_HYOSUNG);
	bool updaterCertsExist = !!IsExistFile(CA_ROOT_CERTS_FILE_UPDATER);

	DeleteFile(_T(CA_ROOT_CERTS_FILE));
	std::ofstream caFiles(CA_ROOT_CERTS_FILE, std::ios_base::binary );

	if (customerCertsExist)
	{
		NHDBG((L"Adding customer certs...\r\n"));

		std::ifstream customerCerts(CA_ROOT_CERTS_FILE_CUSTOMER, std::ios_base::binary);

		caFiles << customerCerts.rdbuf() << "\r\n";
	}

	// Use AP certs, then fall back to Updater.zip certs
	if (hyosungCertsExist)
	{
		NHDBG((L"Adding Hyosung certs...\r\n"));

		std::ifstream hyosungCerts(CA_ROOT_CERTS_FILE_HYOSUNG, std::ios_base::binary);

		caFiles << hyosungCerts.rdbuf();
	}
	else if (updaterCertsExist)
	{
		NHDBG((L"Adding Updater certs...\r\n"));

		std::ifstream customerCerts(CA_ROOT_CERTS_FILE_UPDATER, std::ios_base::binary);

		caFiles << customerCerts.rdbuf();
	}

	return true;
}
