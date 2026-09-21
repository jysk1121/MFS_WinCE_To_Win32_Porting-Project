// EventNoticeCtrl.cpp: implementation of the CEventNoticeCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//#define NH_DEBUG

#include ".\Common\NHDbgApi.h"
#include ".\Dev\DevDefine.h"
#include "EventNoticeCtrl.h"
#include "afxinet.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define		CONN_TIMEOUT	20
#define		CONN_TIMEOUT2	30	// [#RWC6-40] PAI MCCP Transaction Delay
#define		DATA_TIMEOUT	120

CEventNoticeCtrl::CEventNoticeCtrl(CLineCtrl *line) : CFlowCtrl(line)
{

}

CEventNoticeCtrl::~CEventNoticeCtrl()
{

}

int CEventNoticeCtrl::SendData(BYTE *pSendBuf, int Length, LPCTSTR pDestInfo, LPCTSTR port, LPCTSTR option)
{
	NHDEBUG(1, (_T("CEventNoticeCtrl::SendData(Length=[%d], Destination[%s], Port=[%s], Option[%s])\n"), Length, pDestInfo, port, option));
	
	int nDataTimeOut = 0;			// [#2574] US Justin GivePay Enhancement3

	// Parse option
	CString strTerminateString = L"";
	CString strOption = CString(option);

	CStringArray arrStrOpt;
	SplitString(strOption, ",", arrStrOpt);

	// SSL/TLS Option (the first string)
	CString strConOption = arrStrOpt.GetAt(0);

	// Find Options
	for(int i=1; i<arrStrOpt.GetSize(); i++)
	{
		CString strTemp = arrStrOpt.GetAt(i);

		// Terminate String Option
		int nLoc = strTemp.Find( L"TERMINATE:" );
		if(nLoc>=0)
			strTerminateString = strTemp.Mid( nLoc+10 );

		// [#2574] US Justin GivePay Enhancement3, TimeOut
		nLoc = strTemp.Find( L"TIMEOUT_OVERWRITE:" );
		if(nLoc>=0)
			nDataTimeOut = Asc2Int( strTemp.Mid(nLoc+18) );
		// End of [#2574]

		// Other Options.....
		//
		//
	}
	NHDEBUG(1, (_T("CEventNoticeCtrl, Destination=[%s], SSL/TLS OPTION =[%s], Terminate String = [%s], Data TimeOut = [%d]\n"), pDestInfo, strConOption, strTerminateString, nDataTimeOut));

	int		nreturn = 0;

	// queue clear
	ClearQueue();

	memset(m_RecvData, 0, NETBUF_RECV_SIZE);
	m_RecvLen = 0;

	memset(m_SendData, 0, NETBUF_SEND_SIZE);
	m_SendLen = 0;

	memcpy( m_SendData, pSendBuf, Length);
	m_SendLen =Length;

	nreturn = m_pNetLine->LineOpen(pDestInfo, port, strConOption);		// [#2374] US Justin
	NHDEBUG(1, (_T("  m_pNetLine->LineOpen() = [%d]\n"), nreturn));
	if (!nreturn)
	{
		NHDEBUG(1, (_T("EventNotice FLOW MATRIX START\n")));
		SetCheckTimer(CONN_TIMEOUT2);							// [#RWC6-40] PAI MCCP Transaction Delay
		//SetCheckTimer(CONN_TIMEOUT);
		// [#2374] US Justin
		//nreturn = Flow_Matrix();
		if( strTerminateString.GetLength() > 0)
			nreturn = Flow_Matrix(nDataTimeOut, strTerminateString);		// [#2574] US Justin GivePay Enhancement3, TimeOut
		else
			nreturn = Flow_Matrix(nDataTimeOut);							// [#2574] US Justin GivePay Enhancement3, TimeOut
		// End of [#2374]
	}
	m_pNetLine->LineClose();
	NHDEBUG(1, (_T("  m_pNetLine->LineClose()\n")));
	
	return nreturn;
}

BOOL CEventNoticeCtrl::CloseLine()
{
	m_pNetLine->LineClose();
	return TRUE;
}
			
