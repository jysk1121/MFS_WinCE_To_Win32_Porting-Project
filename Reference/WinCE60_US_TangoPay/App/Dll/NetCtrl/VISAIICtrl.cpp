// VISAIICtrl.cpp: implementation of the CVISAIICtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#define NH_DEBUG
#include ".\Common\NHDbgApi.h"
#include ".\Dev\DevDefine.h"
#include "VISAIICtrl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define		CONN_TIMEOUT			60

#define		CONN_TIMEOUT_TCPIP		20		// [#2068] NZ KSK 2011.06.09 Timeout시간 단축을 위해 추가

#define		ENQ_TIMEOUT				35

#define		ACK_TIMEOUT				30
#define		DATA_TIMEOUT			60			// 2008-01-08 V01.02.24 SRC-17

// [#105] KSK 2008.04.14
#define		EOT_TIMEOUT_REQUIRED	60
#define		EOT_TIMEOUT_OPTIONAL	10
// end of [#105]

#define		DATA_STX		0x02
#define		DATA_ETX		0x03
#define		DATA_EOT		0x04
#define		DATA_ENQ		0x05
#define		DATA_ACK		0x06
#define		DATA_NAK		0x15

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVISAIICtrl::CVISAIICtrl(CLineCtrl *line) : CFlowCtrl(line)
{
	// [#2012] NH KJW 2011.02.09
	//memset(m_RecvData, 0, sizeof(m_RecvData));
	//memset(m_SendData, 0, sizeof(m_SendData));
	// end of [#2012]

	m_bPreDialStart = FALSE;		// [#525] US KSK 2009.05.25
}

CVISAIICtrl::~CVISAIICtrl()
{

}

