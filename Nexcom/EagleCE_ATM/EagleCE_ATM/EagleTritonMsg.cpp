/** **************************************************
*	@file		EagleTritonMsg.cpp
*	@brief		Triton Message Process Class, base on Triton Spec 5.22
*	@date		2017-08-23
*	@version	1.0.1
*****************************************************/
#include "stdafx.h"
#include "EagleTritonMsg.h"
#include "EagleSVCLib.h"
#include "SVC_Manager.h"

#include "../../EagleCE_Framework/EagleCE_Framework/EagleDataManager.h"
#include "../../EagleCE_Framework/EagleCE_Framework/Util.h"
#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"

#include "../../EagleCE_Screen/EagleCE_Screen/SCR_Manager.h"


// 인스턴스
CEagleTritonMsg* CEagleTritonMsg::m_pInstance = NULL;

/** **********************************************************
*	@brief		인스턴스 생성
    
*	@retval		없음
************************************************************/
void CEagleTritonMsg::CreateInstance()
{
	m_pInstance = new CEagleTritonMsg();

}


/** **********************************************************
*	@brief		인스턴스 취득
*	@retval		없음
************************************************************/
CEagleTritonMsg* CEagleTritonMsg::GetInstance()
{
	return m_pInstance;
}


/** **********************************************************
*	@brief		인스턴스 해제
*	@retval		없음
************************************************************/
void CEagleTritonMsg::ReleaseInstance()
{
	if (m_pInstance)
	{
		delete m_pInstance;
	}
}


/** **********************************************************
*	@brief		생성자
************************************************************/
CEagleTritonMsg::CEagleTritonMsg()
{
	m_pSendBuffer = NULL;
	m_pSendBuffer = new BYTE[NET_SEND_BUFF_SIZE];

	m_pRecvBuffer = NULL;
	m_pRecvBuffer = new BYTE[NET_RECV_BUFF_SIZE];
}


/** **********************************************************
*	@brief		소멸자
************************************************************/
CEagleTritonMsg::~CEagleTritonMsg()
{
	if (m_pSendBuffer != NULL)
		delete [] m_pSendBuffer;

	if (m_pRecvBuffer != NULL)
		delete [] m_pRecvBuffer;
}


/** **********************************************************
*	@brief		초기화 Triton Communication Header Informations
************************************************************/
void TRITON_COMMHEADERS::Clear(void)
{
	m_strCommsHeaderID_8.Empty();
	m_strTerminalID_2.Empty();
	m_strSoftwareVersionNo_2.Empty();
	m_strEncryptionModeFlag_1.Empty();
	m_strInformationHeader_7.Empty();
	m_strTerminalID_15.Empty();
}



/** **********************************************************
*	@brief		초기화 Triton Transaction Request Informations
************************************************************/
void TRITON_TRANREQMSG::Clear(void)
{
	m_strTransactionCode_2.Empty();
	m_strSequenceNumber_4.Empty();
	m_strTrack2_37.Empty();
	m_strAmount1_8.Empty();	   
	m_strAmount2_8.Empty();    
	m_strPINBlock_16.Empty();
	m_strMiscellaneous1.Empty();
	m_strMiscellaneous2.Empty();
	m_strStatusMonintoring.Empty();
	m_strMiscellaneous3.Empty();
}


/** **********************************************************
*	@brief		초기화 Triton Transaction Response Informations
************************************************************/
void TRITON_TRANRESPMSG::Clear(void)
{
	m_strInformation_Header_12.Empty();
	m_strMulti_Block_Indicator_1.Empty();
	m_strTerminalID_15.Empty();
	m_strTransactionCode_2.Empty();
	m_strSequenceNumber_4.Empty();
	m_strResponseCode_3.Empty();
	m_strAuthorizationNum_8.Empty();
	m_strTransactionDate_6.Empty();
	m_strTransactionTime_6.Empty();
	m_strBusinessDate_6.Empty();
	m_strAmount1_8.Empty();
	m_strAmount2_8.Empty();
	m_strAvailableBalance_8.Empty();
	m_strAmount1_Ext12.Empty();
	m_strEMVTag.Empty();
}


/** **********************************************************
*	@brief		초기화 Triton Reversal Request Informations
************************************************************/
void TRITON_REVERSALREQMSG::Clear(void)
{
	m_strTransactionCode_2.Empty();
	m_strSequenceNumber_4.Empty();
	m_strTrack2_37.Empty();
	m_strAmount1_8.Empty();
	m_strAmount2_8.Empty();
	m_strAmount3_8.Empty();
	m_strStatusMonintoring.Empty();
	m_strMiscellaneousX_V.Empty();
}


/** **********************************************************
*	@brief		초기화 Triton Reversal Response Informations
************************************************************/
void TRITON_REVERSALRESPMSG::Clear(void)
{
	m_strInformation_Header_12.Empty();
	m_strTerminalID_15.Empty();
	m_strTransactionCode_2.Empty();
	m_strSequenceNumber_4.Empty();
	m_strResponseCode_3.Empty();
}


/** **********************************************************
*	@brief		초기화 Triton Config Request Informations
************************************************************/
void TRITON_CONFIGREQMSG::Clear(void)
{
	m_strTransactionCode_2.Empty();
	m_strStatusMonitoringField_X.Empty();
	m_strMiscellaneousX_V.Empty();
}


/** **********************************************************
*	@brief		초기화 Triton Config Response Informations
************************************************************/
void TRITON_CONFIGRESPMSG::Clear(void)
{
	m_strInformation_Header_12.Empty();
	m_strTerminalID_15.Empty();
	m_strTransactionCode_2.Empty();
	m_strFieldIDCode1_1.Empty();
	m_strEncryptedPINKey_16.Empty();
	m_strFieldIDCode2_1.Empty();
	m_strEncryptedPINKey2_16.Empty();
	m_strFieldIDCode3_1.Empty();
	m_strSurchargeAmount_8.Empty();
	m_strFieldIDCode4_1.Empty();
	m_strEncryptedPINKey4_16.Empty();
	m_strFieldIDCode5_1.Empty();
	m_strEncryptedPINKey5_16.Empty();
}


/** **********************************************************
*	@brief		초기화 Triton Host Total Request Informations
************************************************************/
void TRITON_HOSTTOTALSREQMSG::Clear(void)
{
	m_strTransactionCode_2.Empty();
	m_strStatusMonintoring.Empty();
	m_strTotalWithdrawals_4.Empty();
	m_strTotalInquiries_4.Empty();
	m_strTotalTransfers_4.Empty();
	m_strSettlement_12.Empty();
	m_strMiscellaneousX_V.Empty();
}


/** **********************************************************
*	@brief		초기화 Triton Host Total Response Informations
************************************************************/
void TRITON_HOSTTOTALSRESPMSG::Clear(void)
{
	m_strInformation_Header_12.Empty();
	m_strTerminalID_15.Empty();
	m_strTransactionCode_2.Empty();
	m_strBusinessDate_6.Empty();
	m_strNumberOfWithdrawals_4.Empty();
	m_strNumberOfInquiries_4.Empty();
	m_strNumberOfTransfers_4.Empty();
	m_strSettlement_8.Empty();
	m_strSettlement_Ext12.Empty();
}


/** **********************************************************
*	@brief		초기화 Triton Status Monitoring Informations
************************************************************/
void TRITON_STATUS_MONITOR::Clear(void)
{
	m_strProgramVersionNo_10.Empty();
	m_strTableVersionNo_10.Empty();
	m_strFirmwareVersionNo_10.Empty();
	m_strAlarm_ChestDoorOpen_1.Empty();
	m_strAlarm_TopDoorOpen_1.Empty();
	m_strAlarm_SupervisonActive_1.Empty();
	m_strReceiptPrinter_PaperStatus_1.Empty();
	m_strReceiptPrinter_RibbonStatus_1.Empty();
	m_strJournalPrinter_PaperStatus_1.Empty();
	m_strJournalPrinter_RibbonStatus_1.Empty();
	m_strNoteStatus_Dispenser_1.Empty();
	m_strReceiptPrinter_1.Empty();
	m_strJournalPrinter_1.Empty();
	m_strDispenser_1.Empty();
	m_strCommunicationsSystem_1.Empty();
	m_strCardReader_1.Empty();
	m_strCardsRetained_3.Empty();
	m_strElectronicsSystem_2.Empty();
	m_strCurrentErrorCode_3.Empty();
	m_strCommunicatioinsFailures_3.Empty();
	m_strCassetteA_Denomination_3.Empty();
	m_strCassetteA_NotesLoaded_4.Empty();
	m_strCassetteA_NotesDispensed_4.Empty();
	m_strCassetteA_RejectEvents_3.Empty();
	m_strCassetteB_Denomination_3.Empty();
	m_strCassetteB_NotesLoaded_4.Empty();
	m_strCassetteB_NotesDispensed_4.Empty();
	m_strCassetteB_RejectEvents_3.Empty();
	m_strCassetteC_Denomination_3.Empty();
	m_strCassetteC_NotesLoaded_4.Empty();
	m_strCassetteC_NotesDispensed_4.Empty();
	m_strCassetteC_RejectEvents_3.Empty();
	m_strCassetteD_Denomination_3.Empty();
	m_strCassetteD_NotesLoaded_4.Empty();
	m_strCassetteD_NotesDispensed_4.Empty();
	m_strCassetteD_RejectEvents_3.Empty();
	m_strTotalNotesPurged_3.Empty();
}


/** **********************************************************
*	@brief		초기화 Triton Message Buffer
************************************************************/
void CEagleTritonMsg::InitializeVariable()
{
	if (m_pSendBuffer != NULL)
		memset(m_pSendBuffer, 0, NET_SEND_BUFF_SIZE);

	if (m_pRecvBuffer != NULL)
		memset(m_pRecvBuffer, 0, NET_RECV_BUFF_SIZE);

	m_strSendData.Empty();
	m_nSendLen = 0;

	m_strRecvData.Empty();
	m_nRecvLen = 0;

	m_strCrcResult_16.Empty();
	m_strArrRecvData.RemoveAll();

	m_sTriton_CommHeader.Clear();
	m_sTriton_Status_Field.Clear();

	m_sTriton_TranReq.Clear();
	m_sTriton_TranResp.Clear();

	m_sTriton_ReversalReq.Clear();
	m_sTriton_ReversalResp.Clear();

	m_sTriton_ConfigReq.Clear();
	m_sTriton_ConfigResp.Clear();

	m_sTriton_TotalReq.Clear();
	m_sTriton_TotalResp.Clear();

	CEagleDataManager::GetInstance()->m_Client_Info.m_bRecvConfigInfo = FALSE;
}

/** **********************************************************
*	@brief		송신 전문을 생성하는 함수 
*	@param		nType : 거래 종류
*	@return		Send Length
************************************************************/
int CEagleTritonMsg::Triton_MakeHostMsg(int nTranType)
{
	InitializeVariable();

	////////////////////////////////////////////
	// 1. Make Message
	Triton_MakeHeader();					// Send Triton Host Make Header
	Triton_MakeBody(nTranType);				// Send Triton Host Make Message

	if(CEagleDataManager::GetInstance()->m_Config.m_Host.strCrc_flg == S_ENABLE)			// CRC option is Enabled
	{
		// CRC check
		CString strCRCDefault;
		int retCrcKeyFind = -1;

		strCRCDefault.Format(_T("ub0000%c"), FS);	
		retCrcKeyFind = m_strSendData.Find(strCRCDefault);			

		if(retCrcKeyFind != -1)
		{
			char temp_snddata[NET_SEND_BUFF_SIZE] = { 0, };

			sprintf(temp_snddata, "%S", m_strSendData);
			WORD tempLen = strlen(temp_snddata);

			// STANDARD TCP/IP 프로토콜은 Length 2바이트를 앞에 붙여서 CRC 계산
			if (CEagleDataManager::GetInstance()->m_Config.m_Host.strProtocolType == _T("STANDARD"))
			{
				memmove(&temp_snddata[2], &temp_snddata[0], tempLen);
				temp_snddata[0] = (tempLen& 0xFF00) >> 8;
				temp_snddata[1] = tempLen & 0x00FF;
				tempLen += 2;
			}

			UINT crc = CUtil::crc16((BYTE*)temp_snddata, tempLen);

			CString strTemp;

			strTemp.Format(_T("ub%04X%c"), crc, FS);
			m_strSendData.Replace(strCRCDefault, strTemp);
		}
	}

	////////////////////////////////////////////
	// 3. Make Buffer
	m_nSendLen = __min( NET_SEND_BUFF_SIZE, m_strSendData.GetLength() );
	sprintf((char*)m_pSendBuffer, "%S", m_strSendData);

	// Track2 and PIN Block Masking
	CString strMaskingData;
	strMaskingData = m_strSendData;
	if (nTranType == EAGLE_TRAN_CODE_WITHDRAWAL || nTranType == EAGLE_TRAN_CODE_REVERSAL || nTranType == EAGLE_TRAN_CODE_TRANSFER || nTranType == EAGLE_TRAN_CODE_BALANCE)
	{
		if (CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTrack2Data.IsEmpty() == FALSE)
			strMaskingData.Replace(CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTrack2Data, _T("XXXXXXXXXX"));

		if (CEagleDataManager::GetInstance()->m_Client_Info.m_strPinBlock.IsEmpty() == FALSE)
			strMaskingData.Replace(CEagleDataManager::GetInstance()->m_Client_Info.m_strPinBlock, _T("XXXXXXXXXXXXXXXX"));

		// EMV Data중 57 tag 와 5A Masking 처리 추가
		if (CEagleDataManager::GetInstance()->m_Client_Info.m_strTag57Data.IsEmpty() == FALSE)
			strMaskingData.Replace(CEagleDataManager::GetInstance()->m_Client_Info.m_strTag57Data, _T("XXXXXXXXXXXXXXXX"));

		if (CEagleDataManager::GetInstance()->m_Client_Info.m_strTag5AData.IsEmpty() == FALSE)
			strMaskingData.Replace(CEagleDataManager::GetInstance()->m_Client_Info.m_strTag5AData, _T("XXXXXXXXXXXX"));

	}

	LOG(Info, _T("------------------------SEND DATA-----------------------"));
	CEagleSVCLib::GetInstance()->StringHexaDump(strMaskingData);
	LOG(Info, _T("--------------------------------------------------------"));

	return m_nSendLen;
}

/** **********************************************************
*	@brief		송신 Header Data를 생성하는 함수
*	@param		None
*	@return		0
************************************************************/
int CEagleTritonMsg::Triton_MakeHeader()
{
	CString strTmp;

	if (CEagleDataManager::GetInstance()->m_Config.m_Host.strCommunication_header == S_ENABLE)	// OP에 설정된 Communication Header 정보 참조 필요 
	{
		// 1. Communications Identifier 8 ASCII
		m_sTriton_CommHeader.m_strCommsHeaderID_8 = CEagleDataManager::GetInstance()->m_Config.m_Host.strCommunicationID;	// OP에 설정된 값
		m_sTriton_CommHeader.m_strCommsHeaderID_8 += SPACE_8;
		m_sTriton_CommHeader.m_strCommsHeaderID_8 = m_sTriton_CommHeader.m_strCommsHeaderID_8.Left(8);

		// 2. Terminal Identifier 2 ACSII
		m_sTriton_CommHeader.m_strTerminalID_2 = _T("td");			// Fixed Field

		// 3. Software Version Number 2자리
		m_sTriton_CommHeader.m_strSoftwareVersionNo_2 = _T("01");	// AP SW Version 기입 필요

		// 4. Encryption Mode Flag (Fixed : Triple Des)
		m_sTriton_CommHeader.m_strEncryptionModeFlag_1 = _T("2");

		// 5. Information Header 7 ACSII , Unused Must be space filled.
		m_sTriton_CommHeader.m_strInformationHeader_7 = SPACE_7;

		m_strSendData = m_sTriton_CommHeader.m_strCommsHeaderID_8 +
						m_sTriton_CommHeader.m_strTerminalID_2 +
						m_sTriton_CommHeader.m_strSoftwareVersionNo_2 +
						m_sTriton_CommHeader.m_strEncryptionModeFlag_1 +
						m_sTriton_CommHeader.m_strInformationHeader_7;
		m_strSendData += FS;
	}

	m_sTriton_CommHeader.m_strTerminalID_15 = CEagleDataManager::GetInstance()->m_Config.m_Option.strTerminal_ID;
	m_sTriton_CommHeader.m_strTerminalID_15 += SPACE_15;
	m_sTriton_CommHeader.m_strTerminalID_15 = m_sTriton_CommHeader.m_strTerminalID_15.Left(15);

	m_strSendData += m_sTriton_CommHeader.m_strTerminalID_15;
	m_strSendData += FS;

	return 0;
}

/** **********************************************************
*	@brief		Triton 거래별 Data 생성
*	@param		없음
*	@return		0
************************************************************/
int CEagleTritonMsg::Triton_MakeBody(int nTranType)
{
	switch(nTranType)	// Transaction Type 참고해야함
	{
	case EAGLE_TRAN_CODE_WITHDRAWAL:
	case EAGLE_TRAN_CODE_TRANSFER:
	case EAGLE_TRAN_CODE_BALANCE:
		Triton_MakeTransMsg(nTranType);
		break;

	case EAGLE_TRAN_CODE_REVERSAL:
		Triton_MakeReversalMsg();
		break;

	case EAGLE_TRAN_CODE_HOST_TOTALS:
	case EAGLE_TRAN_CODE_GET_HOST_TOTALS:
		Triton_MakeTotalMsg(nTranType);
		break;

	case EAGLE_TRAN_CODE_DOWNLOAD_CONFIG:
		Triton_MakeConfigMsg();
		break;

	default:
		break;
	}

	return 0;
}

