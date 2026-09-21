#include "stdafx.h"
#include "DEV_HOST.h"
#include "DEV_Manager.h"

#include "../../EagleCE_ATM/EagleCE_ATM/SVC_Define.h"
#include "../../EagleCE_Screen/EagleCE_Screen/SCR_Manager.h"
#include "../../EagleCE_Framework/EagleCE_Framework/IniFile.h"

/** ********************************************************************
* @brief MultiByte => WideChar
* @param char *pMultiChar	 MultiChar
* @param LPWSTR pWideChar	 WideChar
* @param int nLenWideChar	 WideChar 길이
* @retval 없음
************************************************************************/
void CDEV_HOST::MultiToWide(char *pMultiChar, LPWSTR pWideChar, int nLenWideChar, int nMaxLength/*=MAX_PATH*/)
{
	memset(pWideChar, 0, nMaxLength);
	MultiByteToWideChar(CP_ACP, 0, pMultiChar, -1, pWideChar, nLenWideChar);
}


/** ********************************************************************
* @brief MultiByte => WideChar
* @param char *pMultiChar	 MultiChar
* @param CString &strOutput	 최종 문자열
* @param int nLenWideChar	 WideChar 길이
* @param BOOL bNumeric		 응답전문 data가 숫자타입인지 문자열 타입인지
* @retval 없음
************************************************************************/
void CDEV_HOST::MultiToWideEx(char *pMultiChar, CString &strOutput, int nLenWideChar, BOOL bNumeric/*=TRUE*/)
{
	TCHAR wszTempBuff[MAX_PATH] = {0,};
	CString strData = _T("");

	memset(wszTempBuff, 0, MAX_PATH);
	MultiByteToWideChar(CP_ACP, 0, pMultiChar, -1, (LPWSTR)wszTempBuff, nLenWideChar);
	
	strData.Format(_T("%s"), wszTempBuff);

	if(bNumeric)	// ex) 권종: "020"
	{
		strOutput = CUtil::IntToString(CUtil::StringToInt(strData));
	}
	else			// 문자열은 스페이스(0x20)로 채워지기 때문에 스페이스 제거함.
	{
		strData.TrimLeft();
		strData.TrimRight();

		strOutput = strData;
	}
}


/** ********************************************************************
* @brief WideChar => MultiByte
* @param LPCTSTR pWideChar	 WideChar
* @param char *pMultiChar	 MultiChar
* @param int nLenMultiChar	 MultiChar 길이
* @retval 없음
************************************************************************/
void CDEV_HOST::WideToMulti(LPCTSTR pWideChar, char *pMultiChar, int nLenMultiChar)
{
	memset(pMultiChar, 0, nLenMultiChar);
	WideCharToMultiByte(CP_ACP, 0, pWideChar, -1, pMultiChar, nLenMultiChar, NULL, NULL);
}



/** ********************************************************************
* @brief WideChar => MultiByte
* @param LPCTSTR pWideChar	 WideChar
* @param char *pMultiChar	 MultiChar
* @param int nLenMultiChar	 MultiChar 길이
* @retval 없음
************************************************************************/
void CDEV_HOST::WideToMultiEx(LPCTSTR pWideChar, char *pMultiChar, int nLenMultiChar, BOOL bNumeric/*=TRUE*/, CString strInsert/*=_T(" ")*/, BOOL bLeftInsert/*=FALSE*/)
{
	int nData = 0;
	CString strData = _T("");

	memset(pMultiChar, 0, nLenMultiChar);

	if(TRUE == bNumeric)
	{
		nData = CUtil::StringToInt((CString)pWideChar);
		strData = CUtil::IntToStringEx(nData, nLenMultiChar);
	}
	else
	{
		strData = ConvertString((CString)pWideChar, nLenMultiChar, strInsert, bLeftInsert);
	}

	WideCharToMultiByte(CP_ACP, 0, (LPCTSTR)strData, -1, pMultiChar, nLenMultiChar, NULL, NULL);
}


/** ********************************************************************
* @brief 특정 문자를 insert 함.
* @param CString strData	orginal 문자열
* @param int nLen			전체 문자열 길이
* @param CString strInsert	insert할 문자
* @param BOOL bLeft			insert 위치
* @retval CString			변환된 문자열
************************************************************************/
CString CDEV_HOST::ConvertString(CString strData, int nLen, CString strInsert/*=_T(" ")*/, BOOL bLeft/*=FALSE*/)
{
	int i=0;
	int nDataLen = strData.GetLength();
	CString strResult = _T("");

	if(TRUE == bLeft)
	{
		for(i=0; i<(nLen - nDataLen); i++)
			strData.Insert(0, strInsert);
	}
	else
	{
		for(i=0; i<(nLen - nDataLen); i++)
			strData.Append(strInsert);
	}

	strResult = strData;

	return strResult;
}


/** **********************************************************
*	@brief	Backup Log Files
*	@param	CString strDestDirectory	: Backup할 위치
*	@retval	없음
************************************************************/
BOOL CDEV_HOST::AMS_Backup_LogFile(CString strDestDirectory)
{
	BOOL	bResult = FALSE;
	CString strZipFileName;
	int		nIndex = 0;

	SYSTEMTIME		st;
	GetLocalTime(&st);

	// make file name for search.
	bResult = CUtil::IsExistFile(PATH_LOG_ABS);

	if (bResult == TRUE)
	{
		// 현재 Trace 폴더에 있는 File을 open시 에러가 발생하므로 memory상에 Temp folder에 copy후 진행하도록 변경
		CUtil::CopyFileInDirectory(PATH_LOG_ABS, _T("*.*"), PATH_TEMP_ABS);

		strZipFileName.Format(_T("%s\\000_LOG_%04d%02d%02d_%02d%02d.zip"), strDestDirectory, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
		bResult = CUtil::ZippingFromFolder(PATH_TEMP_ABS, strZipFileName);

		CUtil::DeleteFileInDirectory(PATH_TEMP_ABS, _T("*.log"));

		LOG(Info, _T("AMS Backup Log File success (%d)"), bResult);
	}
	else
	{
		LOG(Info, _T("Trace directory is not exist files"));
	}

	return bResult;
}


/** **********************************************************
*	@brief	정상 상태의 Device의 ErrorCode clear
*	@param	CString strDeviceKind	: Device 종류
*	@retval	없음
************************************************************/
void CDEV_HOST::Clear_DeviceError(CString strDeviceKind)
{
	CString strErrorType = CEagleDataManager::GetInstance()->m_Client_Info.m_strErrorCode.Left(1);

	LOG(Info, _T("ErrorCode Device Kind: [%s]"), strDeviceKind);

	// 장애코드와 동일한 Device일 경우에만 All Clear
	if(strErrorType.CompareNoCase(strDeviceKind) == 0)
	{
		Clear_Error();
	}
	else	// 장애코드와 동일하지 않은 Device인 경우에는 Device 종류만 삭제(ErrorCode 유지)
	{
		if(DEVICE_CDR == strDeviceKind)
		{
			CEagleDataManager::GetInstance()->m_Client_Info.m_nErrorDevice &= ~(ERROR_DEVICE_IDC);
		}
		else if(DEVICE_CDM == strDeviceKind)
		{
			CEagleDataManager::GetInstance()->m_Client_Info.m_nErrorDevice &= ~(ERROR_DEVICE_CDM);
		}
		else if(DEVICE_EPP == strDeviceKind)
		{
			CEagleDataManager::GetInstance()->m_Client_Info.m_nErrorDevice &= ~(ERROR_DEVICE_EPP);
		}
		else if(DEVICE_EJL == strDeviceKind)
		{

		}
		else if(DEVICE_PTR == strDeviceKind)
		{
			CEagleDataManager::GetInstance()->m_Client_Info.m_nErrorDevice &= ~(ERROR_DEVICE_PTR);
		}
		else if(DEVICE_DIO == strDeviceKind)
		{
			CEagleDataManager::GetInstance()->m_Client_Info.m_nErrorDevice &= ~(ERROR_DEVICE_SIU);
		}
	}

	LOG(Info, _T("After Clear => ErrorCode:[%s], ErrorDevice:[0x%08x]"), CEagleDataManager::GetInstance()->m_Client_Info.m_strErrorCode, CEagleDataManager::GetInstance()->m_Client_Info.m_nErrorDevice);
}


/** **********************************************************
*	@brief Error Code 초기화
*	@retval	없음
************************************************************/
void CDEV_HOST::Clear_Error()
{
	CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetError = FALSE;
	CEagleDataManager::GetInstance()->m_Client_Info.m_strErrorCode.Empty();
	CEagleDataManager::GetInstance()->m_Client_Info.m_nErrorDevice = 0;
	CEagleDataManager::GetInstance()->m_Client_Info.m_strErrorMsg.Empty();
}


/** **********************************************************
*	@brief	OP에서 Device 장애 발생시 ErrorCode 설정
*	@param	CString strDeviceKind	: Device 종류
*	@param	CString strErrorCode	: Error Code
*	@param	CString strErrorMsg		: Error Message
*	@retval	없음
************************************************************/
void CDEV_HOST::Set_DeviceErrorCode(CString strDeviceKind, CString strErrorCode, CString strErrorMsg /*= _T("")*/)
{
	CString strErrorType = CEagleDataManager::GetInstance()->m_Client_Info.m_strErrorCode.Left(1);

//	if(strErrorType.CompareNoCase(strDeviceKind) == 0)
	if( (strErrorType.CompareNoCase(strDeviceKind) == 0) || (FALSE == CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetError) )
	{
		LOG(Error, _T("Set_DeviceErrorCode(%s)"), strErrorCode);

		CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetError = TRUE;
		CEagleDataManager::GetInstance()->m_Client_Info.m_strErrorCode = strErrorCode;
		CEagleDataManager::GetInstance()->m_Client_Info.m_strErrorMsg = strErrorMsg;
	}

	if(DEVICE_CDR == strDeviceKind)
	{
		CEagleDataManager::GetInstance()->m_Client_Info.m_nErrorDevice |= ERROR_DEVICE_IDC;
	}
	else if(DEVICE_CDM == strDeviceKind)
	{
		CEagleDataManager::GetInstance()->m_Client_Info.m_nErrorDevice |= ERROR_DEVICE_CDM;
	}
	else if(DEVICE_EPP == strDeviceKind)
	{
		CEagleDataManager::GetInstance()->m_Client_Info.m_nErrorDevice |= ERROR_DEVICE_EPP;
	}
	else if(DEVICE_EJL == strDeviceKind)
	{

	}
	else if(DEVICE_PTR == strDeviceKind)
	{
		CEagleDataManager::GetInstance()->m_Client_Info.m_nErrorDevice |= ERROR_DEVICE_PTR;
	}
	else if(DEVICE_DIO == strDeviceKind)
	{
		CEagleDataManager::GetInstance()->m_Client_Info.m_nErrorDevice |= ERROR_DEVICE_SIU;
	}

	LOG(Info, _T("Set_DeviceErrorCode => ErrorCode:[%s], ErrorDevice:[0x%08x]"), CEagleDataManager::GetInstance()->m_Client_Info.m_strErrorCode, CEagleDataManager::GetInstance()->m_Client_Info.m_nErrorDevice);
}


/** ********************************************************************
* @brief AMS으로부터 명령 받을 준비
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
BOOL CDEV_HOST::AMS_Proc_Comm(int nOpenType, CString strSendCommand/*=AMS_REQ_STATUS*/)
{
	if(AMS_LISTEN == nOpenType)
	{
		return AMS_Proc_Requested();
	}
	else// if(AMS_CONNECT == nOpenType)
	{
		return AMS_Proc_Send(strSendCommand);
	}
}


/** ********************************************************************
* @brief AMS로부터 전문을 받기 위해 대기함. 
* @retval TRUE	성공 
* @retval FALSE	실패 
************************************************************************/
BOOL CDEV_HOST::AMS_Proc_Requested()
{
	//BYTE *pbyRecvData = NULL;
	int nRecvLength = 0;
	BOOL bResult = FALSE;
	CString strCommand, strSubCommand;
	BOOL bExitCommand = FALSE;
	BOOL bAdditionalCommandResult = FALSE;

	//pbyRecvData = new BYTE[NET_RECV_BUFF_SIZE];
	memset(m_pAMSRecvBuffer, 0, NET_RECV_BUFF_SIZE);

	m_bChangedIP = FALSE;

	// 1. Accept. ConnectOpen is client mode.
	if(AMS_OK != AMS_Async_ConnectAccept())
	{
		LOG(Error, _T("AMS_Proc_Requested() AMS Accep is failed."));
		goto ErrorReturn;
	}

	m_nAMSProcessStatus = AMS_STATUS_AUTH;

	while(!bExitCommand)
	{
		memset(m_pAMSRecvBuffer, 0, NET_RECV_BUFF_SIZE);
		nRecvLength = 0;

		switch(m_nAMSProcessStatus)
		{
		case AMS_STATUS_AUTH:
			{
				LOG(Info, _T("AMS_Proc_Requested() AMS_STATUS_AUTH"));

				bResult = AMS_ProcReceiveData(strCommand, strSubCommand, m_pAMSRecvBuffer, nRecvLength, TRUE);
				if(FALSE == bResult)
				{
					LOG(Error, _T("AMS_Proc_Requested() AMS_STATUS_AUTH AMS_ProcReceiveData is failed."));
					goto ErrorReturn;
				}
				
				m_nAMSProcessStatus = AMS_STATUS_MAIN;
			}
			break;
			
		case AMS_STATUS_MAIN:
			{
				LOG(Info, _T("AMS_Proc_Requested() AMS_STATUS_MAIN"));

				bResult = AMS_ProcReceiveData(strCommand, strSubCommand, m_pAMSRecvBuffer, nRecvLength);
				if(FALSE == bResult)
				{
					if(TRUE == bAdditionalCommandResult)
					{
						LOG(Info, _T("AMS_Proc_Requested() Additional Command is not exist."));

						bExitCommand = TRUE;
						break;
					}
					else
					{
						LOG(Error, _T("AMS_Proc_Requested() AMS_STATUS_MAIN AMS_ProcReceiveData is failed."));
						goto ErrorReturn;
					}
				}

				if(AMS_REQ_SETUP_WRITE == strCommand)
				{
					if(TRUE == m_bChangedIP)	// IP변경시에는 EOT를 전송함으로써 통신 종료
					{
						// EOT 전송
						// Additional Command를 전송하지 못하는 Command에서는 EOT를 전송해서 통신종료를 Server에 알려줌.
						// S/W Update, System Reboot, Setup Write(TerminalIP/DHCP 변경시) 일 경우
						BYTE byDataEOT = DATA_EOT;
						int nResult = HOST_Sync_Send(&byDataEOT, 1, _T(""), _T(""), _T(""), TRUE);

						if(nResult != AMS_OK)
						{
							LOG(Error, _T("Setup Write Send EOT Failed."));
						}

						CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_1000MS);
						bExitCommand = TRUE;
					}

					// EMV AID List 갱신
					CDEV_Manager::GetInstance()->m_DEV_CDR.Save_AIDs_InformToFile();
				}

				m_nAMSProcessStatus = AMS_STATUS_ADDITIONAL;
			}
			break;

		case AMS_STATUS_ADDITIONAL:
			{
				LOG(Info, _T("AMS_Proc_Requested() AMS_STATUS_ADDITIONAL"));

				bAdditionalCommandResult = FALSE;

				bResult = AMS_SendATMInquiryAddCmd(AMS_REQ_STATUS, AMS_REQ_SUB_INQUIRY_ADD_CMD);

				if(TRUE == bResult)	// ACK를 수신한 경우
				{
					m_nAMSProcessStatus = AMS_STATUS_MAIN;
					bAdditionalCommandResult = TRUE;
				}
				else
				{
					LOG(Error, _T("AMS_Proc_Requested() Inquiry Additional command is failed."));
					goto ErrorReturn;
				}
			}
			break;

		default:
			{
				LOG(Error, _T("AMS_Proc_Requested() AMS Status is unknown. : [%d]"), m_nAMSProcessStatus);
				goto ErrorReturn;
			}
			break;
		}

		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	AMS_Sync_ConnectClose();

	if(AMS_REQ_SETUP_WRITE == strCommand)
	{
		//CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_1000MS);

		// Terminal IP가 변경된 경우 Reload Lan Card를 수행하도록 로직 보완
		CDEV_Manager::GetInstance()->Set_Network_Config(m_bChangedIP);

		if(TRUE == m_bChangedIP)	// IP변경시에는 AMS와 통신이 불가능하므로 종료시킴.
		{
			m_bChangedIP = FALSE;

			// IP가 DHCP인 경우는 Network 설정 후에 IP 정보를 취득해서 갱신하도록 함
			if (CEagleDataManager::GetInstance()->m_Config.m_Host.strUseDHCP == S_ENABLE)
			{
				CString strTemp;
				strTemp.Empty();
				CUtil::GetDHCP_IP(strTemp);
				CEagleDataManager::GetInstance()->m_Config.m_Host.strTerminalip = strTemp;

				strTemp.Empty();
				CUtil::GetDHCP_SubnetMask(strTemp);
				CEagleDataManager::GetInstance()->m_Config.m_Host.strSubnet = strTemp;

				strTemp.Empty();
				CUtil::GetDHCP_GateWay(strTemp);
				CEagleDataManager::GetInstance()->m_Config.m_Host.strGateway = strTemp;

				strTemp.Empty();
				CUtil::GetDHCP_DNS(strTemp);
				CEagleDataManager::GetInstance()->m_Config.m_Host.strDNS = strTemp;

				CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("terminal_ip"), CEagleDataManager::GetInstance()->m_Config.m_Host.strTerminalip);
				CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("subnet"), CEagleDataManager::GetInstance()->m_Config.m_Host.strSubnet);
				CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("gateway"), CEagleDataManager::GetInstance()->m_Config.m_Host.strGateway);
				CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("DNS"), CEagleDataManager::GetInstance()->m_Config.m_Host.strDNS);
			}

			// TerminalIP 변경시, 서버에서도 IP 정보를 알고 있어야 이후에 통신이 가능하기 때문에
			// Status 정보를 전송함.
			if (CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_Status_en_flg == S_ENABLE)
			{
				AMS_Proc_Send(AMS_REQ_STATUS);
			}
		}
	}

	LOG(Info, _T("AMS_Proc_Requested() SUCCESS!!!"));

	return TRUE;

ErrorReturn:
	AMS_Sync_ConnectClose();

	LOG(Error, _T("AMS_Proc_Requested() ERROR!!!"));

	m_bChangedIP = FALSE;

	return FALSE;
}


/** ********************************************************************
* @brief 응답 전문에 따른 처리
* @param	BOOL bVerify		: data 검증 처리 여부
* @param	CString &strCommand	: Main Command
* @param	CString &strSubCommand	: Sub Command
* @param	BYTE *pbyRecvData	: RecvData
* @param	int &nRecvLength	: RecvLength
* @retval	TRUE	성공 
* @retval	FALSE	실패 
************************************************************************/
BOOL CDEV_HOST::AMS_ProcReceiveData(CString &strCommand, CString &strSubCommand, BYTE *pbyRecvData, int &nRecvLength, BOOL bVerify/*=FALSE*/)
{
	CString strRecvData;
	CStringArray strArrRecvData;

	// Recv CMD
	// [PCI-SSF Fix] pbyRecvData는 항상 m_pAMSRecvBuffer(NET_RECV_BUFF_SIZE 크기)로 호출되므로 해당 상수로 상한 지정
	nRecvLength = HOST_Sync_Recv(pbyRecvData, NET_RECV_BUFF_SIZE, TRUE);

	if (nRecvLength <= (LEN_LENGTH + 1))	// +1 => 0x1C
	{
		LOG(Error, _T("AMS_ProcReceiveData() RecvSize is invalid. nRecvLength : [%d]"), nRecvLength);
		return FALSE;
	}

	if(0 == strlen((char*)pbyRecvData))
	{
		LOG(Error, _T("AMS_ProcReceiveData() RecvData is NULL"));
		return FALSE;
	}

	strArrRecvData.RemoveAll();
	strRecvData = pbyRecvData;

	CUtil::ParsingStringToStringArray(strRecvData, (CString)FS, strArrRecvData);

	if(strArrRecvData.GetCount() > 1)
		strCommand = strArrRecvData[1];

	if(strArrRecvData.GetCount() > 2)
		strSubCommand = strArrRecvData[2];

#if (SEND_RECV_DEBUG_AMS)
	//		if(_T("H") != strCommand)	// SW Upload Command
	LOG(Info, _T("RecvLength:[%d], byRecvData:[%s]"), nRecvLength, strRecvData);
#endif

	if(TRUE == bVerify)
	{
		if (strCommand != AMS_REQ_STATUS)
		{
			// Log In이 아닌 경우에는 무조건 Error Return
			return FALSE;
		}

		// Verify
		if(FALSE == AMS_VerifyData(pbyRecvData, nRecvLength))
		{
			LOG(Error, _T("AMS_ProcReceiveData() AMS_VerifyData is failed."));
			return FALSE;
		}
	}

	// Processing AMS Command
	return AMS_ParseCMD(pbyRecvData, nRecvLength);
}


/** ********************************************************************
* @brief AMS로 상태 및 저널 정보를 전달하기 위함.
* @retval TRUE	성공 
* @retval FALSE	실패 
************************************************************************/
BOOL CDEV_HOST::AMS_Proc_Send(CString strSendCommand)
{
	BOOL bResult = FALSE;
	CString strRMSIPAddress, strRMSPort, strPhoneNumber;
	CString strDestInfo;

	//BYTE *pbyRecvData = NULL;
	int nRecvLength = 0;
	CString strCommand, strSubCommand;
	BOOL bExitCommand = FALSE;
	BOOL bAdditionalCommandResult = FALSE;

	//pbyRecvData = new BYTE[NET_RECV_BUFF_SIZE];
	memset(m_pAMSRecvBuffer, 0, NET_RECV_BUFF_SIZE);

	strRMSIPAddress = CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_IP_Address;
	strRMSIPAddress.TrimLeft();
	strRMSIPAddress.TrimRight();
	strRMSPort.Format(_T("%d"), CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_Port));
	strPhoneNumber = CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_primary_num;

	if (CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType == _T("TCP/IP"))
		strDestInfo = strRMSIPAddress;
	else
		strDestInfo = strPhoneNumber;

	strDestInfo.TrimLeft();
	strDestInfo.TrimRight();

	// 1. Connect. when TCP/IP Mode, ConnectOpen is client mode.
	// [2019.02.22] SSL 적용
//	if(AMS_OK != AMS_Sync_ConnectOpen(OPEN_CONNECT, strDestInfo, strRMSPort, _T("0")))
	if(AMS_OK != AMS_Sync_ConnectOpen(OPEN_CONNECT, strDestInfo, strRMSPort))
	{
		LOG(Error, _T("AMS_Proc_Send() AMS Open is failed."));
		goto ErrorReturn;
	}

	m_nAMSProcessStatus = AMS_STATUS_SEND;

	while(!bExitCommand)
	{
		memset(m_pAMSRecvBuffer, 0, NET_RECV_BUFF_SIZE);
		nRecvLength = 0;

		switch(m_nAMSProcessStatus)
		{
		case AMS_STATUS_SEND:
			{
				LOG(Info, _T("AMS_Proc_Send() AMS_STATUS_SEND"));

				if(AMS_REQ_STATUS == strSendCommand)
					bResult = AMS_StatusSend();
				else if(AMS_REQ_PRINT_JNL == strSendCommand)
					bResult = AMS_JournalSend();
				else
				{
					LOG(Error, _T("AMS_Proc_Send() SendCommand(%s) is undefined."), strSendCommand);
					bResult = FALSE;
				}

				if(FALSE == bResult)
				{
					LOG(Error, _T("AMS_Proc_Send() AMS_STATUS_SEND AMS_Send is failed."));
					goto ErrorReturn;
				}

				m_nAMSProcessStatus = AMS_STATUS_ADDITIONAL;
			}
			break;

		case AMS_STATUS_ADDITIONAL:
			{
				LOG(Info, _T("AMS_Proc_Send() AMS_STATUS_ADDITIONAL"));

				bAdditionalCommandResult = FALSE;

				bResult = AMS_SendATMInquiryAddCmd(AMS_REQ_STATUS, AMS_REQ_SUB_INQUIRY_ADD_CMD);

				if(TRUE == bResult)	// ACK를 수신한 경우
				{
					m_nAMSProcessStatus = AMS_STATUS_MAIN;
					bAdditionalCommandResult = TRUE;
				}
				else
				{
					LOG(Error, _T("AMS_Proc_Send() Inquiry Additional command is failed."));
					goto ErrorReturn;
				}
			}
			break;

		case AMS_STATUS_MAIN:
			{
				LOG(Info, _T("AMS_Proc_Send() AMS_STATUS_MAIN"));

				bResult = AMS_ProcReceiveData(strCommand, strSubCommand, m_pAMSRecvBuffer, nRecvLength);
				if(FALSE == bResult)
				{
					if(TRUE == bAdditionalCommandResult)
					{
						LOG(Info, _T("AMS_Proc_Send() Additional Command is not exist."));

						bExitCommand = TRUE;
						break;
					}
					else
					{
						LOG(Error, _T("AMS_Proc_Send() AMS_STATUS_MAIN AMS_ProcReceiveData is failed."));
						goto ErrorReturn;
					}
				}

				if(AMS_REQ_SETUP_WRITE == strCommand)
				{
					if(TRUE == m_bChangedIP)	// IP변경시에는 EOT를 전송함으로써 통신 종료
					{
						// EOT 전송
						// Additional Command를 전송하지 못하는 Command에서는 EOT를 전송해서 통신종료를 Server에 알려줌.
						// S/W Update, System Reboot, Setup Write(TerminalIP/DHCP 변경시) 일 경우
						BYTE byDataEOT = DATA_EOT;
						int nResult = HOST_Sync_Send(&byDataEOT, 1, _T(""), _T(""), _T(""), TRUE);

						if(nResult != AMS_OK)
						{
							LOG(Error, _T("Setup Write Send EOT Failed."));
						}

						CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_1000MS);
						bExitCommand = TRUE;
					}

					// EMV AID List 갱신
					CDEV_Manager::GetInstance()->m_DEV_CDR.Save_AIDs_InformToFile();
				}

				m_nAMSProcessStatus = AMS_STATUS_ADDITIONAL;
			}
			break;

		default:
			{
				LOG(Error, _T("AMS_Proc_Send() AMS Status is unknown. : [%d]"), m_nAMSProcessStatus);
				goto ErrorReturn;
			}
			break;
		}

		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	AMS_Sync_ConnectClose();

	if(TRUE == m_bChangedIP)	// IP변경시에는 AMS와 통신이 불가능하므로 종료시킴.
	{
		//CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_1000MS);

		// Terminal IP가 변경된 경우 Reload Lan Card를 수행하도록 로직 보완
		CDEV_Manager::GetInstance()->Set_Network_Config();

		m_bChangedIP = FALSE;

		// IP가 DHCP인 경우는 Network 설정 후에 IP 정보를 취득해서 갱신하도록 함
		if (CEagleDataManager::GetInstance()->m_Config.m_Host.strUseDHCP == S_ENABLE)
		{
			CString strTemp;
			strTemp.Empty();
			CUtil::GetDHCP_IP(strTemp);
			CEagleDataManager::GetInstance()->m_Config.m_Host.strTerminalip = strTemp;

			strTemp.Empty();
			CUtil::GetDHCP_SubnetMask(strTemp);
			CEagleDataManager::GetInstance()->m_Config.m_Host.strSubnet = strTemp;

			strTemp.Empty();
			CUtil::GetDHCP_GateWay(strTemp);
			CEagleDataManager::GetInstance()->m_Config.m_Host.strGateway = strTemp;

			strTemp.Empty();
			CUtil::GetDHCP_DNS(strTemp);
			CEagleDataManager::GetInstance()->m_Config.m_Host.strDNS = strTemp;

			CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("terminal_ip"), CEagleDataManager::GetInstance()->m_Config.m_Host.strTerminalip);
			CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("subnet"), CEagleDataManager::GetInstance()->m_Config.m_Host.strSubnet);
			CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("gateway"), CEagleDataManager::GetInstance()->m_Config.m_Host.strGateway);
			CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("DNS"), CEagleDataManager::GetInstance()->m_Config.m_Host.strDNS);
		}

		// TerminalIP 변경시, 서버에서도 IP 정보를 알고 있어야 이후에 통신이 가능하기 때문에
		// Status 정보를 전송함.
		if (CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_Status_en_flg == S_ENABLE)
		{
			AMS_Proc_Send(AMS_REQ_STATUS);
		}
	}

	LOG(Info, _T("AMS_Proc_Send() SUCCESS!!!"));

	return TRUE;

ErrorReturn:
	AMS_Sync_ConnectClose();

	m_bChangedIP = FALSE;

	LOG(Error, _T("AMS_Proc_Send() ERROR!!!"));

	return FALSE;
}