int CVISAIICtrl::SendData(BYTE *pSendBuf, int Length, LPCTSTR pDestInfo, LPCTSTR port, LPCTSTR option)
{
	// [#525] US KSK 2009.05.25
	// Pre Dialing 지원을 위해 Logic 변경
	// option : 0 정상 거래
	// option : 1 SSL Version 23 test
	// option : 2 SSL Version 3 test
	// option : 3 modem host device test
	// option : 4 modem rms device test (현재 사용 안함)
	// option : 5 modem host call test
	// option : 6 modem rms call test (현재 사용 안함)
	// option : 7 tcp/ip host call test
	BYTE	bcc = 0;
	int		nreturn = 0;

	// [#2016] NH 2011.02.09
	m_bNetworkTest = FALSE;	// 변수 초기화

	// MODEM DEVICE TEST시 (TCP/IP는 Device test가 없음)
	if (option == Int2Asc(MODEM_HOST_DEV_TEST) || option == Int2Asc(MODEM_RMS_DEV_TEST))
	{
		nreturn = m_pNetLine->LineOpen(pDestInfo, port, option);
		m_pNetLine->LineClose();

		// Modem Device Test시에는 Set을 하지는 않지만 보완 Code
		m_bPreDialStart = FALSE;
		
		return nreturn;
	}
	// MODEM DEVICE TEST OK

	// [#2310] NH KSK 2014.11.18 TLS Support
//	if (option == Int2Asc(MODEM_HOST_LINE_TEST) || option == Int2Asc(MODEM_RMS_LINE_TEST) || 
//		option == Int2Asc(TCPIP_HOST_LINE_TEST) || option == Int2Asc(TCPIP_HOST_SSL_VER23_LINE_TEST) || option == Int2Asc(TCPIP_HOST_SSL_VER3_LINE_TEST) ||
//		option == Int2Asc(SSL_VER_23_USE_CERTIFY_LINE_TEST) || option == Int2Asc(SSL_VER_3_USE_CERTIFY_LINE_TEST)) // [#2093] NH PCS 2011.10.31 "SSL CERT."
//		m_bNetworkTest = TRUE;
	// [#2320] NH Justin 2015.01.14 Support NON-"Version Negotiation" Processor
	//if (option == Int2Asc(MODEM_HOST_LINE_TEST) || option == Int2Asc(MODEM_RMS_LINE_TEST) || 
	//	option == Int2Asc(TCPIP_HOST_LINE_TEST) || option == Int2Asc(SSL_VER3_AND_ABOVE_LINE_TEST) || option == Int2Asc(TLS_VER10_AND_ABOVE_LINE_TEST) ||
	//	option == Int2Asc(TLS_VER11_AND_ABOVE_LINE_TEST) || option == Int2Asc(TLS_VER12_LINE_TEST) || option == Int2Asc(SSL_VER3_AND_ABOVE_USE_CERTIFY_LINE_TEST) ||
	//	option == Int2Asc(TLS_VER10_AND_ABOVE_USE_CERTIFY_LINE_TEST) || option == Int2Asc(TLS_VER11_AND_ABOVE_USE_CERTIFY_LINE_TEST) || option == Int2Asc(TLS_VER12_USE_CERTIFY_LINE_TEST))
	//	m_bNetworkTest = TRUE;
	if (option == Int2Asc(MODEM_HOST_LINE_TEST)	|| 
		option == Int2Asc(MODEM_RMS_LINE_TEST) || 
		option == Int2Asc(TCPIP_HOST_LINE_TEST) || 
		option == Int2Asc(CONN_UPTO_SSL_V30_LINE_TEST) || 
		option == Int2Asc(CONN_UPTO_TLS_V10_LINE_TEST) ||
		option == Int2Asc(CONN_UPTO_TLS_V11_LINE_TEST) ||
		option == Int2Asc(CONN_UPTO_TLS_V12_LINE_TEST) ||
		option == Int2Asc(CONN_UPTO_TLS_V13_LINE_TEST) ||
		option == Int2Asc(CONN_UPTO_SSL_V30_USE_CERT_LINE_TEST) ||
		option == Int2Asc(CONN_UPTO_TLS_V10_USE_CERT_LINE_TEST) ||
		option == Int2Asc(CONN_UPTO_TLS_V11_USE_CERT_LINE_TEST) ||
		option == Int2Asc(CONN_UPTO_TLS_V12_USE_CERT_LINE_TEST) ||
		option == Int2Asc(CONN_UPTO_TLS_V13_USE_CERT_LINE_TEST))
		m_bNetworkTest = TRUE;
	// End of [#2320]
	// end of [#2310]
	// end of [#2016]

	if (m_bPreDialStart != TRUE)
		ClearQueue();				// queue clear

	// receive data buffer clear
	// [#2012] NH KJW 2011.02.09
	//memset(m_RecvData, 0, sizeof(m_RecvData));
	memset(m_RecvData, 0, NETBUF_RECV_SIZE);
	// end of [#2012]
	m_RecvLen = 0;

	// send data buffer clear
	// [#2012] NH KJW 2011.02.09
	//memset(m_SendData, 0, sizeof(m_SendData));
	memset(m_SendData, 0, NETBUF_SEND_SIZE);
	// end of [#2012]
	m_SendLen = 0;

	// make send data and save send buffer
	m_SendData[0] = 0x02;
	for(int i=0; i<Length; i++)
	{
		m_SendData[i+1] = pSendBuf[i];
		bcc ^= pSendBuf[i];
	}
	m_SendData[Length+1] = 0x03;
	bcc ^= m_SendData[Length+1];
	m_SendData[Length+2] = bcc;
	m_SendLen = Length+3;
	// make complete

	// [#2075] NH KSK 2011.06.27
	if (option == Int2Asc(RKT_MODE_R0) || option == Int2Asc(RKT_MODE_R1) || option == Int2Asc(RKT_MODE_R2))
	{
		// Flow Matrix Start
		SetCheckTimer(CONN_TIMEOUT); // [#2042] NH KJW 2011.04.04 CONN_TIMEOUT 설정을 소켓 연결 후 -> 소켓 연결 전에 설정하도록 수정함.
		nreturn = Flow_Matrix(RKT_SEND_STS, Asc2Int(option));
		return nreturn;
	}
	// end of [#2075]

	if (m_bPreDialStart != TRUE)
	{
		NHDEBUG(1, (_T("VISAII FLOW MATRIX START\n")));

		// [#2068] NZ KSK 2011.06.09
#if (AU_VERSION)
		if (MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == 1)
			SetCheckTimer(CONN_TIMEOUT);
		else
			SetCheckTimer(CONN_TIMEOUT_TCPIP);
#else
		SetCheckTimer(CONN_TIMEOUT);
#endif
		// end of [#2068]

		nreturn = m_pNetLine->LineOpen(pDestInfo, port, option);
	}

	if (!nreturn)
	{
		// success
		// timer set(connected)
NHDEBUG(1, (_T("VISAII FLOW MATRIX START\n")));
		//SetCheckTimer(CONN_TIMEOUT); // [#2042] NH KJW 2011.04.04 CONN_TIMEOUT 설정을 소켓 연결 후 -> 소켓 연결 전에 설정하도록 수정함.
		// Flow Matrix Start
		nreturn = Flow_Matrix();
	}

	m_bPreDialStart = FALSE;
	m_pNetLine->LineClose();
	return nreturn;
	
}