/** **********************************************************
*	@brief		Triton 거래(출금, 조회, 이체)별 Data 생성
*	@param		없음
*	@return		0
************************************************************/
int CEagleTritonMsg::Triton_MakeTransMsg(int nTranType)
{
	BYTE			szTrancode[3] = { 0, };
	int				nAmount = 0;
	CString			strTemp;

	switch (nTranType)
	{
	case EAGLE_TRAN_CODE_WITHDRAWAL:
		szTrancode[0] = '1';
		break;

	case EAGLE_TRAN_CODE_TRANSFER:
		szTrancode[0] = '2';
		break;

	case EAGLE_TRAN_CODE_BALANCE:
		szTrancode[0] = '3';
		break;

	default:
		break;
	}

	// Source Account ; Account 정보 참고해야함.
	if (CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strFromAccountType == S_CHECKING)
		szTrancode[1] = '1';
	else if (CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strFromAccountType == S_SAVINGS)
		szTrancode[1] = '2';
	else
		szTrancode[1] = '5';

	m_sTriton_TranReq.m_strTransactionCode_2.Format(_T("%2S"), szTrancode);
	m_strSendData += m_sTriton_TranReq.m_strTransactionCode_2;
	m_strSendData += FS;

	// Sequence Number	
	strTemp.Format(_T("%s%s"), ZERO_4, CEagleDataManager::GetInstance()->m_Config.m_System.strSequence_Number);
	m_sTriton_TranReq.m_strSequenceNumber_4 = strTemp.Right(4);
	m_strSendData += m_sTriton_TranReq.m_strSequenceNumber_4;
	m_strSendData += FS;

	// ISO CARD Track2 37 Numeric ( Variable ) ; AP에서 Setting한 정보 참고
	m_sTriton_TranReq.m_strTrack2_37.Format(_T("%s"), CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTrack2Data);
	m_strSendData += m_sTriton_TranReq.m_strTrack2_37.Right(37);
	m_strSendData += FS;

	// Amount 1  8Numeric ( Withdraw or transfer amount )	; AP에서 Setting한 정보 참고
	if (nTranType == EAGLE_TRAN_CODE_TRANSFER)
	{
		strTemp.Format(_T("%s%s"), ZERO_8, CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransferAmount);
		m_sTriton_TranReq.m_strAmount1_8 = strTemp.Right(8);
	}
	// cent 포함된 금액
	else
	{
		strTemp.Format(_T("%s%s"), ZERO_8, CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strRequestAmount + _T("00"));		// cent 포함된 금액
		m_sTriton_TranReq.m_strAmount1_8 = strTemp.Right(8);
	}

	m_strSendData += m_sTriton_TranReq.m_strAmount1_8;
	m_strSendData += FS;

	// Amount 2  8Numeric ( Surcharge amount as set by terminal. )
	strTemp.Format(_T("%s%s"), ZERO_8, CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strSurchargeAmount); // cent 포함된 금액
	m_sTriton_TranReq.m_strAmount2_8 = strTemp.Right(8);
	m_strSendData += m_sTriton_TranReq.m_strAmount2_8;
	m_strSendData += FS;

	/* Get PIN Block */
#if (NETWORK_OFFLINE_MODE)
	CEagleDataManager::GetInstance()->m_Client_Info.m_strPinBlock = _T("1234567890123456");
#else
	CEagleDataManager::GetInstance()->m_Client_Info.m_strPinBlock = CDEV_Manager::GetInstance()->m_DEV_CREPP.MakePinBlock(CEagleDataManager::GetInstance()->m_Client_Info.m_strAccountNo);

	LOG(Info, _T("Make Pin Block Length Information (%d)"), CEagleDataManager::GetInstance()->m_Client_Info.m_strPinBlock.GetLength());
#endif

	// PIN Block  16 Alphanumeric	(Variable)
	m_sTriton_TranReq.m_strPINBlock_16.Format(_T("%16.16s"), CEagleDataManager::GetInstance()->m_Client_Info.m_strPinBlock);
	m_strSendData += m_sTriton_TranReq.m_strPINBlock_16.Right(16);
	m_strSendData += FS;

	// Miscellaneous 1 -> Not USed (User Definition Filed)
	m_strSendData += FS;

	// Miscellaneous 2 -> Not USed (User Definition Filed)
	m_strSendData += FS;

	// Status Monitoring Field	;OP에서 설정된 정보 참고
	if (CEagleDataManager::GetInstance()->m_Config.m_Host.strAtm_status_monitoring == S_ENABLE)
	{
		Triton_Make_StatusMonitoringField();
		m_strSendData += FS;
	}

	// Make MiscellaneousField
	Triton_Make_MiscellaneousField(nTranType);

	return 0;
}

/** **********************************************************
*	@brief		Triton Reversal Data 생성
*	@param		None
*	@return		0
************************************************************/
int CEagleTritonMsg::Triton_MakeReversalMsg()
{
	CString		strTemp;
	int			nAmount = 0;

	//// Transaction Code  Reversal : 29
	m_sTriton_ReversalReq.m_strTransactionCode_2 = _T("29");
	m_strSendData += m_sTriton_ReversalReq.m_strTransactionCode_2;
	m_strSendData += FS;

	//// Sequence Number 4Numeric
	strTemp.Format(_T("%s%s"), ZERO_4, CEagleDataManager::GetInstance()->m_Config.m_System.strSequence_Number);
	m_sTriton_ReversalReq.m_strSequenceNumber_4 = strTemp.Right(4);
	m_strSendData += m_sTriton_ReversalReq.m_strSequenceNumber_4;
	m_strSendData += FS;

	//// ISO CARD Track2 37 Numeric ( Variable )
	m_sTriton_ReversalReq.m_strTrack2_37.Format(_T("%s"), CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTrack2Data);
	m_strSendData += m_sTriton_ReversalReq.m_strTrack2_37.Right(37);
	m_strSendData += FS;

	//// Amount 1  8Numeric (Requested Withdraw amount)
	strTemp.Format(_T("%s%s"), ZERO_8, CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strRequestAmount + _T("00"));		// cent 포함된 금액
	m_sTriton_ReversalReq.m_strAmount1_8 = strTemp.Right(8);
	m_strSendData += m_sTriton_ReversalReq.m_strAmount1_8;
	m_strSendData += FS;

	//// Amount 2  8Numeric (Surcharge amount as set by terminal)
	strTemp.Format(_T("%s%s"), ZERO_8, CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strSurchargeAmount); // cent 포함된 금액
	m_sTriton_ReversalReq.m_strAmount2_8 = strTemp.Right(8);
	m_strSendData += m_sTriton_ReversalReq.m_strAmount2_8;
	m_strSendData += FS;

	//// Amount 3  8Numeric (Actual Dispensed amount)
	strTemp.Format(_T("%s%s"), ZERO_8, CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strDispenseAmount + _T("00")) ;	// cent 포함된 금액
	m_sTriton_ReversalReq.m_strAmount3_8 = strTemp.Right(8);
	m_strSendData += m_sTriton_ReversalReq.m_strAmount3_8;
	m_strSendData += FS;

	//// Status Monitoring Field
	if (CEagleDataManager::GetInstance()->m_Config.m_Host.strAtm_status_monitoring == S_ENABLE)		// Power Off시 처리 조건 추가 필요
	{
		Triton_Make_StatusMonitoringField();
		m_strSendData += FS;
	}

	//// Make MiscellaneousField
	Triton_Make_MiscellaneousField(EAGLE_TRAN_CODE_REVERSAL);

	return 0;
}

/** **********************************************************
*	@brief		Triton Day / Trial Day Close Data 생성
*	@param		None
*	@return		0
************************************************************/
int CEagleTritonMsg::Triton_MakeTotalMsg(int nTranType)
{
	CString			strTemp;

	if(nTranType == EAGLE_TRAN_CODE_HOST_TOTALS)
		m_sTriton_TotalReq.m_strTransactionCode_2 = _T("51");
	else
		m_sTriton_TotalReq.m_strTransactionCode_2 = _T("50");

	m_strSendData += m_sTriton_TotalReq.m_strTransactionCode_2;
	m_strSendData += FS;

	//// Status Monitoring Field
	if (CEagleDataManager::GetInstance()->m_Config.m_Host.strAtm_status_monitoring == S_ENABLE)
	{
		Triton_Make_StatusMonitoringField();
		m_strSendData += FS;
	}

	//// Number of withdrawal 4 Numeric
	m_sTriton_TotalReq.m_strTotalWithdrawals_4 = ZERO_4;
	m_sTriton_TotalReq.m_strTotalWithdrawals_4 += CEagleDataManager::GetInstance()->m_Config.m_TotalInfo.Withdrawal_Count;
	m_strSendData += m_sTriton_TotalReq.m_strTotalWithdrawals_4.Right(4);

	//// Number of Inquiries 4 Numeric
	m_sTriton_TotalReq.m_strTotalInquiries_4 = ZERO_4;
	m_sTriton_TotalReq.m_strTotalInquiries_4 += CEagleDataManager::GetInstance()->m_Config.m_TotalInfo.BalanceInquiry_Count;
	m_strSendData += m_sTriton_TotalReq.m_strTotalInquiries_4.Right(4);

	//// Number of Transfers 4 Numeric
	m_sTriton_TotalReq.m_strTotalTransfers_4 = ZERO_4;
	m_sTriton_TotalReq.m_strTotalTransfers_4 += CEagleDataManager::GetInstance()->m_Config.m_TotalInfo.Transfer_Count;
	m_strSendData += m_sTriton_TotalReq.m_strTotalTransfers_4.Right(4);

	//// Settlement 12 Numeric ( Total amount of all withdrawals. )
	m_sTriton_TotalReq.m_strSettlement_12 = ZERO_12;
	m_sTriton_TotalReq.m_strSettlement_12 += CEagleDataManager::GetInstance()->m_Config.m_TotalInfo.Withdrawal_Amount;
	m_sTriton_TotalReq.m_strSettlement_12 += _T("00");		// Cent 추가 (누락내용 적용)
	m_strSendData += m_sTriton_TotalReq.m_strSettlement_12.Right(12);
	m_strSendData += FS;

	//// Make MiscellaneousField
	Triton_Make_MiscellaneousField(nTranType);

	return 0;
}

/** **********************************************************
*	@brief		Triton Configuration Data 생성
*	@param		None
*	@return		0
************************************************************/
int CEagleTritonMsg::Triton_MakeConfigMsg()
{
	// 7. Transaction Code : "60" Configuration Table Download Request Message
	m_sTriton_ConfigReq.m_strTransactionCode_2 = _T("60");
	m_strSendData += m_sTriton_ConfigReq.m_strTransactionCode_2;
	m_strSendData += FS;

	if (CEagleDataManager::GetInstance()->m_Config.m_Host.strAtm_status_monitoring == S_ENABLE)
	{
		Triton_Make_StatusMonitoringField();
		m_strSendData += FS;
	}

	// Make MiscellaneousField
	Triton_Make_MiscellaneousField(EAGLE_TRAN_CODE_DOWNLOAD_CONFIG);

	return 0;
}

/** **********************************************************
*	@brief		Triton Miscellaneous Data 생성
*	@param		None
*	@return		0
************************************************************/
int CEagleTritonMsg::Triton_Make_MiscellaneousField(int nTranType)
{
	BYTE chMisc = 0x00;
	int nMisc_Index=0, i=0;
	UINT nMisc_FID2 = 0, nTmp = 0;
	CString strTemp;

	switch (nTranType)
	{
		case EAGLE_TRAN_CODE_WITHDRAWAL:
		case EAGLE_TRAN_CODE_TRANSFER:
		case EAGLE_TRAN_CODE_BALANCE:
			nMisc_Index = MISC_REQ_TRAN;
			break;

		case EAGLE_TRAN_CODE_REVERSAL:
			nMisc_Index = MISC_REQ_REVERSAL;
			break;

		case EAGLE_TRAN_CODE_HOST_TOTALS:
		case EAGLE_TRAN_CODE_GET_HOST_TOTALS:
			nMisc_Index = MISC_REQ_TOTAL;
			break;

		case EAGLE_TRAN_CODE_DOWNLOAD_CONFIG:
			nMisc_Index = MISC_REQ_CONFIG;
			break;

		default:
			return 1;
	}

	for( i = 0; chMisc_Requests[nMisc_Index][i] != 0; i++)
	{
		chMisc = (BYTE)chMisc_Requests[nMisc_Index][i];
		switch( chMisc ) 
		{
		case FD_EXT_SEQ_NUM:
			break;

		// 28 Digit Account Number(i.e. Checking,Savings,etc) Left justified, padded with spaces.
		case FD_ACCNT_NUM:
			break;

		// PIN Change PIN Block. 16 alphanumeric. ANSI standard encrypted PIN block.
		case FD_CHG_PIN_BLOCK:
			break;

		// Terminal is Triple-DES capable ( 2: Terminal is Triple-DES, '[2' )
		case FD_ENCRYPT_MODE:
			// Working Key가 존재하는지 확인 후 송신하도록 로직 변경
			strTemp = CDEV_Manager::GetInstance()->m_DEV_CREPP.Get_KeyCheckValue(WORKING_KEYNAME);

			if (strTemp.IsEmpty() == FALSE)	// Pin Key가 존재하면,
				m_strSendData += _T("[2");
			else
				m_strSendData += _T("[0");

			m_strSendData += FS;
			break;

		// MAC(Message Authentication Code) Result. 9ASCII characters	^A423 34CD => Not Support
		case FD_MAC_BLCK:
			break;

		case FD_TRACK3_INFO:
			break;

		case FD_PAN_TRACK3:
			break;

		case FD_RSN_RVRSL:
			// reason for reversal 값 셋팅.
			// 1: Incorrect dispense, 2: Protocol error, 3: Lost eot, 4: Mac error, 
			// 5: Customer cancellation, 6: VAS challenge/response error, 7: ICC declined transaction
			if(CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strReasonforReversal.IsEmpty() == FALSE)
			{
				strTemp.Format(_T("n%2.2s"), CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strReasonforReversal);
				m_strSendData += strTemp;
				m_strSendData += FS;
			}
			break;

		case FD_EXT_FIELD_U: // Two character FIDs
			// Make the second FID
			nMisc_FID2 = chMisc_Requests[nMisc_Index][i];
			nMisc_FID2 &= 0xFFFFFF00;

			// 'b' 16bit CRC 4 ASCII
			if ( (nMisc_FID2 & FID2_b) == FID2_b )
			{
				// CRC 처리추가, CRC필드 default값 처리, 실제 CRC계산은 나중에한다.
				if(CEagleDataManager::GetInstance()->m_Config.m_Host.strCrc_flg == S_ENABLE)		// CRC option is Enabled
				{
					m_strSendData += _T("ub0000");
					m_strSendData += FS;
				}
			}
			
			// 'd' Encoded EMV data block ASCII characters. Variable length.
			if ( (nMisc_FID2 & FID2_d) == FID2_d )
			{
				if (CEagleDataManager::GetInstance()->m_Client_Info.m_bEMV_Transaction == TRUE)
				{
					Triton_Make_EMV_RequestData(nMisc_Index);
				}
				else
				{
					if (CDEV_Manager::GetInstance()->m_DEV_CDR.IsEMV_Enable() == TRUE)
					{
						// Fallback 거래 정보 송부 Check

						// AID mismatch인 경우 또는 Power Off Reversal인 경우 Fallback 정보 미송신
						if ((CDEV_Manager::GetInstance()->m_DEV_CDR.m_bUnknownAID == FALSE) && (CEagleDataManager::GetInstance()->m_Client_Info.m_bPowerOffFlag == FALSE))
						{
							int nIndex = 0;

							strTemp = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTrack2Data;
							nIndex = strTemp.Find(_T("="));

							if (nIndex < 0)	// "="을 못찾는 경우 미송신
								break;

							// Track2 Data 오류로 인해 미송신
							if (strTemp.GetLength() <= nIndex)
								break;

							// Card Type 판별이 불가능할 경우 Fallback 전문 미송신
							if (strTemp.GetLength() > nIndex + 5)
							{
								// MS Data Check 후 Card Type이 IC인 경우에만 Fallback 정보 송신
								if (strTemp.GetAt(nIndex + 5)  == '2' || strTemp.GetAt(nIndex + 5) == '6')
								{
									m_strSendData += _T("ud");
									m_strSendData += TRITON_EMV_FALLBACK_CODE;
									m_strSendData += FS;
								}
							}
						}
					}
				}
			}

			// EMV 거래시 'uh'필드 추가
			if ( (nMisc_FID2 & FID2_h) == FID2_h )
			{
				// Issuer Script Result는 존재하는 경우에만 송신함. (Reversal 전문시만 송신됨)
				
			}
			break;

		default:
			break;

		}
	}

	return 0;
}

