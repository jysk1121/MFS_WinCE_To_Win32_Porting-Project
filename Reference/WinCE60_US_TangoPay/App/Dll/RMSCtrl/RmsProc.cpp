/////////////////////////////////////////////////////////////////////////////
// TranRMS.cpp : implementation file
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

//#define NH_DEBUG

#include ".\Common\CmnLib.h"
#include ".\Common\NHDbgApi.h"

#include ".\Dev\DevDefine.h"
#include ".\Rms\RmsCtrl.h"
#include "..\H\Tran\TranCmnDefine.h"	// [#387] [NH] KSK 2008.7.28
//#include "zip.h"						// [#523] US KSK 2009.05.20 Upload File은 무조건 .zip으로해서 올려야 한다. [###2] CmnLib 사용으로 변경
#include "LibUpdate\LibUpdate.h"
#include ".\Dev\DiskManager.h"			// [#RWC6-127] Add Disk Management to RMS Status
#include ".\Common\LibertyXConfigurationManager.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

//[#635] SOOK 2010.04.01 호주 APCA 인증 대응 Download File Authentication 
#if (AU_VERSION)
#include <Wincrypt.h>
#endif
//end of [#635]


//////////////////////////////////////////
// Global Variable.

#define GUARD_MESSAGE_LEN(len, data, member) \
{ \
	int verLen = ((int)&data->member - (int)data) + sizeof(data->member); \
	if (len < verLen) return TRUE; \
}

#define		DATA_STX		0x02
#define		DATA_ETX		0x03
#define		DATA_EOT		0x04
#define		DATA_ENQ		0x05
#define		DATA_ACK		0x06
#define		DATA_NAK		0x15
#define		DATA_REQ_CERT		0x43	//[#635] SOOK 2010.04.01 호주 APCA 인증 대응 Download File Authentication 
#define		DATA_RES_FAILURE	0x46	//[#635] SOOK 2010.04.01 호주 APCA 인증 대응 Download File Authentication 

#define XOR_MIN_LENGHT	4
#define	XOR_TBL_SIZE	64

BYTE XOR_TBL[XOR_TBL_SIZE] =
		{0x08, 0x40, 0x16, 0x48, 0x24, 0x56, 0x32, 0x64,
		 0x07, 0x39, 0x15, 0x47, 0x23, 0x55, 0x31, 0x63,
		 0x06, 0x38, 0x14, 0x46, 0x22, 0x54, 0x30, 0x62,
		 0x05, 0x37, 0x13, 0x45, 0x21, 0x53, 0x29, 0x61,
		 0x04, 0x36, 0x12, 0x44, 0x20, 0x52, 0x28, 0x60,
		 0x03, 0x35, 0x11, 0x43, 0x19, 0x51, 0x27, 0x59,
		 0x02, 0x34, 0x10, 0x42, 0x18, 0x50, 0x26, 0x58,
 		 0x01, 0x33, 0x09, 0x41, 0x17, 0x49, 0x25, 0x57}; 

// buffers.
#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
	#define RMS_BUFSIZE		(1024 * 128)				// 0x010311 버전의 데이터 사이즈 증가로 버퍼사이즈 늘림[#546] SOOK 2009.07.23 다기능 BIN LIST 기능 추가함 
#else
	#define RMS_BUFSIZE		(1024 * 10)				// 0x010305 버전의 데이터 사이즈 증가로 버퍼사이즈 늘림. 2008.08.05 psc
													// [#2213] US Justin 2013.07.30  01.03.19 Increase Data Buffer 1024*8 => 1024*10
#endif

BYTE	gRMSBuffer[RMS_BUFSIZE];
BYTE	*gpRMSBuffer = gRMSBuffer;

#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
	#define RMS_DATA_BUFSIZE		(1024 * 128)		// 0x010311 버전의 데이터 사이즈 증가로 버퍼사이즈 늘림[#546] SOOK 2009.07.23 다기능 BIN LIST 기능 추가함 
#else
	#define RMS_DATA_BUFSIZE		(1024 * 10)		// 0x010305 버전의 데이터 사이즈 증가로 버퍼사이즈 늘림. 2008.08.05 psc
													// [#2213] US Justin 2013.07.30  01.03.19 Increase Data Buffer 1024*8 => 1024*10
#endif

BYTE	gRMSDataBuffer[RMS_DATA_BUFSIZE];
BYTE	*gpRMSDataBuffer = gRMSDataBuffer;

#define RMS_FILE_BUFSIZE		((32*1024-6)+1)
BYTE	gRMSFileBuffer[RMS_FILE_BUFSIZE];
BYTE	*gpRMSFileBuffer = gRMSFileBuffer;
#define SIZEOF_FILEBUF	(sizeof(gRMSFileBuffer))

#define TEMP_BUFSIZE	(MAX_PATH)
TCHAR	gwTempBuf[TEMP_BUFSIZE+2];

///////////////////////////
// Line Type
int				g_LineType;
int				g_OpenType;
unsigned char	g_preMsgID;

///////////////////////////
// EJ Upload 관련.
EJ_UPLOAD_MARK	g_EJMark;
WORD			g_EJUploadTraceIdx;

///////////////////////////
// Last X Upload 관련.
LONG			g_lUpOffsetLX0;
LONG			g_lUpOffsetLX1;
LONG			g_lUpFileIdx;
LONG			g_lUpOffsetfileLX;
BOOL			g_lfileload;
DWORD			g_SentXCount;
DWORD			g_LastXCount;

// [#2239] NH KMK 2013.10.24 Log Backup 관련
typedef struct
{
	WORD	LastNo1;
	WORD	LastNo2;
	WORD	Valid;				// if not 9114 , then initialize NVRAM Log Area
	WORD	Rsvd2[5];
	BYTE	Buff[64*1024 - 16];
}	sLog, *pLog;
// end of [#2239]

//////////////////////////////////////////
// Function Define.

BYTE CRmsCtrl::RMS_GetMachineType()
{
	CNHConfig* config = GetConfigFuncPointer();
	if (config->GetCEVersion() == WINCE_7)
	{
		return WINCE7_RMS_COMMON_MACHINECODE;
	}

	CString strMachineKind = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND);

	if (strMachineKind.Left(6) == L"NH2700")		return WINCE_RMS_2700_MACHINECODE_FRONT;
	else if (strMachineKind == L"NH1800SE")			return WINCE_RMS_1800SE_MACHINECODE_FRONT;
	else if (strMachineKind == L"MX4000W")			return WINCE_RMS_MX4000W_MACHINECODE;
	else if (strMachineKind == L"MX5000SE")			return WINCE_RMS_MX5000SE_MACHINECODE;								
	else if (strMachineKind == L"NH2600")			return WINCE_RMS_HALO2600_MACHINECODE;
	else if (strMachineKind == L"NH1500SE")			return WINCE_RMS_NH1500SE_MACHINECODE;
	else if (strMachineKind == L"MX5200SE")			return WINCE_RMS_MX5200SE_MACHINECODE_FRONT;
	else if (strMachineKind == L"MX2600SE")			return WINCE_RMS_HALO2_MACHINECODE;
	else if (strMachineKind == L"MX5300SE")			return WINCE_RMS_MX5300SE_MACHINECODE;
	else if (strMachineKind == L"MX2800SE")			return WINCE_RMS_MX2800SE_MACHINECODE;
	else if (strMachineKind == L"MX2800T")			return WINCE_RMS_MX2800T_MACHINECODE;

	return WINCE_RMS_2700_MACHINECODE_FRONT;
}

//////////////////////////////////////////
// RMS 통신 담당.

/*-------------------------------------------------------------------
 CLASS    NAME: CRmsCtrl
 FUNCTION NAME: RMS_Communication()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : RMS와의 통신을 수행하는 Main 분기 이다.
-------------------------------------------------------------------*/
//BOOL CRmsCtrl::RMS_Communication(int LineType, int OpenType)
BOOL CRmsCtrl::RMS_Communication(int LineType, int OpenType, char SubCommand)	// [#613] AU_C KSK 2010.01.18
{
	// line type save
	g_LineType = LineType;
	g_OpenType = OpenType;
	
	// clear previous MsgID
	g_preMsgID = 0x0;
	
	// all buffer reset.
	memset(gpRMSBuffer, 0, RMS_BUFSIZE);
	memset(gpRMSDataBuffer, 0, RMS_DATA_BUFSIZE);
	memset(gwTempBuf, 0, TEMP_BUFSIZE);

	if (LineType == RMS_LINE_DIALUP)
	{
		if (OpenType == RMS_LISTEN)
			return RMS_Proc_Dial();
//		else if (OpenType == RMS_CONNECT)
//			return RMS_Status_Dial();
		else if (OpenType == RMS_CONNECT)	// [#613] AU_C KSK 2010.01.18
		{
			if (SubCommand == 0)
				return RMS_Status_Dial();
// [#2037] AU KSK 2011.03.31	
// [#2457] US Justin 2016.12.09 Support Moniview Journal Uploading
// [#2500] CA Justin 2017.08.22 Support Moniview Journal Uploading
#if (AU_VERSION || US_VERSION  || CA_VERSION)	
			else if (SubCommand == RMS_REQ_JNL)
				return RMS_JournalUpload_Dial();
#endif
		}									// end of [#613]
	}
	else if (LineType == RMS_LINE_TCP)
	{
		if (OpenType == RMS_LISTEN)
			return RMS_Proc_Tcp();
//		else if (OpenType == RMS_CONNECT)
//			return RMS_Status_Tcp();
		else if (OpenType == RMS_CONNECT)	// [#613] AU_C KSK 2010.01.18
		{
			if (SubCommand == 0)
				return RMS_Status_Tcp();
			// [#2580] AU Kook 2018.11.07 RMS Test
#if (AU_A_VERSION)
			else if (SubCommand == 1)
				return RMS_Status_Tcp_NotRecvRmsCmd();
#endif
			// end of [#2580]
// [#2037] AU KSK 2011.03.31	
// [#2457] US Justin 2016.12.09 Support Moniview Journal Uploading
// [#2500] CA Justin 2017.08.22 Support Moniview Journal Uploading
#if (AU_VERSION || US_VERSION  || CA_VERSION)	
			else if (SubCommand == RMS_REQ_JNL)
				return RMS_JournalUpload_Tcp();
#endif
		}									// end of [#613]
	}

	NHDEBUG(1, (_T("RMS Communication Parameter is wrong.. Line(%d), Act(%d)\n"), LineType, OpenType));

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CRmsCtrl
 FUNCTION NAME: RMS_Proc_Dial()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : RMS ENABLE 통신 절차를 수행한다. (DialUp 용)
-------------------------------------------------------------------*/
BOOL CRmsCtrl::RMS_Proc_Dial()
{
	int				nRecv;
	PRMS_CMD_COMM	pMSG;
	
	NHDEBUG(1, (_T("\n//// RMS DIAL ENABLE ///////////////////////////\n")));
	
	NHDEBUG(1, (_T("Call Accept FROM RMS..\n")));
	// 1. Connect.
	if (m_pDevCmn->fnNET_RMSConnectAccept() != RMS_OK)
	{
		NHDEBUG(1, (_T("<RMS_DIAL_ENABLE> ConnectAccept Failed\n")));
		goto ErrorReturn;
	}
	
	NHDEBUG(1, (_T("Wait First Command FROM RMS..\n")));
	// RMS -> ATM
	// 2. Recv CMD
	memset(gpRMSBuffer, 0, RMS_BUFSIZE);
	nRecv = RMS_BUFSIZE;
	if (RMS_Recv(gpRMSBuffer, &nRecv, 0) != RMS_OK)
	{
		NHDEBUG(1, (_T("<RMS_DIAL_ENABLE> RMS_Recv() Failed\n")));
		goto ErrorReturn;
	}
	else
	{
		// Verify.
		if (!RMS_VerifyMsg(gpRMSBuffer, nRecv))
		{
			NHDEBUG(1, (_T("<RMS_DIAL_ENABLE> RMS_VerifyMsg() Failed\n")));
			goto ErrorReturn;
		}
		
		// OK PASS
		RMS_SendControl(DATA_ACK);
	}
	
	// Save Command.
	memcpy(gpRMSDataBuffer, gpRMSBuffer, nRecv);
	
	pMSG = (PRMS_CMD_COMM)gpRMSBuffer;
	
	// SETUP WRITE, FILE DOWNLOAD 는 STATUS 전송 하지 않음.
	if (pMSG->Header.MsgID != RMS_SET_SETUP && pMSG->Header.MsgID != RMS_REQ_FILE_DOWN &&
		pMSG->Header.MsgID != RMS_REQ_STATUS && pMSG->Header.MsgID != RMS_SET_BINLIST &&	// [#546] SOOK 2009.08.05 다기능 BIN LIST 기능 추가함 
		pMSG->Header.MsgID != RMS_ENHANCED_COUPON_DOWNLOAD && 								// [#546] SOOK 2009.08.05 다기능 BIN LIST 기능 추가함 // [#620] US KSK 2010.03.03 
		pMSG->Header.MsgID != RMS_SET_WEATHERE_INFO ) //[#586] SOOK 2009.12.29 iTM
	{
		// 3. Response. 
		// 3.1 first. Send Status 
		if (!RMS_SendATMStatus(pMSG->Header.MsgID))
			goto ErrorReturn;
	}
	
	// 3.2 Processing RMS CMD
	RMS_ParseCMD(gpRMSDataBuffer);
	
	Delay_Msg(5);
	// Close Connection.
	m_pDevCmn->fnNET_RMSConnectClose();	
	
	NHDEBUG(1, (_T("\n----NORMAL---------------------------------\n")));

	return TRUE;

ErrorReturn:

	// 에러 종료. Close Connection.
	
	// Send EOT
	RMS_SendControl(DATA_EOT);
	Delay_Msg(5);
	m_pDevCmn->fnNET_RMSConnectClose();
	NHDEBUG(1, (_T("\n------ERROR-------------------------------\n")));

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CRmsCtrl
 FUNCTION NAME: RMS_Status_Dial()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : RMS Status Send 통신 절차를 수행한다. (DialUp 용)
-------------------------------------------------------------------*/
BOOL CRmsCtrl::RMS_Status_Dial()
{
	NHDEBUG(1, (_T("\n//// RMS DIAL STATUS ///////////////////////////\n")));

	m_pDevCmn->fnNET_RMSConnectClose();
	Delay_Msg(1000);

	// 1. Connect
	if (m_pDevCmn->fnNET_RMSConnectOpen(RMS_CONNECT) != RMS_OK)
	{
		NHDEBUG(1, (_T("<RMS_DIAL_STATUS> DIAL Failed\n")));
		goto ErrorReturn;
	}

	// 3. Status Send
	if (!RMS_SendATMStatus(RMS_REQ_STATUS))
	{
		NHDEBUG(1, (_T("<RMS_DIAL_STATUS> Status Send Failed\n")));
		goto ErrorReturn;
	}

	// 4. Journal Upload <- 추후 지원.

	// 5. Send EOT
	RMS_SendControl(DATA_EOT);
	Delay_Msg(5);

	// 6. Close Connection.
	m_pDevCmn->fnNET_RMSConnectClose();	

	NHDEBUG(1, (_T("\n----NORMAL---------------------------------\n")));

	return TRUE;

ErrorReturn:

	// 에러 종료. Close Connection.
	
	// Send EOT
	RMS_SendControl(DATA_EOT);
	Delay_Msg(5);
	m_pDevCmn->fnNET_RMSConnectClose();
	NHDEBUG(1, (_T("\n------ERROR-------------------------------\n")));

	return FALSE;
}

// [#613] AU_C KSK 2010.01.21
/*-------------------------------------------------------------------
 CLASS    NAME: CRmsCtrl
 FUNCTION NAME: RMS_JournalUpload_Dial()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : RMS Journal Upload 통신 절차를 수행한다. (Dialup 용)
-------------------------------------------------------------------*/
BOOL CRmsCtrl::RMS_JournalUpload_Dial()
{
	NHDEBUG(1, (_T("\n//// RMS JOURNAL UPLOAD DIAL///////////////////////////\n")));

	m_pDevCmn->fnNET_RMSConnectClose();
	Delay_Msg(1000);

	// 1. Connect
	if (m_pDevCmn->fnNET_RMSConnectOpen(RMS_CONNECT) != RMS_OK)
	{
		NHDEBUG(1, (_T("<RMS_JournalUpload_Dial> DIAL Failed\n")));
		goto ErrorReturn;
	}

	// 3. Status Send
	if (!RMS_SendATMStatus(RMS_SCHEDULED_JOURNAL_UPLOAD))
	{
		NHDEBUG(1, (_T("<RMS_JournalUpload_Dial> Status Send Failed\n")));
		goto ErrorReturn;
	}

	// 4. Journal Upload
	RMS_ProcJNLUpload();

	// 5. Send EOT
	RMS_SendControl(DATA_EOT);
	Delay_Msg(5);

	// 6. Close Connection.
	m_pDevCmn->fnNET_RMSConnectClose();	

	NHDEBUG(1, (_T("\n----NORMAL---------------------------------\n")));

	return TRUE;

ErrorReturn:

	// 에러 종료. Close Connection.
	
	// Send EOT
	RMS_SendControl(DATA_EOT);
	Delay_Msg(5);
	m_pDevCmn->fnNET_RMSConnectClose();
	NHDEBUG(1, (_T("\n------ERROR-------------------------------\n")));

	return FALSE;
}
// end of [#613]


/*-------------------------------------------------------------------
 CLASS    NAME: CRmsCtrl
 FUNCTION NAME: RMS_Proc_Tcp()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : RMS ENABLE 통신 절차를 수행한다. (Tcp/ip 용)
-------------------------------------------------------------------*/
BOOL CRmsCtrl::RMS_Proc_Tcp()
{
	int				nRecv;
	PRMS_CMD_COMM	pMSG;
	
	NHDEBUG(1, (_T("\n//// RMS TCP ENABLE ///////////////////////////////\n")));
	// 1. Connect. when TCP/IP Mode, ConnectOpen is client mode.
	if (m_pDevCmn->fnNET_RMSConnectAccept() != RMS_OK)
	{
		NHDEBUG(1, (_T("<RMS_TCP_ENABLE> fnNET_RMSConnectAccept() Failed\n")));
		goto ErrorReturn;
	}
	
	// RMS -> ATM
	// 2. Recv CMD
	memset(gpRMSBuffer, 0, RMS_BUFSIZE);
	nRecv = RMS_BUFSIZE;
	if (RMS_Recv(gpRMSBuffer, &nRecv, 0) != RMS_OK)
	{
		NHDEBUG(1, (_T("<RMS_TCP_ENABLE> RMS_Recv() Failed\n")));
		goto ErrorReturn;
	}
 	else
	{
		// Verify.
		if (!RMS_VerifyMsg(gpRMSBuffer, nRecv))
		{
			NHDEBUG(1, (_T("<RMS_TCP_ENABLE> RMS_VerifyMsg() Failed\n")));
			goto ErrorReturn;
		}
	
		// OK PASS
		RMS_SendControl(DATA_ACK);
	}

	// Save Command.
	memcpy(gpRMSDataBuffer, gpRMSBuffer, nRecv);

	pMSG = (PRMS_CMD_COMM)gpRMSBuffer;
	
	// STATUS 전송 하지 않는 Case 지정 (SETUP WRITE, FILE DOWNLOAD, ..)
	if (pMSG->Header.MsgID != RMS_SET_SETUP && pMSG->Header.MsgID != RMS_REQ_FILE_DOWN &&
		pMSG->Header.MsgID != RMS_REQ_STATUS && pMSG->Header.MsgID != RMS_SET_BINLIST &&// [#546] SOOK 2009.08.05 다기능 BIN LIST 기능 추가함 
		pMSG->Header.MsgID != RMS_ENHANCED_COUPON_DOWNLOAD && 							// [#546] SOOK 2009.08.05 다기능 BIN LIST 기능 추가함 // [#620] US KSK 2010.03.03 
		pMSG->Header.MsgID != RMS_SET_WEATHERE_INFO ) //[#586] SOOK 2009.12.29 iTM	
	{
		// 3. Response. 
		// 3.1 first. Send Status 
		if (!RMS_SendATMStatus(pMSG->Header.MsgID))
			goto ErrorReturn;
	}
	NHDEBUG(1, (_T("<RMS_TCP_ENABLE> received data Length = [%d]\n"), nRecv ));

	// 3.2 Processing RMS CMD
	RMS_ParseCMD(gpRMSDataBuffer);			
	
	Delay_Msg(5);
	// 정상 종료. Close Connection.
	m_pDevCmn->fnNET_RMSConnectClose();
	NHDEBUG(1, (_T("\n----NORMAL---------------------------------\n")));
	
	return TRUE;

ErrorReturn:

	// 에러 종료. Close Connection.
	
	// Send EOT
	RMS_SendControl(DATA_EOT);
	Delay_Msg(5);
	m_pDevCmn->fnNET_RMSConnectClose();
	NHDEBUG(1, (_T("\n------ERROR-------------------------------\n")));

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CRmsCtrl
 FUNCTION NAME: RMS_Status_Tcp()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : RMS Status Send 통신 절차를 수행한다. (Tcp/ip 용)
-------------------------------------------------------------------*/
BOOL CRmsCtrl::RMS_Status_Tcp()
{
	int				nRecv;

	m_pDevCmn->fnNET_RMSConnectClose();
	Delay_Msg(1000);

	while (1)
	{
		NHDEBUG(1, (_T("\n//// RMS TCP STATUS ///////////////////////////////\n")));
		// 1. Connect. when TCP/IP Mode, ConnectOpen is client mode.
		if (m_pDevCmn->fnNET_RMSConnectOpen(RMS_CONNECT) != RMS_OK)
		{
			NHDEBUG(1, (_T("<RMS_TCP_STATUS> RMSConnectOpen() Failed\n")));
			goto ErrorReturn;
		}
		
		// 1. Send Status.
		NHDEBUG(1, (_T("<RMS_TCP_STATUS> Send Status Data\n")));
		if (!RMS_SendATMStatus(RMS_REQ_STATUS))
		{
			NHDEBUG(1, (_T("<RMS_TCP_STATUS> RMS_SendATMStatus(RMS_REQ_STATUS) Failed\n")));
			goto ErrorReturn;
		}
		
		// RMS -> ATM
		// 2. Recv CMD
		memset(gpRMSBuffer, 0, RMS_BUFSIZE);
		nRecv = RMS_BUFSIZE;
		if (RMS_Recv(gpRMSBuffer, &nRecv, 0) != RMS_OK)
		{
			NHDEBUG(1, (_T("<RMS_TCP_STATUS> RMS_Recv() Failed\n")));
			goto ErrorReturn;
		}

		// END OF PROC RMS
		if (nRecv == 1 && gpRMSBuffer[0] == DATA_EOT)
		{
			NHDEBUG(1, (_T("<RMS_TCP_STATUS> Recv EOT\n")));
			break;
		}
		
		// Verify.
		if (!RMS_VerifyMsg(gpRMSBuffer, nRecv))
		{
			NHDEBUG(1, (_T("<RMS_TCP_STATUS> RMS_VerifyMsg() Failed\n")));
			goto ErrorReturn;
		}
		
		// OK PASS
		RMS_SendControl(DATA_ACK);
		
		// 3.2 Processing RMS CMD
		if (!RMS_ParseCMD(gpRMSBuffer))
			break;
		
		Delay_Msg(5);

		// Close Connection.
		m_pDevCmn->fnNET_RMSConnectClose();
		
		// 2008-01-07 V01.02.24 SRC-14
		Delay_Msg(1000);
		
		NHDEBUG(1, (_T("\n-----CONTINUE-------------------------------\n")));
	}
	
	Delay_Msg(5);
	// 정상 종료. Close Connection.
	m_pDevCmn->fnNET_RMSConnectClose();
	NHDEBUG(1, (_T("\n----NORMAL---------------------------------\n")));
	
	return TRUE;

ErrorReturn:

	// 에러 종료. Close Connection.
	
	// Send EOT
	RMS_SendControl(DATA_EOT);
	Delay_Msg(5);
	m_pDevCmn->fnNET_RMSConnectClose();
	NHDEBUG(1, (_T("\n------ERROR-------------------------------\n")));

	return FALSE;
}

// [#2580] AU Kook 2018.11.07 RMS Test
/*-------------------------------------------------------------------
CLASS    NAME: CRmsCtrl
FUNCTION NAME: RMS_Status_Tcp_NotRecvRmsCmd()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : RMS Status Upload (TCP/IP) without receiving command.
-------------------------------------------------------------------*/
BOOL CRmsCtrl::RMS_Status_Tcp_NotRecvRmsCmd()
{
	BOOL bRet = FALSE;

	m_pDevCmn->fnNET_RMSConnectClose();
	Delay_Msg(1000);		// removing Delay() seems fine in test environment... but how about real world?

	NHDEBUG(1, (_T("\n//// RMS Journal Upload TCP/IP ///////////////////////////////\n")));
	// 1. Connect. when TCP/IP Mode, ConnectOpen is client mode.
	if (m_pDevCmn->fnNET_RMSConnectOpen(RMS_CONNECT) != RMS_OK)
	{
		NHDEBUG(1, (_T("<RMS_TCP_STATUS> RMSConnectOpen() Failed\n")));
		goto ErrorReturn;
	}

	// 2. Send Status.
	// use a message indicator of 'SCHEDULED JOURNAL UPLOAD' instead of 'STATUS UPLOAD'
	// to avoid receiving RMS commands, which are to be executed afterwards, from MV server.
	// (no journals will be uploaded via this function. STATUS ONLY)
	NHDEBUG(1, (_T("<RMS_TCP_STATUS> Send Status Data\n")));
	if (!RMS_SendATMStatus(RMS_SCHEDULED_JOURNAL_UPLOAD))
	{
		NHDEBUG(1, (_T("<RMS_TCP_STATUS> RMS_SendATMStatus(RMS_SCHEDULED_JOURNAL_UPLOAD) Failed\n")));
		goto ErrorReturn;
	}

	bRet = TRUE;
	Delay_Msg(1000);		// removing Delay() seems fine in test environment... but how about real world?

	// [#2580] AU Kook 2019.06.21 send all journals to update its new count on MV server. ('Last X' doesn't update count)
	//// 2018.11.27 send last 1 journal which is one of last transaction.	// 2019.01.23 customer request: increase count from 1 to 5. // 2019.02.20 5 -> 20
	//g_LastXCount = 20;	//5;	//1;
	//RMS_ProcJNLXUpload();
	RMS_ProcJNLUpload();
	// end of [#2580]

ErrorReturn:
	// Send EOT
	RMS_SendControl(DATA_EOT);
	Delay_Msg(5);
	m_pDevCmn->fnNET_RMSConnectClose();

	return bRet;
}
// end of [#2580]


// [#613] AU_C KSK 2010.01.21
/*-------------------------------------------------------------------
 CLASS    NAME: CRmsCtrl
 FUNCTION NAME: RMS_JournalUpload_Tcp()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : RMS Journal Upload 통신 절차를 수행한다. (Tcp/ip 용)
-------------------------------------------------------------------*/
BOOL CRmsCtrl::RMS_JournalUpload_Tcp()
{
	m_pDevCmn->fnNET_RMSConnectClose();
	Delay_Msg(1000);


	NHDEBUG(1, (_T("\n//// RMS Journal Upload TCP/IP ///////////////////////////////\n")));
	// 1. Connect. when TCP/IP Mode, ConnectOpen is client mode.
	if (m_pDevCmn->fnNET_RMSConnectOpen(RMS_CONNECT) != RMS_OK)
	{
		NHDEBUG(1, (_T("<RMS_TCP_STATUS> RMSConnectOpen() Failed\n")));
		goto ErrorReturn;
	}
	
	// 2. Send Status.
	NHDEBUG(1, (_T("<RMS_TCP_STATUS> Send Status Data\n")));
	if (!RMS_SendATMStatus(RMS_SCHEDULED_JOURNAL_UPLOAD))
	{
		NHDEBUG(1, (_T("<RMS_TCP_STATUS> RMS_SendATMStatus(RMS_SCHEDULED_JOURNAL_UPLOAD) Failed\n")));
		goto ErrorReturn;
	}
	
	Delay_Msg(1000);

	// 3. Journal Upload 수행
	RMS_ProcJNLUpload();

	// Send EOT
	RMS_SendControl(DATA_EOT);	
	Delay_Msg(5);
	// 정상 종료. Close Connection.
	m_pDevCmn->fnNET_RMSConnectClose();
	NHDEBUG(1, (_T("\n----NORMAL---------------------------------\n")));
	
	return TRUE;

ErrorReturn:

	// 에러 종료. Close Connection.
	// Send EOT
	RMS_SendControl(DATA_EOT);
	Delay_Msg(5);
	m_pDevCmn->fnNET_RMSConnectClose();
	NHDEBUG(1, (_T("\n------ERROR-------------------------------\n")));

	return FALSE;
}
// end of [#613]


BOOL CRmsCtrl::RMS_ParseCMD(BYTE *pBuffer)
{
	unsigned char   curMsgID;
	PRMS_CMD_COMM	pMSG;

	pMSG = (PRMS_CMD_COMM)pBuffer;

	NHDEBUG(1, (_T("Recv Command (%c) FROM RMS..\n"), (char)pMSG->Header.MsgID));

	curMsgID = pMSG->Header.MsgID;

	// Process CMD
	switch(curMsgID)
	{
	case RMS_REQ_SETUP:
		{
			NHDEBUG(1, (_T(" RECV CMD : RMS_REQ_SETUP\n")));
			
			// AIREAT_2007-10-10오후 11:41:01
			// When Broken NVRAM, it's that prevent infinte try to regist
			if (g_preMsgID == RMS_REQ_SETUP)
			{
				NHDEBUG(1, (_T("~!!!!! it's exact same to previous cmd. i'will disconnect with RMS\n")));
				// Send EOT
				RMS_SendControl(DATA_EOT);
				return FALSE;
			}
			// end of AIREAT_2007-10-10오후 11:41:01

			// Send Setup Response.
			RMS_ProcReqSetup();
			// Send EOT
			RMS_SendControl(DATA_EOT);
		}
		break;
		
	case RMS_SET_SETUP:
		{
			NHDEBUG(1, (_T(" RECV CMD : RMS_SET_SETUP\n")));
			if (RMS_ProcSetSetup())
			{
				// 적용 시키기 위해..
				MemSetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSCOMMAND, RMSCMD_REBOOT);
				return FALSE;
			}
		}
		break;

#if (RMS_VERSION >= 0x010302)
	#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
		// [#309] NH AIREAT 2008.06.17 - Change Password CMD 추가.
	case RMS_SET_CHANGE_PASSWORD:
		{
			PRMS_CMD_CHANGE_PASSWORD pChangePassWord = (PRMS_CMD_CHANGE_PASSWORD)pBuffer;
			char	Passowrd[7];
			CString	strMaster, strOperator, strService;
			
			// Get Master Password
			memset(Passowrd, 0, sizeof(Passowrd));
			strncpy(Passowrd, (const char*)pChangePassWord->MasterPW, 6);
			strMaster = Passowrd;

			// Get Manager Password
			memset(Passowrd, 0, sizeof(Passowrd));
			strncpy(Passowrd, (const char*)pChangePassWord->ManagerPW, 6);
			strOperator = Passowrd;

			// Get Service Password
			memset(Passowrd, 0, sizeof(Passowrd));
			strncpy(Passowrd, (const char*)pChangePassWord->ServicePW, 6);
			strService = Passowrd;

			// 6자리.
			if (strMaster.GetLength() == 6 && strOperator.GetLength() == 6 && strService.GetLength() == 6)
			{
				// 서로 같지 않아야 한다.
				if (strMaster != strOperator && strMaster != strService && strOperator != strService)
				{
					MemSetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MASTERPW, strMaster);
					MemSetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_OPERATORPW, strOperator);
					MemSetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SERVICEPW, strService);
				}
			}
			
			// Send EOT
			RMS_SendControl(DATA_EOT);
		}
		break;
		// end of [#309]
	#endif
#endif

// [#424] [NH] KSK 2008.9.18
#if (RMS_VERSION >= 0x010306)
	case RMS_REQ_ERR_TOTAL:
			// Send Setup Response.
			RMS_ProcReqErrTotal(FALSE);
			// Send EOT
			RMS_SendControl(DATA_EOT);		
		break;

	case RMS_REQ_ERR_TOTALNCLR:
			// Send Setup Response.
			RMS_ProcReqErrTotal(TRUE);
			// Send EOT
			RMS_SendControl(DATA_EOT);
		break;
#endif

// [#2115] [MX] KSK 2012.02.05
#if (RMS_VERSION >= 0x010308)
	#if (MX_VERSION)
	// [#2137] MX PCS 2012.07.17 "환율 주석 처리함."
// 	case RMS_SET_CHANGE_EXCHANGE_RATE:
// 		{
// 			PRMS_CMD_CHANGE_EXCHANGERATE pChangeExchangeRate = (PRMS_CMD_CHANGE_EXCHANGERATE)pBuffer;
// 			char	ExchangeRate[13];
// 			CString	strExchangeRate;
// 			
// 			memset(ExchangeRate, 0, sizeof(ExchangeRate));
// 			strncpy(ExchangeRate, (const char*)pChangeExchangeRate->ExchangeRate, 12);
// 			strExchangeRate = ExchangeRate;
// 
// 			if ((atof(ExchangeRate) > 0) && (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE))
// 			{
// 
// 				CString strOldExchangeValue;
// 				strOldExchangeValue.Format(L"%s", MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EXCHANGE_RATE));
// 
// 				MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EXCHANGE_RATE, strExchangeRate);
// 				CString strTemp;
// 				strTemp.Format(L"OLD : %s, NEW : %s", strOldExchangeValue, strExchangeRate);
// 
// 				m_pDevCmn->m_JNLMgr.Save(CHANGE_EXCHANGE_RATE, strTemp);
// 			}
// 
// 			// Send EOT
// 			RMS_SendControl(DATA_EOT);
// 		}
// 		break;
	#endif
#endif
// end of [#2115]

	case RMS_REQ_INITALLDEV:
		{
			NHDEBUG(1, (_T(" RECV CMD : RMS_REQ_INITALLDEV\n")));
			MemSetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSCOMMAND, RMSCMD_INITALLDEV);		
			// Send EOT
			RMS_SendControl(DATA_EOT);
			return FALSE;	// 다시 상태 전송 하지 않기 위해.
		}
		break;
	case RMS_REQ_INITERRDEV:
		{
			NHDEBUG(1, (_T(" RECV CMD : RMS_REQ_INITERRDEV\n")));
			MemSetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSCOMMAND, RMSCMD_INITERRDEV);
			// Send EOT
			RMS_SendControl(DATA_EOT);
			return FALSE;	// 다시 상태 전송 하지 않기 위해.
		}
		break;
		
	case RMS_REQ_REBOOT_FORCE:
	case RMS_REQ_REBBOT_NONTX:
		{
			NHDEBUG(1, (_T(" RECV CMD : RMS_REQ_REBBOT_NONTX or RMS_REQ_REBOOT_FORCE\n")));
			MemSetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSCOMMAND, RMSCMD_REBOOT);
			// Send EOT
			RMS_SendControl(DATA_EOT);
			return FALSE;	// 다시 상태 전송 하지 않기 위해.
		}
		break;

	case RMS_REQ_FILE_DOWN:
		{
			NHDEBUG(1, (_T(" RECV CMD : RMS_REQ_FILE_DOWN\n")));
			if (RMS_ProcFileDownLoad(pBuffer))
			{
				MemSetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSCOMMAND, RMSCMD_SWUPDATE);
				return FALSE;	// 다시 상태 전송 하지 않기 위해.
			}
			// [#222] CSW 2008.5.21
			// RMS Download 실패후 ATM REBOOT함.(3M이상 Download시 실패함)
			NHDEBUG(1, (_T(" RECV CMD : RMS_REQ_REBBOT_NONTX or RMS_REQ_REBOOT_FORCE\n")));
			MemSetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSCOMMAND, RMSCMD_REBOOT);
			// Send EOT
			RMS_SendControl(DATA_EOT);
			return FALSE;	// 다시 상태 전송 하지 않기 위해.
			// end of [#222]
		}
		break;

#if (RMS_VERSION >= 0x010309)	// [#523] US KSK 2009.05.19 File Upload 공통화
	case RMS_REQ_FILE_UPLOAD:
		{
			NHDEBUG(1, (_T(" RECV CMD : RMS_REQ_FILE_UPLOAD\n")));
			RMS_ProcFileUpLoad(pBuffer);

			// Send EOT
			RMS_SendControl(DATA_EOT);
			NHDEBUG(1, (_T(" RECV CMD : RMS_REQ_FILE_UPLOAD END\n")));
			return FALSE;	// 다시 상태 전송 하지 않기 위해.
			// end of [#222]
		}
		break;
#endif							// end of [#523]

	case RMS_REQ_JNL:
		{
			NHDEBUG(1, (_T(" RECV CMD : RMS_REQ_JNL\n")));
			RMS_ProcJNLUpload();

			// Send EOT
			RMS_SendControl(DATA_EOT);
		}
		break;

	case RMS_REQ_X_JNL:
		{
			NHDEBUG(1, (_T(" RECV CMD : RMS_REQ_X_JNL\n")));
			PRMS_CMD_LAST_X_JNR pXJournal = (PRMS_CMD_LAST_X_JNR)pBuffer;
			
			g_LastXCount = pXJournal->JournalCount;
			g_SentXCount = 0;
			
			RMS_ProcJNLXUpload();

			// Send EOT
			RMS_SendControl(DATA_EOT);
		}
		break;

#if (RMS_VERSION >= 0x010309)	// [#524] US KSK 2009.05.19 Journal By Date 기능 추가
	case RMS_REQ_DATE_JNL:
		{
			NHDEBUG(1, (_T(" RECV CMD : RMS_REQ_DATE_JNL\n")));
			RMS_ProcJNLDateUpload(pBuffer);
			
			// Send EOT
			RMS_SendControl(DATA_EOT);
		}
		break;
#endif							// end of [#524]

	case RMS_REQ_STATUS:
		{
			NHDEBUG(1, (_T(" RECV CMD : RMS_REQ_STATUS\n")));
			// SEND Status Response
			// Status tcp는 연속 CMD때문에 1로 보냄.
			if (g_LineType == RMS_LINE_TCP && g_OpenType == RMS_CONNECT)
				RMS_SendATMStatus(RMS_REQ_STATUS, 1);
			else
				RMS_SendATMStatus(RMS_REQ_STATUS, 0);

			// Send EOT
			RMS_SendControl(DATA_EOT);
		}
		break;

#if (RMS_VERSION >= 0x010310)
	// [#530] NH KSK 2009.06.03
	case RMS_SET_ADDCASH:
		{
			NHDEBUG(1, (_T(" RECV CMD : RMS_SET_ADDCASH\n")));

			memset(&m_AddCashInfo, 0, sizeof(m_AddCashInfo));	// Buffer Initialize
			memcpy(&m_AddCashInfo, pBuffer, sizeof(m_AddCashInfo));

			MemSetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSCOMMAND, RMSCMD_SETADDCASH);
			
			// Send EOT
			RMS_SendControl(DATA_EOT);
		}
		break;
	// end of [#530]

	// [#531] NH KSK 2009.06.03
	case RMS_EXEC_CSTTOTAL:
		{
			NHDEBUG(1, (_T(" RECV CMD : RMS_EXEC_CSTTOTAL\n")));
			MemSetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSCOMMAND, RMSCMD_EXECCSTTOTAL);

			// Send EOT
			RMS_SendControl(DATA_EOT);
		}
		break;
	// end of [#531]

	// [#532] NH KSK 2009.06.03
	case RMS_EXEC_TRIALDAYTOTAL:
		{
			NHDEBUG(1, (_T(" RECV CMD : RMS_EXEC_TRIALDAYTOTAL\n")));
			MemSetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSCOMMAND, RMSCMD_EXECTRIALDAYTOTAL);
			
			// Send EOT
			RMS_SendControl(DATA_EOT);
		}
		break;

	case RMS_EXEC_DAYTOTAL:
		{
			NHDEBUG(1, (_T(" RECV CMD : RMS_EXEC_DAYTOTAL\n")));
			MemSetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSCOMMAND, RMSCMD_EXECDAYTOTAL);
			
			// Send EOT
			RMS_SendControl(DATA_EOT);
		}
		break;
	// end of [#532]

	// [#533] NH KSK 2009.06.05
	case RMS_REQ_REJECTTOTAL:
		{
			// Send Setup Response.
			RMS_ProcReqRejectTotal(FALSE);
			// Send EOT
			RMS_SendControl(DATA_EOT);
		}
		break;

	case RMS_REQ_REJECTTOTALNCLR:
		{
			// Send Setup Response.
			RMS_ProcReqRejectTotal(TRUE);
			// Send EOT
			RMS_SendControl(DATA_EOT);
		}
		break;
	// end of [#533]
#endif

//[#546] SOOK 2009.07.23 다기능 BIN LIST 기능 추가함
#if (RMS_VERSION >= 0x010311)

	#if (AU_VERSION)
	case RMS_SET_BINLIST:
		{
			NHDEBUG(1, (_T(" RECV CMD : RMS_SET_ADDTIONAL_SETUP\n")));
			if (RMS_ProcSetBinList() == FALSE)
			{
				NHDEBUG(1, (_T("Setup ID doesn't support")));
			}
		}
		break;
	case RMS_REQ_BINLIST:
		{
			NHDEBUG(1, (_T(" RECV CMD : RMS_REQ_ADDTIONAL_SETUP\n")));

			// Send Setup Response.
			RMS_ProcReqBinList();

			// Send EOT
			RMS_SendControl(DATA_EOT);
		}
		break;
	#endif
#endif
//end of [#546]

//[#611] SOOK 2010.01.16 Change ATM Time추가 
#if (RMS_VERSION >= 0x010313)
	case RMS_SET_DATETIME:
		{
			NHDEBUG(1, (_T(" RECV CMD : RMS_SET_DATETIME\n")));

			memset(&m_DateTimeInfo, 0, sizeof(m_DateTimeInfo));	// Buffer Initialize
			memcpy(&m_DateTimeInfo, pBuffer, sizeof(m_DateTimeInfo));

			MemSetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSCOMMAND, RMSCMD_CHANGEDATETIME);
	
			// Send EOT
			RMS_SendControl(DATA_EOT);
		}
		break;
#endif
//end of [#611]

// [#620] US KSK 2010.02.22
#if (RMS_VERSION >= 0x010315)
	#if (US_VERSION || CA_VERSION || MX_VERSION)	// [#2115] MX KSK 2012.02.05
	case RMS_ENHANCED_COUPON_DOWNLOAD:
		NHDEBUG(1, (_T(" RECV CMD : RMS_ENHANCED_COUPON_DOWNLOAD\n")));
		
		// Send Setup Response.
		RMS_ProcSetEnhancedCoupon();
		// Send EOT
		RMS_SendControl(DATA_EOT);
		break;

	case RMS_ENHANCED_COUPON_UPLOAD:
		NHDEBUG(1, (_T(" RECV CMD : RMS_ENHANCED_COUPON_UPLOAD\n")));

		RMS_ProcReqEnhancedCoupon();
		// Send EOT
		RMS_SendControl(DATA_EOT);
		break;
	#endif
#endif
// end of [#620]

//[#586] SOOK 2009.12.04 iTM MoniView 연동 부분 추가 
	case RMS_REQ_JNL_IMAGE:
		{
			NHDEBUG(1, (_T(" RECV CMD : RMS_REQ_JNL_IMAGE\n")));
			RMS_ProcUploadJNLImage(pBuffer);

			// Send EOT
			RMS_SendControl(DATA_EOT);
			NHDEBUG(1, (_T(" RECV CMD : RMS_REQ_JNL_IMAGE END\n")));
		}
		break;
	case RMS_SET_WEATHERE_INFO:
		{
//			NHDEBUG(1, (_T(" RECV CMD : RMS_SET_WEATHERE_INFO\n")));
//			RMS_ProcSetWeatherInfo();
//			NHDEBUG(1, (_T(" RECV CMD : RMS_SET_WEATHERE_INFO END\n")));

			// [#11] NH KSK 2010.09.14 1800SE는 Weather Service 미 지원
			NH_OS_VERSION	eOSVersion = m_pDevCmn->m_pConfig->GetOSVersion();
//			if (eOSVersion != NH_OS_NH1800SE_B)					// [#11] NH KSK 2010.09.14 Weather Service 미 지원
			if (!(eOSVersion == NH_OS_NH1800SE_B || eOSVersion == NH_OS_NH1500SE))					// [#2267] US KSK 2014.05.08
			{
				NHDEBUG(1, (_T(" RECV CMD : RMS_SET_WEATHERE_INFO\n")));
				RMS_ProcSetWeatherInfo();
				NHDEBUG(1, (_T(" RECV CMD : RMS_SET_WEATHERE_INFO END\n")));
			}
			// end of [#11]
		}
		break;
//end of [#586] 

#if (RMS_VERSION >= 0x010321)

	#if (US_VERSION)
		#ifdef PUBLIC_PASSWORD_CHANGE_NO_VERIF // Enable the non-verified password change job
			// [#2276] NH Justin 2014.06.10 Implement Change Password
			case RMS_CHANGE_RETAIL_PASSWORDS:
				{
					PRMS_CMD_CHANGE_RETAIL_PASSWORD pChangePassWord = (PRMS_CMD_CHANGE_RETAIL_PASSWORD) pBuffer;
					char	Passowrd[7];
					CString	strMaster, strOperator, strService, strRMS;
					
					// Get Master Password
					memset(Passowrd, 0, sizeof(Passowrd));
					strncpy(Passowrd, (const char*)pChangePassWord->MasterPW, 6);
					strMaster = Passowrd;

					// Get Service Password
					memset(Passowrd, 0, sizeof(Passowrd));
					strncpy(Passowrd, (const char*)pChangePassWord->ServicePW, 6);
					strService = Passowrd;

					// Get Operator Password
					memset(Passowrd, 0, sizeof(Passowrd));
					strncpy(Passowrd, (const char*)pChangePassWord->OperatorPW, 6);
					strOperator = Passowrd;

					// Get RMS Password
					memset(Passowrd, 0, sizeof(Passowrd));
					strncpy(Passowrd, (const char*)pChangePassWord->RMSPW, 6);
					strRMS = Passowrd;

					// 6자리.
					if (strMaster.GetLength() == 6 && strService.GetLength() == 6 && strOperator.GetLength() == 6 && strRMS.GetLength() == 6)
					{
						// 서로 같지 않아야 한다.
						if (strMaster != strOperator && strMaster != strService && strOperator != strService)
						{
							// Set Passwords
							CLoginManager manager;
							manager.UpdatePassword(Master, strMaster);
							manager.UpdatePassword(Service, strService);
							manager.UpdatePassword(Operator, strOperator);

							// Set RMS Password
							MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSPASSWORD, strRMS);
						}
					}			
					// Send EOT
					RMS_SendControl(DATA_EOT);
				}
				break;
			// End of [#2276] 
		#endif

	//[#2277] NH Justin 2014.06.10 Support In/Out of service
	case RMS_SET_ATM_STATUS:
		{
			PRMS_CMD_SET_ATM_STATUS pNewATMStataus = (PRMS_CMD_SET_ATM_STATUS) pBuffer;

			if(pNewATMStataus->ATMStatus == 0x00)		// Clear Buffer - In Service
				m_pDevCmn->m_nRemoteStatusCmd = 0;
			else if(pNewATMStataus->ATMStatus == 0x01)	// Set Flag - Out of Service 
				m_pDevCmn->m_nRemoteStatusCmd = 1;
			// Send EOT
			RMS_SendControl(DATA_EOT);
		}
		break;
	// End of [#2277]
	#endif

#endif

// [#2283] NH Justin Customized Hot Key
#if (RMS_VERSION >= 0x010322)
	#if (US_VERSION || CA_VERSION || MX_VERSION)
	case RMS_SET_HOT_KEY:
		{
			PRMS_CMD_SET_HOT_KEY pHotKey = (PRMS_CMD_SET_HOT_KEY) pBuffer;
			char	szHotKey[43];

			memset(szHotKey, 0, sizeof(szHotKey));
			strncpy(szHotKey, (const char*)pHotKey->HotKeyString, 42);
			CString strNewHotKey = szHotKey;
			strNewHotKey.TrimLeft();
			strNewHotKey.TrimRight();
			strNewHotKey.MakeUpper();
			if( m_pDevCmn->IsValidHotKey(strNewHotKey) )
			{
				MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HOT_KEY_STRING, strNewHotKey );
				m_pDevCmn->m_strStoredHotKey = strNewHotKey;
			}
			RMS_SendControl(DATA_EOT);
		}
		break;
	#endif
#endif
// End of [#2283]

// [#2342] NH Justin 2015.05.11 AID Selection
#if (RMS_VERSION >= 0x010326)
	case RMS_SET_EMV_AID:
		{
			CStringArray arrAIDGroup;
			// strAIDStatus.Format( L"%S", &pBuffer[ sizeof(RMS_CMD_SET_EMV_AID) ]); ====>  not working.. Truncated after 1K(1024 byte)
	
			CString strAIDSep1 = AID_FIELD_DELIMITER;		
			CString strTemp;
			BYTE* pRcvData = &pBuffer[sizeof(RMS_CMD_SET_EMV_AID)];
			BOOL bFinish = FALSE;
			strTemp = _T("");
			long nBufLoc = 0;
			do
			{	
				if(pRcvData[nBufLoc]>0 ) 
				{
					if(pRcvData[nBufLoc] == AID_FIELD_DELIMITER2)
					{
						if(strTemp.GetLength()>0)
						{
							NHDEBUG(1, (_T(" AID GROUP ADDED = [%s]\n"), strTemp ));
							arrAIDGroup.Add(strTemp);
						}
						strTemp.Empty();
					}
					else 
						strTemp += CString( (char) pRcvData[nBufLoc]);

					nBufLoc++;
					if (nBufLoc>=RMS_DATA_BUFSIZE)
						bFinish = TRUE;	
				}
				else
				{
					if(strTemp.GetLength()>0)
					{
						NHDEBUG(1, (_T(" AID GROUP ADDED = [%s]\n"), strTemp ));
						arrAIDGroup.Add(strTemp);
					}
					bFinish = TRUE;
				}
			} while (bFinish==FALSE);

			CString strUpdateOption;
			BOOL bValid = FALSE;
			BOOL bUpdateAIDPool = FALSE;
			BOOL bRebootATM = FALSE;
			int  nKernelVersion = m_pDevCmn->fnEMV_GetKernelVersion();
			int	 nCommonAIDOption = COMMONAID_USE_COMMON;

			// check options.
			if( arrAIDGroup.GetSize() > 1)
			{
				CStringArray arrOptions;
				strTemp = arrAIDGroup.GetAt(0);			
				SplitString(strTemp, strAIDSep1, arrOptions);
				if(arrOptions.GetSize() >= 3 )
				{
					NHDEBUG(1, (_T(" Update Option  = [%s]\n"), arrOptions.GetAt(0) ));
					NHDEBUG(1, (_T(" Kernel Version = [%s]\n"), arrOptions.GetAt(1) ));
					NHDEBUG(1, (_T(" USE Common AID = [%s]\n"), arrOptions.GetAt(2) ));

					// Check Update Option and Kernel Version 
					// Update Option
					//	"1" : Update All - Kernel Version, AID List and Termdata, AID En/Disable Status
					//	"2" : Update     -                 AID List and Termdata, AID En/Disable Status
					//	"3" : Update     -                                        AID En/Disable Status
					//	"4" : Update     - Kernel Version,                        AID En/Disable Status
					//  "5" : Disable EMV + "1" 
					//  "6" : Disable EMV + "2" 
					//  "7" : Disable EMV + "3" 
					//  "8" : Disable EMV + "4" 
					//  "9" : Enable  EMV + "1" 
					//  "A" : Enable  EMV + "2" 
					//  "B" : Enable  EMV + "3" 
					//  "C" : Enable  EMV + "4" 

					strUpdateOption = arrOptions.GetAt(0);

					// [#2423] US Justin Support En/Disable EMV through individual command
					#if (RMS_VERSION >= 0x010328)
						int nEnDisableEMV = -1;
						if( (strUpdateOption.Compare(L"5")==0) || (strUpdateOption.Compare(L"9")==0) )
						{
							if(strUpdateOption.Compare(L"5")==0)	nEnDisableEMV = 0;
							else									nEnDisableEMV = 1;
							strUpdateOption = L"1";
						}
						else if( (strUpdateOption.Compare(L"6")==0) || (strUpdateOption.Compare(L"A")==0) )
						{
							if(strUpdateOption.Compare(L"6")==0)	nEnDisableEMV = 0;
							else									nEnDisableEMV = 1;
							strUpdateOption = L"2";
						}
						else if( (strUpdateOption.Compare(L"7")==0) || (strUpdateOption.Compare(L"B")==0) )
						{
							if(strUpdateOption.Compare(L"7")==0)	nEnDisableEMV = 0;
							else									nEnDisableEMV = 1;
							strUpdateOption = L"3";
						}
						else if( (strUpdateOption.Compare(L"8")==0) || (strUpdateOption.Compare(L"C")==0) )
						{
							if(strUpdateOption.Compare(L"8")==0)	nEnDisableEMV = 0;
							else									nEnDisableEMV = 1;
							strUpdateOption = L"4";
						}

						if(nEnDisableEMV==0)			// Disable EMV....
							MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_EMV_ENABLE, 0);
						else if(nEnDisableEMV==1)		// Enable  EMV....
						{
							if( MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_EMV_AVAILABLE) == ENABLE)
								MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_EMV_ENABLE, ENABLE);
						}
					#endif
					// End of [#2423]

					// [#2423] US Justin Support En/Disable EMV through individual command
					#if (RMS_VERSION >= 0x010328)
						// [#2435] NH KSK 2016.07.14 MoniView - ATM간 동기화가 필요함 Remote로 V6 Setting시 MoniView Agent Version을 올리고 대응해야 하는지 확인 필요
						// ATM이 V6까지지원하고 MoniView가 V5만 지원하면 문제 없음
						// ATM이 V5만 지원하고  MoniView가 V6 지원시 Setting 안되는 문제 발생 (NHA 검토 필요)
						//     ======> [NHA] 2016.07.14 :   Moniview will allow chaning "Kernel Version"(Command "1" and "4") from "0x010329"
						// 1 : Update - Kernel version, AID List and Contents, AID En/Disable   
						// 4 : Update - Kernel version,                        AID En/Disable   
						if( (strUpdateOption.Compare(L"1")==0) || (strUpdateOption.Compare(L"4")==0) )		
						{
							// Change Kernel version
							#if (CA_VERSION)															
								nKernelVersion = Asc2Int( arrOptions.GetAt(1));
								//if( (nKernelVersion!=EMV_KERNEL_V4)&&(nKernelVersion!=EMV_KERNEL_V5) )	// Invalid Value => Assign Current Value
								if( (nKernelVersion!=EMV_KERNEL_V4)&&(nKernelVersion!=EMV_KERNEL_V5)&&(nKernelVersion!=EMV_KERNEL_V6) )		// Invalid Value => Assign Current Value.. [#2435] Support Kernel V6
									nKernelVersion = m_pDevCmn->fnEMV_GetKernelVersion();

								if( nKernelVersion != m_pDevCmn->fnEMV_GetKernelVersion())					// If received Kernel is different from the current one
									bRebootATM = TRUE;
							//#elif (US_VERSION)															
							#elif ( (US_VERSION)||(MX_VERSION)||(AU_VERSION) )	// KSK 2016.10.13 Support AU country															
								nKernelVersion = Asc2Int( arrOptions.GetAt(1));
								//if( nKernelVersion!=EMV_KERNEL_V5 )										// Invalid Value => Assign Current Value
								if( (nKernelVersion!=EMV_KERNEL_V5)&&(nKernelVersion!=EMV_KERNEL_V6) )		// Invalid Value => Assign Current Value.. [#2435] Support Kernel V6
									nKernelVersion = m_pDevCmn->fnEMV_GetKernelVersion();

								if( nKernelVersion != m_pDevCmn->fnEMV_GetKernelVersion())					// If received Kernel is different from the current one
									bRebootATM = TRUE;
							#endif
							bValid = TRUE;
						} 
					#else
						// Update Kernel Version
						if( strUpdateOption.Compare(L"1")==0 )			// Update Kernel version, AID List and Contents, AID En/Disable
						{
							// Kernel version Change : CA ONLY, Other Countries : Ignore this option as of May, 2015
							#if (CA_VERSION)															
							nKernelVersion = Asc2Int( arrOptions.GetAt(1));
							if( (nKernelVersion!=EMV_KERNEL_V4)&&(nKernelVersion!=EMV_KERNEL_V5) )	// Invalid Value => Assign Current Value
								nKernelVersion = m_pDevCmn->fnEMV_GetKernelVersion();

							if( nKernelVersion != m_pDevCmn->fnEMV_GetKernelVersion())					// If received Kernel is different from the current one
								bRebootATM = TRUE;
							#endif
							bValid = TRUE;
						} 
					#endif		// End of [#2423]
						else if( (strUpdateOption.Compare(L"2")==0)||(strUpdateOption.Compare(L"3")==0)	)	// 2 :Update AID List and Contents, AID En/Disable,   3: AID En/Disable ONLY
							bValid = TRUE;

					// Check "Use Common AID" Option.				// Ignore this option except US : May, 2015
					#if (US_VERSION)															
					if(bValid)
					{
						// Save this option later with other parameters.
						nCommonAIDOption = Asc2Int(arrOptions.GetAt(2));
						if( (nCommonAIDOption!=COMMONAID_USE_COMMON)&&
							(nCommonAIDOption!=COMMONAID_USE_ALL)&&
							(nCommonAIDOption!=COMMONAID_USE_INTERNATIONAL) )
							bValid = FALSE;
					}
					#endif
				}				
			}

			// Update AID List ===> Option "1" or "2"
			FILE	*Stream;
			DWORD	dwFileWriten = 0;
			char	buf[512], bufContents[512];
			if( (bValid == TRUE) && ( (strUpdateOption.Compare(L"1")==0)||(strUpdateOption.Compare(L"2")==0) )	)
			{
				strTemp.Format(L"%S", GET_EMV_AID_POOLFILENAME(nKernelVersion) );
				DeleteFile(strTemp);
				if ( (Stream = _tfopen(strTemp, _T("wb+"))) == NULL)
				{
					NHDEBUG(1, (_T(" FAIL TO CREATE EMV POOL DATA = [%s]\n"), strTemp));
					bValid = FALSE;
				}
				else
				{
					// section  [AIDCOUNT]
					dwFileWriten = fwrite("[AIDCOUNT]\r\n", sizeof(char), strlen("[AIDCOUNT]\r\n"), Stream);
					memset(buf, 0, sizeof(buf));
					sprintf(buf, "TOTALCOUNT=%d\r\n\r\n", arrAIDGroup.GetSize() - 1 );
					dwFileWriten = fwrite(buf, sizeof(char), strlen(buf), Stream);
					NHDEBUG(1, (_T("Total AID (pool file) = [%d]\n"), arrAIDGroup.GetSize()-1 ));
					bUpdateAIDPool = TRUE;
				}
			}

			int nReceivedAID = 0;
			if( bValid == TRUE )
			{
				// En/Disable AID
				CStringArray arrAIDContent;
				CString strAIDName, strReceivedAID, strTermData, strAIDEnStatus, strEnabledAIDList;
				strEnabledAIDList = _T("");
				for(int nRcvAID=1; nRcvAID<arrAIDGroup.GetSize(); nRcvAID++)
				{
					arrAIDContent.RemoveAll();
					strTemp = arrAIDGroup.GetAt(nRcvAID);
					SplitString(strTemp, strAIDSep1, arrAIDContent);
					if( arrAIDContent.GetSize() == 4)
					{
						strAIDName		= arrAIDContent.GetAt(0);
						strReceivedAID	= arrAIDContent.GetAt(1); 
						strTermData		= arrAIDContent.GetAt(2);
						strAIDEnStatus	= arrAIDContent.GetAt(3);

						if( (strAIDName.GetLength()>0)		&& (strReceivedAID.GetLength()>=10)&&
							(strTermData.GetLength()==42)	&& ( (strAIDEnStatus.Compare(L"0")==0)||(strAIDEnStatus.Compare(L"1")==0) ) )
						{
							// Received AID is Valid
							nReceivedAID++;

							// Update POOL Data
							if( bUpdateAIDPool)
							{
								// session : [TERMINALX]
								memset(buf, 0, sizeof(buf));
								sprintf(buf, "[TERMINAL%d]\r\n", nReceivedAID);
								dwFileWriten = fwrite (buf, sizeof(char), strlen(buf), Stream);

								// application name : APPLICATIONNAME=VISA, APPLICATIONNAME=MASTER
								memset(bufContents, 0, sizeof(bufContents));	memset(buf, 0, sizeof(buf));
								RMS_WideToMulti(strAIDName, (char*)bufContents, sizeof(bufContents));
								sprintf(buf, "APPLICATIONNAME=%s\r\n", bufContents);
								dwFileWriten = fwrite (buf, sizeof(char), strlen(buf), Stream);

								// AID : AID=A0000002771010
								memset(bufContents, 0, sizeof(bufContents));	memset(buf, 0, sizeof(buf));
								RMS_WideToMulti(strReceivedAID, (char*)bufContents, sizeof(bufContents));
								sprintf(buf, "AID=%s\r\n", bufContents);
								dwFileWriten = fwrite (buf, sizeof(char), strlen(buf), Stream);

								// Terminal Data : TERMINALDATA=D9050000000000DA05FFFFFFFFFFD805FFFFFFFFFF
								memset(bufContents, 0, sizeof(bufContents));	memset(buf, 0, sizeof(buf));
								RMS_WideToMulti(strTermData, (char*)bufContents, sizeof(bufContents));
								if( nKernelVersion == EMV_KERNEL_V4 )
								{
									char chAPVer[5] = {0,};	
									m_pDevCmn->Get_AID_AP_VersionNumber(strReceivedAID, chAPVer, sizeof(chAPVer)); 
									sprintf(buf, "TERMINALDATA=%s%s%4.4s%s\r\n\r\n", bufContents, TAG_EMV_APVERSION, chAPVer, GET_EMV_DEFAULT_TERMINALDATA(nKernelVersion));
								}
								else
									sprintf(buf, "TERMINALDATA=%s\r\n\r\n", bufContents);
								dwFileWriten = fwrite (buf, sizeof(char), strlen(buf), Stream);
							}

							// Update Enabled AID List
							if( (strReceivedAID.GetLength()>=10) && (strAIDEnStatus.Compare(L"1")==0 ) )
							{
								strEnabledAIDList += strReceivedAID;
								strEnabledAIDList += AID_FIELD_DELIMITER;
							}
						}
					}
				}

				if( bUpdateAIDPool)
				{
					fflush(Stream);
					fclose(Stream);
				}
				NHDEBUG(1, (_T("Enabled AID = [%s]\n"),strEnabledAIDList ));

				// En/Disable US COMMON AID => US ONLY
				#if (US_VERSION)															
				MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_USE_USCOMMONAID, nCommonAIDOption);
				#endif
				
				// If Kernel version is changed
				if(bRebootATM==TRUE)			
				{
					NHDEBUG(1, (_T("Kernel Changed ==> REBOOT....\n")));
					MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EMV_KERNELVERSION, nKernelVersion);
					MemSetStr(_MEM_FLD_EMVCONFIG, _MEM_VAR_EMV_CONFIG_ENABLEDAID, strEnabledAIDList);
					MemSetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSCOMMAND, RMSCMD_REBOOT);
				}
				else
				{
					CString strCurEnabledAIDList = MemGetStr(_MEM_FLD_EMVCONFIG, _MEM_VAR_EMV_CONFIG_ENABLEDAID);
					if(strCurEnabledAIDList.CompareNoCase(strEnabledAIDList) != 0 )
					{
						NHDEBUG(1, (_T("Creating TERM_DATA_FILE Again \n")));
						MemSetStr(_MEM_FLD_EMVCONFIG, _MEM_VAR_EMV_CONFIG_ENABLEDAID, strEnabledAIDList);
						m_pDevCmn->Create_EMV_Termdata_From_POOL();
					}
				}
			}
			RMS_SendControl(DATA_EOT);
		}
		break;
#endif
// End of [#2342]

// [#RWC6-56] US William Hashed Password
#if (RMS_VERSION >= 0x010343)
	case RMS_RESET_PASSWORD_HASHED:
		{
			RMS_HASHEDPASSWORDCHANGE job;
			unsigned int offset = sizeof(RMS_CMD_COMM);
			unsigned int currLen = 0;
			char *tempPassword;

			//
			// Parse
			//

			// Old Master Password
			currLen = strlen((char *)(pBuffer + offset)) + 1;
			job.OldMasterPassword = new char[currLen]();
			strcpy(job.OldMasterPassword, (char *)(pBuffer + offset));
			offset += currLen;

			// New Master Password
			currLen = strlen((char *)(pBuffer + offset)) + 1;
			job.NewMasterPassword = new char[currLen]();
			strcpy(job.NewMasterPassword, (char *)(pBuffer + offset));
			offset += currLen;

			// New Operator Password
			currLen = strlen((char *)(pBuffer + offset)) + 1;
			job.NewOperatorPassword = new char[currLen]();
			strcpy(job.NewOperatorPassword, (char *)(pBuffer + offset));
			offset += currLen;

			// New Service Password
			currLen = strlen((char *)(pBuffer + offset)) + 1;
			job.NewServicePassword = new char[currLen]();
			strcpy(job.NewServicePassword, (char *)(pBuffer + offset));
			offset += currLen;

			NHDBG((L"Read %d bytes for the password reset job\r\n", offset));

			// Change Passwords

			CLoginManager manager;
			for (int i = IDENTITY_MAX - 1; i >= 0; i--)
			{
				tempPassword = job.PasswordForIdentity((Identity)i);
				if (tempPassword == NULL)
				{
					NHERROR((L"Identity overflow?!\r\n"));
					break;
				}

				if (strlen(tempPassword) == 0)
				{
					NHDBG((L"Blank password indicates no change\r\n"));
					continue;
				}

// If the following macro is defined, the build supports non-verified password changes. See LoginManager.h > PUBLIC_PASSWORD_CHANGE_NO_VERIF
#ifdef PUBLIC_PASSWORD_CHANGE_NO_VERIF
				if (!manager.UpdatePassword((Identity) i, tempPassword))
#else
				if (!manager.ChangePassword((Identity) i, CString(job.OldMasterPassword), CString(tempPassword)))
#endif
				{
					NHERROR((L"Failed to change passwords\r\n"));
				}
			}

			// Cleanup
			memset(job.OldMasterPassword, 0, strlen(job.OldMasterPassword) + 1);
			memset(job.NewMasterPassword, 0, strlen(job.NewMasterPassword) + 1);
			memset(job.NewOperatorPassword, 0, strlen(job.NewOperatorPassword) + 1);
			memset(job.NewServicePassword, 0, strlen(job.NewServicePassword) + 1);
			delete [] job.OldMasterPassword;
			delete [] job.NewMasterPassword;
			delete [] job.NewOperatorPassword;
			delete [] job.NewServicePassword;

			RMS_SendControl(DATA_EOT);

			break;
		}

#if (US_VERSION)
#if (APP_JUST_CASH)
// [#RWC6-119] US William 2020.01.16 Add JustCash RMS changes
	case RMS_JUSTCASH_REGISTRATION:
		NHINFO((L"Recieved JustCash Registration Job\r\n"));

		if (m_pTranCmn->P_NH_JUSTCASH_RegisterService() == RES_OK)
		{
			NHINFO((L"JustCash registration job completed successfully\r\n"));
		}
		else
		{
			NHERROR((L"JustCash registration job failed\r\n"));
		}

		RMS_SendControl(DATA_EOT);

		break;
#endif
#endif
#endif
// end of [#RWC6-119]

// [#RWC6-134] US William 2020.03.24 RMS Remote Updates
#if (RMS_VERSION >= 0x010345)
	case RMS_INSTALL_REMOTE_UPDATE:
		{
			NHDEBUG(1, (_T(" RECV CMD : RMS_INSTALL_REMOTE_UPDATE\n")));
			RMS_ProcRemoteUpdate(pBuffer);

			// Send EOT
			RMS_SendControl(DATA_EOT);
			NHDEBUG(1, (_T(" RECV CMD : RMS_INSTALL_REMOTE_UPDATE END\n")));
		}
		break;
#endif
// End of [#RWC6-134]

	// [#RWC6-208] US William 2020.08.05 Add LTX change password job
#if (RMS_VERSION >= 0x010347)
	case RMS_CHANGE_LTX_PASSWORD:
		{
			NHDBG((L" RECV CMD : RMS_CHANGE_LTX_PASSWORD\r\n"));
			
			PRMS_CMD_CHANGELTXPASSWORD changePasswordJob = (PRMS_CMD_CHANGELTXPASSWORD) pBuffer;
			changePasswordJob->ChangePassword();

			RMS_SendControl(DATA_EOT);
			NHDBG((L" RECV CMD : RMS_CHANGE_LTX_PASSWORD END\r\n"));
		}	
		break;
#endif

	// [#RWC6-533] US ryan.payton 2023.06.01 [PAI] RMS MCCP Key Reset
#if (RMS_VERSION >= 0x010353)
	case RMS_RESET_MCCP_SYMMETRICAL_KEY:
	{
		NHDBG((L" RECV CMD : RMS_RESET_MCCP_SYMMETRICAL_KEY\r\n"));
			
		// Reset MCCP Symmetrical Keys
		m_pTranCmn->m_Pin4.RemoveDownloadCredentials();

		// Reboot ATM
		MemSetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSCOMMAND, RMSCMD_REBOOT);

		RMS_SendControl(DATA_EOT);
		NHDBG((L" RECV CMD : RMS_RESET_MCCP_SYMMETRICAL_KEY END\r\n"));
		break;
	}
#endif
	// End of [#RWC6-533]

	default:
		NHDEBUG(1, (_T("RMS Message Indicator wrong (%X)\n"), pMSG->Header.MsgID));
		// don't support command. so. Send EOT
		RMS_SendControl(DATA_EOT);
		break;
	}

	g_preMsgID = curMsgID;

	return TRUE;
}

int	CRmsCtrl::RMS_SendControl(BYTE ctl)
{
	BYTE	ctrl_rsp	= 0;

	ctrl_rsp = ctl;
	return RMS_Send(&ctrl_rsp, 1, 0);
}

int CRmsCtrl::RMS_Send(BYTE *pData, int nLen, int nSendSec)
{
	PRMS_COMM	pMSG = (PRMS_COMM)pData;
	
	// Encrypt.
	if (nLen > XOR_MIN_LENGHT)
	{
		BYTE	nTidx;

		nTidx = RMS_EncryptXOR(&pData[3], (pMSG->Length-1));
		pMSG->TableIndex = nTidx;

		nLen = pMSG->Length + 2;
		pMSG->Length = pMSG->Length;
	}

	return m_pDevCmn->fnNET_RMSSendData(pData, nLen, nSendSec);
}

int CRmsCtrl::RMS_Recv(BYTE *pData, int *pnLen, int nRecvSec)
{
	int nRes = m_pDevCmn->fnNET_RMSRecvData(pData, pnLen, nRecvSec);

	if (nRes != RMS_OK)
		return nRes;

	// Decrypt.
	if (*pnLen > XOR_MIN_LENGHT)
	{
		RMS_DecryptXOR((BYTE)pData[2], &pData[3], (*pnLen - 3));
	}

	return nRes;
}

// verify RMS MSG
// check Serial Number or Terminal Number AND Password
BOOL CRmsCtrl::RMS_VerifyMsg(BYTE *pData, int Len)
{
	PRMS_CMD_COMM	pMSG = (PRMS_CMD_COMM)pData;
	CString	strTemp;

	if (Len < SIZE_RMS_CMD_COMM)
	{
		NHDEBUG(1, (_T("Verify MSG : SIZE is Valid.. RECV(%d), (%d)\n"), Len, SIZE_RMS_CMD_COMM));
		return FALSE;
	}

	// check Terminal Number.
	strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID);
	strTemp.TrimLeft();
	strTemp.TrimRight();
	RMS_MultiToWide((char*)pMSG->TerminalNumber, (LPWSTR)gwTempBuf, TEMP_BUFSIZE);

	if (strTemp != (LPWSTR)gwTempBuf)
	{
		NHDEBUG(1, (_T("Verify MSG : Terminal ID Worng [%s], RMS[%s]\n"), (LPCTSTR)strTemp, gwTempBuf));

		return FALSE;
	}

	// check RMS Password
	strTemp = MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSPASSWORD);
	strTemp.TrimLeft();
	strTemp.TrimRight();
	RMS_MultiToWide((char*)pMSG->RMSPassword, (LPWSTR)gwTempBuf, TEMP_BUFSIZE);

	if (strTemp != (LPWSTR)gwTempBuf)
	{
		NHDEBUG(1, (_T("Verify MSG : RMS Password Worng [%s], RMS[%s]\n"), (LPCTSTR)strTemp, gwTempBuf));
		return FALSE;
	}

	NHDEBUG(1, (_T("Verify MSG OK..")));

	return TRUE;
}

BOOL CRmsCtrl::RMS_SendATMStatus(NUINT8 MsgID, int nFrameNumber)
{
	PRMS_RES_STATUS		pStatusMsg;

	// Make msg.
	RMS_MakeStatusMsg(gpRMSBuffer, RMS_BUFSIZE);
	
	pStatusMsg = (PRMS_RES_STATUS)gpRMSBuffer;
	pStatusMsg->Header.MsgID = MsgID;
	pStatusMsg->Header.Length = LEN_RMS_RES_STATUS;
	pStatusMsg->FrameNumber = nFrameNumber;
	
	NHDEBUG(1, (_T("Send Status Msg TO RMS..\n")));
	
	// Send.
	if (RMS_Send((BYTE*)pStatusMsg, pStatusMsg->Header.Length, 0) != RMS_OK)
	{
		NHDEBUG(1, (_T("Send Status Msg TO RMS..\n")));
		return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////
//
//	RMS_ProcXXXXXXXX(void) 함수 처리시 공통사항.
//
//  <Buffer 사용>
//
//
//  <Return> : 값은 의미가 없다.
//	함수 종료시 Caller에서 다음 사항이 처리된다. 
//	1. Send EOT, 
//  2. close RMS connection

BOOL CRmsCtrl::RMS_ProcReqSetup(void)
{
	PRMS_SETUP_READ_DATA	pSetupRead;
	
	int		nWriteData, FrameNumber;
	BYTE*	pSetupData;

	NHDEBUG(1, (_T("RMS_ProcReqSetup() : READ SETUP\n")));
	
	// 1. Send Setup Data
	RMS_MakeSetupDataMsg(gpRMSDataBuffer, RMS_DATA_BUFSIZE);
	
	pSetupRead = (PRMS_SETUP_READ_DATA) gpRMSBuffer;
	pSetupData = gpRMSDataBuffer;
	nWriteData = sizeof(RMS_SETUP_DATA);
	
	FrameNumber = 1;
	
	while(nWriteData > 0)
	{
		RMS_MakeSetupReadDataMsg((BYTE*)pSetupRead, RMS_BUFSIZE);
		memset(pSetupRead->SetupData, 0, sizeof(pSetupRead->SetupData));
		
		if (nWriteData > RMS_SETUP_DATA_FRAME_SIZE)
			nWriteData = RMS_SETUP_DATA_FRAME_SIZE;
		
		memcpy(pSetupRead->SetupData, pSetupData, nWriteData);
		
		pSetupRead->Header.MsgID = RMS_REQ_SETUP;
		pSetupRead->Header.Length = LEN_RMS_SETUP_READ_DATA - (RMS_SETUP_DATA_FRAME_SIZE - nWriteData);
		pSetupRead->FrameNumber = (NUINT16)FrameNumber++;
		pSetupData += nWriteData;
		
		// First Frame.
		if (pSetupRead->FrameNumber == 1)
			pSetupRead->FrameNumber |= 0x0000;


		// Last Frame.
		if (nWriteData < RMS_SETUP_DATA_FRAME_SIZE)
			pSetupRead->FrameNumber |= 0x1000;

		
		// update.
		nWriteData = sizeof(RMS_SETUP_DATA) - (pSetupData - gpRMSDataBuffer);
		
		NHDEBUG(1, (_T("Send RMS_REQ_SETUP. Frame(%d) TO RMS..\n"), pSetupRead->FrameNumber));
		
		if (RMS_Send((BYTE*)pSetupRead, pSetupRead->Header.Length, 0) != RMS_OK)
		{
			NHDEBUG(1, (_T("Send Failed\n")));
			break;
		}
			
	}

	return TRUE;
}

BOOL CRmsCtrl::RMS_ProcSetSetup(void)
{
	PRMS_SETUP_WRITE_DATA	pSetupWrite;

	BYTE*	pSetupData;
	int		nSavedData = 0, nRecvData = 0;
	
	NHDEBUG(1, (_T("RMS_ProcSetSetup() : WRITE SETUP\n")));

	// 1. Recv Setup Data.
	memset(gpRMSDataBuffer, 0, RMS_DATA_BUFSIZE);
	pSetupData = gpRMSDataBuffer;
	
	while(1)
	{
		NHDEBUG(1, (_T("Wait RMS_SET_SETUP Data. FROM RMS\n")));
		
		memset(gpRMSBuffer, 0, RMS_BUFSIZE);
		nRecvData = RMS_BUFSIZE;
		if (RMS_Recv(gpRMSBuffer, &nRecvData, 0) != RMS_OK)
		{
			NHDEBUG(1, (_T("RMS_Recv() Failed\n")));
			goto ErrorReturn;
		}
		
		// Verify.
		if (!RMS_VerifyMsg(gpRMSBuffer, nRecvData))
		{
			NHDEBUG(1, (_T("RMS_VerifyMsg() Failed\n")));
			goto ErrorReturn;
		}
		// OK PASS
		RMS_SendControl(DATA_ACK);
		
		// Setup Data is 0
		if (nRecvData < (LEN_RMS_SETUP_WRITE_DATA - sizeof(pSetupWrite->SetupData)))
		{
			NHDEBUG(1, (_T("RMS_SET_SETUP RecvData is Invalid\n")));
			goto ErrorReturn;
		}
		
		pSetupWrite = (PRMS_SETUP_WRITE_DATA)gpRMSBuffer;
		
		// Setup Data Length.
		nRecvData = pSetupWrite->Header.Length - 27;
		
		// check buffer reamin size.
		if ((nSavedData + nRecvData) > RMS_DATA_BUFSIZE)
		{
			NHDEBUG(1, (_T("RMS_SET_SETUP Save Buffer Full. cur(%d)\n"), (nSavedData + nRecvData)));
			goto ErrorReturn;
		}
		
		// saved..
		memcpy((pSetupData+nSavedData), pSetupWrite->SetupData, nRecvData);
		
		NHDEBUG(1, (_T("RECVED RMS_SET_SETUP. FRAME(%d)\n"), pSetupWrite->FrameNumber));
		
		nSavedData += nRecvData;
		
		// It's Last Frame.
		if ((pSetupWrite->FrameNumber & 0x1000) == 0x1000)
		{
		NHDEBUG(1, (_T("RECVED RMS_SET_SETUP. LASTFRAME(%d)\n"), pSetupWrite->FrameNumber));
			break;
		}

	}
	
	// Wait EOT..
	memset(gpRMSBuffer, 0, RMS_BUFSIZE);
	nRecvData = RMS_BUFSIZE;
	if (RMS_Recv(gpRMSBuffer, &nRecvData, 0) != RMS_OK)
	{
		NHDEBUG(1, (_T("Wait EOT. RMS_Recv() Failed\n")));
		goto ErrorReturn;
	}

	if (nRecvData != 1 || gpRMSBuffer[0] != DATA_EOT)
	{
		NHDEBUG(1, (_T("RECV Data. But Not EOT\n")));
		goto ErrorReturn;
	}

	NHDEBUG(1, (_T("Write RMS_SET_SETUP Data\n")));

#if SUPPORT_CHANGE_PARAMETER_JNL		//[#610] SOOK 2010.01.14 Configuration 저널 저장 
	m_pDevCmn->SetPreviousParameter();	
#endif		//end of [#610]
	// Adjust SetupData
	RMS_WriteSetupDataMsg(pSetupData, nSavedData);
	
#if SUPPORT_CHANGE_PARAMETER_JNL		//[#610] SOOK 2010.01.14 Configuration 저널 저장 
		m_pDevCmn->SaveChangeParameter(CHANGE_USER_MONIVIEW);	
#endif		//end of [#610]

	return TRUE;	// [#2362] NH KSK 2015.08.09 Write Setup Data 성공시에만 rebooting하도록 변경
ErrorReturn:

//	return TRUE;
	return FALSE;	// [#2362] NH KSK 2015.08.09 Write Setup Data Fail시에 rebooting하지 않도록 변경
}

BOOL CRmsCtrl::RMS_ProcJNLUpload(void)
{
	PRMS_RES_JNL	pMsg;
	int		FrameNumber = 0;	
	CString	strJnlData;

	NHDEBUG(1, (_T("RMS_ProcJNLUpload() : JNL Upload\n")));

	FrameNumber = 1;
	
	while(1)
	{
		strJnlData = m_pDevCmn->m_JNLMgr.UploadMoveNext(UP_RMS);
		
		if (strJnlData.GetLength() == 0)
			break;
		
		memset(gpRMSDataBuffer, 0, RMS_DATA_BUFSIZE);
		pMsg = (PRMS_RES_JNL)gpRMSDataBuffer;
		
		MakeRMSJNLUpload((BYTE*)pMsg, RMS_DATA_BUFSIZE, strJnlData);
		
		pMsg->Header.MsgID = RMS_REQ_JNL;
		pMsg->Header.Length += LEN_RMS_RES_JNL;
		pMsg->FrameNumber = (NUINT16)FrameNumber++;
		
		if (RMS_Send((BYTE*)pMsg, pMsg->Header.Length, 0) != RMS_OK)
		{
			NHDEBUG(1, (_T("Send Failed\n")));
			break;
		}
		
		NHDEBUG(1, (_T("Send JNL Number(%d)\n"), m_pDevCmn->m_JNLMgr.GetUploadLastIndex(UP_RMS)));
	}

	return TRUE;
}

BOOL CRmsCtrl::RMS_ProcJNLXUpload(void)
{
	PRMS_RES_JNL	pMsg;
	int		FrameNumber = 0;	
	CString	strJnlData;

	NHDEBUG(1, (_T("RMS_ProcJNLXUpload() : LAST X JNL Upload\n")));

	FrameNumber = 1;

	if (g_LastXCount > 0)
	{
		// [#306] NH AIREAT 2008.06.16 - Last X 루틴 보안
		g_LastXCount -= 1;
		m_pDevCmn->m_JNLMgr.MoveLast();

		if (g_LastXCount == 0)
			strJnlData = m_pDevCmn->m_JNLMgr.MoveLast();
		else
		{
			if ((INT32)g_LastXCount <= m_pDevCmn->m_JNLMgr.GetTotalCount())
				strJnlData = m_pDevCmn->m_JNLMgr.MovePrev(g_LastXCount);
			else
				strJnlData = m_pDevCmn->m_JNLMgr.MoveFirst();
		}
		// end of [#306]
					
		while(1)
		{
			if (strJnlData.GetLength() == 0)
				break;
			
			memset(gpRMSDataBuffer, 0, RMS_DATA_BUFSIZE);
			pMsg = (PRMS_RES_JNL)gpRMSDataBuffer;
			
			MakeRMSJNLUpload((BYTE*)pMsg, RMS_DATA_BUFSIZE, strJnlData);
			
			pMsg->Header.MsgID = RMS_REQ_X_JNL;
			pMsg->Header.Length += LEN_RMS_RES_JNL;
			pMsg->FrameNumber = (NUINT16)FrameNumber++;
			
			NHDEBUG(1, (_T("JNL Length = %d\n"), pMsg->Header.Length));
			
			if (RMS_Send((BYTE*)pMsg, pMsg->Header.Length, 0) != RMS_OK)
			{
				NHDEBUG(1, (_T("Send Failed\n")));
				break;
			}
			
			strJnlData = m_pDevCmn->m_JNLMgr.MoveNext();
		}
	}

	return TRUE;
}

// [#524] US KSK 2009.05.19
BOOL CRmsCtrl::RMS_ProcJNLDateUpload(BYTE *pBuffer)
{
	PRMS_RES_JNL		pMsg;
	PRMS_CMD_DATE_JNR	pDateByJnl;
	int					FrameNumber = 0;	
	CStringArray		strResultIndexs;
	CString				strJnlData;
	
	NHDEBUG(1, (_T("RMS_ProcJNLDateUpload() : Date By JNL Upload\n")));

	FrameNumber = 1;

	pDateByJnl = (PRMS_CMD_DATE_JNR) pBuffer;

	// check validate of value
	if (IsValidDateTime(pDateByJnl->wMonth, pDateByJnl->wDate, pDateByJnl->wYear) != 0)
	{
		CTime	sDate(pDateByJnl->wYear, pDateByJnl->wMonth, pDateByJnl->wDate, 0, 0, 0);
		CTime	eDate(pDateByJnl->wYear, pDateByJnl->wMonth, pDateByJnl->wDate, 23, 59, 59);
		
		// search
		if (m_pDevCmn->m_JNLMgr.SearchByDate(sDate, eDate, strResultIndexs) == TRUE)
		{
			for (int i=0; i< strResultIndexs.GetSize(); i++)
			{
				strJnlData = m_pDevCmn->m_JNLMgr.SearchIndex(Asc2Int(strResultIndexs.GetAt(i)));

				if (strJnlData.GetLength() == 0)
					break;

				memset(gpRMSDataBuffer, 0, RMS_DATA_BUFSIZE);
				pMsg = (PRMS_RES_JNL)gpRMSDataBuffer;

				MakeRMSJNLUpload((BYTE*)pMsg, RMS_DATA_BUFSIZE, strJnlData);
				
				pMsg->Header.MsgID = RMS_REQ_DATE_JNL;
				pMsg->Header.Length += LEN_RMS_RES_JNL;
				pMsg->FrameNumber = (NUINT16)FrameNumber++;

				if (RMS_Send((BYTE*)pMsg, pMsg->Header.Length, 0) != RMS_OK)
				{
					NHDEBUG(1, (_T("Send Failed\n")));
					break;
				}	
			}
		}
	}
	
	return TRUE;
}
// end of [#524]

BOOL CRmsCtrl::RMS_ProcFileDownLoad(BYTE *pBuffer)
{
	PRMS_CMD_FILE_DOWN		pMSG;
	PRMS_FILE_DLOAD_DATA	pDownFile;

	CString		strFileName, strTempName;
	HANDLE		hFile = INVALID_HANDLE_VALUE;	// [#2022] NH KSK 2011.02.22
	int			i, nFrameNumber, nRecvData = 0, Oring;
	DWORD		nWriten;
	int			nTotalRecvData = 0;

//[#635] SOOK 2010.04.01 호주 APCA 인증 대응 Download File Authentication 
#if ( AU_VERSION )
	BOOL	bReceivedLastFrame = FALSE;
#endif
//end of[#635]
	NHDEBUG(1, (_T("RMS_ProcFileDownLoad() : File Download\n")));

	if (g_LineType == RMS_LINE_DIALUP)
		Oring = 0x1000;		// DIALUP
	else
		Oring = 0x8000;		// TCP/IP

	pMSG = (PRMS_CMD_FILE_DOWN)pBuffer;

	// create Directory..
	// [#GLDV-2853] AU Kook 2021.03.10 WEC7
// 	CreateDirectory(_T("Update"), NULL);
// 	CreateDirectory(_T("Update\\NH2700CE"), NULL);
	CreateDirectoryWithIntermediate(m_pDevCmn->m_pConfig->GetUpdatePath(BASEDIR_ROOT));
	// end of [#GLDV-2853]

	while(1)
	{
		// Get Command;
		if (pMSG == NULL)
		{
			memset(gpRMSBuffer, 0, RMS_BUFSIZE);
			nRecvData = RMS_BUFSIZE;
			if (RMS_Recv(gpRMSBuffer, &nRecvData, 0) != RMS_OK)
			{
				NHDEBUG(1, (_T("RMS_Recv() Failed\n")));
				goto ErrorReturn;
			}
			else
			{
				if (nRecvData == 1 && gpRMSBuffer[0] == DATA_EOT)
				{
					NHDEBUG(1, (_T("END OF FILE DATA\n")));
					break;
				}

				// Verify.
				if (!RMS_VerifyMsg(gpRMSBuffer, nRecvData))
				{
					NHDEBUG(1, (_T("RMS_VerifyMsg() Failed\n")));
					goto ErrorReturn;
				}
				
				// OK PASS
#if ( AU_VERSION )
				if (bReceivedLastFrame == TRUE)
				{
					pMSG = (PRMS_CMD_FILE_DOWN)gpRMSBuffer;
					break;
				}
#endif
				RMS_SendControl(DATA_ACK);
			}
			
			pMSG = (PRMS_CMD_FILE_DOWN)gpRMSBuffer;
			if (pMSG->Header.MsgID != RMS_REQ_FILE_DOWN)
				break;
		}
		
		nFrameNumber = 1;
		
		// get FileName;
		strFileName = pMSG->Dir;
		
		NHDEBUG(1, (_T("RECV FILE NAME :(%s)\n"), strFileName));

		int nIndex = strFileName.ReverseFind('\\');
		if(nIndex != -1)
			strFileName = strFileName.Right(strFileName.GetLength() - (nIndex+1));

		// [#GLDV-2853] AU Kook 2021.03.10 WEC7
// 		strTempName = "Update\\NH2700CE\\" + strFileName;
		strTempName.Format(_T("%s\\%s"), m_pDevCmn->m_pConfig->GetUpdatePath(BASEDIR_ROOT), strFileName);
		// end of [#GLDV-2853]

		NHDEBUG(1, (_T("FILE OPEN :(%s)\n"), strTempName));

		// try twice.
		for (i = 0; i < 2; i++)
		{
			// open file
			hFile = CreateFile (strTempName,				// Open NVRAM.TXT.
									GENERIC_WRITE,          // Open for writing
									0,                      // Do not share
									NULL,                   // No security
									CREATE_ALWAYS,          // Open or create
									FILE_ATTRIBUTE_NORMAL,  // Normal file
									NULL);                  // No template file
			// Open Success.
//			if (hFile)
			if (hFile != INVALID_HANDLE_VALUE)	// [#2022] NH KSK 2011.02.21 Code Sonar 지적사항 대책
				break;
		}
		
//		if (hFile == NULL)
		if (hFile == INVALID_HANDLE_VALUE)	// [#2022] NH KSK 2011.02.21 Code Sonar 지적사항 대책
		{
			NHDEBUG(1, (_T("FILE OPEN FAILED(%s)\n"), strTempName));
			return FALSE;
		}
		
		strFileName = pMSG->Dir;
		NHDEBUG(1, (_T("File DownLoad : (%s)\n"), (LPTSTR)(LPCTSTR)strFileName));
		NHDEBUG(1, (_T("Save Folderd : (%s)\n"), (LPTSTR)(LPCTSTR)strTempName));
		
		// Receive File Data.
		while(1)
		{
			// Get File Data.
			memset(gpRMSDataBuffer, 0, RMS_DATA_BUFSIZE);
			nRecvData = RMS_DATA_BUFSIZE;
			if (RMS_Recv(gpRMSDataBuffer, &nRecvData, 0) != RMS_OK)
			{
				NHDEBUG(1, (_T("RMS_Recv() Failed\n")));
				goto ErrorReturn;
			}
			
			pDownFile = (PRMS_FILE_DLOAD_DATA)gpRMSDataBuffer;

			NHDEBUG(1, (_T("We Recved Data From RMS. Frame Number(%d)\n"), nFrameNumber));
			
			// Check Error.
			if (nRecvData == 1 && gpRMSDataBuffer[0] == DATA_EOT)
			{
				NHDEBUG(1, (_T("Recv EOT. so Error!!\n")));
				goto ErrorReturn;
			}
			else if (nFrameNumber == 1 && (pDownFile->FrameNumber & 0x0001) != 0x0001)
			{
				NHDEBUG(1, (_T("not recv First Frame. recv Frame(%d)\n"), pDownFile->Header.Length));
				goto ErrorReturn;
			}
			
			// check Frame Number..
			if (nFrameNumber == (pDownFile->FrameNumber & ~(Oring)))
			{
				BYTE	*pFileBuf = pDownFile->FileData;
				int		nFileBufSize = nRecvData - 2 - 4;
				int		nWriteFile = 0;

#if 0
				// [#222] CSW 2008.5.21
				nTotalRecvData += nFileBufSize;
				if (nTotalRecvData >= 3145728)	// MAX 3M(3*1024*1024)
				{
					goto ErrorReturn;
				}
				// end of [#222]
#endif

				// 512 byte 씩 저장한다.
				while (nFileBufSize > 0)
				{
					nWriten = 0;

					if (nFileBufSize > 2048)
						nWriteFile = 512;
					else
						nWriteFile = nFileBufSize;

					if (!WriteFile(hFile, pFileBuf, nWriteFile, &nWriten, NULL))
					{
						NHDEBUG(1, (_T("File Write Failed\n")));
						goto ErrorReturn;
					}

					pFileBuf += nWriten;
					nFileBufSize -= nWriten;

//					NHDEBUG(1, (_T("WRITE FILE Write - (%d), Remain(%d)\n"), nWriten, nFileBufSize));
				}
				nFrameNumber++;
			}
			// frame 놓친 경우.
			else if (nFrameNumber == 1 && nFrameNumber < pDownFile->FrameNumber)
			{
				NHDEBUG(1, (_T("File Frame Missed. want(%d), recv(%d)\n\n"), nFrameNumber, pDownFile->FrameNumber));
				goto ErrorReturn;
			}
			
			// OK PASS
			RMS_SendControl(DATA_ACK);
			
			// End of File Data
			if ((pDownFile->FrameNumber & Oring) == Oring)
			{
#if ( AU_VERSION )
				bReceivedLastFrame = TRUE;
#endif
				break;
			}
		}
		
		CloseHandle(hFile);
		hFile = NULL;
		pMSG = NULL;

		NHDEBUG(1, (_T("FILE SAVED SUCCESS !!! - (%s)\n"), (LPTSTR)(LPCTSTR)strTempName));
	}

	if (!pMSG)
	{
		NHDEBUG(1, (_T("PMSG is NULL \n")));
		goto ErrorReturn;
	}

	if (pMSG->Header.MsgID != RMS_SET_GO)
	{
		NHDEBUG(1, (_T("RECV CMD.. BUT It's not GO CMD. (0x%X)\n"), pMSG->Header.MsgID));
		goto ErrorReturn;
	}
//[#635] SOOK 2010.04.01 호주 APCA 인증 대응 Download File Authentication 
#if (AU_VERSION)
	else 
	{
		CString strCertiFilePath = _T("\\ATM\\MoniView.der");

		//MoniView.der 파일 검색 
		CCeFileFind finder;
		BOOL	bExistCertificate = finder.FindFile(strCertiFilePath);
		NHDEBUG(1, (_T("Search  Certification = %d\n"), bExistCertificate));

		if (nRecvData > 0x21) //추가로 Hash Data가 왔으면 
		{
			PRMS_GO_COMMAND pGoCommad = (PRMS_GO_COMMAND)gpRMSBuffer;
			PRMS_CERTIFICATE pCerti = NULL;	
			//Hash Data Parsing 함 
			pGoCommad->HashValue[pGoCommad->HashValueLength] = 0x00;
			NHDEBUG(1, (_T("Parsed Hash Value, File Count = %d, wHashLength = %d\n"), pGoCommad->FileCount, pGoCommad->HashValueLength ));

			if ( bExistCertificate == FALSE )
			{
				//파일이 없으면 'C' 커맨드 송신하여 인증서 파일 요청 
				RMS_SendControl(DATA_REQ_CERT);

				if (RMS_Recv(gpRMSBuffer, &nRecvData, 0) != RMS_OK)
				{
					NHDEBUG(1, (_T("RMS_Recv() Failed\n")));
					goto ErrorReturn;
				}
				if (nRecvData == 1 )
				{
					NHDEBUG(1, (_T("Certification File Failed\n")));
					goto ErrorReturn;
				}

				// Verify.
				if (!RMS_VerifyMsg(gpRMSBuffer, nRecvData))
				{
					NHDEBUG(1, (_T("RMS_VerifyMsg() Failed\n")));
					goto ErrorReturn;
				}

				//인증서 파일 전문 받았으면 파일에 저장함 
				pCerti = (PRMS_CERTIFICATE)gpRMSBuffer;

				NHDEBUG(1, (_T("Save Certification. CertificateLength = %d\n"), pCerti->CertifiateLength  ));
				if(	RMS_SaveCertificateFile((unsigned char *)strCertiFilePath.GetBuffer(0), pCerti->Certifiate, pCerti->CertifiateLength ) == FALSE)
				{
					RMS_SendControl(DATA_RES_FAILURE);
					NHDEBUG(1, (_T("File Saving Failed\n")));
					goto ErrorReturn;
				}
//				Sleep(1000);
			}
			BOOL bValid = FALSE;
			if ( bExistCertificate == TRUE )
				bValid = RMS_CheckFileHashValue(strTempName.GetBuffer(0), pGoCommad->HashValue, pGoCommad->HashValueLength);
			else if ( pCerti != NULL)
				bValid = RMS_CheckFileHashValue(strTempName.GetBuffer(0), pGoCommad->HashValue, pGoCommad->HashValueLength, pCerti->Certifiate, pCerti->CertifiateLength );

			//인증서 파일로 HASH 데이타 검증
			if ( bValid == TRUE)
				RMS_SendControl(DATA_ACK);
			else
			{
				//검증 실패 하면 'F' 송신 
				RMS_SendControl(DATA_RES_FAILURE);
				NHDEBUG(1, (_T("File Verification Failed\n")));
				goto ErrorReturn;
			}
		}
		else
		{
			//인증서 파일이 존재하는데 Hash 데이타가 안 온 경우 
			if ( bExistCertificate )
			{
				//검증 실패 하면 'F' 송신 
				RMS_SendControl(DATA_RES_FAILURE);
				NHDEBUG(1, (_T("File Verification Failed\n")));
				goto ErrorReturn;
			}
			else
				RMS_SendControl(DATA_ACK);
		}
	}
#endif
//end of [#635]


	// Wait EOT..
	memset(gpRMSBuffer, 0, RMS_BUFSIZE);
	nRecvData = RMS_BUFSIZE;
	if (RMS_Recv(gpRMSBuffer, &nRecvData, 0) != RMS_OK)
	{
		NHDEBUG(1, (_T("RMS_Recv() Failed\n")));
		goto ErrorReturn;
	}

	if (nRecvData != 1 || gpRMSBuffer[0] != DATA_EOT)
	{
		NHDEBUG(1, (_T("RECV Data. But Not EOT\n")));
		goto ErrorReturn;
	}

	NHDEBUG(1, (_T("WE RECV GO CMD.. It will SWUPDATE...\n")));

	return TRUE;

ErrorReturn:

//	if (hFile)
	if ( (hFile != INVALID_HANDLE_VALUE) && (hFile != NULL)	)// [#2022] NH KSK 2011.02.21 Code Sonar 지적사항 대책
		CloseHandle(hFile);

	return FALSE;
}

// [#523] US KSK 2009.05.19
BOOL CRmsCtrl::RMS_ProcFileUpLoad(BYTE *pBuffer)
{
	PRMS_CMD_FILE_UPLOAD		pMSG;
	RMS_FILE_UPLOAD_DATA		TempUploadData;

	CString		strRecvFileName, strFileName, strZipFileName;
	HANDLE		hFile = INVALID_HANDLE_VALUE;	// [#2022] NH KSK 2011.02.22
	int			nFrameNumber, nRecvData = 0;

	bool		bIsLogBackup		= false;	// [#2239] NH KMK 2013.12.11 Log Backup 변수. 주석 추가

	// Check File exist
	CFile datFile;
	CFileException e;

	NHDEBUG(1, (_T("RMS_ProcFileUpLoad() : File Upload\n")));

	pMSG = (PRMS_CMD_FILE_UPLOAD)pBuffer;

	// Get Command;
	if (pMSG == NULL)
	{
		memset(gpRMSBuffer, 0, RMS_BUFSIZE);
		nRecvData = RMS_BUFSIZE;

		if (RMS_Recv(gpRMSBuffer, &nRecvData, 0) != RMS_OK)
		{
			NHDEBUG(1, (_T("RMS_Recv() Failed\n")));
			return FALSE;
		}
		else
		{
			if (nRecvData == 1 && gpRMSBuffer[0] == DATA_EOT)
			{
				NHDEBUG(1, (_T("END OF FILE DATA\n")));
				return FALSE;
			}

			// Verify.
			if (!RMS_VerifyMsg(gpRMSBuffer, nRecvData))
			{
				NHDEBUG(1, (_T("RMS_VerifyMsg() Failed\n")));
				return FALSE;
			}
			
			// OK PASS
			RMS_SendControl(DATA_ACK);
		}
		
		pMSG = (PRMS_CMD_FILE_UPLOAD)gpRMSBuffer;

		if (pMSG->Header.MsgID != RMS_REQ_FILE_UPLOAD)
			return FALSE;
	}
	
	// get FileName;
	strRecvFileName = pMSG->UploadDir;
	
	NHDEBUG(1, (_T("RECV FILE NAME :(%s)\n"), strRecvFileName));

	// [#2239] NH KMK 2013.10.23 upload file 파라미터로 '\CMD=[COMMAND]' 수신 시
	if (strRecvFileName.Left(5).CompareNoCase(L"\\CMD=") == 0)
	{
		CString strRecvCommand = strRecvFileName.Mid(5);	// [COMMAND] 추출

		// LOGBACKUP
 		if (strRecvCommand.CompareNoCase(L"LOGBACKUP") == 0)				// LOGBACKUP
		{
			bIsLogBackup = true;

			// 현재 NVRAM에 있는 로그를 파일로 추출한다 (\CLOG.log)
			pLog pNVLog;
			FILE *Stream;

			if ( (Stream = _wfopen(L"\\CLOG.log", L"w+b")) != NULL)
			{
				pNVLog = (pLog)GetLOGAddr(0);

				fwrite((void*)pNVLog, sizeof(char), sizeof(sLog), Stream);

				fflush(Stream);
				fclose(Stream);

				FreeLOGAddr(pNVLog);
			}
			else
				return FALSE;

			// upload file 이름을 \\CLOG.log로 변경
			if (bIsLogBackup == true)
				strRecvFileName.Format(L"\\CLOG.log");

			// 이후 과정(압축, 전송 등)은 아래에서 bIsLogBackup 값을 참조하여 분기처리됨
		}
// 		else if (strRecvCommand.CompareNoCase(L"SOMETHING_NEW") )		// 추후 다른 기능 추가 시
// 		{
//			
// 		}
		else
			return FALSE;
	}
	// end of [#2239]

	nFrameNumber = 1;

	// File Check
    hFile = CreateFile(strRecvFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);

	if (INVALID_HANDLE_VALUE == hFile)
	{
		memset(&TempUploadData, 0, sizeof(TempUploadData));
		RMS_MakeUploadFileDataMsg((BYTE*)&TempUploadData, sizeof(TempUploadData));
		
		TempUploadData.Header.MsgID = RMS_REQ_FILE_UPLOAD;
		TempUploadData.FrameNumber = (NUINT16)nFrameNumber++;
		
		TempUploadData.FrameNumber |= 0x8000;
		TempUploadData.Header.Length = sizeof(TempUploadData) - sizeof(TempUploadData.FileData) - 2;
		TempUploadData.UploadDataSize = (NUINT16)0xFFFF;
		
		RMS_Send((BYTE*)&TempUploadData, TempUploadData.Header.Length, 0);
		
		return FALSE;
	}

	CloseHandle(hFile);
	hFile = INVALID_HANDLE_VALUE;	// [#2022] NH KSK 2011.02.22

	// Make zip File
	// Remove File Path
	int Index = strRecvFileName.ReverseFind('\\');
	int Count = strRecvFileName.GetLength() - Index;

	if (Index == -1 || Count <= 1)	return FALSE;

	strFileName = strRecvFileName.Right(Count-1);
	NHDEBUG(1, (_T("FILE NAME :(%s)\n"), strFileName));
		
	strZipFileName.Format(L"\\%s.zip", strFileName);	// root memory에다가 zip file을 만든다.
	NHDEBUG(1, (_T("ZIP FILE NAME :(%s)\n"), strZipFileName));

	// [#2012] NH KJW 2011.01.20 Zip을 CmnLib로 이동하여 함수명 변경함
	/*
	HZIP hz = CreateZip(strZipFileName,0);
	ZipAdd(hz, strFileName, strRecvFileName);
	CloseZip(hz);
	*/
	HZIP hz = zCreateZip(strZipFileName, 0);
	zZipAdd(hz, strFileName, strRecvFileName);
	
	// [#2239] NH KMK 2013.10.24 LOG BACKUP인 경우 \ATM2\LOG 폴더를 추가 압축한다
	if (bIsLogBackup == true)
	{
		WIN32_FIND_DATA	FileData;
		HANDLE			hSearch = INVALID_HANDLE_VALUE;
		CString			strSrcFileName, strDestFileName;

		strSrcFileName.Format(_T("\\ATM2\\LOG\\*.*"));
		hSearch = FindFirstFile(strSrcFileName, &FileData);
		if (hSearch != INVALID_HANDLE_VALUE)
		{
			while(1)
			{
				strDestFileName.Format(_T("%s\\%s"), _T("\\ATM2\\LOG"), FileData.cFileName);

				zZipAdd(hz, FileData.cFileName, strDestFileName);

				if (!FindNextFile(hSearch, &FileData))
					break;
			}
			FindClose(hSearch);
		}
	}
	// end of [#2239]

	zCloseZip(hz);
	// end of [#2012]

	NHDEBUG(1, (_T("ZIP FILE MAKE SUCCESS! GO~~ \n")));

	if(datFile.Open(strZipFileName, CFile::modeRead, &e))		// zip file 전송을 위해 file open
	{
		UINT nSize = (UINT) datFile.GetLength();
		UINT nSendIndex = 0;
		BYTE *pBuffer = new BYTE[nSize+1];

		memset(pBuffer, 0, nSize+1);
		if( datFile.Read(pBuffer, nSize) == nSize)
		{
			if (nSize >= 3145728)	// MAX 3M(3*1024*1024)
			{
				delete [] pBuffer;
				datFile.Close();
				DeleteFile(strZipFileName);	// zip File 삭제

				return FALSE;
			}

			// File Data Upload Start
			while(nSize > 0)
			{
				// Make Upload File Data
				// Make Upload Common Header Information
				memset(&TempUploadData, 0, sizeof(TempUploadData));
				RMS_MakeUploadFileDataMsg((BYTE*)&TempUploadData, sizeof(TempUploadData));
			
				TempUploadData.Header.MsgID = RMS_REQ_FILE_UPLOAD;
				TempUploadData.FrameNumber = (NUINT16)nFrameNumber++;

				if (nSize > sizeof(TempUploadData.FileData))
				{
					TempUploadData.FrameNumber |= 0x0000;
					TempUploadData.Header.Length = sizeof(TempUploadData) - 2;
					TempUploadData.UploadDataSize = sizeof(TempUploadData.FileData);
					memcpy(TempUploadData.FileData, &pBuffer[nSendIndex], sizeof(TempUploadData.FileData));
					nSendIndex = nSendIndex + sizeof(TempUploadData.FileData);
					nSize = nSize - sizeof(TempUploadData.FileData);
				}
				else
				{
					// Last Frame
					TempUploadData.FrameNumber |= 0x8000;		// Upload일 경우에는 0x1000이 아닌 0x8000을 사용함 (RMS 특이사항)
					TempUploadData.Header.Length = sizeof(TempUploadData) - sizeof(TempUploadData.FileData) + nSize - 2;
					TempUploadData.UploadDataSize = nSize;
					memcpy(TempUploadData.FileData, &pBuffer[nSendIndex], nSize);
					nSendIndex = nSendIndex + nSize;
					nSize = 0;
				}
									
				NHDEBUG(1, (_T("Send RMS_REQ_FILE_UPLOAD. Frame(%x) nSize(%d)TO RMS..\n"), TempUploadData.FrameNumber, nSize));
				
				if (RMS_Send((BYTE*)&TempUploadData, TempUploadData.Header.Length, 0) != RMS_OK)
				{
					NHDEBUG(1, (_T("Send Failed\n")));
					break;
				}
			}
		}
		delete [] pBuffer;
		datFile.Close();
		DeleteFile(strZipFileName);	// zip File 삭제
	}

	return TRUE;
}
// end of [#523]

// [#424] [NH] KSK 2008.9.18
BOOL CRmsCtrl::RMS_ProcReqErrTotal(BOOL bClear)
{
	PRMS_CMD_ERROR_TOTAL	pErrTotal;	
	int		nWriteData;

	NHDEBUG(1, (_T("RMS_ProcReqErrTotal()\n")));
	
	// 1. Send Setup Data
	nWriteData = RMS_MakeErrTotalMsg(gpRMSBuffer);

	pErrTotal = (PRMS_CMD_ERROR_TOTAL)gpRMSBuffer;
	
	if (bClear == TRUE)
		pErrTotal->Header.MsgID = RMS_REQ_ERR_TOTALNCLR;
	else
		pErrTotal->Header.MsgID = RMS_REQ_ERR_TOTAL;

	pErrTotal->Header.Length = nWriteData;
	pErrTotal->FrameNumber = 0x1001;
	
	NHDEBUG(1, (_T("Send RMS_ProcReqErrTotal TO RMS..\n")));
	
	// Send.
	if (RMS_Send((BYTE*)pErrTotal, pErrTotal->Header.Length, 0) != RMS_OK)
	{
		NHDEBUG(1, (_T("Send RMS_ProcReqErrTotal TO RMS..\n")));
		return FALSE;
	}

	if (bClear == TRUE)
	{
		ClearErrSum();
#if SUPPORT_CHANGE_PARAMETER_JNL
		m_pDevCmn->m_JNLMgr.Save(MONIVIEW_ACTION, L"Error Summary Cleared"); //[#610] SOOK 2010.01.16 Configuration Change 저널 추가 
#endif //end of [#610]
	}

	return TRUE;
}
// end of [#424]

// [#533] [NH] KSK 2009.06.05
BOOL CRmsCtrl::RMS_ProcReqRejectTotal(BOOL bClear)
{
	PRMS_CMD_REJECT_TOTAL	pRejectTotal;	
	
	NHDEBUG(1, (_T("RMS_ProcReqRejectTotal()\n")));
	
	// 1. Send Setup Data
	RMS_MakeRejectTotalMsg(gpRMSBuffer);
	
	pRejectTotal = (PRMS_CMD_REJECT_TOTAL)gpRMSBuffer;
	
	if (bClear == TRUE)
		pRejectTotal->Header.MsgID = RMS_REQ_REJECTTOTALNCLR;
	else
		pRejectTotal->Header.MsgID = RMS_REQ_REJECTTOTAL;
	
	pRejectTotal->Header.Length = sizeof(RMS_CMD_REJECT_TOTAL) - 2;
	pRejectTotal->FrameNumber = 0x1001;
	
	NHDEBUG(1, (_T("Send RMS_ProcReqRejectTotal TO RMS..\n")));
	
	// Send.
	if (RMS_Send((BYTE*)pRejectTotal, pRejectTotal->Header.Length, 0) != RMS_OK)
	{
		NHDEBUG(1, (_T("Send RMS_ProcReqRejectTotal TO RMS..\n")));
		return FALSE;
	}
	
	if (bClear == TRUE)
	{
		ClearRejectTotal();
#if SUPPORT_CHANGE_PARAMETER_JNL //[#610] SOOK 2010.01.16 Configuration Change 저널 추가 
		m_pDevCmn->m_JNLMgr.Save(MONIVIEW_ACTION, L"Reject Analysis Cleared"); 
#endif  //end of [#610]
	}
	
	return TRUE;
}
// end of [#533]

/**
 * Handle the remote update command sent by RMS
 * @param pBuffer[in] The buffer containing the data sent by RMS
 * @return true when successful
 */
BOOL CRmsCtrl::RMS_ProcRemoteUpdate(BYTE *pBuffer)
{
	BOOL result = TRUE;
	char *swVersion = NULL;
	CFile file;
	CString destinationVersion;
	PackageList packages;
	bool foundVersion = false;
	Package destinationPackage;
	UpdateSourceInfo config;
	CUpdateRepoConfigurationManager configManager;
	CUpdateRepoService *repo;
	PRMS_CMD_INSTALLREMOTEUPDATE request = (PRMS_CMD_INSTALLREMOTEUPDATE) pBuffer;
	PRMS_RES_INSTALLREMOTEUDPATE response = new RMS_RES_INSTALLREMOTEUPDATE();

	NHDBG((L"Handling remote update command\r\n"));

	destinationVersion = request->GetVersionName();
	NHDBG((L"Update requested to %s\r\n", destinationVersion));

	if (configManager.GetConfiguration(&config) != ERR_OK)
	{
		NHERROR((L"Configuration failed to load\r\n"));
		response->StatusCode = 1;
		strcpy(response->ResultMessage, "Config file error");
		result = FALSE;

		goto send;
	}

	repo = new CUpdateRepoService(&config);
	if (repo->GetComponentContents(COMPONENT_AP, packages) != REPOERR_OK)
	{
		NHERROR((L"Failed to download packages from remote repo\r\n"));
		response->StatusCode = 2;
		strcpy(response->ResultMessage, "Package list error");
		result = FALSE;

		goto send;
	}

	// Find package
	for (int i = 0; i < packages.GetCount(); i++)
	{
		POSITION pos = packages.FindIndex(i);
		Package p = packages.GetAt(pos);

		if (p.ReadableVersion == destinationVersion)
		{
			// Found the right one!
			foundVersion = true;
			destinationPackage = p;

			break;
		}
	}

	if (!foundVersion)
	{
		NHERROR((L"%s not found in repository listing\r\n", destinationVersion));
		response->StatusCode = 3;
		strcpy(response->ResultMessage, "Version not found");
		result = FALSE;

		goto send;
	}

	// Set registry settings
	if (!file.Open(REMOTE_UPDATE_INIT_FILE, CFile::modeReadWrite | CFile::modeCreate))
	{
		NHERROR((L"Could not open file for writing\r\n"));
		response->StatusCode = 4;
		strcpy(response->ResultMessage, "Failed to invoke update");
		result = FALSE;
		
		goto send;
	}

	swVersion = new char[destinationVersion.GetLength() + 1]();
	WideToMulti(swVersion, destinationVersion, destinationVersion.GetLength());

	file.Write(swVersion,destinationVersion.GetLength());
	file.Close();

	delete [] swVersion;

	MemSetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSCOMMAND, RMSCMD_SWUPDATE_REPO);

	response->StatusCode = 0;
	strcpy(response->ResultMessage, "Success");

send:
	NHDBG((L"Send RMS_ProcRemoteUpdate TO RMS..\n"));
	
	// Send.
	response->Header.Length = LEN_RMS_RES_REMOTEUPDATE;
	response->Header.MsgID = RMS_INSTALL_REMOTE_UPDATE;
	response->FrameNumber = 1;
	if (RMS_Send((BYTE*)response, sizeof(RMS_RES_INSTALLREMOTEUPDATE), 0) != RMS_OK)
	{
		NHDEBUG(1, (_T("Send RMS_ProcRemoteUpdate TO RMS..\n")));
		result = FALSE;
	}

	if (repo)
	{
		delete repo;
		repo = NULL;
	}

	if (response)
	{
		delete response;
		response = NULL;
	}

	return result;
}

// [#620] US KSK 2010.02.22
BOOL CRmsCtrl::RMS_ProcReqEnhancedCoupon()
{
	PRMS_READ_ENHANCED_COUPON	pReadEnhancedCoupon;
	
	int		nWriteData, FrameNumber;
	BYTE*	pEnhancedCouponData;
	
	NHDEBUG(1, (_T("RMS_ProcReqEnhancedCoupon() : Read Enhanced Coupon\n")));
	
	// 1. Send Setup Data
	RMS_MakeEnhancedCouponMsg(gpRMSDataBuffer);
	
	pReadEnhancedCoupon = (PRMS_READ_ENHANCED_COUPON) gpRMSBuffer;
	pEnhancedCouponData = gpRMSDataBuffer;
	nWriteData = sizeof(RMS_ENHANCED_COUPON);
	
	FrameNumber = 1;
	
	while(nWriteData > 0)
	{
		RMS_MakeEnhancedCouponReadDataMsg((BYTE*)pReadEnhancedCoupon);
		memset(pReadEnhancedCoupon->EnhancedCouponData, 0, sizeof(pReadEnhancedCoupon->EnhancedCouponData));
		
		if (nWriteData > 512)
			nWriteData = 512;
		
		memcpy(pReadEnhancedCoupon->EnhancedCouponData, pEnhancedCouponData, nWriteData);
		
		pReadEnhancedCoupon->Header.MsgID = RMS_ENHANCED_COUPON_UPLOAD;
		pReadEnhancedCoupon->Header.Length = LEN_RMS_ENHANCEDCOUPON_READ_DATA - (512 - nWriteData);
		pReadEnhancedCoupon->FrameNumber = (NUINT16)FrameNumber++;
		pEnhancedCouponData += nWriteData;
		
		// First Frame.
		if (pReadEnhancedCoupon->FrameNumber == 1)
			pReadEnhancedCoupon->FrameNumber |= 0x0000;
		
		
		// Last Frame.
		if (nWriteData < 512)
			pReadEnhancedCoupon->FrameNumber |= 0x1000;
		
		
		// update.
		nWriteData = sizeof(RMS_ENHANCED_COUPON) - (pEnhancedCouponData - gpRMSDataBuffer);
		
		NHDEBUG(1, (_T("Send RMS_ENHANCED_COUPON_UPLOAD. Frame(%d) TO RMS..\n"), pReadEnhancedCoupon->FrameNumber));
		
		if (RMS_Send((BYTE*)pReadEnhancedCoupon, pReadEnhancedCoupon->Header.Length, 0) != RMS_OK)
		{
			NHDEBUG(1, (_T("Send Failed\n")));
			break;
		}
		
	}
	
	return TRUE;
}

BOOL CRmsCtrl::RMS_ProcSetEnhancedCoupon()
{
	PRMS_WRITE_ENHANCED_COUPON	pEnhancedCouponWrite;
	
	BYTE*	pEnhancedCouponData;
	int		nSavedData = 0, nRecvData = 0;
	
	NHDEBUG(1, (_T("RMS_ProcSetEnhancedCoupon() : WRITE ENHANCED COUPON\n")));
	
	// 1. Recv Setup Data.
	memset(gpRMSDataBuffer, 0, RMS_DATA_BUFSIZE);
	pEnhancedCouponData = gpRMSDataBuffer;
	
	while(1)
	{
		NHDEBUG(1, (_T("Wait ENHANCED COUPON Data. FROM RMS\n")));
		
		memset(gpRMSBuffer, 0, RMS_BUFSIZE);
		nRecvData = RMS_BUFSIZE;
		if (RMS_Recv(gpRMSBuffer, &nRecvData, 0) != RMS_OK)
		{
			NHDEBUG(1, (_T("RMS_Recv() Failed\n")));
			goto ErrorReturn;
		}
		
		// Verify.
		if (!RMS_VerifyMsg(gpRMSBuffer, nRecvData))
		{
			NHDEBUG(1, (_T("RMS_VerifyMsg() Failed\n")));
			goto ErrorReturn;
		}
		// OK PASS
		RMS_SendControl(DATA_ACK);
		
		// Setup Data is 0
		if (nRecvData < (LEN_RMS_WRITE_ENHANCED_COUPON - sizeof(pEnhancedCouponWrite->EnhancedCouponData)))
		{
			NHDEBUG(1, (_T("WRITE ENHANCED COUPON RecvData is Invalid\n")));
			goto ErrorReturn;
		}
		
		pEnhancedCouponWrite = (PRMS_WRITE_ENHANCED_COUPON)gpRMSBuffer;
		
		// Setup Data Length.
		nRecvData = pEnhancedCouponWrite->Header.Length - 27;
		
		// check buffer reamin size.
		if ((nSavedData + nRecvData) > RMS_DATA_BUFSIZE)
		{
			NHDEBUG(1, (_T("WRITE ENHANCED COUPON Save Buffer Full. cur(%d)\n"), (nSavedData + nRecvData)));
			goto ErrorReturn;
		}
		
		// saved..
		memcpy((pEnhancedCouponData+nSavedData), pEnhancedCouponWrite->EnhancedCouponData, nRecvData);
		
		NHDEBUG(1, (_T("RECVED WRITE ENHANCED COUPON. FRAME(%d)\n"), pEnhancedCouponWrite->FrameNumber));
		
		nSavedData += nRecvData;
		
		// It's Last Frame.
		if ((pEnhancedCouponWrite->FrameNumber & 0x1000) == 0x1000)
		{
			NHDEBUG(1, (_T("RECVED WRITE ENHANCED COUPON. LASTFRAME(%d)\n"), pEnhancedCouponWrite->FrameNumber));
			break;
		}
		
	}
	
	// Wait EOT..
	memset(gpRMSBuffer, 0, RMS_BUFSIZE);
	nRecvData = RMS_BUFSIZE;
	if (RMS_Recv(gpRMSBuffer, &nRecvData, 0) != RMS_OK)
	{
		NHDEBUG(1, (_T("Wait EOT. RMS_Recv() Failed\n")));
		goto ErrorReturn;
	}
	
	if (nRecvData != 1 || gpRMSBuffer[0] != DATA_EOT)
	{
		NHDEBUG(1, (_T("RECV Data. But Not EOT\n")));
		goto ErrorReturn;
	}
	
	NHDEBUG(1, (_T("Write ENHANCED COUPON Data\n")));
	
	// Adjust Enhanced Coupon Data
	RMS_WriteEnhancedCouponMsg(pEnhancedCouponData, nSavedData);
	
ErrorReturn:
	
	return TRUE;
}
// end of [#620]

//[#586] SOOK 2009.12.04 iTM MoniView 연동 부분 추가 
BOOL CRmsCtrl::RMS_ProcSetWeatherInfo()
{
	PRMS_SETUP_WRITE_DATA	pSetupWrite;

	BYTE*	pSetupData;
	int		nSavedData = 0, nRecvData = 0;
	
	NHDEBUG(1, (_T("RMS_ProcSetSetup() : WRITE WEATHER\n")));

	// 1. Recv Setup Data.
	memset(gpRMSDataBuffer, 0, RMS_DATA_BUFSIZE);
	pSetupData = gpRMSDataBuffer;
	
	while(1)
	{
		NHDEBUG(1, (_T("Wait RMS_SET_WEATHER Data. FROM RMS\n")));
		
		memset(gpRMSBuffer, 0, RMS_BUFSIZE);
		nRecvData = RMS_BUFSIZE;
		if (RMS_Recv(gpRMSBuffer, &nRecvData, 0) != RMS_OK)
		{
			NHDEBUG(1, (_T("RMS_Recv() Failed\n")));
			goto ErrorReturn;
		}
		
		// Verify.
		if (!RMS_VerifyMsg(gpRMSBuffer, nRecvData))
		{
			NHDEBUG(1, (_T("RMS_VerifyMsg() Failed\n")));
			goto ErrorReturn;
		}
		// OK PASS
		RMS_SendControl(DATA_ACK);
		
		// Setup Data is 0
		if (nRecvData < (LEN_RMS_SETUP_WRITE_DATA - sizeof(pSetupWrite->SetupData)))
		{
			NHDEBUG(1, (_T("RMS_SET_WEATHER RecvData is Invalid\n")));
			goto ErrorReturn;
		}
		
		pSetupWrite = (PRMS_SETUP_WRITE_DATA)gpRMSBuffer;
		
		// Setup Data Length.
		nRecvData = pSetupWrite->Header.Length - 27;
		
		// check buffer reamin size.
		if ((nSavedData + nRecvData) > RMS_DATA_BUFSIZE)
		{
			NHDEBUG(1, (_T("RMS_SET_WEATHER Save Buffer Full. cur(%d)\n"), (nSavedData + nRecvData)));
			goto ErrorReturn;
		}
		
		// saved..
		memcpy((pSetupData+nSavedData), pSetupWrite->SetupData, nRecvData);
		
		NHDEBUG(1, (_T("RECVED RMS_SET_WEATHER. FRAME(%d)\n"), pSetupWrite->FrameNumber));
		
		nSavedData += nRecvData;
		
		// It's Last Frame.
		if ((pSetupWrite->FrameNumber & 0x1000) == 0x1000)
			break;
	}
	
	// Wait EOT..
	memset(gpRMSBuffer, 0, RMS_BUFSIZE);
	nRecvData = RMS_BUFSIZE;
	if (RMS_Recv(gpRMSBuffer, &nRecvData, 0) != RMS_OK)
	{
		NHDEBUG(1, (_T("Wait EOT. RMS_Recv() Failed\n")));
		goto ErrorReturn;
	}

	if (nRecvData != 1 || gpRMSBuffer[0] != DATA_EOT)
	{
		NHDEBUG(1, (_T("RECV Data. But Not EOT\n")));
		goto ErrorReturn;
	}

	NHDEBUG(1, (_T("Write RMS_SET_WEATHER Data\n")));

	// Adjust SetupData
	//RMS_WriteSetupDataMsg(pSetupData, nSavedData);
	RMS_WirteWeatherDataMsg(pSetupData, nSavedData);
	
ErrorReturn:

	return TRUE;
}
BOOL CRmsCtrl::RMS_ProcUploadJNLImage(BYTE *pBuffer)
{
//	PRMS_CMD_FILE_UPLOAD		pMSG;
	RMS_IMAGE_UPLOAD_DATA 		TempUploadData;
	PRMS_CMD_IMAGE_UPLOAD		pMSG;

	CString		strRecvFileName, strFileName, strZipFileName;
//	HANDLE		hFile = NULL;	// [#2022] NH KSK 2011.02.22
	int			nFrameNumber, nRecvData = 0;

	// Check File exist
	CFile datFile;
	CFileException e;

	NHDEBUG(1, (_T("RMS_ProcFileUpLoad() : Jnl Image File Upload\n")));

	pMSG = (PRMS_CMD_IMAGE_UPLOAD)pBuffer;

	// Get Command;
	if (pMSG == NULL)
	{
		memset(gpRMSBuffer, 0, RMS_BUFSIZE);
		nRecvData = RMS_BUFSIZE;

		if (RMS_Recv(gpRMSBuffer, &nRecvData, 0) != RMS_OK)
		{
			NHDEBUG(1, (_T("RMS_Recv() Failed\n")));
			return FALSE;
		}
		else
		{
			if (nRecvData == 1 && gpRMSBuffer[0] == DATA_EOT)
			{
				NHDEBUG(1, (_T("END OF FILE DATA\n")));
				return FALSE;
			}

			// Verify.
			if (!RMS_VerifyMsg(gpRMSBuffer, nRecvData))
			{
				NHDEBUG(1, (_T("RMS_VerifyMsg() Failed\n")));
				return FALSE;
			}
			
			// OK PASS
			RMS_SendControl(DATA_ACK);
		}
		
		pMSG = (PRMS_CMD_IMAGE_UPLOAD)gpRMSBuffer;

		if (pMSG->Header.MsgID != RMS_REQ_JNL_IMAGE)
			return FALSE;
	}
	
	// get FileName;
	strRecvFileName = m_pDevCmn->m_JNLMgr.GetJnlImageFileName(pMSG->StackNumber, pMSG->StackYear, pMSG->StackMonth, pMSG->StackDay);
	
	NHDEBUG(1, (_T("RECV FILE NAME :(%s)\n"), strRecvFileName));

	nFrameNumber = 1;

	if ( m_pDevCmn->m_JNLMgr.DoesJnlImageExist(pMSG->StackNumber, pMSG->StackYear, pMSG->StackMonth, pMSG->StackDay) == FALSE)
	{
		memset(&TempUploadData, 0, sizeof(TempUploadData));
		RMS_MakeJNLImageDataMsg((BYTE*)&TempUploadData, sizeof(TempUploadData));
		
		TempUploadData.Header.MsgID = RMS_REQ_JNL_IMAGE;
		TempUploadData.FrameNumber = (NUINT16)nFrameNumber++;
		
		TempUploadData.FrameNumber |= 0x8000;
		TempUploadData.TotalImageCount = 1;	//CE는 저널 1개당 1컷만 찍음 
		TempUploadData.CurrentImageSeq = 1;	//CE는 저널 1개당 1컷만 찍음 
		TempUploadData.Header.Length = sizeof(TempUploadData) - sizeof(TempUploadData.FileData) - 2;
		TempUploadData.UploadDataSize = (NUINT16)0xFFFF;
		
		RMS_Send((BYTE*)&TempUploadData, TempUploadData.Header.Length, 0);
		
		return FALSE;
	}


	NHDEBUG(1, (_T("ZIP FILE MAKE SUCCESS! GO~~ \n")));

	if(datFile.Open(strRecvFileName, CFile::modeRead, &e))		// zip file 전송을 위해 file open
	{
		UINT  nSize = (UINT)datFile.GetLength();
		UINT nSendIndex = 0;
		BYTE *pBuffer = new BYTE[nSize+1];

		memset(pBuffer, 0, nSize+1);
		if( datFile.Read(pBuffer, nSize) == nSize)
		{
			if (nSize >= 3145728)	// MAX 3M(3*1024*1024)
			{
				delete [] pBuffer;
				datFile.Close();

				return FALSE;
			}

			// File Data Upload Start
			while(nSize > 0)
			{
				// Make Upload File Data
				// Make Upload Common Header Information
				memset(&TempUploadData, 0, sizeof(TempUploadData));
				RMS_MakeJNLImageDataMsg ((BYTE*)&TempUploadData, sizeof(TempUploadData));
			
				TempUploadData.Header.MsgID = RMS_REQ_JNL_IMAGE;
				TempUploadData.FrameNumber = (NUINT16)nFrameNumber++;

				TempUploadData.TotalImageCount = 1;	//CE는 저널 1개당 1컷만 찍음 
				TempUploadData.CurrentImageSeq = 1;	//CE는 저널 1개당 1컷만 찍음 


				if (nSize > sizeof(TempUploadData.FileData))
				{
					TempUploadData.FrameNumber |= 0x0000;
					TempUploadData.Header.Length = sizeof(TempUploadData) - 2;
					TempUploadData.UploadDataSize = sizeof(TempUploadData.FileData);
					memcpy(TempUploadData.FileData, &pBuffer[nSendIndex], sizeof(TempUploadData.FileData));
					nSendIndex = nSendIndex + sizeof(TempUploadData.FileData);
					nSize = nSize - sizeof(TempUploadData.FileData);
				}
				else
				{
					// Last Frame
					TempUploadData.FrameNumber |= 0x8000;		// Upload일 경우에는 0x1000이 아닌 0x8000을 사용함 (RMS 특이사항)
					TempUploadData.Header.Length = sizeof(TempUploadData) - sizeof(TempUploadData.FileData) + nSize - 2;
					TempUploadData.UploadDataSize = nSize;
					memcpy(TempUploadData.FileData, &pBuffer[nSendIndex], nSize);
					nSendIndex = nSendIndex + nSize;
					nSize = 0;
				}
									
				NHDEBUG(1, (_T("Send RMS_REQ_FILE_UPLOAD. Frame(%x) nSize(%d)TO RMS..\n"), TempUploadData.FrameNumber, nSize));
				
				if (RMS_Send((BYTE*)&TempUploadData, TempUploadData.Header.Length, 0) != RMS_OK)
				{
					NHDEBUG(1, (_T("Send Failed\n")));
					break;
				}
			}
		}
		delete [] pBuffer;
		datFile.Close();
	}

	return TRUE;
}
// end of [#523]
/////////////////////////////////////////////////////////////////////
//
//	RMS_ProcXXXXXXXX(void) 함수 처리시 공통사항.

BOOL CRmsCtrl::RMS_MakeStatusMsg(BYTE *pMsg, int Len)
{
	CString		strTemp;
	PRMS_RES_STATUS	pStatusMsg;

	// check buffer size.
	if (sizeof(RMS_RES_STATUS) > Len)
		return FALSE;

	pStatusMsg = (PRMS_RES_STATUS)pMsg;

	memset(pStatusMsg, 0, sizeof(RMS_RES_STATUS));

	// 2. Length : Status

	// 5. Frame
	pStatusMsg->FrameNumber = 0x0000;

	// 6. 시간 정보.
	SYSTEMTIME localTime;
	::GetLocalTime(&localTime);
	pStatusMsg->dwYear = localTime.wYear;
	pStatusMsg->dwMonth = localTime.wMonth;
	pStatusMsg->dwDay = localTime.wDay;
	pStatusMsg->dwHour = localTime.wHour;
	pStatusMsg->dwMinute = localTime.wMinute;
	pStatusMsg->dwSecond = localTime.wSecond;

	// 7. Machine Type
	pStatusMsg->MachineType = RMS_GetMachineType();

	// 8. Serial Number
	strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ATMSERIALNO);
	//if serial number is default value, ATM sends Terminal ID instead of it
	//Then RMS will identify the terminal ID
	if(strTemp == "0000000000") //default setting value = "0000000000"
		strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID);
	strTemp.TrimLeft();
	strTemp.TrimRight();
	RMS_WideToMulti(strTemp, (char*)pStatusMsg->SerialNumber, sizeof(pStatusMsg->SerialNumber));

	// 9. RMS Password.
	strTemp = MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSPASSWORD);
	strTemp.TrimLeft();
	strTemp.TrimRight();
	RMS_WideToMulti(strTemp, (char*)pStatusMsg->RMSPassword, sizeof(pStatusMsg->RMSPassword));
	
	// 10. Terminal Number.
	strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID);
	strTemp.TrimLeft();
	strTemp.TrimRight();
	RMS_WideToMulti(strTemp, (char*)pStatusMsg->TerminalNumber, sizeof(pStatusMsg->TerminalNumber));

	// 11. RMS Phone Number 1
	strTemp = MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSPHONENO1);
	RMS_WideToMulti(strTemp, (char*)pStatusMsg->RMSTel1, sizeof(pStatusMsg->RMSTel1));

	//     RMS Phone Number 2
	strTemp = MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSPHONENO2);
	RMS_WideToMulti(strTemp, (char*)pStatusMsg->RMSTel2, sizeof(pStatusMsg->RMSTel2));

	CST_STATUS	CSTStatus;
	GetCSTStatus(&CSTStatus);

	// 12. CST1 Denomination
	pStatusMsg->CST1Denomination = CSTStatus.CST1Denomination;

	// 13. CST1 Bill Count
	pStatusMsg->CST1BillCount = CSTStatus.CST1BillCount;

	// 14. CST1 Status 
	pStatusMsg->CST1Status = CSTStatus.CST1Status;

	// 15. CST2 Denomination
	pStatusMsg->CST2Denomination = CSTStatus.CST2Denomination;

	// 16. CST2 Bill Count
	pStatusMsg->CST2BillCount = CSTStatus.CST2BillCount;

	// 17. CST2 Status 
	pStatusMsg->CST2Status = CSTStatus.CST2Status;

	// 18. CST3 Denomination
	pStatusMsg->CST3Denomination = CSTStatus.CST3Denomination;

	// 19. CST3 Bill Count
	pStatusMsg->CST3BillCount = CSTStatus.CST3BillCount;

	// 20. CST3 Status 
	pStatusMsg->CST3Status = CSTStatus.CST3Status;

	// 21. CST4 Denomination
	pStatusMsg->CST4Denomination = CSTStatus.CST4Denomination;

	// 22. CST4 Bill Count
	pStatusMsg->CST4BillCount = CSTStatus.CST4BillCount;

	// 23. CST4 Status 
	pStatusMsg->CST4Status = CSTStatus.CST4Status;

	// 24. Journal Count
	if (m_pDevCmn->m_JNLMgr.GetUploadRemainCount(UP_RMS) > 0)
		pStatusMsg->JournalCount = (NUINT16)m_pDevCmn->m_JNLMgr.GetUploadRemainCount(UP_RMS);
	else
		pStatusMsg->JournalCount = 0;

	// 25. ATM Status.
	BYTE bATMStatus = (BYTE)MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ATMSTATUS);
	if (bATMStatus > ATM_RMSACTIVE) 
		bATMStatus = 0xff;
	else
		bATMStatus +=0x31;
	pStatusMsg->ATMStatus = bATMStatus;

	// 26. Error Code.
	// [#419] [NH] KSK 2008.9.11
	if (!m_pDevCmn->fnAPL_CheckError())
		sprintf((char*)pStatusMsg->ErrorCode, "%7.7S", m_pDevCmn->fstrAPL_GetErrorCode());
	else
		memcpy(pStatusMsg->ErrorCode, "0000000", 7);
	// end of [#419]
	strTemp = pStatusMsg->ErrorCode;

	// 27. AP Version
	strTemp = MemGetStr(_MEM_FLD_APP_VERSIONINFO, _MEM_VAR_APP_AP_VERSION);	// [#127] KSK 2008.04.21
	RMS_WideToMulti(strTemp, (char*)pStatusMsg->APVersion, sizeof(pStatusMsg->APVersion));

	// 28. RMS Version
	strTemp = MemGetStr(_MEM_FLD_APP_VERSIONINFO, _MEM_VAR_APP_RMS_VERSION);	// [#127] KSK 2008.04.21
	RMS_WideToMulti(strTemp, (char*)pStatusMsg->RMSVersion, sizeof(pStatusMsg->RMSVersion));
	
	// 29. Last Tran Date
	strTemp = MemGetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_LOCALTRANDATE);
	strTemp.TrimLeft();
	strTemp.TrimRight();
	if(strTemp.GetLength() == 8)
	{
		//YYYY
		pStatusMsg->LastTranDate[0] = Asc2Int(strTemp.Right(4));
		//MM
		pStatusMsg->LastTranDate[1] = Asc2Int(strTemp.Left(2));
		//DD
		pStatusMsg->LastTranDate[2] = Asc2Int(strTemp.Mid(2,2));
		//end of YID_64
	}
	else if(strTemp.GetLength() == 6)
	{
		pStatusMsg->LastTranDate[0] = 2000 + Asc2Int(strTemp.Right(2));
		//MM
		pStatusMsg->LastTranDate[1] = Asc2Int(strTemp.Left(2));
		//DD
		pStatusMsg->LastTranDate[2] = Asc2Int(strTemp.Mid(2,2));
	}
	else
	{	//default setting
		pStatusMsg->LastTranDate[0] = 2000;
		pStatusMsg->LastTranDate[1] = 01;
		pStatusMsg->LastTranDate[2] = 01;
	}
	
	// 2)Time
	strTemp = MemGetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_LOCALTRANTIME);
	if(strTemp.GetLength() == 6)
	{
		//HH
		pStatusMsg->LastTranDate[3] = Asc2Int(strTemp.Left(2));
		//MM
		pStatusMsg->LastTranDate[4] = Asc2Int(strTemp.Mid(2,2));
		//SS
		pStatusMsg->LastTranDate[5] = Asc2Int(strTemp.Right(2));
	}

	// 30. Device Status
	DWORD dwATMStatus = 0x00000000;
	DWORD dwDevStatus = 0x00000000;
	DWORD dwATMDevStatus = 0x00000000;
	int nSPRPaperStat = 0;
	// [#419] [NH] KSK 2008.9.11
	if (!m_pDevCmn->fnAPL_CheckError())
		dwATMStatus = 0x00000001; // out of service
	else
		dwATMStatus = 0x00000000; //In service
	// end of [#419]
	dwATMDevStatus = dwATMDevStatus | dwATMStatus;
	//OR (ATM | CDUCST status)
	dwATMDevStatus = dwATMDevStatus | CSTStatus.dwCDUCSTstatus;
//	nSPRPaperStat = MemGetInt(_MEM_FLD_INI_DEVINFO, _MEM_VAR_DEVINFO_SPRSP_PAPERSTAT);
	nSPRPaperStat = m_pDevCmn->fnSPR_GetPaperStatus();	// KSK 2009.12.23
	if(nSPRPaperStat == 1) // EMPTY
		dwDevStatus = 0x00000080; //Out of Receipt
	else if(nSPRPaperStat == 2) // Low
		dwDevStatus = 0x00000040; //Low Receipt
	else						  //if 0, Normal
		dwDevStatus = 0x00000000;
	//OR (ATM | CDUCST status | SPRPaper status)
	dwATMDevStatus = dwATMDevStatus | dwDevStatus;
	pStatusMsg->DeviceStatus = dwATMDevStatus;

	// 31. MWI Version
	strTemp = MemGetStr(_MEM_FLD_APP_VERSIONINFO, _MEM_VAR_APP_MWI_VERSION);	// [#127] KSK 2008.04.21
	RMS_WideToMulti(strTemp, (char*)pStatusMsg->MWIVer, sizeof(pStatusMsg->MWIVer));

	// 32. VDM Version
	strTemp = MemGetStr(_MEM_FLD_APP_VERSIONINFO, _MEM_VAR_APP_VDM_VERSION);	// [#127] KSK 2008.04.21
	RMS_WideToMulti(strTemp, (char*)pStatusMsg->VDMVer, sizeof(pStatusMsg->VDMVer));

	// 33. CDM SP Version
	strTemp = MemGetVersion(_MEMKEY_SPVERSION, L"CDM");
	RMS_WideToMulti(strTemp, (char*)pStatusMsg->CDMSPVer, sizeof(pStatusMsg->CDMSPVer));
	
	// 34. IDC SP Version
	strTemp = MemGetVersion(_MEMKEY_SPVERSION, L"IDC");
	RMS_WideToMulti(strTemp, (char*)pStatusMsg->IDCSPVer, sizeof(pStatusMsg->IDCSPVer));

	// 35. PIN SP Version
	strTemp = MemGetVersion(_MEMKEY_SPVERSION, L"PIN");
	RMS_WideToMulti(strTemp, (char*)pStatusMsg->PINPADSPVer, sizeof(pStatusMsg->PINPADSPVer));

	// 36. SIU SP Version
	strTemp = MemGetVersion(_MEMKEY_SPVERSION, L"SIU");
	RMS_WideToMulti(strTemp, (char*)pStatusMsg->SIUSPVer, sizeof(pStatusMsg->SIUSPVer));
	
	// 37. TTU SP Version
	memset(pStatusMsg->TTUSPVer, 0, sizeof(pStatusMsg->TTUSPVer));
	memcpy(pStatusMsg->TTUSPVer, "N/A", strlen("N/A"));

	// 38. VFD SP Version
	memset(pStatusMsg->VFDSPVer, 0, sizeof(pStatusMsg->VFDSPVer));
	memcpy(pStatusMsg->VFDSPVer, "N/A", strlen("N/A"));

	// 39. SPR SP Version
	strTemp = MemGetVersion(_MEMKEY_SPVERSION, L"SPR");
	RMS_WideToMulti(strTemp, (char*)pStatusMsg->SPRSPVer, sizeof(pStatusMsg->SPRSPVer));

	// 40. UPS SP Version
	memset(pStatusMsg->UPSSPVer, 0, sizeof(pStatusMsg->UPSSPVer));
	memcpy(pStatusMsg->UPSSPVer, "N/A", strlen("N/A"));

	// 41. PTR SP : 실제 PTR SP 없는데 Version structure에 정의 되어 있음.
	memset(pStatusMsg->PTRSPVer, 0, sizeof(pStatusMsg->PTRSPVer));
	memcpy(pStatusMsg->PTRSPVer, "N/A", strlen("N/A"));

	// 42. JPR SP Version
	memset(pStatusMsg->JPRSPVer, 0, sizeof(pStatusMsg->JPRSPVer));
	memcpy(pStatusMsg->JPRSPVer, "N/A", strlen("N/A"));

	// 43. CAM SP Version
	memset(pStatusMsg->CAMSPVer, 0, sizeof(pStatusMsg->CAMSPVer));
	memcpy(pStatusMsg->CAMSPVer, "N/A", strlen("N/A"));

	// 44. PNC SP : 실제 PNC SP 없는데 Version structure에 정의 되어 있음.
	memset(pStatusMsg->PNCSPVer, 0, sizeof(pStatusMsg->PNCSPVer));
	memcpy(pStatusMsg->PNCSPVer, "N/A", strlen("N/A"));

	// 45. COM SP Version
	memset(pStatusMsg->COMSPVer, 0, sizeof(pStatusMsg->COMSPVer));
	memcpy(pStatusMsg->COMSPVer, "N/A", strlen("N/A"));

	// 46. CDM EP Version
	strTemp = MemGetVersion(_MEMKEY_EPVERSION, L"CDM");
	RMS_WideToMulti(strTemp, (char*)pStatusMsg->CDMEPVer, sizeof(pStatusMsg->CDMEPVer));

	// 47. IDC EP Version
	strTemp = MemGetVersion(_MEMKEY_EPVERSION, L"IDC");
	RMS_WideToMulti(strTemp, (char*)pStatusMsg->IDCEPVer, sizeof(pStatusMsg->IDCEPVer));

	// 48. PIN EP Version
	strTemp = MemGetVersion(_MEMKEY_EPVERSION, L"PIN");
	RMS_WideToMulti(strTemp, (char*)pStatusMsg->PINPADEPVer, sizeof(pStatusMsg->PINPADEPVer));

	// 49. SIU EP Version
	memset(pStatusMsg->SIUEPVer, 0, sizeof(pStatusMsg->SIUEPVer));
	memcpy(pStatusMsg->SIUEPVer, "N/A", strlen("N/A"));

	// 50. TTU EP Version
	memset(pStatusMsg->TTUEPVer, 0, sizeof(pStatusMsg->TTUEPVer));
	memcpy(pStatusMsg->TTUEPVer, "N/A", strlen("N/A"));

	// 51. VFD EP Version
	memset(pStatusMsg->VFDEPVer, 0, sizeof(pStatusMsg->VFDEPVer));
	memcpy(pStatusMsg->VFDEPVer, "N/A", strlen("N/A"));

	// 52. SPR EP Version
	strTemp = MemGetVersion(_MEMKEY_EPVERSION, L"SPR");
	RMS_WideToMulti(strTemp, (char*)pStatusMsg->SPREPVer, sizeof(pStatusMsg->SPREPVer));

	// 53. UPS EP Version
	memset(pStatusMsg->UPSEPVer, 0, sizeof(pStatusMsg->UPSEPVer));
	memcpy(pStatusMsg->UPSEPVer, "N/A", strlen("N/A"));

	// 54. PTR EP : 실제 PTR EP 없는데 Version structure에 정의 되어 있음.
	memset(pStatusMsg->PTREPVer, 0, sizeof(pStatusMsg->PTREPVer));
	memcpy(pStatusMsg->PTREPVer, "N/A", strlen("N/A"));

	// 55. JPR EP Version
	memset(pStatusMsg->JPREPVer, 0, sizeof(pStatusMsg->JPREPVer));
	memcpy(pStatusMsg->JPREPVer, "N/A", strlen("N/A"));

	// 56. CAM EP Version
	memset(pStatusMsg->CAMEPVer, 0, sizeof(pStatusMsg->CAMEPVer));
	memcpy(pStatusMsg->CAMEPVer, "N/A", strlen("N/A"));

	// 57. PNC EP Version
	memset(pStatusMsg->PNCEPVer, 0, sizeof(pStatusMsg->PNCEPVer));
	memcpy(pStatusMsg->PNCEPVer, "N/A", strlen("N/A"));

	// 58. COM EP : 실제 COM EP 없는데 Version structure에 정의 되어 있음.
	memset(pStatusMsg->COMEPVer, 0, sizeof(pStatusMsg->COMEPVer));
	memcpy(pStatusMsg->COMEPVer, "N/A", strlen("N/A"));

	// 59. Sidecar RFID Status
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE) == ENABLE)
		pStatusMsg->SC_RFIDStatus = m_pDevCmn->fnRFID_GetDeviceStatus();
	else
		pStatusMsg->SC_RFIDStatus = NODEVICE;

	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_SUPPORT) == ENABLE)
	{
		// 60. Sidecar BCR Status
		pStatusMsg->SC_BCRStatus = m_pDevCmn->fnBCR_GetDeviceStatus();
	
		// 61. Sidecar CIM Status
		pStatusMsg->SC_CIMStatus = m_pDevCmn->fnBNA_GetAggregateDeviceStatus();

		// 62. Sidecar CST Bill Count
		pStatusMsg->SC_CSTBillCount = m_pDevCmn->m_nBnaNumberOfItem;

		// 63. Sidecar CST Status
		pStatusMsg->SC_CSTStatus = m_pDevCmn->m_nBnaCSTStatus;
	}
	else
	{
		// 60. Sidecar BCR Status
		pStatusMsg->SC_BCRStatus = NODEVICE;
	
		// 61. Sidecar CIM Status
		pStatusMsg->SC_CIMStatus = NODEVICE;

		// 62. Sidecar CST Bill Count
		pStatusMsg->SC_CSTBillCount = 0;

		// 63. Sidecar CST Status
		pStatusMsg->SC_CSTStatus = NODEVICE;
	}
	
	return TRUE;
}

BOOL CRmsCtrl::RMS_MakeSetupReadDataMsg(BYTE *pMsg, int Len)
{
	CString		strTemp;
	PRMS_SETUP_READ_DATA	pSetupRead;

	// check Buffer Size.
	if (sizeof(RMS_SETUP_READ_DATA) > Len)
		return FALSE;

	pSetupRead = (PRMS_SETUP_READ_DATA) pMsg;

	memset(pSetupRead, 0, sizeof(RMS_SETUP_READ_DATA));

	// 2. Length
	//pSetupRead->Header.Length = (4 + SIZE_RMS_SETUP_READ_DATA);

	// 5. FrameNumber
	

	// 6. Year-Month-Day, Hour-Miniute-Second
	SYSTEMTIME localTime;
	::GetLocalTime(&localTime);
	pSetupRead->dwYear = localTime.wYear;
	pSetupRead->dwMonth = localTime.wMonth;
	pSetupRead->dwDay = localTime.wDay;
	pSetupRead->dwHour = localTime.wHour;
	pSetupRead->dwMinute = localTime.wMinute;
	pSetupRead->dwSecond = localTime.wSecond;

	// 7. Machine Type
	pSetupRead->MachineType = RMS_GetMachineType();

	// 8. Serial Number
	strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ATMSERIALNO);
	//if serial number is default value, ATM sends Terminal ID instead of it
	//Then RMS will identify the terminal ID
	if(strTemp == "0000000000") //default setting value = "0000000000"
		strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID);
	strTemp.TrimLeft();
	strTemp.TrimRight();
	RMS_WideToMulti(strTemp, (char*)pSetupRead->SerialNumber, sizeof(pSetupRead->SerialNumber));

	CST_STATUS	CSTStatus;
	GetCSTStatus(&CSTStatus);

	// 9. CST1 Denomination
	pSetupRead->CST1Denomination = CSTStatus.CST1Denomination;

	// 10. CST1 Bill Count
	pSetupRead->CST1BillCount = CSTStatus.CST1BillCount;


	// 11. CST2 Denomination
	pSetupRead->CST2Denomination = CSTStatus.CST2Denomination;

	// 12. CST2 Bill Count
	pSetupRead->CST2BillCount = CSTStatus.CST2BillCount;


	// 13. CST3 Denomination
	pSetupRead->CST3Denomination = CSTStatus.CST3Denomination;

	// 14. CST3 Bill Count
	pSetupRead->CST3BillCount = CSTStatus.CST3BillCount;


	// 15. CST4 Denomination
	pSetupRead->CST4Denomination = CSTStatus.CST4Denomination;

	// 16. CST4 Bill Count
	pSetupRead->CST4BillCount = CSTStatus.CST4BillCount;


	// 17. Journal Count
	if (m_pDevCmn->m_JNLMgr.GetUploadRemainCount(UP_RMS) > 0)
		pSetupRead->JournalCount = (NUINT16)m_pDevCmn->m_JNLMgr.GetUploadRemainCount(UP_RMS);
	else
		pSetupRead->JournalCount = 0;

	// 18. ATM Status.
	BYTE bATMStatus = (BYTE)MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ATMSTATUS);
	
	if (bATMStatus > ATM_RMSACTIVE) 
		bATMStatus = 0xff;
	else
		bATMStatus +=0x31;

	pSetupRead->ATMStatus = bATMStatus;

	// 19. Error Code.
	// [#419] [NH] KSK 2008.9.11
	if (!m_pDevCmn->fnAPL_CheckError())
		sprintf((char*)pSetupRead->ErrorCode, "%7.7S", m_pDevCmn->fstrAPL_GetErrorCode());
	else
		memcpy(pSetupRead->ErrorCode, "0000000", 7);
	// end of [#419]
	return TRUE;
}

BOOL CRmsCtrl::RMS_MakeSetupDataMsg(BYTE *pMsg, int Len)
{
	int		nValue, nTemp, i;
	CString	strTemp;
	PRMS_SETUP_DATA	pSetupData;

	// check buffer size.
	if (sizeof(RMS_SETUP_DATA) > Len)
		return FALSE;

	pSetupData = (PRMS_SETUP_DATA) pMsg;
	memset(pSetupData, 0, sizeof(RMS_SETUP_DATA));

	//[R] 1. Terminal Number
	strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID);
	strTemp.TrimLeft();
	strTemp.TrimRight();
	RMS_WideToMulti(strTemp, (char*)pSetupData->TerminalNumber, sizeof(pSetupData->TerminalNumber));

	//[R] 2. Serial Number
	strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ATMSERIALNO);
	if(strTemp == "0000000000") //default setting value = "0000000000"
		strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID);
	strTemp.TrimLeft();
	strTemp.TrimRight();
	RMS_WideToMulti(strTemp, (char*)pSetupData->SerialNumber, sizeof(pSetupData->SerialNumber));

	//[R] 3. Routing ID
	strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ROUTINGID);
	RMS_WideToMulti(strTemp, (char*)pSetupData->RoutingID, sizeof(pSetupData->RoutingID));

	//[R] 4. Communication ID
	strTemp = MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_COMMUNICATIONID);
	RMS_WideToMulti(strTemp, (char*)pSetupData->CommunicationID, sizeof(pSetupData->CommunicationID));

	//[R] 5. Host Phone number 1 (Host Tel1)
	strTemp = MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_PHONENODEFINE1);
	RMS_WideToMulti(strTemp, (char*)pSetupData->HostTel1, sizeof(pSetupData->HostTel1));

	//[R] 6. Host Phone number 2 (Host Tel2)
	strTemp = MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_PHONENODEFINE2);
	RMS_WideToMulti(strTemp, (char*)pSetupData->HostTel2, sizeof(pSetupData->HostTel2));
	
	//[R] 7. Host Phone number 3 (Host Tel3) - not support
	memset(pSetupData->HostTel3, 0, sizeof(pSetupData->HostTel3));
	memcpy(pSetupData->HostTel3, "N/A", strlen("N/A"));

	//[R] 8. Modem Connect Timer
//	nValue = MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_CONNECTTIMER);	KSK 2009.12.23 사용 안함으로 수정
//	pSetupData->ModemConnetTimer = (NUINT16)nValue;

	//[R] 9. Modem Speaker Enable - H/W not support
//	nValue = MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_MODEMSPEAKER);	KSK 2009.12.23 사용 안함으로 수정
//	pSetupData->ModemSpeakerEnable = (NUINT8)nValue;

	//[R] 10. Modem Initial String.
	strTemp = MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_MODEMINITSTR);
	RMS_WideToMulti(strTemp, (char*)pSetupData->ModemInitialString, sizeof(pSetupData->ModemInitialString));

	//[R] 11. Modem Vendor
//	strTemp = MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_MODEMVENDOR);
	strTemp = L"C";		// KSK 2009.12.23 Fix Data
	RMS_WideToMulti(strTemp, (char*)pSetupData->ModemVendor, sizeof(pSetupData->ModemVendor));

	//[R] 12. ENQ Timeout Interval. KSK 2009.12.23 사용안함으로 수정
//	nValue = MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_ENQTIMEOUT);
//	pSetupData->ENQTimeoutInterval = (NUINT16)nValue;

	//[R] 13. EOT Check Mode
	strTemp = MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_EOTCHECKMODE);
	if(strTemp == EOT_OPTIONAL)
		pSetupData->EOTCheckMode = 2;
	else if(strTemp == EOT_REQUIRED)
		pSetupData->EOTCheckMode = 3;
	else if(strTemp == NO_EOT_REQUIRED)
		pSetupData->EOTCheckMode = 4;
	else if(strTemp == NO_ENQ_REQUIRED)
		pSetupData->EOTCheckMode = 5;
	else if(strTemp == NO_ENQ_EOT_REQUIRED) //[#556] SOOK 2009.08.12 NO ENQ/EOT REQUIRED 추가 
		pSetupData->EOTCheckMode  = 6;	//end of [#556]
	else
		pSetupData->EOTCheckMode = 1;

	//[R] 14. Processor Code
	strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE);
	if(strTemp == MSG_HYOSUNG_TYPE)
		pSetupData->ProcessorCode = 1;
	else if(strTemp == MSG_CSP200_TYPE)
		pSetupData->ProcessorCode = 2;
	else if(strTemp == MSG_TRITON_TYPE)
		pSetupData->ProcessorCode = 3;
	else if(strTemp == MSG_EPS_TYPE)
		pSetupData->ProcessorCode = 5;
	else
		pSetupData->ProcessorCode = 0;

	//[R] 15. Fromat Option
//	nValue = MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_MSGDEFINE_FORMATOPTION);	KSK 2009.12.23 사용 안함으로 변경
//	pSetupData->FormatOption = (NUINT8)nValue;

	//[R] 16. Health Check Send Enable
	nValue = MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_HEALTHYFLAG);
	pSetupData->HealthCheckSendEnable = (NUINT8)nValue;

	//[R] 17. Health Check Interval
	nValue = MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_HSDELAYINT);
	pSetupData->HealthCheckInterval = (NUINT16)nValue;

	//[R] 18. RMS Accept Enable
	nValue = MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSFLAG);
	pSetupData->RMSAcceptEnable = (NUINT8)nValue;

	//[R] 19. RMS Status Send Enable
	nValue = MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSSTATUSSEND);
	pSetupData->RMSStatusSendEnable = (NUINT8)nValue;

	//[R] 20. RMS Password
	strTemp = MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSPASSWORD);
	strTemp.TrimLeft();
	strTemp.TrimRight();
	if (strTemp.GetLength() == 0)
	{
		// Set Default Password.
		strTemp = _T("333333");
		MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSPASSWORD, (LPCTSTR)strTemp);
	}
	RMS_WideToMulti(strTemp, (char*)pSetupData->RMSPassword, sizeof(pSetupData->RMSPassword));

	//[R] 21. RMS New Password
	memset(pSetupData->RMSNewPassword, 0, sizeof(pSetupData->RMSNewPassword));
	memcpy(pSetupData->RMSNewPassword, "       ", 7);

	//[R] 22. RMS Phone Number 1 (RMS Tel1)
	strTemp = MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSPHONENO1);
	RMS_WideToMulti(strTemp, (char*)pSetupData->RMSTel1, sizeof(pSetupData->RMSTel1));

	//[R] 23. RMS Phone Number 2 (RMS Tel2)
	strTemp = MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSPHONENO2);
	RMS_WideToMulti(strTemp, (char*)pSetupData->RMSTel2, sizeof(pSetupData->RMSTel2));

	//[R] 24. RMS Ring Count - AP not support
	nValue = MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSRINGCNT);
	pSetupData->RMSRingCount = (NUINT8)nValue;

	//[R] 25. Speaker Volume.
	nValue = MemGetInt(_MEM_FLD_INI_DEVINFO, _MEM_VAR_DEVINFO_VOLUMELEVEL);
	pSetupData->SpeakerVolume = (NUINT8)nValue;

	//[R] 26. ISO 1 Enable
	nValue = MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ISO1_FLAG);
	pSetupData->ISO1Enable = (BYTE)nValue;

	//[R] 27. ISO 2 Enable
	nValue = MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ISO2_FLAG);
	pSetupData->ISO2Enable = (BYTE)nValue;

	//[R] 28. ISO 3 Enable
	nValue = MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ISO3_FLAG);
	pSetupData->ISO3Enable = (BYTE)nValue;

	//[R] 29. Surcharge Owner
	strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_OPSURCHARGEOWNER);
	RMS_WideToMulti(strTemp, (char*)pSetupData->SurchargeOwner, sizeof(pSetupData->SurchargeOwner));

	//[R] 30. Surcharge Enable
	nValue = MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHARGEENABLE);
	pSetupData->SurchargeEnable = (BYTE)nValue;

	//[R] 31. Surcharge Amount
	strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHAREAMT);
	//00000150인 경우 앞에 0를 제거한다.
	nTemp = strTemp.GetLength();
	for(i=0; i<nTemp; i++)
	{
		if(strTemp.Left(1) == "0")
			strTemp = strTemp.Right(strTemp.GetLength() - 1);
		else
			break;
	}
	nValue = Asc2Int(strTemp);
	pSetupData->SurchargeAmount = (NUINT16)nValue;

	// [#274] [NH] KSK 2008.6.10 RMS로 Fastcash값이 Fix값으로 올라가는 Bug Fix
	//[R] 32. Fast Cash Amount
	for(i=0; i<CDU_CST_MAXFASTCASH; i++)
		pSetupData->FastCashAmount[i] = (NUINT16)MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_FASTCASH1 + i);
	// end of [#274]

	//[R] 33. Max Dispense Amount
	strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_DISPENSELIMIT);
	nValue = Asc2Int(strTemp);
	pSetupData->MaxDispenseAmount = nValue;

	//[R] 34. Low Currency Check Enable
	nValue = MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_LOWCRCYCHECK);
	pSetupData->LowCurrencyCheckEnable = (NUINT8)nValue;

	// [#275] [NH] KSK 2008.6.10 RMS로 Language값이 Fix값으로 올라가는 Bug Fix
	//[R] 35. English Mode
	pSetupData->EnglishMode =	(NUINT8)MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ENGMODE);
	//[R] 36. Spanish Mode
	pSetupData->SpanishMode =	(NUINT8)MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SPNMODE);
	//[R] 37. Japanese Mode  - AP not Support
	pSetupData->JapaneseMode = (NUINT8)MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_JAPMODE);
	//[R] 38. French Mode
	pSetupData->FrenchMode =	(NUINT8)MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_FRNMODE);
	//[R] 39. Korean Mode  - AP not Support
	pSetupData->KoreanMode =	(NUINT8)MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_KORMODE);
	//[R] 40. Chinese Mode  - AP not Support
	pSetupData->ChineseMode =	(NUINT8)MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_CHNMODE);
	// end of [#275]
	
	//[R] 41. Select Receipt - AP not Support(Only Canada support)
	// US V01.03.04부터 적용
	pSetupData->SelectReceipt = (NUINT8)MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SELECTRECEIPT);

	//[R] 42. Receipt Enable - AP not Support
	pSetupData->ReceiptEnable = 1;

	//[R] 43. Receipt header 1-4
	for (i = 0; i < 4; i++)
	{
		strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_RECEIPT_HEADER1+i);
		if(strTemp.GetLength())
			RMS_WideToMulti(strTemp, (char*)pSetupData->ReceiptHeader[i], sizeof(pSetupData->ReceiptHeader[i]));
	}

	//[R] 44. Receipt Address 1-3
	for (i = 0; i < 3; i++)
	{
		strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_RECEIPT_ADDRESS1+i);
		if(strTemp.GetLength())
			RMS_WideToMulti(strTemp, (char*)pSetupData->ReceiptAddress[i], sizeof(pSetupData->ReceiptAddress[i]));
	}

	//[R] 45. Receipt Phone No
	strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_RECEIPT_PHONENO);
	if(strTemp.GetLength())
		RMS_WideToMulti(strTemp, (char*)pSetupData->ReceiptPhoneNo, sizeof(pSetupData->ReceiptPhoneNo));

	//[R] 46. VFD Display Option - not support
	//[R] 47. VFD Font Option - not support
	//[R] 48. VFD Text - not support


	/** [#RWC6-56] US William 2019.12.26 Hash passwords
	 *	The passwords are no longer stored in plaintext, so they will not be sent to MoniView
	 */
	//[R] 49. Master Password
	sprintf((char*)pSetupData->MasterPassword, "******");
	
	//[R] 50. Service Password
	sprintf((char*)pSetupData->ServicePassword, "******");

	//[R] 51. Operator Password
	sprintf((char*)pSetupData->OperatorPassword, "******");
	
	//[R] 52. Machine Type
	pSetupData->MachineType = CD;
	//[R] 53. SPR Type
	pSetupData->SPRType = SPR_T_TYPE;
	//[R] 54. JPR Type
	pSetupData->JPRType = NOT_SET;
	//[R] 55. MCU Type
	pSetupData->MCUType = MCU_D_TYPE;
	//[R] 56. CDU Type
	pSetupData->CDUType = CDU_SET;
	//[R] 57. PB Type
	pSetupData->PBType = NOT_SET;
	//[R] 58. Key Type
	if (!(m_pDevCmn->m_pConfig->GetOSVersion() == NH_OS_NH2700_L || m_pDevCmn->m_pConfig->GetOSVersion() == NH_OS_NH2700_T_T))		// [#11] NH KSK 2010.10.1 [#2064] NH KSK 2011.05.18
		pSetupData->KeyType = DRIVER_TYPE;
	else
		pSetupData->KeyType = TOUCH_TYPE;
	//[R] 59. Sound Type
	pSetupData->SoundType = SOUND_SET;
	//[R] 60. Camera Type
	pSetupData->CameraType = NOT_SET;
	//[R] 61. IC Card Type (1: support, 0: not support
	if (m_pDevCmn->GetDeviceType(L"MCU") == MCU_MAGTEK_DIP || m_pDevCmn->GetDeviceType(L"MCU") == MCU_SANKYO_DIP) //[#568] SOOK 2009.09.29 MCU Manaufacrue 변경	[#2073] NH KSK 2011.06.24
		pSetupData->ICCardType = 1;
	else
		pSetupData->ICCardType = NOT_SET;
	//[R] 62. Power Type
	pSetupData->PowerType = NOT_SET;

	// [#2060] NH KSK 2011.05.13
	//[R] 63. SPL Device
//	if (m_pDevCmn->m_pConfig->GetOSVersion() == NH_OS_NH2700_F_T || m_pDevCmn->m_pConfig->GetOSVersion() == NH_OS_NH2700_T_T)	// [#2064] NH KSK 2011.05.18	
	if (m_pDevCmn->GetDeviceType(L"SPL") == SPL_SET)		// [#GLDV-2505] Support MX2800T
		pSetupData->SPLDevice = SPL_SET;
	// end of [#2060]

	//[R] 64. VFD Device - not support

	//[R] 65. Dual Monitor
//	if (m_pDevCmn->m_pConfig->GetOSVersion() == NH_OS_NH2700_F_T || m_pDevCmn->m_pConfig->GetOSVersion() == NH_OS_NH2700_T_T)	// [#2064] NH KSK 2011.05.18	
	if (m_pDevCmn->GetDeviceType(L"SPL") == SPL_SET)		// [#GLDV-2505] Support MX2800T
		pSetupData->DualMonitor = DUAL_LCD;
	else
		pSetupData->DualMonitor = ONE_LCD;
	
	//[R] 66. DVR Device - not support

	//[R] 67. OP Device
//	if (m_pDevCmn->m_pConfig->GetOSVersion() == NH_OS_NH2700_F_T || m_pDevCmn->m_pConfig->GetOSVersion() == NH_OS_NH2700_T_T)	// [#2064] NH KSK 2011.05.18	
	if (m_pDevCmn->GetDeviceType(L"SPL") == SPL_SET)		// [#GLDV-2505] Support MX2800T
		pSetupData->OPDevice = OP_REAR;
	else
		pSetupData->OPDevice = OP_FRONT;

	//[R] 68. MCU Manufacturer - not support
	// 1: Motor OMROM, 2: Motor KDE, 3: Motor Hyosung, 4: Motor Sankyo, 5: Dip Magtek, 6: Dip Hyosung	7: Dip Sankyo2008.08.25 추가 PSC
	if (m_pDevCmn->GetDeviceType(L"MCU") == MCU_MAGTEK_DIP) //[#568] SOOK 2009.09.29 MCU Manaufacrue 변경
		pSetupData->MCUManufacturer = 5;
	else if (m_pDevCmn->GetDeviceType(L"MCU") == MCU_SANKYO_DIP)	// [#2106] NH KSK 2011.12.20
		pSetupData->MCUManufacturer = 7;
	else
		pSetupData->MCUManufacturer = 6;
	//[R] 69. CDU Shutter - not support
	
	//[R] 70. Ada Device
#if (US_VERSION || AU_VERSION)		// [#2220] AU KMK 2014.03.01 AU ADA 지원함
	pSetupData->AdaDevice = 1;	// ADA_WAV
#else
	pSetupData->AdaDevice = 0;	// Not Set
#endif

	//[R] 71. Cash Accepter - not support
	//[R] 72. Deposit Device - not support

#if (US_VERSION || CA_VERSION || MX_VERSION)	// [#622] NH KSK 2010.02.24	// [#2115] MX KSK 2012.02.05
	//[R] 73. BIN List count
	nValue = MemGetInt(_MEM_FLD_APP_BININFO, _MEM_VAR_APP_SETBINLIST);
	pSetupData->BINListCount = (NUINT16)nValue;

	//[R] 74. BIN LIST
	for (i = 0; i < pSetupData->BINListCount; i++)
	{
		strTemp = MemGetStr(_MEM_FLD_APP_BININFO, _MEM_VAR_APP_BINLIST+i);
		if(strTemp.GetLength())
			RMS_WideToMulti(strTemp, (char*)pSetupData->BINList[i], sizeof(pSetupData->BINList[i]));
	}
#endif

	//[R] 75. RMS Send Interval
	pSetupData->RMSSendInterval = MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_RMSSENDINTERVAL);

	//[R] 76. RMS Journal Upload with Status - AP not Support
	//[R] 77. CDU Low Count - not support
	//[R] 78. RMS Only Callback - not support
	//[R] 79. RMS Callback Retry Count - not support

	//[R] 80. DHCP Enable
	nValue = MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_DHCP);
	pSetupData->DHCPEnable = (NUINT8)nValue;

	//[R] 81. ATM IP
	strTemp = MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_IPADDR);
	if (strTemp.GetLength() > 0)
		RMS_WideToMulti(strTemp, (char*)pSetupData->ATMIP, sizeof(pSetupData->ATMIP));

	//[R] 82. SubnetMask
	strTemp = MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_SUBNET);
	if (strTemp.GetLength() > 0)
		RMS_WideToMulti(strTemp, (char*)pSetupData->SubnetMask, sizeof(pSetupData->SubnetMask));

	//[R] 83. GateWay
	strTemp = MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_GATEWAY);
	if (strTemp.GetLength() > 0)
		RMS_WideToMulti(strTemp, (char*)pSetupData->GateWay, sizeof(pSetupData->GateWay));

	//[R] 84. DNS Server
	strTemp = MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_DNS);
	if (strTemp.GetLength() > 0)
		RMS_WideToMulti(strTemp, (char*)pSetupData->DNSServer, sizeof(pSetupData->DNSServer));

	//[R] 85. IsHostUseURL
	nValue = MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ISHOSTUSEURL);
	pSetupData->IsHostUseURL = (NUINT8)nValue;

	//[R] 86. Host1 Name
	strTemp = MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_HOST1NAME);
	if (strTemp.GetLength() > 0)
		RMS_WideToMulti(strTemp, (char*)pSetupData->HostName1, sizeof(pSetupData->HostName1));

	//[R] 87. Host1 Port No
	nValue = MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_HOST1PORTNO);
	pSetupData->HostPort1 = (NUINT16)nValue;

	//[R] 88. Host2 Name
	strTemp = MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_HOST2NAME);
	if (strTemp.GetLength() > 0)
		RMS_WideToMulti(strTemp, (char*)pSetupData->HostName2, sizeof(pSetupData->HostName2));

	//[R] 89. Host2 Port No
	nValue = MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_HOST2PORTNO);
	pSetupData->HostPort2 = (NUINT16)nValue;

	//[R] 90. Network Type
	nValue = MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE);
	pSetupData->NetworkType = (NUINT8)nValue;

	//[R] 91. SSL Enable
	nValue = MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_SSLENABLE);
	pSetupData->SSLEnable = (NUINT8)nValue;

	//[R] 92. TCP Type
	nValue = MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_TYPE);
	pSetupData->TCPType = (NUINT8)nValue;

	//[R] 93. CRC Enable
	nValue = MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_CRCENABLE);
	pSetupData->CRCEnable = (NUINT8)nValue;

	//[R] 94. IsRMSUseUrl
	nValue = MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ISRMSUSEURL);
	pSetupData->IsRMSUseURL = (NUINT8)nValue;

	//[R] 95. RMS Connection Type
	//[R] TCP/IP 사양 변경으로 사용하지 않음. Default(1)
	pSetupData->RMSConnectType = (NUINT8)1;

	//[R] 96. RMS Name
	strTemp = MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_RMSNAME);
	if (strTemp.GetLength() > 0)
		RMS_WideToMulti(strTemp, (char*)pSetupData->RMSName, sizeof(pSetupData->RMSName));

	//[R] 97. RMS Inbound Port No
	nValue = MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_RMSPORTNO);
	pSetupData->RMSInboundPort = (NUINT16)nValue;

	//[R] 98. RMS Outbound Port No
	nValue = MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_RMSSVRPORTNO);
	pSetupData->RMSOutboundPort = (NUINT16)nValue;

	//[R] 99. Advertisement Guide Display Term
	nValue = MemGetInt(_MEM_FLD_ADVINFO, _MEM_VAR_ADV_GUIDEDISP_TIME);
	pSetupData->AdvGuideTerm = (NUINT16)nValue;

	//[R] 100. Advertisement Enable
	for (i = 0, nValue = 0; i < 6; i++)
	{
		if (MemGetInt(_MEM_FLD_ADVINFO, _MEM_VAR_ADV_ENDISFLAG1+i))
			nValue |= (1 << i);
	}
	pSetupData->AdvEnable = (NUINT8)nValue;

	//[R] 101. advertisement Title
	for (i = 0; i < 6; i++)
	{
		strTemp = MemGetStr(_MEM_FLD_ADVINFO, _MEM_VAR_ADV_TITLE1 + (i*3));
		if (strTemp.GetLength() > 0)
			RMS_WideToMulti(strTemp, (char*)pSetupData->AdvTitle[i], sizeof(pSetupData->AdvTitle[i]));
	}

	//[R] 102. Coupon Enable
	for (i = 0, nValue = 0; i < 6; i++)
	{
		if (MemGetInt(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON1_ENDISFLAG+i))
			nValue |= (1 << i);
	}
	pSetupData->AdvCouponEnable = (NUINT8)nValue;

	//[R] 103 - 108
	for (i = 0; i < 2; i++)
	{
		//[R] 103. coupon1 msg 1,2
		strTemp = MemGetStr(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON1_TEXT1+i);
		if (strTemp.GetLength() > 0)
			RMS_WideToMulti(strTemp, (char*)pSetupData->AdvCouponMsg1[i], sizeof(pSetupData->AdvCouponMsg1[i]));

		//[R] 104. coupon2 msg 1,2
		strTemp = MemGetStr(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON2_TEXT1+i);
		if (strTemp.GetLength() > 0)
			RMS_WideToMulti(strTemp, (char*)pSetupData->AdvCouponMsg2[i], sizeof(pSetupData->AdvCouponMsg2[i]));

		//[R] 105. coupon3 msg 1,2
		strTemp = MemGetStr(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON3_TEXT1+i);
		if (strTemp.GetLength() > 0)
			RMS_WideToMulti(strTemp, (char*)pSetupData->AdvCouponMsg3[i], sizeof(pSetupData->AdvCouponMsg3[i]));

		//[R] 106. coupon4 msg 1,2
		strTemp = MemGetStr(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON4_TEXT1+i);
		if (strTemp.GetLength() > 0)
			RMS_WideToMulti(strTemp, (char*)pSetupData->AdvCouponMsg4[i], sizeof(pSetupData->AdvCouponMsg4[i]));

		//[R] 107. coupon5 msg 1,2
		strTemp = MemGetStr(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON5_TEXT1+i);
		if (strTemp.GetLength() > 0)
			RMS_WideToMulti(strTemp, (char*)pSetupData->AdvCouponMsg5[i], sizeof(pSetupData->AdvCouponMsg5[i]));

		//[R] 108. coupon6 msg 1,2
		strTemp = MemGetStr(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON6_TEXT1+i);
		if (strTemp.GetLength() > 0)
			RMS_WideToMulti(strTemp, (char*)pSetupData->AdvCouponMsg6[i], sizeof(pSetupData->AdvCouponMsg6[i]));
	}

	// [#571] NH KSK 2009.10.05 공통 Logic화
	if (m_pDevCmn->fnMCU_IsEmvAvailable())	// [#494] [NH] KSK 2009.2.6 EMV 지원이 안될 경우 정보도 올리지 않는다.
	{
		// [#79] NH PSC 2008.04.14 AID List RMS setup 추가
		//[R] 109. AID List(for EMV) - [UK][NZ]
		memset(pSetupData->AIDList, 0x00 ,sizeof(pSetupData->AIDList));

		// [#2342] US Justin 2015.05.08 Upload Enabled AID from NVRAM
		//char szIndex[12];

		//GetIniString(GET_EMV_AID_WORKFILENAME(m_pDevCmn->fnEMV_GetKernelVersion()), "AIDCOUNT", "TOTALCOUNT", szIndex);	// [#2226] NH Justin 2013.10.22 Load term data depends on Kernel Version
		//int nList = min(Asc2Int(szIndex, sizeof(szIndex)), sizeof(pSetupData->AIDList)/sizeof(pSetupData->AIDList[0]));
		//for(i = 0; i < nList; i++)
		//{
		//	memset(szIndex, 0x00, sizeof(szIndex));
		//	sprintf(szIndex,"TERMINAL%d",i+1);
		//	GetIniString(GET_EMV_AID_WORKFILENAME(m_pDevCmn->fnEMV_GetKernelVersion()), szIndex, "AID", (char*)&pSetupData->AIDList[i][0]);		// [#2226] NH Justin 2013.10.22 Load term data depends on Kernel Version
		//}

		// For Moniview Backward compatibility for Other countries.....
		#if (RMS_VERSION >= 0x010326)
			// Enabled AID Buffer
			BYTE bufAID[sizeof(pSetupData->AIDList)];
			memset(bufAID, 0x00 ,sizeof(bufAID));

			// Get Enabled AID String
			strTemp = MemGetStr(_MEM_FLD_EMVCONFIG, _MEM_VAR_EMV_CONFIG_ENABLEDAID);
			if(strTemp.GetLength() > sizeof(bufAID) )
				strTemp = strTemp.Left(sizeof(bufAID));

			// Convert to multi byte
			RMS_WideToMulti(strTemp, (char*)bufAID, sizeof(bufAID));

			// replace AID_FIELD_DELIMITER with NULL
			for(int nALoc=0; nALoc<sizeof(bufAID); nALoc++)
			{
				if(bufAID[nALoc] == AID_FIELD_DELIMITER)
					bufAID[nALoc] = 0;
			}

			// Copy to Setup Buffer
			memcpy( pSetupData->AIDList, bufAID,sizeof(bufAID));
		#else
			char szIndex[12];

			GetIniString(GET_EMV_AID_WORKFILENAME(m_pDevCmn->fnEMV_GetKernelVersion()), "AIDCOUNT", "TOTALCOUNT", szIndex);	// [#2226] NH Justin 2013.10.22 Load term data depends on Kernel Version
			int nList = min(Asc2Int(szIndex, sizeof(szIndex)), sizeof(pSetupData->AIDList)/sizeof(pSetupData->AIDList[0]));
			for(i = 0; i < nList; i++)
			{
				memset(szIndex, 0x00, sizeof(szIndex));
				sprintf(szIndex,"TERMINAL%d",i+1);
				GetIniString(GET_EMV_AID_WORKFILENAME(m_pDevCmn->fnEMV_GetKernelVersion()), szIndex, "AID", (char*)&pSetupData->AIDList[i][0]);		// [#2226] NH Justin 2013.10.22 Load term data depends on Kernel Version
			}
		#endif
		// End of [#2342]
		// end of [#79]
	}
	// end of [#571]
	
	//[R] 110. Encryption Key
	pSetupData->EncryptionKey = MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE);

#if (US_VERSION || CA_VERSION || AU_VERSION)		// [#@2220] AU KMK 2014.03.01 Percent Surcharge AU 추가
	//[R] 111. Percent Surcharge Enable - [US][CA]
	pSetupData->PercentSurchargeEnable = MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PERSURCHARGE_ENABLE);
	//[R] 112. Surcharge Warning Manner - [US][CA]
	pSetupData->SurchargeWarningManner = MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PERSURCHARGE_MANNER);
	//[R] 113. Percent Surcharge Amount - [US][CA]
	pSetupData->PercentSurchargeAmount = MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PERSURCHARGE_PERCENT);
#endif

// [#525] US KSK 2009.05.25
#if (US_VERSION || AU_VERSION || CA_VERSION)	// RMS VERSION V010309부터 지원		// AIREAT, AU/NZ 지원.	// [#548] CA KSK 2009.07.24	// [#2069] NH KSK 2011.06.13
	//[R] 114. Pre-dial Enable - [UK] - not implement
	pSetupData->PreDialEnable = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PREDIALENDISABLE);
	//[R] 115. Pre-dial Option - [UK] - not implement
	pSetupData->PreDialOption = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PREDIALOPTION);
#endif
// end of [#525]

#if (US_VERSION || MX_VERSION)	// [#2115] MX KSK 2012.02.05
	//[R] 116. Balance at First Enable - [US] [MX]
	pSetupData->BalanceAtFirstEnable = MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PREBALANCE);
#endif

	//[R] 117. Fastcash at First Enable - [XP] - not support

	//[R] 118. Mode10 Enable
	pSetupData->Mode10Enable = MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_MOD10_ENABLE);

	//[R] 119. Status Monitoring
	pSetupData->StatusMonitoring = MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_STATUSMONITORINGENABLE);

	//[R] 120. Surcharge Owner Print Enable - [XP] - not support

	// US, CA는 V01.03.07부터 지원함
	//[R] 121. Auto Day Total Enable - [NZ] - 
	pSetupData->AutoDayTotalEnable = MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_AUTODAYTOTAL_ENABLE);

	//[R] 122. Auto Day Total Time - [NZ] - Default : 0000
	strTemp.Format(L"%02d%02d", MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_AUTODAYTOTAL_TIME_HOUR),
								MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_AUTODAYTOTAL_TIME_MIN));
	if (strTemp.GetLength() > 0)
		RMS_WideToMulti(strTemp, (char*)pSetupData->AutoDayTotalTime, sizeof(pSetupData->AutoDayTotalTime));
	
	//[R] 123. Last Day Total Date - [NZ] Default : 20000101
	strTemp = MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_DAYTOTAL_LASTDATE);
	if (strTemp.GetLength() > 0)
		RMS_WideToMulti(strTemp, (char*)pSetupData->LastDayTotalDate, sizeof(pSetupData->LastDayTotalDate));

	if (strlen((const char*)pSetupData->LastDayTotalDate) < 8 || strcmp((const char*)pSetupData->LastDayTotalDate, "00000000") == 0)
	{
		strcpy((char*)pSetupData->LastDayTotalDate, "20000101");
	}
	// end of [#416]

	// US는 V01.03.04부터 지원함
	// [#192] UK AIREAT 2008.5.16
	//[R] 124. Welcome Message	- [UK][AU][CA] (3개만 지원)
	for (i = 0; i < 3; i++)
	{
		strTemp = MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_WELCOME_MESSAGE1+i);
		if (strTemp.GetLength() > 0)
			RMS_WideToMulti(strTemp, (char*)pSetupData->WelcomeMessage[i], sizeof(pSetupData->WelcomeMessage[i]));
	}
	// end of [#192]

#if (AU_VERSION) // [#2031] NZ KJW 2011.03.16	// [#2069] NH KSK 2011.06.13
	// [#337] AU AIREAT 2008.06.25
	for (i = 0; i < 4; i++)
	{
		//[R] 125. Store Message - [AU]
		strTemp = MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_STORE_MESSAGE1+i);
		if (strTemp.GetLength() > 0)
			RMS_WideToMulti(strTemp, (char*)pSetupData->StoreMessage[i], sizeof(pSetupData->StoreMessage[i]));

		//[R] 126. Processor Message - [AU]
		strTemp = MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PROCESSOR_MESSAGE1+i);
		if (strTemp.GetLength() > 0)
			RMS_WideToMulti(strTemp, (char*)pSetupData->ProcessorMessage[i], sizeof(pSetupData->ProcessorMessage[i]));

		//[R] 127. Marketing Message - [AU]
		strTemp = MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_MARKETING_MESSAGE1+i);
		if (strTemp.GetLength() > 0)
			RMS_WideToMulti(strTemp, (char*)pSetupData->MarketingMessage[i], sizeof(pSetupData->MarketingMessage[i]));
	}
	// end of [#337]
#endif

// [#521] US KSK 2009.05.18 DevCmn의 SetOwner에서 보정처리하므로 여기서는 NVRAM값 그대로 전송하도록 수정
	//[R] 128. Checking Account - [UK][AU]
	pSetupData->CheckingAccount = MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CHECKING);

	//[R] 129. Saving Account - [UK][AU]
	pSetupData->SavingAccount = MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_SAVINGS);

	//[R] 130. Credit Account - [UK][AU]
	pSetupData->CreditAccount = MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CREDITCARD);
// end of [#521]

#if (RMS_VERSION >= 0x010301)	// [#286] [NH] KSK 2008.6.12
	// [#514] [MX] KSK 2009.3.12
// 	#if (MX_VERSION)
// 		// [#284] [MX] KSK 2008.6.11
// 		//[R] 132. Surcharge (Cash)
// 		pSetupData->CashWithdrawalSurcharge = MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_WITHDRAWAL_SURCHARGE);
// 		//[R] 133. Surcharge (Balance)
// 		pSetupData->BalanceSurcharge = MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_BALANCE_SURCHARGE);
// 		//[R] 134. Surcharge (PIN Change)
// 		pSetupData->PINChangeSurcharge = MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PINCHANGE_SURCHARGE);
// 	#endif
		// end of [#284]

	// [#441] [US] KSK 2008.11.03
	#if (US_VERSION || CA_VERSION || MX_VERSION)		// 2008.07.03 by PSC canada default language 추가.	// [#2115] MX KSK 2012.02.05
		// V01.03.04부터 지원
		//[R] 135. Default Language
		pSetupData->DefaultLanguage = MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_DEFAULT_LANGUAGE);
	#endif

	#if (US_VERSION || MX_VERSION)	// [#2115] MX KSK 2012.02.05
		// V01.03.07부터 지원
		//[R] 136. Continue After Pre-Balance
		pSetupData->ContinueAfterPreBalance = MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_CONTINUETRANS_ENABLE);
	#endif						// end of [#311]
	// end of [#514]
#endif						// end of [#286]

#if (RMS_VERSION >= 0x010303)
	#if (AU_VERSION) // [#2031] NZ KJW 2011.03.16	// [#337] AU AIREAT 2008.06.25	// [#2069] NH KSK 2011.06.13
		// [R] 137. Exit Message
		for (i = 0; i < 3; i++)
		{
			strTemp = MemGetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_EXIT_MESSAGE1+i);
			if (strTemp.GetLength() > 0)
				RMS_WideToMulti(strTemp, (char*)pSetupData->ExitMessage[i], sizeof(pSetupData->ExitMessage[i]));
		}
	#endif

		// [R] 138. Inquiry Surcharge
	#if (AU_VERSION)
		pSetupData->nInquirySurchargeAmount = MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_INQUIRY_SURCHARGE);
	#endif				// end of [#337]
#endif

#if (RMS_VERSION >= 0x010304)	// [#366] [NH] KSK 2008.7.15
	#if (US_VERSION)
		// [R] 139. CheckCashing Enable
		pSetupData->CheckCashing = 0;
		// [R] 140. CheckCashing Pin Option Enable
		pSetupData->PINOption = MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PINOPTION_ENABLE);
		// [R] 141. CheckCashing Available Enable
		pSetupData->CheckCashingAvailable = 0;	// [#378] [NH] KSK 2008.7.18
	#endif
#endif							// end of [#366]

#if (RMS_VERSION >= 0x010305)	// [#397] [NH] psc 2008.08.01 EMV 관련 옵션 추가.
	#if (CA_VERSION || MX_VERSION || AU_VERSION || US_VERSION)			// 국가별 Define이 되어야만 한다	// [#2115] MX KSK 2012.02.05 // [#2220] AU KMK 2014.03.01 AU 추가 [#2261] US Justin Add US (EMV Function)
	// [R] 142. EMV Available
	pSetupData->EmvAvailable = MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_EMV_AVAILABLE);
	// [R] 143. EMV Enable
	pSetupData->EmvEnable = MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_EMV_ENABLE);

	// [R] 144. Surcharge Warning Location
	#if (CA_VERSION || US_VERSION)	// [#2261] US Justin Add US
	pSetupData->nSurchargeWarningLocation = MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_SURCHARGE_WARNINGLOCATION);
	#endif

	// [R] 145. EMV transaction option(MS/IC first) - 0: MS first, 1: IC first
	pSetupData->EmvTransactionOption = MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_EMV_TRANSACTIONOPTION);
	#endif

	//[#2261] US Justin Add US (EMV Function)
//	#elif (US_VERSION)	// [#443] [US] KSK 2008.11.05
//	// V01.03.07부터 지원
//	pSetupData->nSurchargeWarningLocation = MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_SURCHARGE_WARNINGLOCATION);
//	#endif
	// End of [#2261]
#endif							// end of [#397]

#if (RMS_VERSION >= 0x010307)
		// [R] 146 - Auto Day Total Type
	pSetupData->AutoDayTotalType = MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_AUTODAYTOTAL_TYPE);

	#if (AU_VERSION) // [#2031] NZ KJW 2011.03.16 NZ지원여부 확인필요!	// [#2069] NH KSK 2011.06.13
		// [R] 147 - Surcharge Owner Contact Info
		for (i = 0; i < 3; i++)
		{
			strTemp = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SURCHARGEOWNER_CONTACT1+i);
			if (strTemp.GetLength() > 0)
				RMS_WideToMulti(strTemp, (char*)pSetupData->SurchargeOwnerContactInfo[i], sizeof(pSetupData->SurchargeOwnerContactInfo[i]));
		}
	#endif
	// end of [#434]
#endif

// [#2115] MX KSK 2012.02.05
#if (RMS_VERSION >= 0x010308)
	#if (MX_VERSION)
		// [#2137] MX PCS 2012.07.17 환율 주석 처리함.
// 		pSetupData->CurrencyExchangeEnable = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MULTI_CURRENCY_ENABLE);
// 		strTemp = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EXCHANGE_RATE);
// 		if (strTemp.GetLength() > 0)
// 			RMS_WideToMulti(strTemp, (char*)pSetupData->CurrencyExchangeRate, sizeof(pSetupData->CurrencyExchangeRate));

		strTemp = m_pDevCmn->m_strCurrencyID;	// 현재는 CST1에서만 Currency ID를 가져온다(추후 카세트별 Multi Currency 지원이 필요할 것으로 보임)

		for(i = 0; i<m_pDevCmn->CSTCnt; i++)
		{
			if (strTemp.GetLength() > 0)
				RMS_WideToMulti(strTemp, (char*)pSetupData->CSTCurrencyID[i], sizeof(pSetupData->CSTCurrencyID[i]));
		}
	#endif
#endif
// end of [#2115]

// [#528] AU AIREAT 2009.06.02
#if (RMS_VERSION >= 0x010310)
	// [R] 151 - Need More Time
	#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
		pSetupData->NeedMoreTime = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_NEEDMORETIME);
	#else
		pSetupData->NeedMoreTime = 0;
	#endif
#endif
// end of [#528]

// [#546] SOOK 2009.07.23 다기능 BIN LIST 기능 추가함 
#if (RMS_VERSION >= 0x010311)
	// [R] 152 - AWARD COUPON ENABLE
	#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
		for ( i = 0; i < 6; i++)
			pSetupData->AwardCouponEnable[i] = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_AWARD_COUPON1_ENABLE + i);
		// [R] 153 - MULTIPLE BIN AVAILAVLE
		pSetupData->MultipleBinAvailable = 1;
	#else
		for ( i = 0; i < 6; i++)
			pSetupData->AwardCouponEnable[i] = 0;
		// [R] 153 - MULTIPLE BIN AVAILAVLE
		pSetupData->MultipleBinAvailable = 0;
	#endif
#endif
// end of [#546]
		
// [#558] KSK 2009.08.17 STANDARD1 OPTION 기능 추가
#if (RMS_VERSION >= 0x010312)
	// [R] 154 - TERMINAL STATUS FIELD ENABLE
	pSetupData->TerminalStatusFieldEnable = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_TERMINAL_STATUS_FIELD_ENABLE);

	// [R] 155 - REVERSAL WHEN PROTOCOL ERROR
	pSetupData->ReversalAtHostError = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_REVERSAL_AT_HOSTERROR);

	// [R] 156 - STANDARD1 OPTION AVAILABLE
	pSetupData->Standard1OptionAvailable = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_STANDARD1OPTION_AVAILABLE);
#endif
// end of [#558]

#if (RMS_VERSION >= 0x010313)
	// [R] 157 ~ 164 : WINXP SETUP 정보임

	// [R] 165 - PAPER LOW SENSOR ENABLE
	pSetupData->PaperLowSensorEnable  = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PAPER_LOW_SENSOR_ENABLE);	//[#596] SOOK 2009.12.11 SPR PAPER LOW SENSOR EN/DISABLE 기능 추가

	// [R] 166 - EPP Flicker Option
	pSetupData->EPPFlickerOption = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EPP_FLICKER_OPTION);	// [#604] NH KSK 2010.01.05

	// [R] 167 - CST Sound Option
	pSetupData->CSTSoundOption = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST_SOUND_OPTION);	// [#604] NH KSK 2010.01.05

	// [R] 168 - Emv Latch Option
	pSetupData->EmvLatchOption = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EMV_LATCH_ENABLE);	// [#605] NH KSK 2010.01.06

	// [R] 169 - Reason For Reversal Option (Standard1 Option)
	pSetupData->ReasonForReversal = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_REASONFORREVERSAL_ENABLE);	// [#606] NH KSK 2010.01.06
#endif

#if (RMS_VERSION >= 0x010314)	// [#613] AU_C KSK 2010.01.18
	// [R] 170 - Configuration Option	// [#615] AU_C KSK 2010.01.21
	pSetupData->ConfigurationOption = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CONFIGURATION_ENABLE);

	// [#2037] AU KSK 2011.03.31
	// [#2457] US Justin 2016.12.09 Support Moniview Journal Uploading	
	// [#2500] CA Justin 2017.08.22 Support Moniview Journal Uploading
	#if (AU_VERSION || US_VERSION  || CA_VERSION)		
	{
		// [R] 171 - Scheduled Journal Upload Enable
		pSetupData->ScheduledJournalEnable = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_ENABLE);
		
		// [R] 172 - Scheduled Journal Upload Option
		pSetupData->ScheduledJournalOption = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_OPTION);
		
		// [R] 173 - Scheduled Journal Count
		pSetupData->ScheduledJournalCount = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_COUNT);
		
		// [R] 174 - Scheduled Journal Day
		pSetupData->ScheduledJournalDay = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_DAY);
		
		// [R] 175 - Scheduled Journal Hour
		pSetupData->ScheduledJournalHour = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_HOUR);

		// [R] 176 - Scheduled Journal Available
		pSetupData->ScheduledJournalAvailable = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_AVAILABLE);	// KSK 2010.02.02 Journal Available Field 추가
	}
	#endif
	// End of [#2457]
	// End of [#2500]

	#if (AU_VERSION)	// [#2037] AU KSK 2011.03.31
	//[#616] AU_C SOOK 2010.02.05 AD BRAND NAME
	// [R] 177 - AD Brand Name
	strTemp = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_AD_BRAND_NAME );
	if (strTemp.GetLength() > 0)
		RMS_WideToMulti(strTemp, (char*)pSetupData->ADBrandName, sizeof(pSetupData->ADBrandName));
	// [R] 178 - Transaction AD Enable1
	pSetupData->TransactionAdver1Enable = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_TRANSACTION_ADVER1_ENABLE);
	// [R] 179 - Transaction AD Enable2
	pSetupData->TransactionAdver2Enable = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_TRANSACTION_ADVER2_ENABLE);
	//end of [#616]
	#endif
#endif							// end of [#613]

#if (RMS_VERSION >= 0x010315)	// [#620] US KSK 2010.02.22
	#if (US_VERSION || CA_VERSION || MX_VERSION)	// [#2115] MX KSK 2012.02.05
	// [R] 180 - Enhanced Coupon Available
	pSetupData->EnhancedCouponAvailable = MemGetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON_AVAILABLE);
	#endif
#endif

#if (RMS_VERSION >= 0x010316) // [#631] MX, 2010.03.29 JERRY
 	#if (MX_VERSION)
 	// [R] 181 - Bank Name Fee Print Enable
 	pSetupData->BankNameFeePrint = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_BANK_NAME_FEE_PRINT);
 	#endif
#endif

#if (RMS_VERSION >= 0x010317)
	// [R] 182 - SSL Version (V23 or V3)
	pSetupData->SSLVersion = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SSLVERSION);

	// [R] 183 - Transaction AD Display Time
	pSetupData->TransactionADDispTime = MemGetInt(_MEM_FLD_ADVINFO, _MEM_VAR_TRAN_ADV_DISP_TIME);

	// [R] 184 - Transaction AD Enable (1~6)
	for (i=0; i<6; i++)
		pSetupData->TransactionADEnable[i] = MemGetInt(_MEM_FLD_ADVINFO, _MEM_VAR_TRAN_ADV_ENDISFLAG1 + i);

	// [R] 185 - Change Background Enable
	pSetupData->ChangeBackgroundEnable = MemGetInt(_MEM_FLD_ADVINFO, _MEM_VAR_BACK_CHANGE_ENDISFLAG);

	// [R] 186 - Change Background Screen Enable (1~6)
	for (i=0; i<6; i++)
		pSetupData->ChangeBackgroundScreenEnable[i] = MemGetInt(_MEM_FLD_ADVINFO, _MEM_VAR_BACK_ENDISFLAG_TYPE1 + i);

	// [R] 187 - Default Background Screen
	pSetupData->DefaultBackgroundScreen = MemGetInt(_MEM_FLD_ADVINFO, _MEM_VAR_BACK_DEFAULT_TYPE);

	// [#11] NH KSK 2010.09.20
	NH_OS_VERSION	eOSVersion = m_pDevCmn->m_pConfig->GetOSVersion();
//	if (eOSVersion != NH_OS_NH1800SE_B)
	if (!(eOSVersion == NH_OS_NH1800SE_B || eOSVersion == NH_OS_NH1500SE))					// [#2267] US KSK 2014.05.08
	{
		// [R] 188 - Touch Vibration Enable
		pSetupData->TouchVibrationEnable = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_TOUCHVIBRATION_ENABLE);

		// [R] 189 - Notice Enable
		pSetupData->NoticeEnable = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_NOTICE_SERVICE_ENABLE);

		// [R] 190 - Notice Title
		strTemp = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_NOTICE_SERVICE_TITLE);
		if (strTemp.GetLength() > 0)
			RMS_WideToMulti(strTemp, (char*)pSetupData->NoticeTitle, sizeof(pSetupData->NoticeTitle));

		// [R] 191 - Notice Message
		for(i=0; i<3; i++)
		{
			strTemp = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_NOTICE_SERVICE_MSG1 + i);
			if (strTemp.GetLength() > 0)
				RMS_WideToMulti(strTemp, (char*)pSetupData->NoticeMsg[i], sizeof(pSetupData->NoticeMsg[i]));
		}
	}
	// end of [#11]
#endif

// [#2106] NH KSK 2011.12.19
#if (RMS_VERSION >= 0x010318)
	// [R] 192 - ESU Enable
	pSetupData->ESUEnable = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ANTISKIMMING_ENABLE);

	// [R] 193 - ESU Shutter Enable
	pSetupData->ESUShutterEnable = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ANTISKIMMING_SHUTTER_ENABLE);

	// [R] 194 - ESU Time Threshold
	pSetupData->ESUTimeThreshold = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ANTISKIMMING_TIMETHRESHOLD);

	// [R] 195 - ESU Recovery Time
#if (RMS_VERSION >= 0x010346)
	pSetupData->ESURecoveryTime = 0;
#else
	pSetupData->ESURecoveryTime = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ANTISKIMMING_AUTORECOVERYTIME);
#endif

	// [R] 196 - RKT Available
	pSetupData->RKTAvailable = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RKT_AVAILABLE);

	// [R] 197 - RKT Enable
	pSetupData->RKTEnable = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RKT_ENABLE);

	// [R] 198 - RKT Use Random Number
	pSetupData->RKTRandomNumberEnable = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RKT_RANDOMNUMBER_ENABLE);

	// [R] 199 - EJ Upload Enable
	pSetupData->EJUPLOADEnable = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EJUPLOAD_ENABLE);
#endif
// end of [#2106]


// [#2206] NH Justin 2013.06.27 Add additional RMS Information -- Dynamic Flow Setup, Dual Host DCC, Receipt Option
#if (RMS_VERSION >= 0x010319)

	// [R] 200 - Supervisor Language
	pSetupData->SupervisorLanguage = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SUPERVISOR_LANGUAGE);	// [#2213] US Justin 2013.07.30 Add Supervisore Language

	// [R] 201 - Dynamic Flow En/Disable
	pSetupData->DynamicFlowEnable = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_ENABLE);

	// [R] 202 - PIN Change 
	pSetupData->DynamicFlowPINChange = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_PINCHANGE);

	// [R] 203 - DCC
	pSetupData->DynamicFlowDCC = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_DCC);

	// [R] 204 - DCC Option (General, CIBC) 
	pSetupData->DynamicFlowDCCOption = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCC_CUSTOMOPTION);

	// [R] 205 - Dual Host Enable
	pSetupData->DualHostDCCEnable = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_ENABLE);

	// [R] 206 - Dual Host SSL Enable
	pSetupData->DualHostDCCSSLEnable = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_SSL_ENABLE);

	// [R] 207 - Dual Host SSL Version
	pSetupData->DualHostDCCSSLVersion = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_SSL_VERSION);

	// [R] 208 - Dual Host URL Enable
	pSetupData->DualHostDCCIsHostUseURL = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_ISHOSTUSEURL);

	// [R] 209 - Dual Host Host Name1
	strTemp = MemGetStr(_MEM_FLD_OPTIONALSETTING_2,_MEM_VAR_OPT2_DUALHOSTDCC_HOST1NAME);
	if (strTemp.GetLength() > 0)
		RMS_WideToMulti(strTemp, (char*)pSetupData->DualHostDCCHostName1, sizeof(pSetupData->DualHostDCCHostName1));

	// [R] 210 - Dual Host Port1
	nValue = MemGetInt(_MEM_FLD_OPTIONALSETTING_2,_MEM_VAR_OPT2_DUALHOSTDCC_HOST1PORTNO);
	pSetupData->DualHostDCCHostPort1 = (NUINT16)nValue;

	// [R] 211 - Dual Host Host Name2
	strTemp = MemGetStr(_MEM_FLD_OPTIONALSETTING_2,_MEM_VAR_OPT2_DUALHOSTDCC_HOST2NAME);
	if (strTemp.GetLength() > 0)
		RMS_WideToMulti(strTemp, (char*)pSetupData->DualHostDCCHostName2, sizeof(pSetupData->DualHostDCCHostName2));

	// [R] 212 - Dual Host Port2
	nValue = MemGetInt(_MEM_FLD_OPTIONALSETTING_2,_MEM_VAR_OPT2_DUALHOSTDCC_HOST2PORTNO);
	pSetupData->DualHostDCCHostPort2 = (NUINT16)nValue;

	// [R] 213 - Disclaimer 6 Lines
	// Line 1 ~ 3
	for (i = 0; i < 3; i++)
	{
		strTemp = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_DCC_DISCLAIMER1+i);
		if (strTemp.GetLength() > 0)
			RMS_WideToMulti(strTemp, (char*)pSetupData->DCCDIsclaimer[i], sizeof(pSetupData->DCCDIsclaimer[i]));
	}
	// Line 4 ~ 6
	for (i = 0; i < 3; i++)
	{
		strTemp = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_DCC_DISCLAIMER4+i);
		if (strTemp.GetLength() > 0)
			RMS_WideToMulti(strTemp, (char*)pSetupData->DCCDIsclaimer[i+3], sizeof(pSetupData->DCCDIsclaimer[i+3]));
	}

	// [R] 214 - Receipt On Screen
	pSetupData->ReceiptOnScreen = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RECEIPT_ON_SCREEN);

#endif
// end of [#2206]

// [#2242] NH Justin 2013.12.18 Dynamic Flow 1st Call Option
#if (RMS_VERSION >= 0x010320)
	pSetupData->DynamicFlow1stCallOption = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_1STCALL_OPTION);	// [#2251] US Justin 2014.01.29 
#endif
// End of [#2242]

// [#2255] NH KSK 2014.02.26 Added NOTE Counting Option
#if (RMS_VERSION >= 0x010321)
	pSetupData->NoteCountingOption = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE);
#endif
// end of [#2255]

// [#2309] US Justin 2014.11.17 US Balance No Fee Notice
#if (RMS_VERSION >= 0x010323)
	pSetupData->BalanceFeeNotice = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_BALANCE_NOFEE_NOTICE);
#endif
// end of [#2309]

// [#2316] US Justin 2014.12.17 Decimal Point percentage
#if (RMS_VERSION >= 0x010324)
	nValue = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DECIMAL_SURCHARGE);
	pSetupData->DecimalPointPercentage = (NUINT16)nValue;
#endif
// end of [#2316]

// [#2342] US Justin 2015.05.14 USE Common AID
#if (RMS_VERSION >= 0x010326)
	pSetupData->UseCommonAID = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_USE_USCOMMONAID);
	pSetupData->EnableDonation = 0;
	// End of [#2442]
#endif
// end of [#2342]

// [#2359] US Justin 2015.07.15 Add MV Setup
#if (RMS_VERSION >= 0x010327)
	#if (US_VERSION || CA_VERSION)
	pSetupData->PercentageSurchargeNotice	= MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PERCENT_SURCHARGE_NOTICE);
	pSetupData->PopmoneyEnable				= MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_POPMONEY_ENABLE);
	#endif
#endif
// end of [#2359]

// [#2405] US Justin 2016.03.21 Add MV Setup HalCash Online
#if (RMS_VERSION >= 0x010328)
	#if (US_VERSION || CA_VERSION)
	pSetupData->Pin4Enable					= MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PIN4_ENABLE);
	pSetupData->MoniMobileQR				= MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MOBILEAPP_QRCODE);	// [#2409] US Justin 2016.03.28 Add Moniview MoniMobile Setup Information
	#endif

	#if (AU_VERSION)	// [#2469] AU KSK 2017.01.25 
	pSetupData->EMVLanguageSelection		= MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EMV_LANGUAGESELECTION);
	#endif				// end of [#2469]
#endif
// end of [#2405]

// [#2435] US Justin Add Kernel Version
#if (RMS_VERSION >= 0x010329)
	pSetupData->RMSEMVKernelVersion				= m_pDevCmn->fnEMV_GetKernelVersion();
#endif
// End of [#2435]

// [#2442] US Justin Add Donation Type
#if (RMS_VERSION >= 0x010330)
	#if (US_VERSION || CA_VERSION)
	pSetupData->NHDonationType = 0;
	pSetupData->NHDualBalance = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_DUALBALANCE);		// [#2444] US Justin
	#endif
#endif
// End of [#2442]

// [#2449] NH Justin Add Paypal and Just.Cash Setup
#if (RMS_VERSION >= 0x010331)

	#if (US_VERSION)
		pSetupData->NHPaypalCCA = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PAYDIANT_CCA_ENABLE);
		pSetupData->NHJustCash  = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_JUSTCASH_ENABLE);
	#endif

	pSetupData->HostSSLTLSCertificate  = MemGetInt(_MEM_FLD_OPTIONALSETTING_2,_MEM_VAR_OPT2_SSL_CERTIFY);		// [#2475] US Justin 2017.02.22 	
#endif
// End of [#2449]

// [#2508] NH Justin 2017.09.27 Upload Master Key checksums to Moniview
#if (RMS_VERSION >= 0x010332)

	if (MemGetVersion(_MEMKEY_EPVERSION, L"PIN").CompareNoCase(PIN_512K_EP_VERSION) >= 0)
	{
		// Master Key Checksum
		strTemp = MemGetStr(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYCHECKSUM0);
		if (strTemp.GetLength() >= 4)
			RMS_WideToMulti(strTemp, (char*)pSetupData->MasterKeyCSum[0], sizeof(pSetupData->MasterKeyCSum[0]));

		// Dual Master Key Checksum
		strTemp = MemGetStr(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYCHECKSUM1);
		if(strTemp.GetLength() >= 4)
			RMS_WideToMulti(strTemp, (char*)pSetupData->MasterKeyCSum[1], sizeof(pSetupData->MasterKeyCSum[1]));
	}
	else
	{
		strTemp = MemGetStr(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYCHECKSUM0 + MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYINDEX));
		if (strTemp.GetLength() >= 4)
			RMS_WideToMulti(strTemp, (char*)pSetupData->MasterKeyCSum[0], sizeof(pSetupData->MasterKeyCSum[0]));
	}
		
	// MAC Key
	int nEPPKeyMode = MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE);
	if ( nEPPKeyMode == KEYMODE_NON_UNIQ_SDES_MACING || nEPPKeyMode == KEYMODE_UNIQ_SDES_MACING ||
		 nEPPKeyMode == KEYMODE_TDES_MACING			 || nEPPKeyMode == KEYMODE_TDES_TMACING )
	{
		strTemp = MemGetStr(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYCHECKSUM15);
		if(strTemp.GetLength() >= 4)
			RMS_WideToMulti(strTemp, (char*)pSetupData->MACKeyCSum, sizeof(pSetupData->MACKeyCSum));
	}	
#endif
// End of [#2508]

// [#2497] AU Kook 2017.10.24 ATS ALPHI, support MoniView
#if (RMS_VERSION >= 0x010333)
	#if (AU_VERSION)

		pSetupData->AlphiEnDisable = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ALPHI_ENDISABLE);

		strTemp = MemGetStr(_MEM_FLD_OPTIONALSETTING_2,_MEM_VAR_OPT2_ALPHI_SERVER_ADDRESS);
		if (strTemp.GetLength() > 0)
			RMS_WideToMulti(strTemp, (char*)pSetupData->AlphiAddress, sizeof(pSetupData->AlphiAddress));

		nValue = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ALPHI_SERVER_PORT);
		pSetupData->AlphiPort =	(NUINT16) nValue;

		strTemp = MemGetStr(_MEM_FLD_OPTIONALSETTING_2,_MEM_VAR_OPT2_ALPHI_ID);
		if (strTemp.GetLength() > 0)
			RMS_WideToMulti(strTemp, (char*)pSetupData->AlphiID, sizeof(pSetupData->AlphiID));

		strTemp = MemGetStr(_MEM_FLD_OPTIONALSETTING_2,_MEM_VAR_OPT2_ALPHI_TERMINAL_ID);
		if (strTemp.GetLength() > 0)
			RMS_WideToMulti(strTemp, (char*)pSetupData->AlphiTermID, sizeof(pSetupData->AlphiTermID));

		strTemp = MemGetStr(_MEM_FLD_OPTIONALSETTING_2,_MEM_VAR_OPT2_ALPHI_OPERATOR_ID);
		if (strTemp.GetLength() > 0)
			RMS_WideToMulti(strTemp, (char*)pSetupData->AlphiOprID, sizeof(pSetupData->AlphiOprID));

		strTemp = MemGetStr(_MEM_FLD_OPTIONALSETTING_2,_MEM_VAR_OPT2_ALPHI_TDES_KEY);
		if (strTemp.GetLength() > 0)
			RMS_WideToMulti(strTemp, (char*)pSetupData->AlphiTDESKey, sizeof(pSetupData->AlphiTDESKey));

		strTemp = MemGetStr(_MEM_FLD_OPTIONALSETTING_2,_MEM_VAR_OPT2_ALPHI_EXTRA_INFORMATION);
		if (strTemp.GetLength() > 0)
			RMS_WideToMulti(strTemp, (char*)pSetupData->AlphiExtraInformation, sizeof(pSetupData->AlphiExtraInformation));

		//nValue = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ALPHI_MSG_AUDITNUMBER);
		//pSetupData->AlphiMsgAuditNum =		(NUINT16) nValue;

		strTemp = MemGetStr(_MEM_FLD_OPTIONALSETTING_2,_MEM_VAR_OPT2_ALPHI_CURRENCY_CODE);  // [#2497] NH woooZ 2017.10.25 ALPHI currency code
		if (strTemp.GetLength() > 0)
			RMS_WideToMulti(strTemp, (char*)pSetupData->AlphiCurrencyCode, sizeof(pSetupData->AlphiCurrencyCode));

		pSetupData->AlphiReversalMaxRetry =	MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ALPHI_REVERSAL_MAX_RETRY);
		pSetupData->AlphiCardDataToBeUsed =	MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ALPHI_CARDDATA_TO_BE_USED);

		// [#2579] AU Kook 2018.10.12 Support ALPHI SSL Configuration via MoniView
		pSetupData->AlphiSSLEnDisable = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ALPHI_SSL_ENDISABLE);			// 2017.12.18
 		pSetupData->AlphiSSLCertEnDisable = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ALPHI_SSL_CERT_ENDISABLE);	// [#2497] NH woooZ 2018.01.26 MoniView개발 후 주석 풀것
		// end of [#2579]
	#endif
#endif
// end of [#2497]

// [#2517] US Justin 2017.11.29 EMV Fallback Enable/Diable Option
#if (RMS_VERSION >= 0x010334)
	#if (US_VERSION)
		pSetupData->EMV_Fallback4UnknownICCard	= MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_FALLBACK_4_UNKNOWNAID);
		pSetupData->EMV_Fallback_EnDisable		= MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_FALLBACK_ENABLE_DISABLE);
	#endif
#endif
// End of [#2517]

// [#2538] NH Justin 2018.03.09 MoniMobile Cash Balance Option
#if (RMS_VERSION >= 0x010335)
	// MoniMObile Cash Balance Option ---  MV : 0(Disable) or 1(Enable) <= ATM 1(Enable), 0/2(Disable)
	if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MONIMOBILE_CASHBALANCE) == 1 )
		pSetupData->MoniMobileCashBalanceOption	= 1;
	else
		pSetupData->MoniMobileCashBalanceOption	= 0;

	// [#2548] NH Justin 2018.04.27 Change Camera Option Name : MoniVision, Live-Feed, Journal Pic	// [#GLDV-2505] Support MX-2800T
	if (GetConfigFuncPointer()->SupportsCamera())
	{
		pSetupData->MoniVisionEnable			= MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CAM_ENABLE);
		pSetupData->MoniVisionLiveFeedOption	= MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CAM_ANTI_SHLDR_SURF_ENABLE);
		pSetupData->MoniVisionJournalPicOption	= MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CAM_FACE_MEMORIZE_ENABLE);
	}
	else
	{
		pSetupData->MoniVisionEnable = pSetupData->MoniVisionLiveFeedOption = pSetupData->MoniVisionJournalPicOption = 0;
	}
	// End of [#2548]

	// [#2549] NH Justin 2018.05.08 Add US Territory Option
	#if (US_VERSION)
		pSetupData->USTerritoryOption			= MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_US_TERRITORY_OPTION);
	#endif
	// End of [#2549]
#endif
// End of [#2538]

// [#2558] NH Justin 2018.06.13 Add Scheduled Reboot Optioin
#if (RMS_VERSION >= 0x010336)
	pSetupData->RebootOption	= MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_REBOOT_OPTION);
	pSetupData->RebootTime		= MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_REBOOT_TIME);
	pSetupData->RebootInterval	= MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_REBOOT_INTERVAL);
	pSetupData->GivePayEnable	= MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_GIVEPAY_ENABLE_DISABLE);	// [#2557] NH Justin 2018.06.12 GivePay Enhancement 
	pSetupData->RMSSendRetry	= MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RMSSEND_RETRY);			// [#2559] NH Justin 2018.06.18 Retry Status Send if "send" failed
#endif
// End of [#2558]

// [#2574] US Justin GivePay Enhancement3, Add Download Config Time
#if (RMS_VERSION >= 0x010338)
	pSetupData->GivePayConfigTime	= MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_GIVEPAY_CONFIG_TIME);
	
	//int nGivePayRMSOption = GIVEPAY_APPEAR_BOTH;
	int nGivePayRMSOption = RMS_GIVEPAY_BOTH;
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_GIVEPAY_UPSELL) == GIVEPAY_APPEAR_BOTH)
		nGivePayRMSOption = RMS_GIVEPAY_BOTH;
	else if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_GIVEPAY_UPSELL) == GIVEPAY_APPEAR_INDIVIDUAL)
		nGivePayRMSOption = RMS_GIVEPAY_INDIVIDUAL;
	else if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_GIVEPAY_UPSELL) == GIVEPAY_APPEAR_PIGGYBACK)
		nGivePayRMSOption = RMS_GIVEPAY_PIGGYBACK;	
	pSetupData->GivePayUpSell		= nGivePayRMSOption;
	//pSetupData->GivePayUpSell		= MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_GIVEPAY_UPSELL);	// [#J006] Upsell Option
	//pSetupData->GivePayEnrolled		= MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_GIVEPAY_ENROLLED);	// [#RWC6-1, #2583] US Brandon 2018.12.17 GivePay DG911 fix, WinATM error fix, Enhancement 6 // TODO add this after RMS update

	pSetupData->DynamicFlowDCCType	= MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DYNAMICFLOW_DCC_TYPE);	// [#RWC6-1, #2583] US Brandon 2018.12.17 GivePay DG911 fix, WinATM error fix, Enhancement 6 // TODO add this after RMS update
	pSetupData->DualHostDCCType		= MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOST_DCC_TYPE);		// [#RWC6-1, #2583] US Brandon 2018.12.17 GivePay DG911 fix, WinATM error fix, Enhancement 6 // TODO add this after RMS update	
#endif
// End of [#2574]

// [#RWC6-10] Adding secondary DNS with default value of 8.8.8.8. or 8.8.4.4, for PAI
#if (RMS_VERSION >= 0x010339)
	strTemp = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ATM_DNS_2ND);
	if (strTemp.GetLength() > 0)
		RMS_WideToMulti(strTemp, (char*)pSetupData->DNSServer2nd, sizeof(pSetupData->DNSServer2nd));
#endif
// End of [#RWC6-10]

	// [#RWC6-29] PAI Add ALLPOIN
#if (RMS_VERSION >= 0x010340)
	#if (APP_CUSTOM_PAI)
		pSetupData->AllPointSurcharge = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_SURCHARGE);
	#endif
#endif
	// end of [#RWC6-29]

// [#RWC6-67] US William 2019.10.25 MoniView TLS
#if (RMS_VERSION >= 0x010341)
	pSetupData->RMSTLSEnable = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RMSTLSENABLE);
	pSetupData->RMSTLSListeningPortNumber = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RMSTLSPORT);
#endif
// End of [#RWC6-67] US William 2019.10.25 MoniView TLS

// [#RWC6-119] US William 2020.01.16 Add JustCash RMS changes
#if (RMS_VERSION >= 0x010343)
#if (US_VERSION)
#if (APP_JUST_CASH)
	pSetupData->JustCashMode = (NUINT8) MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_JUSTCASH_ENABLE);
	pSetupData->JustCashRegistrationState = (NUINT8) m_pTranCmn->m_JustCashData.m_bJustCashRegistered;
#endif
#endif
#endif
// end of [#RWC6-119]


// [#RWC6-140] US William 2020.02.21 B4U RMS
#if (RMS_VERSION >= 0x010343)	
	// B4U Enabled
	pSetupData->B4UEnabled = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_B4U_ENABLED);
#endif
// End of [#RWC6-140] US William 2020.02.21 B4U RMS

	// [#RWC6-169] US William 2020.04.28 Add PAI Credit Segmentation RMS
#if (RMS_VERSION >= 0x010345)
	{
	// To add credit card segmentation for US general version as well.
	#if (US_VERSION && !APP_TDL_OPTION)
			int creditSurchargeAmount = Asc2Int(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_CREDITSURCHARGE));
			pSetupData->CreditAdditionalSurchargeAmount = creditSurchargeAmount;
	#endif

		// [#RWC6-170] US William 2020.04.30 Add Remote Update Config to RMS
		CUpdateRepoConfigurationManager configManager;
		UpdateSourceInfo config;

		pSetupData->UpdateRepoConfigValid = 0;
		pSetupData->ScheduledUpdatesEnabled = 0;
		pSetupData->ScheduledUpdateDayOfWeek = 0;
		memset(pSetupData->UpdateRepoHostname, 0, sizeof(pSetupData->UpdateRepoHostname));
		memset(pSetupData->UpdateRepoHives, 0, sizeof(pSetupData->UpdateRepoHives));

		if (configManager.GetConfiguration(&config) == REPOERR_OK)
		{
			pSetupData->UpdateRepoConfigValid = 1;
			pSetupData->ScheduledUpdatesEnabled = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULE_U2D_ENABLE) == ENABLE;
			pSetupData->ScheduledUpdateDayOfWeek = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULE_U2D_DOW);
			
			// Repo hostname
			RMS_WideToMulti(config.RepositoryBaseURL, (char *)pSetupData->UpdateRepoHostname, sizeof(pSetupData->UpdateRepoHostname)); 
			
			// Repo Hives
			RMS_WideToMulti(config.GetHivesRMSString(), (char *)pSetupData->UpdateRepoHives, sizeof(pSetupData->UpdateRepoHives));
		}


		// [#RWC6-127] Add Disk Management to RMS Status
		DiskUsageInformation atmUsage, atm2Usage, totalUsage;
		CDiskManager* manager = new CDiskManager();

		manager->GetDiskUtilization(AP_DISK_ATM, atmUsage);
		manager->GetDiskUtilization(AP_DISK_ATM2, atm2Usage);

		pSetupData->DiskMgmtAtmUsedCapKiBytes = (unsigned long) (atmUsage.UsedCapacityBytes / 1024);
		pSetupData->DiskMgmtAtmFreeCapKiBytes = (unsigned long) (atmUsage.FreeCapacityBytes / 1024);
		pSetupData->DiskMgmtAtm2UsedCapKiBytes = (unsigned long) (atm2Usage.UsedCapacityBytes / 1024);
		pSetupData->DiskMgmtAtm2FreeCapKiBytes = (unsigned long) (atm2Usage.FreeCapacityBytes / 1024);
		pSetupData->DiskMgmtTotalUsedCapKiBytes = (unsigned long) (atmUsage.TotalCapacityBytes / 1024);
		pSetupData->DiskMgmtTotalFreeCapKiBytes = (unsigned long) (atmUsage.TotalCapacityBytes / 1024);

		delete manager;
		manager = NULL;
	}
#endif

	// [#RWC6-178] US William 2020.50.22 Add std3 comm header enablement to RMS setup data
#if (RMS_VERSION >= 0x010346)
	pSetupData->Standard3CommHeaderEnabled = MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_COMMUNICATIONHEADERENABLE);

	// [#RWC6-168] US William 2020.06.09 Anti-skimming 2B RMS setup field
	pSetupData->ESURecoveryTime2Byte = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ANTISKIMMING_AUTORECOVERYTIME);
#endif

	// [#RWC6-207] US William 2020.08.05 Add LibertyX enablement to setup data
#if (RMS_VERSION >= 0x010347)
	LXConfiguration lxConfig;
	CLibertyXConfigurationManager ltxConfigManager;
	if (ltxConfigManager.GetConfiguration(lxConfig))
	{
		pSetupData->LibertyXBuyEnabled = lxConfig.BuyBitcoinEnabled;
	}
	else 
	{
		pSetupData->LibertyXBuyEnabled = 0;
	}
#endif

	// [#RWC6-214] US William 2020.09.01 Add CDU dispense style
#if (RMS_VERSION >= 0x010348)
	pSetupData->CDUDispenseStyle = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DISPENSE_STYLE);
#endif

	// [#RWC6-224] US William 2020.10.21 LibertyX Dispense
#if (RMS_VERSION >= 0x010349)
	if (ltxConfigManager.GetConfiguration(lxConfig))
	{
		pSetupData->LibertyXSellEnabled = lxConfig.SellBitcoinEnabled;
	}
	else 
	{
		pSetupData->LibertyXSellEnabled = 0;
	}
#endif

#if (RMS_VERSION >= 0x010350)
	memset(pSetupData->LibertyXUsername, 0, sizeof(pSetupData->LibertyXUsername));
	if (ltxConfigManager.GetConfiguration(lxConfig))
	{
		RMS_WideToMulti(lxConfig.Username, (char *)pSetupData->LibertyXUsername, min(sizeof(pSetupData->LibertyXUsername), lxConfig.Username.GetLength()));
	}
#endif

#if (RMS_VERSION >= 0x010351)
	strTemp = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND);
	if (strTemp.GetLength() > 0)
		RMS_WideToMulti(strTemp, (char*)pSetupData->ModelName, sizeof(pSetupData->ModelName));

	RETAILMSG(1, (L"pSetupData->ModelName: '%S'\n", (char*)pSetupData->ModelName));

	pSetupData->LocalDCCSurchargeEnabled = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCCLOOKUP_LOCALSURCHARGE);
	pSetupData->CDUBindingEnabled = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDU_BINDING);
	pSetupData->CDUBindingActive = MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_CDU_AUTHFLAG);
#endif

#if (RMS_VERSION >= 0x010353)
	pSetupData->SidecarEnabled = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_SUPPORT);
	pSetupData->AcceptorType = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_BNATYPE);
	pSetupData->RFIDEnabled = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE);
#endif

	NHDEBUG(1, (_T("-------------------------------------------------------------\n")));
	return TRUE;
}

// [#424] [NH] KSK 2008.9.18
int CRmsCtrl::RMS_MakeErrTotalMsg(BYTE *pMsg)
{
	CString		strTemp, strTemp2;
	int			nDataLen = 0;	// (comm header1 30 - length(2)) + comm header3 27 = 55
	PRMS_CMD_ERROR_TOTAL	pErrTotal;

	pErrTotal = (PRMS_CMD_ERROR_TOTAL) pMsg;

	memset(pErrTotal, 0, sizeof(RMS_CMD_ERROR_TOTAL));

	// 6. Year-Month-Day, Hour-Miniute-Second
	SYSTEMTIME localTime;
	::GetLocalTime(&localTime);
	pErrTotal->dwYear = localTime.wYear;
	pErrTotal->dwMonth = localTime.wMonth;
	pErrTotal->dwDay = localTime.wDay;
	pErrTotal->dwHour = localTime.wHour;
	pErrTotal->dwMinute = localTime.wMinute;
	pErrTotal->dwSecond = localTime.wSecond;

	// 7. Machine Type
	pErrTotal->MachineType = RMS_GetMachineType();

	// 8. Serial Number
	strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ATMSERIALNO);
	if(strTemp == "0000000000") //default setting value = "0000000000"
		strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID);
	strTemp.TrimLeft();
	strTemp.TrimRight();
	RMS_WideToMulti(strTemp, (char*)pErrTotal->SerialNumber, sizeof(pErrTotal->SerialNumber));

	CST_STATUS	CSTStatus;
	GetCSTStatus(&CSTStatus);

	// 8. CST1 Denomination
	pErrTotal->CST1Denomination = CSTStatus.CST1Denomination;

	// 9. CST1 Bill Count
	pErrTotal->CST1BillCount = CSTStatus.CST1BillCount;

	// 10. CST2 Denomination
	pErrTotal->CST2Denomination = CSTStatus.CST2Denomination;

	// 11. CST2 Bill Count
	pErrTotal->CST2BillCount = CSTStatus.CST2BillCount;

	// 12. CST3 Denomination
	pErrTotal->CST3Denomination = CSTStatus.CST3Denomination;

	// 13. CST3 Bill Count
	pErrTotal->CST3BillCount = CSTStatus.CST3BillCount;

	// 14. CST4 Denomination
	pErrTotal->CST4Denomination = CSTStatus.CST4Denomination;

	// 15. CST4 Bill Count
	pErrTotal->CST4BillCount = CSTStatus.CST4BillCount;

	// 16. Journal Count
	if (m_pDevCmn->m_JNLMgr.GetUploadRemainCount(UP_RMS) > 0)
		pErrTotal->JournalCount = (NUINT16)m_pDevCmn->m_JNLMgr.GetUploadRemainCount(UP_RMS);
	else
		pErrTotal->JournalCount = 0;

	// 17. ATM Status.
	BYTE bATMStatus = (BYTE)MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ATMSTATUS);
	
	if (bATMStatus > ATM_RMSACTIVE) 
		bATMStatus = 0xff;
	else
		bATMStatus +=0x31;

	pErrTotal->ATMStatus = bATMStatus;

	// 18. Error Code.
	if (!m_pDevCmn->fnAPL_CheckError())
		sprintf((char*)pErrTotal->ErrorCode, "%7.7S", m_pDevCmn->fstrAPL_GetErrorCode());
	else
		memcpy(pErrTotal->ErrorCode, "0000000", 7);

	// 19. Start Date Time
	strTemp = GetErrSumDate();		// format MMDDYYYY
	strTemp2 = GetErrSumTime();		// format HHMMSS
	strTemp.TrimRight();
	strTemp2.TrimRight();
	pErrTotal->StartDateTime[0] = Asc2Int(strTemp.Right(4));		// YYYY
	pErrTotal->StartDateTime[1] = Asc2Int(strTemp.Left(2));			// MM
	pErrTotal->StartDateTime[2] = Asc2Int(strTemp.Mid(2,2));		// DD
	pErrTotal->StartDateTime[3] = Asc2Int(strTemp2.Left(2));		// HH
	pErrTotal->StartDateTime[4] = Asc2Int(strTemp2.Mid(2,2));		// MM
	pErrTotal->StartDateTime[5] = Asc2Int(strTemp2.Right(2));		// SS

	// 20. End Date Time
	pErrTotal->EndDateTime[0] = localTime.wYear;
	pErrTotal->EndDateTime[1] = localTime.wMonth;
	pErrTotal->EndDateTime[2] = localTime.wDay;
	pErrTotal->EndDateTime[3] = localTime.wHour;
	pErrTotal->EndDateTime[4] = localTime.wMinute;
	pErrTotal->EndDateTime[5] = localTime.wSecond;

	// 21. Error Code Count
	pErrTotal->ErrorCodeCnt = GetErrSumTotalCount();

	nDataLen = 80;
	// 22. Error Code & Error Count
	for (int i=0; i<GetErrSumTotalCount(); i++)
	{
		sprintf((char*)pErrTotal->ErrSum[i].ErrCode, "%7.7S", GetErrSumErrCode(i));
		pErrTotal->ErrSum[i].ErrCount = (NUINT16)GetErrSumStackCount(i);
		nDataLen += sizeof(RMS_ERR_SUM);	// 가변부 (ERRORCODE(8)+ERRORCOUNT(2)) = 10
	}

	return nDataLen;
}

// [#523] US KSK 2009.05.19
BOOL CRmsCtrl::RMS_MakeUploadFileDataMsg(BYTE *pMsg, int Len)
{
	CString		strTemp, strTemp2;
	int			nDataLen = 0;	// (comm header1 30 - length(2)) + comm header3 27 = 55
	PRMS_FILE_UPLOAD_DATA	pUploadData;
	
	pUploadData = (PRMS_FILE_UPLOAD_DATA) pMsg;
	
	// 6. Year-Month-Day, Hour-Minute-Second
	SYSTEMTIME localTime;
	::GetLocalTime(&localTime);

	pUploadData->dwYear = localTime.wYear;
	pUploadData->dwMonth = localTime.wMonth;
	pUploadData->dwDay = localTime.wDay;
	pUploadData->dwHour = localTime.wHour;
	pUploadData->dwMinute = localTime.wMinute;
	pUploadData->dwSecond = localTime.wSecond;
	
	// 7. Machine Type
	pUploadData->MachineType = RMS_GetMachineType();

	// 8. Serial Number
	strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ATMSERIALNO);
	if(strTemp == "0000000000") //default setting value = "0000000000"
		strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID);
	strTemp.TrimLeft();
	strTemp.TrimRight();
	RMS_WideToMulti(strTemp, (char*)pUploadData->SerialNumber, sizeof(pUploadData->SerialNumber));
	
	CST_STATUS	CSTStatus;
	GetCSTStatus(&CSTStatus);
	
	// 8. CST1 Denomination
	pUploadData->CST1Denomination = CSTStatus.CST1Denomination;
	
	// 9. CST1 Bill Count
	pUploadData->CST1BillCount = CSTStatus.CST1BillCount;
	
	// 10. CST2 Denomination
	pUploadData->CST2Denomination = CSTStatus.CST2Denomination;
	
	// 11. CST2 Bill Count
	pUploadData->CST2BillCount = CSTStatus.CST2BillCount;
	
	// 12. CST3 Denomination
	pUploadData->CST3Denomination = CSTStatus.CST3Denomination;
	
	// 13. CST3 Bill Count
	pUploadData->CST3BillCount = CSTStatus.CST3BillCount;
	
	// 14. CST4 Denomination
	pUploadData->CST4Denomination = CSTStatus.CST4Denomination;
	
	// 15. CST4 Bill Count
	pUploadData->CST4BillCount = CSTStatus.CST4BillCount;
	
	// 16. Journal Count
	if (m_pDevCmn->m_JNLMgr.GetUploadRemainCount(UP_RMS) > 0)
		pUploadData->JournalCount = (NUINT16)m_pDevCmn->m_JNLMgr.GetUploadRemainCount(UP_RMS);
	else
		pUploadData->JournalCount = 0;
	
	// 17. ATM Status.
	BYTE bATMStatus = (BYTE)MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ATMSTATUS);
	
	if (bATMStatus > ATM_RMSACTIVE) 
		bATMStatus = 0xff;
	else
		bATMStatus +=0x31;
	
	pUploadData->ATMStatus = bATMStatus;
	
	// 18. Error Code.
	if (!m_pDevCmn->fnAPL_CheckError())
		sprintf((char*)pUploadData->ErrorCode, "%7.7S", m_pDevCmn->fstrAPL_GetErrorCode());
	else
		memcpy(pUploadData->ErrorCode, "0000000", 7);

	return TRUE;
}
// end of [#523]

// [#533] NH KSK 2009.06.08
int	CRmsCtrl::RMS_MakeRejectTotalMsg(BYTE *pMsg)
{
	CString		strTemp, strTemp2;
	PRMS_CMD_REJECT_TOTAL	pRejectTotal;

	pRejectTotal = (PRMS_CMD_REJECT_TOTAL) pMsg;

	memset(pRejectTotal, 0, sizeof(RMS_CMD_REJECT_TOTAL));

	// 6. Year-Month-Day, Hour-Miniute-Second
	SYSTEMTIME localTime;
	::GetLocalTime(&localTime);
	pRejectTotal->dwYear = localTime.wYear;
	pRejectTotal->dwMonth = localTime.wMonth;
	pRejectTotal->dwDay = localTime.wDay;
	pRejectTotal->dwHour = localTime.wHour;
	pRejectTotal->dwMinute = localTime.wMinute;
	pRejectTotal->dwSecond = localTime.wSecond;

	// 7. Machine Type
	pRejectTotal->MachineType = RMS_GetMachineType();

	// 8. Serial Number
	strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ATMSERIALNO);
	if(strTemp == "0000000000") //default setting value = "0000000000"
		strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID);
	strTemp.TrimLeft();
	strTemp.TrimRight();
	RMS_WideToMulti(strTemp, (char*)pRejectTotal->SerialNumber, sizeof(pRejectTotal->SerialNumber));

	CST_STATUS	CSTStatus;
	GetCSTStatus(&CSTStatus);

	// 8. CST1 Denomination
	pRejectTotal->CST1Denomination = CSTStatus.CST1Denomination;

	// 9. CST1 Bill Count
	pRejectTotal->CST1BillCount = CSTStatus.CST1BillCount;

	// 10. CST1 Status
//	pRejectTotal->CST1Status = CSTStatus.CST1Status;

	// 11. CST2 Denomination
	pRejectTotal->CST2Denomination = CSTStatus.CST2Denomination;

	// 12. CST2 Bill Count
	pRejectTotal->CST2BillCount = CSTStatus.CST2BillCount;

	// 13. CST2 Status
//	pRejectTotal->CST2Status = CSTStatus.CST2Status;

	// 14. CST3 Denomination
	pRejectTotal->CST3Denomination = CSTStatus.CST3Denomination;

	// 15. CST3 Bill Count
	pRejectTotal->CST3BillCount = CSTStatus.CST3BillCount;

	// 16. CST3 Status
//	pRejectTotal->CST3Status = CSTStatus.CST3Status;

	// 17. CST4 Denomination
	pRejectTotal->CST4Denomination = CSTStatus.CST4Denomination;

	// 18. CST4 Bill Count
	pRejectTotal->CST4BillCount = CSTStatus.CST4BillCount;

	// 19. CST4 Status
//	pRejectTotal->CST4Status = CSTStatus.CST4Status;

	// 20. Journal Count
	if (m_pDevCmn->m_JNLMgr.GetUploadRemainCount(UP_RMS) > 0)
		pRejectTotal->JournalCount = (NUINT16)m_pDevCmn->m_JNLMgr.GetUploadRemainCount(UP_RMS);
	else
		pRejectTotal->JournalCount = 0;

	// 21. ATM Status.
	BYTE bATMStatus = (BYTE)MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ATMSTATUS);
	
	if (bATMStatus > ATM_RMSACTIVE) 
		bATMStatus = 0xff;
	else
		bATMStatus +=0x31;

	pRejectTotal->ATMStatus = bATMStatus;

	// 22. Error Code.
	if (!m_pDevCmn->fnAPL_CheckError())
		sprintf((char*)pRejectTotal->ErrorCode, "%7.7S", m_pDevCmn->fstrAPL_GetErrorCode());
	else
		memcpy(pRejectTotal->ErrorCode, "0000000", 7);

	// 23. Start Date Time
	int	nYear, nMonth, nDay, nHour, nMinute, nSecond;
	GetRejTotalStartDate(&nYear, &nMonth, &nDay);
	GetRejTotalStartTime(&nHour, &nMinute, &nSecond);

	pRejectTotal->StartDateTime[0] = (NUINT16) nYear;
	pRejectTotal->StartDateTime[1] = (NUINT16) nMonth;
	pRejectTotal->StartDateTime[2] = (NUINT16) nDay;
	pRejectTotal->StartDateTime[3] = (NUINT16) nHour;
	pRejectTotal->StartDateTime[4] = (NUINT16) nMinute;
	pRejectTotal->StartDateTime[5] = (NUINT16) nSecond;

	// 24. End Date Time
	pRejectTotal->EndDateTime[0] = localTime.wYear;
	pRejectTotal->EndDateTime[1] = localTime.wMonth;
	pRejectTotal->EndDateTime[2] = localTime.wDay;
	pRejectTotal->EndDateTime[3] = localTime.wHour;
	pRejectTotal->EndDateTime[4] = localTime.wMinute;
	pRejectTotal->EndDateTime[5] = localTime.wSecond;

	// 25. Total Dispensed Count
	// 26. CST1 Dispensed Count
	// 27. CST2 Dispensed Count
	// 28. CST3 Dispensed Count
	// 29. CST4 Dispensed Count
	// 30. Total Reject Count (AP와 마찬가지로 보정장치 필요여부 사양협의 필요)
	for(int i=0; i<CDU_MAX_CST_COUNT; i++)
	{
		// Total Dispensed Count
		pRejectTotal->CSTDispensedCount[i] = (NUINT32)GetSumOfDispensed(CDU_CST_1 + i);
		pRejectTotal->TotalDispensedCount += pRejectTotal->CSTDispensedCount[i];

		// Total Reject Count
		pRejectTotal->TotalRejectCount += (NUINT32)GetSumOfRejected(CDU_CST_1 + i);
	}

	// 31. CST1 Skew Count
	pRejectTotal->CST1SkewCount = GetSumOfDetectedSkew(CDU_CST_1);

	// 32. CST1 Gap Count
	pRejectTotal->CST1SkewCount = GetSumOfDetectedGap(CDU_CST_1);

	// 33. CST1 Long Count
	pRejectTotal->CST1LongCount = GetSumOfDetectedLong(CDU_CST_1);

	// 34. CST1 Short Count
	pRejectTotal->CST1ShortCount = GetSumOfDetectedShort(CDU_CST_1);

	// 35. CST1 Double Count
	pRejectTotal->CST1DoubleCount = GetSumOfDetectedDouble(CDU_CST_1);

	// 36. CST2 Skew Count
	pRejectTotal->CST2SkewCount = GetSumOfDetectedSkew(CDU_CST_2);
	
	// 37. CST2 Gap Count
	pRejectTotal->CST2SkewCount = GetSumOfDetectedGap(CDU_CST_2);
	
	// 38. CST2 Long Count
	pRejectTotal->CST2LongCount = GetSumOfDetectedLong(CDU_CST_2);
	
	// 40. CST2 Short Count
	pRejectTotal->CST2ShortCount = GetSumOfDetectedShort(CDU_CST_2);
	
	// 41. CST2 Double Count
	pRejectTotal->CST2DoubleCount = GetSumOfDetectedDouble(CDU_CST_2);

	// 42. CST3 Skew Count
	pRejectTotal->CST3SkewCount = GetSumOfDetectedSkew(CDU_CST_3);
	
	// 43. CST3 Gap Count
	pRejectTotal->CST3SkewCount = GetSumOfDetectedGap(CDU_CST_3);
	
	// 44. CST3 Long Count
	pRejectTotal->CST3LongCount = GetSumOfDetectedLong(CDU_CST_3);
	
	// 45. CST3 Short Count
	pRejectTotal->CST3ShortCount = GetSumOfDetectedShort(CDU_CST_3);
	
	// 46. CST3 Double Count
	pRejectTotal->CST3DoubleCount = GetSumOfDetectedDouble(CDU_CST_3);

	// 47. CST4 Skew Count
	pRejectTotal->CST4SkewCount = GetSumOfDetectedSkew(CDU_CST_4);
	
	// 48. CST4 Gap Count
	pRejectTotal->CST4SkewCount = GetSumOfDetectedGap(CDU_CST_4);
	
	// 49. CST4 Long Count
	pRejectTotal->CST4LongCount = GetSumOfDetectedLong(CDU_CST_4);
	
	// 50. CST4 Short Count
	pRejectTotal->CST4ShortCount = GetSumOfDetectedShort(CDU_CST_4);
	
	// 51. CST4 Double Count
	pRejectTotal->CST4DoubleCount = GetSumOfDetectedDouble(CDU_CST_4);
	
	return TRUE;
}
// end of [#533]

//[#586] SOOK 2009.12.04 iTM MoniView 연동 부분 추가 
BOOL CRmsCtrl::RMS_MakeJNLImageDataMsg(BYTE *pMsg, int Len)
{
	CString		strTemp, strTemp2;
	int			nDataLen = 0;	// (comm header1 30 - length(2)) + comm header3 27 = 55
	PRMS_IMAGE_UPLOAD_DATA 	pUploadData;
	
	pUploadData = (PRMS_IMAGE_UPLOAD_DATA) pMsg;
	
	// 6. Year-Month-Day, Hour-Miniute-Second
	SYSTEMTIME localTime;
	::GetLocalTime(&localTime);

	pUploadData->dwYear = localTime.wYear;
	pUploadData->dwMonth = localTime.wMonth;
	pUploadData->dwDay = localTime.wDay;
	pUploadData->dwHour = localTime.wHour;
	pUploadData->dwMinute = localTime.wMinute;
	pUploadData->dwSecond = localTime.wSecond;
	
	// 7. Machine Type
	pUploadData->MachineType = RMS_GetMachineType();

	// 8. Serial Number
	strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ATMSERIALNO);
	if(strTemp == "0000000000") //default setting value = "0000000000"
		strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID);
	strTemp.TrimLeft();
	strTemp.TrimRight();
	RMS_WideToMulti(strTemp, (char*)pUploadData->SerialNumber, sizeof(pUploadData->SerialNumber));
	
	CST_STATUS	CSTStatus;
	GetCSTStatus(&CSTStatus);
	
	// 8. CST1 Denomination
	pUploadData->CST1Denomination = CSTStatus.CST1Denomination;
	
	// 9. CST1 Bill Count
	pUploadData->CST1BillCount = CSTStatus.CST1BillCount;
	
	// 10. CST2 Denomination
	pUploadData->CST2Denomination = CSTStatus.CST2Denomination;
	
	// 11. CST2 Bill Count
	pUploadData->CST2BillCount = CSTStatus.CST2BillCount;
	
	// 12. CST3 Denomination
	pUploadData->CST3Denomination = CSTStatus.CST3Denomination;
	
	// 13. CST3 Bill Count
	pUploadData->CST3BillCount = CSTStatus.CST3BillCount;
	
	// 14. CST4 Denomination
	pUploadData->CST4Denomination = CSTStatus.CST4Denomination;
	
	// 15. CST4 Bill Count
	pUploadData->CST4BillCount = CSTStatus.CST4BillCount;
	
	// 16. Journal Count
	if (m_pDevCmn->m_JNLMgr.GetUploadRemainCount(UP_RMS) > 0)
		pUploadData->JournalCount = (NUINT16)m_pDevCmn->m_JNLMgr.GetUploadRemainCount(UP_RMS);
	else
		pUploadData->JournalCount = 0;
	
	// 17. ATM Status.
	BYTE bATMStatus = (BYTE)MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ATMSTATUS);
	
	if (bATMStatus > ATM_RMSACTIVE) 
		bATMStatus = 0xff;
	else
		bATMStatus +=0x31;
	
	pUploadData->ATMStatus = bATMStatus;
	
	// 18. Error Code.
	if (!m_pDevCmn->fnAPL_CheckError())
		sprintf((char*)pUploadData->ErrorCode, "%7.7S", m_pDevCmn->fstrAPL_GetErrorCode());
	else
		memcpy(pUploadData->ErrorCode, "0000000", 7);

	return TRUE;

}

BOOL CRmsCtrl::RMS_WirteWeatherDataMsg(BYTE *pMsg, int Len)
{
	PRMS_WEATHER_DATA 	pWeatherData;

	pWeatherData = (PRMS_WEATHER_DATA)pMsg;

	CString strTemp;
	CString strTotalString= _T("");
	strTemp.Format(_T("[WeatherInfo]\r\nTotalCount=%d\r\nTemperatureType=%d"), 
		pWeatherData->ContentsCount, pWeatherData->TemperatureType );
	strTotalString = strTemp;

	for ( int i = 0; i < pWeatherData->ContentsCount; i++) 
	{
		strTemp.Format(L"\r\n\r\n[Weather%d]\r\nYear=%d\r\nMonth=%d\r\nDay=%d\r\nDayOfWeek=%S\r\nWeatherCode=%d\r\nHighestTemp=%d\r\nLowestTemp=%d\r\n",
			i+1, pWeatherData->WeaterInfo[i].Year, pWeatherData->WeaterInfo[i].Month, pWeatherData->WeaterInfo[i].Day, pWeatherData->WeaterInfo[i].DayOfWeek,
			pWeatherData->WeaterInfo[i].WeatherCode, pWeatherData->WeaterInfo[i].HighestTemp, pWeatherData->WeaterInfo[i].LowestTemp);
		strTotalString += strTemp;
	}
	
	CFile datFile;
	CFileException e;

	DeleteFile(WEATHER_FILENAME);

	if(datFile.Open(WEATHER_FILENAME, CFile::modeCreate|CFile::modeWrite, &e))
	{

		UINT nSize = strTotalString.GetLength();

		if (nSize > 0)	// [#2022] NH KSK 2011.02.21 Code Sonar 지적사항 대책
		{
			BYTE *pBuffer = new BYTE[nSize+1];
			memset(pBuffer, 0, nSize+1);
			RMS_WideToMulti(strTotalString , (char*)pBuffer, nSize);
			datFile.Write(pBuffer, nSize);
			delete [] pBuffer;
		}				// end of [#2022]

		datFile.Close();
	}

	return TRUE;
}
//end of [#586] 

BOOL CRmsCtrl::RMS_WriteSetupDataMsg(BYTE *pMsg, int Len)
{
	int i;
	PRMS_SETUP_DATA	pSetupData;
	CString strTemp;

	// check buffer size.
	// RMS에서 AP의 RMS 버전을 보고 RMS_SETUP_DATA의 크기를 정확히 내려 준다.
	// 현재 SETUP 데이터 보다 크면 버린다. (ATM은 하위 호환은 가능하나 상위 호환은 RMS에서 책임을 진다.)
	if (LEN_RMS_SETUP_DATA < Len)		// 2008.07.03 by Cha Bug fix.
		return FALSE;

	pSetupData = (PRMS_SETUP_DATA)pMsg;

	//[W] 1. Terminal Number - READ ONLY
	//[W] 2. Serial Number - READ ONLY

	//[W] 3. Routing ID
	RMS_MultiToWide((char*)pSetupData->RoutingID, (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
	MemSetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ROUTINGID, (LPCTSTR)gwTempBuf);

	//[W] 4. Communication ID
	RMS_MultiToWide((char*)pSetupData->CommunicationID, (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
	MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_COMMUNICATIONID, (LPCTSTR)gwTempBuf);

	//[W] 5. Host Phone number 1 (Host Tel1)
	RMS_MultiToWide((char*)pSetupData->HostTel1, (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
	MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_PHONENODEFINE1, (LPCTSTR)gwTempBuf);
	
	//[W] 6. Host Phone number 2 (Host Tel2)
	RMS_MultiToWide((char*)pSetupData->HostTel2, (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
	MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_PHONENODEFINE2, (LPCTSTR)gwTempBuf);
	
	//[W] 7. Host Phone number 3 (Host Tel3) - not support

	//[W] 8. Modem Connect Timer
//	MemSetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_CONNECTTIMER, (int)pSetupData->ModemConnetTimer);	KSK 2009.12.23 사용 안함으로 수정
	
	//[W] 9. Modem Speaker Enable - H/W not support
//	MemSetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_MODEMSPEAKER, (int)pSetupData->ModemSpeakerEnable);	KSK 2009.12.23 사용 안함으로 수정
	
	//[W] 10. Modem Initial String.
	RMS_MultiToWide((char*)pSetupData->ModemInitialString, (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
	MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_MODEMINITSTR, (LPCTSTR)gwTempBuf);
	
	//[W] 11. Modem Vendor - READ ONLY

	//[W] 12. ENQ Timeout Interval.
//	MemSetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_ENQTIMEOUT, (int)pSetupData->ENQTimeoutInterval);	KSK 2009.12.23 사용안함으로 수정
	
	//[W] 13 EOT Check Mode
	// [#2364] US Justin 2015.07.23 remote update msg format, eot option, fast cash
	#if (RMS_VERSION >= 0x010327)
 		#if (US_VERSION || CA_VERSION || AU_VERSION)
		if( (pSetupData->EOTCheckMode>=2) && (pSetupData->EOTCheckMode<=6) )
		{
			if(pSetupData->EOTCheckMode == 2)			strTemp = EOT_OPTIONAL;
			else if(pSetupData->EOTCheckMode == 3)		strTemp = EOT_REQUIRED;
			else if(pSetupData->EOTCheckMode == 4)		strTemp = NO_EOT_REQUIRED;
			else if(pSetupData->EOTCheckMode == 5)		strTemp = NO_ENQ_REQUIRED;
			else if(pSetupData->EOTCheckMode == 6)		strTemp = NO_ENQ_EOT_REQUIRED;

			MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_EOTCHECKMODE, strTemp);
		}
		#endif
	#endif
	// End of [#2364]

	//[W] 14. Processor Code - READ ONLY	
	// [#2364] US Justin 2015.07.23 remote update msg format, eot option, fast cash
	#if (RMS_VERSION >= 0x010327)
 		#if (US_VERSION || CA_VERSION || AU_VERSION)
		if( (pSetupData->ProcessorCode==1)||(pSetupData->ProcessorCode==2)||(pSetupData->ProcessorCode==3)||(pSetupData->ProcessorCode==5) )
		{
			if(pSetupData->ProcessorCode == 1)			strTemp = MSG_HYOSUNG_TYPE;
			else if(pSetupData->ProcessorCode == 2)		strTemp = MSG_CSP200_TYPE;
			else if(pSetupData->ProcessorCode == 3)		strTemp = MSG_TRITON_TYPE;
			else if(pSetupData->ProcessorCode == 5)		strTemp = MSG_EPS_TYPE;
			MemSetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE, strTemp);
		}
		#endif
	#endif
	// End of [#2364]

	//[W] 15. Format option - not use
	
	//[W] 16. Health Check Send Enable.
	MemSetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_HEALTHYFLAG, (int)pSetupData->HealthCheckSendEnable);

	//[W] 17. Health check interval
	MemSetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_HSDELAYINT, (int)pSetupData->HealthCheckInterval);

	//[W] 18. RMS Accept Enable
	MemSetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSFLAG, (int)pSetupData->RMSAcceptEnable);

	//[W] 19. RMS Status Send Enable
	MemSetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSSTATUSSEND, (int)pSetupData->RMSStatusSendEnable);

	//[W] 20. RMS Password - READ ONLY
	//[W] 21. RMS New Password - READ ONLY

	//[W] 22. RMS Phone Number 1 (RMS Tel1)
	RMS_MultiToWide((char*)pSetupData->RMSTel1, (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
	MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSPHONENO1, (LPCTSTR)gwTempBuf);

	//[W] 23. RMS Phone Number 2 (RMS Tel2)
	RMS_MultiToWide((char*)pSetupData->RMSTel2, (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
	MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSPHONENO2, (LPCTSTR)gwTempBuf);

	//[W] 24. RMS Ring Count
	if (pSetupData->RMSRingCount > 0 && pSetupData->RMSRingCount < 8)
		MemSetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSRINGCNT, (int)pSetupData->RMSRingCount);

	//[W] 25. Speaker Volume.
	if (pSetupData->SpeakerVolume > 9)
		pSetupData->SpeakerVolume = 9;
	else if (pSetupData->SpeakerVolume < 0)
		pSetupData->SpeakerVolume = 0;
	MemSetInt(_MEM_FLD_INI_DEVINFO, _MEM_VAR_DEVINFO_VOLUMELEVEL, (int)pSetupData->SpeakerVolume);

	//[W] 26. ISO1 en/disable
	MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ISO1_FLAG, (int)pSetupData->ISO1Enable);
	
	//[W] 27. ISO2 en/disable
	MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ISO2_FLAG, (int)pSetupData->ISO2Enable);
	
	//[W] 28. ISO3 en/disable
	MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ISO3_FLAG, (int)pSetupData->ISO3Enable);
	
	//[W] 29. Surcharge Owner
	// KSK 2010.10.22 내부검사 지적사항 대책
	// RMS는 25로 되어 있으나, WINCE는 20자로 되어져 있어, ATM에서 20자리수까지만 처리하도록 수정
	RMS_MultiToWide((char*)pSetupData->SurchargeOwner, (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
	strTemp = gwTempBuf;
	MemSetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_OPSURCHARGEOWNER, strTemp.Left(20));
	
	//[W] 30. Surcharge en/disable flag
	MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHARGEENABLE, (int)pSetupData->SurchargeEnable);
	
	//[W] 31. Surcharge amount - 138.balance surcharge amount
	memset(gwTempBuf, 0, sizeof(gwTempBuf));
	swprintf(gwTempBuf, _T("%8.8d"), pSetupData->SurchargeAmount);
	MemSetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHAREAMT, (LPCTSTR)gwTempBuf);
	
	//[W] 32. Fast Cash Amount - READ ONLY
	// [#2364] US Justin 2015.07.23 remote update msg format, eot option, fast cash
	#if (RMS_VERSION >= 0x010327)
		for(i=0; i<CDU_CST_MAXFASTCASH; i++)
			MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_FASTCASH1 + i, (int) pSetupData->FastCashAmount[i] );
	#endif
	// End of [#2364]
	
	//[W] 33. Max Dispense Amount
	memset(gwTempBuf, 0, sizeof(gwTempBuf));
	swprintf(gwTempBuf, _T("%07d"), pSetupData->MaxDispenseAmount);
	MemSetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_DISPENSELIMIT, (LPCTSTR)gwTempBuf);
	
	//[W] 34. Low Currency Check Enable
	MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_LOWCRCYCHECK, (int)pSetupData->LowCurrencyCheckEnable);

	// [#275] [NH] KSK 2008.6.10 지원되는 Language Enable/Disable 되도록 수정
	//[W] 35. English Mode - Support
	//[W] 36. Spanish Mode - Support
	//[W] 37. Japanese Mode - AP not Support
	//[W] 38. French Mode - Support
	//[W] 39. Korean Mode - AP not Support

//#if (US_VERSION || AU_A_VERSION)		// [#2434] AU Kook 2016.08.11 Support Multi Language with MoniView
#if (US_VERSION || AU_VERSION)		// [#2468] AU KSK 2017.01.18
	MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ENGMODE, pSetupData->EnglishMode);
	MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SPNMODE, pSetupData->SpanishMode);
	MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_FRNMODE, pSetupData->FrenchMode);
	MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_CHNMODE, pSetupData->ChineseMode);
	MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_KORMODE, pSetupData->KoreanMode);
	MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_JAPMODE, pSetupData->JapaneseMode);
#elif (CA_VERSION)
	MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ENGMODE, pSetupData->EnglishMode);
	// SPANISH NOT SUPPORT in CA
	MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_FRNMODE, pSetupData->FrenchMode);
	MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_CHNMODE, pSetupData->ChineseMode);
	MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_KORMODE, pSetupData->KoreanMode);
	MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_JAPMODE, pSetupData->JapaneseMode);
#elif (MX_VERSION)
	// [#2115] MX KSK 2012.02.05
	MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ENGMODE, pSetupData->EnglishMode);
	MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SPNMODE, pSetupData->SpanishMode);
	// end of [#2115]
#endif
	// end of KSK 2011.07.18


	//[W] 40. Chinese Mode - AP not Support
	//[W] 41. Select Receipt - AP not Support

	// US V01.03.04부터 적용
	//[W] 42. Receipt Enable
	MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SELECTRECEIPT, pSetupData->SelectReceipt);
	// end of [#275]

	//[W] 43. Receipt Header 1-4
	for (i = 0; i < 4; i++)
	{
		RMS_MultiToWide((char*)pSetupData->ReceiptHeader[i], (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
		MemSetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_RECEIPT_HEADER1+i, (LPCTSTR)gwTempBuf);
	}

	//[W] 44. Receipt Address 1-4
	for (i = 0; i < 3; i++)
	{
		RMS_MultiToWide((char*)pSetupData->ReceiptAddress[i], (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
		MemSetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_RECEIPT_ADDRESS1+i, (LPCTSTR)gwTempBuf);
	}

	//[W] 45. Receipt Phone No.
	RMS_MultiToWide((char*)pSetupData->ReceiptPhoneNo, (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
	MemSetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_RECEIPT_PHONENO, (LPCTSTR)gwTempBuf);

	//[W] 46. VFD Display Option - not support
	//[W] 47. VFD Font Option - not support
	//[W] 48. VFD Text - not support
	//[W] 49. Master Password - READ ONLY
	//[W] 50. Service Password - READ ONLY
	//[W] 51. Operator Password - READ ONLY
	//[W] 52. Machine Type - READ ONLY
	//[W] 53. SPR Type - READ ONLY
	//[W] 54. JPR Type - READ ONLY
	//[W] 55. MCU Type - READ ONLY
	//[W] 56. CDU Type - READ ONLY
	//[W] 57. PB Type - READ ONLY
	//[W] 58. Key Type - READ ONLY
	//[W] 59. Sound Type - READ ONLY
	//[W] 60. Camera Type - READ ONLY
	//[W] 61. IC Card Type - READ ONLY
	//[W] 62. Power Type - READ ONLY
	//[W] 63. SPL Device - not support
	//[W] 64. VFD Device - not support
	//[W] 65. Dual Monitor - not support
	//[W] 66. DVR Device - not support
	//[W] 67. OP Device - not support
	//[W] 68. MCU Manufacturer - not support
	//[W] 69. CDU Shutter - not support
	//[W] 70. Ada Device - READ ONLY
	//[W] 71. Cash Accepter - not support
	//[W] 72. Deposit Device - not support

#if !(AU_VERSION)	// [#2069] NH KSK 2011.06.13
	//[W] 73. Bin List Count.
	if (pSetupData->BINListCount > BINLIST_SIZE)
		pSetupData->BINListCount = BINLIST_SIZE;
	else if (pSetupData->BINListCount < 0)
		pSetupData->BINListCount = 0;
	MemSetInt(_MEM_FLD_APP_BININFO, _MEM_VAR_APP_SETBINLIST, (int)pSetupData->BINListCount);

	//[W] 74. BIN List
	for(i = 0; i < BINLIST_SIZE; i++)
	{
		if (i < pSetupData->BINListCount)
		{
			RMS_MultiToWide((char*)pSetupData->BINList[i], (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
			MemSetStr(_MEM_FLD_APP_BININFO, _MEM_VAR_APP_BINLIST+i, (LPCTSTR)gwTempBuf);
		}
		else
		{
			MemSetStr(_MEM_FLD_APP_BININFO, _MEM_VAR_APP_BINLIST+i, _T(""));
		}
	}
#endif
	//[W] 75. RMS SendInterval
	if (pSetupData->RMSSendInterval <= 0)
		pSetupData->RMSSendInterval = 1;
	if (pSetupData->RMSSendInterval >= 24)
		pSetupData->RMSSendInterval = 24;
	MemSetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_RMSSENDINTERVAL, pSetupData->RMSSendInterval);

	//[W] 76. RMS Journal Upload with Status - AP not Support
	//[W] 77. CDU Low Count - not support
	//[W] 78. RMS Only Callback - not support
	//[W] 79. RMS Callback Retry Count - not support

	// [#2509] US Justin 2017.09.27 PAI Request - Chaning ATM network setting : PAI Version ONLY
	// SKIP APPLYING ATM NETWORK SETTING for all versions except PAI
	//[W] 80. DHCP Enable - READ ONLY
	//[W] 81. ATM IP - READ ONLY
	//[W] 82. SubnetMask - READ ONLY
	//[W] 83. GateWay - READ ONLY
	//[W] 84. DNS Server - READ ONLY

	#if( (RMS_VERSION>=0x010332) && ((APP_CUSTOM_PAI)||(APP_CUSTOM_CASHDEPOT)))    // RWC6-669 Update Network Fields CASH DEPOT Moniview
	//[W] 80. DHCP Enable
	if(pSetupData->DHCPEnable>0)	MemSetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_DHCP, ENABLE);
	else							MemSetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_DHCP, 0);

	//[W] 81. ATM IP
	RMS_MultiToWide((char*)pSetupData->ATMIP, (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
	MemSetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_IPADDR,  (LPCTSTR)gwTempBuf);

	// [#2538] US Justin 2018.03.09 Trim String.
	strTemp = MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_IPADDR );
	strTemp.TrimLeft(), strTemp.TrimRight();
	MemSetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_IPADDR,  strTemp);
	// End of [#2538]

	//[W] 82. SubnetMask
	RMS_MultiToWide((char*)pSetupData->SubnetMask, (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
	MemSetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_SUBNET,  (LPCTSTR)gwTempBuf);

	// [#2538] US Justin 2018.03.09 Trim String.
	strTemp = MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_SUBNET );
	strTemp.TrimLeft(), strTemp.TrimRight();
	MemSetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_SUBNET, strTemp);
	// End of [#2538]

	//[W] 83. GateWay
	RMS_MultiToWide((char*)pSetupData->GateWay, (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
	MemSetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_GATEWAY, (LPCTSTR)gwTempBuf);

	// [#2538] US Justin 2018.03.09 Trim String.
	strTemp = MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_GATEWAY );
	strTemp.TrimLeft(), strTemp.TrimRight();
	MemSetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_GATEWAY, strTemp);
	// End of [#2538]

	//[W] 84. DNS Server
	RMS_MultiToWide((char*)pSetupData->DNSServer, (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
	MemSetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_DNS,	  (LPCTSTR)gwTempBuf);

	// [#2538] US Justin 2018.03.09 Trim String.
	strTemp = MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_DNS);
	strTemp.TrimLeft(), strTemp.TrimRight();
	MemSetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_DNS, strTemp);
	// End of [#2538]

	#endif
	// End of [#2509]

	//[W] 85. IsHostUseURL
	MemSetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ISHOSTUSEURL, (int)pSetupData->IsHostUseURL);

	//[W] 86. Host1 Name
	RMS_MultiToWide((char*)pSetupData->HostName1, (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
	MemSetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_HOST1NAME, (LPCTSTR)gwTempBuf);

	//[W] 87. Host1 Port
	MemSetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_HOST1PORTNO, (int)pSetupData->HostPort1);

	//[W] 88. Host2 Name
	RMS_MultiToWide((char*)pSetupData->HostName2, (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
	MemSetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_HOST2NAME, (LPCTSTR)gwTempBuf);

	//[W] 89. Host2 Port
	MemSetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_HOST2PORTNO, (int)pSetupData->HostPort2);

	//[W] 90. Network type - READ ONLY

	//[W] 91. SSL Enable
	MemSetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_SSLENABLE, (int)pSetupData->SSLEnable);

	//[W] 92. TCP Type
	MemSetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_TYPE, (int)pSetupData->TCPType);

	//[W] 93. CRC Enable
	MemSetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_CRCENABLE, (int)pSetupData->CRCEnable);

	//[W] 94. IsRMSUseUrl
	MemSetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ISRMSUSEURL, (int)pSetupData->IsRMSUseURL);

	//[W] 95. RMS Connection Type - not support

	//[W] 96. RMS Name
	RMS_MultiToWide((char*)pSetupData->RMSName, (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
	MemSetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_RMSNAME, (LPCTSTR)gwTempBuf);

	//[W] 97. RMS Inbound Port No
	MemSetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_RMSPORTNO, (int)pSetupData->RMSInboundPort);

	//[W] 98. RMS Outbound Port No
	MemSetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_RMSSVRPORTNO, (int)pSetupData->RMSOutboundPort);

	//[W] 99. Advertisement Guide Display Term
	MemSetInt(_MEM_FLD_ADVINFO, _MEM_VAR_ADV_GUIDEDISP_TIME, (int)pSetupData->AdvGuideTerm);

	//[W] 100. Advertisement Enable
	for (i = 0; i < 6; i++)
	{
		if (pSetupData->AdvEnable & (1 << i))
			MemSetInt(_MEM_FLD_ADVINFO, _MEM_VAR_ADV_ENDISFLAG1+i, 1);
		else
			MemSetInt(_MEM_FLD_ADVINFO, _MEM_VAR_ADV_ENDISFLAG1+i, 0);
	}

	//[W] 101. advertisement Title
	for (i = 0; i < 6; i++)
	{
		RMS_MultiToWide((char*)pSetupData->AdvTitle[i], (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
		MemSetStr(_MEM_FLD_ADVINFO, _MEM_VAR_ADV_TITLE1 + (i*3), (LPCTSTR)gwTempBuf);
	}

	//[W] 102. Coupon Enable.
	for (i = 0; i < 6; i++)
	{
		if (pSetupData->AdvCouponEnable & (1 << i))
			MemSetInt(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON1_ENDISFLAG+i, 1);
		else
			MemSetInt(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON1_ENDISFLAG+i, 0);
	}

	//[W] 103 - 108
	for (i = 0; i < 2; i++)
	{
		//[W] 103. coupon1 msg 1,2 [#11] NH KSK 2010.09.27
		RMS_MultiToWide((char*)pSetupData->AdvCouponMsg1[i], (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
		MemSetStr(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON1_TEXT1+i, (LPCTSTR)gwTempBuf);

		//[W] 104. coupon2 msg 1,2 [#11] NH KSK 2010.09.2
		RMS_MultiToWide((char*)pSetupData->AdvCouponMsg2[i], (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
		MemSetStr(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON2_TEXT1+i, (LPCTSTR)gwTempBuf);

		//[W] 105. coupon3 msg 1,2 [#11] NH KSK 2010.09.2
		RMS_MultiToWide((char*)pSetupData->AdvCouponMsg3[i], (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
		MemSetStr(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON3_TEXT1+i, (LPCTSTR)gwTempBuf);

		//[W] 106. coupon4 msg 1,2 [#11] NH KSK 2010.09.2
		RMS_MultiToWide((char*)pSetupData->AdvCouponMsg4[i], (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
		MemSetStr(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON4_TEXT1+i, (LPCTSTR)gwTempBuf);

		//[W] 107. coupon5 msg 1,2 [#11] NH KSK 2010.09.2
		RMS_MultiToWide((char*)pSetupData->AdvCouponMsg5[i], (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
		MemSetStr(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON5_TEXT1+i, (LPCTSTR)gwTempBuf);

		//[W] 108. coupon msg 1,2 [#11] NH KSK 2010.09.2
		RMS_MultiToWide((char*)pSetupData->AdvCouponMsg6[i], (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
		MemSetStr(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON6_TEXT1+i, (LPCTSTR)gwTempBuf);
	}

	//[W] 109. AID List (for EMV) - [UK][NZ] - Read Only
	//[W] 110. Encryption Key - Read Only

#if (US_VERSION || CA_VERSION || AU_VERSION)	// [###]
	//[W] 111. Percent Surcharge Enable - [US]
	MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PERSURCHARGE_ENABLE, pSetupData->PercentSurchargeEnable);
	//[W] 112. Surcharge Warning Manner - [US]
	MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PERSURCHARGE_MANNER, pSetupData->SurchargeWarningManner);
	//[W] 113. Percent Surcharge Amount - [US]
	// [#388] [NH] KSK 2008.7.30
	if (pSetupData->PercentSurchargeAmount > 0 && pSetupData->PercentSurchargeAmount < 100)
		MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PERSURCHARGE_PERCENT, pSetupData->PercentSurchargeAmount);
	// end of [#388]
#endif

// [#525] US KSK 2009.05.25
#if (US_VERSION || AU_VERSION || CA_VERSION)	// RMS VERSION V010309부터 지원		// AIREAT, AU/NZ 지원.	// [#548] CA KSK 2009.07.24	// [#2069] NH KSK 2011.06.13
	//[W] 114. Pre-dial Enable - [UK] - not implement
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PREDIALENDISABLE, pSetupData->PreDialEnable);
	//[W] 115. Pre-dial Option - [UK] - not implement
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PREDIALOPTION, pSetupData->PreDialOption);
#endif
// end of [#525]

#if (US_VERSION || MX_VERSION)	// [#2115] MX KSK 2012.02.05
	//[W] 116. Balance at First Enable - [US] [MX]
	MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PREBALANCE, pSetupData->BalanceAtFirstEnable);
#endif

	//[W] 117. FastCash at First Enable - [XP] - not support

	//[W] 118. Mode10 Enable
	MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_MOD10_ENABLE, pSetupData->Mode10Enable);

	//[W] 119. Status Monitoring
	MemSetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_STATUSMONITORINGENABLE, pSetupData->StatusMonitoring);
	
	//[W] 120. Surcharge Owner Print Enable - [XP] - not support

	//[#416] NZ AIREAT 2008.09.08
	// US는 V01.03.07부터 지원함
	//[W] 121. Trial Day Total Enable - [NZ]
	MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_AUTODAYTOTAL_ENABLE, pSetupData->AutoDayTotalEnable);
	
	//[W] 122. Trial Day Total Time - [NZ]
	{
		RMS_MultiToWide((char*)pSetupData->AutoDayTotalTime, (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
		CString strTemp = gwTempBuf;
		MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_AUTODAYTOTAL_TIME_HOUR, Asc2Int(strTemp.Left(2)));
		MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_AUTODAYTOTAL_TIME_MIN, Asc2Int(strTemp.Right(2)));
	}

	//[W] 123. Last Trial Date - [NZ] -- Readonly
	// end of [#416]

	// US V01.03.04부터 적용
	// [#192] UK AIREAT 2008.5.16
	//[W] 124. Welcome Message - [UK] - 3개만 지원.
	for (i = 0; i < 3; i++)
	{
		RMS_MultiToWide((char*)pSetupData->WelcomeMessage[i], (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
		MemSetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_WELCOME_MESSAGE1+i, (LPCTSTR)gwTempBuf);
	}
	// end of [#192]

#if (AU_VERSION) // [#2031] NZ KJW 2011.03.16	// [#2069] NH KSK 2011.06.13
	// [#337] AU AIREAT 2008.06.25
	for (i = 0; i < 4; i++)
	{
		//[W] 125. Store Message - [AU]
		RMS_MultiToWide((char*)pSetupData->StoreMessage[i], (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
		MemSetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_STORE_MESSAGE1+i, (LPCTSTR)gwTempBuf);

		//[W] 126. Processor Message - [AU]
		RMS_MultiToWide((char*)pSetupData->ProcessorMessage[i], (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
		MemSetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PROCESSOR_MESSAGE1+i, (LPCTSTR)gwTempBuf);

		//[W] 127. Marketing Message - [AU]
		RMS_MultiToWide((char*)pSetupData->MarketingMessage[i], (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
		MemSetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_MARKETING_MESSAGE1+i, (LPCTSTR)gwTempBuf);
	}
	// end of [#337]
#endif

// [#571] NH KSK 2009.10.05
//	//[W] 128. Checking Account - [UK]

//	//[W] 129. Saving Account - [UK]
	//[#570] SOOK 2009.10.06 호주 Saving Account로 설정 가능하도록 변경함 
#if (AU_VERSION) // [#2031] NZ KJW 2011.03.16	// [#2069] NH KSK 2011.06.13
	MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_SAVINGS, pSetupData->SavingAccount);
#endif //end of [570]

//	//[W] 130. Credit Account - [UK]
#if (AU_VERSION || US_VERSION || CA_VERSION) // [#2031] NZ KJW 2011.03.16	// [#2069] NH KSK 2011.06.13
	//[W] 130. Credit Account - [AU]
	MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CREDITCARD, pSetupData->CreditAccount);
#endif

#if (RMS_VERSION >= 0x010301)	// [#286] [NH] KSK 2008.6.12

	// V010301까지의 크기
	GUARD_MESSAGE_LEN(Len, pSetupData, ContinueAfterPreBalance)
 
// 	#if (MX_VERSION)				// [#311] NH AIREAT 2008.06.18
// 		// [#284] [MX] KSK 2008.6.11
// 		//[W] 132. Surcharge (Cash)
// 		MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_WITHDRAWAL_SURCHARGE, pSetupData->CashWithdrawalSurcharge);
// 		//[W] 133. Surcharge (Balance)
// 		MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_BALANCE_SURCHARGE, pSetupData->BalanceSurcharge);
// 		//[W] 134. Surcharge (PIN Change)
// 		MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PINCHANGE_SURCHARGE, pSetupData->PINChangeSurcharge);
// 
// 		//[W] 135. Default Language
// 		MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_DEFAULT_LANGUAGE, pSetupData->DefaultLanguage);
// 		MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ENGMODE + (pSetupData->DefaultLanguage-1), ENABLE);
// 
// 		//[W] 136. Continue After Pre-Balance
// 		MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_CONTINUETRANS_ENABLE, pSetupData->ContinueAfterPreBalance);
// 		// end of [#284]
// 	// [#441] [US] KSK 2008.11.03
// 
// 	#elif (US_VERSION)	// 2008.07.03 by PSC canada default language 추가.
 	#if (US_VERSION || MX_VERSION)	// 2008.07.03 by PSC canada default language 추가.	// [#2115] MX KSK 2012.02.05
		//[W] 135. Default Language
		MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_DEFAULT_LANGUAGE, pSetupData->DefaultLanguage);
		MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ENGMODE + (pSetupData->DefaultLanguage-1), ENABLE);
		//[W] 136. Continue After Pre-Balance
		MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_CONTINUETRANS_ENABLE, pSetupData->ContinueAfterPreBalance);

	#elif (CA_VERSION)	// 2008.07.03 by PSC canada default language 추가.
		//[W] 135. Default Language
		MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_DEFAULT_LANGUAGE, pSetupData->DefaultLanguage);
		MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ENGMODE + (pSetupData->DefaultLanguage-1), ENABLE);
	#endif							// end of [#311]
	// end of [#441]
#endif							// end of [#286] 


#if (RMS_VERSION >= 0x010303)

	// V010303까지의 크기
	GUARD_MESSAGE_LEN(Len, pSetupData, nInquirySurchargeAmount)

	#if (AU_VERSION) // [#2031] NZ KJW 2011.03.16	// [#337] AU AIREAT 2008.06.25	// [#2069] NH KSK 2011.06.13
		// [W] 137. Exit Message
		for (i = 0; i < 3; i++)
		{
			RMS_MultiToWide((char*)pSetupData->ExitMessage[i], (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
			MemSetStr(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_EXIT_MESSAGE1+i, (LPCTSTR)gwTempBuf);
		}

		// [W] 138. Inquiry Surcharge - 29, 30, 31 surcharge 관련 값.
		MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_INQUIRY_SURCHARGE, pSetupData->nInquirySurchargeAmount);
	#endif				// end of [#337]
#endif

#if (RMS_VERSION >= 0x010304)	// [#366] [NH] KSK 2008.7.15

	// V010304까지의 크기
	GUARD_MESSAGE_LEN(Len, pSetupData, CheckCashingAvailable)
#endif							// end of [#366]

#if (RMS_VERSION >= 0x010305)	// [#397] [NH] psc 2008.08.01 EMV 관련 옵션 추가.
	
	// V010305까지의 크기
	GUARD_MESSAGE_LEN(Len, pSetupData, EmvTransactionOption)
	
	// [W] 143. EMV Enable
	if (m_pDevCmn->fnMCU_IsEmvAvailable())	// [#494] [NH] KSK 2009.2.6
	{
		MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_EMV_ENABLE, pSetupData->EmvEnable);

		// [W] 145. EMV transaction option(MS/IC first) - 0: MS first, 1: IC first
//		MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_EMV_TRANSACTIONOPTION, pSetupData->EmvTransactionOption);	[#2082] CA KSK 2011.07.02

	}

	// [#2150] US Justin 2012.10.03 Surcharge Warning => After Amount (Dynamic Flow)
	/*
//	#if (US_VERSION || CA_VERSION)	// [#493] [CA] KSK 2009.2.6
	#if (US_VERSION)	// [#493] [CA] KSK 2009.2.6	[#2089] CA KSK 2011.08.13 CA는 IC거래로 인해 무조건 AFTER AMOUNT로만 설정하게 변경
		// [W] 144. Surcharge Warning location
		MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_SURCHARGE_WARNINGLOCATION, pSetupData->nSurchargeWarningLocation);
	#endif
	*/
	// End of [#2150]
#endif							// end of [#397]

#if (RMS_VERSION >= 0x010307)
	// V010307까지의 크기
	GUARD_MESSAGE_LEN(Len, pSetupData, SurchargeOwnerContactInfo)

	// [#434] NH AIREAT 2008.10.15
		// [W] 146 - Auto Day Total Type
		if (pSetupData->AutoDayTotalType == AUTO_DAY_TOTAL)
			MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_AUTODAYTOTAL_TYPE, AUTO_DAY_TOTAL);
		else if (pSetupData->AutoDayTotalType == AUTO_TRIAL_DAY_TOTAL)
			MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_AUTODAYTOTAL_TYPE, AUTO_TRIAL_DAY_TOTAL);

	#if (AU_VERSION) // [#2031] NZ KJW 2011.03.16 NZ적용 확인필요!	// [#2069] NH KSK 2011.06.13
		// [W] 147 - Surcharge Owner Contact Info
		for (i = 0; i < 3; i++)
		{
			RMS_MultiToWide((char*)pSetupData->SurchargeOwnerContactInfo[i], (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
			MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SURCHARGEOWNER_CONTACT1+i, (LPCTSTR)gwTempBuf);
		}
	#endif
	// end of [#434]
#endif

// [#518] [MX] KSK 2009.3.9
 #if (RMS_VERSION >= 0x010308)
	GUARD_MESSAGE_LEN(Len, pSetupData, CSTCurrencyID)
 
// 	#if (MX_VERSION)
// 		// [W] 148 - Currency Exchange Enable (Read Only)
// 		// [W] 149 - Currency Exchange Rate (별도의 Command로 setting하므로 무시한다)
// 		// [W] 150 - Cassette(1~4) Currency ID (Read Only)
// 	#endif
 #endif
// end of [#518]

// [#528] AU AIREAT 2009.06.02
#if (RMS_VERSION >= 0x010310)
	GUARD_MESSAGE_LEN(Len, pSetupData, NeedMoreTime)

	#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
		// [W] 151 - Need More Time
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_NEEDMORETIME, pSetupData->NeedMoreTime);
	#endif
#endif
// end of [#528]

// [#546] SOOK 2009.07.23 다기능 BIN LIST 기능 추가함 
#if (RMS_VERSION >= 0x010311)
// [W] 152 - AwardCouponEnable
	GUARD_MESSAGE_LEN(Len, pSetupData, MultipleBinAvailable)

	#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
		for ( i = 0; i < 6; i++)
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_AWARD_COUPON1_ENABLE + i, pSetupData->AwardCouponEnable[i]);
	#endif
// [W] 153 - Multiple Bin Available은 Read Only
#endif
// end of [#546]

// [#558] KSK 2009.08.17 STANDARD1 OPTION 기능 추가
#if (RMS_VERSION >= 0x010312)

	GUARD_MESSAGE_LEN(Len, pSetupData, Standard1OptionAvailable)

	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_STANDARD1OPTION_AVAILABLE) == ENABLE)
	{
		// [R] 154 - TERMINAL STATUS FIELD ENABLE
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_TERMINAL_STATUS_FIELD_ENABLE, pSetupData->TerminalStatusFieldEnable);
	
		// [R] 155 - REVERSAL WHEN PROTOCOL ERROR
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_REVERSAL_AT_HOSTERROR, pSetupData->ReversalAtHostError);
	}	
	
	// [R] 156 - STANDARD1 OPTION AVAILABLE은 Read Only
#endif
// end of [#558]
#if (RMS_VERSION >= 0x010313)
	GUARD_MESSAGE_LEN(Len, pSetupData, ReasonForReversal)

	// [R] 157 ~ 164 : WINXP SETUP 정보임 

	// [R] 165 - SPR PAPER LOW SENSOR EN/DISABLE
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PAPER_LOW_SENSOR_ENABLE  , pSetupData->PaperLowSensorEnable);//[#596] SOOK 2009.12.11

	// [R] 166 - EPP Flicker Option
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EPP_FLICKER_OPTION, pSetupData->EPPFlickerOption);	// [#604] NH KSK 2010.01.05
	
	// [R] 167 - CST Sound Option
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST_SOUND_OPTION, pSetupData->CSTSoundOption);	// [#604] NH KSK 2010.01.05

	// [R] 168 - Emv Latch Option
	// [#2073] NH KSK 2011.07.09 Sankyo Dip인 경우는 무조건 Latch Enable 해야함
	if (m_pDevCmn->GetDeviceType(L"MCU") == MCU_MAGTEK_DIP || m_pDevCmn->GetDeviceType(L"MCU") == MCU_SANKYO_DIP)	// [#2458] AU KSK 2016.12.08 Sankyo Latch Option 추가
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EMV_LATCH_ENABLE, pSetupData->EmvLatchOption);	// [#605] NH KSK 2010.01.06
	// end of [#2073]

	// [R] 169 - Reason For Reversal Option (Standard1 Option)
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_REASONFORREVERSAL_ENABLE, pSetupData->ReasonForReversal);	// [#606] NH KSK 2010.01.06
#endif

#if (RMS_VERSION >=0x010314)
	GUARD_MESSAGE_LEN(Len, pSetupData, TransactionAdver2Enable)

	// [R] 170 - Configuration Option	// [#615] AU_C KSK 2010.01.21
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CONFIGURATION_ENABLE, pSetupData->ConfigurationOption);

	// [#2457] US Justin 2016.12.09 Support Moniview Journal Uploading	
	// [#2500] CA Justin 2017.08.22 Support Moniview Journal Uploading
	#if (AU_VERSION || US_VERSION  || CA_VERSION)		// [#2037] AU KSK 2011.03.31
	{
		// [R] 171 - Scheduled Journal Upload Enable
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_ENABLE, pSetupData->ScheduledJournalEnable);

		// [R] 172 - Scheduled Journal Upload Option
		if (pSetupData->ScheduledJournalOption >= 0 && pSetupData->ScheduledJournalOption <= 2)
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_OPTION, pSetupData->ScheduledJournalOption);

		// [R] 173 - Scheduled Journal Count
		if (pSetupData->ScheduledJournalCount >= 1 && pSetupData->ScheduledJournalCount <= 9999)
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_COUNT, pSetupData->ScheduledJournalCount);

		// [R] 174 - Scheduled Journal Day
		if (pSetupData->ScheduledJournalDay >= 1 && pSetupData->ScheduledJournalDay <= 31)
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_DAY, pSetupData->ScheduledJournalDay);

		// [R] 175 - Scheduled Journal Hour
		if (pSetupData->ScheduledJournalHour >= 00 && pSetupData->ScheduledJournalHour <= 23)
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_HOUR, pSetupData->ScheduledJournalHour);

		// [R] 176 - Scheduled Journal Available
	}
	#endif
	// End of [#2457]
	// End of [#2500]

  #if ( AU_VERSION )	// [#2037] AU KSK 2011.03.31
	// [R] 177 - AD Brand Name
	// [R] 178 - Transaction AD Enable1
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_TRANSACTION_ADVER1_ENABLE, pSetupData->TransactionAdver1Enable);
	// [R] 179 - Transaction AD Enable2
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_TRANSACTION_ADVER2_ENABLE, pSetupData->TransactionAdver2Enable);
  #endif

#endif

#if (RMS_VERSION >= 0x010315)	// [#620] US KSK 2010.02.22
	GUARD_MESSAGE_LEN(Len, pSetupData, EnhancedCouponAvailable)

	#if (US_VERSION || CA_VERSION || MX_VERSION)	// [#2115] MX KSK 2012.02.05
	// [R] 180 - Enhanced Coupon Available (Read Only)
	#endif
#endif

#if (RMS_VERSION >= 0x010316)	// [#631] MX, JERRY 2010.03.29
	GUARD_MESSAGE_LEN(Len, pSetupData, BankNameFeePrint)

 	#if (MX_VERSION)
 		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_BANK_NAME_FEE_PRINT,	pSetupData->BankNameFeePrint);			
 	#endif
#endif // end of [#631]

#if (RMS_VERSION >= 0x010317)
		GUARD_MESSAGE_LEN(Len, pSetupData, NoticeMsg)

		// [R] 182 - SSL Version (V23 or V3)
		// [#2310] NH KSK 2014.11.18 TLS Support
		//MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SSLVERSION, pSetupData->SSLVersion);

		// [#2320] NH Justin 2015.01.14 Support NON-"Version Negotiation" Processor
		//if( (pSetupData->SSLVersion==SSL_VER3_AND_ABOVE) ||(pSetupData->SSLVersion==TLS_VER10_AND_ABOVE)||
		//	(pSetupData->SSLVersion==TLS_VER11_AND_ABOVE)||(pSetupData->SSLVersion==TLS_VER12) )
		//	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SSLVERSION, pSetupData->SSLVersion);
		//else // Default SSL_VER3_AND_ABOVE
		//	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SSLVERSION, SSL_VER3_AND_ABOVE);
		if( pSetupData->SSLVersion==CONN_UPTO_SSL_V30 ||
			pSetupData->SSLVersion==CONN_UPTO_TLS_V10 ||
			pSetupData->SSLVersion==CONN_UPTO_TLS_V11 ||
			pSetupData->SSLVersion==CONN_UPTO_TLS_V12 ||
			pSetupData->SSLVersion==CONN_UPTO_TLS_V13 )
		{
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SSLVERSION, pSetupData->SSLVersion);
		}
		else // Default
		{
			// US Jefferson 2020.6.12 Make default TLS 1.3 for all regions
				MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SSLVERSION,		CONN_UPTO_TLS_V13);			// Default UP_TO_TLSV1.2
		}
		// End of [#2320]
		// end of [#2310]

		// [R] 183 - Transaction AD Display Time
		MemSetInt(_MEM_FLD_ADVINFO, _MEM_VAR_TRAN_ADV_DISP_TIME, pSetupData->TransactionADDispTime);

		// [R] 184 - Transaction AD Enable (1~6)
		for (i=0; i<6; i++)
			MemSetInt(_MEM_FLD_ADVINFO, _MEM_VAR_TRAN_ADV_ENDISFLAG1 + i, pSetupData->TransactionADEnable[i]);

		// [R] 185 - Change Background Enable
		MemSetInt(_MEM_FLD_ADVINFO, _MEM_VAR_BACK_CHANGE_ENDISFLAG, pSetupData->ChangeBackgroundEnable);

		// [R] 186 - Change Background Screen Enable (1~6)
		for (i=0; i<6; i++)
			MemSetInt(_MEM_FLD_ADVINFO, _MEM_VAR_BACK_ENDISFLAG_TYPE1 + i, pSetupData->ChangeBackgroundScreenEnable[i]);

		// [R] 187 - Default Background Screen
		MemSetInt(_MEM_FLD_ADVINFO, _MEM_VAR_BACK_DEFAULT_TYPE, pSetupData->DefaultBackgroundScreen);

		// [#11] NH KSK 2010.09.20
		NH_OS_VERSION	eOSVersion = m_pDevCmn->m_pConfig->GetOSVersion();
//		if (eOSVersion != NH_OS_NH1800SE_B)
		if (!(eOSVersion == NH_OS_NH1800SE_B || eOSVersion == NH_OS_NH1500SE))					// [#2267] US KSK 2014.05.08
		{
			// [R] 188 - Touch Vibration Enable
			if (eOSVersion == NH_OS_NH2700_L || eOSVersion == NH_OS_NH2700_T_T)	// [#2064] NH KSK 2011.05.18
				MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_TOUCHVIBRATION_ENABLE, pSetupData->TouchVibrationEnable);

			// [R] 189 - Notice Enable
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_NOTICE_SERVICE_ENABLE, pSetupData->NoticeEnable);

			// [R] 190 - Notice Title
			RMS_MultiToWide((char*)pSetupData->NoticeTitle, (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
			MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_NOTICE_SERVICE_TITLE, (LPCTSTR)gwTempBuf);

			// [R] 191 - Notice Message
			for(i=0; i<3; i++)
			{
				RMS_MultiToWide((char*)pSetupData->NoticeMsg[i], (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
				MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_NOTICE_SERVICE_MSG1 + i, (LPCTSTR)gwTempBuf);
			}
		}
		// end of [#11]
#endif

// [#2106] NH KSK 2011.12.19 Setup 정보 추가
#if (RMS_VERSION >= 0x010318)
		GUARD_MESSAGE_LEN(Len, pSetupData, EJUPLOADEnable)

		if (m_pDevCmn->GetDeviceType(L"MCU") == MCU_SANKYO_DIP)	// Sankyo Card Reader인 경우에만 적용함
		{
			// [R] 192 - ESU Enable
			// [#2206] NH Justin 2013.06.27 ESU : Check EP version
			if( m_pDevCmn->fbESU_IsAntiSkimmingAvailable() )
				MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ANTISKIMMING_ENABLE, pSetupData->ESUEnable);
			else
				MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ANTISKIMMING_ENABLE, 0);
			// End of [#2206]

			// [R] 193 - ESU Shutter Enable
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ANTISKIMMING_SHUTTER_ENABLE, pSetupData->ESUShutterEnable);

			// [R] 194 - ESU Time Threshold
			if (pSetupData->ESUTimeThreshold >= 1 && pSetupData->ESUTimeThreshold <= 9999)
				MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ANTISKIMMING_TIMETHRESHOLD, pSetupData->ESUTimeThreshold);

			// [R] 195 - ESU Recovery Time
		#if (RMS_VERSION <= 0x010346)
			if (pSetupData->ESURecoveryTime >= 5 && pSetupData->ESURecoveryTime <= 255)
				MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ANTISKIMMING_AUTORECOVERYTIME, pSetupData->ESURecoveryTime);
		#endif

		}

		// [R] 196 - RKT Available Read Only


		// [R] 197 - RKT Enable
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RKT_ENABLE, pSetupData->RKTEnable);

		// [R] 198 - RKT Use Random Number
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RKT_RANDOMNUMBER_ENABLE, pSetupData->RKTRandomNumberEnable);

		// [R] 199 - EJ Upload Enable
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EJUPLOAD_ENABLE, pSetupData->EJUPLOADEnable);
#endif
// end of [#2106]

// [#2206] NH Justin 2013.06.27 Add additional RMS Information -- Dynamic Flow Setup, Dual Host DCC, Receipt Option
#if (RMS_VERSION >= 0x010319)
		GUARD_MESSAGE_LEN(Len, pSetupData, ReceiptOnScreen)

		// [R] 200 - Supervisore Language
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SUPERVISOR_LANGUAGE, pSetupData->SupervisorLanguage);	// [#2213] US Justin 2013.07.30 Add Supervisore Language


		// [R] 201 - Dynamic Flow En/Disable
		// [R] 202 - PIN Change 
		// [R] 203 - DCC
		// [R] 204 - DCC Option (General, CIBC) 

		// [R] 205 - Dual Host Enable
		// [R] 206 - Dual Host SSL Enable
		// [R] 207 - Dual Host SSL Version
		// [R] 208 - Dual Host URL Enable
		// [R] 209 - Dual Host Host Name1
		// [R] 210 - Dual Host Port1
		// [R] 211 - Dual Host Host Name2
		// [R] 212 - Dual Host Port2

		// [#2499] US Justin 2017.08.22
		// Before 0x010332 : PAI has an exclusive right for "Dual Host DCC"
	#if(RMS_VERSION < 0x010332)
		#if (APP_CUSTOM_PAI)
			// Dynamic Flow Settings....
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_ENABLE, 0);
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_PINCHANGE, 0);
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_DCC, 0);
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCC_CUSTOMOPTION, DCC_CUSTOMOPTION_GENERAL);

			// Dual Host 
			//  En/Disable
			if (MemGetVersion(_MEMKEY_EPVERSION, L"PIN").CompareNoCase(PIN_512K_EP_VERSION) >= 0)
				MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_ENABLE, pSetupData->DualHostDCCEnable);
			else
				MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_ENABLE, 0);

			//  SSL En/Disable
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_SSL_ENABLE, pSetupData->DualHostDCCSSLEnable);

			//  SSL Version
			// [#2310] NH KSK 2014.11.18 TLS Support
	//		if( pSetupData->DualHostDCCSSLVersion == SSL_VER_23 )
	//			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_SSL_VERSION, SSL_VER_23);
	//		else
	//			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_SSL_VERSION, SSL_VER_3);

			// [#2320] NH Justin 2015.01.14 Support NON-"Version Negotiation" Processor
			//if( (pSetupData->DualHostDCCSSLVersion==SSL_VER3_AND_ABOVE) ||(pSetupData->DualHostDCCSSLVersion==TLS_VER10_AND_ABOVE)||
			//	(pSetupData->DualHostDCCSSLVersion==TLS_VER11_AND_ABOVE)||(pSetupData->DualHostDCCSSLVersion==TLS_VER12) )
			//	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_SSL_VERSION, pSetupData->SSLVersion);
			//else // Default SSL_VER3_AND_ABOVE
			//	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_SSL_VERSION, SSL_VER3_AND_ABOVE);

			if( (pSetupData->DualHostDCCSSLVersion==CONN_UPTO_SSL_V30) || (pSetupData->DualHostDCCSSLVersion==CONN_UPTO_TLS_V10)||
				(pSetupData->DualHostDCCSSLVersion==CONN_UPTO_TLS_V11) || (pSetupData->DualHostDCCSSLVersion==CONN_UPTO_TLS_V12) )
				MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_SSL_VERSION, pSetupData->SSLVersion);
			else // Default UP TO TLS12
				MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_SSL_VERSION, CONN_UPTO_TLS_V12);
			// End of [#2320]
			// end of [#2310]

			//  Use URL?
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_ISHOSTUSEURL, pSetupData->DualHostDCCIsHostUseURL);

			//  Host IP 1
			RMS_MultiToWide((char*)pSetupData->DualHostDCCHostName1, (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
			MemSetStr(_MEM_FLD_OPTIONALSETTING_2,_MEM_VAR_OPT2_DUALHOSTDCC_HOST1NAME, (LPCTSTR)gwTempBuf);

			//  Host Port 1
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2,_MEM_VAR_OPT2_DUALHOSTDCC_HOST1PORTNO,  (int)pSetupData->DualHostDCCHostPort1);

			//  Host IP 2
			RMS_MultiToWide((char*)pSetupData->DualHostDCCHostName2, (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
			MemSetStr(_MEM_FLD_OPTIONALSETTING_2,_MEM_VAR_OPT2_DUALHOSTDCC_HOST2NAME, (LPCTSTR)gwTempBuf);

			//  Host Port 2
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2,_MEM_VAR_OPT2_DUALHOSTDCC_HOST2PORTNO,  (int)pSetupData->DualHostDCCHostPort2);

		#else
			// Dynamic Flow En/Disable
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_ENABLE, pSetupData->DynamicFlowEnable);

			// Dynamic Flow Pin Change En/Disable
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_PINCHANGE, pSetupData->DynamicFlowPINChange);

			// Dynamic Flow DCC En/Disable
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_DCC, pSetupData->DynamicFlowDCC);

			// Dynamic Flow DCC Option
			// [#2426] Justin Support STD1 Cardtronics Option
			//MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCC_CUSTOMOPTION, pSetupData->DynamicFlowDCCOption);
			#if !(AU_VERSION)	// [#2450] AU KSK 2016.11.02 AU인 경우 DCC Option이 없으므로 미설정되도록 보완처리
			BOOL bUpdateCustomerType = TRUE;
			UINT8 nMVCustomerType = pSetupData->DynamicFlowDCCOption;
			#if( APP_TDL_OPTION )
				if( nMVCustomerType == DCC_CUSTOMOPTION_GENERAL)
					bUpdateCustomerType = FALSE;
			#else
				if( nMVCustomerType == DCC_CUSTOMOPTION_CARDTRONICS)
					bUpdateCustomerType = FALSE;
			#endif
			if( bUpdateCustomerType == TRUE)
				MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCC_CUSTOMOPTION, nMVCustomerType);
			#endif				// end of [#2450]
			// End of [#2426]

			// Dual Host Setting....
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_ENABLE, 0);

			// Ignore .. Dual Host SSL Enable / SSL Version / Host IP URL / IP1 / Port1 / IP2 / Port 2
		#endif

	#else

		// Dynamic Flow En/Disable
		UINT8 nDynamicFlowOption = pSetupData->DynamicFlowEnable;
		if( (nDynamicFlowOption!=DYNAMICFLOW_DISABLE ) && (nDynamicFlowOption!=DYNAMICFLOW_ENABLE ) && (nDynamicFlowOption!=DYNAMICFLOW_DUALHOST ) )
			nDynamicFlowOption = DYNAMICFLOW_DISABLE;
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_ENABLE, nDynamicFlowOption);

		// Dynamic Flow Pin Change En/Disable => DISABLE
		//MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_PINCHANGE, pSetupData->DynamicFlowPINChange);
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_PINCHANGE, 0);

		// Dynamic Flow DCC En/Disable
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_DCC, pSetupData->DynamicFlowDCC);

		// Dynamic Flow DCC Customer Option
		#if !(AU_VERSION)	// [#2450] AU KSK 2016.11.02 AU인 경우 DCC Option이 없으므로 미설정되도록 보완처리
			BOOL bUpdateCustomerType = TRUE;
			UINT8 nMVCustomerType = pSetupData->DynamicFlowDCCOption;
			#if( APP_TDL_OPTION )
				if( nMVCustomerType == DCC_CUSTOMOPTION_GENERAL)
					bUpdateCustomerType = FALSE;
			#else
				if( nMVCustomerType == DCC_CUSTOMOPTION_CARDTRONICS)
					bUpdateCustomerType = FALSE;
			#endif
			if( bUpdateCustomerType == TRUE)
				MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCC_CUSTOMOPTION, nMVCustomerType);
		#endif				// end of [#2450]

		// Dual Host DCC
		//  En/Disable
		if( (MemGetVersion(_MEMKEY_EPVERSION, L"PIN").CompareNoCase(PIN_512K_EP_VERSION) >= 0) && (pSetupData->DualHostDCCEnable!=0) )
		{
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_ENABLE, ENABLE);
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_1STCALL_OPTION, DYNAMICFLOW_1STCALL_ON);
		}
		else
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_ENABLE, 0);

		//  SSL En/Disable
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_SSL_ENABLE, pSetupData->DualHostDCCSSLEnable);

		if( pSetupData->DualHostDCCSSLVersion==CONN_UPTO_SSL_V30 || 
			pSetupData->DualHostDCCSSLVersion==CONN_UPTO_TLS_V10 ||
			pSetupData->DualHostDCCSSLVersion==CONN_UPTO_TLS_V11 ||
			pSetupData->DualHostDCCSSLVersion==CONN_UPTO_TLS_V12 ||
			pSetupData->DualHostDCCSSLVersion==CONN_UPTO_TLS_V13 )
		{
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_SSL_VERSION, pSetupData->DualHostDCCSSLVersion);		// [#2547] NH Justin 2018.04.27 Dual Host RMS SSL/TLS Version Change Fix
		}
		else // Default UP TO TLS13
		{	
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_SSL_VERSION, CONN_UPTO_TLS_V13);
		}

		//  Use URL
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_ISHOSTUSEURL, pSetupData->DualHostDCCIsHostUseURL);

		//  Host IP 1
		RMS_MultiToWide((char*)pSetupData->DualHostDCCHostName1, (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
		MemSetStr(_MEM_FLD_OPTIONALSETTING_2,_MEM_VAR_OPT2_DUALHOSTDCC_HOST1NAME, (LPCTSTR)gwTempBuf);

		//  Host Port 1
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2,_MEM_VAR_OPT2_DUALHOSTDCC_HOST1PORTNO,  (int)pSetupData->DualHostDCCHostPort1);

		//  Host IP 2
		RMS_MultiToWide((char*)pSetupData->DualHostDCCHostName2, (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
		MemSetStr(_MEM_FLD_OPTIONALSETTING_2,_MEM_VAR_OPT2_DUALHOSTDCC_HOST2NAME, (LPCTSTR)gwTempBuf);

		//  Host Port 2
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2,_MEM_VAR_OPT2_DUALHOSTDCC_HOST2PORTNO,  (int)pSetupData->DualHostDCCHostPort2);
	#endif
	// End of [#2499]

	// [R] 213 - Disclaimer 
	#if !(AU_VERSION)	// [#2450] AU KSK 2016.11.02 AU인 경우 Disclaimer 미사용 (타국가 적용 검토 필요)
	// Line 1 ~ 3
	for (i = 0; i < 3; i++)
	{
		RMS_MultiToWide((char*)pSetupData->DCCDIsclaimer[i], (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
		MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_DCC_DISCLAIMER1+i, (LPCTSTR)gwTempBuf);
	}
	// Line 4 ~ 6
	for (i = 0; i < 3; i++)
	{
		RMS_MultiToWide((char*)pSetupData->DCCDIsclaimer[i+3], (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
		MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_DCC_DISCLAIMER4+i, (LPCTSTR)gwTempBuf);
	}
	#endif				// end of [#2450]
	// End of [#2426]

	// [R] 214 - Receipt On Screen
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RECEIPT_ON_SCREEN, pSetupData->ReceiptOnScreen);

#endif
// End of [#2206]

// [#2242] NH Justin 2013.12.18 Add DF 1st Call Option
#if (RMS_VERSION >= 0x010320)
	GUARD_MESSAGE_LEN(Len, pSetupData, DynamicFlow1stCallOption)

		#if (!AU_VERSION)	// [#2450] AU KSK 2016.11.02 AU인 경우 1STCALL을 사용 안하므로 설정 못하도록 수정함
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_1STCALL_OPTION, pSetupData->DynamicFlow1stCallOption);

			// [#2499] US Justin 2017.08.23
			if(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_ENABLE) == ENABLE)
				MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_1STCALL_OPTION, DYNAMICFLOW_1STCALL_ON);
			// End of [#2499]

		#endif				// end of [#2450]
#endif
// End of [#2242]

// [#2255] NH KSK 2014.02.26 Added NOTE Counting Option
#if (RMS_VERSION >= 0x010321)
		GUARD_MESSAGE_LEN(Len, pSetupData, NoteCountingOption)

//		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE, pSetupData->NoteCountingOption);	// KSK 2014.07.25 Monitoring만 하고 Setting은 불가하도록 함
#endif
// end of [#2255]

// [#2309] US Justin 2014.11.17 US Balance No Fee Notice
#if (RMS_VERSION >= 0x010323)
		GUARD_MESSAGE_LEN(Len, pSetupData, BalanceFeeNotice)

		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_BALANCE_NOFEE_NOTICE, pSetupData->BalanceFeeNotice);
#endif
// end of [#2309]

// [#2316] US Justin 2014.12.17 Decimal point percentage
#if (RMS_VERSION >= 0x010324)
		GUARD_MESSAGE_LEN(Len, pSetupData, DecimalPointPercentage)

		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DECIMAL_SURCHARGE,  (int)pSetupData->DecimalPointPercentage);
#endif
// end of [#2316]

// [#2342] US Justin 2015.05.14 NH Justin USE Common AID
#if (RMS_VERSION >= 0x010326)
 	#if (US_VERSION)
		GUARD_MESSAGE_LEN(Len, pSetupData, EnableDonation)

		if( (pSetupData->UseCommonAID==COMMONAID_USE_COMMON)||(pSetupData->UseCommonAID==COMMONAID_USE_ALL)||(pSetupData->UseCommonAID==COMMONAID_USE_INTERNATIONAL) )
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_USE_USCOMMONAID, pSetupData->UseCommonAID);
 	#endif
#endif
// end of [#2342]

// [#2359] US Justin 2015.07.15 Add Additional data.
#if (RMS_VERSION >= 0x010327)
 	#if (US_VERSION || CA_VERSION)
		GUARD_MESSAGE_LEN(Len, pSetupData, LoanAddEnable)

		if( (pSetupData->PercentageSurchargeNotice==SURCHARGENOTICE_AMOUNT)||(pSetupData->PercentageSurchargeNotice==SURCHARGENOTICE_PERCENTAGE)||(pSetupData->PercentageSurchargeNotice==SURCHARGENOTICE_BOTH))
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PERCENT_SURCHARGE_NOTICE, pSetupData->PercentageSurchargeNotice);

		#if (APP_POPMONEY)		// (APP_CUSTOM_PAI) [#2471] NH Justin 2017.02.01 Enable Popmomey to All US Customers
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_POPMONEY_ENABLE, pSetupData->PopmoneyEnable);
		#endif

	#endif
#endif
// end of [#2359]

// [#2405] US Justin 2016.03.21 Add Additional data - HalCash Enable
#if (RMS_VERSION >= 0x010328)
 	#if (US_VERSION || CA_VERSION )
		GUARD_MESSAGE_LEN(Len, pSetupData, LocalLoanHealthCheck)

		// En/Disabling HalCash
 		#if (APP_PIN4_CASHPICKUP)	// (APP_CUSTOM_PAI) [#2471] NH Justin 2017.02.01 Enable Pin4 to All US Customers
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PIN4_ENABLE, pSetupData->Pin4Enable);
		#endif

		// En/Disabling MoniMobile
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MOBILEAPP_QRCODE, pSetupData->MoniMobileQR);		// [#2409] US Justin 2016.03.28 Add Moniview MoniMobile Setup Information 

		// EMV Language Selection
	 	#if (US_VERSION)
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EMV_LANGUAGESELECTION, pSetupData->EMVLanguageSelection);		// [#2425] US Justin 2016.05.23
		#endif
	#endif

	#if (AU_VERSION)	// [#2469] AU KSK 2017.01.18
		GUARD_MESSAGE_LEN(Len, pSetupData, LocalLoanHealthCheck)

		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EMV_LANGUAGESELECTION, pSetupData->EMVLanguageSelection);		// [#2425] US Justin 2016.05.23
	#endif				// end of [#2469]

#endif
// end of [#2405]

//#if (RMS_VERSION >= 0x010329)
//	Not chaning Kernel Version Through Setup....
//	Kernel Version can be changed only through Moniview AID Management Command
//	pSetupData->RMSEMVKernelVersion

#if (RMS_VERSION >= 0x010330)
 	#if (US_VERSION)
		GUARD_MESSAGE_LEN(Len, pSetupData, NHDualBalance)

		// [#2444] US Justin
		if( pSetupData->NHDualBalance )		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_DUALBALANCE, ENABLE);
		else								MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_DUALBALANCE, 0);
		// End of [#2444]
	#endif
#endif
// End of [#2442]

// [#2449] US Justin Add Paypal and Just.Cash
#if (RMS_VERSION >= 0x010331)
		GUARD_MESSAGE_LEN(Len, pSetupData, HostSSLTLSCertificate)

	#if (US_VERSION)
		if( pSetupData->NHPaypalCCA )		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PAYDIANT_CCA_ENABLE, ENABLE);
		else								MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PAYDIANT_CCA_ENABLE, 0);

		// [#2496] US Justin 2017.08.17
		//if( pSetupData->NHJustCash )		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_JUSTCASH_ENABLE, ENABLE);
		//else								MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_JUSTCASH_ENABLE, 0);
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_JUSTCASH_ENABLE, pSetupData->NHJustCash);
		// End of [#2496]
	#endif

	// [#2475] US Justin 2017.02.22 
	if( pSetupData->HostSSLTLSCertificate )		MemSetInt(_MEM_FLD_OPTIONALSETTING_2,_MEM_VAR_OPT2_SSL_CERTIFY, ENABLE);
	else										MemSetInt(_MEM_FLD_OPTIONALSETTING_2,_MEM_VAR_OPT2_SSL_CERTIFY, 0);
	// End of [#2475]

#endif
// End of [#2449]

#if (RMS_VERSION >= 0x010333)
	GUARD_MESSAGE_LEN(Len, pSetupData, AlphiCurrencyCode)

	#if (AU_VERSION)
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ALPHI_ENDISABLE, pSetupData->AlphiEnDisable);

		RMS_MultiToWide((char*)pSetupData->AlphiAddress, (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
		MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ALPHI_SERVER_ADDRESS, (LPCTSTR)gwTempBuf);

		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ALPHI_SERVER_PORT, (int)pSetupData->AlphiPort);

		RMS_MultiToWide((char*)pSetupData->AlphiID, (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
		MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ALPHI_ID, (LPCTSTR)gwTempBuf);

		RMS_MultiToWide((char*)pSetupData->AlphiTermID, (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
		MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ALPHI_TERMINAL_ID, (LPCTSTR)gwTempBuf);

		RMS_MultiToWide((char*)pSetupData->AlphiOprID, (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
		MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ALPHI_OPERATOR_ID, (LPCTSTR)gwTempBuf);

		RMS_MultiToWide((char*)pSetupData->AlphiTDESKey, (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
		MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ALPHI_TDES_KEY, (LPCTSTR)gwTempBuf);

		RMS_MultiToWide((char*)pSetupData->AlphiExtraInformation, (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
		MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ALPHI_EXTRA_INFORMATION, (LPCTSTR)gwTempBuf);

		// AuditNum is not configurable.
		//MemSetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_OPT2_ALPHI_MSG_AUDITNUMBER, (int)pSetupData->AlphiMsgAuditNum);

		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ALPHI_REVERSAL_MAX_RETRY, (int)pSetupData->AlphiReversalMaxRetry);
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ALPHI_CARDDATA_TO_BE_USED, (int)pSetupData->AlphiCardDataToBeUsed);
		
		RMS_MultiToWide((char*)pSetupData->AlphiCurrencyCode, (LPWSTR)gwTempBuf, TEMP_BUFSIZE);  // [#2497] NH woooZ 2017.10.25 ALPHI currency code
		MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ALPHI_CURRENCY_CODE, (LPCTSTR)gwTempBuf);

	#endif
#endif

// [#2517] US Justin 2017.11.29 EMV Fallback Enable/Diable Option
#if (RMS_VERSION >= 0x010334)
		GUARD_MESSAGE_LEN(Len, pSetupData, EMV_Fallback_EnDisable)

	#if (US_VERSION)
		if( pSetupData->EMV_Fallback4UnknownICCard )	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_FALLBACK_4_UNKNOWNAID, 1);
		else											MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_FALLBACK_4_UNKNOWNAID, 0);

		if( pSetupData->EMV_Fallback_EnDisable )		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_FALLBACK_ENABLE_DISABLE, 1);
		else											MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_FALLBACK_ENABLE_DISABLE, 0);
	#endif

#endif
// End of [#2517]

// [#2538] NH Justin 2018.03.09 MoniMobile Cash Balance Option		
#if (RMS_VERSION >= 0x010335)
		GUARD_MESSAGE_LEN(Len, pSetupData, USTerritoryOption)

	// MoniMobile Cash Balance Option ---- MV : 0(Disable) or 1(Enable) => ATM 1(Enable), 0/2(Disable)
	if(pSetupData->MoniMobileCashBalanceOption==1)		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MONIMOBILE_CASHBALANCE, 1);
	else												MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MONIMOBILE_CASHBALANCE, 2);	// Disable

	// [#2548] NH Justin 2018.04.27 Change Camera Option Name : MoniVision, Live-Feed, Journal Pic	// [#GLDV-2505] Support MX-2800T
	if (GetConfigFuncPointer()->SupportsCamera())
	{
		if(pSetupData->MoniVisionEnable==1)				MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CAM_ENABLE, 1);
		else											MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CAM_ENABLE, 0);

		if(pSetupData->MoniVisionLiveFeedOption==1)		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CAM_ANTI_SHLDR_SURF_ENABLE, 1);
		else											MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CAM_ANTI_SHLDR_SURF_ENABLE, 0);

		if(pSetupData->MoniVisionJournalPicOption==1)	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CAM_FACE_MEMORIZE_ENABLE, 1);
		else											MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CAM_FACE_MEMORIZE_ENABLE, 0);
	}
	// End of [#2548]

	// [#2549] NH Justin 2018.05.08 Add US Territory Option
	#if (US_VERSION)
		if( (pSetupData->USTerritoryOption>=US_TERRITORY_US) && (pSetupData->USTerritoryOption<=US_TERRITORY_SAMOA) )
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_US_TERRITORY_OPTION, pSetupData->USTerritoryOption);
		else
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_US_TERRITORY_OPTION, US_TERRITORY_US);
	#endif
	// End of [#2549]

#endif
// End of [#2538]


// [#2558] NH Justin 2018.06.13 Add Scheduled Reboot Optioin
#if (RMS_VERSION >= 0x010336)
	{
		GUARD_MESSAGE_LEN(Len, pSetupData, RMSSendRetry)

		if( (pSetupData->RebootOption==REBOOT_OPTION_DISABLE)||(pSetupData->RebootOption==REBOOT_OPTION_ONTIME)||(pSetupData->RebootOption==REBOOT_OPTION_INTERVAL) )
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_REBOOT_OPTION, pSetupData->RebootOption);
		else
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_REBOOT_OPTION, REBOOT_OPTION_DISABLE);

		int nRcvHH = (int) (0.01 * pSetupData->RebootTime);
		int nRcvMM = pSetupData->RebootTime % 100;
		if( (nRcvHH>=0) && (nRcvHH<=23) && (nRcvMM>=0) && (nRcvMM<=59) )					
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_REBOOT_TIME, pSetupData->RebootTime);
		else																				
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_REBOOT_TIME, 0);

		if( (pSetupData->RebootInterval>0) && ( pSetupData->RebootInterval<=REBOOT_OPTION_MAX_INTERVAL) )
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_REBOOT_INTERVAL, pSetupData->RebootInterval);
		else																				
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_REBOOT_INTERVAL, 0);

		// [#2557] NH Justin 2018.06.12 GivePay Enhancement 
		if( pSetupData->GivePayEnable == ENABLE)	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_GIVEPAY_ENABLE_DISABLE, ENABLE);	
		else										MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_GIVEPAY_ENABLE_DISABLE, 0);	
		// End of [#2557]

		// [#2559] NH Justin 2018.06.18 Retry Status Send if "send" failed
		if(pSetupData->RMSSendRetry>=0)				MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RMSSEND_RETRY, pSetupData->RMSSendRetry);			
		else										MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RMSSEND_RETRY, 0);				
		// End of [#2559]
	}
#endif
// End of [#2558]

// [#2579] AU Kook 2018.10.12 Support ALPHI SSL Configuration via MoniView
#if (RMS_VERSION >= 0x010337)
	GUARD_MESSAGE_LEN(Len, pSetupData, AlphiSSLCertEnDisable)

	#if (AU_VERSION)
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ALPHI_SSL_ENDISABLE, pSetupData->AlphiSSLEnDisable);
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ALPHI_SSL_CERT_ENDISABLE, pSetupData->AlphiSSLCertEnDisable);	// [#2497] NH woooZ 2018.01.26 MoniView개발 후 주석 풀것
	#endif
#endif
// end of [#2579]

// [#2574] US Justin GivePay Enhancement3, Add Download Config Time
#if (RMS_VERSION >= 0x010338)
	{
		GUARD_MESSAGE_LEN(Len, pSetupData, DualHostDCCType)

		int nCfgHH = (int) (0.01 * pSetupData->GivePayConfigTime);
		int nCfgMM = pSetupData->GivePayConfigTime % 100;
		if( (nCfgHH>=0) && (nCfgHH<=23) && (nCfgMM>=0) && (nCfgMM<=59) )					
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_GIVEPAY_CONFIG_TIME, pSetupData->GivePayConfigTime);
		else																				
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_GIVEPAY_CONFIG_TIME, 0);

		// [#J006] Upsell Option
		// [#RWC6-1, #2583] US Brandon 2018.12.17 GivePay DG911 fix, WinATM error fix, Enhancement 
		//int nGivePayAppearOption = GIVEPAY_APPEAR_INDIVIDUAL; 
		int nGivePayAppearOption = GIVEPAY_APPEAR_BOTH; // end of [#RWC6-1, #2583]
		if(pSetupData->GivePayUpSell==RMS_GIVEPAY_PIGGYBACK)
			nGivePayAppearOption = GIVEPAY_APPEAR_PIGGYBACK;
		else if(pSetupData->GivePayUpSell==RMS_GIVEPAY_INDIVIDUAL)
			nGivePayAppearOption = GIVEPAY_APPEAR_INDIVIDUAL;
		else if(pSetupData->GivePayUpSell==RMS_GIVEPAY_BOTH)
			nGivePayAppearOption = GIVEPAY_APPEAR_BOTH;
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_GIVEPAY_UPSELL, nGivePayAppearOption);
		// End of [#J006]

		
		// [#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD1/STD3 message
		BOOL bUpdateDCCType = TRUE;	
		int nDynamicFlowDCCType = DYNAMICFLOW_DCC_TYPE_BOTH;	// general version default: both = 0
		int nDualHostDCCType = DUALHOST_DCC_TYPE_BOTH;			// general version default: both = 0 

		#if( APP_TDL_OPTION )
			bUpdateDCCType = FALSE;	// disabled for Cardtronics	
			nDynamicFlowDCCType = DYNAMICFLOW_DCC_TYPE_BOTH;	// Cardtronics version default: both = 0 
			nDualHostDCCType = DUALHOST_DCC_TYPE_BOTH;			// Cardtronics version default: both = 0
		#endif
			
		// DynamicFlow DCC Type Value:
		if( (bUpdateDCCType==TRUE) &&
			((pSetupData->DynamicFlowDCCType==DYNAMICFLOW_DCC_TYPE_MASTERONLY) || (pSetupData->DynamicFlowDCCType==DYNAMICFLOW_DCC_TYPE_VISAONLY) ||
			 (pSetupData->DynamicFlowDCCType==DYNAMICFLOW_DCC_TYPE_BOTH)) )
			nDynamicFlowDCCType = pSetupData->DynamicFlowDCCType; 
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DYNAMICFLOW_DCC_TYPE, nDynamicFlowDCCType);
			
		// DualHost DCC Type Value:
		if( (bUpdateDCCType==TRUE) &&
			((pSetupData->DualHostDCCType==DUALHOST_DCC_TYPE_MASTERONLY) || (pSetupData->DualHostDCCType==DUALHOST_DCC_TYPE_VISAONLY) ||
			 (pSetupData->DualHostDCCType==DUALHOST_DCC_TYPE_BOTH)) )
			 nDualHostDCCType = pSetupData->DualHostDCCType;
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOST_DCC_TYPE, nDualHostDCCType);

		/*
		int nDCCType = DYNAMICFLOW_DCC_TYPE_BOTH; 
		if( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DYNAMICFLOW_DCC_TYPE)==DYNAMICFLOW_DCC_TYPE_MASTERONLY) || (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DYNAMICFLOW_DCC_TYPE)==DYNAMICFLOW_DCC_TYPE_VISAONLY) ||
			(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DYNAMICFLOW_DCC_TYPE)==DYNAMICFLOW_DCC_TYPE_BOTH)  )
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DYNAMICFLOW_DCC_TYPE, nDCCType);
		
		// DualHost DCC Type Value:
		nDCCType = DUALHOST_DCC_TYPE_BOTH; 
		if( (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOST_DCC_TYPE)==DUALHOST_DCC_TYPE_MASTERONLY) || (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOST_DCC_TYPE)==DUALHOST_DCC_TYPE_VISAONLY) ||
			(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOST_DCC_TYPE)==DUALHOST_DCC_TYPE_BOTH)  )
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOST_DCC_TYPE, nDCCType);
		*/
		// end of [#RWC6-1, #2583]
	}
#endif
// End of [#2574]


// [#RWC6-10] Adding secondary DNS with default value of 8.8.8.8. or 8.8.4.4, for PAI
#if (RMS_VERSION >= 0x010339)
	{
		// Secondary DNS Server
		RMS_MultiToWide((char*)pSetupData->DNSServer2nd, (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
		MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ATM_DNS_2ND,	  (LPCTSTR)gwTempBuf);
	}
#endif
// end of [#RWC6-10]

	// [#RWC6-29] PAI Add ALLPOIN
#if (RMS_VERSION >= 0x010340)
	{
	#if (APP_CUSTOM_PAI)
		if(pSetupData->AllPointSurcharge==1)
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_SURCHARGE, 1);
		else
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_SURCHARGE, 0);
	#endif
	}
#endif
	// end of [#RWC6-29]

// [#RWC6-67] US William 2019.10.25 MoniView TLS
#if (RMS_VERSION >= 0x010341)
	{
		// Check the message length
		GUARD_MESSAGE_LEN(Len, pSetupData, RMSTLSEnable)

		// MoniView TLS Enabled
		bool rmsTlsEnabled = pSetupData->RMSTLSEnable == ENABLE;
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RMSTLSENABLE, (int)rmsTlsEnabled);

		// MoniView TLS Port
		WORD rmsTlsPortNumber = pSetupData->RMSTLSListeningPortNumber;
		if (rmsTlsPortNumber >= 1024 && rmsTlsPortNumber <= 65355)
		{
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RMSTLSPORT, (int)rmsTlsPortNumber);
		}
	}
#endif
// End of [#RWC6-67] US William 2019.10.25 MoniView TLS

#if (RMS_VERSION >= 0x010342)
	// HQ - Print total amount options
#endif

// [#RWC6-119] US William 2020.01.16 Add JustCash RMS changes
#if (RMS_VERSION >= 0x010343)
	GUARD_MESSAGE_LEN(Len, pSetupData, B4UEnabled)

	int justCashMode = pSetupData->JustCashMode;
	if (justCashMode >= 0 && justCashMode <= 3)
	{
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_JUSTCASH_ENABLE, justCashMode);
	}
// end of [#RWC6-119]

// [#RWC6-140] US William 2020.02.21 B4U RMS				
	{
		// B4U Enabled
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_B4U_ENABLED, pSetupData->B4UEnabled);
	}
#endif
// End of [#RWC6-140] US William 2020.02.21 B4U RMS

	// [#RWC6-170] US William 2020.05.01 Add Remote Update Config to RMS
#if (RMS_VERSION >= 0x010345)
	GUARD_MESSAGE_LEN(Len, pSetupData, ScheduledUpdateDayOfWeek)

	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULE_U2D_ENABLE, pSetupData->ScheduledUpdatesEnabled);
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULE_U2D_DOW, pSetupData->ScheduledUpdateDayOfWeek);
#endif

	// [#RWC6-169] US William 2020.04.28 Add PAI Credit Segmentation RMS
	// To add credit card segmentation for US general version as well.
#if (RMS_VERSION >= 0x010345 && (US_VERSION && !APP_TDL_OPTION))
	{
		CString creditSurchargeAmount;
		creditSurchargeAmount.Format(L"%d", pSetupData->CreditAdditionalSurchargeAmount);
		MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_CREDITSURCHARGE, creditSurchargeAmount);
	}
#endif

	// [#RWC6-178] US William 2020.50.22 Add std3 comm header enablement to RMS setup data
#if (RMS_VERSION >= 0x010346)
	GUARD_MESSAGE_LEN(Len, pSetupData, ESURecoveryTime2Byte)

	MemSetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_COMMUNICATIONHEADERENABLE, pSetupData->Standard3CommHeaderEnabled);

	// [#RWC6-168] US William 2020.06.09 Anti-skimming 2B RMS setup field
	if (pSetupData->ESURecoveryTime2Byte >= 5 && pSetupData->ESURecoveryTime2Byte <= 6000)
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ANTISKIMMING_AUTORECOVERYTIME, pSetupData->ESURecoveryTime2Byte);
#endif

	// [#RWC6-207] US William 2020.08.05 Add LibertyX enablement to setup data
#if (RMS_VERSION >= 0x010347)
	GUARD_MESSAGE_LEN(Len, pSetupData, LibertyXBuyEnabled)

	LXConfiguration lxConfig;
	CLibertyXConfigurationManager ltxConfigManager;
	if (ltxConfigManager.GetConfiguration(lxConfig))
	{
		lxConfig.BuyBitcoinEnabled = !!pSetupData->LibertyXBuyEnabled;

		ltxConfigManager.SaveConfiguration(lxConfig);
	}
#endif

	// [#RWC6-214] US William 2020.09.01 Add CDU dispense style
#if (RMS_VERSION >= 0x010348)
	GUARD_MESSAGE_LEN(Len, pSetupData, CDUDispenseStyle)

	int dispenseStyle = pSetupData->CDUDispenseStyle;
	if ((dispenseStyle >= 1 && dispenseStyle <= 3) || dispenseStyle == 10)
	{
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DISPENSE_STYLE, dispenseStyle);
	}
#endif

	// [#RWC6-224] US William 2020.10.21 LibertyX Dispense
#if (RMS_VERSION >= 0x010349)
	GUARD_MESSAGE_LEN(Len, pSetupData, LibertyXSellEnabled)

	if (ltxConfigManager.GetConfiguration(lxConfig))
	{
		lxConfig.SellBitcoinEnabled = !!pSetupData->LibertyXSellEnabled;

		ltxConfigManager.SaveConfiguration(lxConfig);
	}
#endif

#if (RMS_VERSION >= 0x010351)
	GUARD_MESSAGE_LEN(Len, pSetupData, CDUBindingActive)

	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCCLOOKUP_LOCALSURCHARGE, !!pSetupData->LocalDCCSurchargeEnabled);
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDU_BINDING, !!pSetupData->CDUBindingEnabled);
#endif

#if (RMS_VERSION >= 0x010353)
	int nSavedSideCarEnabled = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_SUPPORT);
	int nSavedBnaType = m_pDevCmn->GetDeviceType(L"BNA");

	bool bSiderCarEnabled = pSetupData->SidecarEnabled;
	int nAcceptorType = pSetupData->AcceptorType;
	bool bRFIDEnabled = pSetupData->RFIDEnabled;

	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_SUPPORT, !!pSetupData->SidecarEnabled);
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_BNATYPE, pSetupData->AcceptorType);
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE, !!pSetupData->RFIDEnabled);

	if (nSavedSideCarEnabled != MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_SUPPORT))
	{
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_SUPPORT) == ENABLE)
		{
			if (m_pDevCmn->GetDeviceType(L"BNA") == BNATYPE_MEI)
				RegSetStr(L"SOFTWARE\\XFS\\SERVICE_PROVIDERS\\NHCIMCE", L"Exec", L"\\ATM\\NW_CIM_CE.exe");
			else
				RegSetStr(L"SOFTWARE\\XFS\\SERVICE_PROVIDERS\\NHCIMCE", L"Exec", L"\\ATM\\NW_CIM_CE_JCM.exe");

			SIUInit();

			m_pDevCmn->fnCMN_AddActiveDevice(DEV_BCR);
			m_pDevCmn->fnCMN_AddActiveDevice(DEV_BNA);
			m_pDevCmn->fnBCR_Initialize();
			m_pDevCmn->fnBNA_Initialize();
		}
		else
		{
			m_pDevCmn->fnBCR_Deinitialize();
			m_pDevCmn->fnBNA_Deinitialize();
			m_pDevCmn->fnCMN_RemoveActiveDevice(DEV_BCR);
			m_pDevCmn->fnCMN_RemoveActiveDevice(DEV_BNA);

			SIUDeInit();
		}
	}
	else
	{
		// In case "BNAType" is only changed
		if (nSavedBnaType != m_pDevCmn->GetDeviceType(L"BNA"))
		{
			m_pDevCmn->fnBNA_Deinitialize();

			if (m_pDevCmn->GetDeviceType(L"BNA") == BNATYPE_MEI)
				RegSetStr(L"SOFTWARE\\XFS\\SERVICE_PROVIDERS\\NHCIMCE", L"Exec", L"\\ATM\\NW_CIM_CE.exe");
			else
				RegSetStr(L"SOFTWARE\\XFS\\SERVICE_PROVIDERS\\NHCIMCE", L"Exec", L"\\ATM\\NW_CIM_CE_JCM.exe");

			m_pDevCmn->fnBNA_Initialize();
		}
	}

	// Update "saved" values
	nSavedSideCarEnabled = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_SUPPORT);
	nSavedBnaType = m_pDevCmn->GetDeviceType(L"BNA");

	// Revert changes if they're not applied
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_SUPPORT, nSavedSideCarEnabled);
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_BNATYPE, nSavedBnaType);
#endif

	return TRUE;
}

// [#620] US KSK 2010.02.22
BOOL CRmsCtrl::RMS_MakeEnhancedCouponMsg(BYTE *pMsg)
{
	int		nValue, i, j;
	CString	strTemp;
	PRMS_ENHANCED_COUPON	pEnhancedCouponData;
	
	pEnhancedCouponData = (PRMS_ENHANCED_COUPON) pMsg;
	memset(pEnhancedCouponData, 0, sizeof(RMS_ENHANCED_COUPON));
	
	//[R] 1. Enhanced Coupon Use
	nValue = MemGetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON_ENABLE);
	pEnhancedCouponData->EnhancedCouponUse = (NUINT8) nValue;
	
	//[R] 2. Coupon Enable
	for (i = 0, nValue = 0; i < 6; i++)
	{
		if (MemGetInt(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON1_ENDISFLAG+i))
			nValue |= (1 << i);
	}
	pEnhancedCouponData->AdvCouponEnable = (NUINT8)nValue;
	
	//[R] 3 - 8
	for (i = 0; i < 2; i++)
	{
		//[R] 3. coupon1 msg 1,2
		strTemp = MemGetStr(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON1_TEXT1+i);
		if (strTemp.GetLength() > 0)
			RMS_WideToMulti(strTemp, (char*)pEnhancedCouponData->AdvCouponMsg1[i], sizeof(pEnhancedCouponData->AdvCouponMsg1[i]));
		
		//[R] 4. coupon2 msg 1,2
		strTemp = MemGetStr(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON2_TEXT1+i);
		if (strTemp.GetLength() > 0)
			RMS_WideToMulti(strTemp, (char*)pEnhancedCouponData->AdvCouponMsg2[i], sizeof(pEnhancedCouponData->AdvCouponMsg2[i]));
		
		//[R] 5. coupon3 msg 1,2
		strTemp = MemGetStr(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON3_TEXT1+i);
		if (strTemp.GetLength() > 0)
			RMS_WideToMulti(strTemp, (char*)pEnhancedCouponData->AdvCouponMsg3[i], sizeof(pEnhancedCouponData->AdvCouponMsg3[i]));
		
		//[R] 6. coupon4 msg 1,2
		strTemp = MemGetStr(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON4_TEXT1+i);
		if (strTemp.GetLength() > 0)
			RMS_WideToMulti(strTemp, (char*)pEnhancedCouponData->AdvCouponMsg4[i], sizeof(pEnhancedCouponData->AdvCouponMsg4[i]));
		
		//[R] 7. coupon5 msg 1,2
		strTemp = MemGetStr(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON5_TEXT1+i);
		if (strTemp.GetLength() > 0)
			RMS_WideToMulti(strTemp, (char*)pEnhancedCouponData->AdvCouponMsg5[i], sizeof(pEnhancedCouponData->AdvCouponMsg5[i]));
		
		//[R] 8. coupon6 msg 1,2
		strTemp = MemGetStr(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON6_TEXT1+i);
		if (strTemp.GetLength() > 0)
			RMS_WideToMulti(strTemp, (char*)pEnhancedCouponData->AdvCouponMsg6[i], sizeof(pEnhancedCouponData->AdvCouponMsg6[i]));
	}
	
	//[R] 9. Enhanced Coupon Cut Option
	nValue = MemGetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON_CUT_OPTION);
	pEnhancedCouponData->EnhancedCouponCutOption = (NUINT8) nValue;
	
	//[R] 10 ~ 18. Enhanced Coupon Enable
	for(i=0; i<6; i++)
	{
		//[R] Enhanced Coupon Enable
		nValue = MemGetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON1_OPTION + i);
		pEnhancedCouponData->EnhancedCouponOption[i] = (NUINT8) nValue;
		
		nValue = MemGetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON1_PRINT_START_TIME + i);
		pEnhancedCouponData->EnhancedCouponPrintStartTime[i] = (NUINT8) nValue;
		
		nValue = MemGetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON1_PRINT_END_TIME + i);
		pEnhancedCouponData->EnhancedCouponPrintEndTime[i] = (NUINT8) nValue;
		
		for(j=0; j<16; j++)
		{
			strTemp = MemGetStr(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON1_TEXT1+((i*16)+j));
			if (strTemp.GetLength() > 0)
				RMS_WideToMulti(strTemp, (char*)pEnhancedCouponData->EnhancedCouponText[i][j], sizeof(pEnhancedCouponData->EnhancedCouponText[i][j]));
		}
	}

	return TRUE;
}

BOOL CRmsCtrl::RMS_MakeEnhancedCouponReadDataMsg(BYTE *pMsg)
{
	CString		strTemp;
	PRMS_READ_ENHANCED_COUPON	pEnhancedCouponRead;

	pEnhancedCouponRead = (PRMS_READ_ENHANCED_COUPON) pMsg;

	memset(pEnhancedCouponRead, 0, sizeof(RMS_READ_ENHANCED_COUPON));

	// 2. Length
	//pSetupRead->Header.Length = (4 + SIZE_RMS_SETUP_READ_DATA);

	// 5. FrameNumber
	

	// 6. Year-Month-Day, Hour-Miniute-Second
	SYSTEMTIME localTime;
	::GetLocalTime(&localTime);
	pEnhancedCouponRead->dwYear = localTime.wYear;
	pEnhancedCouponRead->dwMonth = localTime.wMonth;
	pEnhancedCouponRead->dwDay = localTime.wDay;
	pEnhancedCouponRead->dwHour = localTime.wHour;
	pEnhancedCouponRead->dwMinute = localTime.wMinute;
	pEnhancedCouponRead->dwSecond = localTime.wSecond;

	// 7. Machine Type
	pEnhancedCouponRead->MachineType = RMS_GetMachineType();

	// 8. Serial Number
	strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ATMSERIALNO);
	//if serial number is default value, ATM sends Terminal ID instead of it
	//Then RMS will identify the terminal ID
	if(strTemp == "0000000000") //default setting value = "0000000000"
		strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID);
	strTemp.TrimLeft();
	strTemp.TrimRight();
	RMS_WideToMulti(strTemp, (char*)pEnhancedCouponRead->SerialNumber, sizeof(pEnhancedCouponRead->SerialNumber));

	CST_STATUS	CSTStatus;
	GetCSTStatus(&CSTStatus);

	// 9. CST1 Denomination
	pEnhancedCouponRead->CST1Denomination = CSTStatus.CST1Denomination;

	// 10. CST1 Bill Count
	pEnhancedCouponRead->CST1BillCount = CSTStatus.CST1BillCount;


	// 11. CST2 Denomination
	pEnhancedCouponRead->CST2Denomination = CSTStatus.CST2Denomination;

	// 12. CST2 Bill Count
	pEnhancedCouponRead->CST2BillCount = CSTStatus.CST2BillCount;


	// 13. CST3 Denomination
	pEnhancedCouponRead->CST3Denomination = CSTStatus.CST3Denomination;

	// 14. CST3 Bill Count
	pEnhancedCouponRead->CST3BillCount = CSTStatus.CST3BillCount;


	// 15. CST4 Denomination
	pEnhancedCouponRead->CST4Denomination = CSTStatus.CST4Denomination;

	// 16. CST4 Bill Count
	pEnhancedCouponRead->CST4BillCount = CSTStatus.CST4BillCount;


	// 17. Journal Count
	if (m_pDevCmn->m_JNLMgr.GetUploadRemainCount(UP_RMS) > 0)
		pEnhancedCouponRead->JournalCount = (NUINT16)m_pDevCmn->m_JNLMgr.GetUploadRemainCount(UP_RMS);
	else
		pEnhancedCouponRead->JournalCount = 0;

	// 18. ATM Status.
	BYTE bATMStatus = (BYTE)MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ATMSTATUS);
	
	if (bATMStatus > ATM_RMSACTIVE) 
		bATMStatus = 0xff;
	else
		bATMStatus +=0x31;

	pEnhancedCouponRead->ATMStatus = bATMStatus;

	// 19. Error Code.
	// [#419] [NH] KSK 2008.9.11
	if (!m_pDevCmn->fnAPL_CheckError())
		sprintf((char*)pEnhancedCouponRead->ErrorCode, "%7.7S", m_pDevCmn->fstrAPL_GetErrorCode());
	else
		memcpy(pEnhancedCouponRead->ErrorCode, "0000000", 7);
	// end of [#419]
	return TRUE;
}

BOOL CRmsCtrl::RMS_WriteEnhancedCouponMsg(BYTE *pMsg, int Len)
{
	int i, j;
	PRMS_ENHANCED_COUPON	pEnhancedCouponData;
	CString	strTemp;
	
	// check buffer size.
	if (LEN_RMS_ENHANCED_COUPON_DATA < Len)		// 2008.07.03 by Cha Bug fix.
		return FALSE;

	pEnhancedCouponData = (PRMS_ENHANCED_COUPON)pMsg;

	//[R] 1. Enhanced Coupon Use
	MemSetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON_ENABLE, (int)pEnhancedCouponData->EnhancedCouponUse);
	
	//[W] 2. Coupon Enable.
	for (i = 0; i < 6; i++)
	{
		if (pEnhancedCouponData->AdvCouponEnable & (1 << i))
			MemSetInt(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON1_ENDISFLAG+i, 1);
		else
			MemSetInt(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON1_ENDISFLAG+i, 0);
	}
	
	//[W] 3 - 8
	for (i = 0; i < 2; i++)
	{
		//[W] 3. coupon1 msg 1,2
		RMS_MultiToWide((char*)pEnhancedCouponData->AdvCouponMsg1[i], (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
		MemSetStr(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON1_TEXT1+i, (LPCTSTR)gwTempBuf);
		
		//[W] 4. coupon2 msg 1,2
		RMS_MultiToWide((char*)pEnhancedCouponData->AdvCouponMsg2[i], (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
		MemSetStr(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON2_TEXT1+i, (LPCTSTR)gwTempBuf);
		
		//[W] 5. coupon3 msg 1,2
		RMS_MultiToWide((char*)pEnhancedCouponData->AdvCouponMsg3[i], (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
		MemSetStr(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON3_TEXT1+i, (LPCTSTR)gwTempBuf);
		
		//[W] 6. coupon4 msg 1,2
		RMS_MultiToWide((char*)pEnhancedCouponData->AdvCouponMsg4[i], (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
		MemSetStr(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON4_TEXT1+i, (LPCTSTR)gwTempBuf);
		
		//[W] 7. coupon5 msg 1,2
		RMS_MultiToWide((char*)pEnhancedCouponData->AdvCouponMsg5[i], (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
		MemSetStr(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON5_TEXT1+i, (LPCTSTR)gwTempBuf);
		
		//[W] 8. coupon msg 1,2
		RMS_MultiToWide((char*)pEnhancedCouponData->AdvCouponMsg6[i], (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
		MemSetStr(_MEM_FLD_ADVINFO, _MEM_VAR_COUPON6_TEXT1+i, (LPCTSTR)gwTempBuf);
	}
	
	//[R] 9. Enhanced Coupon Cut Option
	MemSetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON_CUT_OPTION, (int) pEnhancedCouponData->EnhancedCouponCutOption);
	
	//[R] 10 ~ 18. Enhanced Coupon Enable
	for(i=0; i<6; i++)
	{
		//[R] Enhanced Coupon Enable
		MemSetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON1_OPTION + i, (int) pEnhancedCouponData->EnhancedCouponOption[i]);
		
		if ((pEnhancedCouponData->EnhancedCouponPrintStartTime[i] >= 0 && pEnhancedCouponData->EnhancedCouponPrintStartTime[i] < 24) &&
			(pEnhancedCouponData->EnhancedCouponPrintStartTime[i] != pEnhancedCouponData->EnhancedCouponPrintEndTime[i]))
			MemSetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON1_PRINT_START_TIME + i, (int) pEnhancedCouponData->EnhancedCouponPrintStartTime[i]);
		
		if ((pEnhancedCouponData->EnhancedCouponPrintEndTime[i] >= 0 && pEnhancedCouponData->EnhancedCouponPrintEndTime[i] < 24) &&
			(pEnhancedCouponData->EnhancedCouponPrintStartTime[i] != pEnhancedCouponData->EnhancedCouponPrintEndTime[i]))
			MemSetInt(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON1_PRINT_END_TIME + i, (int) pEnhancedCouponData->EnhancedCouponPrintEndTime[i]);
		
		for(j=0; j<16; j++)
		{
			RMS_MultiToWide((char*)pEnhancedCouponData->EnhancedCouponText[i][j], (LPWSTR)gwTempBuf, TEMP_BUFSIZE);
			MemSetStr(_MEM_FLD_ENHANCED_COUPON, _MEM_VAR_ENHANCED_COUPON1_TEXT1+((i*16)+j), (LPCTSTR)gwTempBuf);
		}
	}

	return TRUE;
}
// end of [#620]

/*-------------------------------------------------------------------
 CLASS    NAME: CRmsCtrl
 FUNCTION NAME: MakeRMSJNLUpload()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : RMS Upload 저널을 만든다.
-------------------------------------------------------------------*/
BOOL CRmsCtrl::MakeRMSJNLUpload(BYTE *pMsg, int MsgLen, CString strJnlData)
{
	PRMS_RES_JNL	pJNLData;
	CStringArray	arJnlField;
	CTime	CurTime = CTime::GetCurrentTime();
	CString	strTemp;

	// check buffer size.
	if (sizeof(RMS_RES_JNL) > MsgLen)
		return FALSE;

	pJNLData = (PRMS_RES_JNL)pMsg;
	
	SplitString(strJnlData,"^", arJnlField);

	// exception
	if (arJnlField.GetSize() < 9)
		return FALSE;

	// 6. Year-Month-Day, Hour-Minute-Second
	pJNLData->dwYear = CurTime.GetYear();
	pJNLData->dwMonth = CurTime.GetMonth();
	pJNLData->dwDay = CurTime.GetDay();
	pJNLData->dwHour = CurTime.GetHour();
	pJNLData->dwMinute = CurTime.GetMinute();
	pJNLData->dwSecond = CurTime.GetSecond();

	// 7. Machine Type
	pJNLData->MachineType = RMS_GetMachineType();

	// 8. Serial Number
	strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ATMSERIALNO);
	if(strTemp == "0000000000") //default setting value = "0000000000"
		strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID);
	strTemp.TrimLeft();
	strTemp.TrimRight();
	RMS_WideToMulti(strTemp, (char*)pJNLData->SerialNumber, sizeof(pJNLData->SerialNumber));

	CST_STATUS	CSTStatus;
	GetCSTStatus(&CSTStatus);

	// 8. CST1 Denomination
	pJNLData->CST1Denomination = CSTStatus.CST1Denomination;

	// 9. CST1 Bill Count
	pJNLData->CST1BillCount = CSTStatus.CST1BillCount;


	// 10. CST2 Denomination
	pJNLData->CST2Denomination = CSTStatus.CST2Denomination;

	// 11. CST2 Bill Count
	pJNLData->CST2BillCount = CSTStatus.CST2BillCount;


	// 12. CST3 Denomination
	pJNLData->CST3Denomination = CSTStatus.CST3Denomination;

	// 13. CST3 Bill Count
	pJNLData->CST3BillCount = CSTStatus.CST3BillCount;


	// 14. CST4 Denomination
	pJNLData->CST4Denomination = CSTStatus.CST4Denomination;

	// 15. CST4 Bill Count
	pJNLData->CST4BillCount = CSTStatus.CST4BillCount;

	// 16. Journal Count
	//pJNLData->JournalCount = (NUINT16)m_pDevCmn->m_JNLMgr.GetUploadRemainCount(UP_RMS);		// [CODESONAR] /* Cast Alters Value (ID: 31) */
	if (m_pDevCmn->m_JNLMgr.GetUploadRemainCount(UP_RMS) > 0)
		pJNLData->JournalCount = (NUINT16)m_pDevCmn->m_JNLMgr.GetUploadRemainCount(UP_RMS);
	else
		pJNLData->JournalCount = 0;

	// 17. ATM Status.
	BYTE bATMStatus = (BYTE)MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ATMSTATUS);
	
	//Validation (ATM_INIT(0) ~ ATM_RMSACTIVE(8) )
	if (bATMStatus > ATM_RMSACTIVE) 
		bATMStatus = 0xff;
	else
		bATMStatus +=0x31;

	pJNLData->ATMStatus = bATMStatus;

	// 18. Error Code.
	// [#419] [NH] KSK 2008.9.11
	if (!m_pDevCmn->fnAPL_CheckError())
		sprintf((char*)pJNLData->ErrorCode, "%7.7S", m_pDevCmn->fstrAPL_GetErrorCode());
	else
		memcpy(pJNLData->ErrorCode, "0000000", 7);
	// end of [#419]
	// 19. JNL Data Size

	// 20. JNL KindCode
	arJnlField[1].TrimLeft();
	arJnlField[1].TrimRight();
	RMS_WideToMulti(arJnlField[1], (char*)pJNLData->JNL_KindCode, sizeof(pJNLData->JNL_KindCode));
	
	// 21. JNL StackNumber
	pJNLData->JNL_StackNum = (NUINT16)Asc2Int(arJnlField[2]);
	
	// 22. JNL Stack Month
	pJNLData->JNL_StackMonth = (NUINT8)Asc2Int(arJnlField[4]);

	// 23. JNL Stack Day
	pJNLData->JNL_StackDay = (NUINT8)Asc2Int(arJnlField[5]);

	// 24. JNL Stack Year
	pJNLData->JNL_StackYear = (NUINT16)Asc2Int(arJnlField[3]);

	// 25. JNL Stack Hour
	pJNLData->JNL_StackHour = (NUINT8)Asc2Int(arJnlField[6]);

	// 26. JNL Stack Minute
	pJNLData->JNL_StackMinute = (NUINT8)Asc2Int(arJnlField[7]);

	// 27. JNL Stack Second
	pJNLData->JNL_StackSecond = (NUINT8)Asc2Int(arJnlField[8]);

	// [#2270] AU KSK 2014.05.26 AP에서 관리하는 매수정보를 표시하도록 추가 수정하여 매수정보 표시하도록 재수정
	// [#2241] AU KMK 2014.02.05 CDU-M 논리매수 차감 비활성화 시 MoniView에 Initial/Remaining Count를 -1 -> 2000매로 변경 후 전달함
	//int nJnlFieldSize = arJnlField.GetSize();
	//if (nJnlFieldSize >= 13)	// 2014.03.03 값이 있는 경우에 한해서만 수행
	//{
	//	if (Asc2Int(arJnlField[12]) < 0)	// Initial Count가 -1이면 논리매수 차감 비활성화 상태
	//	{
	//		arJnlField[12] = "2000";	// CST1 Initial Count
	//		arJnlField[16] = "2000";	// CST1 Remaining Count
	//	}
	//}
	//if (nJnlFieldSize >= 19)	// 2014.03.03 값이 있는 경우에 한해서만 수행
	//{
	//	if (Asc2Int(arJnlField[18]) < 0)
	//	{
	//		arJnlField[18] = "2000";	// CST2 Initial Count
	//		arJnlField[22] = "2000";	// CST2 Remaining Count
	//	}
	//}
	//if (nJnlFieldSize >= 25)	// 2014.03.03 값이 있는 경우에 한해서만 수행
	//{
	//	if (Asc2Int(arJnlField[24]) < 0)
	//	{
	//		arJnlField[24] = "2000";	// CST3 Initial Count
	//		arJnlField[28] = "2000";	// CST3 Remaining Count
	//	}
	//}
	//if (nJnlFieldSize >= 31)	// 2014.03.03 값이 있는 경우에 한해서만 수행
	//{
	//	if (Asc2Int(arJnlField[30]) < 0)
	//	{
	//		arJnlField[30] = "2000";	// CST4 Initial Count
	//		arJnlField[34] = "2000";	// CST4 Remaining Count
	//	}
	//}
	// end of [#2241]
	// end of [#2270]

	//////////////////////////////////
	//[#586] SOOK 2009.12.09 iTM MoniView 연동 부분 추가 
	int nJnlDataLength = 0; 
	// Transaction JNL
	if (arJnlField[1] == NORMAL_TRX || arJnlField[1] == TRX_ERROR || arJnlField[1] == REVERSAL_TRX ||
		arJnlField[1] == TRX_NOT_APPROVED || arJnlField[1] == COMMUNICATION_ERROR || arJnlField[1] == CHANGE_ERROR_MODE)
	{
		nJnlDataLength =  JNLConvertTran((BYTE*)pJNLData->JNL_Data, sizeof(pJNLData->JNL_Data), arJnlField);
	}
	//////////////////////////////////
	// Reversal Failure JNL
	else 
	if (arJnlField[1] == L"RO" || arJnlField[1] == L"RP")
	{
		nJnlDataLength =  JNLConvertReversalFailure((BYTE*)pJNLData->JNL_Data, sizeof(pJNLData->JNL_Data), arJnlField);
	}

	// [#578] NH KSK 2009.11.09
	//////////////////////////////////
	// Add Cash JNL
	else 
	if (arJnlField[1] == ADDCASH_CST1 || arJnlField[1] == ADDCASH_CST2 || arJnlField[1] == ADDCASH_CST1_2 ||
		arJnlField[1] == ADDCASH_CST3 || arJnlField[1] == ADDCASH_CST1_3 || arJnlField[1] == ADDCASH_CST2_3 ||			// [#332] NH AIREAT 2008.06.24
		arJnlField[1] == ADDCASH_CST1_2_3 || arJnlField[1] == ADDCASH_CST4 || arJnlField[1] == ADDCASH_CST1_4 ||
		arJnlField[1] == ADDCASH_CST2_4 || arJnlField[1] == ADDCASH_CST1_2_4 || arJnlField[1] == ADDCASH_CST3_4 ||
		arJnlField[1] == ADDCASH_CST1_3_4 || arJnlField[1] == ADDCASH_CST2_3_4 || arJnlField[1] == ADDCASH_CST1_2_3_4 ||
		arJnlField[1] == REMOTE_ADDCASH_CST1 || arJnlField[1] == REMOTE_ADDCASH_CST2 || arJnlField[1] == REMOTE_ADDCASH_CST1_2 ||
		arJnlField[1] == REMOTE_ADDCASH_CST3 || arJnlField[1] == REMOTE_ADDCASH_CST1_3 || arJnlField[1] == REMOTE_ADDCASH_CST2_3 ||			// [#332] NH AIREAT 2008.06.24
		arJnlField[1] == REMOTE_ADDCASH_CST1_2_3 || arJnlField[1] == REMOTE_ADDCASH_CST4 || arJnlField[1] == REMOTE_ADDCASH_CST1_4 ||
		arJnlField[1] == REMOTE_ADDCASH_CST2_4 || arJnlField[1] == REMOTE_ADDCASH_CST1_2_4 || arJnlField[1] == REMOTE_ADDCASH_CST3_4 ||
		arJnlField[1] == REMOTE_ADDCASH_CST1_3_4 || arJnlField[1] == REMOTE_ADDCASH_CST2_3_4 || arJnlField[1] == REMOTE_ADDCASH_CST1_2_3_4)	// [#530] NH KSK 2009.06.09
	{
		nJnlDataLength =  JNLConvertAddCash((BYTE*)pJNLData->JNL_Data, sizeof(pJNLData->JNL_Data), arJnlField);
	}
	//////////////////////////////////
	// Denomination JNL
	else
	if (arJnlField[1] == CHANGE_DENOM_CST1 || arJnlField[1] == CHANGE_DENOM_CST2 || arJnlField[1] == CHANGE_DENOM_CST1_2 ||
		arJnlField[1] == CHANGE_DENOM_CST3 || arJnlField[1] == CHANGE_DENOM_CST1_3 || arJnlField[1] == CHANGE_DENOM_CST2_3 ||			// [#332] NH AIREAT 2008.06.24
		arJnlField[1] == CHANGE_DENOM_CST1_2_3 || arJnlField[1] == CHANGE_DENOM_CST4 || arJnlField[1] == CHANGE_DENOM_CST1_4 ||
		arJnlField[1] == CHANGE_DENOM_CST2_4 || arJnlField[1] == CHANGE_DENOM_CST1_2_4 || arJnlField[1] == CHANGE_DENOM_CST3_4 ||
		arJnlField[1] == CHANGE_DENOM_CST1_3_4 || arJnlField[1] == CHANGE_DENOM_CST2_3_4 || arJnlField[1] == CHANGE_DENOM_CST1_2_3_4)																	// [#332] NH AIREAT 2008.06.24
	{
		nJnlDataLength =  JNLConvertDenomi((BYTE*)pJNLData->JNL_Data, sizeof(pJNLData->JNL_Data), arJnlField);
	}
	// end of [#578]
	//////////////////////////////////
	// Day Total, Trial Day Total
	else
	if (arJnlField[1] == DAY_TOTAL || arJnlField[1] == TRIALDAY_TOTAL ||
		arJnlField[1] == REMOTE_DAY_TOTAL || arJnlField[1] == REMOTE_TRIALDAY_TOTAL)
	{
		nJnlDataLength =  JNLConvertDayTotal((BYTE*)pJNLData->JNL_Data, sizeof(pJNLData->JNL_Data), arJnlField);
	}
	//////////////////////////////////
	// CST Total, Trial CST Total
	else if (arJnlField[1] == CST_TOTAL || arJnlField[1] == TRIALCST_TOTAL || arJnlField[1] == REMOTE_CST_TOTAL)		// [#531] NH KSK 2009.06.09
	{
		nJnlDataLength =  JNLConvertCassetteTotal((BYTE*)pJNLData->JNL_Data, sizeof(pJNLData->JNL_Data), arJnlField);
	}
	//////////////////////////////////
	// [#2442] US Justin Brief Day Total, Trial Day Total : Pin4, PopMoney
	else if ( arJnlField[1] == PIN4_DAY_TOTAL		|| arJnlField[1] == PIN4_TRIALDAY_TOTAL		||			
			  arJnlField[1] == POPMONEY_DAY_TOTAL	|| arJnlField[1] == POPMONEY_TRIALDAY_TOTAL	||
			  arJnlField[1] == JUSTCASH_DAY_TOTAL	|| arJnlField[1] == JUSTCASH_TRIALDAY_TOTAL	||			// [#2445]
			  arJnlField[1] == PAYPAL_DAY_TOTAL		|| arJnlField[1] == PAYPAL_TRIALDAY_TOTAL	||			// [#2446]
			  arJnlField[1] == DIGITALCUR_DAY_TOTAL	|| arJnlField[1] == DIGITALCUR_TRIALDAY_TOTAL )
	{
		nJnlDataLength =  JNLConvertBriefDayTotal((BYTE*)pJNLData->JNL_Data, sizeof(pJNLData->JNL_Data), arJnlField);
	}
	//////////////////
	// Processor JNL
	else if (arJnlField[1] == CHANGE_PROCESSOR)
	{
		nJnlDataLength =  JNLConvertChangeProcessor((BYTE*)pJNLData->JNL_Data, sizeof(pJNLData->JNL_Data), arJnlField);
	}
	//////////////////////////////////
	// Change Password
	else if (arJnlField[1] == CHANGE_MASTER_PWD || arJnlField[1] == CHANGE_OPERATOR_PWD || arJnlField[1] == CHANGE_SUPERVISOR_PWD)
	{
		nJnlDataLength =  JNLConvertChangePassword((BYTE*)pJNLData->JNL_Data, sizeof(pJNLData->JNL_Data), arJnlField);
	}
	//////////////////////////////////
	// Change Configuration
	else if ( arJnlField[1] == OPERATOR_ACTION	|| arJnlField[1] == MONIVIEW_ACTION || 
			  arJnlField[1] == HOST_ACTION		|| arJnlField[1] == CHANGE_OP_MODE			) //[#610] SOOK 2010.01.20 OP 진입시 Login User 도 올릴 수 있게 수정함. 
	{
		nJnlDataLength =  JNLConvertOperatorAction((BYTE*)pJNLData->JNL_Data, sizeof(pJNLData->JNL_Data), arJnlField);
	}
	//////////////////////////////////
	// CHANGE EXCHANGE RATE
	else if (arJnlField[1] == CHANGE_EXCHANGE_RATE)	// 2009.3.30
	{
		nJnlDataLength =  JNLConvertChangeExchangeRate((BYTE*)pJNLData->JNL_Data, sizeof(pJNLData->JNL_Data), arJnlField);
	}
	//////////////////////////////////
	// USER CANCEL
	else if ( arJnlField[1] == CANCEL_AT_READ_CARD     || arJnlField[1] == CANCEL_AT_SELECT_LANGUAGE || arJnlField[1] == CANCEL_AT_INPUT_PIN    ||
			  arJnlField[1] == CANCEL_AT_SELECT_TRAN   || arJnlField[1] == CANCEL_AT_CONFIRM_FEE     || arJnlField[1] == CANCEL_AT_RECV_RECEIPT ||
			  arJnlField[1] == CANCEL_AT_RECEIPT_ERROR || arJnlField[1] == CANCEL_AT_RECEIPT_SCREEN  || arJnlField[1] == CANCEL_AT_FROM_ACCOUNT ||
			  arJnlField[1] == CANCEL_AT_TO_ACCOUNT    || arJnlField[1] == CANCEL_AT_FASTCASH        || arJnlField[1] == CANCEL_AT_OTHER_AMOUNT ||
			  arJnlField[1] == CANCEL_AT_TRANSFER_AMOUNT)
	{
		nJnlDataLength =  JNLConvertUserCancel((BYTE*)pJNLData->JNL_Data, sizeof(pJNLData->JNL_Data), arJnlField);
	}
	/////////////////////////////////////////
	//CHANGE PARAMETER (CONFIGURATION)
	else if ( arJnlField[1] == CHANGE_PARAMETER)
	{
		pJNLData->Header.Length += JNLConvertChangeParameter((BYTE*)pJNLData->JNL_Data, sizeof(pJNLData->JNL_Data), arJnlField);
	}
	/////////////////////////////////////////
	// [#2279] Justin 2014.06.17 Uploading EMV data to Moniview
	else if ( arJnlField[1] == TRX_EMV_DATA)
	{
		nJnlDataLength = JNLConvertTrxEMVData((BYTE*)pJNLData->JNL_Data, sizeof(pJNLData->JNL_Data), arJnlField);
	}
	/////////////////////////////////////////
	// LibertyX Journal
	else if ( arJnlField[1] == LIBERTYX_TXN || arJnlField[1] == LIBERTYX_TXN_ERROR )
	{
		nJnlDataLength = JNLConvertLibertyXTransactions((BYTE*)pJNLData->JNL_Data, sizeof(pJNLData->JNL_Data), arJnlField);
	}
	else if ( arJnlField[1] == LIBERTYX_TXN_DISPENSE )
	{
		nJnlDataLength = JNLConvertLibertyXDispenseTransactions((BYTE*)pJNLData->JNL_Data, sizeof(pJNLData->JNL_Data), arJnlField);
	}
	/////////////////////////////////////////
	// B4U Journal
	else if ( arJnlField[1] == B4U_TXN || arJnlField[1] == B4U_TXN_ERROR_CANCEL || 
		arJnlField[1] == B4U_TXN_ERROR_FAIL || arJnlField[1] == B4U_TXN_ERROR_TIMEOUT)
	{
		nJnlDataLength = JNLConvertB4UTransactions((BYTE*)pJNLData->JNL_Data, sizeof(pJNLData->JNL_Data), arJnlField);
	}
	/////////////////////////////////////////
	// DigitalMint Journal
	else if (arJnlField[1] == DIGITALMINT_TXN || arJnlField[1] == DIGITALMINT_TXN_ERROR)
	{
		nJnlDataLength = JNLConvertDigitalMintTransactions((BYTE*)pJNLData->JNL_Data, sizeof(pJNLData->JNL_Data), arJnlField);
	}
	else if (arJnlField[1] == TANGOPAY_TXN || arJnlField[1] == TANGOPAY_TXN_ERROR)
	{
		// [RWC6-676] SKKim 2024.05.09 TODO: MoniView와 연동 data 개발 필요
	}
	/////////////////////////////////////////
	// Body가 없는 것들.. 쯧쯧..
	else
	{
	}
	
	// RMS 요청으로 DISABLE 처리.
#if 0
	//[#586] 모든 저널 데이타 끝에 저널 이미지 존재 여부 송신함 
	//우선은 WINCE 6.0에만 적용함 
	BOOL bExistJnlImage = m_pDevCmn->m_JNLMgr.DoesJnlImageExist
		(pJNLData->JNL_StackNum,pJNLData->JNL_StackYear, pJNLData->JNL_StackMonth, pJNLData->JNL_StackDay);

	pJNLData->JNL_Data[nJnlDataLength++] = JNL_IMAGE_DELIMITER;
	pJNLData->JNL_Data[nJnlDataLength++] = ( bExistJnlImage ? 1: 0);
#endif

	pJNLData->Header.Length += nJnlDataLength; 

//end of [#586]

	//////////////////////////////////
	
	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CRmsCtrl
 FUNCTION NAME: JNLConvertTran()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Transaction JNL을 ATM->RMS 형식으로 변경한다.
-------------------------------------------------------------------*/
int  CRmsCtrl::JNLConvertTran(BYTE *pMsg, int Len, CStringArray &arJnlField)
{
	PRMS_JNL_TRAN	pJnlTran = (PRMS_JNL_TRAN)pMsg;
	int	nTempValue;

	if (Len < sizeof(RMS_JNL_TRAN))
		return 0;

	// exception
	if (arJnlField.GetSize() < 35)
		return 0;

	memset(pJnlTran, 0, sizeof(RMS_JNL_TRAN));

	// 1. Terminal Number
	arJnlField[9].TrimLeft();
	arJnlField[9].TrimRight();
	RMS_WideToMulti(arJnlField[9], (char*)pJnlTran->TerminalNumber, sizeof(pJnlTran->TerminalNumber));

	// 2. Sequence Number
	arJnlField[10].TrimLeft();
	arJnlField[10].TrimRight();
	RMS_WideToMulti(arJnlField[10], (char*)pJnlTran->SequenceNumber, sizeof(pJnlTran->SequenceNumber));

	//  3. Transaction Type
	arJnlField[11].TrimLeft();
	arJnlField[11].TrimRight();
	RMS_WideToMulti(arJnlField[11], (char*)pJnlTran->TranType, sizeof(pJnlTran->TranType));

	//  4. From Account
	arJnlField[12].TrimLeft();
	arJnlField[12].TrimRight();
	RMS_WideToMulti(arJnlField[12], (char*)pJnlTran->FromAccount, sizeof(pJnlTran->FromAccount));

	//  5. To Account
	arJnlField[13].TrimLeft();
	arJnlField[13].TrimRight();
	RMS_WideToMulti(arJnlField[13], (char*)pJnlTran->ToAccount, sizeof(pJnlTran->ToAccount));

	//  6. Bank Code
	arJnlField[14].TrimLeft();
	arJnlField[14].TrimRight();
	RMS_WideToMulti(arJnlField[14], (char*)pJnlTran->BankCode, sizeof(pJnlTran->BankCode));

	//  7. Bank Account
	arJnlField[15].TrimLeft();
	arJnlField[15].TrimRight();
	RMS_WideToMulti(arJnlField[15], (char*)pJnlTran->BankAccount, sizeof(pJnlTran->BankAccount));

	arJnlField[16].TrimLeft();
	//  8. Host Date _ Month
	nTempValue = Asc2Int(arJnlField[16].Left(2));
	pJnlTran->HostDate_Month = (NUINT8)nTempValue;

	//  9. Host Date _ Day
	nTempValue = Asc2Int(arJnlField[16].Mid(2,2));
	pJnlTran->HostDate_Day = (NUINT8)nTempValue;

	// 10. Host Date _ Year
	nTempValue = Asc2Int(arJnlField[16].Right(2));
	nTempValue += 2000;
	pJnlTran->HostDate_Year = (NUINT16)nTempValue;

	arJnlField[17].TrimLeft();
	// 11. Host Date _ Hour
	nTempValue = Asc2Int(arJnlField[17].Left(2));
	pJnlTran->HostTime_Hour = (NUINT8)nTempValue;

	// 12. Host Date _ Minute
	nTempValue = Asc2Int(arJnlField[17].Mid(2,2));
	pJnlTran->HostTime_Minute = (NUINT8)nTempValue;

	// 13. Host Date _ Second
	nTempValue = Asc2Int(arJnlField[17].Right(2));
	pJnlTran->HostTime_Second = (NUINT8)nTempValue;

	// 14. Host Avail Balance
	arJnlField[18].TrimLeft();
	arJnlField[18].TrimRight();

	if (arJnlField[18].GetLength() != 0)
	{
		RMS_WideToMulti(arJnlField[18], (char*)pJnlTran->HostAvailBalance, sizeof(pJnlTran->HostAvailBalance));
	}
	else
	{
		// AIREAT 2008.04.29 Host Avail Balance가 공백일경우 공백으로 RMS로 전송.
		strncpy((char*)pJnlTran->HostAvailBalance, "        ", sizeof(pJnlTran->HostAvailBalance));
	}

	// 15. Host Retrieval Number
	arJnlField[19].TrimLeft();
	arJnlField[19].TrimRight();
	RMS_WideToMulti(arJnlField[19], (char*)pJnlTran->HostRetrievalNumber, sizeof(pJnlTran->HostRetrievalNumber));

	// 16.17  Host Audit & Net ID
	arJnlField[20].TrimLeft();
	arJnlField[20].TrimRight();
	RMS_WideToMulti(arJnlField[20], (char*)pJnlTran->HostAuditNetID, sizeof(pJnlTran->HostAuditNetID));

	arJnlField[22].TrimLeft();
	// 18. Host Settle Date _ Month
	nTempValue = Asc2Int(arJnlField[22].Left(2));
	pJnlTran->HostSettleDate_Month = (NUINT8)nTempValue;

	// 19. Host Settle Date _ Day
	nTempValue = Asc2Int(arJnlField[22].Mid(2,2));
	pJnlTran->HostSettleDate_Day = (NUINT8)nTempValue;

	// 20. Host Settle Date _ Year
	nTempValue = Asc2Int(arJnlField[22].Right(2));
	nTempValue += 2000;
	pJnlTran->HostSettleDate_Year = (NUINT16)nTempValue;

	// 21. Surcharge Amount
	arJnlField[23].TrimLeft();
	arJnlField[23].TrimRight();
	RMS_WideToMulti(arJnlField[23], (char*)pJnlTran->Surcharge, sizeof(pJnlTran->Surcharge));

	// 22. Request Amount
	arJnlField[24].TrimLeft();
	arJnlField[24].TrimRight();
	RMS_WideToMulti(arJnlField[24], (char*)pJnlTran->RequestAmount, sizeof(pJnlTran->RequestAmount));

	// 23. Dispense Amount
	arJnlField[25].TrimLeft();
	arJnlField[25].TrimRight();
	RMS_WideToMulti(arJnlField[25], (char*)pJnlTran->DispensedAmount, sizeof(pJnlTran->DispensedAmount));

	// 24. Remain Amount
	// ATM 12자리  RMS는 10자리. --> Pack을 해서 올림.
	{
		char	szRemainAmount[19];
		CString strRemainAmount;
		arJnlField[26].TrimLeft();
		arJnlField[26].TrimRight();

		if (arJnlField[26].GetLength() != 0)
		{
			// [#502] [NH] 2009.2.16 KSK 2009.2.16
			// Ledger Balance값이 9자리 이상인 경우 Int값을 넘어가므로 Convert 필요함
			// '-' 일 경우. e-00001234565, e00000123456
// 			int	nRemainAmount = Asc2Int(arJnlField[26]);
// 			if (nRemainAmount >= 0)
// 				strRemainAmount.Format(L"%018d", nRemainAmount);
// 			else
// 				strRemainAmount.Format(L"%016d", abs(nRemainAmount));
// 			
// 			memset(szRemainAmount, 0, sizeof(szRemainAmount));
// 
// 			RMS_WideToMulti(strRemainAmount, (char*)szRemainAmount, sizeof(szRemainAmount));
// 			
// 			pJnlTran->RemainAmount[0] = (NUINT8)0x65;
// 			if (nRemainAmount >= 0)
// 				MakePack(szRemainAmount, &pJnlTran->RemainAmount[1], 18);
// 			else
// 			{
// 				pJnlTran->RemainAmount[1] = (NUINT8)'-';
// 				MakePack(szRemainAmount, &pJnlTran->RemainAmount[2], 16);
// 			}

 			memset(szRemainAmount, 0, sizeof(szRemainAmount));
 			RMS_WideToMulti(arJnlField[26], (char*)szRemainAmount, sizeof(szRemainAmount));

			__int64 nRemainAmount = _atoi64(szRemainAmount);

 			if (nRemainAmount >= 0)
 				strRemainAmount.Format(L"%018I64d", nRemainAmount);
 			else
 				strRemainAmount.Format(L"%016I64d", nRemainAmount * (-1));

			memset(szRemainAmount, 0, sizeof(szRemainAmount));
 			RMS_WideToMulti(strRemainAmount, (char*)szRemainAmount, sizeof(szRemainAmount));

			pJnlTran->RemainAmount[0] = (NUINT8)0x65;
			if (nRemainAmount >= 0)
				MakePack(szRemainAmount, &pJnlTran->RemainAmount[1], 18);
			else
			{
				pJnlTran->RemainAmount[1] = (NUINT8)'-';
				MakePack(szRemainAmount, &pJnlTran->RemainAmount[2], 16);
 			}
			// end of [#502]
		}
		else
		{
			// AIREAT 2008.04.25 Remain Amount가 공백일경우 공백으로 RMS로 전송.
			strncpy((char*)pJnlTran->RemainAmount, "          ", sizeof(pJnlTran->RemainAmount));
		}	
	}

	// 25. Proc Count
	nTempValue = Asc2Int(arJnlField[27]);
	pJnlTran->ProcCount = (NUINT8)nTempValue;

	// 26. Transaction Result
	RMS_WideToMulti(arJnlField[28], (char*)pJnlTran->Result, sizeof(pJnlTran->Result));

	// 27. Error Code
	RMS_WideToMulti(arJnlField[29], (char*)pJnlTran->ErrorCode, sizeof(pJnlTran->ErrorCode));

	// 28. Card Data
	RMS_WideToMulti(arJnlField[30], (char*)pJnlTran->CardData, sizeof(pJnlTran->CardData));

	// 29. Non-Cash Value
	RMS_WideToMulti(arJnlField[31], (char*)pJnlTran->NonCashValue, sizeof(pJnlTran->NonCashValue));

	// 30. Non-Cash Type
	RMS_WideToMulti(arJnlField[32], (char*)pJnlTran->NonCashType, sizeof(pJnlTran->NonCashType));

	CString strOtherTotalMsg, strOtherRMSMsg,strOtheError;
	strOtherTotalMsg = strOtherRMSMsg = strOtheError = L"";

	if (arJnlField[33].GetLength())	
	{
		CStringArray	strOtherMsgArray;
		SplitString(arJnlField[33], UNIT_DELIMITER, strOtherMsgArray);

		for(int i=0; i<strOtherMsgArray.GetSize(); i++)
		{
			if (strOtherMsgArray[i].GetLength())
			{
				if (strOtherMsgArray[i].Left(1) == RMS_OTHERMSG_DELIMITER)
				{
					// Not Validating values here
					// These values was stored in JnlMgr.cpp depends on Country code.
					if( strOtherMsgArray[i].GetLength() > 2)
						strOtherRMSMsg += strOtherMsgArray[i];
				}
				else if(strOtherMsgArray[i].Left(1)==CDU_OVER_DISPENSE )	// 2 Units (Process Counter + Error Code)
				{
					if(strOtherMsgArray.GetSize() >= i+2) 
					{
						strOtheError = strOtherMsgArray[i+1];
						i++;	
					}
				}
				else
				{
					if (strOtherMsgArray.GetSize() >= i+2)					// 2 Units (Process Counter + Error Code)
					{
						strOtheError.Format(L"Error 2  Code		: %s", strOtherMsgArray[i+1]);		
						i++;
					}
				}
			}
		}
	}
	strOtherTotalMsg = strOtheError + strOtherRMSMsg;
	RMS_WideToMulti(strOtherTotalMsg, (char*)pJnlTran->OtherMessage, sizeof(pJnlTran->OtherMessage));
	// End of [#2460]

	return sizeof(RMS_JNL_TRAN);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CRmsCtrl
 FUNCTION NAME: JNLConvertAddCash()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Reversal Failure JNL을 ATM->RMS 형식으로 변경한다.
-------------------------------------------------------------------*/
/////////////////////////////// LOOKS LIKE NOT USING THIS ROUTINE => JUSTIN 2014.10.07
int CRmsCtrl::JNLConvertReversalFailure(BYTE *pMsg, int Len, CStringArray &arJnlField)
{
	PRMS_JNL_REVER_NG		pJnlReversalNG = (PRMS_JNL_REVER_NG)pMsg;
	int	nTempValue;

	if (Len < sizeof(RMS_JNL_REVER_NG))
		return 0;

	// exception
	if (arJnlField.GetSize() < 35)
		return 0;

	memset(pJnlReversalNG, 0, sizeof(RMS_JNL_REVER_NG));

	// 1. Terminal Number
	arJnlField[9].TrimLeft();
	arJnlField[9].TrimRight();
	RMS_WideToMulti(arJnlField[9], (char*)pJnlReversalNG->TerminalNumber, sizeof(pJnlReversalNG->TerminalNumber));

	// 2. Sequence Number
	arJnlField[10].TrimLeft();
	arJnlField[10].TrimRight();
	RMS_WideToMulti(arJnlField[10], (char*)pJnlReversalNG->SequenceNumber, sizeof(pJnlReversalNG->SequenceNumber));

	arJnlField[16].TrimLeft();
	//  3. Host Date _ Month
	nTempValue = Asc2Int(arJnlField[16].Left(2));
	pJnlReversalNG->TranDate_Month = (NUINT8)nTempValue;

	//  4. Host Date _ Day
	nTempValue = Asc2Int(arJnlField[16].Mid(2,2));
	pJnlReversalNG->TranDate_Day = (NUINT8)nTempValue;

	//  5. Host Date _ Year
	nTempValue = Asc2Int(arJnlField[16].Right(2));
	nTempValue += 2000;
	pJnlReversalNG->TranDate_Year = (NUINT16)nTempValue;

	arJnlField[17].TrimLeft();
	//  6. Host Date _ Hour
	nTempValue = Asc2Int(arJnlField[17].Left(2));
	pJnlReversalNG->TranTime_Hour = (NUINT8)nTempValue;

	//  7. Host Date _ Minute
	nTempValue = Asc2Int(arJnlField[17].Mid(2,2));
	pJnlReversalNG->TranTime_Minute = (NUINT8)nTempValue;

	//  8. Host Date _ Second
	nTempValue = Asc2Int(arJnlField[17].Right(2));
	pJnlReversalNG->TranTime_Second = (NUINT8)nTempValue;

	//  9. Host Retrieval Number
	arJnlField[19].TrimLeft();
	arJnlField[19].TrimRight();
	RMS_WideToMulti(arJnlField[19], (char*)pJnlReversalNG->RetrievalNumber, sizeof(pJnlReversalNG->RetrievalNumber));

	// 10. Request Amount
	arJnlField[24].TrimLeft();
	arJnlField[24].TrimRight();
	RMS_WideToMulti(arJnlField[24], (char*)pJnlReversalNG->RequestAmount, sizeof(pJnlReversalNG->RequestAmount));

	// 11. Dispense Amount
	arJnlField[25].TrimLeft();
	arJnlField[25].TrimRight();
	RMS_WideToMulti(arJnlField[25], (char*)pJnlReversalNG->DispensedAmount, sizeof(pJnlReversalNG->DispensedAmount));

	// 12. Surcharge Amount
	arJnlField[23].TrimLeft();
	arJnlField[23].TrimRight();
	RMS_WideToMulti(arJnlField[23], (char*)pJnlReversalNG->SurchargeAmount, sizeof(pJnlReversalNG->SurchargeAmount));

	// 13. Surcharge Flag.
	arJnlField[34].TrimLeft();
	arJnlField[34].TrimRight();
	nTempValue = Asc2Int(arJnlField[34]);

	// 14. Card Track2 Data
	{
		CString strTrack2;

		arJnlField[30].TrimLeft();
		arJnlField[30].TrimRight();

		strTrack2 = CString('*', 21);
		strTrack2 += arJnlField[30];

		RMS_WideToMulti(strTrack2, (char*)pJnlReversalNG->CardData, sizeof(pJnlReversalNG->CardData));
	}

	// 15. OtherMessage
	RMS_WideToMulti(arJnlField[33], (char*)pJnlReversalNG->OtherMessage, sizeof(pJnlReversalNG->OtherMessage));
	
	return sizeof(RMS_JNL_REVER_NG);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CRmsCtrl
 FUNCTION NAME: JNLConvertAddCash()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : AddCash JNL을 ATM->RMS 형식으로 변경한다.
-------------------------------------------------------------------*/
int  CRmsCtrl::JNLConvertAddCash(BYTE *pMsg, int Len, CStringArray &arJnlField)
{
	PRMS_JNL_ADDCASH	pJnlAddCash = (PRMS_JNL_ADDCASH)pMsg;
	int	nTempValue;
	int	nJnlField;

	if (Len < sizeof(RMS_JNL_ADDCASH))
		return 0;

	memset(pJnlAddCash, 0, sizeof(RMS_JNL_ADDCASH));

	// [#332] NH AIREAT 2008.06.24

	nJnlField = arJnlField.GetSize();

	if (nJnlField >= 11)
	{
		nTempValue = Asc2Int(arJnlField[9]);
		pJnlAddCash->CST1_Remain = (NUINT16)nTempValue;

		nTempValue = Asc2Int(arJnlField[10]);
		pJnlAddCash->CST1_Additon = (NUINT16)nTempValue;
	}

	if (nJnlField >= 13)
	{
		nTempValue = Asc2Int(arJnlField[11]);
		pJnlAddCash->CST2_Remain = (NUINT16)nTempValue;

		nTempValue = Asc2Int(arJnlField[12]);
		pJnlAddCash->CST2_Additon = (NUINT16)nTempValue;
	}

	if (nJnlField >= 15)
	{
		nTempValue = Asc2Int(arJnlField[13]);
		pJnlAddCash->CST3_Remain = (NUINT16)nTempValue;

		nTempValue = Asc2Int(arJnlField[14]);
		pJnlAddCash->CST3_Additon = (NUINT16)nTempValue;
	}

	if (nJnlField >= 17)
	{
		nTempValue = Asc2Int(arJnlField[15]);
		pJnlAddCash->CST4_Remain = (NUINT16)nTempValue;

		nTempValue = Asc2Int(arJnlField[16]);
		pJnlAddCash->CST4_Additon = (NUINT16)nTempValue;
	}

	// end of [#332]

	return sizeof(RMS_JNL_ADDCASH);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CRmsCtrl
 FUNCTION NAME: JNLConvertDenomi()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Denomination JNL을 ATM->RMS 형식으로 변경한다.
-------------------------------------------------------------------*/
int  CRmsCtrl::JNLConvertDenomi(BYTE *pMsg, int Len, CStringArray &arJnlField)
{
	PRMS_JNL_DENO	pJnlDeno = (PRMS_JNL_DENO)pMsg;
	int	nTempValue;
	int	nJnlField;

	if (Len < sizeof(RMS_JNL_DENO))
		return 0;

	memset(pJnlDeno, 0, sizeof(RMS_JNL_DENO));

	// [#332] NH AIREAT 2008.06.24

	nJnlField = arJnlField.GetSize();

	if (nJnlField >= 11)
	{
		nTempValue = Asc2Int(arJnlField[9]);
		pJnlDeno->CST1_Old = (NUINT16)nTempValue;

		nTempValue = Asc2Int(arJnlField[10]);
		pJnlDeno->CST1_New = (NUINT16)nTempValue;
	}

	if (nJnlField >= 13)
	{
		nTempValue = Asc2Int(arJnlField[11]);
		pJnlDeno->CST2_Old = (NUINT16)nTempValue;

		nTempValue = Asc2Int(arJnlField[12]);
		pJnlDeno->CST2_New = (NUINT16)nTempValue;
	}

	if (nJnlField >= 15)
	{
		nTempValue = Asc2Int(arJnlField[13]);
		pJnlDeno->CST3_Old = (NUINT16)nTempValue;

		nTempValue = Asc2Int(arJnlField[14]);
		pJnlDeno->CST3_New = (NUINT16)nTempValue;
	}

	if (nJnlField >= 17)
	{
		nTempValue = Asc2Int(arJnlField[15]);
		pJnlDeno->CST4_Old = (NUINT16)nTempValue;

		nTempValue = Asc2Int(arJnlField[16]);
		pJnlDeno->CST4_New = (NUINT16)nTempValue;
	}

	// end of [#332]

	return sizeof(RMS_JNL_DENO);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CRmsCtrl
 FUNCTION NAME: JNLConvertDaytotal()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Day Total JNL을 ATM->RMS 형식으로 변경한다.
-------------------------------------------------------------------*/
int  CRmsCtrl::JNLConvertDayTotal(BYTE *pMsg, int Len, CStringArray &arJnlField)
{
	PRMS_JNL_DAYTOTAL	pJnlDayTotal = (PRMS_JNL_DAYTOTAL)pMsg;
	int	nTempValue;

	if (Len < sizeof(RMS_JNL_DAYTOTAL))
		return 0;

	// exception
	if (arJnlField.GetSize() < 29)
		return 0;

	memset(pJnlDayTotal, 0, sizeof(RMS_JNL_DAYTOTAL));

	//  1. Logging Start Date
	nTempValue = Asc2Int(arJnlField[9].Left(2));
	pJnlDayTotal->StartDate_Month = (NUINT16)nTempValue;

	nTempValue = Asc2Int(arJnlField[9].Mid(3,2));
	pJnlDayTotal->StartDate_Day = (NUINT16)nTempValue;

	nTempValue = Asc2Int(arJnlField[9].Mid(6,4));
	pJnlDayTotal->StartDate_Year = (NUINT16)nTempValue;

	nTempValue = Asc2Int(arJnlField[9].Mid(11,2));
	pJnlDayTotal->StartDate_Hour = (NUINT16)nTempValue;

	nTempValue = Asc2Int(arJnlField[9].Mid(14,2));
	pJnlDayTotal->StartDate_Minute = (NUINT16)nTempValue;

	nTempValue = Asc2Int(arJnlField[9].Mid(17,2));
	pJnlDayTotal->StartDate_Second = (NUINT16)nTempValue;

	//  2. Logging End Date
	nTempValue = Asc2Int(arJnlField[10].Left(2));
	pJnlDayTotal->EndDate_Month = (NUINT16)nTempValue;

	nTempValue = Asc2Int(arJnlField[10].Mid(3,2));
	pJnlDayTotal->EndDate_Day = (NUINT16)nTempValue;

	nTempValue = Asc2Int(arJnlField[10].Mid(6,4));
	pJnlDayTotal->EndDate_Year = (NUINT16)nTempValue;

	nTempValue = Asc2Int(arJnlField[10].Mid(11,2));
	pJnlDayTotal->EndDate_Hour = (NUINT16)nTempValue;

	nTempValue = Asc2Int(arJnlField[10].Mid(14,2));
	pJnlDayTotal->EndDate_Minute = (NUINT16)nTempValue;

	nTempValue = Asc2Int(arJnlField[10].Mid(17,2));
	pJnlDayTotal->EndDate_Second = (NUINT16)nTempValue;

	//  3. Terminal Number
	arJnlField[11].TrimLeft();
	arJnlField[11].TrimRight();
	RMS_WideToMulti(arJnlField[11], (char*)pJnlDayTotal->TerminalNumber, sizeof(pJnlDayTotal->TerminalNumber));

	//  4. Result
	RMS_WideToMulti(L"SUCCESS", (char*)pJnlDayTotal->Result, sizeof(pJnlDayTotal->Result));

	//  5. Count of Withdrawals (HOST)
	arJnlField[12].TrimLeft();
	arJnlField[12].TrimRight();
	RMS_WideToMulti(arJnlField[12], (char*)pJnlDayTotal->Host_Count_Withdrawals, sizeof(pJnlDayTotal->Host_Count_Withdrawals));

	//  6. Count of Balance (HOST)
	arJnlField[13].TrimLeft();
	arJnlField[13].TrimRight();
	RMS_WideToMulti(arJnlField[13], (char*)pJnlDayTotal->Host_Count_Balance, sizeof(pJnlDayTotal->Host_Count_Balance));

	//  7. Count of Transfer (HOST)
	arJnlField[14].TrimLeft();
	arJnlField[14].TrimRight();
	RMS_WideToMulti(arJnlField[14], (char*)pJnlDayTotal->Host_Count_Transfer, sizeof(pJnlDayTotal->Host_Count_Transfer));

	//  8. Count of Non-Cash (HOST)
	arJnlField[15].TrimLeft();
	arJnlField[15].TrimRight();
	RMS_WideToMulti(arJnlField[15], (char*)pJnlDayTotal->Host_Count_NonCash, sizeof(pJnlDayTotal->Host_Count_NonCash));

	//  9. Amount of Dispense (HOST)
	arJnlField[16].TrimLeft();
	arJnlField[16].TrimRight();
	RMS_WideToMulti(arJnlField[16], (char*)pJnlDayTotal->Host_Amt_Dispense, sizeof(pJnlDayTotal->Host_Amt_Dispense));

	// 10. Amount of Non-Cash (HOST)
	arJnlField[17].TrimLeft();
	arJnlField[17].TrimRight();
	RMS_WideToMulti(arJnlField[17], (char*)pJnlDayTotal->Host_Amt_NonCash, sizeof(pJnlDayTotal->Host_Amt_NonCash));

	// 11. Amount of Surcharge (HOST)
	arJnlField[18].TrimLeft();
	arJnlField[18].TrimRight();
	RMS_WideToMulti(arJnlField[18], (char*)pJnlDayTotal->Host_Amt_Surcharge, sizeof(pJnlDayTotal->Host_Amt_Surcharge));

	// 12. Count of Withdrawals (ATM)
	arJnlField[19].TrimLeft();
	arJnlField[19].TrimRight();
	RMS_WideToMulti(arJnlField[19], (char*)pJnlDayTotal->Atm_Count_Withdrawals, sizeof(pJnlDayTotal->Atm_Count_Withdrawals));

	// 13. Count of Balance (ATM)
	arJnlField[20].TrimLeft();
	arJnlField[20].TrimRight();
	RMS_WideToMulti(arJnlField[20], (char*)pJnlDayTotal->Atm_Count_Balance, sizeof(pJnlDayTotal->Atm_Count_Balance));

	// 14. Count of Transfer (ATM)
	arJnlField[21].TrimLeft();
	arJnlField[21].TrimRight();
	RMS_WideToMulti(arJnlField[21], (char*)pJnlDayTotal->Atm_Count_Transfer, sizeof(pJnlDayTotal->Atm_Count_Transfer));

	// 15. Count of Dispensed (ATM)
	arJnlField[22].TrimLeft();
	arJnlField[22].TrimRight();
	RMS_WideToMulti(arJnlField[22], (char*)pJnlDayTotal->Atm_Count_Dispensed, sizeof(pJnlDayTotal->Atm_Count_Dispensed));

	// 16. Count of NonCash (ATM)
	arJnlField[23].TrimLeft();
	arJnlField[23].TrimRight();
	RMS_WideToMulti(arJnlField[23], (char*)pJnlDayTotal->Atm_Count_NonCash, sizeof(pJnlDayTotal->Atm_Count_NonCash));

	// 17. Count of Reversal (ATM)
	arJnlField[24].TrimLeft();
	arJnlField[24].TrimRight();
	RMS_WideToMulti(arJnlField[24], (char*)pJnlDayTotal->Atm_Count_Reversal, sizeof(pJnlDayTotal->Atm_Count_Reversal));

	// 18. Amount of Withdrawals (ATM)
	arJnlField[25].TrimLeft();
	arJnlField[25].TrimRight();
	RMS_WideToMulti(arJnlField[25], (char*)pJnlDayTotal->Atm_Amt_Withdrawals, sizeof(pJnlDayTotal->Atm_Amt_Withdrawals));

	// 19. Amount of NonCash (ATM)
	arJnlField[26].TrimLeft();
	arJnlField[26].TrimRight();
	RMS_WideToMulti(arJnlField[26], (char*)pJnlDayTotal->Atm_Amt_NonCash, sizeof(pJnlDayTotal->Atm_Amt_NonCash));

	// 20. Amount of Surcharge (ATM)
	arJnlField[27].TrimLeft();
	arJnlField[27].TrimRight();
	RMS_WideToMulti(arJnlField[27], (char*)pJnlDayTotal->Atm_Amt_Surcharge, sizeof(pJnlDayTotal->Atm_Amt_Surcharge));

	// 21. Amount of Reversal (ATM)
	arJnlField[28].TrimLeft();
	arJnlField[28].TrimRight();
	RMS_WideToMulti(arJnlField[28], (char*)pJnlDayTotal->Atm_Amt_Reversal, sizeof(pJnlDayTotal->Atm_Amt_Reversal));

	// 22. Amount of Transfer (ATM)
	RMS_WideToMulti(L"0", (char*)pJnlDayTotal->Atm_Amt_Transfer, sizeof(pJnlDayTotal->Atm_Amt_Transfer));

	return sizeof(RMS_JNL_DAYTOTAL);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CRmsCtrl
 FUNCTION NAME: JNLConvertCassetteTotal()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Cassette Total JNL을 ATM->RMS 형식으로 변경한다.
-------------------------------------------------------------------*/
int CRmsCtrl::JNLConvertCassetteTotal(BYTE *pMsg, int Len, CStringArray &arJnlField)
{
	PRMS_JNL_CSTTOTAL	pJnlCSTTotal = (PRMS_JNL_CSTTOTAL)pMsg;
	int	nTempValue;

	if (Len < sizeof(RMS_JNL_CSTTOTAL))
		return 0;

	// exception
	if (arJnlField.GetSize() < 29)
		return 0;

	memset(pJnlCSTTotal, 0, sizeof(RMS_JNL_CSTTOTAL));

	//  1. Logging Start Date
	nTempValue = Asc2Int(arJnlField[9].Left(2));
	pJnlCSTTotal->StartDate_Month = (NUINT16)nTempValue;

	nTempValue = Asc2Int(arJnlField[9].Mid(3,2));
	pJnlCSTTotal->StartDate_Day = (NUINT16)nTempValue;

	nTempValue = Asc2Int(arJnlField[9].Mid(6,4));
	pJnlCSTTotal->StartDate_Year = (NUINT16)nTempValue;

	nTempValue = Asc2Int(arJnlField[9].Mid(11,2));
	pJnlCSTTotal->StartDate_Hour = (NUINT16)nTempValue;

	nTempValue = Asc2Int(arJnlField[9].Mid(14,2));
	pJnlCSTTotal->StartDate_Minute = (NUINT16)nTempValue;

	nTempValue = Asc2Int(arJnlField[9].Mid(17,2));
	pJnlCSTTotal->StartDate_Second = (NUINT16)nTempValue;

	//  2. Logging End Date
	nTempValue = Asc2Int(arJnlField[10].Left(2));
	pJnlCSTTotal->EndDate_Month = (NUINT16)nTempValue;

	nTempValue = Asc2Int(arJnlField[10].Mid(3,2));
	pJnlCSTTotal->EndDate_Day = (NUINT16)nTempValue;

	nTempValue = Asc2Int(arJnlField[10].Mid(6,4));
	pJnlCSTTotal->EndDate_Year = (NUINT16)nTempValue;

	nTempValue = Asc2Int(arJnlField[10].Mid(11,2));
	pJnlCSTTotal->EndDate_Hour = (NUINT16)nTempValue;

	nTempValue = Asc2Int(arJnlField[10].Mid(14,2));
	pJnlCSTTotal->EndDate_Minute = (NUINT16)nTempValue;

	nTempValue = Asc2Int(arJnlField[10].Mid(17,2));
	pJnlCSTTotal->EndDate_Second = (NUINT16)nTempValue;

	//  3. Terminal Number
	arJnlField[11].TrimLeft();
	arJnlField[11].TrimRight();
	RMS_WideToMulti(arJnlField[11], (char*)pJnlCSTTotal->TerminalNumber, sizeof(pJnlCSTTotal->TerminalNumber));

	//  4. CST1 Initial Notes
	nTempValue = Asc2Int(arJnlField[12]);
	pJnlCSTTotal->CST1_Initial = (NUINT16)nTempValue;

	//  5. CST1 Dispensed Notes
	nTempValue = Asc2Int(arJnlField[13]);
	pJnlCSTTotal->CST1_Dispensed = (NUINT16)nTempValue;

	//  6. CST1 Rejected Notes
	nTempValue = Asc2Int(arJnlField[14]);
	pJnlCSTTotal->CST1_Rejected = (NUINT16)nTempValue;

	//  7. CST1 Jam Notes
	nTempValue = Asc2Int(arJnlField[15]);
	pJnlCSTTotal->CST1_Jam = (NUINT16)nTempValue;

	//  8. CST1 Remaining Notes
	nTempValue = Asc2Int(arJnlField[16]);
	pJnlCSTTotal->CST1_Remain = (NUINT16)nTempValue;

	//  9. CST1 Denomination
	nTempValue = Asc2Int(arJnlField[17]);
	pJnlCSTTotal->CST1_Denomi = (NUINT16)nTempValue;


	// 10. CST2 Initial Notes
	nTempValue = Asc2Int(arJnlField[18]);
	pJnlCSTTotal->CST2_Initial = (NUINT16)nTempValue;

	// 11. CST2 Dispensed Notes
	nTempValue = Asc2Int(arJnlField[19]);
	pJnlCSTTotal->CST2_Dispensed = (NUINT16)nTempValue;

	// 12. CST2 Rejected Notes
	nTempValue = Asc2Int(arJnlField[20]);
	pJnlCSTTotal->CST2_Rejected = (NUINT16)nTempValue;

	// 13. CST2 Jam Notes
	nTempValue = Asc2Int(arJnlField[21]);
	pJnlCSTTotal->CST2_Jam = (NUINT16)nTempValue;

	// 14. CST2 Remaining Notes
	nTempValue = Asc2Int(arJnlField[22]);
	pJnlCSTTotal->CST2_Remain = (NUINT16)nTempValue;

	// 15. CST2 Denomination
	nTempValue = Asc2Int(arJnlField[23]);
	pJnlCSTTotal->CST2_Denomi = (NUINT16)nTempValue;


	// 16. CST3 Initial Notes
	nTempValue = Asc2Int(arJnlField[24]);
	pJnlCSTTotal->CST3_Initial = (NUINT16)nTempValue;

	// 17. CST3 Dispensed Notes
	nTempValue = Asc2Int(arJnlField[25]);
	pJnlCSTTotal->CST3_Dispensed = (NUINT16)nTempValue;

	// 18. CST3 Rejected Notes
	nTempValue = Asc2Int(arJnlField[26]);
	pJnlCSTTotal->CST3_Rejected = (NUINT16)nTempValue;

	// 19. CST3 Jam Notes
	nTempValue = Asc2Int(arJnlField[27]);
	pJnlCSTTotal->CST3_Jam = (NUINT16)nTempValue;

	// 20. CST3 Remaining Notes
	nTempValue = Asc2Int(arJnlField[28]);
	pJnlCSTTotal->CST3_Remain = (NUINT16)nTempValue;

	// 21. CST3 Denomination
	nTempValue = Asc2Int(arJnlField[29]);
	pJnlCSTTotal->CST3_Denomi = (NUINT16)nTempValue;


	// 22. CST4 Initial Notes
	nTempValue = Asc2Int(arJnlField[30]);
	pJnlCSTTotal->CST4_Initial = (NUINT16)nTempValue;

	// 23. CST4 Dispensed Notes
	nTempValue = Asc2Int(arJnlField[31]);
	pJnlCSTTotal->CST4_Dispensed = (NUINT16)nTempValue;

	// 24. CST4 Rejected Notes
	nTempValue = Asc2Int(arJnlField[32]);
	pJnlCSTTotal->CST4_Rejected = (NUINT16)nTempValue;

	// 25. CST4 Jam Notes
	nTempValue = Asc2Int(arJnlField[33]);
	pJnlCSTTotal->CST4_Jam = (NUINT16)nTempValue;

	// 26. CST4 Remaining Notes
	nTempValue = Asc2Int(arJnlField[34]);
	pJnlCSTTotal->CST4_Remain = (NUINT16)nTempValue;

	// 27. CST4 Denomination
	nTempValue = Asc2Int(arJnlField[35]);
	pJnlCSTTotal->CST4_Denomi = (NUINT16)nTempValue;

	// 28. Non-Cash Value
	nTempValue = Asc2Int(arJnlField[36]);
	pJnlCSTTotal->NonCashValue = (NUINT16)nTempValue;

	// 29. Non-Cash Type
	nTempValue = Asc2Int(arJnlField[37]);
	pJnlCSTTotal->NonCashType = (NUINT8)nTempValue;

	return sizeof(RMS_JNL_CSTTOTAL);
}

int  CRmsCtrl::JNLConvertBriefDayTotal(BYTE *pMsg, int Len, CStringArray &arJnlField)
{
	PRMS_JNL_BRIEF_DAYTOTAL	pJnlDayTotal = (PRMS_JNL_BRIEF_DAYTOTAL)pMsg;
	int	nTempValue;

	if (Len < sizeof(RMS_JNL_BRIEF_DAYTOTAL))
		return 0;

	// exception
	if (arJnlField.GetSize() < 14)
		return 0;

	memset(pJnlDayTotal, 0, sizeof(RMS_JNL_BRIEF_DAYTOTAL));

	//  1. Logging Start Date
	nTempValue = Asc2Int(arJnlField[9].Left(2));
	pJnlDayTotal->StartDate_Month = (NUINT16)nTempValue;

	nTempValue = Asc2Int(arJnlField[9].Mid(3,2));
	pJnlDayTotal->StartDate_Day = (NUINT16)nTempValue;

	nTempValue = Asc2Int(arJnlField[9].Mid(6,4));
	pJnlDayTotal->StartDate_Year = (NUINT16)nTempValue;

	nTempValue = Asc2Int(arJnlField[9].Mid(11,2));
	pJnlDayTotal->StartDate_Hour = (NUINT16)nTempValue;

	nTempValue = Asc2Int(arJnlField[9].Mid(14,2));
	pJnlDayTotal->StartDate_Minute = (NUINT16)nTempValue;

	nTempValue = Asc2Int(arJnlField[9].Mid(17,2));
	pJnlDayTotal->StartDate_Second = (NUINT16)nTempValue;

	//  2. Logging End Date
	nTempValue = Asc2Int(arJnlField[10].Left(2));
	pJnlDayTotal->EndDate_Month = (NUINT16)nTempValue;

	nTempValue = Asc2Int(arJnlField[10].Mid(3,2));
	pJnlDayTotal->EndDate_Day = (NUINT16)nTempValue;

	nTempValue = Asc2Int(arJnlField[10].Mid(6,4));
	pJnlDayTotal->EndDate_Year = (NUINT16)nTempValue;

	nTempValue = Asc2Int(arJnlField[10].Mid(11,2));
	pJnlDayTotal->EndDate_Hour = (NUINT16)nTempValue;

	nTempValue = Asc2Int(arJnlField[10].Mid(14,2));
	pJnlDayTotal->EndDate_Minute = (NUINT16)nTempValue;

	nTempValue = Asc2Int(arJnlField[10].Mid(17,2));
	pJnlDayTotal->EndDate_Second = (NUINT16)nTempValue;

	//  3. Terminal Number
	arJnlField[11].TrimLeft();
	arJnlField[11].TrimRight();
	RMS_WideToMulti(arJnlField[11], (char*)pJnlDayTotal->TerminalNumber, sizeof(pJnlDayTotal->TerminalNumber));

	//  4. Count of Withdrawals
	arJnlField[12].TrimLeft();
	arJnlField[12].TrimRight();
	RMS_WideToMulti(arJnlField[12], (char*)pJnlDayTotal->Count_Withdrawals, sizeof(pJnlDayTotal->Count_Withdrawals));

	//  5. Amount of Dispense
	arJnlField[13].TrimLeft();
	arJnlField[13].TrimRight();
	RMS_WideToMulti(arJnlField[13], (char*)pJnlDayTotal->Amt_Dispense, sizeof(pJnlDayTotal->Amt_Dispense));

	return sizeof(RMS_JNL_BRIEF_DAYTOTAL);
}

int  CRmsCtrl::JNLConvertBriefCassetteTotal(BYTE *pMsg, int Len, CStringArray &arJnlField)
{
	PRMS_JNL_BRIEF_CSTTOTAL	pJnlCSTTotal = (PRMS_JNL_BRIEF_CSTTOTAL)pMsg;
	int	nTempValue;

	if (Len < sizeof(RMS_JNL_BRIEF_CSTTOTAL))
		return 0;

	// exception
	if (arJnlField.GetSize() < 16)
		return 0;

	memset(pJnlCSTTotal, 0, sizeof(RMS_JNL_BRIEF_CSTTOTAL));

	//  1. Logging Start Date
	nTempValue = Asc2Int(arJnlField[9].Left(2));
	pJnlCSTTotal->StartDate_Month = (NUINT16)nTempValue;

	nTempValue = Asc2Int(arJnlField[9].Mid(3,2));
	pJnlCSTTotal->StartDate_Day = (NUINT16)nTempValue;

	nTempValue = Asc2Int(arJnlField[9].Mid(6,4));
	pJnlCSTTotal->StartDate_Year = (NUINT16)nTempValue;

	nTempValue = Asc2Int(arJnlField[9].Mid(11,2));
	pJnlCSTTotal->StartDate_Hour = (NUINT16)nTempValue;

	nTempValue = Asc2Int(arJnlField[9].Mid(14,2));
	pJnlCSTTotal->StartDate_Minute = (NUINT16)nTempValue;

	nTempValue = Asc2Int(arJnlField[9].Mid(17,2));
	pJnlCSTTotal->StartDate_Second = (NUINT16)nTempValue;

	//  2. Logging End Date
	nTempValue = Asc2Int(arJnlField[10].Left(2));
	pJnlCSTTotal->EndDate_Month = (NUINT16)nTempValue;

	nTempValue = Asc2Int(arJnlField[10].Mid(3,2));
	pJnlCSTTotal->EndDate_Day = (NUINT16)nTempValue;

	nTempValue = Asc2Int(arJnlField[10].Mid(6,4));
	pJnlCSTTotal->EndDate_Year = (NUINT16)nTempValue;

	nTempValue = Asc2Int(arJnlField[10].Mid(11,2));
	pJnlCSTTotal->EndDate_Hour = (NUINT16)nTempValue;

	nTempValue = Asc2Int(arJnlField[10].Mid(14,2));
	pJnlCSTTotal->EndDate_Minute = (NUINT16)nTempValue;

	nTempValue = Asc2Int(arJnlField[10].Mid(17,2));
	pJnlCSTTotal->EndDate_Second = (NUINT16)nTempValue;

	//  3. Terminal Number
	arJnlField[11].TrimLeft();
	arJnlField[11].TrimRight();
	RMS_WideToMulti(arJnlField[11], (char*)pJnlCSTTotal->TerminalNumber, sizeof(pJnlCSTTotal->TerminalNumber));

	//  5. CST1 Dispensed Notes
	nTempValue = Asc2Int(arJnlField[12]);
	pJnlCSTTotal->CST1_Dispensed = (NUINT16)nTempValue;

	//  6. CST2 Dispensed Notes
	nTempValue = Asc2Int(arJnlField[13]);
	pJnlCSTTotal->CST2_Dispensed = (NUINT16)nTempValue;

	//  7. CST3 Dispensed Notes
	nTempValue = Asc2Int(arJnlField[14]);
	pJnlCSTTotal->CST3_Dispensed = (NUINT16)nTempValue;

	//  8. CST4 Dispensed Notes
	nTempValue = Asc2Int(arJnlField[15]);
	pJnlCSTTotal->CST4_Dispensed = (NUINT16)nTempValue;

	return sizeof(RMS_JNL_BRIEF_CSTTOTAL);
}
// End of [#2385]

/*-------------------------------------------------------------------
 CLASS    NAME: CRmsCtrl
 FUNCTION NAME: JNLConvertLibertyXTransactions()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Convert LibertyX Journal entry to RMS journal
-------------------------------------------------------------------*/
int  CRmsCtrl::JNLConvertLibertyXTransactions(BYTE* pMsg, int Len, CStringArray &arJnlField)
{
	PRMS_JNL_LIBERTYX jnl = (PRMS_JNL_LIBERTYX)pMsg;
	const int jnlSize = sizeof(RMS_JNL_LIBERTYX);

	if (Len < jnlSize)
	{
		return 0;
	}

	memset(jnl, 0, jnlSize);

	// 1. Terminal ID
	RMS_WideToMulti(arJnlField[9], (char*)jnl->LocationId, sizeof(jnl->LocationId));

	// 2. Routing ID
	RMS_WideToMulti(arJnlField[10], (char*)jnl->RoutingId, sizeof(jnl->RoutingId));

	// 3. API Host
	RMS_WideToMulti(arJnlField[11], (char*)jnl->APIUrl, sizeof(jnl->APIUrl));

	// 4. TLS y/n
	RMS_WideToMulti(arJnlField[12], (char*)jnl->TLSEnabled, sizeof(jnl->TLSEnabled));

	// 5. Payment ID
	RMS_WideToMulti(arJnlField[13], (char*)jnl->PaymentId, sizeof(jnl->PaymentId));

	// 6. Order ID
	RMS_WideToMulti(arJnlField[14], (char*)jnl->Token, sizeof(jnl->Token));

	// 7. Amount
	RMS_WideToMulti(arJnlField[15], (char*)jnl->SourceAmount, sizeof(jnl->SourceAmount));

	// 8-9. Authorization Result
	RMS_WideToMulti(arJnlField[16], (char*)jnl->AuthStatus, sizeof(jnl->AuthStatus));
	RMS_WideToMulti(arJnlField[17], (char*)jnl->AuthMessage, sizeof(jnl->AuthMessage));

	// 10. Debit Auth Code
	CString authCode = arJnlField[18].Left(arJnlField[18].Find('/'));
	if (authCode != arJnlField[18])
	{
		// Format: "XXXXXX/Seq.# SSSS"
		// There is a sequence number, so we need to reformat it for MoniView
		CString sequence = arJnlField[18].Right(4);
		arJnlField[18].Format(L"%s/%s", authCode, sequence);
	}
	RMS_WideToMulti(arJnlField[18], (char*)jnl->AuthCode, sizeof(jnl->AuthCode));

	// 11-12. Card Data (Last 4, Name)
	RMS_WideToMulti(arJnlField[19], (char*)jnl->Last4, sizeof(jnl->Last4));
	RMS_WideToMulti(arJnlField[20], (char*)jnl->NameOnCard, sizeof(jnl->NameOnCard));

	// 13. Error Code
	if (!arJnlField[21].IsEmpty())
	{
		RMS_WideToMulti(arJnlField[21], (char*)jnl->Error, sizeof(jnl->Error));
	}
	else
	{
		strcpy((char *)jnl->Error, "0000000");
	}

	return jnlSize;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CRmsCtrl
 FUNCTION NAME: JNLConvertLibertyXDispenseTransactions()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Convert LibertyX Cash Out Journal entry to RMS journal
-------------------------------------------------------------------*/
int CRmsCtrl::JNLConvertLibertyXDispenseTransactions(BYTE* pMsg, int Len, CStringArray &arJnlField)
{
	PRMS_JNL_LIBERTYX_DISPENSE jnl = (PRMS_JNL_LIBERTYX_DISPENSE)pMsg;
	const int jnlSize = sizeof(RMS_JNL_LIBERTYX_DISPENSE);

	if (Len < jnlSize)
	{
		return 0;
	}

	memset(jnl, 0, jnlSize);

	// 1. Terminal ID
	RMS_WideToMulti(arJnlField[9], (char*)jnl->LocationId, sizeof(jnl->LocationId));

	// 2. Routing ID
	RMS_WideToMulti(arJnlField[10], (char*)jnl->RoutingId, sizeof(jnl->RoutingId));

	// 3. API Host
	RMS_WideToMulti(arJnlField[11], (char*)jnl->APIUrl, sizeof(jnl->APIUrl));

	// 4. TLS y/n
	RMS_WideToMulti(arJnlField[12], (char*)jnl->TLSEnabled, sizeof(jnl->TLSEnabled));

	// 5. Token
	RMS_WideToMulti(arJnlField[13], (char*)jnl->Token, sizeof(jnl->Token));

	// 6. Nonce
	RMS_WideToMulti(arJnlField[14], (char*)jnl->Nonce, sizeof(jnl->Nonce));

	// 7. Amount
	RMS_WideToMulti(arJnlField[15], (char*)jnl->Amount, sizeof(jnl->Amount));

	// 8-9. Authorization Result
	RMS_WideToMulti(arJnlField[16], (char*)jnl->AuthResult, sizeof(jnl->AuthResult));
	RMS_WideToMulti(arJnlField[17], (char*)jnl->AuthMessage, sizeof(jnl->AuthMessage));

	// 10-12. Execute Status
	RMS_WideToMulti(arJnlField[18], (char*)jnl->ExecuteResult, sizeof(jnl->ExecuteResult));
	RMS_WideToMulti(arJnlField[19], (char*)jnl->ExecuteMessage, sizeof(jnl->ExecuteMessage));
	RMS_WideToMulti(arJnlField[20], (char*)jnl->ExecuteMessageCustomer, sizeof(jnl->ExecuteMessageCustomer));

	// 13-15. Signatures
	RMS_WideToMulti(arJnlField[21], (char*)jnl->SignatureA, sizeof(jnl->SignatureA));
	RMS_WideToMulti(arJnlField[22], (char*)jnl->SignatureB, sizeof(jnl->SignatureB));
	RMS_WideToMulti(arJnlField[23], (char*)jnl->SignatureC, sizeof(jnl->SignatureC));

	// 16-18. Certs
	RMS_WideToMulti(arJnlField[24], (char*)jnl->CertACN, sizeof(jnl->CertACN));
	RMS_WideToMulti(arJnlField[25], (char*)jnl->CertBCN, sizeof(jnl->CertBCN));
	RMS_WideToMulti(arJnlField[26], (char*)jnl->CertCCN, sizeof(jnl->CertCCN));

	// 19-20. Dispense Result
	RMS_WideToMulti(arJnlField[27], (char*)jnl->RequesetedAmount, sizeof(jnl->RequesetedAmount));
	RMS_WideToMulti(arJnlField[28], (char*)jnl->DispensedAmount, sizeof(jnl->DispensedAmount));

	// 21-22. Error Code
	if (!arJnlField[29].IsEmpty())
	{
		RMS_WideToMulti(arJnlField[29], (char*)jnl->Error, sizeof(jnl->Error));
		RMS_WideToMulti(arJnlField[30], (char*)jnl->ErrorMessage, sizeof(jnl->ErrorMessage));
		RMS_WideToMulti(arJnlField[31], (char*)jnl->ErrorData, sizeof(jnl->ErrorData));
	}
	else
	{
		strcpy((char *)jnl->Error, "0000000");
		strcpy((char *)jnl->ErrorMessage, "");
		strcpy((char *)jnl->ErrorData, "");
	}

	return jnlSize;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CRmsCtrl
 FUNCTION NAME: JNLConvertB4UTransactions()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Convert B4U Journal entry to RMS journal
-------------------------------------------------------------------*/
int  CRmsCtrl::JNLConvertB4UTransactions(BYTE* pMsg, int Len, CStringArray &arJnlField)
{
	PRMS_JNL_B4U jnl = (PRMS_JNL_B4U)pMsg;
	const int jnlSize = sizeof(RMS_JNL_B4U);

	if (Len < jnlSize)
	{
		return 0;
	}

	memset(jnl, 0, jnlSize);

	// 1. Transaction Time // YYYYMMDDHHMMSS
	int nTmp;
	CString strTmp;
	CString sDateTime = L"";
	strTmp.Format(L"%s", arJnlField[3]);
	if( strTmp.GetLength() == 2 )	strTmp.Format(L"20%s", arJnlField[3]);
	sDateTime.Format(L"%4.4s%2.2s%2.2s%2.2s%2.2s%2.2s", strTmp, arJnlField[4], arJnlField[5], arJnlField[6], arJnlField[7], arJnlField[8]);
	RMS_WideToMulti(sDateTime, (char*)jnl->TransactionTime, sizeof(jnl->TransactionTime));

	// 2. Terminal Number
	arJnlField[9].TrimLeft();
	arJnlField[9].TrimRight();
	RMS_WideToMulti(arJnlField[9], (char*)jnl->TerminalNumber, sizeof(jnl->TerminalNumber));

	// 3. Sequence Number
	arJnlField[10].TrimLeft();
	arJnlField[10].TrimRight();
	RMS_WideToMulti(arJnlField[10], (char*)jnl->SequenceNumber, sizeof(jnl->SequenceNumber));

	// 5. BtcCharged
	RMS_WideToMulti(arJnlField[22], (char*)jnl->BtcCharged, sizeof(jnl->BtcCharged));

	// 4. TransactionHash
	RMS_WideToMulti(arJnlField[23], (char*)jnl->TransactionHash, sizeof(jnl->TransactionHash));

	// 6. Request Amount
	arJnlField[24].TrimLeft();
	arJnlField[24].TrimRight();
	nTmp = Asc2Int(arJnlField[24]);
	strTmp.Format(L"%08d", nTmp);
	RMS_WideToMulti(strTmp, (char*)jnl->RequestAmount, sizeof(jnl->RequestAmount));

	// 7. Dispense Amount
	arJnlField[25].TrimLeft();
	arJnlField[25].TrimRight();
	nTmp = Asc2Int(arJnlField[25]);
	strTmp.Format(L"%08d", nTmp);
	RMS_WideToMulti(strTmp, (char*)jnl->DispensedAmount, sizeof(jnl->DispensedAmount));

	// 8. Transaction Result Code
	arJnlField[1].TrimLeft();
	RMS_WideToMulti(arJnlField[1], (char*)jnl->TransactionResult, sizeof(jnl->TransactionResult));

	return jnlSize;
}

// [#RWC6-399] US ryan.payton 2022.08.02 DigitalMint
/*-------------------------------------------------------------------
 CLASS    NAME: CRmsCtrl
 FUNCTION NAME: JNLConvertDigitalMintTransactions()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Convert DigitalMint Journal entry to RMS journal
-------------------------------------------------------------------*/
int  CRmsCtrl::JNLConvertDigitalMintTransactions(BYTE* pMsg, int Len, CStringArray &arJnlField)
{
	PRMS_JNL_DIGITALMINT jnl = (PRMS_JNL_DIGITALMINT)pMsg;
	const int jnlSize = sizeof(RMS_JNL_DIGITALMINT);

	if (Len < jnlSize)
	{
		return 0;
	}

	memset(jnl, 0, jnlSize);

	// Date/Time
	CString strResult;
	strResult.AppendFormat(L"%s %2.2s:%2.2s:%2.2s", GetCmnLocalDate(arJnlField[4], arJnlField[5], arJnlField[3]), arJnlField[6], arJnlField[7], arJnlField[8]);
	RMS_WideToMulti(strResult, (char*)jnl->TransactionTime, sizeof(jnl->TransactionTime));

	// Terminal ID
	RMS_WideToMulti(arJnlField[9], (char*)jnl->TerminalNumber, sizeof(jnl->TerminalNumber));

	// Session ID
	RMS_WideToMulti(arJnlField[10], (char*)jnl->SessionId, sizeof(jnl->SessionId));

	// Transaction ID
	RMS_WideToMulti(arJnlField[11], (char*)jnl->TransactionId, sizeof(jnl->TransactionId));

	// Account ID
	RMS_WideToMulti(arJnlField[12], (char*)jnl->AccountId, sizeof(jnl->AccountId));

	// API URL
	RMS_WideToMulti(arJnlField[13], (char*)jnl->APIUrl, sizeof(jnl->APIUrl));

	// Selected Coin
	RMS_WideToMulti(arJnlField[14], (char*)jnl->SelectedCoin, sizeof(jnl->SelectedCoin));

	// Transaction Mode
	RMS_WideToMulti(arJnlField[15], (char*)jnl->TransactionMode, sizeof(jnl->TransactionMode));

	// Transaction Amount
	int amount = Asc2Int(arJnlField[16].Trim());
	strResult.Format(L"%08d", amount);
	RMS_WideToMulti(strResult, (char*)jnl->RequestAmount, sizeof(jnl->RequestAmount));

	// Error Code
	if (arJnlField[1] == DIGITALMINT_TXN)
	{
		strcpy((char *)jnl->Error, "       ");
	}
	else if (arJnlField[17].IsEmpty())
	{
		strcpy((char *)jnl->Error, "0000000");
	}
	else
	{
		RMS_WideToMulti(arJnlField[17], (char*)jnl->Error, sizeof(jnl->Error));
	}

	return jnlSize;
}
// End of [#RWC6-188]

/*-------------------------------------------------------------------
 CLASS    NAME: CRmsCtrl
 FUNCTION NAME: JNLConvertChangeProcessor()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Change Processor JNL을 ATM->RMS 형식으로 변경한다.
-------------------------------------------------------------------*/
int  CRmsCtrl::JNLConvertChangeProcessor(BYTE *pMsg, int Len, CStringArray &arJnlField)
{
	PRMS_JNL_PROCESSOR	pJnlProcessor = (PRMS_JNL_PROCESSOR)pMsg;

	if (Len < sizeof(RMS_JNL_PROCESSOR))
		return 0;

	// exception
	if (arJnlField.GetSize() < 11)
		return 0;

	memset(pJnlProcessor, 0, sizeof(RMS_JNL_PROCESSOR));

	//  1. Old Processor
	arJnlField[9].TrimLeft();
	arJnlField[9].TrimRight();
	RMS_WideToMulti(arJnlField[9], (char*)pJnlProcessor->OldProcessor, sizeof(pJnlProcessor->OldProcessor));

	//  2. New Processor
	arJnlField[10].TrimLeft();
	arJnlField[10].TrimRight();
	RMS_WideToMulti(arJnlField[10], (char*)pJnlProcessor->NewProcessor, sizeof(pJnlProcessor->NewProcessor));

	return sizeof(RMS_JNL_PROCESSOR);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CRmsCtrl
 FUNCTION NAME: JNLConvertChangePassword()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Change Processor JNL을 ATM->RMS 형식으로 변경한다.
-------------------------------------------------------------------*/
int  CRmsCtrl::JNLConvertChangePassword(BYTE *pMsg, int Len, CStringArray &arJnlField)
{
	PRMS_JNL_PASSWORD	pJnlPassword = (PRMS_JNL_PASSWORD)pMsg;

	if (Len < sizeof(RMS_JNL_PASSWORD))
		return 0;

	// exception
	if (arJnlField.GetSize() < 11)
		return 0;

	memset(pJnlPassword, '*', sizeof(RMS_JNL_PASSWORD));

	return sizeof(RMS_JNL_PASSWORD);
}

// [#408] AU AIREAT 2008.08.26
/*-------------------------------------------------------------------
 CLASS    NAME: CRmsCtrl
 FUNCTION NAME: JNLConvertOperatorAction()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Operator Action JNL을 ATM->RMS 형식으로 변경한다.
-------------------------------------------------------------------*/
int  CRmsCtrl::JNLConvertOperatorAction(BYTE *pMsg, int Len, CStringArray &arJnlField)
{
	PRMS_JNL_OPERATOR_ACTION	pJnlOperatorAct = (PRMS_JNL_OPERATOR_ACTION)pMsg;

	if (Len < sizeof(RMS_JNL_OPERATOR_ACTION))
		return 0;

	// exception
	if (arJnlField.GetSize() < 10)
		return 0;

	RMS_WideToMulti(arJnlField[9], (char*)pJnlOperatorAct->Action, sizeof(pJnlOperatorAct->Action));

	return sizeof(RMS_JNL_OPERATOR_ACTION);
}
// end of [#408]


// KSK 2009.3.30
/*-------------------------------------------------------------------
 CLASS    NAME: CRmsCtrl
 FUNCTION NAME: JNLConvertChangeExchangeRate()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : JNLConvertChangeExchangeRate JNL을 ATM->RMS 형식으로 변경한다.
-------------------------------------------------------------------*/
int  CRmsCtrl::JNLConvertChangeExchangeRate(BYTE *pMsg, int Len, CStringArray &arJnlField)
{
	PRMS_JNL_CHANGE_EXCHANGE_RATE	pJnlChangeExchangeRate = (PRMS_JNL_CHANGE_EXCHANGE_RATE)pMsg;

	if (Len < sizeof(RMS_JNL_CHANGE_EXCHANGE_RATE))
		return 0;

	// exception
	if (arJnlField.GetSize() < 10)
		return 0;
	
	RMS_WideToMulti(arJnlField[9], (char*)pJnlChangeExchangeRate->Action, sizeof(pJnlChangeExchangeRate->Action));

	return sizeof(RMS_JNL_CHANGE_EXCHANGE_RATE);
}
// end of [#408]

// [#529] AU AIREAT 2009.06.02
/*-------------------------------------------------------------------
 CLASS    NAME: CRmsCtrl
 FUNCTION NAME: JNLConvertUserCancel()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : JNLConvertUserCancel JNL을 ATM->RMS 형식으로 변경한다.
-------------------------------------------------------------------*/
int	 CRmsCtrl::JNLConvertUserCancel(BYTE *pMsg, int Len, CStringArray &arJnlField)
{
	PRMS_JNL_USER_CANCEL	pJnlUserCancel = (PRMS_JNL_USER_CANCEL)pMsg;

	if (Len < sizeof(RMS_JNL_USER_CANCEL))
		return 0;

	// exception
	if (arJnlField.GetSize() < 10)
		return 0;

	RMS_WideToMulti(arJnlField[9], (char*)pJnlUserCancel->Action, sizeof(pJnlUserCancel->Action));

	return sizeof(RMS_JNL_USER_CANCEL);
}
// end of [#529]
// [#610] SOOK 2010.01.18
/*-------------------------------------------------------------------
 CLASS    NAME: CRmsCtrl
 FUNCTION NAME: JNLConvertChangeParameter()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : JNLConvertChangeParameter JNL을 ATM->RMS 형식으로 변경한다.
-------------------------------------------------------------------*/
int	 CRmsCtrl::JNLConvertChangeParameter(BYTE *pMsg, int Len, CStringArray &arJnlField)
{
	PRMS_JNL_CHANGE_PARAMETER 	pJnlChangeParameter = (PRMS_JNL_CHANGE_PARAMETER)pMsg;

	if (Len < sizeof(RMS_JNL_CHANGE_PARAMETER))
		return 0;

	// exception
	if (arJnlField.GetSize() < 10)
		return 0;
	
	int nTempValue = Asc2Int(arJnlField[9]);
	pJnlChangeParameter->User = (NUINT8) nTempValue;
	RMS_WideToMulti(arJnlField[10], (char*)pJnlChangeParameter->Parameter, sizeof(pJnlChangeParameter->Parameter));

	CString strOldValue = arJnlField[11];
	CString strNewValue = arJnlField[12];

	CString strTotalString;
	strTotalString.Format(L"%s^%s", strOldValue, strNewValue);

	RMS_WideToMulti(strTotalString, (char*)pJnlChangeParameter->Value, strTotalString.GetLength());

	int nLen = sizeof(pJnlChangeParameter->Parameter) + strTotalString.GetLength() + 1;// 2: (nUser(1) + UNIT_DELIMITER(1))

	return nLen;
}
// end of [#610]

// [#2279] NH Justin 2014.06.17 Uploading EMV data to Moniview
/*-------------------------------------------------------------------
 CLASS    NAME: CRmsCtrl
 FUNCTION NAME: JNLConvertTrxEMVData()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : JNLConvertTrxEMVData : Convert Journal format : ATM->RMS
-------------------------------------------------------------------*/
int CRmsCtrl::JNLConvertTrxEMVData(BYTE *pMsg, int Len, CStringArray &arJnlField)
{
	PRMS_JNL_TRXEMVDATA	pJnlTrxEMV = (PRMS_JNL_TRXEMVDATA)pMsg;

	if (Len < sizeof(RMS_JNL_TRXEMVDATA))
		return 0;

	if (arJnlField.GetSize() < 13)
		return 0;

	memset(pJnlTrxEMV, 0, sizeof(RMS_JNL_TRXEMVDATA));

	// 1. Terminal Number
	arJnlField[9].TrimLeft();
	arJnlField[9].TrimRight();

	// 2. Sequence Number
	arJnlField[10].TrimLeft();
	arJnlField[10].TrimRight();

	// 3.  Host Audit & Net ID
	arJnlField[11].TrimLeft();
	arJnlField[11].TrimRight();

	// 4. EMV Data
	arJnlField[12].TrimLeft();
	arJnlField[12].TrimRight();

	CString strTotalData;
	strTotalData.Format(L"%s^%s^%s^%s", arJnlField[9], arJnlField[10], arJnlField[11], arJnlField[12]);

	if(strTotalData.GetLength()>300)	// Moniview column Size
		return 0;
	// "REAL" EMV journal data is not bigger than 300, If it is bigger than 300, it is incorrect data.
	// TID(15), SEQ(4), Audit(8), ^(3), AID(6+16), Name(6+20), 
	// AC(6+16), ARPC(6+4or6), Service Code(6+3), TC(6+1), POS Entry(6+3), TVR(6+10), IAC(6+10)
	// Total 171 Byte => 129 Byte Left
	// [#2353] AU KSK 2015.06.28 Added EMV Data
	// 5F34(6+2), 82(6+4), 8A(6+4), 9C(6+2), 9B(6+4), 9F10(6+20), 9F27(6+2), 9F36(6+2), 9F37(6+8)
	// Total 273 Byte => 27 Byte Left

	RMS_WideToMulti(strTotalData, (char*)pJnlTrxEMV->EMVData, strTotalData.GetLength() );

	int nLen = strTotalData.GetLength();  

	return nLen;
}
// End of [#2279]

BOOL CRmsCtrl::GetCSTStatus(PCST_STATUS pCSTStatus)
{
	int			nCSTCnt, nTemp;

	memset(pCSTStatus, 0, sizeof(CST_STATUS));

	// CST
	nCSTCnt = MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_NUMBEROFCST);

	//v01.01.01
	//Edit for checking CST status "Not Available". Once ATM notices usType is "NA"
	//ATM reports RMS the value as "0x06"
	//For detail added statement, refer to "**ADDED**"
	pCSTStatus->CST1Status = 0x06;
	pCSTStatus->CST2Status = 0x06;
	pCSTStatus->CST3Status = 0x06;
	pCSTStatus->CST4Status = 0x06;

	// CST1 Denomination, Bill Count, Status.
	if(nCSTCnt >= 1)
	{
		pCSTStatus->CST1Denomination = MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CSTVALUE1);
		// [#2270] AU KSK 2014.05.26 
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE) == 0)
			pCSTStatus->CST1BillCount = MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CSTCOUNT1);
		else
			pCSTStatus->CST1BillCount = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_CURRENTCOUNT_AP);
		// end of [#2270]

		nTemp = MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CSTSTATUS1);
		//nTemp는 XFS 사양의 CU status (0 ~ 9)
		switch(nTemp)
		{
		case 0:		//WFS_CDM_STATCUOK
			pCSTStatus->CST1Status = 0x01;
			break;
		case 1:		//WFS_CDM_STATCUFULL		
		case 2:		//WFS_CDM_STATCUHIGH
			pCSTStatus->CST1Status = 0x05;
			break;
		case 3:		//WFS_CDM_STATCULOW
			pCSTStatus->CST1Status = 0x03;
			break;
		case 4:		//WFS_CDM_STATCUEMPTY
			pCSTStatus->CST1Status = 0x04;
			break;
		case 5:		//WFS_CDM_STATCUINOP
		case 6:		//WFS_CDM_STATCUMISSING
			pCSTStatus->CST1Status = 0x02;
			break;
		default:
			pCSTStatus->CST1Status = 0xff;
			break;
		}
	}

	// 15, 16, 17 CST2 Denomination, Bill Count, Status.
	if(nCSTCnt >= 2)
	{
		pCSTStatus->CST2Denomination = MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CSTVALUE2);

		// [#2270] AU KSK 2014.05.26 
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE) == 0)
			pCSTStatus->CST2BillCount = MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CSTCOUNT2);
		else
			pCSTStatus->CST2BillCount = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST2_CURRENTCOUNT_AP);
		// end of [#2270]

		nTemp = MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CSTSTATUS2);
		//nTemp는 XFS 사양의 CU status (0 ~ 9)
		switch(nTemp)
		{
		case 0:		//WFS_CDM_STATCUOK
			pCSTStatus->CST2Status = 0x01;
			break;
		case 1:		//WFS_CDM_STATCUFULL		
		case 2:		//WFS_CDM_STATCUHIGH
			pCSTStatus->CST2Status = 0x05;
			break;
		case 3:		//WFS_CDM_STATCULOW
			pCSTStatus->CST2Status = 0x03;
			break;
		case 4:		//WFS_CDM_STATCUEMPTY
			pCSTStatus->CST2Status = 0x04;
			break;
		case 5:		//WFS_CDM_STATCUINOP
		case 6:		//WFS_CDM_STATCUMISSING
			pCSTStatus->CST2Status = 0x02;
			break;
		default:
			pCSTStatus->CST2Status = 0xff;
			break;
		}
	}

	// 18, 19, 20 CST3 Denomination, Bill Count, Status.
	if(nCSTCnt >= 3)
	{
		pCSTStatus->CST3Denomination = MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CSTVALUE3);

		// [#2270] AU KSK 2014.05.26 
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE) == 0)
			pCSTStatus->CST3BillCount = MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CSTCOUNT3);
		else
			pCSTStatus->CST3BillCount = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST3_CURRENTCOUNT_AP);
		// end of [#2270]

		nTemp = MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CSTSTATUS3);
		//nTemp는 XFS 사양의 CU status (0 ~ 9)
		switch(nTemp)
		{
		case 0:		//WFS_CDM_STATCUOK
			pCSTStatus->CST3Status = 0x01;
			break;
		case 1:		//WFS_CDM_STATCUFULL		
		case 2:		//WFS_CDM_STATCUHIGH
			pCSTStatus->CST3Status = 0x05;
			break;
		case 3:		//WFS_CDM_STATCULOW
			pCSTStatus->CST3Status = 0x03;
			break;
		case 4:		//WFS_CDM_STATCUEMPTY
			pCSTStatus->CST3Status = 0x04;
			break;
		case 5:		//WFS_CDM_STATCUINOP
		case 6:		//WFS_CDM_STATCUMISSING
			pCSTStatus->CST3Status = 0x02;
			break;
		default:
			pCSTStatus->CST3Status = 0xff;
			break;
		}
	}

	// 21, 22, 23 CST4 Denomination, Bill Count, Status.
	if(nCSTCnt >= 4)
	{
		pCSTStatus->CST4Denomination = MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CSTVALUE4);

		// [#2270] AU KSK 2014.05.26 
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE) == 0)
			pCSTStatus->CST4BillCount = MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CSTCOUNT4);
		else
			pCSTStatus->CST4BillCount = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST4_CURRENTCOUNT_AP);
		// end of [#2270]

		nTemp = MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CSTSTATUS4);
		//nTemp는 XFS 사양의 CU status (0 ~ 9)
		switch(nTemp)
		{
		case 0:		//WFS_CDM_STATCUOK
			pCSTStatus->CST4Status = 0x01;
			break;
		case 1:		//WFS_CDM_STATCUFULL		
		case 2:		//WFS_CDM_STATCUHIGH
			pCSTStatus->CST4Status = 0x05;
			break;
		case 3:		//WFS_CDM_STATCULOW
			pCSTStatus->CST4Status = 0x03;
			break;
		case 4:		//WFS_CDM_STATCUEMPTY
			pCSTStatus->CST4Status = 0x04;
			break;
		case 5:		//WFS_CDM_STATCUINOP
		case 6:		//WFS_CDM_STATCUMISSING
			pCSTStatus->CST4Status = 0x02;
			break;
		default:
			pCSTStatus->CST4Status = 0xff;
			break;
		}
	}

	//save CST status for "Device Status" field
	pCSTStatus->dwCDUCSTstatus = 0x00000000;

	//Set CST status for "Device Status" field
	int CstNoteStatus = m_pDevCmn->fnCDU_GetAllCSTStatus();
	if (CstNoteStatus == CST_EMPTY || CstNoteStatus == CST_NEAR)
		pCSTStatus->dwCDUCSTstatus |= 0x00000020;	// Out of Cash
	else if (m_pDevCmn->fnCDU_GetRecycleBoxStatus() == CST_NEAR)	// 전체 Cassette가 low인지를 Check한다.
		pCSTStatus->dwCDUCSTstatus |= 0x00000010;	// Low Cash
	// end of [#228]

	// KSK 2009.2.25 ADD CDU Device Fault Status
	if (m_pDevCmn->fnAPL_GetDownErrorDevice(DEV_CDU))
		pCSTStatus->dwCDUCSTstatus |= 0x00000002;	// CDU Device fault
	// end of KSK 2009.2.25

	return TRUE;
}
//[#546] SOOK 2009.07.23 다기능 BIN LIST 기능 추가함 

#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
int  CRmsCtrl::RMS_MakeBinListMsg(BYTE *pMsg)	
{
	CString	strTemp;
	PRMS_BINLIST_INFO 	pBinList;

	pBinList = (PRMS_BINLIST_INFO) pMsg;
	memset(pBinList, 0, sizeof(RMS_BINLIST_INFO));


	pBinList->AllowOnlyListedBinEnable = (NUINT8)m_pDevCmn->m_BINMgr.GetAllowOnlyBinEnable();
	int nCount = m_pDevCmn->m_BINMgr.GetTotalBinCount();
	pBinList->TotalBinCount = (NUINT16)nCount;
//	pBinList->SetupDataLength = (NUINT32)(nCount*16) + 3;

	for( int i = 0; i < nCount; i++)
	{
		BinProperties binProperties;
		memset(&binProperties, 0x00, sizeof( BinProperties));

		m_pDevCmn->m_BINMgr.GetBinPropertesByIndex(i, &binProperties);
		
		memcpy(&pBinList->BinList[i], &binProperties, sizeof(RMS_BINPROPERTIES));
	}
	
	return (int)(nCount*16) + 3; 
}

BOOL CRmsCtrl::RMS_WriteBinListMsg(BYTE *pMsg, int Len)
{
	PRMS_BINLIST_INFO	pBinList;

	pBinList = (PRMS_BINLIST_INFO)pMsg;
	if (pBinList->TotalBinCount > BIN_MAXIMUN_COUNT ) return FALSE;
	//Allow ONLY LIST BIN Enable 셋팅 
	if ( pBinList->AllowOnlyListedBinEnable != 0)
		pBinList->AllowOnlyListedBinEnable = 1;
	m_pDevCmn->m_BINMgr.SetAllowOnlyBinEnable((WORD)pBinList->AllowOnlyListedBinEnable);

	//기존에 있는 것들을 모두 지운다. 
	m_pDevCmn->m_BINMgr.DeleteAllList();

	//하나씩 삽입한다. 
	for( int i = 0; i < pBinList->TotalBinCount; i++)
	{
		BinProperties newBin;
		memcpy(newBin.BinNo, pBinList->BinList[i].BinNumber, 10);
		newBin.wAction = pBinList->BinList[i].BinAction;
		newBin.dwValue = pBinList->BinList[i].BinValue;
		
		if ( m_pDevCmn->m_BINMgr.GetTotalBinCount() < BIN_MAXIMUN_COUNT)
			m_pDevCmn->m_BINMgr.InsertBinPropertiesWithoutSorting (newBin);
	}
	
	return TRUE;
}
#endif
/////////////////////////////////////////////////////////////////////
//
//	RMS_ProcXXXXXXXX(void) 함수 처리시 공통사항.
//
//  <Buffer 사용>
//
//
//  <Return> : 값은 의미가 없다.
//	함수 종료시 Caller에서 다음 사항이 처리된다. 
//	1. Send EOT, 
//  2. close RMS connection
#if (AU_VERSION)	// [#622] NH KSK 2010.02.24	// [#2069] NH KSK 2011.06.13

BOOL CRmsCtrl::RMS_ProcReqBinList()
{
	int nLength = RMS_MakeBinListMsg(gpRMSDataBuffer);

	BYTE*	pSetupData =gpRMSDataBuffer;
	PRMS_SETUP_READ_DATA	pSetupRead =(PRMS_SETUP_READ_DATA) gpRMSBuffer;
	int		nWriteData, FrameNumber;
	nWriteData = nLength;

	FrameNumber = 1;

	while(nWriteData > 0)
	{
//		RMS_MakeSetupDataMsg((BYTE*)pSetupRead, RMS_BUFSIZE);  //SOOK 
		RMS_MakeSetupReadDataMsg((BYTE*)pSetupRead, RMS_BUFSIZE);
		memset(pSetupRead->SetupData, 0, sizeof(pSetupRead->SetupData));
		if (nWriteData > 512)
			nWriteData = 512;

//		memcpy(pSetupRead->SetupData, pSetupData, nWriteData);
		memcpy(pSetupRead->SetupData, pSetupData, nWriteData);
		
		pSetupRead->Header.MsgID = RMS_REQ_BINLIST;
		pSetupRead->Header.Length = LEN_RMS_SETUP_READ_DATA - (512 - nWriteData);
		pSetupRead->FrameNumber = (NUINT16)FrameNumber++;
		pSetupData += nWriteData;
		
		// First Frame.
		if (pSetupRead->FrameNumber == 1)
			pSetupRead->FrameNumber |= 0x0000;


		// Last Frame.
		if (nWriteData < 512)
			pSetupRead->FrameNumber |= 0x1000;

		
		// update.
		nWriteData = nLength - (pSetupData - gpRMSDataBuffer);
		
		NHDEBUG(1, (_T("Send RMS_REQ_SETUP. Frame(%d) TO RMS..\n"), pSetupRead->FrameNumber));
		
		if (RMS_Send((BYTE*)pSetupRead, pSetupRead->Header.Length, 0) != RMS_OK)
		{
			NHDEBUG(1, (_T("Send Failed\n")));
			break;
		}
			

	}

	return TRUE;
}

BOOL CRmsCtrl::RMS_ProcSetBinList(void)
{
	PRMS_SETUP_WRITE_DATA	pSetupWrite;

	BYTE*	pSetupData;
	int		nSavedData = 0, nRecvData = 0;
	
	NHDEBUG(1, (_T("RMS_ProcSetBinList() : WRITE BINLIST\n")));

	// 1. Recv Setup Data.
	memset(gpRMSDataBuffer, 0, RMS_DATA_BUFSIZE);
	pSetupData = gpRMSDataBuffer;
	
	while(1)
	{
		NHDEBUG(1, (_T("Wait RMS_SET_BINLIST Data. FROM RMS\n")));
		
		memset(gpRMSBuffer, 0, RMS_BUFSIZE);
		nRecvData = RMS_BUFSIZE;
		if (RMS_Recv(gpRMSBuffer, &nRecvData, 0) != RMS_OK)
		{
			NHDEBUG(1, (_T("RMS_Recv() Failed\n")));
			goto ErrorReturn;
		}
		
		// Verify.
		if (!RMS_VerifyMsg(gpRMSBuffer, nRecvData))
		{
			NHDEBUG(1, (_T("RMS_VerifyMsg() Failed\n")));
			goto ErrorReturn;
		}
		// OK PASS
		RMS_SendControl(DATA_ACK);
		
		// Setup Data is 0
		if (nRecvData < (LEN_RMS_SETUP_WRITE_DATA - sizeof(pSetupWrite->SetupData)))
		{
			NHDEBUG(1, (_T("RMS_SET_BINLIST RecvData is Invalid\n")));
			goto ErrorReturn;
		}
		
		pSetupWrite = (PRMS_SETUP_WRITE_DATA)gpRMSBuffer;
		
		// Setup Data Length.
		nRecvData = pSetupWrite->Header.Length - 27;
		
		// check buffer reamin size.
		if ((nSavedData + nRecvData) > RMS_DATA_BUFSIZE)
		{
			NHDEBUG(1, (_T("RMS_SET_BINLIST Save Buffer Full. cur(%d)\n"), (nSavedData + nRecvData)));
			goto ErrorReturn;
		}
		
		// saved..
		memcpy((pSetupData+nSavedData), pSetupWrite->SetupData, nRecvData);
		
		NHDEBUG(1, (_T("RECVED RMS_SET_BINLIST. FRAME(%d)\n"), pSetupWrite->FrameNumber));
		
		nSavedData += nRecvData;
		
		// It's Last Frame.
		if ((pSetupWrite->FrameNumber & 0x1000) == 0x1000)
			break;
	}
	
	// Wait EOT..
	memset(gpRMSBuffer, 0, RMS_BUFSIZE);
	nRecvData = RMS_BUFSIZE;
	if (RMS_Recv(gpRMSBuffer, &nRecvData, 0) != RMS_OK)
	{
		NHDEBUG(1, (_T("Wait EOT. RMS_Recv() Failed\n")));
		goto ErrorReturn;
	}

	if (nRecvData != 1 || gpRMSBuffer[0] != DATA_EOT)
	{
		NHDEBUG(1, (_T("RECV Data. But Not EOT\n")));
		goto ErrorReturn;
	}

	NHDEBUG(1, (_T("Write RMS_SET_BINLIST Data\n")));

#if SUPPORT_CHANGE_PARAMETER_JNL		//[#610] SOOK 2010.01.14 Configuration 저널 저장 
		m_pDevCmn->SetPreviousParameter();	
#endif		//end of [#610]
	// Adjust SetupData
	RMS_WriteBinListMsg(pSetupData, nSavedData);

#if SUPPORT_CHANGE_PARAMETER_JNL		//[#610] SOOK 2010.01.14 Configuration 저널 저장 
		m_pDevCmn->SaveChangeParameter(CHANGE_USER_MONIVIEW);	
#endif		//end of [#610]

	
ErrorReturn:

	return TRUE;
}
#endif
//end of [#546]

void CRmsCtrl::RMS_WideToMulti(LPCTSTR pWideChar, char *pMultiChar, int nLenMultiChar)
{
	memset(pMultiChar, 0, nLenMultiChar);
	WideCharToMultiByte(RMS_CODEPAGE, 0, pWideChar, -1, pMultiChar, nLenMultiChar, NULL, NULL);
}

void CRmsCtrl::RMS_MultiToWide(char *pMultiChar, LPWSTR pWideChar, int nLenWideChar)
{
	memset(pWideChar, 0, nLenWideChar);
	MultiByteToWideChar(RMS_CODEPAGE, 0, pMultiChar, -1, pWideChar, nLenWideChar);
}

BYTE CRmsCtrl::RMS_EncryptXOR(BYTE *pData, int nLen)
{
	BYTE	nStartTableIndex, nTableIndex;

	nStartTableIndex = rand() % XOR_TBL_SIZE;
	nTableIndex = nStartTableIndex;

	for (int i = 0; i < nLen; i++)
	{
		nTableIndex = (nTableIndex % XOR_TBL_SIZE);
		pData[i] ^= XOR_TBL[nTableIndex++];
	}

	return nStartTableIndex;
}

void CRmsCtrl::RMS_DecryptXOR(BYTE nTableIndex, BYTE *pData, int nLen)
{
	for (int i = 0; i < nLen; i++)
	{
		nTableIndex = (nTableIndex % XOR_TBL_SIZE);
		pData[i] ^= XOR_TBL[nTableIndex++];
	}
}

//[#635] SOOK 2010.04.01 호주 APCA 인증 대응 Download File Authentication 
#if (AU_VERSION)
/*-------------------------------------------------------------------
 CLASS    NAME: CRmsCtrl
 FUNCTION NAME: RMS_CheckFileHashValue()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : MoniView에서 받은 Hash 값과 수신된 파일에서 생성한 값과 검증한다. 
-------------------------------------------------------------------*/
BOOL CRmsCtrl::RMS_CheckFileHashValue(LPCTSTR pFileName, BYTE *pHashValue, WORD wHashLength, BYTE *pCertiValue, WORD wCertiLen)
{
	//SOOK 2010.04.30 
	// 인증서 파일이 없는 경우 인증서 파일을 서버에서 받아서 저장하고 처음 Verify하면 실패 남 
	// 원인 불명으로 인증서가 없을 경우에는 Verify는 스킵한다.(디버깅 후 아래부분 제거함)
	// 아래처럼 MoniView에서 받은 인증서롤 메모리로 해봐도 실패함. 
	if ( pCertiValue != NULL)
	{
		return TRUE;
	}//end of [SOOK 2010.04.30]

	HANDLE hFile = ::CreateFile( 
		/*(const unsigned short *)*/pFileName,
		GENERIC_READ, 
		FILE_SHARE_READ, 
		NULL, 
		OPEN_EXISTING, 
		FILE_ATTRIBUTE_READONLY,
		NULL);
	if ( hFile == INVALID_HANDLE_VALUE)
	{
		NHDEBUG(1, (_T("FAILED TO CREATE RECEIVED FILE\n")));
		return FALSE;
	}

	DWORD dwFileSizeHigh = 0;
	DWORD dwTotalFileSize = ::GetFileSize(hFile, &dwFileSizeHigh);

	LPBYTE lpFileBuffer = new BYTE [dwTotalFileSize +1];
	
	if ( FALSE == ReadFile(hFile, lpFileBuffer, dwTotalFileSize, &dwFileSizeHigh, NULL))
	{
		NHDEBUG(1, (_T("FAILED TO READ RECEIVED FILE\n")));
		CloseHandle(hFile);
// 		delete lpFileBuffer;
		delete [] lpFileBuffer;	// Codesonar 지적
		return FALSE;
	}
	lpFileBuffer[dwTotalFileSize] = 0x00;

	// FIle Hash 값 체크 
	BOOL bValid = FALSE;

	PCCERT_CONTEXT	pCertContext = NULL;
	HCRYPTPROV		hProv = NULL;
	HCRYPTHASH		hHash = NULL;
	HCRYPTKEY		hPubKey = NULL;

	LPBYTE lpCertFileBuffer = NULL;
	DWORD dwTotalCertiFileSize = 0;
	DWORD dwCertFileSizeHigh = 0;
	HANDLE 	 hCertificateFile = NULL;
NHDEBUG(1, (_T("READ DOWNLOADED FILE\n")));

	if ( pCertiValue == NULL)
	{
NHDEBUG(1, (_T("OPEN CERTIFICATE FILE\n")));
		hCertificateFile = CreateFile(
			 L"\\ATM\\MoniView.der",                  // the file name
			 GENERIC_READ,					// access mode: // read from and write to this file
			 FILE_SHARE_READ,                            // share mode
			 NULL,                         // security 
			 OPEN_EXISTING,                // how to create
			 FILE_ATTRIBUTE_NORMAL,        // file attributes
			 NULL);                        // template
		if (hCertificateFile == INVALID_HANDLE_VALUE) 
		{ 
			NHDEBUG(1, (_T("FAILED TO CREATE CERTIFICATE FILE\n")));
			CloseHandle(hFile);
// 	 		delete lpFileBuffer;
			delete [] lpFileBuffer;	// Codesonar 지적
			return FALSE;
		}

		dwTotalCertiFileSize = ::GetFileSize(hCertificateFile, &dwCertFileSizeHigh);
		lpCertFileBuffer = new BYTE[dwTotalCertiFileSize+1];
		if ( FALSE == ReadFile(hCertificateFile, lpCertFileBuffer, dwTotalCertiFileSize, &dwCertFileSizeHigh, NULL))
		{
			NHDEBUG(1, (_T("FAILED TO READ CERTIFICATE FILE\n")));
			if ( hCertificateFile ) CloseHandle(hCertificateFile);
			if ( lpFileBuffer) delete [] lpCertFileBuffer;	// CodeSonar 지적 (lpCertFileBuffer -> [] lpCertFileBuffer)
			CloseHandle(hFile);
			delete [] lpFileBuffer;	// Codesonar 지적 (lpFileBuffer -> [] lpFileBuffer)
			return FALSE;
		}
	}
	else
	{
NHDEBUG(1, (_T("WITHOUT CERTIFICATE FILE\n")));
		dwTotalCertiFileSize = wCertiLen;
		lpCertFileBuffer = new BYTE[dwTotalCertiFileSize+1];
		memcpy(lpCertFileBuffer, pCertiValue, dwTotalCertiFileSize);
	}
	lpCertFileBuffer[dwTotalCertiFileSize] = 0x00;
	NHDEBUG(1, (_T("SUCCEED TO READ CERTIFICATE FILE, dwTotalCertiFileSize=%d,dwCertFileSizeHigh=%d\n"),dwTotalCertiFileSize,dwCertFileSizeHigh ));
	
	pCertContext = CertCreateCertificateContext(
		X509_ASN_ENCODING,
		lpCertFileBuffer, 
		dwTotalCertiFileSize);
	if ( pCertContext == NULL)
	{
		NHDEBUG(1, (_T("FAILED TO CertCreateCertificateContext: 0x%x\n"),pCertContext ));
		if ( hCertificateFile ) CloseHandle(hCertificateFile);
		if ( lpFileBuffer) delete [] lpCertFileBuffer;	// CodeSonar 지적 (lpCertFileBuffer -> [] lpCertFileBuffer)
		CloseHandle(hFile);
// 		delete lpFileBuffer;
		delete [] lpFileBuffer;	// Codesonar 지적
		return FALSE;
	}

NHDEBUG(1, (_T("CertCreateCertificateContext\n")));
	if((CryptAcquireContext(
		&hProv, 
		NULL,
		NULL,
		PROV_RSA_FULL,
		CRYPT_VERIFYCONTEXT
		)))
	{
NHDEBUG(1, (_T("CryptAcquireContext\n")));
		if((CryptCreateHash(
			hProv,
			CALG_SHA1,
			0,
			0,
			&hHash
			)))
		{
NHDEBUG(1, (_T("CryptCreateHash\n")));
			if((CryptHashData(hHash, 
				lpFileBuffer, // message (?)  // password
				dwTotalFileSize, // length				// password length
				0
				)))
			{
NHDEBUG(1, (_T("CryptHashData\n")));
				// Get Public Key
				if((CryptImportPublicKeyInfo( 
					hProv, 
					X509_ASN_ENCODING,
					&pCertContext->pCertInfo->SubjectPublicKeyInfo,
					&hPubKey
					)))
				{
NHDEBUG(1, (_T("CryptImportPublicKeyInfo in RMS_CheckFileHashValue,Algorithm:%S\n"),pCertContext->pCertInfo->SubjectPublicKeyInfo.Algorithm.pszObjId ));
NHDEBUG(1, (_T("CryptImportPublicKeyInfo in RMS_CheckFileHashValue,SignatureAlgorithm:%S\n"),pCertContext->pCertInfo->SignatureAlgorithm.pszObjId  ));

					// Verify the Signature
					if((CryptVerifySignature(
						hHash,
						pHashValue,
						wHashLength,
						hPubKey,
						NULL,
						0
						)))
					{
NHDEBUG(1, (_T("CryptVerifySignature\n")));
						bValid = TRUE;
					}
					else
					{
						int rv = GetLastError();
						NHDEBUG(1, (_T("FAILED TO CryptVerifySignature, Ret = 0x%0X \n"), rv));
					}
				}
				else
					NHDEBUG(1, (_T("FAILED TO CryptImportPublicKeyInfo\n")));
			}
			else
				NHDEBUG(1, (_T("FAILED TO CryptHashData\n")));
		}
		else
			NHDEBUG(1, (_T("FAILED TO CryptCreateHash\n")));
	}
	else
		NHDEBUG(1, (_T("FAILED TO CryptAcquireContext\n")));

	CloseHandle(hFile);
// 	delete lpFileBuffer;
	delete [] lpFileBuffer;	// Codesonar 지적
	if ( hCertificateFile)
		CloseHandle(hCertificateFile);
	if ( lpCertFileBuffer)
		delete [] lpCertFileBuffer;	// CodeSonar 지적 (lpCertFileBuffer -> [] lpCertFileBuffer)


	if ( pCertContext)
		CertFreeCertificateContext(pCertContext);
	if(hHash)
		CryptDestroyHash(hHash);
	if(hProv)
		CryptReleaseContext(hProv,0);
	return bValid;
}
/*-------------------------------------------------------------------
 CLASS    NAME: CRmsCtrl
 FUNCTION NAME: RMS_SaveCertificateFile()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : MoniView에서 받은 인증서 파일을 저장한다. 
-------------------------------------------------------------------*/
BOOL CRmsCtrl::RMS_SaveCertificateFile(BYTE *pFileName, BYTE *pCertiValue, WORD wCertiLen)
{
NHDEBUG(1, (_T("RMS_SaveCertificateFile starts\n")));
	CString strTempName;
	strTempName.Format(L"%s", pFileName );

	// CodeSonar 지적사항 대책
	HANDLE hFile = INVALID_HANDLE_VALUE;
	hFile = CreateFile (///*(const unsigned short *)*/pFileName,				// Open NVRAM.TXT.
							strTempName,
							GENERIC_WRITE,          // Open for writing
							0,                      // Do not share
							NULL,                   // No security
							CREATE_ALWAYS,          // Open or create
							FILE_ATTRIBUTE_NORMAL,  // Normal file
							NULL);                  // No template file
	if (hFile == INVALID_HANDLE_VALUE)
	{
		NHDEBUG(1, (_T("FILE OPEN FAILED(%s)\n"), pFileName));
		return FALSE;
	}

	DWORD nWriten = 0;

	if (!WriteFile(hFile, pCertiValue, wCertiLen, &nWriten, NULL))
	{
		CloseHandle(hFile);
		NHDEBUG(1, (_T("File Write Failed\n")));
		return FALSE;
	}

	CloseHandle(hFile);

	NHDEBUG(1, (_T("RMS_SaveCertificateFile ends\n")));
	return TRUE;
}

#endif
//end of [#635]