BOOL CVISAIICtrl::CloseLine()
{
	m_pNetLine->LineClose();
	return TRUE;
}

int CVISAIICtrl::Flow_Matrix(int initStatus, int nMode)
{
//	int		status		= INIT_STS;
	int		status		= initStatus;	// [#2075] NH KSK 2011.06.27
	int		snd_rty		= 0;
	int		nak_rty		= 0;
	char	chk_bcc		= 0;
	BYTE	recv_code	= 0;
	int		nresult		= 0;
	BYTE	ctrl_rsp	= 0;

	CString	m_strNVLog;

	// AP Holding 방지를 위해 max timeout을 설정한다.
	DWORD	max_timeout = GetTickCount() + (180*1000);
	MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_NETFLOW_STATE,	INIT_STS);			// [#258] NH AIREAT 2008.06.05 - SAVE NET FLOW

	NVDump('O', 'K', "01", L"", L"NEW-MATRIX" );

	// [#595] NH KSK 2009.12.09 PreDialing시에 Host에서 Disconnect Event를 받을 경우 Connect Retry를 하도록 수정
	if (Check_Queue(RECV_EVENT, DISCONNECT_EVT))
	{
		NVDump('O', 'K', "01", L"", L"C-QueDisDT" );
		NHDEBUG(1, (_T("Check Queue Disconnect Detected\n")));
		return 1;
	}
	// end of [#595]

	while(GetTickCount() < max_timeout)
	{
		recv_code = 0;
		nresult = GetQueue(&recv_code);

		if (nresult == RECV_DATA)
		{
			// 7bit 처리를 software로 한다
			recv_code &= 0x7F;

			switch(status)
			{
			case INIT_STS:
				break;
			case CONN_STS:
				// [#105] KSK 2008.04.14
				// NO ENQ RQUIRED 적용
				if (!(MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_EOTCHECKMODE) == NO_ENQ_REQUIRED ||
					  MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_EOTCHECKMODE) == NO_ENQ_EOT_REQUIRED))  //[#556] SOOK 2009.08.12 NO ENQ/EOT REQUIRED 추가 
				{
					if (recv_code == DATA_ENQ)
					{
						NVDump('O', 'K', "01", L"", L"ENQRECV" );
						NHDEBUG(1, (_T("FLOW ENQ RECEIVE[%x]\n"), recv_code));

						// [#2075] KSK 2011.06.27
						if (nMode == RKT_CONNECTION)
						{
							SetCheckTimer(TIMER_CLEAR);
							return 0;
						}
						// end of [#2075]

						// ENQ RECEIVED
						SetCheckTimer(ACK_TIMEOUT);
						// DATA SEND
						status = ENQIN_STS;
						MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_NETFLOW_STATE,	ENQIN_STS);			// [#258] NH AIREAT 2008.06.05 - SAVE NET FLOW
						snd_rty++;
						m_pNetLine->LineSendData(m_SendData, m_SendLen);
						NHDEBUG(1, (_T("DATA SEND[%S][LEN:%d]\n"), m_SendData, m_SendLen));
						NVDump('O', 'K', "01", L"", L"DATASEND" );
					}
					else
					{
						// Dummy Data Received
						m_strNVLog.Format(L"VISA-DMY(%02x)", recv_code);
						NVDump('O', 'K', "01", L"", m_strNVLog);
					}
				}
				else
				{
					// 아무 동작을 안함
				}
				// end of [#105]
				break;
			case ENQIN_STS:
				if (recv_code == DATA_STX)
				{
					NVDump('O', 'K', "01", L"", L"STXRECV" );
					NHDEBUG(1, (_T("FLOW STX RECEIVE[%x]\n"), recv_code));
					SetCheckTimer(DATA_TIMEOUT);
					status = STXIN_STS;
					MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_NETFLOW_STATE,	STXIN_STS);			// [#258] NH AIREAT 2008.06.05 - SAVE NET FLOW
				}
				else if (recv_code == DATA_ACK)
				{
					NVDump('O', 'K', "01", L"", L"ACKRECV" );
					NHDEBUG(1, (_T("FLOW ACK RECEIVE[%x]\n"), recv_code));
					SetCheckTimer(DATA_TIMEOUT);
					status = ACKIN_STS;
					MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_NETFLOW_STATE,	ACKIN_STS);			// [#258] NH AIREAT 2008.06.05 - SAVE NET FLOW
				}
				else if (recv_code == DATA_NAK)
				{
					NVDump('F', 'K', "01", L"", L"NAKRECV" );
					NHDEBUG(1, (_T("FLOW NAK RECEIVE[%x]\n"), recv_code));
					if (snd_rty < 4)
					{
						// 3초후 resend
						Delay_Msg(3000);
						SetCheckTimer(ACK_TIMEOUT);
						// DATA SEND
						snd_rty++;
						m_pNetLine->LineSendData(m_SendData, m_SendLen);
						NHDEBUG(1, (_T("DATA RE-SEND[%s][%d]\n"), m_SendData, m_SendLen));
						NVDump('F', 'K', "01", L"", L"RE-SEND" );
					}
					else
					{
						SetCheckTimer(TIMER_CLEAR);
						NVDump('F', 'K', "01", L"", L"NAKOVER" );
						NHDEBUG(1, (_T("SEND NAK COUNT ERROR[%x]\n"), recv_code));
						return 11;
					}
				}
				else
				{
					// Dummy Data Received
					m_strNVLog.Format(L"VISA-DMY(%02x)", recv_code);
					NVDump('O', 'K', "01", L"", m_strNVLog);
				}
				break;
			case ACKIN_STS:
				if (recv_code == DATA_STX)
				{
					NVDump('O', 'K', "01", L"", L"STXRECV" );
					NHDEBUG(1, (_T("FLOW STX RECEIVE[%x]\n"), recv_code));
					SetCheckTimer(DATA_TIMEOUT);
					status = STXIN_STS;
					MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_NETFLOW_STATE,	STXIN_STS);			// [#258] NH AIREAT 2008.06.05 - SAVE NET FLOW
				}
				else
				{
					// Dummy Data Received
					m_strNVLog.Format(L"VISA-DMY(%02x)", recv_code);
					NVDump('O', 'K', "01", L"", m_strNVLog);
				}
				break;
			case STXIN_STS:
				// calculate bcc
				// buffering
				if (recv_code != DATA_ETX)
				{
					//if (m_RecvLen >= NET_BUFF_SIZE)		// V01.02.29 BUFFER SIZE 변경 REVIEW8 [#89] KSK 2008.04.14
					if (m_RecvLen >= NETBUF_RECV_SIZE)		// [#2012] NH KJW 2011.03.31
					{
						// receive buffer error
						SetCheckTimer(TIMER_CLEAR);

						ClearQueue();					// V01.02.29 BUFFER SIZE 변경 REVIEW8 [#89] KSK 2008.04.14

						// receive buffer clear
						// [#2012] NH KJW 2011.02.09
						//memset(m_RecvData, 0, sizeof(m_RecvData));
						memset(m_RecvData, 0, NETBUF_RECV_SIZE);
						// end of [#2012]
						m_RecvLen = 0;
						chk_bcc = 0;
						return 7;		// V01.02.29 change return value REVIEW8 [#89] KSK 2008.04.14
					}

					m_RecvData[m_RecvLen] = recv_code;
					m_RecvLen++;
				}

				chk_bcc ^=recv_code;
				
				if (recv_code == DATA_ETX)
				{
					status = ETXIN_STS;
					MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_NETFLOW_STATE,	ETXIN_STS);			// [#258] NH AIREAT 2008.06.05 - SAVE NET FLOW
				}
				break;
			case ETXIN_STS:
				// check bcc
				if (chk_bcc == recv_code)
				{
					NVDump('O', 'K', "01", L"", L"DATARECV" );
					NHDEBUG(1, (_T("FLOW DATA RECEIVE OK[%x]\n"), recv_code));
					NHDEBUG(1, (_T("m_RecvLen(%d)\n"), m_RecvLen));
					status = EOTWAIT_STS;
					MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_NETFLOW_STATE,	EOTWAIT_STS);			// [#258] NH AIREAT 2008.06.05 - SAVE NET FLOW
					// [#105] KSK 2008.04.14 EOT OPTIONAL 적용
					if (MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_EOTCHECKMODE) == EOT_OPTIONAL)
					{
						SetCheckTimer(EOT_TIMEOUT_OPTIONAL);
					}
					else
					{
						SetCheckTimer(EOT_TIMEOUT_REQUIRED);
					}
					// end of [#105]

					ctrl_rsp = DATA_ACK;
					m_pNetLine->LineSendData(&ctrl_rsp, 1);

					// [#2075] NH KSK 2011.06.27
					if (nMode == RKT_MODE_R0 || nMode == RKT_MODE_R1)
					{
						SetCheckTimer(TIMER_CLEAR);
						return 0;
					}
					// end of [#2075]

					// [#105] KSK 2008.04.14	NO USE EOT 적용
					if (MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_EOTCHECKMODE) == NO_EOT_REQUIRED
						|| MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_EOTCHECKMODE) == NO_ENQ_EOT_REQUIRED ) //[#556] SOOK 2009.08.12 NO ENQ/EOT REQUIRED 추가 
					{
						return 0;
					}
					// end of [#105]
				}
				else if (nak_rty < 4)
				{
					NVDump('F', 'K', "01", L"", L"LRCERROR" );
					NHDEBUG(1, (_T("FLOW BCC ERROR\n")));
					// [#2012] NH KJW 2011.02.09
					//memset(m_RecvData, 0, sizeof(m_RecvData));
					memset(m_RecvData, 0, NETBUF_RECV_SIZE);
					// end of [#2012]
					m_RecvLen = 0;
					chk_bcc = 0;
					// bcc error nak send
					nak_rty++;
					status = ACKIN_STS;
					MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_NETFLOW_STATE,	ACKIN_STS);			// [#258] NH AIREAT 2008.06.05 - SAVE NET FLOW
					SetCheckTimer(DATA_TIMEOUT);
					ctrl_rsp = DATA_NAK;
					m_pNetLine->LineSendData(&ctrl_rsp, 1);
					NVDump('F', 'K', "01", L"", L"NAKSEND" );
				}
				else
				{
					NVDump('F', 'K', "01", L"", L"NAKOVER" );
					NHDEBUG(1, (_T("FLOW BCC ERROR - CLOSE\n")));
					// [#2012] NH KJW 2011.02.09
					//memset(m_RecvData, 0, sizeof(m_RecvData));
					memset(m_RecvData, 0, NETBUF_RECV_SIZE);
					// end of [#2012]
					m_RecvLen = 0;
					chk_bcc = 0;
					// bcc error & nak retry over
					ctrl_rsp = DATA_EOT;
					m_pNetLine->LineSendData(&ctrl_rsp, 1);
					// error report
					return 10;
				}
				break;
			case EOTWAIT_STS:
				if (recv_code == DATA_EOT || recv_code == DATA_ENQ)
				{
					NVDump('O', 'K', "01", L"", L"RECV:EOT/ENQ" );
					NHDEBUG(1, (_T("FLOW EOT RECEIVE[%x]\n"), recv_code));
					SetCheckTimer(TIMER_CLEAR);
					ctrl_rsp = DATA_EOT;
					m_pNetLine->LineSendData(&ctrl_rsp, 1);
					// success
					return 0;
				}
				else
				{
					m_strNVLog.Format(L"VISA-DMY(%02x)", recv_code);
					NVDump('O', 'K', "01", L"", m_strNVLog );
				}
				break;
			default:
				break;
			}
		}
		else if (nresult == RECV_EVENT)
		{			
			switch(status)
			{
			case INIT_STS:
				if (recv_code == CONNECT_EVT)
				{
					NVDump('O', 'K', "01", L"", L"CONNECT" );
					NHDEBUG(1, (_T("FLOW CONNECT[%x]\n"), recv_code));

					// [#444] [NH] KSK 2008.11.05 위치 이동
					// Connect된 순간에 success로 return함.(사양변경 사유: XP와 동일 처리)
					if (m_bNetworkTest == TRUE)	// [#585] NH KSK 2009.12.03
					{
						m_bNetworkTest = FALSE;		// [#2016] NH KSK 2011.02.09
						// success return
						return 0;
					}
					// end of [#444]

					// [#105] KSK 2008.04.14
					// NO ENQ REQUIRED 적용
					if (MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_EOTCHECKMODE) == NO_ENQ_REQUIRED
						|| MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_EOTCHECKMODE) == NO_ENQ_EOT_REQUIRED ) //[#556] SOOK 2009.08.12 NO ENQ/EOT REQUIRED 추가 
					{
						// [#2075] KSK 2011.06.27
						if (nMode == RKT_CONNECTION)
						{
							SetCheckTimer(TIMER_CLEAR);
							return 0;
						}
						// end of [#2075]

						SetCheckTimer(ACK_TIMEOUT);
						status = ENQIN_STS;
						MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_NETFLOW_STATE,	ENQIN_STS);			// [#258] NH AIREAT 2008.06.05 - SAVE NET FLOW
						snd_rty++;
						m_pNetLine->LineSendData(m_SendData, m_SendLen);
						NVDump('O', 'K', "01", L"", L"DATASEND" );	
					}
					else
					{
						SetCheckTimer(ENQ_TIMEOUT);
						status = CONN_STS;
						MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_NETFLOW_STATE,	CONN_STS);			// [#258] NH AIREAT 2008.06.05 - SAVE NET FLOW
					}
					// end of [#105]
				}
				else if (recv_code == DISCONNECT_EVT || recv_code == USER_NOANSWER ||
					     recv_code == USER_NODIALTONE || recv_code == USER_LINEBUSY || recv_code == TIMEOUT_EVT)
				{
					SetCheckTimer(TIMER_CLEAR);
					CString strLog;
					strLog.Format(L"DISRECV[%02X]", recv_code);
					NVDump('F', 'K', "01", L"", strLog );
					NHDEBUG(1, (_T("INIT_DISC[%x]\n"), recv_code));
					switch(recv_code)
					{
					case USER_NODIALTONE:
						NVDump('O', 'K', "01", L"", L"NODIALTONE" );
						return 2;
					case USER_NOANSWER:
						NVDump('O', 'K', "01", L"", L"NOANSWER" );
						return 3;
					case USER_LINEBUSY:
						NVDump('O', 'K', "01", L"", L"LINEBUSY" );
						return 4;
					default:
						return 1;
					}
				}

				break;
			case CONN_STS:
				if (recv_code == DISCONNECT_EVT || recv_code == USER_NOANSWER ||
					recv_code == USER_NODIALTONE || recv_code == USER_LINEBUSY || recv_code == TIMEOUT_EVT)
				{
					SetCheckTimer(TIMER_CLEAR);
					NVDump('F', 'K', "01", L"", L"DISRECV[C]" );
					return 5;
				}
				break;
			case ENQIN_STS:
				NHDEBUG(1, (_T("FLOW CONNECT[ENQIN_STS]\n")));
				if (recv_code == DISCONNECT_EVT || recv_code == USER_NOANSWER ||
					recv_code == USER_NODIALTONE || recv_code == USER_LINEBUSY || recv_code == TIMEOUT_EVT)
				{
					SetCheckTimer(TIMER_CLEAR);
					NVDump('F', 'K', "01", L"", L"DISRECV[E]" );
					return 6;
				}
				break;
			case ACKIN_STS:
			case STXIN_STS:
			case ETXIN_STS:
				NHDEBUG(1, (_T("FLOW CONNECT[ACKIN_STS STXIN_STS ETXIN_STS]\n")));
				if (recv_code == DISCONNECT_EVT || recv_code == USER_NOANSWER ||
					recv_code == USER_NODIALTONE || recv_code == USER_LINEBUSY || recv_code == TIMEOUT_EVT)
				{
					SetCheckTimer(TIMER_CLEAR);
					NVDump('F', 'K', "01", L"", L"DISRECV[S]" );

					// [#2012] NH KJW 2011.02.09
					//memset(m_RecvData, 0, sizeof(m_RecvData));
					memset(m_RecvData, 0, NETBUF_RECV_SIZE);
					// end of [#2012]
					m_RecvLen = 0;
					chk_bcc = 0;
					return 7;
				}
				break;
			case EOTWAIT_STS:
				NHDEBUG(1, (_T("FLOW CONNECT[EOTWAIT_STS]\n")));
				// [#105] KSK 2008.04.14
				if (recv_code == DISCONNECT_EVT || recv_code == USER_NOANSWER ||
					recv_code == USER_NODIALTONE || recv_code == USER_LINEBUSY)
				{
					SetCheckTimer(TIMER_CLEAR);
					NVDump('F', 'K', "01", L"", L"DISRECV[O]" );
					return 8;
				}
				else if (recv_code == TIMEOUT_EVT)
				{
					SetCheckTimer(TIMER_CLEAR);

					if (MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_EOTCHECKMODE) == EOT_OPTIONAL)
					{
						// 10초 이후에 TIMEOUT인 경우에는 정상처리 한다.
						NVDump('O', 'K', "01", L"", L"DISRECV[O]" );
						return 0;
					}
					else
					{
						// EOT OPTIONAL이 아닌 경우에는 비정상 처리 한다.
						NVDump('F', 'K', "01", L"", L"DISRECV[E]" );
						return 8;
					}
				}
				// end of [#105]
				break;
			default:
				break;
			}
		}
		else
		{
			// [#2075] NH KSK 2011.06.27
			if (status == RKT_SEND_STS)
			{
				SetCheckTimer(ACK_TIMEOUT);
				status = ENQIN_STS;
				snd_rty++;
				m_pNetLine->LineSendData(m_SendData, m_SendLen);
				NVDump('O', 'K', "01", L"", L"DATASEND" );	
			}
			// end of [#2075
		}

		if(nresult == 0)	Delay_Msg(100);

	}
	NHDEBUG(1, (_T("MATRIX RETURN\n")));
	return 1;
}