/** **********************************************************
*	@brief		Triton Status Monitoring Data 생성
*	@param		None
*	@return		0
************************************************************/
int CEagleTritonMsg::Triton_Make_StatusMonitoringField()
{
	int nStatus = 0;
	int nCount = 0;
	int nIndex = 0;
	int nDenomination = 0;
	int nDivider = 0;
	int nNotesLoadedCount = 0;
	int nNotesDispensed = 0;
	int nRejectCount = 0;
	int nTotalNotesPurged = 0;
	CString strVersion, strTemp;

	if (CEagleDataManager::GetInstance()->m_Client_Info.m_bPowerOffFlag == FALSE)
	{
		// 1. Program Version Number (10 ASCII) // AP Version	
		CString strProductVersion;
		strProductVersion = CDEV_Manager::GetInstance()->m_strAP_Version;

		// Version Rule 변경에 따른 코드 수정 기존 Vxx.xx.xx.xx -> Vxxx.xxx.xxx.xxx
		// 01.00.01.00
		strProductVersion.Remove('.');											// xWWxXXxYYxZZ
		m_sTriton_Status_Field.m_strProgramVersionNo_10.Format(_T("%s-%s%s.%s"), strProductVersion.Mid(1,2), strProductVersion.Mid(4,2), strProductVersion.Mid(7,2),strProductVersion.Mid(10,2));
		m_strSendData += m_sTriton_Status_Field.m_strProgramVersionNo_10;

		// 2. Table Version Number (10 ASCII) - Not Used
		//strVersion = CUtil::String_GetRegistry(REGISTRY_OS_INFO_KEY, REGISTRY_KERNEL_VALUE_NAME);
		//strVersion.TrimLeft();
		//strVersion.TrimRight();
		//m_sTriton_Status_Field.m_strTableVersionNo_10.Format(_T("%s-0100.%s"), _T("OS"), strVersion.Right(2));
		m_sTriton_Status_Field.m_strTableVersionNo_10.Format(_T("00-0000.00"));
		m_strSendData += m_sTriton_Status_Field.m_strTableVersionNo_10;

		// 3. Firmware Version Number (10 ASCII) - not use
		strVersion.Format(_T("0000.00000"));
		m_sTriton_Status_Field.m_strFirmwareVersionNo_10 = strVersion;
		m_strSendData += m_sTriton_Status_Field.m_strFirmwareVersionNo_10;

		// 4. Alarm, Chest door open (1 ASCII) 0: False 1: True, <space>: Not supported, X: Not installed
		if (CSVC_Manager::GetInstance()->Is_DIO_Safe_Door_Open() == TRUE)
			m_sTriton_Status_Field.m_strAlarm_ChestDoorOpen_1 = _T("1");
		else
			m_sTriton_Status_Field.m_strAlarm_ChestDoorOpen_1 = _T("0");
		m_strSendData += m_sTriton_Status_Field.m_strAlarm_ChestDoorOpen_1;

		// 5. Alarm, Top door open (1 ASCII) 0: False 1: True <space> Not supported X : Not installed
		if (CSVC_Manager::GetInstance()->Is_DIO_Front_Door_Open() == TRUE)
			m_sTriton_Status_Field.m_strAlarm_TopDoorOpen_1 = _T("1");
		else
			m_sTriton_Status_Field.m_strAlarm_TopDoorOpen_1 = _T("0");
		m_strSendData += m_sTriton_Status_Field.m_strAlarm_TopDoorOpen_1;

		// 6. Alarm, Supervisor active (1 ASCII) 0: False 1: True <space> Not supported
		m_sTriton_Status_Field.m_strAlarm_SupervisonActive_1 = _T(" ");	//one space
		m_strSendData += m_sTriton_Status_Field.m_strAlarm_SupervisonActive_1;

		// 7. Receipt printer, paper status (1 ASCII) 0: OK 1: LOW 2: OUT <space> Not supported
		if( CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_GetPaperStatus() == EAGLE_PRT_PAPER_NORMAL )
			m_sTriton_Status_Field.m_strReceiptPrinter_PaperStatus_1 = _T("0");
		else if(CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_GetPaperStatus() == EAGLE_PRT_PAPER_NEAR)
			m_sTriton_Status_Field.m_strReceiptPrinter_PaperStatus_1 = _T("1");
		else if(CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_GetPaperStatus() == EAGLE_PRT_PAPER_EMPTY)
			m_sTriton_Status_Field.m_strReceiptPrinter_PaperStatus_1 = _T("2");
		else
			m_sTriton_Status_Field.m_strReceiptPrinter_PaperStatus_1 = _T("0");
		m_strSendData += m_sTriton_Status_Field.m_strReceiptPrinter_PaperStatus_1;

		// 8. Receipt Printer, Ribbon status (1 ASCII) 0: OK 1: LOW 2: OUT <space> Not supported T: thermal
		m_sTriton_Status_Field.m_strReceiptPrinter_RibbonStatus_1 = _T("T");
		m_strSendData += m_sTriton_Status_Field.m_strReceiptPrinter_RibbonStatus_1;

		// 9. Journal Printer, Paper status (1 ASCII) 0: OK 1: LOW 2: OUT <space> Not supported, X: Not installed
		m_sTriton_Status_Field.m_strJournalPrinter_PaperStatus_1 = _T("0");
		m_strSendData += m_sTriton_Status_Field.m_strJournalPrinter_PaperStatus_1;

		// 10. Journal Printer, Ribbon status (1 ASCII) 0: OK 1: LOW 2: OUT <space> Not supported, T: Thermal, X: Not installed
		m_sTriton_Status_Field.m_strJournalPrinter_RibbonStatus_1 = _T("0");
		m_strSendData += m_sTriton_Status_Field.m_strJournalPrinter_RibbonStatus_1;

		// 11. Note status, Dispenser (1 ASCII) 0: OK 1: LOW 2: OUT <space> Not supported
		// This is the cumulative summary of all cassettes in the dispenser.
		// Empty individual cassettes are not reflected here
		BOOL	bCheckLowStatus = FALSE;
		int		nNoteCount =CDEV_Manager::GetInstance()->m_DEV_CDM.Get_CBXAllNoteCount();
		int		nCstNoteStatus = CDEV_Manager::GetInstance()->m_DEV_CDM.Get_CBXAllStatus();
		int		nCBXCount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Count);

		if ( nNoteCount <= 0)												// 전체 매수가 0인 경우 (음수값과 양수값 합이 0인 경우에는 문제 소지 있음)
		{
			m_sTriton_Status_Field.m_strNoteStatus_Dispenser_1 = _T("2");	// OUT OF SERVICE
		}
		else
		{
			if (nCstNoteStatus == WFS_CDM_STATCULOW)
				m_sTriton_Status_Field.m_strNoteStatus_Dispenser_1 = _T("1");	// LOW
			else
				m_sTriton_Status_Field.m_strNoteStatus_Dispenser_1 = _T("0");	// OK
		}
		m_strSendData += m_sTriton_Status_Field.m_strNoteStatus_Dispenser_1;

		// 12. Receipt Printer (1 ASCII) 0: OK 1: Needs attention 2: Out of Service <space> Not supported X : Not installed
		if (CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_GetStatus() != EAGLE_PRT_NORMAL)
		{
			m_sTriton_Status_Field.m_strReceiptPrinter_1 = _T("2");
		}
		else 
		{
			nStatus = CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_GetPaperStatus();
			switch( nStatus )
			{
			case EAGLE_PRT_PAPER_EMPTY:
				m_sTriton_Status_Field.m_strReceiptPrinter_1 = _T("2");
				break;
			case EAGLE_PRT_PAPER_NEAR:
				m_sTriton_Status_Field.m_strReceiptPrinter_1 = _T("1");
				break;
			default:
				m_sTriton_Status_Field.m_strReceiptPrinter_1 = _T("0");
				break;
			}
		}
		m_strSendData += m_sTriton_Status_Field.m_strReceiptPrinter_1;

		// 13. Journal Printer (1 ASCII) 0: OK 1: Needs attention 2: Out of Service <space> Not supported X : Not installed
		m_sTriton_Status_Field.m_strJournalPrinter_1 = _T("0");
		m_strSendData += m_sTriton_Status_Field.m_strJournalPrinter_1;

		// 14. Dispenser (1 ASCII) 0: OK 1: Needs attention 2: Out of Service <space> Not supported X : Not installed
		CDMSTATUS stResult;
		memset(&stResult, 0, sizeof(CDMSTATUS));

		if (FALSE == CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_Status(&stResult))
			m_sTriton_Status_Field.m_strDispenser_1 = _T("2");
		else
			m_sTriton_Status_Field.m_strDispenser_1 = _T("0");
		m_strSendData += m_sTriton_Status_Field.m_strDispenser_1;

		// 15. Communications System (1 ASCII) 0: OK 1: Needs attention 2: Out of Service
		m_sTriton_Status_Field.m_strCommunicationsSystem_1 = _T("0");
		m_strSendData += m_sTriton_Status_Field.m_strCommunicationsSystem_1;

		// 16. Card Reader (1 ASCII) 0: OK 1: Needs attention 2: Out of Service
		if (CDEV_Manager::GetInstance()->m_DEV_CDR.GetCDR_DeviceStatus() == CARD_ERROR)
			m_sTriton_Status_Field.m_strCardReader_1 = _T("2");
		else
			m_sTriton_Status_Field.m_strCardReader_1 = _T("0");
		m_strSendData += m_sTriton_Status_Field.m_strCardReader_1;

		// 17. Cards retained (3 ASCII)  000-999, <space> Not supported
		m_sTriton_Status_Field.m_strCardsRetained_3 = _T("   ");
		m_strSendData += m_sTriton_Status_Field.m_strCardsRetained_3;

		// 18. Electronics system (2 ASCII) XX -  , <space> Not supported 
		m_sTriton_Status_Field.m_strElectronicsSystem_2 = _T("  ");
		m_strSendData += m_sTriton_Status_Field.m_strElectronicsSystem_2;

		// Error Conversion Needed for Triton
		// 19. Current Error Code for Terminal (3 ASCII) XXX represents the value of the error code '000' No problems
		if (CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetError == TRUE)
		{
			CString strError = CEagleSVCLib::GetInstance()->Get_ErrorCode();

			if (strError.Left(1) == _T("J"))
				m_sTriton_Status_Field.m_strCurrentErrorCode_3 = _T("552");
			else if (strError.Left(1) == _T("C"))
				m_sTriton_Status_Field.m_strCurrentErrorCode_3 = _T("231");
			else if (strError.Left(1) == _T("D"))
				m_sTriton_Status_Field.m_strCurrentErrorCode_3 = _T("377");
			else if (strError.Left(1) == _T("E"))
				m_sTriton_Status_Field.m_strCurrentErrorCode_3 = _T("579");
			else if (strError.Left(1) == _T("P"))
				m_sTriton_Status_Field.m_strCurrentErrorCode_3 = _T("138");
			else
				m_sTriton_Status_Field.m_strCurrentErrorCode_3 = _T("559");
		}
		else // ERROR CODE가 없을 경우
		{
			m_sTriton_Status_Field.m_strCurrentErrorCode_3 = _T("000");
		}
		m_strSendData += m_sTriton_Status_Field.m_strCurrentErrorCode_3;

		// 20. Communications failures (3 ASCII)
		m_sTriton_Status_Field.m_strCommunicatioinsFailures_3 = _T("000");
		m_strSendData += m_sTriton_Status_Field.m_strCommunicatioinsFailures_3;

		//////////////////////////////////////////////////////////////////////////
		//	Default Cassette A, B, C, D set all '0'
		//  Denomination(3 ASCII), Notes loaded (4 ASCII), 
		//  Notes dispensed(4 ASCII), Reject events(3 ASCII)
		//////////////////////////////////////////////////////////////////////////
		//Denomination(3 ASCII)
		m_sTriton_Status_Field.m_strCassetteA_Denomination_3 = _T("000");
		m_sTriton_Status_Field.m_strCassetteB_Denomination_3 = _T("000");
		m_sTriton_Status_Field.m_strCassetteC_Denomination_3 = _T("000");
		m_sTriton_Status_Field.m_strCassetteD_Denomination_3 = _T("000");

		//Notes loaded (4 ASCII)
		m_sTriton_Status_Field.m_strCassetteA_NotesLoaded_4 = _T("0000");
		m_sTriton_Status_Field.m_strCassetteB_NotesLoaded_4 = _T("0000");
		m_sTriton_Status_Field.m_strCassetteC_NotesLoaded_4 = _T("0000");
		m_sTriton_Status_Field.m_strCassetteD_NotesLoaded_4 = _T("0000");
		//Notes dispensed(4 ASCII)

		m_sTriton_Status_Field.m_strCassetteA_NotesDispensed_4 = _T("0000");
		m_sTriton_Status_Field.m_strCassetteB_NotesDispensed_4 = _T("0000");
		m_sTriton_Status_Field.m_strCassetteC_NotesDispensed_4 = _T("0000");
		m_sTriton_Status_Field.m_strCassetteD_NotesDispensed_4 = _T("0000");
		//Reject events(3 ASCII)

		m_sTriton_Status_Field.m_strCassetteA_RejectEvents_3 = _T("000");
		m_sTriton_Status_Field.m_strCassetteB_RejectEvents_3 = _T("000");
		m_sTriton_Status_Field.m_strCassetteC_RejectEvents_3 = _T("000");
		m_sTriton_Status_Field.m_strCassetteD_RejectEvents_3 = _T("000");

		////////////////////////////////////////////////////////////////////////
		// Cassette A 
		if( nCBXCount >= 1)
		{
			// 21. Cassette A Denomination (3 ASCII) 000-99K 'K' indicate thousands. 01K - 1000
			nDenomination = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Denomination[0]);

			// 권종이 $10인 경우 1K로 보내줘야함
			////////////////////////////////////////////////////////////////////////////////////
			if( (nDenomination%10) == 0 )
			{
				nDivider = nDenomination / 10;
				m_sTriton_Status_Field.m_strCassetteA_Denomination_3.Format(_T("%02dK"), nDivider);
			}
			else
			{
				nDivider = nDenomination * 100;
				m_sTriton_Status_Field.m_strCassetteA_Denomination_3.Format(_T("%03d"), nDivider);
			}

			// 22. Cassette A notes loaded (4 ASCII) 0000-9999
			nNotesLoadedCount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Load_Count[0]);
			m_sTriton_Status_Field.m_strCassetteA_NotesLoaded_4.Format(_T("%4.4d"), nNotesLoadedCount);

			// 23. Cassette A notes dispensed (4 ASCII) 0000-9999
			nNotesDispensed = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Dispense_Count[0]);
			m_sTriton_Status_Field.m_strCassetteA_NotesDispensed_4.Format(_T("%4.4d"), nNotesDispensed);

			// 24. Cassette A reject events (3 ASCII) 000-999
			nRejectCount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Reject_Count[0]);
			m_sTriton_Status_Field.m_strCassetteA_RejectEvents_3.Format(_T("%3.3d"), nRejectCount);
		}
		m_strSendData += m_sTriton_Status_Field.m_strCassetteA_Denomination_3.Right(3)	+
			m_sTriton_Status_Field.m_strCassetteA_NotesLoaded_4.Right(4)	+
			m_sTriton_Status_Field.m_strCassetteA_NotesDispensed_4.Right(4) +
			m_sTriton_Status_Field.m_strCassetteA_RejectEvents_3.Right(3);

		// Cassette B
		if( nCBXCount >= 2)
		{
			// 25. Cassette B Denomination (3 ASCII) 000-99K 'K' indicate thousands. 01K - 1000
			nDenomination = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Denomination[1]);

			// 권종이 $10인 경우 1K로 보내줘야함
			if( (nDenomination%10) == 0 )
			{
				nDivider = nDenomination / 10;
				m_sTriton_Status_Field.m_strCassetteB_Denomination_3.Format(_T("%02dK"), nDivider);
			}
			else
			{
				nDivider = nDenomination * 100;
				m_sTriton_Status_Field.m_strCassetteB_Denomination_3.Format(_T("%03d"), nDivider);
			}

			// 26. Cassette B notes loaded (4 ASCII) 0000-9999
			nNotesLoadedCount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Load_Count[1]);
			m_sTriton_Status_Field.m_strCassetteB_NotesLoaded_4.Format(_T("%4.4d"), nNotesLoadedCount);

			// 27. Cassette B notes dispensed (4 ASCII) 0000-9999
			nNotesDispensed = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Dispense_Count[1]);
			m_sTriton_Status_Field.m_strCassetteB_NotesDispensed_4.Format(_T("%4.4d"), nNotesDispensed) ;

			// 28. Cassette B reject events (3 ASCII) 000-999
			nRejectCount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Reject_Count[1]);
			m_sTriton_Status_Field.m_strCassetteB_RejectEvents_3.Format(_T("%3.3d"), nRejectCount);
		}
		m_strSendData += m_sTriton_Status_Field.m_strCassetteB_Denomination_3.Right(3)	+
			m_sTriton_Status_Field.m_strCassetteB_NotesLoaded_4.Right(4)	+
			m_sTriton_Status_Field.m_strCassetteB_NotesDispensed_4.Right(4) +
			m_sTriton_Status_Field.m_strCassetteB_RejectEvents_3.Right(3);

		// Cassette C
		if( nCBXCount >= 3)
		{
			// 29. Cassette C Denomination (3 ASCII) 000-99K 'K' indicate thousands. 01K - 1000
			nDenomination = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Denomination[2]);

			// 권종이 $10인 경우 1K로 보내줘야함
			////////////////////////////////////////////////////////////////////////////////////
			if( (nDenomination%10) == 0 )
			{
				nDivider = nDenomination / 10;
				m_sTriton_Status_Field.m_strCassetteC_Denomination_3.Format(_T("%02dK"), nDivider);
			}
			else
			{
				nDivider = nDenomination * 100;
				m_sTriton_Status_Field.m_strCassetteC_Denomination_3.Format(_T("%03d"), nDivider);
			}

			// 30. Cassette C notes loaded (4 ASCII) 0000-9999
			nNotesLoadedCount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Load_Count[2]);
			m_sTriton_Status_Field.m_strCassetteC_NotesLoaded_4.Format(_T("%4.4d"), nNotesLoadedCount);

			// 31. Cassette C notes dispensed (4 ASCII) 0000-9999
			nNotesDispensed = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Dispense_Count[2]);
			m_sTriton_Status_Field.m_strCassetteC_NotesDispensed_4.Format(_T("%4.4d"), nNotesDispensed) ;

			// 32. Cassette C reject events (3 ASCII) 000-999
			nRejectCount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Reject_Count[2]);
			m_sTriton_Status_Field.m_strCassetteC_RejectEvents_3.Format(_T("%3.3d"), nRejectCount);
		}
		m_strSendData += m_sTriton_Status_Field.m_strCassetteC_Denomination_3.Right(3)	+
			m_sTriton_Status_Field.m_strCassetteC_NotesLoaded_4.Right(4)	+
			m_sTriton_Status_Field.m_strCassetteC_NotesDispensed_4.Right(4) +
			m_sTriton_Status_Field.m_strCassetteC_RejectEvents_3.Right(3);

		// Cassette D
		if( nCBXCount >= 4)
		{
			// 33. Cassette D Denomination (3 ASCII) 000-99K 'K' indicate thousands. 01K - 1000
			nDenomination = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Denomination[3]);

			// 권종이 $10인 경우 1K로 보내줘야함
			if( (nDenomination%10) == 0 )
			{
				nDivider = nDenomination / 10;
				m_sTriton_Status_Field.m_strCassetteD_Denomination_3.Format(_T("%02dK"), nDivider);
			}
			else
			{
				nDivider = nDenomination * 100;
				m_sTriton_Status_Field.m_strCassetteD_Denomination_3.Format(_T("%03d"), nDivider);
			}

			// 34. Cassette D notes loaded (4 ASCII) 0000-9999
			nNotesLoadedCount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Load_Count[3]);
			m_sTriton_Status_Field.m_strCassetteD_NotesLoaded_4.Format(_T("%4.4d"), nNotesLoadedCount);

			// 35. Cassette D notes dispensed (4 ASCII) 0000-9999
			nNotesDispensed = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Dispense_Count[3]);
			m_sTriton_Status_Field.m_strCassetteD_NotesDispensed_4.Format(_T("%4.4d"), nNotesDispensed) ;

			// 36. Cassette D reject events (3 ASCII) 000-999
			nRejectCount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Reject_Count[3]);
			m_sTriton_Status_Field.m_strCassetteD_RejectEvents_3.Format(_T("%3.3d"), nRejectCount);
		}
		m_strSendData += m_sTriton_Status_Field.m_strCassetteD_Denomination_3.Right(3)		+
			m_sTriton_Status_Field.m_strCassetteD_NotesLoaded_4.Right(4)		+
			m_sTriton_Status_Field.m_strCassetteD_NotesDispensed_4.Right(4)	+
			m_sTriton_Status_Field.m_strCassetteD_RejectEvents_3.Right(3);

		nTotalNotesPurged = CUtil::StringToInt(m_sTriton_Status_Field.m_strCassetteA_RejectEvents_3) + 
			CUtil::StringToInt(m_sTriton_Status_Field.m_strCassetteB_RejectEvents_3) +
			CUtil::StringToInt(m_sTriton_Status_Field.m_strCassetteC_RejectEvents_3) +
			CUtil::StringToInt(m_sTriton_Status_Field.m_strCassetteD_RejectEvents_3) ;

		m_sTriton_Status_Field.m_strTotalNotesPurged_3.Format(L"%3.3d", nTotalNotesPurged);
		m_strSendData += m_sTriton_Status_Field.m_strTotalNotesPurged_3.Right(3);
	}

	return 0;
}