// [#2374] US Justin Add Terminate String
// [#2574] US Justin Add Data Time Out
int CEventNoticeCtrl::Flow_Matrix(int nDataTimeOut, LPCTSTR strTerminate, int initStatus, int nMode)
{
	NHDEBUG(1, (_T("CEventNoticeCtrl::Flow_Matrix DataTimeOut = [%d]\n"), nDataTimeOut ));

	int		status		= initStatus;
	int		snd_rty		= 0;
	WORD	data_len	= 0;
	BYTE	recv_code	= 0;
	int		nresult		= 0;
	
	// [#2347] US Justin .. Terminate String Buffer
	int		nCompLoc = 0;
	CString strTermStr = L"";
	if(strTerminate != NULL)
		strTermStr = CString(strTerminate);
	// End of [#2347]

	DWORD	max_timeout = GetTickCount() + (180*1000);			// max timeout => Prevent ATM Holding
	MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_NETFLOW_STATE,	INIT_STS);

	while(GetTickCount() < max_timeout)
	{
		nresult = GetQueue(&recv_code);

		if (nresult == RECV_DATA)			// One Way Data Sending.. Not receiving Data... So far
		{
			switch(status)
			{
				case INIT_STS:
					//NHDEBUG(1, (_T("  CEventNoticeCtrl::Flow_Matrix, RECV_DATA Status = [INIT_STS], Received = [%c]\n"), recv_code));
					break;
				case CONN_STS:
					//NHDEBUG(1, (_T("  CEventNoticeCtrl::Flow_Matrix, RECV_DATA Status = [CONN_STS], Received = [%c]\n"), recv_code));
					if (m_RecvLen >= NETBUF_RECV_SIZE)
					{
						SetCheckTimer(TIMER_CLEAR);
						ClearQueue();
						memset(m_RecvData, 0, NETBUF_RECV_SIZE);
						m_RecvLen = 0;
						return 7;
					}
					m_RecvData[m_RecvLen] = recv_code;
					m_RecvLen++;
					
					// [#2347] US Justin Terminate String
					if( strTermStr.GetLength() > 0)
					{
						if( strTermStr.GetAt(nCompLoc) == recv_code )
						{
							nCompLoc ++;
							if(nCompLoc>=strTermStr.GetLength())
							{
								NHDEBUG(1, (_T("  CEventNoticeCtrl::Flow_Matrix, RECV_DATA Status = [CONN_STS], FOUND TERMINATE STRING...... \n")));
								SetCheckTimer(TIMER_CLEAR);
								return 0;
							}
						}
						else
							nCompLoc = 0;
					}
					// End of [#2347]
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
					NHDEBUG(1, (_T("  CEventNoticeCtrl::Flow_Matrix, RECV_EVENT Status = [INIT_STS], recv_code = [CONNECT_EVT]\n")));
					NVDump('O', 'K', "01", L"", L"CONNECT" );

					// [#2574] US Justin Add Data Time Out
					//SetCheckTimer(DATA_TIMEOUT);
					if(nDataTimeOut>0)		SetCheckTimer(nDataTimeOut);
					else					SetCheckTimer(DATA_TIMEOUT);
					// End of [#2574]

					status = CONN_STS;
					MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_NETFLOW_STATE,	CONN_STS);
					snd_rty++;
					if (m_pNetLine->LineSendData(m_SendData, m_SendLen) != 0)		// Send Failure
					{
						NHDEBUG(1, (_T("  CEventNoticeCtrl::Flow_Matrix, RECV_EVENT Status = [INIT_STS], LineSendData Error\n")));
						return 1;
					}
					NHDEBUG(1, (_T("DATA SEND\n")));
					NVDump('O', 'K', "01", L"", L"DATASEND" );
				}
				else if (recv_code == DISCONNECT_EVT || recv_code == USER_NOANSWER ||
					     recv_code == USER_NODIALTONE || recv_code == USER_LINEBUSY || recv_code == TIMEOUT_EVT)
				{
					SetCheckTimer(TIMER_CLEAR);
					NHDEBUG(1, (_T("  RECV_EVENT Status = [INIT_STS], recv_code = [DISCONNECT_EVT or ..]\n")));
					NVDump('F', 'K', "01", L"", L"DISRECV[I]" );
					return 1;
				}

				break;
			case CONN_STS:
				if (recv_code == DISCONNECT_EVT || recv_code == USER_NOANSWER ||
					recv_code == USER_NODIALTONE || recv_code == USER_LINEBUSY || recv_code == TIMEOUT_EVT)
				{
					SetCheckTimer(TIMER_CLEAR);
					NHDEBUG(1, (_T("  RECV_EVENT Status = [CONN_STS], recv_code = [%02X]\n"), recv_code));
					NVDump('F', 'K', "01", L"", L"DISRECV[C]" );
					return 0;
				}
				break;
			default:
				break;
			}
		}

		if(nresult == 0)	
			Delay_Msg(100);
	}
	NHDEBUG(1, (_T("MATRIX RETURN\n")));
	return 1;
}