/** ********************************************************************
* @brief AMS로 상태 정보를 전달하기 위함.
* @retval TRUE	성공 
* @retval FALSE	실패 
************************************************************************/
BOOL CDEV_HOST::AMS_StatusSend()
{
	BOOL bResult = FALSE;

	bResult = AMS_SendATMStatus(AMS_REQ_STATUS, AMS_REQ_SUB_STATUS);
	if(FALSE == bResult)
	{
		LOG(Error, _T("AMS_StatusSend() AMS_SendATMStatus is failed."));
	}

	return bResult;
}


/** ********************************************************************
* @brief AMS로 Journal 정보를 전달하기 위함.
* @retval TRUE	성공 
* @retval FALSE	실패 
************************************************************************/
BOOL CDEV_HOST::AMS_JournalSend()
{
	BOOL bResult = FALSE;

	// 2. Send Journal
	bResult = AMS_ProcJournal(AMS_REQ_PRINT_JNL, _T(" "), _T("0"));
	if(FALSE == bResult)
	{
		LOG(Error, _T("AMS_JournalSend() AMS_ProcJournal is failed."));
	}

	return bResult;
}


/** ********************************************************************
* @brief AMS로부터 받은 전문 검증
* @param BYTE *pbyData	 AMS로부터 받은 전문
* @param int nRecvLen	 전문 길이
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
BOOL CDEV_HOST::AMS_VerifyData(BYTE *pbyData, int nRecvLen)
{
	CString	strData;
	CString strTerminalID;
	CString strPassword;

	CString strRecvData;
	CStringArray strArrRecvData;

	strArrRecvData.RemoveAll();
	strRecvData = pbyData;
	CUtil::ParsingStringToStringArray(strRecvData, (CString)FS, strArrRecvData);

	// TerminalID
	if(strArrRecvData.GetCount() > 3)
		strTerminalID = strArrRecvData[3];

	// Password
	if(strArrRecvData.GetCount() > 4)
		strPassword = strArrRecvData[4];
	
	// check Terminal ID
	strData = CEagleDataManager::GetInstance()->m_Config.m_Option.strTerminal_ID;
	if (strData != strTerminalID)
	{
		LOG(Error, _T("AMS_VerifyData() Verify Data : Terminal ID Worng [%s], AMS[%s]"), strData, strTerminalID);
		return FALSE;
	}

	// check Password (T-DES로 내려오는 값으로 비교)
	strData = CEagleDataManager::GetInstance()->m_Config.LoadSystemData(_T("PASSWORD"), _T("ams_pwd"));

	if (strData != strPassword)
	{
		LOG(Error, _T("AMS_VerifyData() Verify Data : Password Worng [%s], AMS[%s]"), strData, strPassword);
		return FALSE;
	}

	LOG(Info, _T("AMS_VerifyData() Verify Data OK.."));

	return TRUE;
}

/** ********************************************************************
* @brief AMS로부터 받은 Command별 처리
* @param BYTE *pBuffer	 AMS로부터 받은 전문
* @param int nRecvLen	 AMS로부터 받은 전문 길이
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
BOOL CDEV_HOST::AMS_ParseCMD(BYTE *pBuffer, int nRecvLen)
{
	BOOL bResult = FALSE;

	CString strCommand, strSubCommand;
	CString strRecvData;
	CString strTemp;

	CStringArray strArrRecvData;
	
	strArrRecvData.RemoveAll();
	strRecvData = pBuffer;

	CUtil::ParsingStringToStringArray(strRecvData, (CString)FS, strArrRecvData);

	if(strArrRecvData.GetCount() > 1)
		strCommand = strArrRecvData.GetAt(1);

	if(strArrRecvData.GetCount() > 2)
		strSubCommand = strArrRecvData.GetAt(2);
	
	LOG(Info, _T("Recv Command (%s) Sub Command (%s) From AMS..."), strCommand, strSubCommand);

	if(AMS_REQ_STATUS == strCommand)
	{
		LOG(Info, _T("RECV CMD : AMS_REQ_STATUS"));
		bResult = AMS_SendATMStatus(strCommand, strSubCommand);

	}
	else if(AMS_REQ_DEVICE_AND_SYSTEM == strCommand)
	{
		LOG(Info, _T("RECV CMD : AMS_REQ_DEVICE_AND_SYSTEM, Param : (%s)"), strSubCommand);
		
		AMS_ProcDeviceSystem(strSubCommand);

		if (CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetError == FALSE)
		{
			// Device Reset 완료 후 장애가 없을 경우
			if (CDEV_Manager::GetInstance()->m_DEV_CDM.Get_CBXAllNoteCount() <= 0)
			{
				// 1. WDM 논리적인 매수가 0인지 한번 더 Check
				LOG(Error, _T("Set_ErrorCode(%s)"), ERROR_ALL_CST_COUNT_EMPTY);

				CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetError = TRUE;
				CEagleDataManager::GetInstance()->m_Client_Info.m_strErrorCode = ERROR_ALL_CST_COUNT_EMPTY;
			}
			else if (CDEV_Manager::GetInstance()->m_DEV_CDM.Get_CBXAllStatus() == WFS_CDM_STATCUEMPTY)
			{
				// 2. 모든 CST가 Empty인지 Check
				LOG(Error, _T("Set_ErrorCode(%s)"), ERROR_ALL_CST_EMPTY);

				CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetError = TRUE;
				CEagleDataManager::GetInstance()->m_Client_Info.m_strErrorCode = ERROR_ALL_CST_EMPTY;
			}
		}

		if (strSubCommand == AMS_REQ_SUB_DEV_RESET)
		{
			strTemp = _T("[AMS]RESET DEVICE");
			CDEV_Manager::GetInstance()->SaveEJL_SetTextValue(text_field, strTemp);

			bResult = AMS_SendATMSystem(strCommand, strSubCommand);
		}
		else if (strSubCommand == AMS_REQ_SUB_SYS_REBOOT)
		{
			strTemp = _T("[AMS]SYSTEM REBOOT");
			CDEV_Manager::GetInstance()->SaveEJL_SetTextValue(reboot, strTemp);

			AMS_SendATMSystem(strCommand, strSubCommand);

			// EOT 전송
			// Additional Command를 전송하지 못하는 Command에서는 EOT를 전송해서 통신종료를 Server에 알려줌.
			// S/W Update, System Reboot, Setup Write(TerminalIP/DHCP 변경시) 일 경우
			BYTE byDataEOT = DATA_EOT;
			int nResult = HOST_Sync_Send(&byDataEOT, 1, _T(""), _T(""), _T(""), TRUE);

			if(nResult != AMS_OK)
			{
				LOG(Error, _T("System Reboot Send EOT Failed."));
			}

			CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_1000MS);

			// Modem or Network Close 후 System Reboot하도록 로직 수정
			AMS_Sync_ConnectClose();

			CDEV_Manager::GetInstance()->System_Reboot();
		}
	}
	else if( (AMS_REQ_PRINT_JNL == strCommand) || (AMS_REQ_LAST_X_JNL == strCommand) )
	{
		LOG(Info, _T("RECV CMD : AMS_REQ_JNL"));

		if(strArrRecvData.GetCount() > 5)
		{
			bResult = AMS_ProcJournal(strCommand, strSubCommand, strArrRecvData.GetAt(5));
		}
	}
	else if(AMS_REQ_SETUP_READ == strCommand)
	{
		LOG(Info, _T("RECV CMD : AMS_REQ_SETUP_READ"));

		bResult = AMS_SendATMSetupRead(strCommand, strSubCommand);
	}
	else if(AMS_REQ_SETUP_WRITE == strCommand)
	{
		LOG(Info, _T("RECV CMD : AMS_REQ_SETUP_WRITE"));

		AMS_ProcSetupWrite(pBuffer);

		strTemp = _T("[AMS]SET TERMINAL SETUP");
		CDEV_Manager::GetInstance()->SaveEJL_SetTextValue(text_field, strTemp);

		AMS_SendATMSetupWrite(strCommand, strSubCommand);

		// Setup을 적용했으므로 Response 실패와 상관없이 OK로 처리
		bResult = TRUE;
	}
	else if(AMS_REQ_FILE_UPDATE == strCommand)
	{
		LOG(Info, _T("RECV CMD : AMS_REQ_FILE_UPDATE"));

		bResult = AMS_ProcSWUpload(strSubCommand, pBuffer, nRecvLen);
	}
	else if(AMS_REQ_REMOTE_CLOSE == strCommand)
	{
		LOG(Info, _T("RECV CMD : AMS_REQ_REMOTE_CLOSE"));

		if (strSubCommand == AMS_SUB_CBX_CLOSE)	// Remote CBX Close
		{
			bResult = AMS_ProcCbxClose();

			// 논리적인 매수가 초기화 되었으므로 Error를 Set한다.
			if (CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetError == FALSE)
			{
				if (CDEV_Manager::GetInstance()->m_DEV_CDM.Get_CBXAllNoteCount() <= 0)
				{
					// 1. WDM 논리적인 매수가 0인지 한번 더 Check
					LOG(Error, _T("Remote CBX Close - Set_ErrorCode(%s)"), ERROR_ALL_CST_COUNT_EMPTY);

					CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetError = TRUE;
					CEagleDataManager::GetInstance()->m_Client_Info.m_strErrorCode = ERROR_ALL_CST_COUNT_EMPTY;
				}
			}

			strTemp = _T("[AMS]CBX CLOSE");
			CDEV_Manager::GetInstance()->SaveEJL_SetTextValue(text_field, strTemp);
		}
		else
		{
			LOG(Error, _T("Remote Close Sub Command Error"));
		}
	}
	else if(AMS_REQ_CHANGE_LOCALTIME == strCommand)
	{
		LOG(Info, _T("RECV CMD : AMS_REQ_CHANGE_LOCALTIME. Start"));

		CString strDate, strTime;

		if(strArrRecvData.GetCount() > 5)
			strDate = strArrRecvData.GetAt(5);	// YYYYMMDD
		if(strArrRecvData.GetCount() > 6)
			strTime = strArrRecvData.GetAt(6);	// HHMMSS

		if ((strDate.GetLength()) == 8 && (strTime.GetLength() == 6))
		{
			SYSTEMTIME st;
			GetLocalTime(&st);

			// 날짜 및 시간 변경시 Journal 저장 로직 추가
			CString strOldValue, strNewValue;
			strOldValue.Format(_T("[AMS]CHANGE TIME = %02d/%02d/%04d %02d:%02d"), st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute);

			st.wYear = CUtil::StringToInt(strDate.Left(4));
			st.wMonth = CUtil::StringToInt(strDate.Mid(4,2));
			st.wDay = CUtil::StringToInt(strDate.Right(2));

			st.wHour = CUtil::StringToInt(strTime.Left(2));
			st.wMinute = CUtil::StringToInt(strTime.Mid(2,2));
			st.wSecond = CUtil::StringToInt(strTime.Right(2));

			bResult = SetLocalTime(&st);

			CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_1000MS);	// 1초 후에 Send ATM Status 송신

			strNewValue.Format(_T("[AMS]CHANGE TIME = %02d/%02d/%04d %02d:%02d"), st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute);
			CDEV_Manager::GetInstance()->SaveEJL_SetChangeValue(strOldValue, strNewValue);
		}
		else
		{
			LOG(Error,_T("Date and Time Value Error (%s) (%s)"), strDate, strTime);
		}
	}
	else if(AMS_REQ_LOG_UPLOAD == strCommand)
	{
		LOG(Info, _T("RECV CMD : AMS_REQ_LOG_UPLOAD"));

		if(strArrRecvData.GetCount() > 5)
			bResult = AMS_ProcLog(strCommand, strSubCommand, strArrRecvData[5]);
	}
	else if(AMS_REQ_IMG_UPLOAD == strCommand)
	{
		if (strSubCommand == AMS_SUB_CAMIMG_UPLOAD)
		{
			if(strArrRecvData.GetCount() > 5)
				bResult = AMS_ProcCAMImageUpload(strCommand, strSubCommand, strArrRecvData[5]);
		}
		else
		{
			LOG(Error, _T("[AMS] Image Upload Sub Command Error - %s"), strSubCommand);
		}
	}
	else
	{
		LOG(Info, _T("AMS_ParseCMD() Command Undefined (%s)"), strCommand);
	}
	
	return bResult;
}

/** ********************************************************************
* @brief AMS로 Status 정보 전송
* @param CString strCommand		Command
* @param CString strSubCommand	SubCommand
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
BOOL CDEV_HOST::AMS_SendATMStatus(CString strCommand, CString strSubCommand)
{
	int nSendLen = 0;
	int nRet = 0;

	memset(m_pAMSSendBuffer, 0, NET_SEND_BUFF_SIZE);

	nSendLen = AMS_MakeStatusMsg(strCommand, strSubCommand, NET_SEND_BUFF_SIZE, m_pAMSSendBuffer);

	LOG(Info, _T("AMS_SendATMStatus() Send Status Msg To AMS.."));

	// +1 => 0x1C
	nRet = HOST_Sync_Send((BYTE*)m_pAMSSendBuffer, nSendLen + LEN_LENGTH + 1, _T(""), _T(""), _T(""), TRUE);

	if(AMS_OK != nRet)
	{
		LOG(Error, _T("AMS_SendATMStatus() Send Status Failed."));
		return FALSE;
	}
	else
	{
		LOG(Info, _T("AMS_SendATMStatus() Send Status SUCCESS"));
	}

	return TRUE;
}


/** ********************************************************************
* @brief AMS로 전송할 Status Data
* @param CString strCommand		Command
* @param CString strSubCommand	Sub Command
* @param int nLen				Max Length
* @param BYTE *pbyData			StatusData
* @param int &nSendLen			송신 전문 길이
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
int CDEV_HOST::AMS_MakeStatusMsg(CString strCommand, CString strSubCommand, int nLen, BYTE *pbyData)
{
	BYTE byCommonData[2048] = {0,};
	CString strSendData;
	int nSendLen = 0;

	AMS_MakeHeaderField(strCommand, strSubCommand, strSendData);
	AMS_MakeCommonField(strSendData);

	strSendData += CString(FS);

	// Length
	nSendLen = strSendData.GetLength();
	strSendData.Insert(0, CUtil::IntToStringEx(nSendLen, LEN_LENGTH));
	strSendData.Insert(0, CString(FS));

	WideToMulti(strSendData, (char*)pbyData, strSendData.GetLength());
	
	return nSendLen;
}


/** ********************************************************************
* @brief Header 정보 세팅
* @param CString strCommand		Command
* @param CString strSubCommand	Sub Command
* @param BYTE *pbyData			Data
* @param int &nSendLen			송신 전문 길이
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
BOOL CDEV_HOST::AMS_MakeHeaderField(CString strCommand, CString strSubCommand, CString &strSendData)
{
	CString strTemp;

	strSendData = CString(FS);
	strSendData += strCommand;
	strSendData += CString(FS);
	strSendData += strSubCommand;
	strSendData += CString(FS);

	// Serial Number
	strTemp.Format(_T("%s"), CEagleDataManager::GetInstance()->m_Config.m_Option.strMachine_Serial_Number);
	strSendData += strTemp;
	strSendData += CString(FS);

	// TerminalID
	strTemp.Format(_T("%s"), CEagleDataManager::GetInstance()->m_Config.m_Option.strTerminal_ID);
	strSendData += strTemp;
	strSendData += CString(FS);

	// TerminalPW
	strTemp = CEagleDataManager::GetInstance()->m_Config.LoadSystemData(_T("PASSWORD"), _T("ams_pwd"));
	strSendData += strTemp;
	strSendData += CString(FS);

	return TRUE;
}


/** ********************************************************************
* @brief CommonField 정보 세팅
* @param BYTE *pbyData				Common Data
* @param CString &strSendData		Send Data
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
BOOL CDEV_HOST::AMS_MakeCommonField(CString &strSendData)
{
	DWORD	dwCBXStatus[MAX_CASSETTE + 1] = {0,};		// Reject Bin / CST1 ~ 4
	CString strCommonData;
	CString strTemp;
	BYTE byCDMStatus[4] = {0,};
	BYTE byPTRMediaStatus = 0;
	CString strData = _T("");
	CStringArray strArrData;
	int	nCSTCnt = 0;

	int nMajor = 0, nMinor = 0;
	BOOL bResult = FALSE;
	EPPRESULT eppResult;
	BYTE byEPPType = 0;

	memset(&eppResult, 0, sizeof(EPPRESULT));

	// 2. Machine Type 추가
	if (GetSystemMetrics(SM_CXSCREEN) == CS130_WIDTH_RESOLUTION)
		strTemp = CS130_MACHINE_TYPE;
	else
		strTemp = MF200_MACHINE_TYPE;

	strCommonData += strTemp;
	strCommonData += CString(RS);

	// 2. mode status
	if (CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetError == TRUE)
		strTemp = _T("O");
	else
		strTemp = _T("I");

	strCommonData += strTemp;
	strCommonData += CString(RS);

	// 3. error code
	strTemp = CEagleDataManager::GetInstance()->m_Client_Info.m_strErrorCode;
	strCommonData += strTemp;
	strCommonData += CString(RS);

	strTemp = strCommonData;
	strTemp.Replace(CString(RS), _T("(RS)"));
	LOG(Info, _T("ATM Status Common Data (%s)"), strTemp);

	// Protocol 변경에 따른 수정
	// 4. Registered Count - 마지막에 Insert
	CString strRegisteredData;

	// Terminal Information
	// 5. Network Type
	if (CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType == _T("TCP/IP"))
		strTemp = _T("0");
	else
		strTemp = _T("1");
	strRegisteredData += strTemp;
	strRegisteredData += CString(RS);

	// 6. IP Information
	if (CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType == _T("TCP/IP"))
	{
		strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strUseDHCP;
		strRegisteredData += strTemp;
		strRegisteredData += CString(RS);

		if(S_ENABLE == CEagleDataManager::GetInstance()->m_Config.m_Host.strUseDHCP)
		{
			// Terminal IP
			strTemp.Empty();
			CUtil::GetDHCP_IP(strTemp);
			strRegisteredData += strTemp;
			strRegisteredData += CString(RS);

			// Subnet
			strTemp.Empty();
			CUtil::GetDHCP_SubnetMask(strTemp);
			strRegisteredData += strTemp;
			strRegisteredData += CString(RS);

			// GateWay
			strTemp.Empty();
			CUtil::GetDHCP_GateWay(strTemp);
			strRegisteredData += strTemp;
			strRegisteredData += CString(RS);

			// DNS
			strTemp.Empty();
			CUtil::GetDHCP_DNS(strTemp);
			strRegisteredData += strTemp;
			strRegisteredData += CString(RS);
		}
		else
		{
			// Static Terminal IP
			strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strTerminalip;
			strRegisteredData += strTemp;
			strRegisteredData += CString(RS);

			// Static Subnet
			strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strSubnet;
			strRegisteredData += strTemp;
			strRegisteredData += CString(RS);

			// Static GateWay
			strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strGateway;
			strRegisteredData += strTemp;
			strRegisteredData += CString(RS);

			// Static DNS
			strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strDNS;
			strRegisteredData += strTemp;
			strRegisteredData += CString(RS);
		}
	}
	else
	{
		strRegisteredData += CString(RS);
		strRegisteredData += CString(RS);
		strRegisteredData += CString(RS);
		strRegisteredData += CString(RS);
		strRegisteredData += CString(RS);
	}

	// OS Version
	strTemp = CUtil::String_GetRegistry(REGISTRY_OS_INFO_KEY, REGISTRY_KERNEL_VALUE_NAME);
	strTemp.Replace(_T("v"), _T("V"));
	strRegisteredData += strTemp;
	strRegisteredData += CString(RS);

	// AP version
	CString strProductVersion, strAPVersion;
	strProductVersion = CDEV_Manager::GetInstance()->m_strAP_Version;
	strAPVersion.Format(_T("V%s"), strProductVersion);

	strTemp = strAPVersion;
	strRegisteredData += strTemp;
	strRegisteredData += CString(RS);

	// agent version
	strTemp = AMS_AGENT_VERSION;
	strRegisteredData += strTemp;
	strRegisteredData += CString(RS);

	// Sub Module OS Version - CS130은 없음
	strRegisteredData += CString(RS);

	// Sub Module Version - CS130은 없음
	strRegisteredData += CString(RS);

	// 해상도 정보
	strTemp.Format(_T("%d_%d"), GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
	strRegisteredData += strTemp;
	strRegisteredData += CString(RS);

	// Support Top and Middle Images - CS130 / MF200 not support
	strRegisteredData += _T("0");
	strRegisteredData += CString(RS);

	// Registered Item Count 넣기
	strArrData.RemoveAll();
	CUtil::ParsingStringToStringArray(strRegisteredData, (CString)RS, strArrData);

	strTemp.Format(_T("%d"), strArrData.GetCount());
	strRegisteredData.Insert(0, CString(RS));
	strRegisteredData.Insert(0, strTemp);

	strCommonData += strRegisteredData;

	strTemp = strRegisteredData;
	strTemp.Replace(CString(RS), _T("(RS)"));
	LOG(Info, _T("ATM Status Registered Data (%s)"), strTemp);

	// Updated Time Item Count - 마지막에 넣기
	CString strUpdatedTimeData;

	// ATM Local Time 추가
	SYSTEMTIME	st;
	GetLocalTime(&st);

	strTemp.Format(_T("%04d%02d%02d%02d%02d%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	strUpdatedTimeData += strTemp;
	strUpdatedTimeData += CString(RS);

	strArrData.RemoveAll();
	CUtil::ParsingStringToStringArray(strUpdatedTimeData, (CString)RS, strArrData);

	strTemp.Format(_T("%d"), strArrData.GetCount());
	strUpdatedTimeData.Insert(0, CString(RS));
	strUpdatedTimeData.Insert(0, strTemp);

	strCommonData += strUpdatedTimeData;

	strTemp = strUpdatedTimeData;
	strTemp.Replace(CString(RS), _T("(RS)"));
	LOG(Info, _T("ATM Status UpdatedTime Data (%s)"), strTemp);

	// Device Version Item Count - 나중에 넣기
	CString strDevVerData;

	// CDR version
	strData.Empty();
	strData = CDEV_Manager::GetInstance()->m_DEV_CDR.m_strCDR_FW_Version;
	strTemp = strData;
	strDevVerData += strTemp;
	strDevVerData += CString(RS);

	// CDM version - 통신 장애시 AMS와 Delay 현상 발생을 최소화 하기 위해 로직 변경
	strData.Format(_T("V%s"), CDEV_Manager::GetInstance()->m_DEV_CDM.m_strWDM_FW_Version);
	strTemp = strData;
	strDevVerData += strTemp;
	strDevVerData += CString(RS);

	// PTR version
	strData.Format(_T("V%s"), CDEV_Manager::GetInstance()->m_DEV_PRT.m_strPRT_FW_Version);
	strTemp = strData;
	strDevVerData += strTemp;
	strDevVerData += CString(RS);

	// EPP version
	strData = CDEV_Manager::GetInstance()->m_DEV_CREPP.m_strREPP_FW_Version;
	strTemp = strData;
	strDevVerData += strTemp;
	strDevVerData += CString(RS);

	// SIU version
	strData = CDEV_Manager::GetInstance()->m_DEV_DIO.m_strDIO_FW_Version;
	strTemp = strData;
	strDevVerData += strTemp;
	strDevVerData += CString(RS);

	// CAM Version
	strTemp = CDEV_Manager::GetInstance()->m_DEV_CAMERA.GetVersion();
	strDevVerData += strTemp;
	strDevVerData += CString(RS);

	strArrData.RemoveAll();
	CUtil::ParsingStringToStringArray(strDevVerData, (CString)RS, strArrData);

	strTemp.Format(_T("%d"), strArrData.GetCount());
	strDevVerData.Insert(0, CString(RS));
	strDevVerData.Insert(0, strTemp);

	strCommonData += strDevVerData;

	strTemp = strDevVerData;
	strTemp.Replace(CString(RS), _T("(RS)"));
	LOG(Info, _T("ATM Status Device Version Data (%s)"), strTemp);

	// CBX Item Count - 마지막에 Insert
	CString strCBXData;
	
	// cassette count
	nCSTCnt = CDEV_Manager::GetInstance()->m_DEV_CDM.m_nCbxCount;
	strTemp.Format(_T("%d"), nCSTCnt);
	strCBXData += strTemp;
	strCBXData += CString(RS);

	CDEV_Manager::GetInstance()->m_DEV_CDM.GetCbxStatus(dwCBXStatus);	// dwCBXStatus[0]:Rej, dwCBXStatus[1] ~ [4]:Cassette

	for(int i=0; i<nCSTCnt; i++)
	{
		strTemp.Format(_T("%d"), CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Load_Count[i]));
		strCBXData += strTemp;
		strCBXData += CString(RS);

		strTemp.Format(_T("%d"), CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Dispense_Count[i]));
		strCBXData += strTemp;
		strCBXData += CString(RS);

		strTemp.Format(_T("%d"), CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Item_Count[i]));
		strCBXData += strTemp;
		strCBXData += CString(RS);

		strTemp.Format(_T("%d"), CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Reject_Count[i]));
		strCBXData += strTemp;
		strCBXData += CString(RS);

		// Reject 누적 사유 추가
		strTemp = CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Reject_Reason[i];
		strCBXData += strTemp;
		strCBXData += CString(RS);

		// Cent 추가
		strTemp.Format(_T("%d%s"), CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Denomination[i]), _T("00"));
		strCBXData += strTemp;
		strCBXData += CString(RS);

		if(WFS_CDM_STATCUMISSING == dwCBXStatus[i+1])
			strTemp = _T("M");
		else if(WFS_CDM_STATCUEMPTY == dwCBXStatus[i+1])
			strTemp = _T("E");
		else if(WFS_CDM_STATCULOW == dwCBXStatus[i+1])
			strTemp = _T("L");
		else
			strTemp = _T("N");

		strCBXData += strTemp;
		strCBXData += CString(RS);
	}

	strArrData.RemoveAll();
	CUtil::ParsingStringToStringArray(strCBXData, (CString)RS, strArrData);

	strTemp.Format(_T("%d"), strArrData.GetCount());
	strCBXData.Insert(0, CString(RS));
	strCBXData.Insert(0, strTemp);

	strCommonData += strCBXData;

	strTemp = strCBXData;
	strTemp.Replace(CString(RS), _T("(RS)"));
	LOG(Info, _T("ATM Status CBX Data (%s)"), strTemp);

	// 37. media status item count - 마지막에 Insert
	CString strMediaStatusData;

	// 38. PTR status
	if (CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_GetStatus() == EAGLE_PRT_NORMAL)
	{
		if(EAGLE_PRT_PAPER_NEAR == CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_GetPaperStatus())
			strTemp = _T("L");
		else if(EAGLE_PRT_PAPER_EMPTY == CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_GetPaperStatus())
			strTemp = _T("E");
		else
			strTemp = _T("N");
	}
	else
	{
		// Printer가 장애시 Media 조회가 안되므로 Error값으로 송신
		strTemp = _T("P");
	}

	strMediaStatusData += strTemp;
	strMediaStatusData += CString(RS);

	strArrData.RemoveAll();
	CUtil::ParsingStringToStringArray(strMediaStatusData, (CString)RS, strArrData);

	strTemp.Format(_T("%d"), strArrData.GetCount());
	strMediaStatusData.Insert(0, CString(RS));
	strMediaStatusData.Insert(0, strTemp);

	strCommonData += strMediaStatusData;

	strTemp = strMediaStatusData;
	strTemp.Replace(CString(RS), _T("(RS)"));
	LOG(Info, _T("ATM Status Media Status Data (%s)"), strTemp);

	// Journal Data Item Count - 마지막에 Insert
	CString strJNLData;

	// 39. journal record count(AMS로 올린 journal 개수)
	strTemp.Format(_T("%d"), CDEV_Manager::GetInstance()->m_DEV_EJL.GetUploadedAMSJnlNo());
	strJNLData += strTemp;
	strJNLData += CString(RS);

	// 40. journal total count(현재 저장되어 있는 journal 개수)
	strTemp.Format(_T("%d"), CDEV_Manager::GetInstance()->m_DEV_EJL.GetLastJournalNo());
	strJNLData += strTemp;
	strJNLData += CString(RS);

	strArrData.RemoveAll();
	CUtil::ParsingStringToStringArray(strJNLData, (CString)RS, strArrData);

	strTemp.Format(_T("%d"), strArrData.GetCount());
	strJNLData.Insert(0, CString(RS));
	strJNLData.Insert(0, strTemp);

	strCommonData += strJNLData;

	strTemp = strJNLData;
	strTemp.Replace(CString(RS), _T("(RS)"));
	LOG(Info, _T("ATM Status journal Data (%s)"), strTemp);

	// CAMERA Info Item Count
	CString CAMInfoData;

	// 42. Camera 정보 추가
	if (CDEV_Manager::GetInstance()->m_DEV_CAMERA.IsExistCameraDriver() == TRUE)
	{
		// Camera가 있는 경우

		// 1. Camera Capability
		strTemp = S_ENABLE;
		CAMInfoData += strTemp;
		CAMInfoData += CString(RS);

		// 2. 사용 유무
		if (CDEV_Manager::GetInstance()->m_DEV_CAMERA.GetCameraSetting() == TRUE)
			strTemp = S_ENABLE;
		else
			strTemp = S_DISABLE;

		CAMInfoData += strTemp;
		CAMInfoData += CString(RS);
	}
	else
	{
		// Camera가 없는 경우

		// 1. Camera Capability
		strTemp = S_DISABLE;
		CAMInfoData += strTemp;
		CAMInfoData += CString(RS);

		// 사용 유무 NULL
		CAMInfoData += CString(RS);
	}

	strArrData.RemoveAll();
	CUtil::ParsingStringToStringArray(CAMInfoData, (CString)RS, strArrData);

	strTemp.Format(_T("%d"), strArrData.GetCount());
	CAMInfoData.Insert(0, CString(RS));
	CAMInfoData.Insert(0, strTemp);

	strCommonData += CAMInfoData;

	strTemp = CAMInfoData;
	strTemp.Replace(CString(RS), _T("(RS)"));
	LOG(Info, _T("ATM Status camera Data (%s)"), strTemp);

	// Coin Info - not used
	/*
	strCommonData += _T("0");	// Item Count - 0
	strCommonData += CString(RS);

	// Check Info - not used
	strCommonData += _T("0");	// Item Count - 0
	strCommonData += CString(RS);

	// Sidecar Info - not used
	strCommonData += _T("0");	// Item Count - 0
	strCommonData += CString(RS);
	*/

	// ADV Info
	CString strADVInfo, strADTitleFileName;

	// ADV Title
	strADTitleFileName.Format(_T("%s%s\\%d_%d%s"), CUtil::GetAppPath(), EAGLE_ATM_AD_PATH, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) ,AD_TITLE_FILE);

	CIniFile iniFile(strADTitleFileName);

	strTemp = iniFile.ReadString(_T("AD_INFO"), _T("AD_TITLE"), _T(""));
	strADVInfo += strTemp;
	strADVInfo += CString(RS);

	// ADV Unique ID
	strTemp = iniFile.ReadString(_T("AD_INFO"), _T("AD_UNIQUE_KEY"), _T(""));
	strADVInfo += strTemp;
	strADVInfo += CString(RS);

	// Advertisement interval time
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_ADS.strAdver_Interval_time;
	strADVInfo += strTemp;
	// 구분자
	strADVInfo += CString(RS);

	// Advertisement screen
	for(int i=0; i<SUPPORT_ADVERTISE; i++)
	{
		strTemp = CEagleDataManager::GetInstance()->m_Config.m_ADS.strAdver_Screen_en[i];
		strADVInfo += strTemp;
		// 구분자
		strADVInfo += CString(RS);
	}

	strArrData.RemoveAll();
	CUtil::ParsingStringToStringArray(strADVInfo, (CString)RS, strArrData);

	strTemp.Format(_T("%d"), strArrData.GetCount());
	strADVInfo.Insert(0, CString(RS));
	strADVInfo.Insert(0, strTemp);

	strCommonData += strADVInfo;

	strTemp = strADVInfo;
	strTemp.Replace(CString(RS), _T("(RS)"));
	LOG(Info, _T("ATM Status ADV. Data (%s)"), strTemp);

	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

	strArrData.RemoveAll();
	CUtil::ParsingStringToStringArray(strCommonData, (CString)RS, strArrData);

	// 1. Common Field Item Count & 구분자
	strTemp.Format(_T("%d"), strArrData.GetCount());
	strCommonData.Insert(0, CString(RS));
	strCommonData.Insert(0, strTemp);

	strSendData += strCommonData;

	return TRUE;
}