/** **********************************************************
*	@brief		Triton EMV Request Data 생성
*	@param		int nTranType
*	@return		0
************************************************************/
int CEagleTritonMsg::Triton_Make_EMV_RequestData(int nTranType)
{
	CString			strTmp_Script;
	CString			strTagLenValue;
	CString			strValue;
	unsigned char	szTemp[1024] = {0,};
	int				nLen = 0;

	LOG(Info, _T("Triton_Make_EMV_RequestData(%d) - Start"), nTranType);

	if (nTranType == MISC_REQ_TRAN)
	{
		// Normal Transaction Process
		// Track2 Equivalent Data (57) (Len : 19Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_Track2EquData, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = CUtil::ConvertHexToString(szTemp, nLen);

			if( nLen > 0 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(_T("%S%02X%s"), "57", nLen, strValue);
				CEagleDataManager::GetInstance()->m_Client_Info.m_strTag57Data = strTagLenValue;	// for Masking
				strTmp_Script += strTagLenValue; 
			}
		}

		// Application Primary Account Number(PAN) (5A) (Len : var)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_PAN, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = CUtil::ConvertHexToString(szTemp, nLen);

			/* Application Primary Account Number Sequence */
			if( nLen > 0 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(_T("%S%02X%s"), "5A", nLen, strValue);
				CEagleDataManager::GetInstance()->m_Client_Info.m_strTag5AData = strTagLenValue;	// for Masking
				strTmp_Script += strTagLenValue; 
			}
		}

		// Application Interchange Profile (82) (Len : 2Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_AIP, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = CUtil::ConvertHexToString(szTemp, nLen);

			/* Application Interchange Profile Max Len 2*/
			if( nLen > 0 && nLen <= 2 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(_T("%S%02X%s"), "82", nLen, strValue);
				strTmp_Script += strTagLenValue; 
			}
		}


		// Dedicated file name (84)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_DFName, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = CUtil::ConvertHexToString(szTemp, nLen);

			/* Dedicated file name Max Len 16 */
			if( nLen > 0 && nLen <= 16 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(_T("%S%02X%s"), "84", nLen, strValue);
				strTmp_Script += strTagLenValue; 
			}
		}

		// Cardholder Verification Method(CVM) List (8E)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_CVMList, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = CUtil::ConvertHexToString(szTemp, nLen);

			/* Cardholder Verification Method(CVM) List Len 252 */
			if( nLen > 0 && nLen <= 252 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(_T("%S%02X%s"), "8E", nLen, strValue);
				strTmp_Script += strTagLenValue; 
			}
		}

		// Terminal Verification Result (95) (Len : 5Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_TVR, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = CUtil::ConvertHexToString(szTemp, nLen);

			/* Terminal Verification Result Max Len 5 */
			if( nLen > 0 && nLen <= 5 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(_T("%S%02X%s"), "95", nLen, strValue);
				strTmp_Script += strTagLenValue; 
			}
		}

		// Transaction Date (9A) (Len : 3Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_TrDate, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = CUtil::ConvertHexToString(szTemp, nLen);

			/* Transaction Date Max Len 3 */
			if( nLen > 0 && nLen <= 3 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(_T("%S%02X%s"), "9A", nLen, strValue);
				strTmp_Script += strTagLenValue; 
			}
		}

		// Transaction Status Information (9B)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_TSI, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = CUtil::ConvertHexToString(szTemp, nLen);

			/* Transaction Status Information Max Len 2 */
			if( nLen > 0 && nLen <= 2 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(_T("%S%02X%s"), "9B", nLen, strValue);
				strTmp_Script += strTagLenValue; 
			}
		}

		// Transaction Type (9C) (Len : 1Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_TrType, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = CUtil::ConvertHexToString(szTemp, nLen);

			/* Transaction Type Max Len 1 */
			if( nLen > 0 && nLen <= 1 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(_T("%S%02X%s"), "9C", nLen, strValue);
				strTmp_Script += strTagLenValue; 
			}
		}

		// Application Expired Date (5F24) (Len : 3)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_AppExpiredDate, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = CUtil::ConvertHexToString(szTemp, nLen);

			/* Application Expired Date  Max Len 3 */
			if( nLen > 0 && nLen <= 3 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(_T("%S%02X%s"), EMVTag_AppExpiredDate, nLen, strValue);
				strTmp_Script += strTagLenValue; 
			}
		}

		// Terminal Currency Code (5F2A) (Len : 2Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_TrCurCod, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = CUtil::ConvertHexToString(szTemp, nLen);

			/* Terminal Currency Code Max Len 2 */
			if( nLen > 0 && nLen <= 2 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(_T("%S%02X%s"), EMVTag_TrCurCod, nLen, strValue);
				strTmp_Script += strTagLenValue; 
			}
		}

		// Application Primary Account Number Sequence Number (5F34) (Len : 1Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_PANSeqNo, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = CUtil::ConvertHexToString(szTemp, nLen);

			/* Application Primary Account Number Sequence Number Max Len 1 */
			if( nLen > 0 && nLen <= 1 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(_T("%S%02X%s"), EMVTag_PANSeqNo, nLen, strValue);
				strTmp_Script += strTagLenValue; 
			}
		}

		// amount authorized (9F02) (Len : 6Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_AmountAuthN, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = CUtil::ConvertHexToString(szTemp, nLen);

			/* transaction category code Max Len 6 */
			if( nLen > 0 && nLen <= 6 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(_T("%S%02X%s"), EMVTag_AmountAuthN, nLen, strValue);
				strTmp_Script += strTagLenValue; 
			}
		}

		// amount other (9F03) (Len : 6Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_AmountOtherN, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = CUtil::ConvertHexToString(szTemp, nLen);

			/* transaction category code Max Len 6 */
			if( nLen > 0 && nLen <= 6 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(_T("%S%02X%s"), EMVTag_AmountOtherN, nLen, strValue);
				strTmp_Script += strTagLenValue; 
			}
		}
		else
		{
			strTagLenValue.Format(_T("%S%02X%s"), EMVTag_AmountOtherN, 0x06, _T("000000000000"));
			strTmp_Script += strTagLenValue; 
		}

		// Application Identifier (AID) (9F06) (Len : 16Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_TerminalAID, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = CUtil::ConvertHexToString(szTemp, nLen);

			/* Application ID Max Len 16 */
			if( nLen > 0 && nLen <= 16) 
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(_T("%S%02X%s"), EMVTag_TerminalAID, nLen, strValue);
				strTmp_Script += strTagLenValue; 
			}
		}

		// Application Version Number (9F09)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_TrAppVerNo, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = CUtil::ConvertHexToString(szTemp, nLen);

			/* Application Version Number Max Len 2 */
			if( nLen > 0 && nLen <= 2 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(_T("%S%02X%s"), EMVTag_TrAppVerNo, nLen, strValue);
				strTmp_Script += strTagLenValue; 
			}
		}

		// Issuer Application Data (9F10) (Len : 32Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_IsuAppData, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = CUtil::ConvertHexToString(szTemp, nLen);

			/* Issuer Application Data Max Len 32*/
			if( nLen > 0 && nLen <= 32 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(_T("%S%02X%s"), EMVTag_IsuAppData, nLen, strValue);
				strTmp_Script += strTagLenValue; 
			}
		}

		// Terminal Country Code (9F1A) (Len : 2Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_TCountryCod, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = CUtil::ConvertHexToString(szTemp, nLen);

			/* Terminal Country Code Max Len 2 */
			if( nLen > 0 && nLen <= 2 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(_T("%S%02X%s"), EMVTag_TCountryCod, nLen, strValue);
				strTmp_Script += strTagLenValue; 
			}	
		}

		// IFD(Interface Device) serial number (9F1E)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_IFDSerialNo, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = CUtil::ConvertHexToString(szTemp, nLen);

			/* IFD(Interface Device) serial number Max Len 8 */
			if( nLen > 0 && nLen <= 8 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(_T("%S%02X%s"), EMVTag_IFDSerialNo, nLen, strValue);
				strTmp_Script += strTagLenValue; 
			}
		}

		// Transaction Time (9F21) (Len : 3 Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_TrTime, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = CUtil::ConvertHexToString(szTemp, nLen);

			/* Transaction Time Max Len 3 */
			if( nLen > 0 && nLen <= 3) 
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(_T("%S%02X%s"), EMVTag_TrTime, nLen, strValue);
				strTmp_Script += strTagLenValue; 
			}
		}

		// Application Cryptogram (9F26)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_AC, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = CUtil::ConvertHexToString(szTemp, nLen);

			/* Application Cryptogram  Max Len 8 */
			if( nLen > 0 && nLen <= 8 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(_T("%S%02X%s"), EMVTag_AC, nLen, strValue);
				strTmp_Script += strTagLenValue; 
			}
		}	

		// Cryptogram Information Data  (9F27) (Len : 1Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_CryptInfData, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = CUtil::ConvertHexToString(szTemp, nLen);

			/* Cryptogram Information Data Max Len 1 */
			if( nLen > 0 && nLen <= 1 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(_T("%S%02X%s"), EMVTag_CryptInfData, nLen, strValue);
				strTmp_Script += strTagLenValue; 
			}
		}

		// Terminal Capabilities (9F33) (Len : 3Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_TerminalCapa, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = CUtil::ConvertHexToString(szTemp, nLen);

			/* Terminal Capabilities Max Len 3 */
			if( nLen > 0 && nLen <= 3 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(_T("%S%02X%s"), EMVTag_TerminalCapa, nLen, strValue);
				strTmp_Script += strTagLenValue; 
			}
		}

		// Card holder Verification Method(CVM) Result (9F34) (Len : 3Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_CVMRslt, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = CUtil::ConvertHexToString(szTemp, nLen);

			/* Cardholder Verification Method(CVM) Result Len 3 */
			if( nLen > 0 && nLen <= 3 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(_T("%S%02X%s"), EMVTag_CVMRslt, nLen, strValue);
				strTmp_Script += strTagLenValue; 
			}
		}

		// Terminal Type (9F35) (Len : 1Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_TerminalType, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = CUtil::ConvertHexToString(szTemp, nLen);

			/* Terminal Type Max Len 1 */
			if( nLen > 0 && nLen <= 1 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(_T("%S%02X%s"), EMVTag_TerminalType, nLen, strValue);
				strTmp_Script += strTagLenValue; 
			}
		}

		// Application Transaction Counter (9F36) (Len : 2Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_ATC, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = CUtil::ConvertHexToString(szTemp, nLen);

			/* Application Transaction Counter Len 2*/
			if( nLen > 0 && nLen <= 2 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(_T("%S%02X%s"), EMVTag_ATC, nLen, strValue);
				strTmp_Script += strTagLenValue; 
			}
		}

		// Unpredictable Number (9F37) (Len : 4Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_UnpredictNo, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = CUtil::ConvertHexToString(szTemp, nLen);

			/* Unpredictable Number Max Len 4 */
			if( nLen > 0 && nLen <= 4 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(_T("%S%02X%s"), EMVTag_UnpredictNo, nLen, strValue);
				strTmp_Script += strTagLenValue; 
			}
		}

		// Point Of Service Entry Mode Code [POS -> 0x05 Fixed] (9F39) (Len : 1Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_POSEntryMode, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = CUtil::ConvertHexToString(szTemp, nLen);

			/* Point Of Service Entry Mode Code Max Len 1 */
			if( nLen > 0 && nLen <= 1 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(_T("%S%02X%s"), EMVTag_POSEntryMode, nLen, strValue);
				strTmp_Script += strTagLenValue; 
			}
		}

		// transaction sequence counter (9F41) (Len : 4Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_TrSeqCnt, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = CUtil::ConvertHexToString(szTemp, nLen);

			/* transaction category code Max Len 4 */
			if( nLen > 0 && nLen <= 4 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(_T("%S%02X%s"), EMVTag_TrSeqCnt, nLen, strValue);
				strTmp_Script += strTagLenValue; 
			}
		}

		// transaction category code (9F53)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_TCC, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = CUtil::ConvertHexToString(szTemp, nLen);

			/* transaction category code Max Len 1 */
			if( nLen > 0 && nLen <= 1 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(_T("%S%02X%s"), EMVTag_TCC, nLen, strValue);
				strTmp_Script += strTagLenValue; 
			}
		}

		m_strSendData += _T("ud") + strTmp_Script;
		m_strSendData += FIELD_DELIMITER;
	}
	else
	{
		// Reversal Process
		strTagLenValue = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_57;

		if (strTagLenValue.GetLength() > 0)
			strTmp_Script += strTagLenValue;

		strTagLenValue = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_5A;

		if (strTagLenValue.GetLength() > 0)
			strTmp_Script += strTagLenValue;

		strTagLenValue = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_82;

		if (strTagLenValue.GetLength() > 0)
			strTmp_Script += strTagLenValue;

		strTagLenValue = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_95;

		if (strTagLenValue.GetLength() > 0)
			strTmp_Script += strTagLenValue;

		strTagLenValue = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_9A;

		if (strTagLenValue.GetLength() > 0)
			strTmp_Script += strTagLenValue;

		strTagLenValue = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_9C;

		if (strTagLenValue.GetLength() > 0)
			strTmp_Script += strTagLenValue;

		strTagLenValue = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_5F2A;

		if (strTagLenValue.GetLength() > 0)
			strTmp_Script += strTagLenValue;

		strTagLenValue = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_5F34;

		if (strTagLenValue.GetLength() > 0)
			strTmp_Script += strTagLenValue;

		strTagLenValue = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_9F02;

		if (strTagLenValue.GetLength() > 0)
			strTmp_Script += strTagLenValue;
		
		strTagLenValue = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_9F03;

		if (strTagLenValue.GetLength() > 0)
			strTmp_Script += strTagLenValue;

		strTagLenValue = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_9F10;

		if (strTagLenValue.GetLength() > 0)
			strTmp_Script += strTagLenValue;

		strTagLenValue = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_9F18;

		if (strTagLenValue.GetLength() > 0)
			strTmp_Script += strTagLenValue;

		strTagLenValue = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_9F1A;

		if (strTagLenValue.GetLength() > 0)
			strTmp_Script += strTagLenValue;

		strTagLenValue = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_9F26;

		if (strTagLenValue.GetLength() > 0)
			strTmp_Script += strTagLenValue;

		strTagLenValue = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_9F27;

		if (strTagLenValue.GetLength() > 0)
			strTmp_Script += strTagLenValue;

		strTagLenValue = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_9F33;

		if (strTagLenValue.GetLength() > 0)
			strTmp_Script += strTagLenValue;

		strTagLenValue = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_9F35;

		if (strTagLenValue.GetLength() > 0)
			strTmp_Script += strTagLenValue;

		strTagLenValue = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_9F36;

		if (strTagLenValue.GetLength() > 0)
			strTmp_Script += strTagLenValue;

		strTagLenValue = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_9F37;

		if (strTagLenValue.GetLength() > 0)
			strTmp_Script += strTagLenValue;

		strTagLenValue = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_9F39;

		if (strTagLenValue.GetLength() > 0)
			strTmp_Script += strTagLenValue;

		strTagLenValue = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_9F41;

		if (strTagLenValue.GetLength() > 0)
			strTmp_Script += strTagLenValue;

		if (strTmp_Script.IsEmpty() == FALSE)
		{
			m_strSendData += _T("ud") + strTmp_Script;
			m_strSendData += FIELD_DELIMITER;
		}

		// Reversal Data는 추가적으로 Issuer Script Result가 있을 경우 송부해야함. (누락분 추가 적용)
		strTagLenValue = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_DF05;
		if (strTagLenValue.IsEmpty() == FALSE)	// Data가 존재하는 경우
		{
			m_strSendData += _T("uh") + strTagLenValue;
			m_strSendData += FIELD_DELIMITER;			
		}
	}

	return 0;
}