// [#525] US KSK 2009.05.25
int	CVISAIICtrl::PreDialStart(CString CurrentPhone)
{
	int	nRetrun = 0;
	if (m_bPreDialStart == FALSE)
	{
		ClearQueue();
		
		m_bPreDialStart = TRUE;
		nRetrun = m_pNetLine->LineOpen(CurrentPhone);
		
		if (nRetrun)
		{
			// LineOpen Error
			m_bPreDialStart = FALSE;		// LineOpen Fail시 기존 FlowControl을 정상적으로 흐르도록 Flag를 Clear해준다.
			m_pNetLine->LineClose();
		}
	}

	return TRUE;
}

int CVISAIICtrl::PreDialCancelByUser()
{
	if (m_bPreDialStart == TRUE)
	{
		ClearQueue();

		m_bPreDialStart = FALSE;
		m_pNetLine->LineClose();
	}

	return FALSE;
}
// end of [#525]

// [#2075] NH KSK 2011.06.27
BOOL CVISAIICtrl::RKTConnectCheck()
{
	return m_pNetLine->LineCallState(1);
}

// 0 : success , else : fail
int	CVISAIICtrl::RKTConnectOpen(LPCTSTR pDestInfo, LPCTSTR port, LPCTSTR option)
{
	int nReturn = 0;

	ClearQueue();

	nReturn = m_pNetLine->LineOpen(pDestInfo, port, option);

	if (!nReturn)
	{
		// Line Open Success
		nReturn = Flow_Matrix(INIT_STS, RKT_CONNECTION);

		if (nReturn)
		{
			m_pNetLine->LineClose();
		}
	}
	else
	{
		// LineOpen Error
		m_pNetLine->LineClose();
	}
	return nReturn;
}

int	CVISAIICtrl::RKTConnectClose()
{
	m_pNetLine->LineClose();
	return FALSE;
}
// end of [#2075]