/** ********************************************************************
* @brief AMS로 System 정보 전송
* @param CString strCommand		Command
* @param CString strSubCommand	Sub Command
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
BOOL CDEV_HOST::AMS_SendATMSystem(CString strCommand, CString strSubCommand)
{
	int nSendLen = 0;
	int nRet = 0;

	memset(m_pAMSSendBuffer, 0, NET_SEND_BUFF_SIZE);

	nSendLen = AMS_MakeSystemMsg(strCommand, strSubCommand, NET_SEND_BUFF_SIZE, m_pAMSSendBuffer);

	LOG(Info, _T("AMS_SendATMSystem() Send Device/System Msg To AMS.."));

	// +1 => 0x1C
	nRet = HOST_Sync_Send((BYTE*)m_pAMSSendBuffer, nSendLen + LEN_LENGTH + 1, _T(""), _T(""), _T(""), TRUE);

	if(AMS_OK != nRet)
	{
		LOG(Error, _T("AMS_SendATMSystem() Send Device/System Failed."));
		return FALSE;
	}

	LOG(Info, _T("AMS_SendATMSystem() Send Device/System SUCCESS"));

	return TRUE;
}


/** ********************************************************************
* @brief AMS로 전송할 System Data
* @param CString strCommand		Command
* @param CString strSubCommand	Sub Command
* @param int nLen				Max Length
* @param BYTE *pbyData			SystemData
* @param int &nSendLen			송신 전문 길이
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
int CDEV_HOST::AMS_MakeSystemMsg(CString strCommand, CString strSubCommand, int nLen, BYTE *pbyData)
{
	int nSendLen = 0;
	CString strSendData;

	// check buffer size.
	if (sizeof(AMS_RES_SYSTEM) > nLen)
	{
		LOG(Error, _T("AMS_MakeSystemMsg() Verify Data : SIZE(AMS_RES_SYSTEM) is invalid."));
		return FALSE;
	}

	AMS_MakeHeaderField(strCommand, strSubCommand, strSendData);
	
	// Length
	nSendLen = strSendData.GetLength();
	strSendData.Insert(0, CUtil::IntToStringEx(nSendLen, LEN_LENGTH));
	strSendData.Insert(0, CString(FS));

	WideToMulti(strSendData, (char*)pbyData, strSendData.GetLength());

	return nSendLen;
}


/** ********************************************************************
* @brief Device Reset과 System Reboot 동작 수행
* @param CString strSubCommand	SubCommand('A':Device Reset, 'B':System Reboot)
* @retval 없음
************************************************************************/
void CDEV_HOST::AMS_ProcDeviceSystem(CString strSubCommand)
{
	BOOL bResult = FALSE;
	int nResult = 0;
	CString strErrorCode;

	if(AMS_REQ_SUB_DEV_RESET == strSubCommand)	// Device Reset
	{
		// 1. CDM Reset
		CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_QuiryStatusStop();	// Polling Stop

		bResult = CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_Initialize();

		CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_QuiryStatusStart();	// Polling Start

		if(FALSE == bResult)
		{
			strErrorCode = CDEV_Manager::GetInstance()->m_DEV_CDM.GetLastError();
			LOG(Error, _T("AMS_ProcDeviceSystem() CDM_Reset is failed. Error Code : ") + strErrorCode);

			Set_DeviceErrorCode(DEVICE_CDM, strErrorCode);
		}
		else
		{
			CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_CfgStatus();	// Version Read를 위해 추가로 Call

			Clear_DeviceError(DEVICE_CDM);
		}
		
		// 2. CDR Reset
		bResult = CDEV_Manager::GetInstance()->m_DEV_CDR.Initialize();
		if(FALSE == bResult)
		{
			strErrorCode = CDEV_Manager::GetInstance()->m_DEV_CDR.GetLastError();
			LOG(Error, _T("AMS_ProcDeviceSystem() CDR_Reset is failed. Error Code : ") + strErrorCode);

			Set_DeviceErrorCode(DEVICE_CDR, strErrorCode);
		}
		else
		{
			CDEV_Manager::GetInstance()->m_DEV_CDR.GetVersionInfo();

			Clear_DeviceError(DEVICE_CDR);
		}

		// 3. PTR Reset
		
		// PTR Quiry Status Stop
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_QuiryStatusStop();	// Polling 로직 누락 추가

		if (CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_Reset() == EAGLE_PRT_SUCCESS)
		{
			// RESET 성공 후 Version Read하도록 추가
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_GetVersionInfo();
		}

		// PTR Quiry Status Start
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_QuiryStatusStart();	// Polling 로직 누락 추가

		CUtil::Sleep_Wait(2000);

		if(EAGLE_PRT_NORMAL != CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_GetStatus())
		{
			strErrorCode = CDEV_Manager::GetInstance()->m_DEV_PRT.GetLastError();
			LOG(Error, _T("AMS_ProcDeviceSystem() PTR_Reset is failed. Error Code : ") + strErrorCode);

//			Set_DeviceErrorCode(DEVICE_PTR, strErrorCode);	// Reset후에 PTR은 Error를 Set하지 않음.
		}
		else
		{
			Clear_DeviceError(DEVICE_PTR);
		}
	}
	else if(AMS_REQ_SUB_SYS_REBOOT == strSubCommand)	// System Reboot
	{
		;
	}
	else
	{
		LOG(Info, _T("AMS_ProcDeviceSystem() SubCommand Undefined[%s]"), strSubCommand);
	}
}


/** ********************************************************************
* @brief Device Reset과 System Reboot 동작 수행
* @param CString strCommand		Command('C':Print Journal, 'D':Last X Journal)
* @param CString strSubCommand	Sub Command
* @param CString strJournalCount	Last Journal 조회 개수
* @retval 없음
************************************************************************/
BOOL CDEV_HOST::AMS_ProcJournal(CString strCommand, CString strSubCommand, CString strJournalCount)
{
	BOOL bRet = FALSE;
	int	nCurrentJnlNo = 0;		
	int nExistJnlCount = 0;
	int nLastJnlNumberToAMS = 0;

	FLSEJCOMMINFO CommInfo;
	FLSEJINFO	JnlInfo;

	memset(&CommInfo, 0x00, sizeof(CommInfo));
	memset(&JnlInfo, 0x00, sizeof(JnlInfo));

	m_nJournalCount = 0;

	nExistJnlCount = CDEV_Manager::GetInstance()->m_DEV_EJL.GetLastJournalNo();

	if(AMS_REQ_PRINT_JNL == strCommand)
	{
		nLastJnlNumberToAMS = CDEV_Manager::GetInstance()->m_DEV_EJL.GetUploadedAMSJnlNo();	// AMS Server로 업로드된 마지막 journal number
		nCurrentJnlNo = nLastJnlNumberToAMS + 1;

		m_nJournalCount = nExistJnlCount - nLastJnlNumberToAMS;

		LOG(Info, _T("AMS_ProcJournal() Send Journal Msg To AMS..(Exist : %d, Uploaded AMS : %d, Search Journal : %d)"), nExistJnlCount, nLastJnlNumberToAMS, nCurrentJnlNo);

		while(TRUE)
		{
			if(nCurrentJnlNo > nExistJnlCount)
			{
				LOG(Info, _T("AMS_ProcJournal() No Information to upload"));
				bRet = TRUE;	// 이후 Additional Command 명령을 위해 데이터가 없어도 성공처리 함.
				break;
			}

			if (CDEV_Manager::GetInstance()->m_DEV_EJL.GetDetailJnlData(nCurrentJnlNo, &CommInfo, &JnlInfo) == TRUE)
			{

				m_nJournalCount--;
				
				bRet = AMS_SendATMJournal(strCommand, strSubCommand, CommInfo, JnlInfo);
			}
			else
			{
				LOG(Error, _T("GetDetailJnlData return FALSE"));
				bRet = FALSE;
			}

			if(FALSE == bRet)
			{
				LOG(Error, _T("AMS Send Journal is failed"));
				break;
			}
			else
			{
				CDEV_Manager::GetInstance()->m_DEV_EJL.SetUploadedAMSJnlNo(nCurrentJnlNo);
			}

			if (nCurrentJnlNo == nExistJnlCount)
			{
				LOG(Info, _T("AMS Send Journal is completed (%d), (%d)"), nCurrentJnlNo, nExistJnlCount);
				break;
			}

			nCurrentJnlNo++;
		}

		LOG(Info, _T("RECV CMD : AMS_REQ_PRINT_JNL"));
	}
	else if(AMS_REQ_LAST_X_JNL == strCommand)	// Last X Journal
	{
		m_nJournalCount = CUtil::StringToInt(strJournalCount);
		LOG(Info, _T("AMS_ProcJournal() Request Last X Journal Count : [%d]"), m_nJournalCount);

		// 입력한 journal count보다 실제 저널이 적게 있는 경우 있는 저널만 Print.
		if (m_nJournalCount > nExistJnlCount)
		{
			m_nJournalCount = nExistJnlCount;
			LOG(Info, _T("AMS_ProcJournal() Modified Journal Count : [%d]"), m_nJournalCount);
		}

		nCurrentJnlNo = nExistJnlCount;

		while(TRUE)
		{
			if (m_nJournalCount <= 0)
			{
				bRet = TRUE;	// 이후 Additional Command 명령을 위해 데이터가 없어도 성공처리 함.
				break;
			}

			if (CDEV_Manager::GetInstance()->m_DEV_EJL.GetDetailJnlData(nCurrentJnlNo, &CommInfo, &JnlInfo) == TRUE)
			{
				m_nJournalCount--;
				nCurrentJnlNo--;

				bRet = AMS_SendATMJournal(strCommand, strSubCommand, CommInfo, JnlInfo);
			}
			else
			{
				LOG(Error, _T("Last X - GetDetailJnlData return FALSE"));
				bRet = FALSE;
			}

			if(FALSE == bRet)
			{
				LOG(Error, _T("Last X - AMS Send Journal is failed"));
				break;
			}
		}
	}
	else
	{
		LOG(Info, _T("AMS_ProcJournal() Command Undefined[%s]"), strCommand);
	}

	return bRet;
}