/** **********************************************************
*	@brief		Triton EMV Data 저장 - Reversal 및 저널
*	@return		0
************************************************************/
int CEagleTritonMsg::Triton_Save_EMV_Data(int nIssuerScriptLen)
{
	unsigned char szTemp[1024];
	CString strValue;
	CString strTagLenValue;
	int		nLen;	

	LOG(Info, _T("Triton_Save_EMV_Data - Save EMV_Data (%d)"), nIssuerScriptLen);

	// Track2 Equivalent Data (57) (Len : 19Byte)
	nLen = 0;
	strValue.Empty();
	memset(szTemp, 0x0, sizeof(szTemp));

	if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_Track2EquData, &nLen, szTemp) == EMV_RSLT_OK)
	{
		strValue = CUtil::ConvertHexToString(szTemp, nLen);

		if( nLen > 0 )
		{
			/* Tag + Length + Value */
			strTagLenValue.Format(_T("%S%02X%s"), "57", nLen, strValue);

			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_57 = strTagLenValue;
		}
	}

	// Application Primary Account Number(PAN) (5A) (Len : var)
	nLen = 0;
	strValue.Empty();
	memset(szTemp, 0x0, sizeof(szTemp));

	if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_PAN, &nLen, szTemp) == EMV_RSLT_OK)
	{
		strValue = CUtil::ConvertHexToString(szTemp, nLen);

		/* Application Primary Account Number Sequence */
		if( nLen > 0 )
		{
			/* Tag + Length + Value */
			strTagLenValue.Format(_T("%S%02X%s"), "5A", nLen, strValue);

			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_5A = strTagLenValue;
		}
	}

	// Application Interchange Profile (82) (Len : 2Byte)
	nLen = 0;
	strValue.Empty();
	memset(szTemp, 0x0, sizeof(szTemp));

	if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_AIP, &nLen, szTemp) == EMV_RSLT_OK)
	{
		strValue = CUtil::ConvertHexToString(szTemp, nLen);

		/* Application Interchange Profile Max Len 2*/
		if( nLen > 0 && nLen <= 2 )
		{
			/* Tag + Length + Value */
			strTagLenValue.Format(_T("%S%02X%s"), "82", nLen, strValue);

			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_82 = strTagLenValue;
		}
	}

	// ARC (8A) (Len : 2Byte) - Journal 저장용
	nLen = 0;
	strValue.Empty();
	memset(szTemp, 0x0, sizeof(szTemp));

	if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_AuthRespCod, &nLen, szTemp) == EMV_RSLT_OK)
	{
		strValue = CUtil::ConvertHexToString(szTemp, nLen);

		if( nLen > 0)
		{
			/* Tag + Length + Value */
			strTagLenValue.Format(_T("%S%02X%s"), "8A00", nLen, strValue);

			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_8A = strTagLenValue;
		}
	}

	// Terminal Verification Result (95)
	nLen = 0;
	strValue.Empty();
	memset(szTemp, 0x0, sizeof(szTemp));

	if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_TVR, &nLen, szTemp) == EMV_RSLT_OK)
	{
		strValue = CUtil::ConvertHexToString(szTemp, nLen);

		/* Terminal Verification Result Max Len 5 */
		if( nLen > 0 && nLen <= 5 )
		{
			/* Tag + Length + Value */
			strTagLenValue.Format(_T("%S%02X%s"), "95", nLen, strValue);

			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_95 = strTagLenValue;
		}
	}

	// Transaction Date (9A) (Len : 3Byte)
	nLen = 0;
	strValue.Empty();
	memset(szTemp, 0x0, sizeof(szTemp));

	if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_TrDate, &nLen, szTemp) == EMV_RSLT_OK)
	{
		strValue = CUtil::ConvertHexToString(szTemp, nLen);

		/* Transaction Date Max Len 3 */
		if( nLen > 0 && nLen <= 3 )
		{
			/* Tag + Length + Value */
			strTagLenValue.Format(_T("%S%02X%s"), "9A", nLen, strValue);

			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_9A = strTagLenValue;
		}
	}

	// Transaction Type (9C) (Len : 1Byte)
	nLen = 0;
	strValue.Empty();
	memset(szTemp, 0x0, sizeof(szTemp));

	if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_TrType, &nLen, szTemp) == EMV_RSLT_OK)
	{
		strValue = CUtil::ConvertHexToString(szTemp, nLen);

		/* Transaction Type Max Len 1 */
		if( nLen > 0 && nLen <= 1 )
		{
			/* Tag + Length + Value */
			strTagLenValue.Format(_T("%S%02X%s"), "9C", nLen, strValue);

			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_9C = strTagLenValue;
		}
	}

	// Terminal Currency Code (5F2A) (Len : 2Byte)
	nLen = 0;
	strValue.Empty();
	memset(szTemp, 0x0, sizeof(szTemp));

	if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_TrCurCod, &nLen, szTemp) == EMV_RSLT_OK)
	{
		strValue = CUtil::ConvertHexToString(szTemp, nLen);

		/* Terminal Currency Code Max Len 2 */
		if( nLen > 0 && nLen <= 2 )
		{
			/* Tag + Length + Value */
			strTagLenValue.Format(_T("%S%02X%s"), EMVTag_TrCurCod, nLen, strValue);

			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_5F2A = strTagLenValue;
		}
	}

	// Application Primary Account Number Sequence Number (5F34) (Len : 1Byte)
	nLen = 0;
	strValue.Empty();
	memset(szTemp, 0x0, sizeof(szTemp));

	if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_PANSeqNo, &nLen, szTemp) == EMV_RSLT_OK)
	{
		strValue = CUtil::ConvertHexToString(szTemp, nLen);

		/* Application Primary Account Number Sequence Number Max Len 1 */
		if( nLen > 0 && nLen <= 1 )
		{
			/* Tag + Length + Value */
			strTagLenValue.Format(_T("%S%02X%s"), EMVTag_PANSeqNo, nLen, strValue);

			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_5F34 = strTagLenValue;
		}
	}

	// amount authorized (9F02) (Len : 6Byte)
	nLen = 0;
	strValue.Empty();
	memset(szTemp, 0x0, sizeof(szTemp));

	if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_AmountAuthN, &nLen, szTemp) == EMV_RSLT_OK)
	{
		strValue = CUtil::ConvertHexToString(szTemp, nLen);

		/* transaction category code Max Len 6 */
		if( nLen > 0 && nLen <= 6 )
		{
			/* Tag + Length + Value */
			strTagLenValue.Format(_T("%S%02X%s"), EMVTag_AmountAuthN, nLen, strValue);

			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_9F02 = strTagLenValue;
		}
	}

	// amount other (9F03) (Len : 6Byte)
	nLen = 0;
	strValue.Empty();
	memset(szTemp, 0x0, sizeof(szTemp));

	strTagLenValue.Format(_T("%S%02X%s"), EMVTag_AmountOtherN, 0x06, ZERO_12);

	CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_9F03 = strTagLenValue;

	// Issuer Application Data (9F10)
	nLen = 0;
	strValue.Empty();
	memset(szTemp, 0x0, sizeof(szTemp));

	if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_IsuAppData, &nLen, szTemp) == EMV_RSLT_OK)
	{
		strValue = CUtil::ConvertHexToString(szTemp, nLen);

		/* Issuer Application Data Max Len 32 */
		if( nLen > 0 && nLen <= 32 )
		{
			/* Tag + Length + Value */
			strTagLenValue.Format(_T("%S%02X%s"), EMVTag_IsuAppData, nLen, strValue);

			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_9F10 = strTagLenValue;
		}
	}

	// issuer script ID and results (9F18)
	if(nIssuerScriptLen > 0)
	{
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));

		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_IsuScriptId, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = CUtil::ConvertHexToString(szTemp, nLen);

			/* Issuer script identifier Max Len 4 */
			if( nLen > 0 && nLen <= 4 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(_T("%S%02X%s"), EMVTag_IsuScriptId, nLen, strValue);

				CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_9F18 = strTagLenValue;
			}
		}

		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_IsuScriptRslt, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = CUtil::ConvertHexToString(szTemp, nLen);

			/* Issuer script result Max Len 128 */
			if( nLen > 0 && nLen <= 128 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(_T("%S%02X%s"), "DF05", nLen, strValue);

				CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_DF05 = strTagLenValue;
			}
		}
	}

	// Terminal Country Code (9F1A) (Len : 2Byte)
	nLen = 0;
	strValue.Empty();
	memset(szTemp, 0x0, sizeof(szTemp));

	if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_TCountryCod, &nLen, szTemp) == EMV_RSLT_OK)
	{
		strValue = CUtil::ConvertHexToString(szTemp, nLen);

		/* Terminal Country Code Max Len 2 */
		if( nLen > 0 && nLen <= 2 )
		{
			/* Tag + Length + Value */
			strTagLenValue.Format(_T("%S%02X%s"), EMVTag_TCountryCod, nLen, strValue);
			
			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_9F1A = strTagLenValue;
		}
	}

	// Application Cryptogram (9F26)
	nLen = 0;
	strValue.Empty();
	memset(szTemp, 0x0, sizeof(szTemp));

	if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_AC, &nLen, szTemp) == EMV_RSLT_OK)
	{
		strValue = CUtil::ConvertHexToString(szTemp, nLen);

		/* Application Cryptogram  Max Len 8 */
		if( nLen > 0 && nLen <= 8 )
		{
			/* Tag + Length + Value */
			strTagLenValue.Format(_T("%S%02X%s"), EMVTag_AC, nLen, strValue);

			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_9F26 = strTagLenValue;
		}
	}

	// Cryptogram Information Data (9F27)
	nLen = 0;
	strValue.Empty();
	memset(szTemp, 0x0, sizeof(szTemp));

	if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_CryptInfData, &nLen, szTemp) == EMV_RSLT_OK)
	{
		strValue = CUtil::ConvertHexToString(szTemp, nLen);

		/* Cryptogram Information Data Max Len 1 */
		if( nLen > 0 && nLen <= 1 )
		{
			/* Tag + Length + Value */
			strTagLenValue.Format(_T("%S%02X%s"), EMVTag_CryptInfData, nLen, strValue);

			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_9F27 = strTagLenValue;
		}
	}

	// Terminal Capabilities (9F33) (Len : 3Byte)
	nLen = 0;
	strValue.Empty();
	memset(szTemp, 0x0, sizeof(szTemp));

	if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_TerminalCapa, &nLen, szTemp) == EMV_RSLT_OK)
	{
		strValue = CUtil::ConvertHexToString(szTemp, nLen);

		/* Terminal Capabilities Max Len 3 */
		if( nLen > 0 && nLen <= 3 )
		{
			/* Tag + Length + Value */
			strTagLenValue.Format(_T("%S%02X%s"), EMVTag_TerminalCapa, nLen, strValue);

			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_9F33 = strTagLenValue;
		}
	}

	// Terminal Type (9F35) (Len : 1Byte)
	nLen = 0;
	strValue.Empty();
	memset(szTemp, 0x0, sizeof(szTemp));

	if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_TerminalType, &nLen, szTemp) == EMV_RSLT_OK)
	{
		strValue = CUtil::ConvertHexToString(szTemp, nLen);

		/* Terminal Type Max Len 1 */
		if( nLen > 0 && nLen <= 1 )
		{
			/* Tag + Length + Value */
			strTagLenValue.Format(_T("%S%02X%s"), EMVTag_TerminalType, nLen, strValue);

			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_9F35 = strTagLenValue;
		}
	}

	// Application Transaction Counter (9F36)
	nLen = 0;
	strValue.Empty();
	memset(szTemp, 0x0, sizeof(szTemp));

	if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_ATC, &nLen, szTemp) == EMV_RSLT_OK)
	{
		strValue = CUtil::ConvertHexToString(szTemp, nLen);

		/* Application Transaction Counter Len 2 */
		if( nLen > 0 && nLen <= 2 )
		{
			/* Tag + Length + Value */
			strTagLenValue.Format(_T("%S%02X%s"), EMVTag_ATC, nLen, strValue);

			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_9F36 = strTagLenValue;
		}
	}

	// Unpredictable Number (9F37)
	nLen = 0;
	strValue.Empty();
	memset(szTemp, 0x0, sizeof(szTemp));

	if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_UnpredictNo, &nLen, szTemp) == EMV_RSLT_OK)
	{
		strValue = CUtil::ConvertHexToString(szTemp, nLen);

		/* Unpredictable Number Max Len 4 */
		if( nLen > 0 && nLen <= 4 )
		{
			/* Tag + Length + Value */
			strTagLenValue.Format(_T("%S%02X%s"), EMVTag_UnpredictNo, nLen, strValue);

			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_9F37 = strTagLenValue;
		}
	}

	// Point Of Service Entry Mode Code [POS -> 0x05 Fixed] (9F39) (Len : 1Byte)
	nLen = 0;
	strValue.Empty();
	memset(szTemp, 0x0, sizeof(szTemp));

	if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_POSEntryMode, &nLen, szTemp) == EMV_RSLT_OK)
	{
		strValue = CUtil::ConvertHexToString(szTemp, nLen);

		/* Point Of Service Entry Mode Code Max Len 1 */
		if( nLen > 0 && nLen <= 1 )
		{
			/* Tag + Length + Value */
			strTagLenValue.Format(_T("%S%02X%s"), EMVTag_POSEntryMode, nLen, strValue);

			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_9F39 = strTagLenValue;
		}
	}

	// transaction sequence counter (9F41) (Len : 4Byte)
	nLen = 0;
	strValue.Empty();
	memset(szTemp, 0x0, sizeof(szTemp));

	if (CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_GetValue(EMVTag_TrSeqCnt, &nLen, szTemp) == EMV_RSLT_OK)
	{
		strValue = CUtil::ConvertHexToString(szTemp, nLen);

		/* transaction category code Max Len 4 */
		if( nLen > 0 && nLen <= 4 )
		{
			/* Tag + Length + Value */
			strTagLenValue.Format(_T("%S%02X%s"), EMVTag_TrSeqCnt, nLen, strValue);

			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTLV_9F41 = strTagLenValue;
		}
	}

	return 0;
}

/** **********************************************************
*	@brief		Host 수신 Data Parsing
*	@param		None
*	@return		정상 - 0, 실패 - (-1)
************************************************************/
int	CEagleTritonMsg::Triton_AnalHostData(int nTranType)
{
	int nRetun = -1;

	// Receive message 미 수신시 예외 처리
	if (strlen((char*)m_pRecvBuffer) <= 0)
	{
		LOG(Error, _T("Triton_AnalHostData - Recv datalength is 0"));
		return -1;
	}

	m_strArrRecvData.RemoveAll();
	m_strCrcResult_16.Empty();

	m_strRecvData = m_pRecvBuffer;
	CUtil::ParsingStringToStringArray(m_strRecvData, (CString)FS, m_strArrRecvData);

	// CRC 검증
	if(CEagleDataManager::GetInstance()->m_Config.m_Host.strCrc_flg == S_ENABLE)		// CRC option is Enabled
	{
		int nCRCIndex = -1;
		int nstartIndex = 0; 

		while(1)
		{
			nCRCIndex = -1;
			nCRCIndex = m_strRecvData.Find(_T("ub"), nstartIndex);

			if ( nCRCIndex == -1)
				break;
			else if ( nCRCIndex > 0 && m_strRecvData.GetAt(nCRCIndex -1) == (TCHAR)0x1C) 
				break; // found
			else 
				nstartIndex += 2;

		} 

		if(nCRCIndex != -1)
		{
			// CRC check
			char chBuffer[NET_RECV_BUFF_SIZE] = { 0, };
			WORD dwtempLen = m_strRecvData.GetLength();

			m_strCrcResult_16 = m_strRecvData.Mid(nCRCIndex+2, 4);

			// CRC 계산을 위해 ubxxxx->ub0000으로 변환한다.
			sprintf(chBuffer, "%S", m_strRecvData);
			memcpy(&chBuffer[nCRCIndex+2], "0000", 4);

			// VISA TYPE을 제외한 나머지 프로토콜은 Length 2바이트를 앞에 붙여서 CRC 계산을 한다.
			if (CEagleDataManager::GetInstance()->m_Config.m_Host.strProtocolType == _T("STANDARD"))
			{
				memmove(&chBuffer[2], &chBuffer[0], dwtempLen);
				chBuffer[0] = (dwtempLen& 0xFF00) >> 8;
				chBuffer[1] = dwtempLen & 0x00FF;
				dwtempLen += 2;
			}

			UINT nCRCResult_16 = CUtil::crc16((BYTE*)chBuffer, dwtempLen);
			CString strCalcCRCResult_16;

			strCalcCRCResult_16.Format(L"%04X", nCRCResult_16);

			if(m_strCrcResult_16.CompareNoCase(strCalcCRCResult_16) != 0)
			{
				LOG(Error, _T("Triton_AnalHostData - CRC mismactch : recv - %s, calc - %s"), m_strCrcResult_16, strCalcCRCResult_16);

				if (Triton_IsReversalCondition())
				{
					CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_nIsReversal = ENABLE_REVERSAL;
					CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strReasonforReversal = REVERSAL_REASON_PROTOCOL_ERROR;
				}

				// CRC Error Set
				CEagleSVCLib::GetInstance()->Set_ErrorCode(ERROR_CRC_MISMATCH, ERROR_DEVICE_NET, CSCR_Manager::GetInstance()->GetAPTextIDString(_T("COMM_ERROR")));

				return -1;	// CRC Error Set
			}
		}
		else
		{
			LOG(Error, _T("Triton_AnalHostData - CRC is not exist"));

			if (Triton_IsReversalCondition())
			{
				CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_nIsReversal = ENABLE_REVERSAL;
				CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strReasonforReversal = REVERSAL_REASON_PROTOCOL_ERROR;
			}

			CEagleSVCLib::GetInstance()->Set_ErrorCode(ERROR_CRC_MISMATCH, ERROR_DEVICE_NET, CSCR_Manager::GetInstance()->GetAPTextIDString(_T("COMM_ERROR")));

			return -1;	// CRC Error Set
		}
	}

	switch (nTranType)
	{
	case EAGLE_TRAN_CODE_DOWNLOAD_CONFIG:
		nRetun = Triton_AnalConfigMsg(0, nTranType);
		break;

	case EAGLE_TRAN_CODE_WITHDRAWAL:
	case EAGLE_TRAN_CODE_TRANSFER:
	case EAGLE_TRAN_CODE_BALANCE:
		nRetun = Triton_AnalTranMsg(0, nTranType);
		break;

	case EAGLE_TRAN_CODE_REVERSAL:
		nRetun = Triton_AnalReversalMsg(0);
		break;

	case EAGLE_TRAN_CODE_HOST_TOTALS:
	case EAGLE_TRAN_CODE_GET_HOST_TOTALS:
		nRetun = Triton_AnalTotalMsg(0, nTranType);
		break;

	default:
		break;
	}

	return nRetun;
}

/** **********************************************************
*	@brief		Config Message 수신 Data Check
*	@param		None
*	@return		정상 - 0, 실패 - (-1)
************************************************************/
int CEagleTritonMsg::Triton_AnalConfigMsg(int nIndex, int nTranType)
{
	int nReturn = -1;

	// 1.Information Header
	if (CEagleDataManager::GetInstance()->m_Config.m_Host.strCommunication_header == S_ENABLE) 
	{
		if (m_strArrRecvData.GetSize() > nIndex)
			m_sTriton_ConfigResp.m_strInformation_Header_12 = m_strArrRecvData[nIndex++];
	}

	// 2.Terminal ID 15Alphanumeric
	if (m_strArrRecvData.GetSize() > nIndex)
		m_sTriton_ConfigResp.m_strTerminalID_15 = m_strArrRecvData[nIndex++];

	// 3.Transaction Code
	if (m_strArrRecvData.GetSize() > nIndex)
		m_sTriton_ConfigResp.m_strTransactionCode_2 = m_strArrRecvData[nIndex++];

	// Response Value Check Logic
	if( m_sTriton_CommHeader.m_strTerminalID_15 != m_sTriton_ConfigResp.m_strTerminalID_15)
	{
		LOG(Error, _T("Triton_AnalConfigMsg - Terminal ID is invalid : Recv(%s)"), m_sTriton_ConfigResp.m_strTerminalID_15);
		return -1;					
	}

	if( m_sTriton_ConfigReq.m_strTransactionCode_2 != m_sTriton_ConfigResp.m_strTransactionCode_2)
	{
		LOG(Error, _T("Triton_AnalConfigMsg - Transaction code is invalid : Recv (%s)"), m_sTriton_ConfigResp.m_strTransactionCode_2);
		return -1;
	}

	Triton_AnalMiscellaneousField(nIndex, nTranType);

	if( m_sTriton_ConfigResp.m_strFieldIDCode1_1 != _T("~") )
	{
		LOG(Error, _T("Triton_AnalConfigMsg - PIN Communication key1 is not received"));
		return -1;
	}

	return SUCCESS;
}

/** **********************************************************
*	@brief		Transaction Message 수신 Data Check
*	@param		None
*	@return		정상 - 0, 실패 - (-1)
************************************************************/
int CEagleTritonMsg::Triton_AnalTranMsg(int nIndex, int nTranType)
{
	int			nResponseCodeIndex = -1;
	int			nEMVResult = 0;
	BOOL		fReversal = FALSE;
	BOOL		fCancel = FALSE;

	// Before checking Transaction Message, Set the Transaction Error by default
	if (CEagleDataManager::GetInstance()->m_Config.m_Host.strCommunication_header == S_ENABLE)
	{
		if (m_strArrRecvData.GetSize() > nIndex)
			m_sTriton_TranResp.m_strInformation_Header_12 = m_strArrRecvData[nIndex++];
	}

	// Multi-Block Indicator
	if (m_strArrRecvData.GetSize() > nIndex)
		m_sTriton_TranResp.m_strMulti_Block_Indicator_1 = m_strArrRecvData[nIndex++];

	// Terminal ID
	if (m_strArrRecvData.GetSize() > nIndex)
		m_sTriton_TranResp.m_strTerminalID_15 = m_strArrRecvData[nIndex++];

	// Transaction Code
	if (m_strArrRecvData.GetSize() > nIndex)
		m_sTriton_TranResp.m_strTransactionCode_2 = m_strArrRecvData[nIndex++];

	// Sequence Number 
	if (m_strArrRecvData.GetSize() > nIndex)
		m_sTriton_TranResp.m_strSequenceNumber_4 = m_strArrRecvData[nIndex++];

	// Transaction Response Code
	if (m_strArrRecvData.GetSize() > nIndex)
		m_sTriton_TranResp.m_strResponseCode_3 = m_strArrRecvData[nIndex++];

	// Authorization Number
	if (m_strArrRecvData.GetSize() > nIndex)
	{
		m_sTriton_TranResp.m_strAuthorizationNum_8 = m_strArrRecvData[nIndex++];
		CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strAutorizationNumber = m_sTriton_TranResp.m_strAuthorizationNum_8;
	}

	// Transaction Date MMDDYY
	if (m_strArrRecvData.GetSize() > nIndex)
	{
		m_sTriton_TranResp.m_strTransactionDate_6 = m_strArrRecvData[nIndex++];
		CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransactionDate = m_sTriton_TranResp.m_strTransactionDate_6;
	}
	// Transaction Time HHMMSS  TransactionTime
	if (m_strArrRecvData.GetSize() > nIndex)
	{
		m_sTriton_TranResp.m_strTransactionTime_6 = m_strArrRecvData[nIndex++];
		CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransactionTime = m_sTriton_TranResp.m_strTransactionTime_6;
	}

	// BUSINSES DATE MMDDYY
	if (m_strArrRecvData.GetSize() > nIndex)
	{
		m_sTriton_TranResp.m_strBusinessDate_6 = m_strArrRecvData[nIndex++];
		CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strBusinessDate = m_sTriton_TranResp.m_strBusinessDate_6;
	}

	// Amount1 Balance amount
	if (m_strArrRecvData.GetSize() > nIndex)
	{
		m_sTriton_TranResp.m_strAmount1_8 = m_strArrRecvData[nIndex++];
		CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strBalance = m_sTriton_TranResp.m_strAmount1_8;
	}

	// Amount2 Amount of Actual Surcharge
	// An ASCII minus sign (e.g."-0000100") in the first position represents a credit.
	if (m_strArrRecvData.GetSize() > nIndex)
	{
		m_sTriton_TranResp.m_strAmount2_8 = m_strArrRecvData[nIndex++];
		CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strSurchargeAmount = m_sTriton_TranResp.m_strAmount2_8;
	}

	if (m_strArrRecvData.GetSize() > nIndex)
	{
		nEMVResult = Triton_AnalMiscellaneousField(nIndex, nTranType);
	}

	if (CEagleDataManager::GetInstance()->m_Client_Info.m_bEMV_Transaction == TRUE)
	{
		if (m_strRecvData.Find(_T("ud")) == -1)			// Host로부터 EMV Data 미 수신시 처리
		{
			LOG(Error, _T("Triton_AnalTranMsg - Transaction is declied (missing emv data field)"));

			if (CEagleDataManager::GetInstance()->m_Client_Info.m_bExec2ndGAC == FALSE)
			{
				CEagleDataManager::GetInstance()->m_Client_Info.m_bExec2ndGAC = TRUE;

				// EMV 필드가 아예 오지 않을 경우에 대한 IC 처리
				CString strDefaultEMVData = _T("8A023030910A00000000000000000000");			// MTIP 06 / 05 / 01 Test Case (8A:3030, 91:00000000000000000000)

				Triton_Anal_EMV_ResponseData(strDefaultEMVData);
			}

			CEagleSVCLib::GetInstance()->Set_ErrorCode(ERROR_EMV_MISSING_DATA, ERROR_DEVICE_NET, CSCR_Manager::GetInstance()->GetAPTextIDString(GETTEXT_014));

			if( m_sTriton_TranResp.m_strResponseCode_3 == _T("000"))
			{
				if (Triton_IsReversalCondition())
				{
					CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_nIsReversal = ENABLE_REVERSAL;
					CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strReasonforReversal = REVERSAL_REASON_PROTOCOL_ERROR;
				}
			}

			return -1;
		}
	}

	// Response Message Error Checking
	// Transaction Response Code Error Check
	if( m_sTriton_TranResp.m_strResponseCode_3 != _T("000"))
	{
		LOG(Error, _T("Triton_AnalTranMsg - Transaction is declined (Invalid response code)"));

		CString strErrorCode, strErrorMsgID, strErrorMsg;

		strErrorCode = m_sTriton_TranResp.m_strResponseCode_3;

		if (strErrorCode.IsEmpty() == TRUE)
		{
			// Host에서 수신받으 Response Code가 NULL일 경우에 대해 임의로 에러코드 생성
			strErrorCode = ERROR_INVALID_RESPCODE;
			strErrorMsg = CSCR_Manager::GetInstance()->GetAPTextIDString(_T("NOT_DEFINED_DECLINE_CODE"));
		}
		else
		{
			strErrorMsgID.Format(_T("DECLINE_CODE_%s"), strErrorCode);

			strErrorMsg = CSCR_Manager::GetInstance()->GetAPTextIDString(strErrorMsgID);

			if (strErrorMsg.IsEmpty() || (strErrorMsg == strErrorMsgID))	// Text String을 못가져올 경우에 대한 처리 보완
				strErrorMsg = CSCR_Manager::GetInstance()->GetAPTextIDString(_T("NOT_DEFINED_DECLINE_CODE"));

			strErrorCode.Insert(0, ERROR_HOST_DECLINED_CODE);	// 앞에 Z3H코드 추가
		}

		CEagleSVCLib::GetInstance()->Set_ErrorCode(strErrorCode, ERROR_DEVICE_NET, strErrorMsg);

		return -1;
	}

	// Terminal ID Check
	if( m_sTriton_CommHeader.m_strTerminalID_15 != m_sTriton_TranResp.m_strTerminalID_15)
	{
		LOG(Error, _T("Triton_AnalTranMsg - Terminal ID is invalid : Recv (%s)"), m_sTriton_TranResp.m_strTerminalID_15);

		CEagleSVCLib::GetInstance()->Set_ErrorCode(ERROR_INVALID_TERMIANID, ERROR_DEVICE_NET, CSCR_Manager::GetInstance()->GetAPTextIDString(GETTEXT_014));

		if (Triton_IsReversalCondition())
		{
			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_nIsReversal = ENABLE_REVERSAL;
			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strReasonforReversal = REVERSAL_REASON_PROTOCOL_ERROR;
		}
			
		return -1;
	}

	// Transaction Code Error Check
	if( m_sTriton_TranReq.m_strTransactionCode_2 != m_sTriton_TranResp.m_strTransactionCode_2)
	{
		LOG(Error, _T("Triton_AnalTranMsg - Transaction code is invalid : Recv (%s)"), m_sTriton_TranResp.m_strTransactionCode_2);

		CEagleSVCLib::GetInstance()->Set_ErrorCode(ERROR_INVALID_TRANCODE, ERROR_DEVICE_NET, CSCR_Manager::GetInstance()->GetAPTextIDString(GETTEXT_014));

		if (Triton_IsReversalCondition())
		{
			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_nIsReversal = ENABLE_REVERSAL;
			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strReasonforReversal = REVERSAL_REASON_PROTOCOL_ERROR;
		}

		return -1;
	}

	// Sequence Number Error Check
	if( m_sTriton_TranReq.m_strSequenceNumber_4 != m_sTriton_TranResp.m_strSequenceNumber_4 )
	{
		LOG(Error, _T("Triton_AnalTranMsg - Sequence number is invalid : Recv (%s)"), m_sTriton_TranResp.m_strSequenceNumber_4);

		CEagleSVCLib::GetInstance()->Set_ErrorCode(ERROR_INVALID_SEQNO, ERROR_DEVICE_NET, CSCR_Manager::GetInstance()->GetAPTextIDString(GETTEXT_014));

		if (Triton_IsReversalCondition())
		{
			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_nIsReversal = ENABLE_REVERSAL;
			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strReasonforReversal = REVERSAL_REASON_PROTOCOL_ERROR;
		}

		return -1;
	}

	// Authorization Number Error Check
	if( m_sTriton_TranResp.m_strAuthorizationNum_8.GetLength() != 8 )
	{
		LOG(Error, _T("Triton_AnalTranMsg - Autorization number is invalid : Recv (%s)"), m_sTriton_TranResp.m_strAuthorizationNum_8);

		CEagleSVCLib::GetInstance()->Set_ErrorCode(ERROR_INVALID_AUTHNO, ERROR_DEVICE_NET, CSCR_Manager::GetInstance()->GetAPTextIDString(GETTEXT_014));

		if (Triton_IsReversalCondition())
		{
			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_nIsReversal = ENABLE_REVERSAL;
			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strReasonforReversal = REVERSAL_REASON_PROTOCOL_ERROR;
		}

		return -1;
	}

	// Transaction Date MMDDYY (Set by Processor) Error Check
	if( m_sTriton_TranResp.m_strTransactionDate_6.GetLength() != 6 )
	{
		LOG(Error, _T("Triton_AnalTranMsg - Transaction date is invalid"));

		CEagleSVCLib::GetInstance()->Set_ErrorCode(ERROR_INVALID_TRANDATE, ERROR_DEVICE_NET, CSCR_Manager::GetInstance()->GetAPTextIDString(GETTEXT_014));

		if (Triton_IsReversalCondition())
		{
			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_nIsReversal = ENABLE_REVERSAL;
			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strReasonforReversal = REVERSAL_REASON_PROTOCOL_ERROR;
		}

		return -1;
	}

	if( m_sTriton_TranResp.m_strTransactionTime_6.GetLength() != 6 )
	{
		LOG(Error, _T("Triton_AnalTranMsg - Transaction time is invalid"));

		CEagleSVCLib::GetInstance()->Set_ErrorCode(ERROR_INVALID_TRANDTIME, ERROR_DEVICE_NET, CSCR_Manager::GetInstance()->GetAPTextIDString(GETTEXT_014));

		if (Triton_IsReversalCondition())
		{
			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_nIsReversal = ENABLE_REVERSAL;
			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strReasonforReversal = REVERSAL_REASON_PROTOCOL_ERROR;
		}

		return -1;
	}

	// BUSINSES DATE Error Check MMDDYY
	if( m_sTriton_TranResp.m_strBusinessDate_6.GetLength() != 6 )
	{
		LOG(Error, _T("Triton_AnalTranMsg - Business date is invalid"));

		CEagleSVCLib::GetInstance()->Set_ErrorCode(ERROR_INVALID_BUSSDATE, ERROR_DEVICE_NET, CSCR_Manager::GetInstance()->GetAPTextIDString(GETTEXT_014));

		if (Triton_IsReversalCondition())
		{
			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_nIsReversal = ENABLE_REVERSAL;
			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strReasonforReversal = REVERSAL_REASON_PROTOCOL_ERROR;
		}

		return -1;
	}

	if (CEagleDataManager::GetInstance()->m_Client_Info.m_bEMV_Transaction == TRUE)
	{
		if (nEMVResult != EMV_RSLT_OK)
		{
			LOG(Error, _T("Triton_AnalTranMsg - [EMV] IC Declined (%d)"), nEMVResult);

			CEagleSVCLib::GetInstance()->Set_ErrorCode(ERROR_EMV_IC_DECLINED, ERROR_DEVICE_NET, CSCR_Manager::GetInstance()->GetAPTextIDString(GETTEXT_015));

			if (Triton_IsReversalCondition())
			{
				CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_nIsReversal = ENABLE_REVERSAL;
				CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strReasonforReversal = REVERSAL_REASON_ICC_DECLINED;
			}

			return -1;
		}
	}

	return SUCCESS;
}

/** **********************************************************
*	@brief			Reversal Message 수신 Data Check
*	@param			없음
*	@return			정상 - 0, 실패 - (-1)
************************************************************/
int CEagleTritonMsg::Triton_AnalReversalMsg(int nIndex)
{
	// 1.Information Header
	if (CEagleDataManager::GetInstance()->m_Config.m_Host.strCommunication_header == S_ENABLE)
		nIndex++;

	// Terminal ID
	if (m_strArrRecvData.GetSize() > nIndex)
		m_sTriton_ReversalResp.m_strTerminalID_15 = m_strArrRecvData[nIndex++];

	// Transaction Code
	if (m_strArrRecvData.GetSize() > nIndex)
		m_sTriton_ReversalResp.m_strTransactionCode_2 = m_strArrRecvData[nIndex++];	

	// Sequence Number
	if (m_strArrRecvData.GetSize() > nIndex)
		m_sTriton_ReversalResp.m_strSequenceNumber_4 = m_strArrRecvData[nIndex++];	

	// Transaction Response Code
	if (m_strArrRecvData.GetSize() > nIndex)
		m_sTriton_ReversalResp.m_strResponseCode_3 = m_strArrRecvData[nIndex++];

	if( m_sTriton_ReversalResp.m_strResponseCode_3 != _T("000") )
	{
		LOG(Error, _T("Reversal is declied (Invalid response code)"));

		CString strErrorCode, strErrorMsgID, strErrorMsg;

		strErrorCode = m_sTriton_ReversalResp.m_strResponseCode_3;

		if (strErrorCode.IsEmpty() == TRUE)
		{
			// Host에서 수신받으 Response Code가 NULL일 경우에 대해 임의로 에러코드 생성
			strErrorCode = ERROR_INVALID_RESPCODE;
			strErrorMsg = CSCR_Manager::GetInstance()->GetAPTextIDString(_T("NOT_DEFINED_DECLINE_CODE"));
		}
		else
		{

			strErrorMsgID.Format(_T("DECLINE_CODE_%s"), strErrorCode);

			strErrorMsg = CSCR_Manager::GetInstance()->GetAPTextIDString(strErrorMsgID);

			if (strErrorMsg.IsEmpty() || (strErrorMsg == strErrorMsgID))
				strErrorMsg = CSCR_Manager::GetInstance()->GetAPTextIDString(_T("NOT_DEFINED_DECLINE_CODE"));

			strErrorCode.Insert(0, ERROR_HOST_DECLINED_CODE);	// 앞에 Z3H코드 추가
		}

		CEagleSVCLib::GetInstance()->Set_ErrorCode(strErrorCode, ERROR_DEVICE_NET, strErrorMsg);
		
		return -1;
	}


	if( m_sTriton_CommHeader.m_strTerminalID_15 != m_sTriton_ReversalResp.m_strTerminalID_15)
	{
		LOG(Error, _T("Triton_AnalReversalMsg - Terminal ID is invalid"));

		CEagleSVCLib::GetInstance()->Set_ErrorCode(ERROR_INVALID_TERMIANID, ERROR_DEVICE_NET);
		
		return -1;
	}

	if( m_sTriton_ReversalReq.m_strTransactionCode_2 != m_sTriton_ReversalResp.m_strTransactionCode_2 )
	{
		LOG(Error, _T("Triton_AnalReversalMsg - Transaction code is invalid"));

		CEagleSVCLib::GetInstance()->Set_ErrorCode(ERROR_INVALID_TRANCODE, ERROR_DEVICE_NET);

		return -1;
	}

	if( m_sTriton_ReversalReq.m_strSequenceNumber_4 != m_sTriton_ReversalResp.m_strSequenceNumber_4)
	{
		LOG(Error, _T("Triton_AnalReversalMsg - Sequence number is invalid"));

		CEagleSVCLib::GetInstance()->Set_ErrorCode(ERROR_INVALID_SEQNO, ERROR_DEVICE_NET);

		return -1;
	}

	Triton_AnalMiscellaneousField(nIndex, EAGLE_TRAN_CODE_REVERSAL);

	return 0;
}

/** **********************************************************
*	@brief		Day Total Message 수신 Data Check
*	@param		None
*	@return		정상 - 0, 실패 - (-1)
************************************************************/
int CEagleTritonMsg::Triton_AnalTotalMsg(int nIndex, int nTranType)
{
	// 1.Information Header
	if (CEagleDataManager::GetInstance()->m_Config.m_Host.strCommunication_header == S_ENABLE) 
		nIndex++;

	// Terminal ID Check
	if (m_strArrRecvData.GetSize() > nIndex)
		m_sTriton_TotalResp.m_strTerminalID_15 = m_strArrRecvData[nIndex++];

	// Transaction Code
	if (m_strArrRecvData.GetSize() > nIndex)
		m_sTriton_TotalResp.m_strTransactionCode_2 = m_strArrRecvData[nIndex++];	

	// BUSINESS DATE
	if (m_strArrRecvData.GetSize() > nIndex)
		m_sTriton_TotalResp.m_strBusinessDate_6 = m_strArrRecvData[nIndex++];	

	// Number of Withdrawals : Total number of withdrawals
	// since the last request for totals.
	if (m_strArrRecvData.GetSize() > nIndex)
	{
		m_sTriton_TotalResp.m_strNumberOfWithdrawals_4 = m_strArrRecvData[nIndex].Mid(0, 4);
		m_sTriton_TotalResp.m_strNumberOfInquiries_4 = m_strArrRecvData[nIndex].Mid(4, 4);
		m_sTriton_TotalResp.m_strNumberOfTransfers_4 = m_strArrRecvData[nIndex].Mid(8, 4);
		m_sTriton_TotalResp.m_strSettlement_8 = m_strArrRecvData[nIndex++].Mid(12, 8);
	}

	if( m_sTriton_CommHeader.m_strTerminalID_15 != m_sTriton_TotalResp.m_strTerminalID_15)
	{
		LOG(Error, _T("Triton_AnalTotalMsg - Terminal ID is invalid"));

		return -1;
	}

	if( m_sTriton_TotalReq.m_strTransactionCode_2 != m_sTriton_TotalResp.m_strTransactionCode_2 )
	{
		LOG(Error, _T("Triton_AnalTotalMsg - Transaction code is invalid"));

		return -1;
	}

	if( m_sTriton_TotalResp.m_strBusinessDate_6.GetLength() != 6 )
	{
		LOG(Error, _T("Triton_AnalTotalMsg - Business date is invalid"));

		return -1;
	}

	if (CUtil::IsNumeric(m_sTriton_TotalResp.m_strSettlement_8) != TRUE )
	{
		LOG(Error, _T("Triton_AnalTotalMsg - Settlement is invalid"));

		return -1;
	}

	Triton_AnalMiscellaneousField(nIndex, nTranType);

	return 0;
}