/** ********************************************************************
* @brief AMS로 Last Journal 정보 전송
* @param CString strCommand		Command
* @param CString strSubCommand	Sub Command
* @param FLSEJCOMMINFO CommInfo		JournalData
* @param FLSEJINFO JnlInfo			JournalData
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
BOOL CDEV_HOST::AMS_SendATMJournal(CString strCommand, CString strSubCommand, FLSEJCOMMINFO CommInfo, FLSEJINFO JnlInfo)
{
	int nSendLen = 0;
	int nJournalDataLen = 0;
	int nRet = 0;

	memset(m_pAMSSendBuffer, 0, NET_SEND_BUFF_SIZE);

	nSendLen = AMS_MakeJournalMsg(strCommand, strSubCommand, CommInfo, JnlInfo, NET_SEND_BUFF_SIZE, m_pAMSSendBuffer);

	// +1 => 0x1C
	nRet = HOST_Sync_Send((BYTE*)m_pAMSSendBuffer, nSendLen + LEN_LENGTH + 1, _T(""), _T(""), _T(""), TRUE);

	if(AMS_OK != nRet)
	{
		LOG(Error, _T("AMS_SendATMLastJournal() Send Last Journal Failed."));
		return FALSE;
	}

	return TRUE;
}


/** ********************************************************************
* @brief AMS로 전송할 Journal Data
* @param CString strCommand		Command
* @param CString strSubCommand	Sub Command
* @param FLSEJCOMMINFO CommInfo		JournalData
* @param FLSEJINFO JnlInfo			JournalData
* @param int nLen				Max Length
* @param BYTE *pbyData			Journal Data
* @param int &nSendLen			송신 전문 길이
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
int CDEV_HOST::AMS_MakeJournalMsg(CString strCommand, CString strSubCommand, FLSEJCOMMINFO CommInfo, FLSEJINFO JnlInfo, int nLen, BYTE *pbyData)
{
	CString strSendData;
	CString strTemp;
	int nSendLen = 0;

	// Header data
	AMS_MakeHeaderField(strCommand, strSubCommand, strSendData);

	// Current Journal Count
	strTemp.Format(_T("%d"), m_nJournalCount);
	strSendData += strTemp;

	// 구분자
	strSendData += CString(FS);

	// Get journal data
	AMS_Get_JournalData(CommInfo, JnlInfo, strSendData);
	strSendData += CString(FS);

	// Length
	nSendLen = strSendData.GetLength();
	strSendData.Insert(0, CUtil::IntToStringEx(nSendLen, LEN_LENGTH));
	strSendData.Insert(0, CString(FS));

	WideToMulti(strSendData, (char*)pbyData, strSendData.GetLength());

	return nSendLen;
}


/** ********************************************************************
* @brief AMS로 전송할 Journal Data
* @param FLSEJCOMMINFO CommInfo		JournalData
* @param FLSEJINFO JnlInfo			JournalData
* @param CString &strSendData		송신 전문
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
void CDEV_HOST::AMS_Get_JournalData(FLSEJCOMMINFO CommInfo, FLSEJINFO JnlInfo, CString &strSendData)
{
	CString strTemp, strTransType, strReturnValue, strTemp2, strTemp3;

	int	nCSTCnt = 0;
	int i=0;
	int nValueLen = 0;

	nCSTCnt = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Count);

	// Common Info
	// 1. JNL NO
	strTemp.Format(_T("%d"), CommInfo.jnl_no);
	strReturnValue += strTemp;
	// 구분자
	strReturnValue += CString(RS);

	// 2. LOG DATE
	strTemp.Format(_T("%S"), CommInfo.log_date);
	strTemp.Replace(_T("/"), _T(""));
	strReturnValue += strTemp;
	// 구분자
	strReturnValue += CString(RS);
	
	// 3. LOG TIME
	strTemp.Format(_T("%S"), CommInfo.log_time);
	strTemp.Replace(_T(":"), _T(""));
	strReturnValue += strTemp;
	// 구분자
	strReturnValue += CString(RS);
	
	// 4. JNL CODE
	strTemp.Format(_T("%d"), CommInfo.jnl_cd);
	strReturnValue += strTemp;
	// 구분자
	strReturnValue += CString(RS);
	
	// 5. LOG SEARCH DATE
	strTemp.Format(_T("%S"), CommInfo.log_srch_date);
	strReturnValue += strTemp;
	// 구분자
	strReturnValue += CString(RS);
	////////////////////////////////////////////////////////////////////////////

	switch(CommInfo.jnl_cd)
	{
	case transaction:
	case withdrawal:
	case balance:
	case transfer:
		{
			// Data Length
			// 1. Jnl no
			strTemp.Format(_T("%d"), JnlInfo.tran_info.jnl_no);
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// 2. Terminal ID
			strTemp.Format(_T("%S"), JnlInfo.tran_info.term_id);
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// 3. Sequence No
			strTemp.Format(_T("%S"), JnlInfo.tran_info.tran_seq_no);
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// 4. TRANSACTION
			strTransType.Format(_T("%S"), JnlInfo.tran_info.tran_cd);
			strTemp.Format(_T("%s"), strTransType);
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// 5. CARD DATA
			strTemp.Format(_T("%S"), JnlInfo.tran_info.card_tr2);
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// 6. AUTHORIZATION NO.
			strTemp.Format(_T("%S"), JnlInfo.tran_info.auth_no);
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// 7. DATE
			// HOST에서 MM/DD/YYYY Format으로 된 걸 -> YYYY-MM-DD로 변경
			strTemp2.Format(_T("%S"), JnlInfo.tran_info.tran_date);
			strTemp2.Replace(_T("/"), _T(""));
	
			if (strTemp2.IsEmpty() == FALSE)
				strTemp.Format(_T("%4.4s%2.2s%2.2s"), strTemp2.Right(4), strTemp2.Left(2), strTemp2.Mid(2,2));
			else
				strTemp.Empty();
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// 8. TIME
			strTemp.Format(_T("%S"), JnlInfo.tran_info.tran_time);
			strTemp.Replace(_T(":"), _T(""));
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// 9. BUSINESS DATE
			strTemp2.Format(_T("%S"), JnlInfo.tran_info.business_date);
			strTemp2.Replace(_T("/"), _T(""));

			if (strTemp2.IsEmpty() == FALSE)
				strTemp.Format(_T("%4.4s%2.2s%2.2s"), strTemp2.Right(4), strTemp2.Left(2), strTemp2.Mid(2,2));
			else
				strTemp.Empty();
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// 10. ACCOUNT
			strTemp.Format(_T("%S"), JnlInfo.tran_info.account_type);
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			int nRequestedAmt = 0;
			int nDispensedAmt = 0;
			int nSurchargeAmt = 0;
			int nTemp = 0;

			if (strTransType == S_WITHDRAWAL)
			{
				// 11. REQUESTED
				strTemp.Format(_T("%S"), JnlInfo.tran_info.req_amt);
				nRequestedAmt = CUtil::StringToInt(strTemp);

				if (nRequestedAmt <= 0)
					strTemp = _T("000");
				else
					strTemp += _T("00");

				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);

				// 12. DISPENSED
				strTemp.Format(_T("%S"), JnlInfo.tran_info.disp_amt);
				nDispensedAmt = CUtil::StringToInt(strTemp);

				if (nDispensedAmt <= 0)
					strTemp = _T("000");
				else
					strTemp += _T("00");

				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);

				// 13. Surcharge
				nSurchargeAmt = CUtil::StringToInt((CString)JnlInfo.tran_info.surc_amt);

 				if (nDispensedAmt > 0 && nSurchargeAmt > 0)	// 방출이 됐다면 무조건 수수료 부과 사양.
 				{
					strTemp.Format(_T("%d"), nSurchargeAmt);
  				}
				else
				{
					strTemp = _T("000");
				}

				strReturnValue += strTemp;
 				strReturnValue += CString(RS);
			}
			else if (strTransType == S_BALANCEINQUIRY)
			{
				// 조회시 수수료 처리 추가시 처리 추가 필요

				// 11. REQUESTED
				// 구분자
				strReturnValue += CString(RS);

				// 12. DISPENSED
				// 구분자
				strReturnValue += CString(RS);

				// 13. Surcharge
				// 구분자
				strReturnValue += CString(RS);
			}
			else if (strTransType == S_TRANSFER)
			{
				// 11. REQUESTED
				strTemp.Format(_T("%S"), JnlInfo.tran_info.req_amt);
				strReturnValue += strTemp;
				strReturnValue += CString(RS);

				// 12. DISPENSED
				// 구분자
				strReturnValue += CString(RS);

				// 13. Surcharge
				// 구분자
				strReturnValue += CString(RS);
			}
			else
			{
				// 11. REQUESTED
				// 구분자
				strReturnValue += CString(RS);

				// 12. DISPENSED
				// 구분자
				strReturnValue += CString(RS);

				// 13. Surcharge
				// 구분자
				strReturnValue += CString(RS);
			}

			// Balance는 Journal에는 무조건 표시 (추후 협의 필요)
//			if (nDispensedAmt >= nRequestedAmt)		// 조회 및 이체는 모두 0이므로 공통으로 처리됨
			{
				// 14. BALANCE
				strTemp.Format(_T("%S"), JnlInfo.tran_info.bal_amt);
				strTemp.TrimLeft();

				if (!strTemp.IsEmpty())
				{
					//strReturnValue += strTemp;
					strTemp.Format(_T("%03d"), CUtil::StringToInt((CString)JnlInfo.tran_info.bal_amt));
					strReturnValue += strTemp;
				}
				
				// 구분자
				strReturnValue += CString(RS);

				// 15. AVALIABLE BALANCE
				strTemp.Format(_T("%S"), JnlInfo.tran_info.avail_bal_amt);
				strTemp.TrimLeft();
				
				if (!strTemp.IsEmpty())
				{
					//strReturnValue += strTemp;
					strTemp.Format(_T("%03d"), CUtil::StringToInt((CString)JnlInfo.tran_info.avail_bal_amt));
					strReturnValue += strTemp;
				}

				// 구분자
				strReturnValue += CString(RS);
			}

			// 출금거래에 한해 DISPENSED / REJECTED 매수 저장
			if(strTransType == S_WITHDRAWAL)
			{
				// 16. DISPENSED COUNT
				strTemp.Format(_T("%S"), JnlInfo.tran_info.disp_cnt);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);

				// 17. REJECTED COUNT
				strTemp.Format(_T("%S"), JnlInfo.tran_info.reject_cnt);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
			}
			else
			{
				// 16. DISPENSED COUNT
				// 구분자
				strReturnValue += CString(RS);

				// 17. REJECTED COUNT
				// 구분자
				strReturnValue += CString(RS);
			}

			// 18. ERROR 1
			strTemp.Format(_T("%S"), JnlInfo.tran_info.err_code_1);
			strTemp.TrimRight();
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// 19. ERROR 2
			strTemp.Format(_T("%S"), JnlInfo.tran_info.err_code_2);
			strTemp.TrimRight();
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);
			
			// 20. ERROR 3 -> error code1 사용으로 변경
			//strTemp.Format(_T("%6.6S"), JnlInfo.tran_info.err_code_3);
			//strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// ETC - reserved - AMS Server와 협의하여 신규 Field를 추가함

			// 구분자
			strReturnValue += CString(RS);			

			// CBX1 reject 매수
			strTemp.Format(_T("%S"), JnlInfo.tran_info.cst1_reject_info);
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// CBX2 reject 매수
			strTemp.Format(_T("%S"), JnlInfo.tran_info.cst2_reject_info);
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// CBX3 reject 매수
			strTemp.Format(_T("%S"), JnlInfo.tran_info.cst3_reject_info);
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// CBX4 reject 매수
			strTemp.Format(_T("%S"), JnlInfo.tran_info.cst4_reject_info);
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// 총 잔여매수
			strTemp.Format(_T("%S"), JnlInfo.tran_info.cst_remain_cnt);
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// 21. EMV DATA
			strTemp = JnlInfo.tran_info.emv_data;

			// EMV Data인 경우에 대한 처리 추가
			strTemp.TrimRight();

			if (strTemp.GetLength() > 0)
			{
				if (strTemp.Left(1) == JNL_ERRMSG_FIELD_EMV_DATA)
				{

					CStringArray	strArrayEMVData;
					CString			strEMVData;

					strTemp = strTemp.Mid(1);		// EMV FIELD CMD 제거

					// EMV DATA FIELD 추가
					strReturnValue += JNL_ERRMSG_FIELD_EMV_DATA;
					strReturnValue += SCR_LINEFEED_DELEMITER;

					// Parsing FS
					CUtil::ParsingStringToStringArray(strTemp, (CString)FS, strArrayEMVData);

					for(i=0; i<strArrayEMVData.GetCount(); i++)
					{
						strEMVData = strArrayEMVData.GetAt(i);

						// Format - Tag / Length / Value
						if (strEMVData.GetLength() > 2)		// 최소 Tag가 존재하는지 Check
						{
							strTemp = strEMVData.Left(2);

							if (strTemp.Right(1) == _T("F"))	// Tag의 2번째가 "F"인 경우는 4BYTE Tag임
							{
								// 4BYTE TAG이므로 Tag Length 정보 제거 후 Value 값 얻어오기 (Tag : 4 / Length : 2)
								strTemp2.Format(_T("%s=%s"), strEMVData.Left(4), strEMVData.Right(strEMVData.GetLength()-6));
							}
							else
							{
								// 2BYTE TAG
								strTemp2.Format(_T("%s=%s"), strEMVData.Left(2), strEMVData.Right(strEMVData.GetLength()-4));
							}

							strReturnValue += strTemp2;
							strReturnValue += SCR_LINEFEED_DELEMITER;
						}
					}
				}
				else if (strTemp.Left(1) == JNL_ERRMSG_FIELD_FALLBACK)
				{
					// EMV DATA FIELD 추가
					strReturnValue += JNL_ERRMSG_FIELD_FALLBACK;
					strReturnValue += SCR_LINEFEED_DELEMITER;

					strReturnValue += _T("FALLBACK TRANSACTION");
					strReturnValue += SCR_LINEFEED_DELEMITER;
				}
			}

			// 구분자
			strReturnValue += CString(RS);


			// DCC Value - reserved
			strTemp.Format(_T("%S"), JnlInfo.tran_info.dcc_data);
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// Camera Image File Name
			strTemp.Format(_T("%S"), JnlInfo.tran_info.cam_img_filename);
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);
		}
		break;

	case add_bill:
		{
			// 1. Jnl no
			strTemp.Format(_T("%d"), JnlInfo.add_bill_info.jnl_no);
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// 2. Terminal ID
			strTemp.Format(_T("%S"), JnlInfo.add_bill_info.term_id);
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// 3. DATE -> MM/DD/YYYY를 YYYY-MM-DD로 변경
			strTemp2.Format(_T("%S"), JnlInfo.add_bill_info.start_date);
			strTemp2.Replace(_T("/"), _T(""));
			strTemp.Format(_T("%4.4s%2.2s%2.2s"), strTemp2.Right(4), strTemp2.Left(2), strTemp2.Mid(2,2));	// Format 변경 YYYYMMDD
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// 4. TIME
			strTemp.Format(_T("%S"), JnlInfo.add_bill_info.start_time);
			strTemp.Replace(_T(":"), _T(""));	// Format 변경 HHMMSS
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// Added CBXCount
			strTemp.Format(_T("%S"),  JnlInfo.add_bill_info.cst_count);

			int nJnlCBXCount = CUtil::StringToInt(strTemp);

			if (nJnlCBXCount > 0)
				nCSTCnt = nJnlCBXCount;

			int nTemp = 0;

			if (nCSTCnt >= 1)
			{
				// 5. 1CBX ADD COUNT
				nTemp = CUtil::StringToInt((CString)JnlInfo.add_bill_info.add_bill_cst1);
				strTemp.Format(_T("%d"), nTemp);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);

				// 6. 1CBX REMAIN COUNT
				nTemp = CUtil::StringToInt((CString)JnlInfo.add_bill_info.remain_cst1);
				strTemp.Format(_T("%d"), nTemp);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
			}

			if (nCSTCnt >= 2)
			{
				// 7. 2CBX ADD COUNT
				nTemp = CUtil::StringToInt((CString)JnlInfo.add_bill_info.add_bill_cst2);
				strTemp.Format(_T("%d"), nTemp);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
				
				// 8. 2CBX REMAIN COUNT
				nTemp = CUtil::StringToInt((CString)JnlInfo.add_bill_info.remain_cst2);
				strTemp.Format(_T("%d"), nTemp);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
			}

			if (nCSTCnt >= 3)
			{
				// 9. 3CBX ADD COUNT
				nTemp = CUtil::StringToInt((CString)JnlInfo.add_bill_info.add_bill_cst3);
				strTemp.Format(_T("%d"), nTemp);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
				
				// 10. 3CBX REMAIN COUNT
				nTemp = CUtil::StringToInt((CString)JnlInfo.add_bill_info.remain_cst3);
				strTemp.Format(_T("%d"), nTemp);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
			}

			if (nCSTCnt >= 4)
			{
				// 11. 4CBX ADD COUNT
				nTemp = CUtil::StringToInt((CString)JnlInfo.add_bill_info.add_bill_cst4);
				strTemp.Format(_T("%d"), nTemp);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
				
				// 12. 4CBX REMAIN COUNT
				nTemp = CUtil::StringToInt((CString)JnlInfo.add_bill_info.remain_cst4);
				strTemp.Format(_T("%d"), nTemp);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
			}

			// 카세트 수보다 큰 카세트 정보 field는 구분자(0x1E)만 채움.
			// ex) nCSTCnt = 3 => add_bill_cst4와 remain_cst4는 0x1E로 채움
			for(i=0; i<(MAX_CBX_COUNT_AMS - nCSTCnt); i++)
			{
				// 구분자
				strReturnValue += CString(RS);

				// 구분자
				strReturnValue += CString(RS);
			}
		}
		break;

	case cst_close:
		{
			// 1. Jnl no
			strTemp.Format(_T("%d"), JnlInfo.cst_close_info.jnl_no);
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// 2. Terminal ID
			strTemp.Format(_T("%S"), JnlInfo.cst_close_info.term_id);
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// 3. DATE
			strTemp2.Format(_T("%S"), JnlInfo.cst_close_info.start_date);
			strTemp2.Replace(_T("/"), _T(""));
			strTemp.Format(_T("%4.4s%2.2s%2.2s"), strTemp2.Right(4), strTemp2.Left(2), strTemp2.Mid(2,2));		// Format 변경 YYYYMMDD
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// 4. TIME
			strTemp.Format(_T("%S"), JnlInfo.cst_close_info.start_time);
			strTemp.Replace(_T(":"), _T(""));		// Format 변경 HHMMSS
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// 5. CT TOTAL TYPE
			strTemp.Format(_T("%S"), JnlInfo.cst_close_info.cst_close_type);
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// Added CBXCount
			strTemp.Format(_T("%S"),  JnlInfo.cst_close_info.cst_count);

			int nJnlCBXCount = CUtil::StringToInt(strTemp);

			if (nJnlCBXCount > 0)
				nCSTCnt = nJnlCBXCount;
			
			int nTemp = 0;
			if (nCSTCnt >= 1)
			{
				// 6. 1CBX LOAD BILL
				nTemp = CUtil::StringToInt((CString)JnlInfo.cst_close_info.load_bill_cst1);
				strTemp.Format(_T("%d"), nTemp);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
				
				// 7. 1CBX DISP BILL
				nTemp = CUtil::StringToInt((CString)JnlInfo.cst_close_info.disp_bill_cst1);
				strTemp.Format(_T("%d"), nTemp);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
				
				// 8. 1CBX REJ BILL
				nTemp = CUtil::StringToInt((CString)JnlInfo.cst_close_info.rej_bill_cst1);
				strTemp.Format(_T("%d"), nTemp);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
				
				// 9. 1CBX REMAIN BILL
				nTemp = CUtil::StringToInt((CString)JnlInfo.cst_close_info.rem_bill_cst1);
				strTemp.Format(_T("%d"), nTemp);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
				
				// 10. 1CBX LOAD AMOUNT
				nTemp = CUtil::StringToInt((CString)JnlInfo.cst_close_info.load_amt_cst1);
				if (nTemp > 0)
					strTemp.Format(_T("%d"), nTemp);
				else
					strTemp = _T("000");
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
				
				// 11. 1CBX DISP AMOUNT
				nTemp = CUtil::StringToInt((CString)JnlInfo.cst_close_info.disp_amt_cst1);
				if (nTemp > 0)
					strTemp.Format(_T("%d"), nTemp);
				else
					strTemp = _T("000");

				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
				
				// 12. 1CBX REMAIN AMOUNT
				nTemp = CUtil::StringToInt((CString)JnlInfo.cst_close_info.rem_amt_cst1);

				if (nTemp > 0)
					strTemp.Format(_T("%d"), nTemp);
				else
					strTemp = _T("000");
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
				
				// 13. 1CBX DENOM
				strTemp.Format(_T("%S00"), JnlInfo.cst_close_info.denom_cst1);	// Cent 포함해서 전송
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
			}

			if (nCSTCnt >= 2)
			{
				// 14. 2CBX LOAD BILL
				nTemp = CUtil::StringToInt((CString)JnlInfo.cst_close_info.load_bill_cst2);
				strTemp.Format(_T("%d"), nTemp);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
				
				// 15. 2CBX DISP BILL
				nTemp = CUtil::StringToInt((CString)JnlInfo.cst_close_info.disp_bill_cst2);
				strTemp.Format(_T("%d"), nTemp);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
				
				// 16. 2CBX REJ BILL
				nTemp = CUtil::StringToInt((CString)JnlInfo.cst_close_info.rej_bill_cst2);
				strTemp.Format(_T("%d"), nTemp);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
				
				// 17. 2CBX REMAIN BILL
				nTemp = CUtil::StringToInt((CString)JnlInfo.cst_close_info.rem_bill_cst2);
				strTemp.Format(_T("%d"), nTemp);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
				
				// 18. 2CBX LOAD AMOUNT
				nTemp = CUtil::StringToInt((CString)JnlInfo.cst_close_info.load_amt_cst2);
				if (nTemp > 0)
					strTemp.Format(_T("%d"), nTemp);
				else
					strTemp = _T("000");

				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
				
				// 19. 2CBX DISP AMOUNT
				nTemp = CUtil::StringToInt((CString)JnlInfo.cst_close_info.disp_amt_cst2);
				if (nTemp > 0)
					strTemp.Format(_T("%d"), nTemp);
				else
					strTemp = _T("000");

				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
				
				// 20. 2CBX REMAIN AMOUNT
				nTemp = CUtil::StringToInt((CString)JnlInfo.cst_close_info.rem_amt_cst2);
				if (nTemp > 0)
					strTemp.Format(_T("%d"), nTemp);
				else
					strTemp = _T("000");

				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
				
				// 21. 2CBX DENOM
				strTemp.Format(_T("%S00"), JnlInfo.cst_close_info.denom_cst2);	// cent 추가해서 송신
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
			}

			if (nCSTCnt >= 3)
			{
				// 22. 3CBX LOAD BILL
				nTemp = CUtil::StringToInt((CString)JnlInfo.cst_close_info.load_bill_cst3);
				strTemp.Format(_T("%d"), nTemp);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
				
				// 23. 3CBX DISP BILL
				nTemp = CUtil::StringToInt((CString)JnlInfo.cst_close_info.disp_bill_cst3);
				strTemp.Format(_T("%d"), nTemp);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
				
				// 24. 3CBX REJ BILL
				nTemp = CUtil::StringToInt((CString)JnlInfo.cst_close_info.rej_bill_cst3);
				strTemp.Format(_T("%d"), nTemp);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
				
				// 25. 3CBX REMAIN BILL
				nTemp = CUtil::StringToInt((CString)JnlInfo.cst_close_info.rem_bill_cst3);
				strTemp.Format(_T("%d"), nTemp);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
				
				// 26. 3CBX LOAD AMOUNT
				nTemp = CUtil::StringToInt((CString)JnlInfo.cst_close_info.load_amt_cst3);
				if (nTemp > 0)
					strTemp.Format(_T("%d"), nTemp);
				else
					strTemp = _T("000");

				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
				
				// 27. 3CBX DISP AMOUNT
				nTemp = CUtil::StringToInt((CString)JnlInfo.cst_close_info.disp_amt_cst3);
				if (nTemp > 0)
					strTemp.Format(_T("%d"), nTemp);
				else
					strTemp = _T("000");

				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
				
				// 28. 3CBX REMAIN AMOUNT
				nTemp = CUtil::StringToInt((CString)JnlInfo.cst_close_info.rem_amt_cst3);
				if (nTemp > 0)
					strTemp.Format(_T("%d"), nTemp);
				else
					strTemp = _T("000");

				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
				
				// 29. 3CBX DENOM
				strTemp.Format(_T("%S00"), JnlInfo.cst_close_info.denom_cst3);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
			}

			if (nCSTCnt >= 4)
			{
				// 30. 4CBX LOAD BILL
				nTemp = CUtil::StringToInt((CString)JnlInfo.cst_close_info.load_bill_cst4);
				strTemp.Format(_T("%d"), nTemp);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
				
				// 31. 4CBX DISP BILL
				nTemp = CUtil::StringToInt((CString)JnlInfo.cst_close_info.disp_bill_cst4);
				strTemp.Format(_T("%d"), nTemp);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
				
				// 32. 4CBX REJ BILL
				nTemp = CUtil::StringToInt((CString)JnlInfo.cst_close_info.rej_bill_cst4);
				strTemp.Format(_T("%d"), nTemp);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
				
				// 33. 4CBX REMAIN BILL
				nTemp = CUtil::StringToInt((CString)JnlInfo.cst_close_info.rem_bill_cst4);
				strTemp.Format(_T("%d"), nTemp);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
				
				// 34. 4CBX LOAD AMOUNT
				nTemp = CUtil::StringToInt((CString)JnlInfo.cst_close_info.load_amt_cst4);
				if (nTemp > 0)
					strTemp.Format(_T("%d"), nTemp);
				else
					strTemp = _T("000");

				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
				
				// 35. 4CBX DISP AMOUNT
				nTemp = CUtil::StringToInt((CString)JnlInfo.cst_close_info.disp_amt_cst4);
				if (nTemp > 0)
					strTemp.Format(_T("%d"), nTemp);
				else
					strTemp = _T("000");

				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
				
				// 36. 4CBX REMAIN AMOUNT
				nTemp = CUtil::StringToInt((CString)JnlInfo.cst_close_info.rem_amt_cst4);
				if (nTemp > 0)
					strTemp.Format(_T("%d"), nTemp);
				else
					strTemp = _T("000");

				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
				
				// 37. 4CBX DENOM
				strTemp.Format(_T("%S00"), JnlInfo.cst_close_info.denom_cst4);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
			}

			// 카세트 수보다 큰 카세트 정보 field는 구분자(0x1E)로 채움
			// ex) nCSTCnt = 3 => 4CBX 정보는 0x1C로 채움
			for(i=0; i<(MAX_CBX_COUNT_AMS - nCSTCnt); i++)
			{
				// 구분자
				strReturnValue += CString(RS);
				// 구분자
				strReturnValue += CString(RS);
				// 구분자
				strReturnValue += CString(RS);
				// 구분자
				strReturnValue += CString(RS);
				// 구분자
				strReturnValue += CString(RS);
				// 구분자
				strReturnValue += CString(RS);
				// 구분자
				strReturnValue += CString(RS);
				// 구분자
				strReturnValue += CString(RS);
			}
		}
		break;

	case day_close:
		{
			// 1. Jnl no
			strTemp.Format(_T("%d"), JnlInfo.day_close_info.jnl_no);
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// 2. Terminal ID
			strTemp.Format(_T("%S"), JnlInfo.day_close_info.term_id);
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// 3. DATE
			strTemp2.Format(_T("%S"), JnlInfo.day_close_info.start_date);
			strTemp2.Replace(_T("/"), _T(""));
			strTemp.Format(_T("%4.4s%2.2s%2.2s"), strTemp2.Right(4), strTemp2.Left(2), strTemp2.Mid(2,2));
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// 4. TIME
			strTemp.Format(_T("%S"), JnlInfo.day_close_info.start_time);
			strTemp.Replace(_T(":"), _T(""));		// Format 변경 HHMMSS
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// 5. Day Close Type
			strTemp.Format(_T("%S"), JnlInfo.day_close_info.day_close_type);
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);
			
			// 6. Day Close Result
			strTemp.Format(_T("%S"), JnlInfo.day_close_info.day_close_Result);
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			if (strTemp == _T("1"))	// Success
			{
				// 7. HOST WITH COUNT
				strTemp.Format(_T("%S"), JnlInfo.day_close_info.host_with_cnt);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);

				// 8. HOST BAL COUNT
				strTemp.Format(_T("%S"), JnlInfo.day_close_info.host_bal_cnt);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
				
				// 9. HOST TRANS COUNT
				strTemp.Format(_T("%S"), JnlInfo.day_close_info.host_trans_cnt);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
				
				// 10. TOTAL HOST AMOUNT
				int nTemp = CUtil::StringToInt((CString)JnlInfo.day_close_info.total_host_amt);
				if (nTemp > 0)
					strTemp.Format(_T("%d"), nTemp);
				else
					strTemp = _T("000");

				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
			}
			else
			{
				// 7. HOST WITH COUNT
				// 구분자
				strReturnValue += CString(RS);

				// 8. HOST BAL COUNT
				// 구분자
				strReturnValue += CString(RS);

				// 9. HOST TRANS COUNT
				// 구분자
				strReturnValue += CString(RS);

				// 10. TOTAL HOST AMOUNT
				// 구분자
				strReturnValue += CString(RS);
			}

			// 11. TERMINAL WITH COUNT
			strTemp.Format(_T("%S"), JnlInfo.day_close_info.term_with_cnt);
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);
			
			// 12. TERMINAL BAL COUNT
			strTemp.Format(_T("%S"), JnlInfo.day_close_info.term_bal_cnt);
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);
			
			// 13. TERMINAL TRANS COUNT
			strTemp.Format(_T("%S"), JnlInfo.day_close_info.term_trans_cnt);
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);
			
			// 14. TOTAL TERMINAL AMOUNT
			int nTemp = CUtil::StringToInt((CString)JnlInfo.day_close_info.total_term_amt);
			if (nTemp > 0)
				strTemp.Format(_T("%d"), nTemp);
			else
				strTemp = _T("000");

			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);
		}
		break;

	case denom_info:
		{
			// 1. Jnl no
			strTemp.Format(_T("%d"), JnlInfo.denomination_info.jnl_no);
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// 2. Terminal ID
			strTemp.Format(_T("%S"), JnlInfo.denomination_info.term_id);
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// 3. DATE
			strTemp2.Format(_T("%S"), JnlInfo.denomination_info.start_date);
			strTemp2.Replace(_T("/"), _T(""));
			strTemp.Format(_T("%4.4s%2.2s%2.2s"), strTemp2.Right(4), strTemp2.Left(2), strTemp2.Mid(2,2));
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// 4. TIME
			strTemp.Format(_T("%S"), JnlInfo.denomination_info.start_time);
			strTemp.Replace(_T(":"), _T(""));		// Format 변경 HHMMSS
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// Added CBXCount
			strTemp.Format(_T("%S"),  JnlInfo.denomination_info.cst_count);

			int nJnlCBXCount = CUtil::StringToInt(strTemp);

			if (nJnlCBXCount > 0)
				nCSTCnt = nJnlCBXCount;

			int nOldDenom = 0, nNewDenom = 0;

			if (nCSTCnt >= 1)
			{
				// 5. 1CBX OLD DENOM
				strTemp.Format(_T("%S00"), JnlInfo.denomination_info.olddenom_cst1);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);

				// 6. 1CBX NEW DENOM
				strTemp.Format(_T("%S00"), JnlInfo.denomination_info.newdenom_cst1);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
			}

			if (nCSTCnt >= 2)
			{
				// 7. 2CBX OLD DENOM
				strTemp.Format(_T("%S00"), JnlInfo.denomination_info.olddenom_cst2);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);

				// 8. 2CBX NEW DENOM
				strTemp.Format(_T("%S00"), JnlInfo.denomination_info.newdenom_cst2);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
			}

			if (nCSTCnt >= 3)
			{
				// 9. 3CBX OLD DENOM
				strTemp.Format(_T("%S00"), JnlInfo.denomination_info.olddenom_cst3);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);

				// 10. 3CBX NEW DENOM
				strTemp.Format(_T("%S00"), JnlInfo.denomination_info.newdenom_cst3);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
			}

			if (nCSTCnt >= 4)
			{
				// 11. 4CBX OLD DENOM
				strTemp.Format(_T("%S00"), JnlInfo.denomination_info.olddenom_cst4);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);

				// 12. 4CBX NEW DENOM
				strTemp.Format(_T("%S00"), JnlInfo.denomination_info.newdenom_cst4);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
			}

			// 카세트 수보다 큰 카세트 정보 field는 구분자(0x1E)만 채움.
			// ex) nCSTCnt = 3 => 4CBX 정보는 0x1C로 채움
			for(i=0; i<(MAX_CBX_COUNT_AMS - nCSTCnt); i++)
			{
				// 구분자
				strReturnValue += CString(RS);
				// 구분자
				strReturnValue += CString(RS);
			}
		}
		break;

	case value_change:
		{
			// 1. Jnl no
			strTemp.Format(_T("%d"), JnlInfo.change_info.jnl_no);
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// 2. Terminal ID
			strTemp.Format(_T("%S"), JnlInfo.change_info.term_id);
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// 3. OLD VALUE
			strTemp.Format(_T("%S"), JnlInfo.change_info.old_value);
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// 4. NEW VALUE
			strTemp.Format(_T("%S"), JnlInfo.change_info.new_value);
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);
		}
		break;

	case text_field:
	case power_on:
	case in_service:
	case out_of_service:
	case in_supervisor:
	case reboot:
		{
			// 1. Jnl no
			strTemp.Format(_T("%d"), JnlInfo.text_info.jnl_no);
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// 2. Terminal ID
			strTemp.Format(_T("%S"), JnlInfo.text_info.term_id);
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// 3. TEXT VALUE
			strTemp.Format(_T("%S"), JnlInfo.text_info.text_value);
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);
		}
		break;

	case test_dispense:
		{
			// 1. Jnl no
			strTemp.Format(_T("%d"), JnlInfo.test_dispense_info.jnl_no);
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// 2. Terminal ID
			strTemp.Format(_T("%S"), JnlInfo.test_dispense_info.term_id);
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// 3. DATE
			strTemp2.Format(_T("%S"), JnlInfo.test_dispense_info.start_date);
			strTemp2.Replace(_T("/"), _T(""));
			strTemp.Format(_T("%4.4s%2.2s%2.2s"), strTemp2.Right(4), strTemp2.Left(2), strTemp2.Mid(2,2));
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// 4. TIME
			strTemp.Format(_T("%S"), JnlInfo.test_dispense_info.start_time);
			strTemp.Replace(_T(":"), _T(""));		// Format 변경 HHMMSS
			strReturnValue += strTemp;

			// 구분자
			strReturnValue += CString(RS);

			// 방출결과
			CString strTestDispResult;
			strTestDispResult.Format(_T("%S"), JnlInfo.test_dispense_info.test_disp_result);
			strReturnValue += strTestDispResult;

			// 구분자
			strReturnValue += CString(RS);

			// Added CBXCount
			strTemp.Format(_T("%S"),  JnlInfo.test_dispense_info.cst_count);

			int nJnlCBXCount = CUtil::StringToInt(strTemp);

			if (nJnlCBXCount > 0)
				nCSTCnt = nJnlCBXCount;

			for(i=0; i<__min(nCSTCnt, MAX_CBX_COUNT_AMS); i++)
			{
				// CBX #n 요구매수
				strTemp.Format(_T("%S"), JnlInfo.test_dispense_info.test_req_bill_cst[i]);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);

				// CBX #n Pickup매수
				strTemp.Format(_T("%S"), JnlInfo.test_dispense_info.test_pickup_bill_cst[i]);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);

				// CBX #n Reject 매수
				strTemp.Format(_T("%S"), JnlInfo.test_dispense_info.test_rej_bill_cst[i]);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);

				// CBX #n Remain 매수
				strTemp.Format(_T("%S"), JnlInfo.test_dispense_info.rem_bill_cst[i]);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);

				// CBX #n Denomination
				strTemp.Format(_T("%S00"), JnlInfo.test_dispense_info.denom_cst[i]);
				strReturnValue += strTemp;

				// 구분자
				strReturnValue += CString(RS);
			}

			for(i=0; i<(MAX_CBX_COUNT_AMS - nCSTCnt); i++)
			{
				// 구분자
				strReturnValue += CString(RS);
				// 구분자
				strReturnValue += CString(RS);
				// 구분자
				strReturnValue += CString(RS);
				// 구분자
				strReturnValue += CString(RS);
				// 구분자
				strReturnValue += CString(RS);
			}

			// 장애코드
			if (strTestDispResult == _T("0"))
			{
				strTemp.Format(_T("%S"), JnlInfo.test_dispense_info.err_code);
				strReturnValue += strTemp;
			}
			strReturnValue += CString(RS);
		}
		break;

	default:
		break;
	}

	strSendData += strReturnValue;
}


/** ********************************************************************
* @brief AMS로 Setup 정보 전송
* @param CString strCommand		Command
* @param CString strSubCommand	Sub Command
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
BOOL CDEV_HOST::AMS_SendATMSetupRead(CString strCommand, CString strSubCommand)
{
	int nRet = 0;
	int nSendDataLen = 0;
	CString strTemp;
	int nSendLen = 0;

	memset(m_pAMSSendBuffer, 0, NET_SEND_BUFF_SIZE);

	nSendLen = AMS_MakeSetupReadMsg(strCommand, strSubCommand, NET_SEND_BUFF_SIZE, m_pAMSSendBuffer);

	LOG(Info, _T("AMS_SendATMSetupRead() Send SetupRead Msg To AMS.."));

	// +1 => 0x1C
	nRet = HOST_Sync_Send((BYTE*)m_pAMSSendBuffer, nSendLen + LEN_LENGTH + 1, _T(""), _T(""), _T(""), TRUE);

	if(AMS_OK != nRet)
	{
		LOG(Error, _T("AMS_SendATMSetupRead() Send Setup Read Failed."));
		return FALSE;
	}

	LOG(Info, _T("AMS_SendATMSetupRead() Send Setup Read SUCCESS"));

	return TRUE;
}


/** ********************************************************************
* @brief AMS로 전송할 Setup Data
* @param CString strCommand		Command
* @param CString strSubCommand	Sub Command
* @param int nLen				Max Length
* @param BYTE *pbyData			Setup Data
* @param int &nSendLen			송신 전문 길이
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
int CDEV_HOST::AMS_MakeSetupReadMsg(CString strCommand, CString strSubCommand, int nLen, BYTE *pbyData)
{
	CString strSendData;
	int nSendLen = 0;


	AMS_MakeHeaderField(strCommand, strSubCommand, strSendData);

	if(AMS_SUB_SETUP_READ_ALL == strSubCommand)
	{
		LOG(Info, _T("SETUP READ RECV SubCMD : AMS_SUB_SETUP_READ_ALL"));

		AMS_MakeSetupSystemMsg(strSendData);	// Add Section
		AMS_MakeSetupHostMsg(strSendData);
		AMS_MakeSetupAMSMsg(strSendData);
		AMS_MakeSetupBINListMsg(strSendData);
		AMS_MakeSetupMessageMsg(strSendData);
		AMS_MakeSetupOptionMsg(strSendData);
		AMS_MakeSetupAdvertisementMsg(strSendData);
		AMS_MakeSetupEMVMsg(strSendData);
	}
	else
	{
		LOG(Info, _T("AMS_MakeSetupReadMsg() SubCommand Undefined[%s]"), strSubCommand);
		return FALSE;
	}

	strSendData += CString(FS);

	// Length
	nSendLen = strSendData.GetLength();
	strSendData.Insert(0, CUtil::IntToStringEx(nSendLen, LEN_LENGTH));
	strSendData.Insert(0, CString(FS));

	WideToMulti(strSendData, (char*)pbyData, strSendData.GetLength());

	return nSendLen;
}


/** ********************************************************************
* @brief AMS로 전송할 Host Setup Data
* @param CString &strSendData		송신 전문
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
BOOL CDEV_HOST::AMS_MakeSetupSystemMsg(CString &strSendData)
{
	CString strSystemData;
	CString strTemp;
	CString str_IP, str_Subnet, str_Gateway, str_DNS;
	CStringArray strArrSystemData;

	BYTE byRecvBuff[MAX_DATA_ARRAY_SIZE] = {0,};
	BOOL bResult = FALSE;

	//>>>>>>> 송신 전문 구성
	// 1. System Field Item Count - System Field 개수이기 때문에 가장 마지막에 설정한다.

	// 2. master password
	strTemp = CEagleDataManager::GetInstance()->m_Config.LoadSystemData(_T("PASSWORD"), _T("master_pwd"));
	strSystemData += strTemp;
	// 구분자
	strSystemData += CString(RS);

	// 3. administrator password (암호화된 값을 송신해야 하므로 File에서 직접 Read하도록 수정)
	strTemp = CEagleDataManager::GetInstance()->m_Config.LoadSystemData(_T("PASSWORD"), _T("admin_pwd"));
	strSystemData += strTemp;
	// 구분자
	strSystemData += CString(RS);

	// 4. replenishment password
	strTemp = CEagleDataManager::GetInstance()->m_Config.LoadSystemData(_T("PASSWORD"), _T("replenish_pwd"));
	strSystemData += strTemp;
	// 구분자
	strSystemData += CString(RS);

	// 5. Card Reader Latch
	bResult = CDEV_Manager::GetInstance()->m_DEV_CDR.ReadLatch(byRecvBuff);
	if(FALSE == bResult)
	{
		// Default값(Enable)으로 송신
		strTemp = S_ENABLE;
	}
	else
	{
		if(0x30 == byRecvBuff[0])
			strTemp = S_DISABLE;
		else
			strTemp = S_ENABLE;
	}
	strSystemData += strTemp;
	// 구분자
	strSystemData += CString(RS);
	
	// 6. Sound Volume
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Option.strSound_Volume;
	strSystemData += strTemp;
	// 구분자
	strSystemData += CString(RS);

	//7. Reboot Cycle	(Daily)
	strTemp = _T("0");
	strSystemData += strTemp;
	// 구분자
	strSystemData += CString(RS);

	//8. Reboot Time
	strTemp.Format(_T("%2.2s%2.2s"), CEagleDataManager::GetInstance()->m_Config.m_Option.strRebootTime.Left(2), CEagleDataManager::GetInstance()->m_Config.m_Option.strRebootTime.Right(2));
	strSystemData += strTemp;
	// 구분자
	strSystemData += CString(RS);

	//9. Network Type
	if (CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType == _T("TCP/IP"))
		strTemp = _T("0");
	else
		strTemp = _T("1");

	strSystemData += strTemp;
	// 구분자
	strSystemData += CString(RS);

	// 10. DHCP Use
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strUseDHCP;
	strSystemData += strTemp;
	// 구분자
	strSystemData += CString(RS);

	if (CEagleDataManager::GetInstance()->m_Config.m_Host.strUseDHCP == S_ENABLE)
	{
		CUtil::GetDHCP_IP(str_IP);

		CUtil::GetDHCP_SubnetMask(str_Subnet);

		CUtil::GetDHCP_GateWay(str_Gateway);

		CUtil::GetDHCP_DNS(str_DNS);
	}
	else
	{
		str_IP = CEagleDataManager::GetInstance()->m_Config.m_Host.strTerminalip;

		str_Subnet = CEagleDataManager::GetInstance()->m_Config.m_Host.strSubnet; 

		str_Gateway = CEagleDataManager::GetInstance()->m_Config.m_Host.strGateway; 

		str_DNS = CEagleDataManager::GetInstance()->m_Config.m_Host.strDNS;
	}

	// 11. Terminal IP
	strTemp = str_IP;
	strSystemData += strTemp;
	// 구분자
	strSystemData += CString(RS);

	// 12. Subnet
	strTemp = str_Subnet;
	strSystemData += strTemp;
	// 구분자
	strSystemData += CString(RS);

	// 13. GateWay
	strTemp = str_Gateway;
	strSystemData += strTemp;
	// 구분자
	strSystemData += CString(RS);

	// 14. DNS
	strTemp = str_DNS;
	strSystemData += strTemp;
	// 구분자
	strSystemData += CString(RS);

	// 15. Setup String
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strSetup_String;
	strSystemData += strTemp;
	// 구분자
	strSystemData += CString(RS);

	// 16. Camera 사용 유무 정보 추가 - 없을 경우에 대해서는 Null로 송신
	strTemp.Empty();
	if (CDEV_Manager::GetInstance()->m_DEV_CAMERA.IsExistCameraDriver() == TRUE)
	{
		if (CDEV_Manager::GetInstance()->m_DEV_CAMERA.GetCameraSetting() == TRUE)
			strTemp = S_ENABLE;
		else
			strTemp = S_DISABLE;
	}
	strSystemData += strTemp;
	strSystemData += CString(RS);

	/////////////////////////////////////////////////////

	strArrSystemData.RemoveAll();
	CUtil::ParsingStringToStringArray(strSystemData, (CString)RS, strArrSystemData);

	// 1. Host Field Item Count & 구분자
	strTemp.Format(_T("%d"), strArrSystemData.GetCount());

	strSystemData.Insert(0, CString(RS));
	strSystemData.Insert(0, strTemp);

	strSendData += strSystemData;

	return TRUE;
}


/** ********************************************************************
* @brief AMS로 전송할 Host Setup Data
* @param CString &strSendData		송신 전문
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
BOOL CDEV_HOST::AMS_MakeSetupHostMsg(CString &strSendData)
{
	CString strHostData;
	CString strTemp;
	CString str_IP, str_Subnet, str_Gateway, str_DNS;
	CStringArray strArrHostData;

	//>>>>>>> 송신 전문 구성
	// 1. Host Field Item Count - Host Field 개수이기 때문에 가장 마지막에 설정한다.

	// 2. Communication Header
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strCommunication_header;
	strHostData += strTemp;
	// 구분자
	strHostData += CString(RS);

	// 3. Communication ID
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strCommunicationID;
	strHostData += strTemp;
	// 구분자
	strHostData += CString(RS);

	// 4. ATM Status monitoring
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strAtm_status_monitoring;
	strHostData += strTemp;
	// 구분자
	strHostData += CString(RS);

	// 5. Heartbeat message
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strHeartbeat_message;
	strHostData += strTemp;
	// 구분자
	strHostData += CString(RS);

	// 6. Heartbeat frequency
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strHeartbeat_frequency;
	strHostData += strTemp;
	// 구분자
	strHostData += CString(RS);

	// 7. Primary Phone number
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strPrimary_phone_number;
	strHostData += strTemp;
	// 구분자
	strHostData += CString(RS);
	
	// 8. Backup Phone number
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strBackup_phone_number;
	strHostData += strTemp;
	// 구분자
	strHostData += CString(RS);
	
	// 9. PreDial
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strPre_dial;
	strHostData += strTemp;
	// 구분자
	strHostData += CString(RS);
	
	// 10. CRC Required
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strCrc_flg;
	strHostData += strTemp;
	// 구분자
	strHostData += CString(RS);

	// 11. Host IP
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strHostip;
	strHostData += strTemp;
	// 구분자
	strHostData += CString(RS);

	// 13. Port
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strPort;
	strHostData += strTemp;
	// 구분자
	strHostData += CString(RS);

	// 12. Backup Host IP
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strBackup_Hostip;
	strHostData += strTemp;
	// 구분자
	strHostData += CString(RS);

	// 13. Backup Port
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strBackupPort;
	strHostData += strTemp;
	// 구분자
	strHostData += CString(RS);
	
	// 14. Protocol Type
	if (CEagleDataManager::GetInstance()->m_Config.m_Host.strProtocolType == _T("VISA"))
		strTemp = _T("0");
	else
		strTemp = _T("1");

	// 15. Protocol Type
	strHostData += strTemp;
	// 구분자
	strHostData += CString(RS);

	// 16. ENQ Required
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strUseENQ;
	strHostData += strTemp;
	// 구분자
	strHostData += CString(RS);

	// 17. EOT Required
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strUseEOT;
	strHostData += strTemp;
	// 구분자
	strHostData += CString(RS);

	// 18. TLS Required
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strUseTLS;
	strHostData += strTemp;
	// 구분자
	strHostData += CString(RS);
	/////////////////////////////////////////////////////

	strArrHostData.RemoveAll();
	CUtil::ParsingStringToStringArray(strHostData, (CString)RS, strArrHostData);

	// 1. Host Field Item Count & 구분자
	strTemp.Format(_T("%d"), strArrHostData.GetCount());

	strHostData.Insert(0, CString(RS));
	strHostData.Insert(0, strTemp);

	strSendData += strHostData;

	return TRUE;
}


/** ********************************************************************
* @brief AMS로 전송할 Host Setup Data
* @param CString &strSendData		송신 전문
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
BOOL CDEV_HOST::AMS_MakeSetupAMSMsg(CString &strSendData)
{
	CString strAMSData;
	CString strTemp;
	CStringArray strArrAMSData;

	//>>>>>>> 송신 전문 구성
	// 1. AMS Field Item Count - AMS Field 개수이기 때문에 가장 마지막에 설정한다.

	// 2. AMS Enable/Disable
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_en_flg;
	strAMSData += strTemp;
	// 구분자
	strAMSData += CString(RS);

	// 3. AMS Status Send
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_Status_en_flg;
	strAMSData += strTemp;
	// 구분자
	strAMSData += CString(RS);

	// 4. AMS Primary Number
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_primary_num;
	strAMSData += strTemp;
	// 구분자
	strAMSData += CString(RS);

	// 5. AMS IP Address
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_IP_Address;
	strAMSData += strTemp;
	// 구분자
	strAMSData += CString(RS);

	// 6. AMS Port
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_Port;
	strAMSData += strTemp;
	// 구분자
	strAMSData += CString(RS);

	// 7. Schedule journal
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strSchedule_journal_flg;
	strAMSData += strTemp;
	// 구분자
	strAMSData += CString(RS);

	// 8. Schedule journal count
	strTemp.Format(_T("%d"), CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Host.strSchedule_journal_count));	// Option변경 hhmm->count
	strAMSData += strTemp;
	// 구분자
	strAMSData += CString(RS);

	/////////////////////////////////////////////////////

	strArrAMSData.RemoveAll();
	CUtil::ParsingStringToStringArray(strAMSData, (CString)RS, strArrAMSData);

	// 1. AMS Field Item Count & 구분자
	strTemp.Format(_T("%d"), strArrAMSData.GetCount());

	strAMSData.Insert(0, CString(RS));
	strAMSData.Insert(0, strTemp);

	strSendData += strAMSData;

	return TRUE;
}


/** ********************************************************************
* @brief AMS로 전송할 ISO Setup Data
* @param CString &strSendData		송신 전문
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
BOOL CDEV_HOST::AMS_MakeSetupBINListMsg(CString &strSendData)
{
	CString strIsoData;
	CString strTemp;
	CStringArray strArrIsoData;

	//>>>>>>> 송신 전문 구성
	// 1. BIN List Field Item Count - Bin List Field 개수이기 때문에 가장 마지막에 설정한다.

	// 2. BIN List count
	strTemp.Format(_T("%d"), MAX_BIN_COUNT);
	strIsoData += strTemp;
	// 구분자
	strIsoData += CString(RS);

	// BIN List (1 ~ 100)
	for(int i=0; i<MAX_BIN_COUNT; i++)
	{
		strTemp = CEagleDataManager::GetInstance()->m_Config.m_Iso.strBLOCK_ISO[i];
		strTemp.TrimRight();		// 공백 제거
		strIsoData += strTemp;
		// 구분자
		strIsoData += CString(RS);
	}
	///////////////////////////////////////////////////////////////

	strArrIsoData.RemoveAll();
	CUtil::ParsingStringToStringArray(strIsoData, (CString)RS, strArrIsoData);

	// 1. ISO Field Item Count & 구분자
	strTemp.Format(_T("%d"), strArrIsoData.GetCount());

	strIsoData.Insert(0, CString(RS));
	strIsoData.Insert(0, strTemp);

	strSendData += strIsoData;

	return TRUE;
}


/** ********************************************************************
* @brief AMS로 전송할 Message Setup Data
* @param CString &strSendData		송신 전문
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
BOOL CDEV_HOST::AMS_MakeSetupMessageMsg(CString &strSendData)
{
	int i=0;
	CString strMsgData;
	CString strTemp;
	CStringArray strArrMsgData;

	//>>>>>>> 송신 전문 구성
	// 1. Message Field Item Count - Message Field 개수이기 때문에 가장 마지막에 설정한다.

	// 2. marketing message 1~4
	for(i=0; i<4; i++)
	{
		strTemp = CEagleDataManager::GetInstance()->m_Config.m_Message.strMarketing_message[i];
		strTemp.TrimRight();
		strMsgData += strTemp;
		// 구분자
		strMsgData += CString(RS);
	}

	// 3. location message 1~4
	for(i=0; i<4; i++)
	{
		strTemp = CEagleDataManager::GetInstance()->m_Config.m_Message.strLocation_message[i];
		strTemp.TrimRight();
		strMsgData += strTemp;
		// 구분자
		strMsgData += CString(RS);
	}

	// 4. attraction message 1~4
	for(i=0; i<3; i++)
	{
		strTemp = CEagleDataManager::GetInstance()->m_Config.m_Message.strAttraction_message[i];
		strTemp.TrimRight();
		strMsgData += strTemp;
		// 구분자
		strMsgData += CString(RS);
	}

	// 5. farewell message 1~4
	for(i=0; i<3; i++)
	{
		strTemp = CEagleDataManager::GetInstance()->m_Config.m_Message.strFarewell_message[i];
		strTemp.TrimRight();
		strMsgData += strTemp;
		// 구분자
		strMsgData += CString(RS);
	}
	///////////////////////////////////////////////////////////////

	strArrMsgData.RemoveAll();
	CUtil::ParsingStringToStringArray(strMsgData, (CString)RS, strArrMsgData);

	// 1. Message Field Item Count & 구분자
	strTemp.Format(_T("%d"), strArrMsgData.GetCount());

	strMsgData.Insert(0, CString(RS));
	strMsgData.Insert(0, strTemp);

	strSendData += strMsgData;

	return TRUE;
}


/** ********************************************************************
* @brief AMS로 전송할 Option Setup Data
* @param CString &strSendData		송신 전문
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
BOOL CDEV_HOST::AMS_MakeSetupOptionMsg(CString &strSendData)
{
	CString strOptionData;
	CString strTemp;
	CStringArray strArrOptionData;

	//>>>>>>> 송신 전문 구성
	// 1. Option Field Item Count - OptionHost Field 개수이기 때문에 가장 마지막에 설정한다.

	// 5. Saving account
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Option.strSaving_account;
	strOptionData += strTemp;
	// 구분자
	strOptionData += CString(RS);
	
	// 6. Credit account
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Option.strCredit_account;
	strOptionData += strTemp;
	// 구분자
	strOptionData += CString(RS);
	
	// 7. Transfer transaction
	//strTemp.Format(_T("%C"), option.byTransferAccount);
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Option.strtransfer_Enable;
	strOptionData += strTemp;
	// 구분자
	strOptionData += CString(RS);
	
	// 8. Balance transaction
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Option.strbalance_Enable;
	strOptionData += strTemp;
	// 구분자
	strOptionData += CString(RS);
	
	// 9 ~ 14. Fast amount
	for(int i=0; i<6; i++)
	{
		strTemp.Format(_T("%s%s"), CEagleDataManager::GetInstance()->m_Config.m_Option.strFast_amount[i], _T("00"));	// cent 포함
		strOptionData += strTemp;
		// 구분자
		strOptionData += CString(RS);
	}

	// 15. Max withdrawal amount
	strTemp.Format(_T("%s%s"), CEagleDataManager::GetInstance()->m_Config.m_Option.strMax_withdrawal_amount, _T("00"));	// cent 포함
	strOptionData += strTemp;
	// 구분자
	strOptionData += CString(RS);

	// 16. Surcharge Type
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Option.strSurcharge_type;
	strOptionData += strTemp;
	// 구분자
	strOptionData += CString(RS);
	
	// 17. Withdrawal surcharge amount
	strTemp.Format(_T("%03d"), CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Option.strWithdrawalSurcharge_amount));
	strOptionData += strTemp;
	// 구분자
	strOptionData += CString(RS);
	
	// 18. Surcharge percentage
	strTemp.Format(_T("%s"), CUtil::ConvertFromValueToAmountwithCent(CEagleDataManager::GetInstance()->m_Config.m_Option.strSurcharge_percentage));
	strOptionData += strTemp;
	// 구분자
	strOptionData += CString(RS);
	
	// 19. Percentage Surcharge option
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Option.strWithdrawal_PercentSurcharge_Option;
	strOptionData += strTemp;
	// 구분자
	strOptionData += CString(RS);

	// 20. Surcharge owner
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Option.strSurcharge_owner;
	strOptionData += strTemp;
	// 구분자
	strOptionData += CString(RS);
	
	// 21. English
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Option.strLansupport[0];
	strOptionData += strTemp;
	// 구분자
	strOptionData += CString(RS);
	
	// 22. Spanish
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Option.strLansupport[1];
	strOptionData += strTemp;
	// 구분자
	strOptionData += CString(RS);
	
	// 23. French
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Option.strLansupport[2];
	strOptionData += strTemp;
	// 구분자
	strOptionData += CString(RS);
	
	// 24. Schedule day close
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Option.strScheduleDayClose;
	strOptionData += strTemp;
	// 구분자
	strOptionData += CString(RS);
	
	// 25. Day close time
	strTemp.Format(_T("%s%s"), CEagleDataManager::GetInstance()->m_Config.m_Option.strDayCloseTime.Left(2), CEagleDataManager::GetInstance()->m_Config.m_Option.strDayCloseTime.Right(2));	// Format 변경 HHMM
	strOptionData += strTemp;
	// 구분자
	strOptionData += CString(RS);
	
	////////////////////////////////////////////////////////////

	strArrOptionData.RemoveAll();
	CUtil::ParsingStringToStringArray(strOptionData, (CString)RS, strArrOptionData);

	// 1. Option Field Item Count & 구분자
	strTemp.Format(_T("%d"), strArrOptionData.GetCount());

	strOptionData.Insert(0, CString(RS));
	strOptionData.Insert(0, strTemp);

	strSendData += strOptionData;

	return TRUE;
}


/** ********************************************************************
* @brief AMS로 전송할 Advertisement Setup Data
* @param AMS_SETUP_ADVERTISEMENT &adv		Advertisement 구조체
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
BOOL CDEV_HOST::AMS_MakeSetupAdvertisementMsg(CString &strSendData)
{
	CString strAdvData;
	CString strTemp;
	CString strADTitleFileName, strADTitle, strADUniqueKey;
	CStringArray strArrAdvData;

	//>>>>>>> 송신 전문 구성
	// 1. Adv Field Item Count - Adv Field 개수이기 때문에 가장 마지막에 설정한다.

	// 2. advertisement interval time
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_ADS.strAdver_Interval_time;
	strAdvData += strTemp;
	// 구분자
	strAdvData += CString(RS);
	
	// 3 ~ 8. advertisement screen
	for(int i=0; i<SUPPORT_ADVERTISE; i++)
	{
		strTemp = CEagleDataManager::GetInstance()->m_Config.m_ADS.strAdver_Screen_en[i];
		strAdvData += strTemp;
		// 구분자
		strAdvData += CString(RS);
	}

	// Current AD Title / UniqueKey Field 추가
	// AD File Read
	strADTitleFileName.Format(_T("%s%s\\%d_%d%s"), CUtil::GetAppPath(), EAGLE_ATM_AD_PATH, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) ,AD_TITLE_FILE);

	CIniFile iniFile(strADTitleFileName);

	strADTitle = iniFile.ReadString(_T("AD_INFO"), _T("AD_TITLE"), _T(""));
	strAdvData += strADTitle;
	strAdvData += CString(RS);

	// AMS로부터 Adver Download시에만 존재하는 값
	strADUniqueKey = iniFile.ReadString(_T("AD_INFO"), _T("AD_UNIQUE_KEY"), _T(""));
	strAdvData += strADUniqueKey;
	strAdvData += CString(RS);
	////////////////////////////////////////////

	strArrAdvData.RemoveAll();
	CUtil::ParsingStringToStringArray(strAdvData, (CString)RS, strArrAdvData);

	// 1. Adv Field Item Count & 구분자
	strTemp.Format(_T("%d"), strArrAdvData.GetCount());

	strAdvData.Insert(0, CString(RS));
	strAdvData.Insert(0, strTemp);

	strSendData += strAdvData;

	return TRUE;
}


/** ********************************************************************
* @brief AMS로 전송할 emv Setup Data
* @param AMS_SETUP_EMV &emv		emv 구조체
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
BOOL CDEV_HOST::AMS_MakeSetupEMVMsg(CString &strSendData)
{
	CString strEmvData;
	CString strTemp;
	CStringArray strArrEmvData;
	int nAidCount = CDEV_Manager::GetInstance()->m_DEV_CDR.m_sEMV_Config_Data.nTotalAidListCount;

	//>>>>>>> 송신 전문 구성
	// 1. EMV Field Item Count - EMV Field 개수이기 때문에 가장 마지막에 설정한다.

	// 2. AID Count
	strTemp.Format(_T("%d"), nAidCount);
	strEmvData += strTemp;
	// 구분자
	strEmvData += CString(RS);
	
	// 3 ~ 62. aid info
	for(int i=0; i<MAX_AID_COUNT; i++)
	{
		// 1. AID enable
		strTemp.Format(_T("%d"), CDEV_Manager::GetInstance()->m_DEV_CDR.m_sEMV_Config_Data.m_sEMV_Config[i].AidUse);
		strEmvData += strTemp;
		// 구분자
		strEmvData += CString(US);
		
		// 2. AID Name
		strTemp = CDEV_Manager::GetInstance()->m_DEV_CDR.m_sEMV_Config_Data.m_sEMV_Config[i].AidName;
		strEmvData += strTemp;
		// 구분자
		strEmvData += CString(US);

		// 3. AID
		strTemp = CUtil::ConvertHexToString(CDEV_Manager::GetInstance()->m_DEV_CDR.m_sEMV_Config_Data.m_sEMV_Config[i].szAid, CDEV_Manager::GetInstance()->m_DEV_CDR.m_sEMV_Config_Data.m_sEMV_Config[i].AidLen);
		strEmvData += strTemp;
		// 구분자
		strEmvData += CString(RS);
	}
	///////////////////////////////////

	strArrEmvData.RemoveAll();
	CUtil::ParsingStringToStringArray(strEmvData, (CString)RS, strArrEmvData);

	// 1. EMV Field Item Count & 구분자
	strTemp.Format(_T("%d"), strArrEmvData.GetCount());

	strEmvData.Insert(0, CString(RS));
	strEmvData.Insert(0, strTemp);

	strSendData += strEmvData;

	return TRUE;
}


/** ********************************************************************
* @brief AMS로 전송할 Setup 정보
* @param CString strCommand		Command
* @param CString strSubCommand		SubCommand
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
BOOL CDEV_HOST::AMS_SendATMSetupWrite(CString strCommand, CString strSubCommand)
{
	int nRet = 0;
	CString strSendData;
	int nSendLen = 0;

	memset(m_pAMSSendBuffer, 0, NET_SEND_BUFF_SIZE);

	nSendLen = AMS_MakeSetupWriteMsg(strCommand, strSubCommand, NET_SEND_BUFF_SIZE, m_pAMSSendBuffer);

	LOG(Info, _T("AMS_SendATMSetupWrite() Send SetupRead Msg To AMS.."));

	// +1 => 0x1C
	nRet = HOST_Sync_Send((BYTE*)m_pAMSSendBuffer, nSendLen + LEN_LENGTH + 1, _T(""), _T(""), _T(""), TRUE);

	if(AMS_OK != nRet)
	{
		LOG(Error, _T("AMS_SendATMSetupWrite() Send Setup Write Failed."));
		return FALSE;
	}
	else
		LOG(Info, _T("AMS_SendATMSetupWrite() Send Setup Write SUCCESS"));

	return TRUE;
}


/** ********************************************************************
* @brief AMS로 전송할 Status Data
* @param CString strCommand		Command
* @param CString strSubCommand	Sub Command
* @param int nLen				Max Length
* @param BYTE *pbyData			StatusData
* @param int &nSendLen			송신 전문 길이
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
int CDEV_HOST::AMS_MakeSetupWriteMsg(CString strCommand, CString strSubCommand, int nLen, BYTE *pbyData)
{
	CString strSendData;
	int nSendLen = 0;

	// check buffer size.
	if (sizeof(AMS_RES_SETUP_WRITE) > nLen)
	{
		LOG(Error, _T("AMS_MakeSetupWriteMsg() Verify Data : SIZE(AMS_RES_SETUP_WRITE) is invalid."));
		return FALSE;
	}

	AMS_MakeHeaderField(strCommand, strSubCommand, strSendData);
	
	strSendData += CString(FS);	// Setup Write 명령에서는 Configuration Data 값이 없음. 따라서 구분자만 넣어줌.

	// Length
	nSendLen = strSendData.GetLength();
	strSendData.Insert(0, CUtil::IntToStringEx(nSendLen, LEN_LENGTH));
	strSendData.Insert(0, CString(FS));

	WideToMulti(strSendData, (char*)pbyData, strSendData.GetLength());
	
	return nSendLen;
}


/** ********************************************************************
* @brief AMS로부터 받은 setup 정보 세팅
* @param CStringArray strArrRecvData AMS로부터 받은 setup 정보
* @param int nRecvLen		AMS로부터 받은 setup 정보 길이
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
BOOL CDEV_HOST::AMS_ProcSetupWrite(BYTE *pbyData)
{
	CStringArray strArrSystemData;
	CStringArray strArrHostData;
	CStringArray strArrAMStData;
	CStringArray strArrISOData;
	CStringArray strArrMSGData;
	CStringArray strArrOptionData;
	CStringArray strArrADVData;
	CStringArray strArrEMVData;

	CStringArray strArrRecvData;
	CStringArray strArrSetupData;

	int i = 0;
	int nItemFieldCount = 0;
	int nIndex = 0;

	CString strTemp;
	CString strRecvData;

	strArrRecvData.RemoveAll();
	strRecvData = pbyData;
	CUtil::ParsingStringToStringArray(strRecvData, (CString)FS, strArrRecvData);

	strArrSetupData.RemoveAll();
	if(strArrRecvData.GetCount() > 5)
		CUtil::ParsingStringToStringArray(strArrRecvData.GetAt(5), (CString)RS, strArrSetupData);

	// 1. System parsing
	nItemFieldCount = 0;
	if(strArrSetupData.GetCount() > nIndex)
	{
		strTemp = strArrSetupData.GetAt(nIndex++);
		nItemFieldCount = CUtil::StringToInt(strTemp);
	}
	else
	{
		LOG(Error, _T("System field data count error nIndex : %d"), nIndex);
		return FALSE;
	}

	LOG(Info, _T("System field (%s), (%d)"), strTemp, nItemFieldCount);

	if (nItemFieldCount <=0 || nItemFieldCount > MAX_SETUP_FIELD_COUNT)
	{
		LOG(Info, _T("System Count error"));
		return FALSE;
	}

	strArrSystemData.RemoveAll();
	for(i=0; i<nItemFieldCount; i++)
	{
		if(strArrSetupData.GetCount() > nIndex)
			strArrSystemData.Add(strArrSetupData.GetAt(nIndex++));
	}

	// 2. host parsing
	nItemFieldCount = 0;
	if(strArrSetupData.GetCount() > nIndex)
	{
		strTemp = strArrSetupData.GetAt(nIndex++);
		nItemFieldCount = CUtil::StringToInt(strTemp);
	}

	LOG(Info, _T("Host field (%s), (%d)"), strTemp, nItemFieldCount);

	if (nItemFieldCount <=0 || nItemFieldCount > MAX_SETUP_FIELD_COUNT)
	{
		LOG(Info, _T("Host Count error"));
		return FALSE;
	}

	strArrHostData.RemoveAll();
	for(i=0; i<nItemFieldCount; i++)
	{
		if(strArrSetupData.GetCount() > nIndex)
			strArrHostData.Add(strArrSetupData.GetAt(nIndex++));
	}

	// 3. AMS parsing
	nItemFieldCount = 0;
	if(strArrSetupData.GetCount() > nIndex)
	{
		strTemp = strArrSetupData.GetAt(nIndex++);
		nItemFieldCount = CUtil::StringToInt(strTemp);
	}

	LOG(Info, _T("AMS field (%s), (%d)"), strTemp, nItemFieldCount);

	if (nItemFieldCount <=0 || nItemFieldCount > MAX_SETUP_FIELD_COUNT)
	{
		LOG(Info, _T("AMS Count error"));
		return FALSE;
	}

	strArrAMStData.RemoveAll();
	for(i=0; i<nItemFieldCount; i++)
	{
		if(strArrSetupData.GetCount() > nIndex)
			strArrAMStData.Add(strArrSetupData.GetAt(nIndex++));
	}

	//////////////////////////////////////////////
	
	// 4. BIN List parsing
	nItemFieldCount = 0;
	if(strArrSetupData.GetCount() > nIndex)
	{
		strTemp = strArrSetupData.GetAt(nIndex++);
	}

	LOG(Info, _T("BIN List Item Count (%s)"), strTemp);

	if(strArrSetupData.GetCount() > nIndex)
	{
		strTemp = strArrSetupData.GetAt(nIndex++);
		nItemFieldCount = CUtil::StringToInt(strTemp);
	}

	LOG(Info, _T("BIN List field (%s), (%d)"), strTemp, nItemFieldCount);

	if (nItemFieldCount <= 0 || nItemFieldCount > MAX_SETUP_FIELD_COUNT)
	{
		LOG(Info, _T("BIN List Count error"));
		return FALSE;
	}

	strArrISOData.RemoveAll();
	for(i=0; i<nItemFieldCount; i++)
	{
		if(strArrSetupData.GetCount() > nIndex)
			strArrISOData.Add(strArrSetupData.GetAt(nIndex++));
	}

	/////////////////////////////////////////

	// 5. Message parsing
	nItemFieldCount = 0;
	if(strArrSetupData.GetCount() > nIndex)
	{
		strTemp = strArrSetupData.GetAt(nIndex++); 
		nItemFieldCount = CUtil::StringToInt(strTemp);
	}

	LOG(Info, _T("Message field (%s), (%d)"), strTemp, nItemFieldCount);

	if (nItemFieldCount <=0 || nItemFieldCount > MAX_SETUP_FIELD_COUNT)
	{
		LOG(Info, _T("Message Count error"));
		return FALSE;
	}

	strArrMSGData.RemoveAll();
	for(i=0; i<nItemFieldCount; i++)
	{
		if(strArrSetupData.GetCount() > nIndex)
			strArrMSGData.Add(strArrSetupData.GetAt(nIndex++));
	}

	/////////////////////////////////////////

	// 6. Option parsing
	nItemFieldCount = 0;
	if(strArrSetupData.GetCount() > nIndex)
	{
		strTemp = strArrSetupData.GetAt(nIndex++);
		nItemFieldCount = CUtil::StringToInt(strTemp);
	}

	LOG(Info, _T("Option field (%s), (%d)"), strTemp, nItemFieldCount);

	if (nItemFieldCount <=0 || nItemFieldCount > MAX_SETUP_FIELD_COUNT)
	{
		LOG(Info, _T("Option Count error"));
		return FALSE;
	}

	strArrOptionData.RemoveAll();
	for(i=0; i<nItemFieldCount; i++)
	{
		if(strArrSetupData.GetCount() > nIndex)
			strArrOptionData.Add(strArrSetupData.GetAt(nIndex++));
	}

	/////////////////////////////////////////

	// 7. adv parsing
	if(strArrSetupData.GetCount() > nIndex)
	{
		strTemp = strArrSetupData.GetAt(nIndex++);
		nItemFieldCount = CUtil::StringToInt(strTemp);
	}

	LOG(Info, _T("ADV field (%s), (%d)"), strTemp, nItemFieldCount);

	if (nItemFieldCount <=0 || nItemFieldCount > MAX_SETUP_FIELD_COUNT)
	{
		LOG(Info, _T("ADV Count error"));
		return FALSE;
	}

	strArrADVData.RemoveAll();
	for(i=0; i<nItemFieldCount; i++)
	{
		if(strArrSetupData.GetCount() > nIndex)
			strArrADVData.Add(strArrSetupData.GetAt(nIndex++));
	}

	/////////////////////////////////////////

	// 6. emv parsing
	nItemFieldCount = 0;
	if(strArrSetupData.GetCount() > nIndex)
	{
		strTemp = strArrSetupData.GetAt(nIndex++);
		nItemFieldCount = CUtil::StringToInt(strTemp);
	}

	LOG(Info, _T("EMV field (%s), (%d)"), strTemp, nItemFieldCount);

	if (nItemFieldCount <=0 || nItemFieldCount > MAX_SETUP_FIELD_COUNT)
	{
		LOG(Info, _T("EMV Count error"));
		return FALSE;
	}

	strArrEMVData.RemoveAll();
	for(i=0; i<nItemFieldCount; i++)
	{
		if(strArrSetupData.GetCount() > nIndex)
			strArrEMVData.Add(strArrSetupData.GetAt(nIndex++));
	}

	/////////////////////////////////////////

	if(strArrRecvData.GetCount() > 2)
	{
		if(AMS_SUB_SETUP_WRITE_ALL == strArrRecvData.GetAt(2))	// sub command
		{
			// Supervisor 진입 전 Configure 정보 Save
			CDEV_Manager::GetInstance()->Save_PreviousTerminalInfo();

			AMS_WriteSetupSystemMsg(strArrSystemData);
			AMS_WriteSetupHostMsg(strArrHostData);
			AMS_WriteSetupAMSMsg(strArrAMStData);
 			AMS_WriteSetupISOMsg(strArrISOData);
 			AMS_WriteSetupMessageMsg(strArrMSGData);
 			AMS_WriteSetupOptionMsg(strArrOptionData);
 			AMS_WriteSetupAdvertisementMsg(strArrADVData);
 			AMS_WriteSetupEMVMsg(strArrEMVData);
		
			CEagleDataManager::GetInstance()->m_Config.Save();

			// Supervisor가 종료될 때 Configure 정보 변경시 저널 저장
			CDEV_Manager::GetInstance()->SaveEJL_SetChangedTerminalInfo(1);
		}
		else
		{
			LOG(Info, _T("AMS_ProcSetupWrite() SubCommand Undefined[%s]"), strArrRecvData.GetAt(2));
		}
	}

	return TRUE;
}


/** ********************************************************************
* @brief AMS로부터 받은 System setup 정보 세팅
* @param CString *pstrRecvData		AMS로부터 받은 정보
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
BOOL CDEV_HOST::AMS_WriteSetupSystemMsg(CStringArray &strArrRecvData)
{
	int nCnt = 0;
	int i=0;

	CString strTemp, strTemp2;
	CString strHour, strMinute;

	BOOL bResult = FALSE;
	BOOL bNumeric = TRUE;
	BOOL bInvalid = FALSE;

	BYTE byRecvBuff[MAX_DATA_ARRAY_SIZE] = { 0, };

	// 1. master password
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			if (CUtil::IsHexValue(strTemp) == TRUE)	// HEX만 가능
				CEagleDataManager::GetInstance()->m_Config.SavePasswordData(_T("master_pwd"), strTemp);
			else
				LOG(Error, _T("Skip - Master Password is invalid (%s)"), strTemp);
		}
		else
		{
			LOG(Error, _T("Receive Skip - Master Password"));
		}
	}

	// 2. administrator password (TDES이므로 File 저장 후 Load를 다시 해야함
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			if (CUtil::IsHexValue(strTemp) == TRUE)	// HEX만 가능
				CEagleDataManager::GetInstance()->m_Config.SavePasswordData(_T("admin_pwd"), strTemp);
			else
				LOG(Error, _T("Skip - Admin Password is invalid (%s)"), strTemp);
		}
		else
		{
			LOG(Error, _T("Receive Skip Value - Admin Password"));
		}
	}

	// 3. replenishment password
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			if (CUtil::IsHexValue(strTemp) == TRUE)	// HEX만 가능
				CEagleDataManager::GetInstance()->m_Config.SavePasswordData(_T("replenish_pwd"), strTemp);
			else
				LOG(Error, _T("Skip - Replenish Password is invalid (%s)"), strTemp);
		}
		else
		{
			LOG(Error, _T("Receive Skip Value - Replenish Password"));
		}
	}

	// System을 다시 Load해야함.
	CEagleDataManager::GetInstance()->m_Config.LoadSystem();

	// 5. Latch Option 설정 (다른 경우에만 설정하도록 함)
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			bResult = CDEV_Manager::GetInstance()->m_DEV_CDR.ReadLatch(byRecvBuff);

			if(TRUE == bResult)
			{
				if (strTemp == S_ENABLE)
				{
					if (byRecvBuff[0] == 0x30)	// Latch가 Off인 경우
					{
						CDEV_Manager::GetInstance()->m_DEV_CDR.SetLatch(TRUE);
					}
					else
					{
						LOG(Info, _T("Do not need to change the Latch Option.(ENABLE)"));
					}
				}
				else
				{
					if (byRecvBuff[0] != 0x30)	// Latch가 On인 경우
					{
						CDEV_Manager::GetInstance()->m_DEV_CDR.SetLatch(FALSE);
					}
					else
					{
						LOG(Info, _T("Do not need to change the Latch Option.(DISABLE)"));
					}
				}
			}
			else
			{
				if (strTemp == S_ENABLE)
				{
					CDEV_Manager::GetInstance()->m_DEV_CDR.SetLatch(TRUE);
				}
				else
				{
					CDEV_Manager::GetInstance()->m_DEV_CDR.SetLatch(FALSE);
				}
			}
		}
	}
	
	// 6. Sound volume
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			if (CUtil::IsNumeric(strTemp) == TRUE)	// 숫자만 가능
			{
				int nVolume = CUtil::StringToInt(strTemp);

				if (nVolume >= 0 && nVolume <= 10)			// 0~10만 Setting
					CEagleDataManager::GetInstance()->m_Config.m_Option.strSound_Volume.Format(_T("%d"), nVolume);
				else
					LOG(Error, _T("Skip - Sound Volume is range error (%d)"), nVolume);
			}
			else
			{
				LOG(Error, _T("Skip - Sound Volume is invalid (%s)"), strTemp);
			}
		}
	}

	//7. Reboot Cycle	(Daily) read only
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);
	}

	//8. Reboot Time - HHMM
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp2 = strArrRecvData.GetAt(nCnt++);

		if (strTemp2 != ESC)
		{
			strTemp2.Replace(_T(":"), _T(""));	// ":"제거

			if (CUtil::IsNumeric(strTemp2) == TRUE)	// 숫자만 가능
			{
				if (strTemp2.GetLength() == 4)
				{
					strHour = strTemp2.Left(2);
					strMinute = strTemp2.Right(2);
					int nHour = CUtil::StringToInt(strHour);
					int nMinute = CUtil::StringToInt(strMinute);

					if ( ((nHour >= 0) && (nHour <= 23)) && ((nMinute >= 0) && (nMinute <= 59)) )
					{
						strTemp.Format(_T("%2.2s%2.2s"), strTemp2.Left(2), strTemp2.Right(2));
						CEagleDataManager::GetInstance()->m_Config.m_Option.strRebootTime = strTemp;
					}
					else
					{
						LOG(Error, _T("Skip - Reboot Time is range error. strHour : (%s), strMinute : (%s)"), strHour, strMinute);
					}
				}
				else
				{
					LOG(Error, _T("Skip - Reboot Time is length error (%s)"), strTemp2);
				}
			}
			else
			{
				LOG(Error, _T("Skip - Reboot Time is invalid (%s)"), strTemp2);
			}
		}
	}

	//9. Network Type - Read Only
	// Network Type은 AMS에 변경 불가능하도록 UI 구성됨
	if (strArrRecvData.GetCount() > nCnt)
		strTemp = strArrRecvData.GetAt(nCnt++);

	// 10. DHCP Use
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);
		strTemp.TrimLeft();
		strTemp.TrimRight();

		if (strTemp != ESC)
		{
			// DHCP 변경 여부 확인
			strTemp2 = CEagleDataManager::GetInstance()->m_Config.m_Host.strUseDHCP;	// Previous DHCP Information
			strTemp2.TrimLeft();
			strTemp2.TrimRight();

			CEagleDataManager::GetInstance()->m_Config.m_Host.strUseDHCP = strTemp;

			if (CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType == _T("TCP/IP"))
			{
				if (strTemp2.CompareNoCase(strTemp) != 0)
				{
					LOG(Info, _T("DHCP from Terminal : [%s], DHCP from AMS : [%s]"), strTemp, strTemp2);
					m_bChangedIP = TRUE;
				}
			}
		}
	}

	// 11. Terminal IP
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);
		strTemp.TrimLeft();
		strTemp.TrimRight();

		if (strTemp != ESC)
		{
			if (CUtil::Check_Valid_IP_Format(strTemp) == TRUE)
			{
				// Terminal IP 변경 여부 확인
				strTemp2 = CEagleDataManager::GetInstance()->m_Config.m_Host.strTerminalip;
				strTemp2.TrimLeft();
				strTemp2.TrimRight();

				CEagleDataManager::GetInstance()->m_Config.m_Host.strTerminalip = strTemp;

				if (CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType == _T("TCP/IP"))
				{
					if (strTemp2.CompareNoCase(strTemp) != 0)
					{
						LOG(Info, _T("TerminalIP from Terminal : [%s], TerminalIP from AMS : [%s]"), strTemp2, strTemp);
						m_bChangedIP = TRUE;
					}
				}
			}
			else
			{		
				LOG(Error, _T("Terminal IP is invalid (%s)"), strTemp);
			}
		}
	}

	// 12. Subnet
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);
		strTemp.TrimLeft();
		strTemp.TrimRight();

		if (strTemp != ESC)
		{
			if (CUtil::Check_Valid_IP_Format(strTemp) == TRUE)
			{
				// Subnet Mask 변경 여부 확인
				strTemp2 = CEagleDataManager::GetInstance()->m_Config.m_Host.strSubnet;
				strTemp2.TrimLeft();
				strTemp2.TrimRight();

				CEagleDataManager::GetInstance()->m_Config.m_Host.strSubnet = strTemp;

				if (CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType == _T("TCP/IP"))
				{
					if (strTemp2.CompareNoCase(strTemp) != 0)
					{
						LOG(Info, _T("Subnet Mask from Terminal : [%s], Subnet Mask from AMS : [%s]"), strTemp2, strTemp);
						m_bChangedIP = TRUE;
					}
				}
			}
			else
			{		
				LOG(Error, _T("Subnet Mask is invalid (%s)"), strTemp);
			}
		}
	}


	// 13. GateWay
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);
		strTemp.TrimLeft();
		strTemp.TrimRight();

		if (strTemp != ESC)
		{
			if (CUtil::Check_Valid_IP_Format(strTemp) == TRUE)
			{
				// Gateway 변경 여부 확인
				strTemp2 = CEagleDataManager::GetInstance()->m_Config.m_Host.strGateway;
				strTemp2.TrimLeft();
				strTemp2.TrimRight();

				CEagleDataManager::GetInstance()->m_Config.m_Host.strGateway = strTemp;

				if (CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType == _T("TCP/IP"))
				{
					if (strTemp2.CompareNoCase(strTemp) != 0)
					{
						LOG(Info, _T("Gateway from Terminal : [%s], Gateway from AMS : [%s]"), strTemp2, strTemp);
						m_bChangedIP = TRUE;
					}
				}
			}
			else
			{		
				LOG(Error, _T("Gateway is invalid (%s)"), strTemp);
			}
		}
	}

	// 14. DNS
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);
		strTemp.TrimLeft();
		strTemp.TrimRight();

		if (strTemp != ESC)
		{
			if (CUtil::Check_Valid_IP_Format(strTemp) == TRUE)
			{
				// DNS 변경 여부 확인
				strTemp2 = CEagleDataManager::GetInstance()->m_Config.m_Host.strDNS;
				strTemp2.TrimLeft();
				strTemp2.TrimRight();

				CEagleDataManager::GetInstance()->m_Config.m_Host.strDNS = strTemp;

				if (CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType == _T("TCP/IP"))
				{
					if (strTemp2.CompareNoCase(strTemp) != 0)
					{
						LOG(Info, _T("DNS from Terminal : [%s], DNS from AMS : [%s]"), strTemp2, strTemp);
						m_bChangedIP = TRUE;
					}
				}
			}
			else
			{		
				LOG(Error, _T("DNS is invalid (%s)"), strTemp);
			}
		}
	}

	// 15. Setup String
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);
		strTemp.TrimLeft();
		strTemp.TrimRight();

		if (strTemp != ESC)
		{
			if(strTemp.GetLength() <= 20)
				CEagleDataManager::GetInstance()->m_Config.m_Host.strSetup_String = strTemp;
			else
				CEagleDataManager::GetInstance()->m_Config.m_Host.strSetup_String = strTemp.Left(20);	// 20자가 넘는 경우 20자까지만 가능하게 처리
		}
	}

	// 16. Camera 사용 유/무 추가
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			if (CDEV_Manager::GetInstance()->m_DEV_CAMERA.IsExistCameraDriver() == TRUE)
			{
				BOOL bBeforeCAMSetting = CDEV_Manager::GetInstance()->m_DEV_CAMERA.GetCameraSetting();

				if (strTemp == S_ENABLE)
				{
					// 이전 상태가 Disable 상태인 경우 Enable 처리 후 Camera Open 처리 (이미 Enable 상태인 경우는 Skip 처리)
					if (bBeforeCAMSetting == FALSE)
					{
						LOG(Info, _T("Active Camera from AMS"));

						CDEV_Manager::GetInstance()->m_DEV_CAMERA.SetUseCamera(TRUE);

						// Camera Driver가 존재한 경우 AP 기동시 이미 Driver Init은 수행한 상태이므로 Open 수행
						if (CDEV_Manager::GetInstance()->m_DEV_CAMERA.IsOpened() == FALSE)
							CDEV_Manager::GetInstance()->m_DEV_CAMERA.Open();
					}
				}
				else if (strTemp == S_DISABLE)
				{
					// 이전 상태가 Enable 상태인 경우 Disable 후 Camera Close 처리
					if (bBeforeCAMSetting == TRUE)
					{
						LOG(Info, _T("DeActive Camera from AMS"));

						CDEV_Manager::GetInstance()->m_DEV_CAMERA.SetUseCamera(FALSE);

						if (CDEV_Manager::GetInstance()->m_DEV_CAMERA.IsOpened() == TRUE)
							CDEV_Manager::GetInstance()->m_DEV_CAMERA.Close();
					}
				}
				else
				{
					// Skip Camera Setting because the data is null
					LOG(Info, _T("Skip - Camera Setting information is not exist from AMS"));
				}
			}
		}
	}

	return TRUE;
}


/** ********************************************************************
* @brief AMS로부터 받은 host setup 정보 세팅
* @param CString *pstrRecvData		AMS로부터 받은 정보
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
BOOL CDEV_HOST::AMS_WriteSetupHostMsg(CStringArray &strArrRecvData)
{
	CString strTemp;

	BOOL bNumeric = TRUE;
	BOOL bInvalid = FALSE;

	int i=0;
	int nCnt = 0;
	int nValue = 0;

	//>>>>> Modem <<<<<
	// 1. Host Field Item Count

	// 2. Communication Header
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			CEagleDataManager::GetInstance()->m_Config.m_Host.strCommunication_header = strTemp;
		}
	}

	// 3. Communication ID
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			if(strTemp.GetLength() <= 8)
				CEagleDataManager::GetInstance()->m_Config.m_Host.strCommunicationID = strTemp;
			else
				CEagleDataManager::GetInstance()->m_Config.m_Host.strCommunicationID = strTemp.Left(8);
		}
	}

	// 4. ATM Status monitoring
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			CEagleDataManager::GetInstance()->m_Config.m_Host.strAtm_status_monitoring = strTemp;
		}
	}

	// 5. Heartbeat message
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			CEagleDataManager::GetInstance()->m_Config.m_Host.strHeartbeat_message = strTemp;
		}
	}

	// 6. Heartbeat frequency
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			if (CUtil::IsNumeric(strTemp) == TRUE)
			{
				nValue = CUtil::StringToInt(strTemp);

				if (nValue >= 1 && nValue <= 999)
					CEagleDataManager::GetInstance()->m_Config.m_Host.strHeartbeat_frequency.Format(_T("%d"), nValue);
				else
					LOG(Error, _T("Heartbeat frequency is range error. Value : (%d)"), nValue);
			}
			else
			{
				LOG(Error, _T("Heartbeat frequency is invalid (%s) - Skip"), strTemp);
			}
		}
	}

	// 7. Primary Host Phone number
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			if(strTemp.GetLength() <= 20)
				CEagleDataManager::GetInstance()->m_Config.m_Host.strPrimary_phone_number = strTemp;
			else
				CEagleDataManager::GetInstance()->m_Config.m_Host.strPrimary_phone_number = strTemp.Left(20);
		}
	}

	// 8. Backup Host Phone number
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			if(strTemp.GetLength() <= 20)
				CEagleDataManager::GetInstance()->m_Config.m_Host.strBackup_phone_number = strTemp;
			else
				CEagleDataManager::GetInstance()->m_Config.m_Host.strBackup_phone_number = strTemp.Left(20);
		}
	}

	// 9. PreDial
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			CEagleDataManager::GetInstance()->m_Config.m_Host.strPre_dial = strTemp;
		}
	}

	// 10. CRC Required
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			CEagleDataManager::GetInstance()->m_Config.m_Host.strCrc_flg = strTemp;
		}
	}

	// 11. Host IP or URL - URL값이 올 수 있으므로 IP Base로 Validation을 Check하면 안됨.
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			CEagleDataManager::GetInstance()->m_Config.m_Host.strHostip = strTemp;
		}
	}

	// 12. Port
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			nValue = CUtil::StringToInt(strTemp);

			if (nValue >= 0 && nValue <= 65535)
				CEagleDataManager::GetInstance()->m_Config.m_Host.strPort.Format(_T("%d"), nValue);
			else
				LOG(Error, _T("Skip - Primary Host Port is invalid (%s)"), strTemp);
		}
	}

	// 13. Backup Host IP or URL - URL값이 올 수 있으므로 IP Base로 Validation을 Check하면 안됨.
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			CEagleDataManager::GetInstance()->m_Config.m_Host.strBackup_Hostip = strTemp;
		}
	}

	// 14. Backup Port
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			nValue = CUtil::StringToInt(strTemp);

			if (nValue >= 0 && nValue <= 65535)
				CEagleDataManager::GetInstance()->m_Config.m_Host.strBackupPort.Format(_T("%d"), nValue);
			else
				LOG(Error, _T("Skip - Backup Host Port is invalid (%s)"), strTemp);
		}
	}

	// 15. Protocol Type
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			if(strTemp == _T("0") )
				CEagleDataManager::GetInstance()->m_Config.m_Host.strProtocolType = _T("VISA");
			else
				CEagleDataManager::GetInstance()->m_Config.m_Host.strProtocolType = _T("STANDARD");
		}
	}

	// 16. ENQ Required
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			CEagleDataManager::GetInstance()->m_Config.m_Host.strUseENQ = strTemp;
		}
	}

	// 17. EOT Required
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			CEagleDataManager::GetInstance()->m_Config.m_Host.strUseEOT = strTemp;
		}
	}

	// 18. TLS Required
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			CEagleDataManager::GetInstance()->m_Config.m_Host.strUseTLS = strTemp;
		}
	}

	return TRUE;
}


/** ********************************************************************
* @brief AMS로부터 받은 AMS setup 정보 세팅
* @param CString *pstrRecvData		AMS로부터 받은 정보
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
BOOL CDEV_HOST::AMS_WriteSetupAMSMsg(CStringArray &strArrRecvData)
{
	CString strTemp;
	//CStringArray strArrData;

	BOOL bNumeric = TRUE;
	BOOL bInvalid = FALSE;

	int i=0;
	int nCnt = 0;
	int nValue = 0;

	// 1. AMS Enable/Disable
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_en_flg = strTemp;
		}
	}

	// 2. AMS Status Send
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_Status_en_flg = strTemp;
		}
	}

	// 3. AMS Primary Number
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			if(strTemp.GetLength() <= 20)
				CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_primary_num = strTemp;
			else
				CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_primary_num = strTemp.Left(20);
		}
	}

	// 4. AMS IP Address
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);
		strTemp.TrimLeft();
		strTemp.TrimRight();

		if (strTemp != ESC)
		{
			// 2021.04.12 LEH - URL 주소로도 통신가능하도록 변경했기에 해당 조건 주석 처리함.
		//	if (CUtil::Check_Valid_IP_Format(strTemp) == TRUE)
			{
				CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_IP_Address = strTemp;
			}
			//else
			//{
			//	LOG(Error, _T("AMS IP is invalid (%s)"), strTemp);
			//}
		}
	}

	// 5. AMS Port
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			nValue = CUtil::StringToInt(strTemp);

			if (nValue >= 0 && nValue <= 65535)
				CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_Port.Format(_T("%d"), nValue);
			else
				LOG(Error, _T("Skip - AMS Port is invalid (%s)"), strTemp);
		}
	}

	// 6. Schedule journal
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			CEagleDataManager::GetInstance()->m_Config.m_Host.strSchedule_journal_flg = strTemp;
		}
	}

	// 7. Schedule journal count
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			CEagleDataManager::GetInstance()->m_Config.m_Host.strSchedule_journal_count = strTemp;
		}
	}

	return TRUE;
}


/** ********************************************************************
* @brief AMS로부터 받은 iso setup 정보 세팅
* @param CString *pstrRecvData		AMS로부터 받은 정보
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
BOOL CDEV_HOST::AMS_WriteSetupISOMsg(CStringArray &strArrRecvData)
{
	int i = 0, nCnt = 0;
	CString strTemp;

	for(i=0; i<MAX_BIN_COUNT; i++)
	{
		if (strArrRecvData.GetCount() > nCnt)
		{
			strTemp = strArrRecvData.GetAt(nCnt++);
			strTemp.TrimLeft();
			strTemp.TrimRight();

			if (strTemp != ESC)
			{
				if ( (strTemp.IsEmpty() == TRUE) || (CUtil::IsNumeric(strTemp) == TRUE) )	// 숫자만 가능
				{
					if(strTemp.GetLength() <= 10)
						CEagleDataManager::GetInstance()->m_Config.m_Iso.strBLOCK_ISO[i] = strTemp;
					else
						CEagleDataManager::GetInstance()->m_Config.m_Iso.strBLOCK_ISO[i] = strTemp.Left(10);
				}
				else
				{
					LOG(Error, _T("Skip - BIN[%d] is invalid (%s)"), i+1, strTemp);
				}
			}
		}
		else
		{
			LOG(Error, _T("Break - BIN Data Count Error (%d)"), strArrRecvData.GetCount());
			break;
		}
	}

	return TRUE;
}


/** ********************************************************************
* @brief AMS로부터 받은 message setup 정보 세팅
* @param CString *pstrRecvData		AMS로부터 받은 정보
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
BOOL CDEV_HOST::AMS_WriteSetupMessageMsg(CStringArray &strArrRecvData)
{
	int i = 0, nCnt = 0;
	CString strTemp;

	// marketing message
	for(i=0; i<4; i++)
	{
		if (strArrRecvData.GetCount() > nCnt)
		{
			// 왼쪽 정렬
			strTemp = strArrRecvData.GetAt(nCnt++);
			strTemp.TrimRight();	// 우측 Space만 제거 (왼쪽 정렬이므로)

			if (strTemp != ESC)
			{
				if(strTemp.GetLength() <= 40)
					CEagleDataManager::GetInstance()->m_Config.m_Message.strMarketing_message[i] = strTemp;
				else
					CEagleDataManager::GetInstance()->m_Config.m_Message.strMarketing_message[i] = strTemp.Left(40);
			}
		}
		else
		{
			LOG(Error, _T("Skip - Marketing Msg Data Count Error (%d)"), strArrRecvData.GetCount());
		}
	}

	// location message
	for(i=0; i<4; i++)
	{
		if (strArrRecvData.GetCount() > nCnt)
		{
			// 왼쪽 정렬
			strTemp = strArrRecvData.GetAt(nCnt++);
			strTemp.TrimRight();

			if (strTemp != ESC)
			{
				if(strTemp.GetLength() <= 40)
					CEagleDataManager::GetInstance()->m_Config.m_Message.strLocation_message[i] = strTemp;
				else
					CEagleDataManager::GetInstance()->m_Config.m_Message.strLocation_message[i] = strTemp.Left(40);
			}	
		}
		else
		{
			LOG(Error, _T("Skip - Location Msg Data Count Error (%d)"), strArrRecvData.GetCount());
		}
	}

	// attraction message
	for(i=0; i<3; i++)
	{
		if (strArrRecvData.GetCount() > nCnt)
		{
			// Center 정렬
			strTemp = strArrRecvData.GetAt(nCnt++);
			strTemp.TrimLeft();
			strTemp.TrimRight();

			if (strTemp != ESC)
			{
				if(strTemp.GetLength() <= 30)
					CEagleDataManager::GetInstance()->m_Config.m_Message.strAttraction_message[i] = strTemp;
				else
					CEagleDataManager::GetInstance()->m_Config.m_Message.strAttraction_message[i] = strTemp.Left(30);
			}
		}
		else
		{
			LOG(Error, _T("Skip - Attraction Msg Data Count Error (%d)"), strArrRecvData.GetCount());
		}
	}

	// farewell message
	for(i=0; i<3; i++)
	{
		if (strArrRecvData.GetCount() > nCnt)
		{
			// Center 정렬
			strTemp = strArrRecvData.GetAt(nCnt++);
			strTemp.TrimLeft();
			strTemp.TrimRight();

			if (strTemp != ESC)
			{
				if(strTemp.GetLength() <= 30)
					CEagleDataManager::GetInstance()->m_Config.m_Message.strFarewell_message[i] = strTemp;
				else
					CEagleDataManager::GetInstance()->m_Config.m_Message.strFarewell_message[i] = strTemp.Left(30);
			}
		}
		else
		{
			LOG(Error, _T("Skip - Farewell Msg Data Count Error (%d)"), strArrRecvData.GetCount());
		}
	}

	return TRUE;
}


/** ********************************************************************
* @brief AMS로부터 받은 option setup 정보 세팅
* @param CString *pstrRecvData		AMS로부터 받은 정보
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
BOOL CDEV_HOST::AMS_WriteSetupOptionMsg(CStringArray &strArrRecvData)
{
	int nCnt = 0;
	CString strTemp, strTemp2;
	CString strHour, strMinute;
	
	// 1. Option Field Item Count

	// 5. Saving account
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			CEagleDataManager::GetInstance()->m_Config.m_Option.strSaving_account = strTemp;
		}
	}

	// 6. Credit account
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			CEagleDataManager::GetInstance()->m_Config.m_Option.strCredit_account = strTemp;
		}
	}

	// 7. Transfer account
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			CEagleDataManager::GetInstance()->m_Config.m_Option.strtransfer_Enable = strTemp;
		}
	}

	// 8. Balance account
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			CEagleDataManager::GetInstance()->m_Config.m_Option.strbalance_Enable = strTemp;
		}
	}

	// 9 ~ 14. Fast amount
	for(int i=0; i<6; i++)
	{
		if (strArrRecvData.GetCount() > nCnt)
		{
			strTemp = strArrRecvData.GetAt(nCnt++);

			if (strTemp != ESC)
			{
				strTemp.Replace(_T("."), _T(""));	// "." 제거

				if (CUtil::IsNumeric(strTemp) == TRUE)	// 숫자만 가능
				{
					if(strTemp.GetLength() <= 5)
						CEagleDataManager::GetInstance()->m_Config.m_Option.strFast_amount[i] = strTemp.Left(strTemp.GetLength()-2);	// cent 제거해서 저장
					else
						LOG(Error, _T("Fast Amount[%d] Length is invalid. (%s)"), i+1, strTemp);
				}
				else
				{
					LOG(Error, _T("Fast Amount[%d] is invalid (%s)"), i+1, strTemp);
				}
			}
		}
	}

	// 15. Max withdrawal amount
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			if (CUtil::IsNumeric(strTemp) == TRUE)	// 숫자만 가능
			{
				if(strTemp.GetLength() <= 5)
					CEagleDataManager::GetInstance()->m_Config.m_Option.strMax_withdrawal_amount = strTemp.Left(strTemp.GetLength()-2);	// cent 제거해서 저장
				else
					LOG(Error, _T("Max withdrawal amount Length is invalid. (%s)"), strTemp);
			}
			else
			{
				LOG(Error, _T("Max withdrawal amount is invalid (%s)"), strTemp);
			}
		}
	}

	// 16. Surcharge Type
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			CEagleDataManager::GetInstance()->m_Config.m_Option.strSurcharge_type = strTemp;
		}
	}

	// 17. Withdrawal surcharge amount
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			strTemp.Replace(_T("."), _T(""));	// "." 제거

			if (CUtil::IsNumeric(strTemp) == TRUE)	// 숫자만 가능
			{
				if(strTemp.GetLength() <= 4)
				{
					CEagleDataManager::GetInstance()->m_Config.m_Option.strWithdrawalSurcharge_amount.Format(_T("%03d"), CUtil::StringToInt(strTemp));	// Cent 포함
				}
				else
				{
					LOG(Error, _T("Withdrawal surcharge amount Length is invalid. (%s)"), strTemp);
				}
			}
			else
			{
				LOG(Error, _T("Withdrawal surcharge amount is invalid (%s)"), strTemp);
			}
		}
	}

	// 18. Surcharge percentage
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			strTemp.Replace(_T("."), _T(""));	// "." 제거

			if (CUtil::IsNumeric(strTemp) == TRUE)	// 숫자만 가능
			{
				// Range 추가 Validation 점검 필요
				if(strTemp.GetLength() <= 4)
					CEagleDataManager::GetInstance()->m_Config.m_Option.strSurcharge_percentage = strTemp;
				else
					LOG(Error, _T("Surcharge percentage Length is invalid. (%s)"), strTemp);
			}
			else
			{
				LOG(Error, _T("Surcharge percentage is invalid (%s)"), strTemp);
			}
		}
	}

	// 19. Percent Surcharge option
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			CEagleDataManager::GetInstance()->m_Config.m_Option.strWithdrawal_PercentSurcharge_Option = strTemp;
		}
	}

	// 20. Surcharge owner
	if (strArrRecvData.GetCount() > nCnt)
	{
		// 왼쪽 정렬
		strTemp = strArrRecvData.GetAt(nCnt++);
		strTemp.TrimRight();

		if (strTemp != ESC)
		{
			if(strTemp.GetLength() <= 20)
				CEagleDataManager::GetInstance()->m_Config.m_Option.strSurcharge_owner = strTemp;
			else
				CEagleDataManager::GetInstance()->m_Config.m_Option.strSurcharge_owner = strTemp.Left(20);
		}
	}

	// 21. English
	if (strArrRecvData.GetCount() > nCnt)
	{
		// 영어는 read only
		strTemp = strArrRecvData.GetAt(nCnt++);
		//CEagleDataManager::GetInstance()->m_Config.m_Option.strLansupport[0] = strTemp;
	}

	// 22. Spanish
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			CEagleDataManager::GetInstance()->m_Config.m_Option.strLansupport[1] = strTemp;
		}
	}

	// 23. French
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			CEagleDataManager::GetInstance()->m_Config.m_Option.strLansupport[2] = strTemp;
		}
	}

	// 24. Schedule day close
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			CEagleDataManager::GetInstance()->m_Config.m_Option.strScheduleDayClose = strTemp;
		}
	}

	// 25. Day close time
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp2 = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			strTemp2.Replace(_T(":"), _T(""));	// ":"제거

			if (CUtil::IsNumeric(strTemp2) == TRUE)	// 숫자만 가능
			{
				if (strTemp2.GetLength() == 4)
				{
					strHour = strTemp2.Left(2);
					strMinute = strTemp2.Right(2);
					int nHour = CUtil::StringToInt(strHour);
					int nMinute = CUtil::StringToInt(strMinute);

					if ( ((nHour >= 0) && (nHour <= 23)) && ((nMinute >= 0) && (nMinute <= 59)) )
					{
						strTemp.Format(_T("%2.2s%2.2s"), strTemp2.Left(2), strTemp2.Right(2));
						CEagleDataManager::GetInstance()->m_Config.m_Option.strDayCloseTime = strTemp;
					}
					else
					{
						LOG(Error, _T("Day close Time is range error. strHour : (%s), strMinute : (%s)"), strHour, strMinute);
					}
				}
				else
				{
					LOG(Error, _T("Day close Time is invalid length (%s)"), strTemp2);
				}
			}
			else
			{
				LOG(Error, _T("Day close Time is invalid (%s)"), strTemp2);
			}
		}
	}

	return TRUE;
}


/** ********************************************************************
* @brief AMS로부터 받은 adv setup 정보 세팅
* @param CString *pstrRecvData		AMS로부터 받은 정보
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
BOOL CDEV_HOST::AMS_WriteSetupAdvertisementMsg(CStringArray &strArrRecvData)
{
	int nCnt = 0;
	CString strTemp;
	
	// 1. advertisement interval time
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);

		if (strTemp != ESC)
		{
			if (CUtil::IsNumeric(strTemp) == TRUE)
			{
				int nValue = CUtil::StringToInt(strTemp);

				if ( (nValue >= 5) && (nValue <= 60) )
					CEagleDataManager::GetInstance()->m_Config.m_ADS.strAdver_Interval_time = strTemp;
				else
					LOG(Error, _T("Adv interval time is range error. (%s)"), strTemp);
			}
			else
			{
				LOG(Error, _T("Adv interval is invalid (%s)"), strTemp);
			}
		}
	}

	// 2. advertisement screen
	for(int i=0; i<SUPPORT_ADVERTISE; i++)
	{
		if (strArrRecvData.GetCount() > nCnt)
		{
			strTemp = strArrRecvData.GetAt(nCnt++);

			if (strTemp != ESC)
			{
				CEagleDataManager::GetInstance()->m_Config.m_ADS.strAdver_Screen_en[i] = strTemp;
			}	
		}
	}

	return TRUE;
}


/** ********************************************************************
* @brief AMS로부터 받은 EMV setup 정보 세팅
* @param CString *pstrRecvData		AMS로부터 받은 정보
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
BOOL CDEV_HOST::AMS_WriteSetupEMVMsg(CStringArray &strArrRecvData)
{
	int nCnt = 0;
	int nAidCount = 0;

	CString strTemp;

	CStringArray strArrAidData;

	// 1. EMV Field Item Count

	// 2. AID Count
	if (strArrRecvData.GetCount() > nCnt)
	{
		strTemp = strArrRecvData.GetAt(nCnt++);
		nAidCount = CUtil::StringToInt(strTemp);
	}

	// 3 ~ 62. aid info
	for(int i=0; i<nAidCount; i++)
	{
		if (strArrRecvData.GetCount() > nCnt)
		{
			strTemp = strArrRecvData.GetAt(nCnt++);

			strArrAidData.RemoveAll();
			CUtil::ParsingStringToStringArray(strTemp, (CString)US, strArrAidData);

			if (strArrAidData.GetCount() > 2)
			{
				if (strArrAidData.GetAt(1).GetLength() > 0)	// AID Name이 존재할 경우에만 Setting
				{
					if (strArrAidData.GetAt(0) != ESC)
					{
						CDEV_Manager::GetInstance()->m_DEV_CDR.m_sEMV_Config_Data.m_sEMV_Config[i].AidUse = CUtil::StringToInt(strArrAidData.GetAt(0));
					}
				}
			}
		}
	}

	return TRUE;
}


/** ********************************************************************
* @brief SW Upload
* @param CString strSubCommand	Sub Command
* @param BYTE *pbyData			AMS로부터 받은 Upload data
* @param int nRecvLen			AMS로부터 받은 data length
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
BOOL CDEV_HOST::AMS_ProcSWUpload(CString strSubCommand, BYTE *pbyData, int nRecvLen)
{
	BOOL bReturn = FALSE;
	AMS_CMD_SW_UPLOAD uploadMsg;
	char *pchRecvBase64Data = NULL;
	BYTE byDataEOT = DATA_EOT;
	int nRecvLength = 0;
	int i = 0, nResult = 0;
	int nIteration = 0;
	int nFileDataLen = 0;
	DWORD dwWriten = 0;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	CString strFileName, strDstFolder, strFilePath, strFileData;
	CString strUploadData;
	CStringArray strArrUploadData;

	strArrUploadData.RemoveAll();
	strUploadData = pbyData;

	CUtil::ParsingStringToStringArray(strUploadData, (CString)FS, strArrUploadData);

	if(strArrUploadData.GetCount() > 5)
		strFileName = strArrUploadData[5];

	//*LEH
	if(AMS_SUB_SW_UPDATE == strSubCommand)
		strDstFolder.Empty();	// Root에서 압축해제를 위해 Null
	else if(AMS_SUB_ADV_UPDATE == strSubCommand)
		strDstFolder.Format(_T("%s%s\\%d_%d"), CUtil::GetAppPath(), EAGLE_ATM_AD_PATH, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
//	else if(_T("C") == strSubCommand)
//		strDstFolder.Format(_T("%s\\Receipt"), CUtil::GetAppPath());
	else
	{
		LOG(Error, _T("AMS_ProcSWUpload() Param(%s) is undefined"), strSubCommand);
		return FALSE;
	}

	CreateDirectory(strDstFolder, NULL);
	CreateDirectory(PATH_TEMP_ABS, NULL);

	strFilePath.Format(_T("%s\\%s"), PATH_TEMP_ABS, strFileName);
	/////////////////////////////////////////////////////////

	LOG(Info, _T("AMS_ProcSWUpload() SW Upload path :[%s]"), strFilePath);

	// open file
	hFile = CreateFile (strFilePath,
		GENERIC_WRITE,					// Open for writing
		0,								// Do not share
		NULL,							// No security
		CREATE_ALWAYS,					// Open or create
		FILE_ATTRIBUTE_NORMAL,			// Normal file
		NULL);							// No template file
	
	if ( (INVALID_HANDLE_VALUE == hFile) || (NULL == hFile)	)
	{
		LOG(Error, _T("AMS_ProcSWUpload() CreateFile is failed."));
		return FALSE;
	}

	memset(m_pAMSRecvBuffer, 0, NET_RECV_BUFF_SIZE);

	pchRecvBase64Data = new char[NET_RECV_BUFF_SIZE];
	memset(pchRecvBase64Data, 0, NET_RECV_BUFF_SIZE);

	if(strArrUploadData.GetCount() > 6)
		nIteration = CUtil::StringToInt(strArrUploadData[6]);

	LOG(Info, _T("AMS_ProcSWUpload() Iteration : [%d]"), nIteration);

	nRecvLength = nRecvLen;
	
	while(1)
	{
		dwWriten = 0;
		memset(&uploadMsg, 0, sizeof(AMS_CMD_SW_UPLOAD));
		strFileData.Empty();

		// Base64 변경에 따라 로직 변경함
		if(strArrUploadData.GetCount() > 7)
		{
			memset(pchRecvBase64Data, 0, NET_RECV_BUFF_SIZE);

			strFileData = strArrUploadData.GetAt(7);
			sprintf(pchRecvBase64Data, "%S", strFileData);

			//LOG(Info, _T("Base64 Data Length : %d"), strFileData.GetLength());

//#if (SEND_RECV_DEBUG_AMS)
//			LOG(Info, _T("Base64 Data : %s"), strFileData);
//#endif


			nFileDataLen = CUtil::Base64_Decoded(pchRecvBase64Data, uploadMsg.byFileData, strFileData.GetLength());
		}

		//LOG(Info, _T("AMS_ProcSWUpload() nFileDataLen : [%d]"), nFileDataLen);

		//memcpy(uploadMsg.byFileData, &pbyData[nFileDataIndex], nFileDataLen);

		if (!WriteFile(hFile, uploadMsg.byFileData, nFileDataLen, &dwWriten, NULL))
		{
			LOG(Error, _T("AMS_ProcSWUpload() File Write Failed."));
			
			bReturn = FALSE;
			break;
		}

		if(0 >= nIteration)	// 파일이 2K 이하(<= 2048)면, nIteration값이 1이다.
		{
			LOG(Info, _T("AMS_ProcSWUpload() SW Upload Success."));

			bReturn = TRUE;
			break;
		}
		else
		{
			// Recv CMD
			memset(m_pAMSRecvBuffer, 0, NET_RECV_BUFF_SIZE);
			nRecvLength = 0;

			// [PCI-SSF Fix] m_pAMSRecvBuffer 실제 용량(NET_RECV_BUFF_SIZE) 전달
			nRecvLength = HOST_Sync_Recv(m_pAMSRecvBuffer, NET_RECV_BUFF_SIZE, TRUE);

			if (nRecvLength <= (LEN_LENGTH + 1))	// +1 => 0x1C
			{
				LOG(Error, _T("AMS_ProcSWUpload() RecvSize is invalid (%d)"), nIteration);
				bReturn = FALSE;
				break;
			}

			if(0 == strlen((char*)m_pAMSRecvBuffer))
			{
				LOG(Error, _T("AMS_ProcSWUpload() RecvData is NULL (%d)"), nIteration);
				bReturn = FALSE;
				break;
			}

			strArrUploadData.RemoveAll();
			strUploadData = m_pAMSRecvBuffer;
			CUtil::ParsingStringToStringArray(strUploadData, (CString)FS, strArrUploadData);

			if(strArrUploadData.GetCount() > 6)
				nIteration = CUtil::StringToInt(strArrUploadData[6]);

			//LOG(Info, _T("AMS_ProcSWUpload() 1 Iteration : [%d]"), nIteration);
		}
	}

	if ( (hFile != INVALID_HANDLE_VALUE) && (hFile != NULL)	)
	{
		CloseHandle(hFile);
	}

	if (pchRecvBase64Data != NULL)
	{
		delete [] pchRecvBase64Data;
		pchRecvBase64Data = NULL;
	}

	if(TRUE == bReturn)
	{
		if (strSubCommand == AMS_SUB_ADV_UPDATE)
		{
			// ADV Update인 경우는 이전 ADV만 제거 후 update하도록 개선
			LOG(Info, _T("Delete Destination ADV Files (%s)"), strDstFolder);

			CUtil::DeleteFileInDirectory(strDstFolder, _T("*.*"));

			CreateDirectory(strDstFolder, NULL);
		}

		LOG(Info, _T("Unzip path : strFilePath(%s), strDstFolder(%s)"), strFilePath, strDstFolder);

		if (!CUtil::UnzippingToFolder(strFilePath, strDstFolder))
		{
			LOG(Error, _T("Failed to Unzip [%s] -> [%s]"), strFilePath, strDstFolder);
			bReturn = FALSE;
		}
		else
		{
			LOG(Info, _T("Unzip success"));
		}
	}

	// File Download 중에 실패 발생시 받은 파일을 삭제하도록 로직 보완
	CUtil::DeleteFileInDirectory(PATH_TEMP_ABS, _T("*.*"));

	CString strTemp;

 	// Execute SoftwareUpdate
	if( strSubCommand == AMS_SUB_SW_UPDATE)
	{
		// EOT 전송
		// Additional Command를 전송하지 못하는 Command에서는 EOT를 전송해서 통신종료를 Server에 알려줌.
		// S/W Update, System Reboot, Setup Write(TerminalIP/DHCP 변경시) 일 경우
		nResult = HOST_Sync_Send(&byDataEOT, 1, _T(""), _T(""), _T(""), TRUE);

		if(nResult != AMS_OK)
		{
			LOG(Error, _T("AMS_ProcSWUpload() Send EOT Failed."));
		}

		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_1000MS);

		// Modem or Network Close 후 System Reboot하도록 로직 수정
		AMS_Sync_ConnectClose();

		if ((bReturn == TRUE) && (CUtil::IsExistFile(EXE_SOFTWARE_UPDATE_FROM_AMS_PATH) == TRUE))
		{
			// Download 받은 Fil에 Softwareupdate.exe가 존재하는지 확인 후 수행하도록 변경
			LOG(Info, _T("Software Update - try to close devices"));

			// Software update전 Device Close 수행
			CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_ClosePort();
			CDEV_Manager::GetInstance()->m_DEV_DIO.Close();
			CDEV_Manager::GetInstance()->m_DEV_CDR.Close();
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_Close();

			if (CDEV_Manager::GetInstance()->m_DEV_CAMERA.IsOpened() == TRUE)
				CDEV_Manager::GetInstance()->m_DEV_CAMERA.Close();

			LOG(Info, _T("Set Software Update Type - AMS update"));

			CUtil::SetSWUpdateType(UPDATE_KIND_BY_AMS_STRING);	// 원격으로 SW Update 하는 것으로 세팅

			strTemp = _T("[AMS]SOFTWARE UPDATE");
			CDEV_Manager::GetInstance()->SaveEJL_SetTextValue(text_field, strTemp);

			strTemp = _T("[AMS]SYSTEM REBOOT");
			CDEV_Manager::GetInstance()->SaveEJL_SetTextValue(reboot, strTemp);

			STARTUPINFO si;
			ZeroMemory (&si, sizeof(si));
			si.cb = sizeof (si);
			si.wShowWindow = SW_HIDE;
			PROCESS_INFORMATION pi;
			ZeroMemory (&pi, sizeof(pi));

			CString strPath;
			strPath.Format(_T("%s%s"), CUtil::GetAppPath(), EXE_TERMINATE_EAGLECE);

			CreateProcess(strPath, NULL,NULL,NULL,NULL,FALSE,0,NULL,&si, &pi);

			LOG(Info, _T("Software Update - execute software update"));
			CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_5MIN);

			LOG(Info, _T("Software Update is successful - try System Reboot"));
			CDEV_Manager::GetInstance()->System_Reboot();
		}
		else
		{
			// Software Update File이 잘못 등록됨. 재부팅 수행
			strTemp = _T("[AMS]SOFTWARE UPDATE ERROR");
			CDEV_Manager::GetInstance()->SaveEJL_SetTextValue(text_field, strTemp);

			strTemp = _T("[AMS]SYSTEM REBOOT");
			CDEV_Manager::GetInstance()->SaveEJL_SetTextValue(reboot, strTemp);

			LOG(Info, _T("Software Update is failed - try System Reboot"));
			CDEV_Manager::GetInstance()->System_Reboot();
		}
	}

	if (strSubCommand == AMS_SUB_ADV_UPDATE)
	{
		if ( bReturn == TRUE )
		{
			strTemp = _T("[AMS]AD UPDATE");
			CDEV_Manager::GetInstance()->SaveEJL_SetTextValue(text_field, strTemp);

			// AD Upload 후 ADV를 다시 Load하도록 로직 추가
			CSCR_Manager::GetInstance()->LoadAdvertisement();

			LOG(Info, _T("AD Update is successful"));
		}
		else
		{
			strTemp = _T("[AMS]AD UPDATE ERROR");
			CDEV_Manager::GetInstance()->SaveEJL_SetTextValue(text_field, strTemp);

			LOG(Info, _T("AD Update is failed"));
		}
	}
	//else if (strSubCommand == _T("C"))
	//{
	//	if ( bReturn == TRUE )
	//	{
	//		strTemp = _T("[AMS]PRINT LOGO UPDATE");
	//		CDEV_Manager::GetInstance()->SaveEJL_SetTextValue(text_field, strTemp);

	//		// 변경된 Print Image를 PRT에 전송하는 로직 추가
	//		// Print Logo Image
	//		strTemp.Format(_T("%s%s"), CUtil::GetAppPath(), RECEIPT_LOGO_IMAGE);
	//		CT2A	szPath(strTemp);

	//		int nPrintError = CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_SetBitmapFlash(EAGLE_PRT_TYPE_USB, &szPath.m_psz, 1);

	//		if (EAGLE_PRT_SUCCESS != nPrintError)
	//		{
	//			LOG(Info, _T("[AMS] Download Bitmap To Flash is Failed - %d"), nPrintError);
	//		}
	//		else
	//		{
	//			LOG(Info, _T("[AMS] Download Bitmap To Flash is success - (%s)"), strTemp);
	//		}
	//	}
	//	else
	//	{
	//		strTemp = _T("[AMS]PRINT LOGO UPDATE ERROR");
	//		CDEV_Manager::GetInstance()->SaveEJL_SetTextValue(text_field, strTemp);

	//		LOG(Info, _T("Receipt Header Image Update is failed"));
	//	}
	//}

	return bReturn;
}


/** ********************************************************************
* @brief AMS로부터 원격으로 Cassette Close 명령 수행
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
BOOL CDEV_HOST::AMS_ProcCbxClose()
{
	int	nCSTCnt = 0, nCSTDenom = 0;
	int nLoadedCount = 0, nDispensedCount = 0, nCurrentCount = 0, nRejectedCount = 0;
	CString	strLoadedAmount, strDispensedAmount, strCurrentAmount;
	int i=0;

	SYSTEMTIME	st;
	GetLocalTime(&st);

	// [EJL]
	FLSEJCOMMINFO	CommInfo;
	FLSEJCSTCLINFO	CBXCloseInfo;

	memset(&CommInfo, 0, sizeof(CommInfo));
	memset(&CBXCloseInfo, 0, sizeof(CBXCloseInfo));

	////////////////////////////////////////////////////////
	// Common Info
	////////////////////////////////////////////////////////
	// 1. JNL NO

	// 2. LOG DATE
	sprintf(CommInfo.log_date, "%04d/%02d/%02d", st.wYear, st.wMonth, st.wDay);

	// 3. LOG TIME
	sprintf(CommInfo.log_time, "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);

	// 4. JNL CODE
	CommInfo.jnl_cd = cst_close;

	// 5. LOG SEARCH DATE
	sprintf(CommInfo.log_srch_date, "%04d%02d%02d%02d%02d%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);


	////////////////////////////////////////////////////////
	// CBX Close Info
	////////////////////////////////////////////////////////

	nCSTCnt = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Count);

	// 1. JNL NO

	// 2. TERMINAL ID
	sprintf(CBXCloseInfo.term_id, "%S", CEagleDataManager::GetInstance()->m_Config.m_Option.strTerminal_ID);

	// 3. start date
	sprintf(CBXCloseInfo.start_date, "%02d/%02d/%04d", st.wMonth, st.wDay, st.wYear);

	// 4. start time
	sprintf(CBXCloseInfo.start_time, "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);

	// 5. close type
	CBXCloseInfo.cst_close_type[0] = '1';
	
	for (i=0; i<nCSTCnt; i++)
	{
		// Denomination information
		nCSTDenom = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Denomination[i]); 

		nLoadedCount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Load_Count[i]);
		strLoadedAmount = CUtil::IntToString(nCSTDenom * nLoadedCount) + _T("00");

		nDispensedCount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Dispense_Count[i]);
		strDispensedAmount = CUtil::IntToString(nCSTDenom * nDispensedCount) + _T("00");

		nRejectedCount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Reject_Count[i]);

		nCurrentCount = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Item_Count[i]);
		strCurrentAmount = CUtil::IntToString(nCSTDenom * nCurrentCount) + _T("00");

		// [EJL]
		switch(i)
		{
		case 0:		// 1 CBX
			// 6. 1CBX LOAD BILL
			sprintf(CBXCloseInfo.load_bill_cst1, "%d", nLoadedCount);

			// 7. 1CBX DISP BILL
			sprintf(CBXCloseInfo.disp_bill_cst1, "%d", nDispensedCount);

			// 8. 1CBX REJ BILL
			sprintf(CBXCloseInfo.rej_bill_cst1, "%d", nRejectedCount);

			// 9. 1CBX REMAIN BILL
			sprintf(CBXCloseInfo.rem_bill_cst1, "%d", nCurrentCount);

			// 10. 1CBX LOAD AMOUNT
			sprintf(CBXCloseInfo.load_amt_cst1, "%S", strLoadedAmount);

			// 11. 1CBX DISP AMOUNT
			sprintf(CBXCloseInfo.disp_amt_cst1, "%S", strDispensedAmount);

			// 12. 1CBX REMAIN AMOUNT
			sprintf(CBXCloseInfo.rem_amt_cst1, "%S", strCurrentAmount);

			// 13. 1CBX DENOM
			sprintf(CBXCloseInfo.denom_cst1, "%d", nCSTDenom);		
			break;

		case 1:		// 2 CBX
			// 14. 2CBX LOAD BILL
			sprintf(CBXCloseInfo.load_bill_cst2, "%d", nLoadedCount);

			// 15. 2CBX DISP BILL
			sprintf(CBXCloseInfo.disp_bill_cst2, "%d", nDispensedCount);

			// 16. 2CBX REJ BILL
			sprintf(CBXCloseInfo.rej_bill_cst2, "%d", nRejectedCount);

			// 17. 2CBX REMAIN BILL
			sprintf(CBXCloseInfo.rem_bill_cst2, "%d", nCurrentCount);

			// 18. 2CBX LOAD AMOUNT
			sprintf(CBXCloseInfo.load_amt_cst2, "%S", strLoadedAmount);

			// 19. 2CBX DISP AMOUNT
			sprintf(CBXCloseInfo.disp_amt_cst2, "%S", strDispensedAmount);

			// 20. 2CBX REMAIN AMOUNT
			sprintf(CBXCloseInfo.rem_amt_cst2, "%S", strCurrentAmount);

			// 21. 2CBX DENOM
			sprintf(CBXCloseInfo.denom_cst2, "%d", nCSTDenom);	
			break;

		case 2:		// 3 CBX
			// 22. 3CBX LOAD BILL
			sprintf(CBXCloseInfo.load_bill_cst3, "%d", nLoadedCount);

			// 23. 3CBX DISP BILL
			sprintf(CBXCloseInfo.disp_bill_cst3, "%d", nDispensedCount);

			// 24. 3CBX REJ BILL
			sprintf(CBXCloseInfo.rej_bill_cst3, "%d", nRejectedCount);

			// 25. 3CBX REMAIN BILL
			sprintf(CBXCloseInfo.rem_bill_cst3, "%d", nCurrentCount);

			// 26. 3CBX LOAD AMOUNT
			sprintf(CBXCloseInfo.load_amt_cst3, "%S", strLoadedAmount);

			// 27. 3CBX DISP AMOUNT
			sprintf(CBXCloseInfo.disp_amt_cst3, "%S", strDispensedAmount);

			// 28. 3CBX REMAIN AMOUNT
			sprintf(CBXCloseInfo.rem_amt_cst3, "%S", strCurrentAmount);

			// 29. 3CBX DENOM
			sprintf(CBXCloseInfo.denom_cst3, "%d", nCSTDenom);			
			break;

		case 3:		// 4 CBX
			// 30. 4CBX LOAD BILL
			sprintf(CBXCloseInfo.load_bill_cst4, "%d", nLoadedCount);

			// 31. 4CBX DISP BILL
			sprintf(CBXCloseInfo.disp_bill_cst4, "%d", nDispensedCount);

			// 32. 4CBX REJ BILL
			sprintf(CBXCloseInfo.rej_bill_cst4, "%d", nRejectedCount);

			// 33. 4CBX REAMIN BILL
			sprintf(CBXCloseInfo.rem_bill_cst4, "%d", nCurrentCount);

			// 34. 4CBX LOAD AMOUNT
			sprintf(CBXCloseInfo.load_amt_cst4, "%S", strLoadedAmount);

			// 35. 4CBX DISP AMOUNT
			sprintf(CBXCloseInfo.disp_amt_cst4, "%S", strDispensedAmount);

			// 36. 4CBX REMAIN AMOUNT
			sprintf(CBXCloseInfo.rem_amt_cst4, "%S", strCurrentAmount);

			// 37. 4CBX DENOM
			sprintf(CBXCloseInfo.denom_cst4, "%d", nCSTDenom);			
			break;
		}
	}

	// 38. Print 여부

	// 39. AMS Upload

	// 40. Host Upload

	// [EJL] SAVE
	LOG(Info, _T("Save CBX Close start"));

 	BOOL bResult = FALSE;
	bResult = CDEV_Manager::GetInstance()->m_DEV_EJL.InsertJnlData(&CommInfo, &CBXCloseInfo);

	LOG(Info, _T("Save CBX Close Reuslt : %d"), bResult);

	// 매수 Clear
	CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.Clear_CBX_Count();
	CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.Clear_Reject_Info();
	CEagleDataManager::GetInstance()->m_Config.SaveCbxInfo();

	LOG(Info, _T("Remote CBX Count is Cleared"));

	return TRUE;
}


/** ********************************************************************
* @brief Printer Image Upload
* @param BYTE *pbyData		AMS로부터 받은 Upload data
* @param int nRecvLen		AMS로부터 받은 data length
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
// BOOL CDEV_HOST::AMS_ProcImageUpload(BYTE *pbyData, int nRecvLen)
// {
// 	PAMS_CMD_IMAGE_UPLOAD pUploadMsg = (PAMS_CMD_IMAGE_UPLOAD)pbyData;
// 	BYTE *pbyRecvData = NULL;
// 	int nRecvLength = 0;
// 	int i = 0, nResult = 0;
// 	int nFileDataLen = 0;
// 	DWORD dwWriten = 0;
// 	HANDLE hFile = INVALID_HANDLE_VALUE;
// 	CString strFileName, strDir, strFilePath;
// 	BYTE byDataAck = DATA_ACK;
// 	int nIteration = 0;
// 	CString strData = _T("");
// 	BOOL bReturn = FALSE;
// 
// 	MultiToWideEx((char*)pUploadMsg->byFileName, strFileName, sizeof(pUploadMsg->byFileName), FALSE);
// 
// 	//*LEH
// 	strDir.Format(_T("%s%s"), CUtil::GetAppPath(), PRINT_IMAGE_PATH);
// 	CreateDirectory(strDir, NULL);
// 
// 	strFilePath.Format(_T("%s\\%s"), strDir, strFileName);
// 	/////////////////////////////////////////////////////////
// 
// 	LOG(Info, _T("AMS_ProcImageUpload() Image Upload path :[%s]"), strFilePath);
// 
// 	// try twice.
// 	for (i = 0; i < 2; i++)
// 	{
// 		// open file
// 		hFile = CreateFile (strFilePath,				// Open NVRAM.TXT.
// 			GENERIC_WRITE,          // Open for writing
// 			0,                      // Do not share
// 			NULL,                   // No security
// 			CREATE_ALWAYS,          // Open or create
// 			FILE_ATTRIBUTE_NORMAL,  // Normal file
// 			NULL);                  // No template file
// 		
// 		if (hFile != INVALID_HANDLE_VALUE)
// 			break;
// 	}
// 
// 	if ( (INVALID_HANDLE_VALUE == hFile) || (NULL == hFile)	)
// 	{
// 		LOG(Error, _T("AMS_ProcImageUpload() CreateFile is failed."));
// 		return FALSE;
// 	}
// 
// 	pbyRecvData = new BYTE[NET_RECV_BUFF_SIZE];
// 	memset(pbyRecvData, 0, NET_RECV_BUFF_SIZE);
// 
// 	MultiToWideEx((char*)pUploadMsg->byIteration, strData, sizeof(pUploadMsg->byIteration), TRUE);
// 	nIteration = CUtil::StringToInt(strData);
// 
// 	LOG(Info, _T("AMS_ProcImageUpload() Iteration : [%d]"), pUploadMsg->byIteration);
// 
// 	nRecvLength = nRecvLen;
// 
// 	while(1)
// 	{
// 		dwWriten = 0;
// 
// 		nFileDataLen = nRecvLength - sizeof(AMS_HEADER) - sizeof(pUploadMsg->byFileName) - sizeof(pUploadMsg->byIteration);
// 
// 		if (!WriteFile(hFile, pUploadMsg->byFileData, nFileDataLen, &dwWriten, NULL))
// 		{
// 			LOG(Error, _T("AMS_ProcImageUpload()File Write Failed."));
// 
// 			bReturn = FALSE;
// 			break;
// 		}
// 
// 		// ACK 전송
// 		nResult = HOST_Sync_Send(&byDataAck, 1, _T(""), _T(""), _T(""), TRUE);
// 
// 		if(nResult != AMS_OK)
// 		{
// 			LOG(Error, _T("AMS_ProcImageUpload() Send ACK Failed."));
// 			
// 			bReturn = FALSE;
// 			break;
// 		}
// 
// 		if(0 == nIteration)
// 		{
// 			LOG(Info, _T("AMS_ProcImageUpload() Image Upload Success."));
// 			
// 			bReturn = TRUE;
// 			break;
// 		}
// 		else
// 		{
// 			// Recv CMD
// 			memset(pbyRecvData, 0, NET_RECV_BUFF_SIZE);
// 			nRecvLength = 0;
// 
// 			nRecvLength = HOST_Sync_Recv(pbyRecvData, TRUE);
// 
// 			if(0 == nRecvLength)
// 			{
// 				LOG(Error, _T("AMS_ProcImageUpload() RecvSize is zero!!!"));
// 				
// 				bReturn = FALSE;
// 				break;
// 			}
// 
// 			pUploadMsg = (PAMS_CMD_IMAGE_UPLOAD)pbyRecvData;
// 
// 			MultiToWideEx((char*)pUploadMsg->byIteration, strData, sizeof(pUploadMsg->byIteration), TRUE);
// 			nIteration = CUtil::StringToInt(strData);
// 
// 			LOG(Error, _T("AMS_ProcImageUpload() RecvLength:[%d], byRecvData:[%S]"), nRecvLength, pbyRecvData);	//*LEHTEST
// 		}
// 	}
// 
// 	if ( (hFile != INVALID_HANDLE_VALUE) && (hFile != NULL)	)
// 		CloseHandle(hFile);
// 
// 	if (pbyRecvData != NULL)
// 	{
// 		delete [] pbyRecvData;
// 		pbyRecvData = NULL;
// 	}
// 
// 	return bReturn;
// }


/** ********************************************************************
* @brief AMS로 전송할 Log Data
* @param CString strCommand		Command
* @param CString strSubCommand	Sub Command
* @param CString strDate		조회조건 날짜
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
BOOL CDEV_HOST::AMS_ProcLog(CString strCommand, CString strSubCommand, CString strDate)
{
	CFile datFile;
	CFileException e;
	CString strLogPath;
	CString strSearchFile;
	CString strZipFileName, strZipFilePath, strTerminalID;
	CString strSrcFileName, strDestFileName;

	WIN32_FIND_DATA	FileData;
	HANDLE hSearch = INVALID_HANDLE_VALUE;
	BOOL bResult = FALSE;
	BOOL bResult2 = FALSE;
	BOOL bFileFind = FALSE;

	CreateDirectory(PATH_TEMP_ABS, NULL);

	strLogPath.Format(_T("%s%s"), CUtil::GetAppPath(), PATH_LOG);

	CUtil::DeleteFileInDirectory(PATH_TEMP_ABS, _T("*.*"));	// ?????? ??????? File ????

	LOG(Info, _T("Recv Get Log Commmand - Before Date : %s"), strDate);

	strDate.Replace(_T("-"), _T(""));		// 서버에서 YYYY-MM-DD Format으로 내려옴

	LOG(Info, _T("Recv Get Log Commmand - After Date : %s"), strDate);

	// Current File은 무조건 Backup하도록 함. -> Temp Folder로 이동
	AMS_Backup_LogFile(PATH_TEMP_ABS);	// Current Log?? TempTrace ?????? ??????? Copy

	bFileFind = CUtil::IsExistFile(PATH_TEMP_ABS);

	if(TRUE == bFileFind)
	{
		// Zip File Name에 Terminal ID 추가하도록 로직 변경
		strTerminalID = CEagleDataManager::GetInstance()->m_Config.m_Option.strTerminal_ID;
		strTerminalID.TrimLeft();
		strTerminalID.TrimRight();

		if (strTerminalID.GetLength() <= 0)
			strTerminalID = _T("DEFAULT");

		// 장애 Backpup한 File을 찾아서 Temp 폴더로 Copy
		strSearchFile.Format(_T("%s\\*%s*.zip"), strLogPath, strDate);
		//////////////////////////////////////////////////////////////////

		LOG(Info, _T("AMS_ProcLog() strSearchFile : [%s]"), strSearchFile);

		hSearch = FindFirstFile(strSearchFile, &FileData);

		if (hSearch != INVALID_HANDLE_VALUE)
		{
			//WCHAR	strSrcFile[255];
			int nFileCount = 0;

			while(TRUE)
			{
				nFileCount++;
				strSrcFileName.Format(_T("%s\\%s"), strLogPath, FileData.cFileName);
				strDestFileName.Format(_T("%s\\%s"), PATH_TEMP_ABS, FileData.cFileName);
				::CopyFile(strSrcFileName, strDestFileName, FALSE);

				if (FindNextFile(hSearch, &FileData) == FALSE)
					break;
			}

			FindClose(hSearch);

			LOG(Info, _T("AMS_ProcLog() File Log File Count (%d)"), nFileCount)

			// Zip File Name에 Terminal ID 추가하도록 로직 변경
			strZipFileName.Format(_T("LOG_%s_%s.zip"), strTerminalID, strDate);
			strZipFilePath.Format(_T("%s\\%s"), PATH_TEMP_ABS, strZipFileName);
			bResult2 = CUtil::ZippingFromFolder(PATH_TEMP_ABS, strZipFilePath);
			LOG(Info, _T("Backup log file result [%d]"), bResult2);
		}
		else
		{
			// Backup File에 현재 날짜가 없는 경우 Temp Folder에 있는 경우에 대한 처리 추가
			hSearch = NULL;

			strSearchFile.Format(_T("%s\\*%s*.zip"), PATH_TEMP_ABS, strDate);
			//////////////////////////////////////////////////////////////////

			LOG(Info, _T("AMS_ProcLog() TempFolder strSearchFile : [%s]"), strSearchFile);

			hSearch = FindFirstFile(strSearchFile, &FileData);

			if (hSearch != INVALID_HANDLE_VALUE)
			{
				FindClose(hSearch);

				strZipFileName.Format(_T("LOG_%s_%s.zip"), strTerminalID, strDate);
				strZipFilePath.Format(_T("%s\\%s"), PATH_TEMP_ABS, strZipFileName);
				bResult2 = CUtil::ZippingFromFolder(PATH_TEMP_ABS, strZipFilePath);
				LOG(Info, _T("Backup log file result 2 [%d]"), bResult2);
			}
			else
			{
				LOG(Info, _T("AMS_ProcLog() FindFile is failed."));
			}
		}

		if(datFile.Open(strZipFilePath, CFile::modeRead, &e))
		{
			UINT nSize = (UINT) datFile.GetLength();
			BYTE *pBuffer = NULL;
			
			if(nSize > 0)
			{
				pBuffer = new BYTE[nSize+1];

				memset(pBuffer, 0, nSize+1);

				if(datFile.Read(pBuffer, nSize) == nSize)
				{
					bResult = AMS_SendATMLogMsg(strCommand, strSubCommand, pBuffer, nSize, strZipFileName);
				}
				else
				{
					LOG(Error, _T("AMS_ProcLog() LogFile read is failed."));
					bResult = AMS_SendATMLogMsg(strCommand, strSubCommand, NULL, 0, _T(""));
				}

				if(pBuffer != NULL)
					delete [] pBuffer;

				datFile.Close();
			}
			else
			{
				LOG(Error, _T("AMS_ProcLog() File size is zero"));
				bResult = AMS_SendATMLogMsg(strCommand, strSubCommand, NULL, 0, _T(""));	// Zip File Read 오류
			}
		}
		else
		{
			LOG(Error, _T("AMS_ProcLog() LogFile open is failed."));
			bResult = AMS_SendATMLogMsg(strCommand, strSubCommand, NULL, 0, _T(""));
		}

		// 임시 파일 삭제
		CUtil::DeleteFileInDirectory(PATH_TEMP_ABS, _T("*.*"));
	}
	else
	{
		LOG(Error, _T("AMS_ProcLog() Trace directory is not exist files."));

		bResult = AMS_SendATMLogMsg(strCommand, strSubCommand, NULL, 0, _T(""));
	}

	return bResult;
}


/** ********************************************************************
* @brief AMS로 전송할 Log Data
* @param CString strCommand		Command
* @param CString strSubCommand	Sub Command
* @param BYTE *pbyFileData	File Data
* @param int nFileLen		File Size
* @param LPCTSTR wsFileName	File Name
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
BOOL CDEV_HOST::AMS_SendATMLogMsg(CString strCommand, CString strSubCommand, BYTE *pbyFileData, int nFileLen, CString strFileName)
{
	AMS_RES_LOG logMsg;
	BYTE byTempData[1024*4] = {0,};		// Base64 적용에 따라 Buffer Size 조정
	char byBase64_Encoded[1024 * 4] = { 0, };
	BYTE byLength[10] = {0,};
	int nFileDataLen = 0;
	int nRet = 0;
	int nIteration = 0;
	UINT nFileDataIndex = 0;
	BOOL bResult = FALSE;
	CString strSendData;
	CString strTemp;
	int nSendLen = 0;
	int nBase64EncodedLen = 0;
	int nFileDataSize = 0;

	if (CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType == _T("TCP/IP"))
		nFileDataSize = FILE_SIZE_1K;
	else
		nFileDataSize = FILE_SIZE_512;

	memset(m_pAMSSendBuffer, 0, NET_SEND_BUFF_SIZE);

	nFileDataLen = nFileLen;

	nIteration = nFileLen / nFileDataSize;

	if( 0 == (nFileLen % nFileDataSize) )
		nIteration -= 1;

	// 보정처리 - nIteration이 0보다 적은 경우에는 0으로 처리
	if (nIteration < 0)
		nIteration = 0;

	LOG(Info, _T("AMS_SendATMLogMsg() Send AMS_LOG_FILE_UPLOAD. Interation[%d] RemainFileSize[%d]"), nIteration, nFileDataLen);

	if (nFileDataLen <= 0)
	{
		// File이 없는 경우에도 Data Response 송부 후에 종료 처리 (AMS Server와 협의됨)
		AMS_MakeHeaderField(strCommand, strSubCommand, strSendData);

		// FileName -> Null로 송신
		strSendData += CString(FS);

		// Iteration -> 0으로 처리
		strTemp = _T("0");
		strSendData += strTemp;
		strSendData += CString(FS);

		// File Data -> Null
		strSendData += CString(FS);

		nSendLen = strSendData.GetLength();
		WideToMulti(strSendData, (char*)byTempData, nSendLen);

		// Length
		WideToMulti(CUtil::IntToStringEx(nSendLen, LEN_LENGTH), (char*)byLength, LEN_LENGTH);
		m_pAMSSendBuffer[0] = (BYTE)FS;
		memcpy(&m_pAMSSendBuffer[1], byLength, LEN_LENGTH);

		memcpy(&m_pAMSSendBuffer[LEN_LENGTH+1], byTempData, nSendLen);

		nRet = HOST_Sync_Send((BYTE*)m_pAMSSendBuffer, nSendLen + LEN_LENGTH + 1, _T(""), _T(""), _T(""), TRUE);

		if(AMS_OK != nRet)
		{
			LOG(Error, _T("AMS_SendATMLogMsg() Send Log Failed."));
			bResult = FALSE;
		}

		goto cleanup;
	}

	// File Data Upload Start
	while(nFileDataLen > 0)
	{
		// Make Upload File Data
		memset(&logMsg, 0, sizeof(AMS_RES_LOG));
		memset(byTempData, 0, sizeof(byTempData));
		memset(byBase64_Encoded, 0, sizeof(byBase64_Encoded));
		strSendData.Empty();
		nSendLen = 0;
		nBase64EncodedLen = 0;
		
		// Make Upload Common Header Information
		AMS_MakeHeaderField(strCommand, strSubCommand, strSendData);

 		WideToMulti(strFileName, (char*)logMsg.byFileName, sizeof(logMsg.byFileName) - 1);
		WideToMulti(CUtil::IntToString(nIteration), (char*)logMsg.byIteration, sizeof(logMsg.byIteration) - 1);
		
		// FileName
		strTemp.Format(_T("%S"), logMsg.byFileName);
		strSendData += strTemp;
		strSendData += CString(FS);

		// Iteration
		strTemp.Format(_T("%S"), logMsg.byIteration);
		strSendData += strTemp;
		strSendData += CString(FS);

		nSendLen = strSendData.GetLength();
		WideToMulti(strSendData, (char*)byTempData, nSendLen);

		if (nFileDataLen > nFileDataSize)
		{
			// Base64로 Encoded 해서 File 저장
			nBase64EncodedLen = CUtil::Base64_Encoded((char*)&pbyFileData[nFileDataIndex], nFileDataSize, byBase64_Encoded);
			memcpy(&byTempData[nSendLen], byBase64_Encoded, nBase64EncodedLen);

			nSendLen += nBase64EncodedLen;

			nFileDataIndex = nFileDataIndex + nFileDataSize;
			nFileDataLen = nFileDataLen - nFileDataSize;
		}
		else
		{
			nBase64EncodedLen = CUtil::Base64_Encoded((char*)&pbyFileData[nFileDataIndex], nFileDataLen, byBase64_Encoded);
			memcpy(&byTempData[nSendLen], byBase64_Encoded, nBase64EncodedLen);
		
			nSendLen += nBase64EncodedLen;

			nFileDataIndex = nFileDataIndex + nFileDataLen;
			nFileDataLen = 0;
		}

		byTempData[nSendLen] = (BYTE)FS;
		nSendLen += 1;

		// Length
		WideToMulti(CUtil::IntToStringEx(nSendLen, LEN_LENGTH), (char*)byLength, LEN_LENGTH);
		m_pAMSSendBuffer[0] = (BYTE)FS;
		memcpy(&m_pAMSSendBuffer[1], byLength, LEN_LENGTH);

		// File Data
		memcpy(&m_pAMSSendBuffer[LEN_LENGTH+1], byTempData, nSendLen);

		// +1 => 0x1C
		nRet = HOST_Sync_Send((BYTE*)m_pAMSSendBuffer, nSendLen + LEN_LENGTH + 1, _T(""), _T(""), _T(""), TRUE);

		if(AMS_OK != nRet)
		{
			LOG(Error, _T("AMS_SendATMLogMsg() Send Log Failed."));
			bResult = FALSE;
			break;
		}

		nIteration--;
	}

cleanup:

	if(AMS_OK == nRet)
	{
		bResult = TRUE;
		LOG(Info, _T("AMS_SendATMLogMsg() Send Log SUCCESS"));
	}

	return bResult;
}


/** ********************************************************************
* @brief AMS로 전송할 Log Data
* @param CString strCommand		Command
* @param CString strSubCommand	Sub Command
* @param CString strDate		Camera Image File Name
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
BOOL CDEV_HOST::AMS_ProcCAMImageUpload(CString strCommand, CString strSubCommand, CString strImgFileName)
{
	CFile datFile;
	CFileException e;
	CString strSearchImgFile, strDestFile;
	CString strZipFilePathName, strZipFileName, strTerminalID;
	BOOL bResult = FALSE;

	// \\Temp Folder Delete
	::CreateDirectory(PATH_TEMP_ABS, NULL);
	CUtil::DeleteFileInDirectory(PATH_TEMP_ABS, _T("*.*"));

	LOG(Info, _T("Recv Get CAM Image Commmand - File Name : %s"), strImgFileName);
	strSearchImgFile.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), CAPTURE_IMAGE_FILE_PATH, strImgFileName);

	bResult = CUtil::IsExistFile(strSearchImgFile);

	if(TRUE == bResult)
	{
		//////////////////////////////////////////////////////////////////

		LOG(Info, _T("AMS_ProcCAMImageUpload() strSearchImageFile : [%s]"), strSearchImgFile);

		// Zip File Name에 Terminal ID 추가하도록 로직 변경
		strTerminalID = CEagleDataManager::GetInstance()->m_Config.m_Option.strTerminal_ID;
		strTerminalID.TrimLeft();
		strTerminalID.TrimRight();

		if (strTerminalID.GetLength() <= 0)
			strTerminalID = _T("DEFAULT");

		strDestFile.Format(_T("%s\\%s"), PATH_TEMP_ABS, strImgFileName);
		bResult = CopyFile(strSearchImgFile, strDestFile, FALSE);

		if (bResult == TRUE)
		{
			// File Copy 성공시 Zipping해서 AMS로 송신
			// Zip File Name에 Terminal ID 추가
			// File Name에 ".jpg" File 제거
			strZipFileName.Format(_T("%s_%s.zip"), strImgFileName.Left(strImgFileName.GetLength() - 4), strTerminalID);
			strZipFilePathName.Format(_T("%s\\%s"), PATH_TEMP_ABS, strZipFileName);
			bResult = CUtil::ZippingFromFolder(PATH_TEMP_ABS, strZipFilePathName);
			LOG(Info, _T("Camera Image Zipping result [%d]"), bResult);

			if(datFile.Open(strZipFilePathName, CFile::modeRead, &e))
			{
				UINT nSize = (UINT) datFile.GetLength();
				BYTE *pBuffer = NULL;

				if(nSize > 0)
				{
					pBuffer = new BYTE[nSize+1];

					memset(pBuffer, 0, nSize+1);

					if(datFile.Read(pBuffer, nSize) == nSize)
					{
						bResult = AMS_SendATMCAMImageMsg(strCommand, strSubCommand, pBuffer, nSize, strZipFileName);
					}
					else
					{
						LOG(Error, _T("AMS_ProcCAMImageUpload() LogFile read is failed."));
						bResult = AMS_SendATMCAMImageMsg(strCommand, strSubCommand, NULL, 0, _T(""));
					}

					if(pBuffer != NULL)
						delete [] pBuffer;

					datFile.Close();
				}
				else
				{
					LOG(Error, _T("AMS_ProcCAMImageUpload() File size is zero"));
					bResult = AMS_SendATMCAMImageMsg(strCommand, strSubCommand, NULL, 0, _T(""));
				}
			}
			else
			{
				LOG(Error, _T("AMS_ProcCAMImageUpload() LogFile open is failed."));
				bResult = AMS_SendATMCAMImageMsg(strCommand, strSubCommand, NULL, 0, _T(""));
			}
		}

		// 임시 파일 삭제
		CUtil::DeleteFileInDirectory(PATH_TEMP_ABS, _T("*.*"));
	}
	else
	{
		LOG(Error, _T("AMS_ProcCAMImageUpload() Camera Image File is not exist."));
		bResult = AMS_SendATMCAMImageMsg(strCommand, strSubCommand, NULL, 0, _T(""));
	}

	return bResult;
}


/** ********************************************************************
* @brief AMS로 전송할 Camera Image Data
* @param CString strCommand		Command
* @param CString strSubCommand	Sub Command
* @param BYTE *pbyFileData	File Data
* @param int nFileLen		File Size
* @param LPCTSTR wsFileName	File Name
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
BOOL CDEV_HOST::AMS_SendATMCAMImageMsg(CString strCommand, CString strSubCommand, BYTE *pbyFileData, int nFileLen, CString strFileName)
{
	AMS_RES_LOG logMsg;
	BYTE byTempData[NET_SEND_BUFF_SIZE] = {0,};
	char byBase64_Encoded[NET_SEND_BUFF_SIZE] = { 0, };
	BYTE byLength[10] = {0,};
	int nFileDataLen = 0;
	int nRet = 0;
	int nIteration = 0;
	UINT nFileDataIndex = 0;
	BOOL bResult = FALSE;
	CString strSendData;
	CString strTemp;
	int nSendLen = 0;
	int nBase64EncodedLen = 0;
	int nFileDataSize = 0;

	if (CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType == _T("TCP/IP"))
		nFileDataSize = FILE_SIZE_1K;
	else
		nFileDataSize = FILE_SIZE_512;

	memset(m_pAMSSendBuffer, 0, NET_SEND_BUFF_SIZE);

	nFileDataLen = nFileLen;

	nIteration = nFileLen / nFileDataSize;

	if( 0 == (nFileLen % nFileDataSize) )
		nIteration -= 1;

	// 보정처리 - nIteration이 0보다 적은 경우에는 0으로 처리
	if (nIteration < 0)
		nIteration = 0;

	LOG(Info, _T("AMS_SendATMCAMImageMsg() Send - Interation[%d] RemainFileSize[%d]"), nIteration, nFileDataLen);

	if (nFileDataLen <= 0)
	{
		// File이 없는 경우에도 Data Response 송부 후에 종료 처리 (AMS Server와 협의됨)
		AMS_MakeHeaderField(strCommand, strSubCommand, strSendData);

		// FileName -> Null로 송신
		strSendData += CString(FS);

		// Iteration -> 0으로 처리
		strTemp = _T("0");
		strSendData += strTemp;
		strSendData += CString(FS);

		// File Data -> Null
		strSendData += CString(FS);

		nSendLen = strSendData.GetLength();
		WideToMulti(strSendData, (char*)byTempData, nSendLen);

		// Length
		WideToMulti(CUtil::IntToStringEx(nSendLen, LEN_LENGTH), (char*)byLength, LEN_LENGTH);
		m_pAMSSendBuffer[0] = (BYTE)FS;
		memcpy(&m_pAMSSendBuffer[1], byLength, LEN_LENGTH);

		memcpy(&m_pAMSSendBuffer[LEN_LENGTH+1], byTempData, nSendLen);

		nRet = HOST_Sync_Send((BYTE*)m_pAMSSendBuffer, nSendLen + LEN_LENGTH + 1, _T(""), _T(""), _T(""), TRUE);

		if(AMS_OK != nRet)
		{
			LOG(Error, _T("AMS_SendATMCAMImageMsg() Send Log Failed."));
			bResult = FALSE;
		}

		goto cleanup;
	}

	// File Data Upload Start
	while(nFileDataLen > 0)
	{
		// Make Upload File Data
		memset(&logMsg, 0, sizeof(AMS_RES_LOG));
		memset(byTempData, 0, sizeof(byTempData));
		memset(byBase64_Encoded, 0, sizeof(byBase64_Encoded));
		strSendData.Empty();
		nSendLen = 0;
		nBase64EncodedLen = 0;

		// Make Upload Common Header Information
		AMS_MakeHeaderField(strCommand, strSubCommand, strSendData);

		WideToMulti(strFileName, (char*)logMsg.byFileName, sizeof(logMsg.byFileName) - 1);
		WideToMulti(CUtil::IntToString(nIteration), (char*)logMsg.byIteration, sizeof(logMsg.byIteration) - 1);

		// FileName
		strTemp.Format(_T("%S"), logMsg.byFileName);
		strSendData += strTemp;
		strSendData += CString(FS);

		// Iteration
		strTemp.Format(_T("%S"), logMsg.byIteration);
		strSendData += strTemp;
		strSendData += CString(FS);

		nSendLen = strSendData.GetLength();
		WideToMulti(strSendData, (char*)byTempData, nSendLen);

		if (nFileDataLen > nFileDataSize)
		{
			// Base64로 Encoded 해서 File 저장
			nBase64EncodedLen = CUtil::Base64_Encoded((char*)&pbyFileData[nFileDataIndex], nFileDataSize, byBase64_Encoded);
			memcpy(&byTempData[nSendLen], byBase64_Encoded, nBase64EncodedLen);

			nSendLen += nBase64EncodedLen;

			nFileDataIndex = nFileDataIndex + nFileDataSize;
			nFileDataLen = nFileDataLen - nFileDataSize;
		}
		else
		{
			nBase64EncodedLen = CUtil::Base64_Encoded((char*)&pbyFileData[nFileDataIndex], nFileDataLen, byBase64_Encoded);
			memcpy(&byTempData[nSendLen], byBase64_Encoded, nBase64EncodedLen);

			nSendLen += nBase64EncodedLen;

			nFileDataIndex = nFileDataIndex + nFileDataLen;
			nFileDataLen = 0;
		}

		byTempData[nSendLen] = (BYTE)FS;
		nSendLen += 1;

		// Length
		WideToMulti(CUtil::IntToStringEx(nSendLen, LEN_LENGTH), (char*)byLength, LEN_LENGTH);
		m_pAMSSendBuffer[0] = (BYTE)FS;
		memcpy(&m_pAMSSendBuffer[1], byLength, LEN_LENGTH);

		// File Data
		memcpy(&m_pAMSSendBuffer[LEN_LENGTH+1], byTempData, nSendLen);

		// +1 => 0x1C
		nRet = HOST_Sync_Send((BYTE*)m_pAMSSendBuffer, nSendLen + LEN_LENGTH + 1, _T(""), _T(""), _T(""), TRUE);

		if(AMS_OK != nRet)
		{
			LOG(Error, _T("AMS_SendATMCAMImageMsg() Send Log Failed."));
			bResult = FALSE;
			break;
		}

		nIteration--;
	}

cleanup:

	if(AMS_OK == nRet)
	{
		bResult = TRUE;
		LOG(Info, _T("AMS_SendATMCAMImageMsg() Send Image SUCCESS"));
	}

	return bResult;
}


/** ********************************************************************
* @brief AMS로 추가 명령이 존재하는지 조회하는 command
* @param CString strCommand		Command
* @param CString strSubCommand	SubCommand
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
BOOL CDEV_HOST::AMS_SendATMInquiryAddCmd(CString strCommand, CString strSubCommand)
{
	int nSendLen = 0;
	int nRet = 0;

	memset(m_pAMSSendBuffer, 0, NET_SEND_BUFF_SIZE);

	nSendLen = AMS_MakeStatusMsg(strCommand, strSubCommand, NET_SEND_BUFF_SIZE, m_pAMSSendBuffer);

	LOG(Info, _T("AMS_SendATMStatus() Send Status Msg To AMS.."));

	// +1 => 0x1C
	nRet = HOST_Sync_Send((BYTE*)m_pAMSSendBuffer, nSendLen + LEN_LENGTH + 1, _T(""), _T(""), _T(""), TRUE);

	if(AMS_OK != nRet)
	{
		LOG(Error, _T("AMS_SendATMInquiryAddCmd() Send Inquiry Additional Command Failed."));
		return FALSE;
	}
	else
	{
		LOG(Info, _T("AMS_SendATMInquiryAddCmd() Send Inquiry Additional Command SUCCESS"));
	}

	return TRUE;
}