/** **********************************************************
*	@brief		Misc Field 수신 Data 처리
*	@param		None
*	@return		정상 - 0, 실패 - (-1)
************************************************************/
int CEagleTritonMsg::Triton_AnalMiscellaneousField(int nCurrentIndex, int nTranType)
{
	int nIndex = 0;
	int nMisc_Index = 0, nKey2Index = 0, i = 0;
	UINT nMisc_FID2 = 0, nTmp = 0;
	BYTE chMisc = 0x00;
	int nEMVResult = 0;

	CString strMisc, strTemp, strTemp2, strTemp3, strLog;

	BOOL bReceivedFirstWorkingKey = FALSE;
	BOOL bReceivedSecondWorkingKey = FALSE;

	UINT	nFID2 = 0;

	switch (nTranType)
	{
	case EAGLE_TRAN_CODE_DOWNLOAD_CONFIG:		// Configuration Message
		nMisc_Index = MISC_RESP_CONFIG;
		break;

	case EAGLE_TRAN_CODE_WITHDRAWAL:
	case EAGLE_TRAN_CODE_TRANSFER:
	case EAGLE_TRAN_CODE_BALANCE:				// Transaction Message
		nMisc_Index = MISC_RESP_TRAN;
		break;

	case EAGLE_TRAN_CODE_REVERSAL:				// Reversal Message
		nMisc_Index = MISC_RESP_REVERSAL;
		break;

	case EAGLE_TRAN_CODE_HOST_TOTALS:			// HOST Total Message
	case EAGLE_TRAN_CODE_GET_HOST_TOTALS:
		nMisc_Index = MISC_RESP_TOTAL;
		break;

	default:
		return -1;
	}

	LOG(Info, _T("Triton Misc Received - Start"));

	for ( nIndex = nCurrentIndex; nIndex < m_strArrRecvData.GetSize(); nIndex++,i = 0 )
	{
		strMisc = m_strArrRecvData[nIndex];

		if( strMisc.IsEmpty())
			continue;

		chMisc = (BYTE)strMisc.GetAt(0);

		while(chMisc_Responses[nMisc_Index][i] != 0x00)
		{
			// search current table
			if( chMisc == (BYTE)chMisc_Responses[nMisc_Index][i])
				break;

			i++;

			// search next table
			if( chMisc_Responses[nMisc_Index][i] == 0x00)
				chMisc = 0x00;
		}

		switch( chMisc ) 
		{
			// Surcharge amount to be displayed to customer during Surcharge Notification screen
			// 8 Numeric ASCII ex) 00032767 ! CAUTION cannot be above 00032767 
		case FD_SRCH_AMT:
			{
				CEagleDataManager::GetInstance()->m_Client_Info.m_bRecvConfigInfo = TRUE;

				strTemp = strMisc.Mid(1);

				// Length Check는 하지 않음
				//if( strTemp.GetLength() != 8)			break;

				// Spec상 Numeric인지 여부 Check 필요
				if (CUtil::IsNumeric(strTemp) == FALSE)
				{
					LOG(Error, _T("Triton_AnalMiscellaneousField - Surcharge Amount is invalid (%s)"), strTemp);
					break;
				}

				m_sTriton_ConfigResp.m_strSurchargeAmount_8 = strTemp;
				m_sTriton_ConfigResp.m_strFieldIDCode3_1 = _T("!");

				
				CEagleDataManager::GetInstance()->m_Config.m_Option.strWithdrawalSurcharge_amount.Format(_T("%03d"), CUtil::StringToInt(strTemp));	// cent 포함된 금액임
				//int nFeeAmount = CUtil::StringToInt(strTemp);
				//if (nFeeAmount == 0)
				//	CEagleDataManager::GetInstance()->m_Config.m_Option.strWithdrawalSurcharge_amount = _T("000");
				//else
				//	CEagleDataManager::GetInstance()->m_Config.m_Option.strWithdrawalSurcharge_amount.Format(_T("%d"), nFeeAmount);	// cent 포함된 금액임
			}
			break;

			// 12 Digit (alphanumeric) Sequence Number
		case FD_EXT_SEQ_NUM:
			break;

			// 28 Digit Account Number(i.e. Checking,Savings,etc) Left justified, padded with spaces.
		case FD_ACCNT_NUM:
			break;

			// MAC enable or disable field. 1ASCII 0:Disable 1:Enable
		case FD_MAC_EN_DIS:
			// 현재 MAC은 CA / AU만 사용하므로 추후 구현
			//strTemp = strMisc.Mid(1);

			//// Spec상 Numeric인지 여부 Check 필요
			//if (CUtil::IsNumeric(strTemp) == FALSE)
			//{
			//	LOG(Error, _T("MAC en/disable is invalid (%s)"), strTemp);
			//	break;
			//}
			break;

			// PIN Change PIN Block. 16 alphanumeric. ANSI standard encrypted PIN block.
		case FD_CHG_PIN_BLOCK:
			break;

			// Terminal is Triple-DES capable (if 2: Terminal is Triple DES - '[2', else 1: - '[0')
		case FD_ENCRYPT_MODE:
			break;

			// MAC(Message Authentication Code) Result. 9ASCII characters	^A423 34CD
		case FD_MAC_BLCK:
			// Mac result 는 데이터 수신시에 처음으로 계산된다. 여기서 계산하지 않음.
			break;

		case FD_PIN_COMM_KEY_2:
			strTemp = strMisc.Mid(1);

			m_sTriton_ConfigResp.m_strEncryptedPINKey2_16 = strTemp;
			m_sTriton_ConfigResp.m_strFieldIDCode2_1 = _T("{");

			bReceivedSecondWorkingKey = TRUE;
			m_bSecondKeyReceived = TRUE;
			break;

			// DES Communications key(PIN Working Key). 16ASCII characters long, encrypted under Master key.
			// Will be used starting with the next transaction. 
			// PIN Working Key 1
		case FD_PIN_COMM_KEY_1:
			strTemp = strMisc.Mid(1);

			m_sTriton_ConfigResp.m_strEncryptedPINKey_16 = strTemp;
			m_sTriton_ConfigResp.m_strFieldIDCode1_1 = _T("~");

			bReceivedFirstWorkingKey = TRUE;
			m_bFirstKeyReceived = TRUE;
			break;

			// MAC Working Key Right Block for Triple-DES capable terminals
		case FD_MAC_COMM_KEY_2:
			// Working Key1. This Key is encrypted by the PIN Master Key 
			// before being transmitted to the terminal
			break;

		case 'a':
			break;

		case FD_SUPPLEMENT_BAL:
			strTemp = strMisc.Mid(1);

			// 'b' field 사양 : "b00000000___________" -> space를 뺀 numeric은 8byte임.
			strTemp.TrimLeft();
			strTemp.TrimRight();

			if (strTemp.GetAt(0) != '-')
			{
				if (CUtil::IsNumeric(strTemp) == TRUE)
				{
					m_sTriton_TranResp.m_strAvailableBalance_8 = strTemp;
					CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strAvailableBalance = strTemp;
				}
				else
				{
					LOG(Error, _T(" Triton_AnalMiscellaneousField - b field is invalid (%s)"), strTemp);
				}
			}
			else
			{
				// 마이너스 처리
				strTemp2 = strTemp.Mid(1);
				if (CUtil::IsNumeric(strTemp2) == TRUE)
				{
					m_sTriton_TranResp.m_strAvailableBalance_8 = strTemp;	// Minus를 포함한 금액으로 Setting
					CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strAvailableBalance = strTemp;
				}
			}
			break;

		case FD_SCHDL_BAL:
			{
				// Day Total HHMM Setting
				strTemp = strMisc.Mid(1);
				
				if (strTemp.GetLength() != 4)
					break;

				if (CUtil::IsNumeric(strTemp) == FALSE)	
					break;

				int	nHour = CUtil::StringToInt(strTemp.Left(2));
				int nMin  = CUtil::StringToInt(strTemp.Right(2));

				if (nHour < 0 || nHour >= 24)
				{
					LOG(Error, _T("Triton_AnalMiscellaneousField - c field is invalid (%s)"), strTemp);
					break;
				}

				if (nMin < 0 || nMin >= 60)
				{
					LOG(Error, _T("Triton_AnalMiscellaneousField - c field is invalid (%s)"), strTemp);
					break;
				}

				// Option에 Setting
				CEagleDataManager::GetInstance()->m_Config.m_Option.strDayCloseTime = strTemp;
			}
			break;

		case FD_MSG:
			{
				CEagleDataManager::GetInstance()->m_Client_Info.m_bRecvConfigInfo = TRUE;

				strLog = strMisc;
				strLog.Replace(_T("%"), _T("."));
				strLog.Replace(_T("&"), _T(".."));

				// 'd' + type + line
				if( strMisc.GetLength() < 3)
				{
					LOG(Error, _T("Triton_AnalMiscellaneousField - d field is invalid (%s)"), strLog);
					break;
				}

				chMisc = (BYTE)strMisc.GetAt(1);
				nMisc_FID2 = chMisc_Responses[nMisc_Index][i] & 0xFFFFFF00;
				if( (Triton_GetFID(chMisc) &  nMisc_FID2) != Triton_GetFID(chMisc))
					break;

				int i = 0, nIndex = 0, nMsgLen = 0;
				int nLineCount = strMisc.GetAt(2) - 0x30;	// Spec상 Max Line은 ASC '1'~'4'값이므로 Int 변환을 위해 0x30을 뺌

				// 1~4까지의 range check
				if (nLineCount < 1 || nLineCount > 4)
				{
					LOG(Error, _T("Triton_AnalMiscellaneousField - d field is invalid Line (%d) (%s)"), nLineCount, strLog);
					break;
				}

				// Get Message Data & Len
				strTemp	= strMisc.Mid(3);
				nMsgLen = strTemp.GetLength();

				// 's' store message(max: 4 lines)
				if (chMisc == FD_MSG_LOCATION )
				{
					// 기존 Message 초기화 후 update 수행
					for(i = 0; i<4; i++)
						CEagleDataManager::GetInstance()->m_Config.m_Message.strLocation_message[i].Empty();

					for(i=0; i<nLineCount; i++)
					{
						if ((nMsgLen - nIndex) <= 0)	break;

						strTemp2 = strTemp.Mid(nIndex, 40);

						if (strTemp2.GetLength() > 0)
						{
							// Setup
							CEagleDataManager::GetInstance()->m_Config.m_Message.strLocation_message[i] = strTemp2;
							nIndex += strTemp2.GetLength();
						}
					}
				}
				// 'm' marketing message(max: 4 lines)
				else if(chMisc == FD_MSG_MARKETING)
				{
					for(i=0; i<4; i++)
						CEagleDataManager::GetInstance()->m_Config.m_Message.strMarketing_message[i].Empty();

					for(i=0; i<nLineCount; i++)
					{
						if ((nMsgLen - nIndex) <= 0)
							break;

						strTemp2 = strTemp.Mid(nIndex, 40);

						if (strTemp2.GetLength() > 0)
						{
							// Setup
							CEagleDataManager::GetInstance()->m_Config.m_Message.strMarketing_message[i] = strTemp2;
							nIndex += strTemp2.GetLength();
						}
					}
				}
				// 'w' welcome message(max: 3 lines)
				else if(chMisc == FD_MSG_ATTRACT)
				{
					if (nLineCount > 3)
					{
						LOG(Error, _T("Triton_AnalMiscellaneousField - dw field is invalid Line Count (%d) (%s)"), nLineCount, strLog);
						break;
					}

					for(i=0; i<3; i++)
						CEagleDataManager::GetInstance()->m_Config.m_Message.strAttraction_message[i].Empty();

					for(i=0; i<nLineCount; i++)
					{
						if ((nMsgLen - nIndex) <= 0)	break;

						strTemp2 = strTemp.Mid(nIndex, 30);

						if (strTemp2.GetLength() > 0)
						{
							// Setup
							CEagleDataManager::GetInstance()->m_Config.m_Message.strAttraction_message[i] = strTemp2;
							nIndex += strTemp2.GetLength();
						}
					}
				}
				// 'e' exit message(max: 3 lines)
				else if(chMisc == FD_MSG_FAREWELL)
				{
					if (nLineCount > 3)
					{
						LOG(Error, _T("Triton_AnalMiscellaneousField - de field is invalid Line Count (%d) (%s)"), nLineCount, strLog);
						break;
					}

					for(i=0; i<3; i++)
						CEagleDataManager::GetInstance()->m_Config.m_Message.strFarewell_message[i].Empty();

					for(i=0; i<nLineCount; i++)
					{
						if ((nMsgLen - nIndex) <= 0)	break;

						strTemp2 = strTemp.Mid(nIndex, 30);

						if (strTemp2.GetLength() > 0)
						{
							CEagleDataManager::GetInstance()->m_Config.m_Message.strFarewell_message[i] = strTemp2;
							nIndex += strTemp2.GetLength();
						}
					}
				}
			}
			break;

		case 'e':
			strTemp = strMisc.Mid(1);

			strTemp.TrimLeft();
			strTemp.TrimRight();

			// Length Check는 무시

			if (nTranType == EAGLE_TRAN_CODE_WITHDRAWAL || nTranType == EAGLE_TRAN_CODE_BALANCE || nTranType == EAGLE_TRAN_CODE_TRANSFER)
			{
				if (strTemp.GetAt(0) != '-')
				{
					if (CUtil::IsNumeric(strTemp) == TRUE)
					{
						m_sTriton_TranResp.m_strAmount1_Ext12 = strTemp;
						CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strBalance = strTemp;
					}
					else
					{
						LOG(Error, _T("Triton_AnalMiscellaneousField - e field is invalid (%s)"), strMisc);
					}
				}
				else
				{
					strTemp2 = strTemp.Mid(1);		// "-"값 제거
					if (CUtil::IsNumeric(strTemp2) == TRUE)
					{
						m_sTriton_TranResp.m_strAmount1_Ext12 = strTemp2;
						CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strBalance = strTemp2;
					}
					else
					{
						LOG(Error, _T("Triton_AnalMiscellaneousField - e field is invalid (%s)"), strMisc);
					}
				}
			}
			else
			{
				if (nTranType == EAGLE_TRAN_CODE_HOST_TOTALS || nTranType == EAGLE_TRAN_CODE_GET_HOST_TOTALS)
				{
					if (CUtil::IsNumeric(strTemp) == TRUE)
					{
						m_sTriton_TotalResp.m_strSettlement_Ext12 = strTemp;
					}
					else
					{
						LOG(Error, _T("Triton_AnalMiscellaneousField - e field is invalid (%s)"), strMisc);
					}
				}
				else
				{
					LOG(Error, _T("Triton_AnalMiscellaneousField - [e field] Transaction Type is invalid (%d)"), nTranType);
				}
			}
			break;

		case 'f':
			break;

		case 'g':
			break;

		case 'h':
			CEagleDataManager::GetInstance()->m_Client_Info.m_bRecvConfigInfo = TRUE;

			strTemp = strMisc.Mid(1);
			CEagleDataManager::GetInstance()->m_Config.m_Host.strHeartbeat_frequency.Format(_T("%d"), CUtil::StringToInt(strTemp));
			break;

		case 'i':
			break;

		case 'j':
			break;

		case 'k':
			break;

		case 'l':
			break;

			// MAC Working Key Left Block for Triple-DES capable terminals
		case 'm':
			break;

		case 'n':
			break;

		case 'o':
			break;

		case 'p':
			// Printing Data Set
			{
				// 1. Get Line Count (Spec상 1~4)
				strTemp = strMisc.Mid(1);

				strLog = strMisc;
				strLog.Replace(_T("%"), _T("."));
				strLog.Replace(_T("&"), _T(".."));

				CStringArray	strTempArray;
				int nLineCnt = strTemp.GetAt(0) - 0x30;	// Bug Fix (Spec상 '1' ~ '4' 이므로 ASCII값 변환을 위해 -0x30처리

				if (nLineCnt < 1 || nLineCnt > 4)
				{
					LOG(Error, _T("Triton_AnalMiscellaneousField - p field is invalid LineCount (%d) (%s)"), nLineCnt, strLog);
					break;
				}

				// 2. Get Message Data
				strTemp = strTemp.Mid(1);	// Line Count 제거

				// 3. Separator
				CUtil::ParsingStringToStringArray(strTemp, (CString)CR, strTempArray);

				if (strTempArray.GetSize() < 1)
				{
					LOG(Error, _T("Triton_AnalMiscellaneousField - p field is invalid Size Error (%s)"), strLog);
					break;
				}

				// Line Count 값이 실제 데이터 값보다 클 경우에 최대 값은 실제 값으로 계산함.
				if (nLineCnt > strTempArray.GetSize())
				{
					LOG(Info, _T("Triton_AnalMiscellaneousField - p field Line Count is adjusted LineCount (%d), DataCount(%d)"), nLineCnt, strTempArray.GetSize());
					nLineCnt = strTempArray.GetSize();
				}
				
				// 'p' print text data save
				for(i=0; i<nLineCnt; i++)
				{
					m_strarrPrintTextData.Add(strTempArray[i]);
				}

			}
			break;

		case 'q':
			break;

		case 'r':
			break;

		case 's':
			break;

		case 't':
			{
				strTemp = strMisc.Mid(1);

				if (strTemp.GetLength() != 6)
				{
					LOG(Error, _T("Triton_AnalMiscellaneousField - t field is invalid Length Error (%s)"), strMisc);
					break;
				}

				if (CUtil::IsNumeric(strTemp) == FALSE)
				{
					LOG(Error, _T("Triton_AnalMiscellaneousField - t field is invalid no numeric (%s)"), strMisc);
					break;
				}

				SYSTEMTIME	ti;
				GetLocalTime(&ti);

				ti.wHour   = CUtil::StringToInt(strTemp.Left(2));
				ti.wMinute = CUtil::StringToInt(strTemp.Mid(2,2));
				ti.wSecond = CUtil::StringToInt(strTemp.Right(2));
				SetLocalTime(&ti);

				LOG(Info, _T("Triton_AnalMiscellaneousField - t field : SYSTEM Time is Changed by Host (%s)"), strTemp);
			}
			break;

		case 'u': // Two character FIDs
			// Make the second FID
			if(strMisc.GetLength() < 2)	
			{
				LOG(Error, _T("Triton_AnalMiscellaneousField - u field is invalid (%s)"), strMisc);
				break;
			}

			chMisc = (BYTE)strMisc.GetAt(1);
			nMisc_FID2 = chMisc_Responses[nMisc_Index][i] & 0xFFFFFF00;
			if( (Triton_GetFID(chMisc) &  nMisc_FID2) != Triton_GetFID(chMisc))
			{
				LOG(Error, _T("Triton_AnalMiscellaneousField - u field is invalid (%s)"), strMisc);
				break;
			}

			// 'a' 6 Numeric ASCII MMDDYY
			if (chMisc == 'a' )
			{
				if( strMisc.GetLength() == 8)
				{
					strTemp = strMisc.Mid(2);

					if (CUtil::IsNumeric(strTemp) == FALSE)
					{
						LOG(Error, _T("Triton_AnalMiscellaneousField - ua field is invalid (%s)"), strMisc);
						break;
					}

					SYSTEMTIME	ti, orgTi;

					GetLocalTime(&ti);
					GetLocalTime(&orgTi);
					ti.wMonth   = CUtil::StringToInt(strTemp.Left(2));
					ti.wDay		= CUtil::StringToInt(strTemp.Mid(2, 2));
					ti.wYear	= 2000 + CUtil::StringToInt(strTemp.Right(2));
					SetLocalTime(&ti);

					if (orgTi.wYear != ti.wYear || orgTi.wMonth != ti.wMonth || orgTi.wDay != ti.wDay)
					{
						if (CEagleDataManager::GetInstance()->m_Config.m_Option.strScheduleDayClose == S_ENABLE)
						{
							// Day Total schedule Time refresh
							CEagleDataManager::GetInstance()->m_Config.m_Option.strDayCloseYear = _T("2000");	// 2000년으로 setting해서 바로 send (host로 인해 system 변경시의 사양)

							// Save File
							CEagleDataManager::GetInstance()->m_Config.SaveOptionData(_T("dayclose_year"), CEagleDataManager::GetInstance()->m_Config.m_Option.strDayCloseYear );

							LOG(Info, _T("Triton_AnalMiscellaneousField - Set Schedule Day Close Year 2000, because system time is changed by host"));
						}
					}
				}
				else
				{
					LOG(Error, _T("Triton_AnalMiscellaneousField - ua field is invalid Length error (%s)"), strMisc);
				}
			}
			else if ( chMisc == '!' )
			{
				CEagleDataManager::GetInstance()->m_Client_Info.m_bRecvConfigInfo = TRUE;

				strTemp = strMisc.Mid(2);

				if (CUtil::IsNumeric(strTemp) == FALSE)
				{
					LOG(Error, _T("Triton_AnalMiscellaneousField - u! field is invalid (%s)"), strMisc);
					break;
				}

				// Balance Inquiry Surcharge Amount set
				CEagleDataManager::GetInstance()->m_Config.m_Option.strBalaceSurcharge_amount = strTemp;
			}
			else if (chMisc == 'd')	// EMV DATA			{
			{
				if (CEagleDataManager::GetInstance()->m_Client_Info.m_bEMV_Transaction == TRUE)
				{
					strTemp = strMisc.Mid(2);

					if (strTemp.GetLength() > 0)
					{
						nEMVResult = Triton_Anal_EMV_ResponseData(strTemp);

						if (nEMVResult == EMV_APPROVED)
							nEMVResult = EMV_RSLT_OK;
						else
							nEMVResult = EMV_ERR_TERMINATE;
					}
					else
					{
						if (CEagleDataManager::GetInstance()->m_Client_Info.m_bExec2ndGAC == FALSE)
						{
							CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_StoreValue(EMVTag_AuthRespCod, 2, (unsigned char*)"05");

							CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_OnlineProcess(AAC);
							CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_Completion();

							Triton_Save_EMV_Data();
							CEagleDataManager::GetInstance()->m_Client_Info.m_bExec2ndGAC = TRUE;
						}
						
						nEMVResult = EMV_ERR_TERMINATE;
					}
				}
			}
			break;

		case 'v':
			break;

		case 'w':
			{
				CEagleDataManager::GetInstance()->m_Client_Info.m_bRecvConfigInfo = TRUE;

				int	nHostDispenseLimit = CUtil::StringToInt(strMisc.Mid(1));

				// 사양서는 $1 - $999    (dollar)
				if (nHostDispenseLimit < 1 || nHostDispenseLimit > 999)
				{
					LOG(Error, _T("Triton_AnalMiscellaneousField - w Dispense Limit range is invalid (%d)"), nHostDispenseLimit);
					break;
				}

				CEagleDataManager::GetInstance()->m_Config.m_Option.strMax_withdrawal_amount.Format(_T("%d"), nHostDispenseLimit);	// Cent 미포함 금액
			}
			break;

		case 'x':
			break;

		case 'y':
			break;

		case 'z':
			break;
		}
	}

	if((bReceivedFirstWorkingKey == TRUE) && (bReceivedSecondWorkingKey == TRUE))
	{
		// Pin Communication key is loading
		strTemp.Format(_T("%s%s"), m_sTriton_ConfigResp.m_strEncryptedPINKey_16, m_sTriton_ConfigResp.m_strEncryptedPINKey2_16);

		// EJL 저장을 위해 주입 전 이전 Working Key값을 취득
		CString strOldPinWorkingKey;
		strTemp3 = CDEV_Manager::GetInstance()->m_DEV_CREPP.Get_KeyCheckValue(WORKING_KEYNAME);

		if (strTemp3.IsEmpty() == FALSE)
			strOldPinWorkingKey.Format(_T("PIN KEY CHECKSUM = %s %s"), strTemp3.Left(2), strTemp3.Mid(2,2));
		else
			strOldPinWorkingKey = _T("PIN KEY CHECKSUM = NONE");

		if (strTemp.GetLength() == 32)
		{
			if (CDEV_Manager::GetInstance()->m_DEV_CREPP.Store_PIN_WorkingKey(strTemp) == TRUE)
			{
				CSVC_Manager::GetInstance()->m_strPINKey_KCV = CDEV_Manager::GetInstance()->m_DEV_CREPP.Get_KeyCheckValue(WORKING_KEYNAME);

				LOG(Info, _T("Triton_AnalMiscellaneousField - CREPP - Update PIN Working Key KCV : %s"), CSVC_Manager::GetInstance()->m_strPINKey_KCV);

				if (CSVC_Manager::GetInstance()->m_strPINKey_KCV.IsEmpty() == FALSE)
					strTemp.Format(_T("PIN KEY CHECKSUM = %s %s"), CSVC_Manager::GetInstance()->m_strPINKey_KCV.Left(2), CSVC_Manager::GetInstance()->m_strPINKey_KCV.Mid(2,2));
				else
					strTemp = _T("PIN KEY CHECKSUM = NONE");

				CDEV_Manager::GetInstance()->SaveEJL_SetChangeValue(strOldPinWorkingKey, strTemp);
			}
			else
			{
				// 동일 Key Error인 경우에는 정상 처리하도록 로직 보완
				if (CDEV_Manager::GetInstance()->m_DEV_CREPP.GetLastError() == _T("E100EF"))
				{
					CDEV_Manager::GetInstance()->SaveEJL_SetTextValue(text_field, _T("SAME PIN KEY IS LOADED"));
				}
				else
				{
					strTemp.Format(_T("PIN KEY LOADING IS FAILED (%s)"), CDEV_Manager::GetInstance()->m_DEV_CREPP.GetLastError());
					CDEV_Manager::GetInstance()->SaveEJL_SetTextValue(text_field, strTemp);
				}

				// Store 결과를 EJL에 저장 후 Error Clear하도록 함
				CDEV_Manager::GetInstance()->m_DEV_CREPP.m_strLastError.Empty();
			}
		}
		else
		{
			LOG(Error, _T("Triton_AnalMiscellaneousField - Working Key Length is invalid (%d)"), strTemp.GetLength());
		}
	}

	return nEMVResult;
}


/** **********************************************************
*	@brief		EMV 수신 Data 처리
*	@param		None
*	@return		정상 - 0, 실패 - (-1)
************************************************************/
int CEagleTritonMsg::Triton_Anal_EMV_ResponseData(CString strEMVResponseData)
{
	CString strEMVDataBlock;
	char	szResponseCd[8];		/* MAX 2  Bytes */
	char	szARPCData[40];			/* MAX 16  Bytes */
	char	szIssuScriptData1[266]; /* MAX 261 Bytes */
	char	szIssuScriptData2[266]; /* MAX 261 Bytes */
	char	szEMVPackData[1024];
	UINT    nARCLen				= 0;
	UINT	nARPCLen			= 0;
	UINT	nIssuScriptData1    = 0;
	UINT	nIssuScriptData2    = 0;
	UINT	nIsuScriptLen		= 0;
	UINT	nValueLen			= 0;
	int		nResult				= 0;
	int		nTagLen				= 0;
	int		nIndex				= 0;
	int		i					= 0;
	int		j					= 0;

	/* Local variable initialization */
	memset(szResponseCd, 0x00, sizeof(szResponseCd));
	memset(szARPCData, 0x00, sizeof(szARPCData));
	memset(szIssuScriptData1, 0x00, sizeof(szIssuScriptData1));
	memset(szIssuScriptData2, 0x00, sizeof(szIssuScriptData2));
	memset(szEMVPackData, 0x00, sizeof(szEMVPackData));

	/* search FS after 'ud' */
	nIndex = strEMVResponseData.Find(FIELD_DELIMITER);

	/* ud Field 이후FS가 존재 할 경우 */
	if( nIndex != -1)
		strEMVDataBlock = strEMVResponseData.Left(nIndex);
	else
		strEMVDataBlock = strEMVResponseData;

	LOG(Info, _T("Triton_Anal_EMV_ResponseData - [EMV DATA] : %s (%d)"), strEMVDataBlock, strEMVDataBlock.GetLength());

	CUtil::ConvertStringToHex(strEMVDataBlock, (BYTE*)szEMVPackData);

	int nLen =  strEMVDataBlock.GetLength() / 2;

	if( nLen > sizeof(szEMVPackData))
		nLen = 1024;

	BOOL	bTagValidCheck		= FALSE;
	nIndex			 = 0;

	// AUTH RESP, AUTH DATA, ISSUER SCRP1, ISSUER SCRP2
	char	szTagTable[5] = { (char)0x8A, (char)0x91, (char)0x71, (char)0x72, (char)0x89 };

	for (i=0; i<nLen; i = nIndex)
	{
		bTagValidCheck = FALSE;
		nValueLen = 0;

		for(j=0; j<sizeof(szTagTable); j++)
		{
			// Search Define Tag
			if (szEMVPackData[nIndex] == szTagTable[j])
			{
				bTagValidCheck = TRUE;
				break;
			}
		}

		if (bTagValidCheck == TRUE)
		{
			if (j == 0)		// Tag : "8A"
			{
				nARCLen = szEMVPackData[nIndex+1];

				/* AUTHORISATION RESPONSE 2Bytes*/
				if (nARCLen != 2)	// Fixed Length Error
				{
					bTagValidCheck = FALSE;
					break;
				}

				memcpy(szResponseCd, &szEMVPackData[nIndex+2], nARCLen);

				nIndex += 1 + 1 + nARCLen;	// Tag(1Byte) + Len(1Byte) + Value

				// Length정보와 실제 Value가 맞지 않을 경우 Error 처리하도록 수정
				if (nIndex > nLen)
				{
					bTagValidCheck = FALSE;
					break;
				}

				// Master Card 인증 대응 (ARC 값이 3030 ~ 3939 값 이외에는 3035로 처리 하도록 수정)
				if ((szResponseCd[0] < 0x30 || szResponseCd[0] > 0x39) || (szResponseCd[1] < 0x30 || szResponseCd[1] > 0x39))
				{
					nARCLen = 0x02;
					szResponseCd[0] = 0x30;
					szResponseCd[1] = 0x35;
				}

				LOG(Info, _T("Triton_Anal_EMV_ResponseData - [8A] szResposeCode[%s] Len[%d] "), CUtil::ConvertHexToString((BYTE*)szResponseCd, nARCLen), nARCLen);
			}
			else if (j == 1)	// "91" Tag
			{
				nARPCLen = szEMVPackData[nIndex+1];

				/* ISSUER AUTHENTICATION 8~16 Bytes : MAX 16Bytes */
				if (nARPCLen < 8 || nARPCLen > 16)
				{
					bTagValidCheck = FALSE;			// Length Error시 AAC 처리
					break;
				}

				memcpy(szARPCData, &szEMVPackData[nIndex+2], nARPCLen);

				nIndex += 1 + 1 + nARPCLen;	// Tag(1Byte) + Len(1Byte) + Value

				// Length정보와 실제 Value가 맞지 않을 경우 Error 처리
				if (nIndex > nLen)
				{
					bTagValidCheck = FALSE;
					break;
				}

				LOG(Info, _T("Triton_Anal_EMV_ResponseData - [91] ARPC[%s] Len[%d]"), CUtil::ConvertHexToString((BYTE*)szARPCData, nARPCLen), nARPCLen);
			}
			else if (j == 2)	// "71" Tag
			{
				// 0x82 or 0x83일 경우에 한해서 length field가 3byte가 됨
				if (szEMVPackData[nIndex+1] == 0x82)			// 0x82에 대해서는 coding되어져 있음
				{
					/* Double Length(0xFFFF) MSB */
					nValueLen = (BYTE)szEMVPackData[nIndex + 2];
					nValueLen <<= 8;
					/* Double Length LSB */
					nValueLen |= szEMVPackData[nIndex + 3];

					if (nValueLen < 0)
					{
						bTagValidCheck = FALSE;			// Length Error시 AAC 처리
						break;
					}

					/* ISSUER SCRIPT MAX 261 */
					if (nValueLen + nIssuScriptData1 > 261)	// Length Error
					{
						bTagValidCheck = FALSE;			// Length Error시 AAC 처리
						break;
					}

					memcpy(&szIssuScriptData1[nIssuScriptData1], &szEMVPackData[nIndex], 1 + 3 + nValueLen);
					nIssuScriptData1 += 1 + 3 + nValueLen;

					LOG(Info, _T("Triton_Anal_EMV_ResponseData - [71] Issuer script[%s] Len[%d]"), CUtil::ConvertHexToString((BYTE*)szIssuScriptData1, nIssuScriptData1), 1 + 3 + nValueLen);

					nIndex += 1 + 3 + nValueLen;	// Tag(1Byte) + Len(3Byte) + Value

					if (nIndex > nLen)
					{
						bTagValidCheck = FALSE;
						break;
					}
				}
				else if (szEMVPackData[nIndex+1] == 0x83)	// 0x83은 추후 필요시 추가 coding 필요
				{
					;
				}
				else
				{
					nValueLen = szEMVPackData[nIndex+1];

					/* ISSUER SCRIPT MAX 261 */
					if (nValueLen + nIssuScriptData1> 261)	// Length Error
					{
						bTagValidCheck = FALSE;			// Length Error시 AAC 처리
						break;
					}

					memcpy(&szIssuScriptData1[nIssuScriptData1], &szEMVPackData[nIndex], 1 + 1 + nValueLen);
					nIssuScriptData1 += 1 + 1 + nValueLen;

					LOG(Info, _T("Triton_Anal_EMV_ResponseData - [71] Issuer script[%s] nLen[%d]"), CUtil::ConvertHexToString((BYTE*)szIssuScriptData1, nIssuScriptData1), 1 + 1 + nValueLen);

					nIndex += 1 + 1 + nValueLen;	// Tag(1Byte) + Len(1Byte) + Value

					// Length정보와 실제 Value가 맞지 않을 경우 Error 처리
					if (nIndex > nLen)
					{
						bTagValidCheck = FALSE;
						break;
					}
				}
			}
			else if (j == 3)	// "72" Tag
			{
				// 0x82 or 0x83일 경우에 한해서 length field가 3byte가 됨
				if (szEMVPackData[nIndex+1] == 0x82)			// 0x82에 대해서는 coding되어져 있음
				{
					/* Double Length(0xFFFF) MSB */
					nValueLen = (BYTE)szEMVPackData[nIndex + 2];
					nValueLen <<= 8;
					/* Double Length LSB */
					nValueLen |= szEMVPackData[nIndex + 3];

					if (nValueLen < 0)
					{
						bTagValidCheck = FALSE;			// Length Error시 AAC 처리
						break;
					}

					/* ISSUER SCRIPT MAX 261 */
					if (nValueLen + nIssuScriptData2 > 261)	// Length Error
					{
						bTagValidCheck = FALSE;			// Length Error시 AAC 처리
						break;
					}

					memcpy(&szIssuScriptData2[nIssuScriptData2], &szEMVPackData[nIndex], 1 + 3 + nValueLen);
					nIssuScriptData2 += 1 + 3 + nValueLen;
					nIndex += 1 + 3 + nValueLen;	// Tag(1Byte) + Len(3Byte) + Value

					// Length정보와 실제 Value가 맞지 않을 경우 Error 처리
					if (nIndex > nLen)
					{
						bTagValidCheck = FALSE;
						break;
					}

					LOG(Info, _T("Triton_Anal_EMV_ResponseData - [72] Issuer script[%s] Len[%d]"), CUtil::ConvertHexToString((BYTE*)szIssuScriptData2, nIssuScriptData2), 1 + 3 + nValueLen);

				}
				else if (szEMVPackData[nIndex+1] == 0x83)	// 0x83은 추후 필요시 추가 coding 필요
				{
					;
				}
				else
				{
					nValueLen = szEMVPackData[nIndex+1];

					/* ISSUER SCRIPT MAX 261 */
					if (nValueLen + nIssuScriptData2> 261)	// Length Error
					{
						bTagValidCheck = FALSE;			// Length Error시 AAC 처리
						break;
					}

					memcpy(&szIssuScriptData2[nIssuScriptData2], &szEMVPackData[nIndex], 1 + 1 + nValueLen);
					nIssuScriptData2 += 1 + 1 + nValueLen;

					LOG(Info, _T("Triton_Anal_EMV_ResponseData - [72] Issuer script[%s] Len[%d]"), CUtil::ConvertHexToString((BYTE*)szIssuScriptData2, nIssuScriptData2), 1 + 1 + nValueLen);
					
					nIndex += 1 + 1 + nValueLen;	// Tag(1Byte) + Len(1Byte) + Value

					// Length정보와 실제 Value가 맞지 않을 경우 Error 처리
					if (nIndex > nLen)
					{
						bTagValidCheck = FALSE;
						break;
					}
				}
			}
			else if (j == 4)	// "89" Tag
			{
				nValueLen = szEMVPackData[nIndex+1];

				/* AUTHORISATION CODE 6Bytes*/
				if (nValueLen != 6)	// Fixed Length Error
				{
					bTagValidCheck = FALSE;			// Length Error시 AAC 처리
					break;
				}

				nIndex += 1 + 1 + 6;	// Tag(1Byte) + Len(1Byte) + Value(6)

				// Length정보와 실제 Value가 맞지 않을 경우 Error 처리
				if (nIndex > nLen)
				{
					bTagValidCheck = FALSE;
					break;
				}
			}
			else
			{
				LOG(Error, _T("Triton_Anal_EMV_ResponseData - UnDefined Tag Information (%02X)"), szEMVPackData[nIndex]);
			}
		}
		else
		{
			LOG(Error, _T("Triton_Anal_EMV_ResponseData - Incorrect Tag Length (%d)"), bTagValidCheck);
			break;
		}
	}

	nIsuScriptLen = nIssuScriptData1 + nIssuScriptData2;

	if (nIsuScriptLen > 522 || bTagValidCheck == FALSE)
	{
		// Error 처리- "ud"를 안받은 것처럼 처리함
		LOG(Error, _T("Triton_Anal_EMV_ResponseData - [EMV DATA] Parsing Error[%d] [%d]"), nIsuScriptLen, bTagValidCheck);

		if (CEagleDataManager::GetInstance()->m_Client_Info.m_bExec2ndGAC == FALSE)
		{
			// Invalid Tag인 경우 Default 처리
			CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_StoreValue(EMVTag_AuthRespCod, 2, (unsigned char*)"05");

			LOG(Error, _T("Triton_Anal_EMV_ResponseData - [EMV] Declined"));

			nResult = CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_OnlineProcess(AAC);

			if (nResult == EMV_RSLT_OK)
				CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_Completion();

			Triton_Save_EMV_Data(nIsuScriptLen);		// 추후 인증시 해당 case에 9F18을 송신하는지 확인 필요
			CEagleDataManager::GetInstance()->m_Client_Info.m_bExec2ndGAC = TRUE;
		}

		return EMV_DECLINED;
	}

	if (CEagleDataManager::GetInstance()->m_Client_Info.m_bExec2ndGAC == FALSE)
	{
		// SAVE ARC / ARPC / Issuer Script
		if (nARCLen > 0)
			CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_StoreValue(EMVTag_AuthRespCod, nARCLen, (unsigned char*)szResponseCd);
		else
			CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_StoreValue(EMVTag_AuthRespCod, 2, (unsigned char*)"05");

		if (nARPCLen > 0)
			CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_StoreValue(EMVTag_IsuAuthData, nARPCLen, (unsigned char*)szARPCData);

		if (nIssuScriptData1 > 0)
			CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_StoreValue(EMVTag_IsuScriptTemp1, nIssuScriptData1, (unsigned char*)szIssuScriptData1);

		if (nIssuScriptData2 > 0)
			CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_TLV_StoreValue(EMVTag_IsuScriptTemp2, nIssuScriptData2, (unsigned char*)szIssuScriptData2);

		BOOL bApproved = FALSE;
		BOOL bBI_85Response = FALSE;

		// MASTER CARD EMV SPEC for "85" response (8A Tag) for Balance Inquiry 
		//	1) ATM should send AAC to the card reader
		//	2) ATM should handle the ramain transaction normally (Print balance) (CUSTOMER SIDE POINT OF VIEW).
		if (m_sTriton_TranResp.m_strResponseCode_3 == _T("000"))
			bApproved = TRUE;
		else if ((CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransType == S_BALANCEINQUIRY) && (m_sTriton_TranResp.m_strResponseCode_3 == _T("085")))
			bApproved = TRUE;

		if (bApproved == TRUE)
		{
			if ((CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransType == S_BALANCEINQUIRY) && (szResponseCd[0] == '8') && (szResponseCd[1] == '5'))
			{
				bBI_85Response = TRUE;

				nResult = CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_OnlineProcess(AAC);

				// 조회시 085 response 수신시 정상 처리
				if (m_sTriton_TranResp.m_strResponseCode_3 == _T("085"))
					m_sTriton_TranResp.m_strResponseCode_3 == _T("000");
			}
			else if ((CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransType == S_BALANCEINQUIRY) && (szResponseCd[0] == '0') && (szResponseCd[1] == '0'))
			{
				bBI_85Response = TRUE;

				nResult = CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_OnlineProcess(AAC);

				// 조회시 085 response 수신시 정상 처리
				if (m_sTriton_TranResp.m_strResponseCode_3 == _T("085"))
					m_sTriton_TranResp.m_strResponseCode_3 == _T("000");
			}
			else
			{
				nResult = CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_OnlineProcess(TC);
			}
		}
		else
		{
			nResult = CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_OnlineProcess(AAC);
		}

		LOG(Info, _T("Triton_Anal_EMV_ResponseData - [EMV] Execute 2nd generate AC - Result (%d)"), nResult);

		if (nResult == EMV_RSLT_OK)
		{
			nResult = CDEV_Manager::GetInstance()->m_DEV_CDR.EMV_Completion();

			LOG(Info, _T("Triton_Anal_EMV_ResponseData - [EMV] Final Result (%d)"), nResult);

			if (bBI_85Response == TRUE)
				nResult = EMV_APPROVED;

			LOG(Info, _T("Triton_Anal_EMV_ResponseData - [EMV] Final Result2 (%d)"), nResult);
		}
		else
		{
			LOG(Info, _T("Triton_Anal_EMV_ResponseData - [EMV] 2nd generate AC Result (%d)"), nResult);
		}

		Triton_Save_EMV_Data(nIsuScriptLen);
		CEagleDataManager::GetInstance()->m_Client_Info.m_bExec2ndGAC = TRUE;
	}

	return nResult;
}



/** **********************************************************
*	@brief		Triton Field ID 판단
*	@param		None
*	@return		Field ID
************************************************************/
UINT CEagleTritonMsg::Triton_GetFID(char fid2)
{
	switch(fid2)
	{
	case 'a':		return FID2_a;
	case 'b':		return FID2_b;
	case 'c':		return FID2_c;
	case 'd':		return FID2_d;
	case 'e':		return FID2_e;
	case 'f':		return FID2_f;
	case 'g':		return FID2_g;
	case 'h':		return FID2_h;
	case 'i':		return FID2_i;
	case 'j':		return FID2_j;
	case 'k':		return FID2_k;
	case 'l':		return FID2_l;
	case 'm':		return FID2_m;
	case 'n':		return FID2_n;

	case '!':		return FID2_21h;
	case 'p':		return FID2_p;
	case 'q':		return FID2_q;
	case 'r':		return FID2_r;
	case 's':		return FID2_s;
	case 't':		return FID2_t;
	case 'u':		return FID2_u;
	case 'v':		return FID2_v;
	case 'w':		return FID2_w;
	case 'x':		return FID2_x;
	default:		return 0xffffffff;
	}
}

/** **********************************************************
*	@brief		Reversal 상태 판단
*	@param		None
*	@return		REVERSAL - TRUE, else FALSE
************************************************************/
BOOL CEagleTritonMsg::Triton_IsReversalCondition()
{
	if (CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransType == S_WITHDRAWAL)
		return TRUE;

	return FALSE;
}
