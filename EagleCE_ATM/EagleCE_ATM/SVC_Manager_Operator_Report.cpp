#include "stdafx.h"
#include "SVC_Manager.h"

#include "EagleSVCLib.h"

#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"
#include "../../EagleCE_Framework/EagleCE_Framework/Util.h"
#include "../../EagleCE_Screen/EagleCE_Screen/SCR_Manager.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif


/** **********************************************************
*	@brief		Report Main
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_Report_Main()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	CString strScreenInform;
	CString strTemp;
	CStringArray	strarrVersionInfo;

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (bShowScreen == TRUE)
		{
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();
			strScreenInform.Empty();
			strarrVersionInfo.RemoveAll();

			strTemp = Get_SW_VersionInfo();

			CUtil::ParsingStringToStringArray(strTemp, AP_DELEMITER, strarrVersionInfo);

			for(int i=0; i<strarrVersionInfo.GetCount(); i++)
			{
				strScreenInform += strarrVersionInfo[i];
				strScreenInform += SCR_CMD_DELIMITER;
			}

			Operator_DisplayScreen(OP_SCREEN_REPORT, bKeyEnable, strScreenInform);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_Report_Main - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER || m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}

			// Next Action
			if (m_strScrOutData == _T("ERRORCODE"))
			{
				// RIGHT 1
				m_strNextScrNum = OP_SCREEN_ERRORCODE;
				break;

			}
			else if (m_strScrOutData == _T("PRINTPARAMETER"))
			{
				if (CEagleSVCLib::GetInstance()->IsPTR_Available() == TRUE)
				{
					// RIGHT 1
					if (Operator_Confirm_Screen(_T("PRINT PARAMETER")) == TRUE)
					{
						Operator_Processing_Screen(CSCR_Manager::GetInstance()->GetAPTextIDString(OP_GUIDE_COMMON_WAIT));

						if (Print_ATM_Parameters() == TRUE)
						{
							Operator_Processing_Screen(_T("PRINT SUCCESS"));
						}
						else
						{
							Operator_Processing_Screen(_T("PRINT ERROR"));
						}


						CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);					
					}
				}
				else
				{
					Operator_Processing_Screen(_T("PRINT ERROR"));
					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
				}
				
				bShowScreen = TRUE;			
			}
			else
			{
				// 비정상적 값인 경우에는 OP MAIN으로 이동
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
		}

		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}
}


/** **********************************************************
*	@brief		Report Main
*	@retval		성공 : TRUE, 실패 : 그 외 값
************************************************************/
BOOL CSVC_Manager::Print_ATM_Parameters()
{
	CStringA	szTemp = "";
	CString		strTemp, strTemp2;
	CString		strNetworkType;

	int i = 0, nPrintError = 0;

	SYSTEMTIME st;
	GetLocalTime(&st);

	strNetworkType = CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType;

	// PTR Quiry Status Stop
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_QuiryStatusStop();

	// 프린터 램프 ON
	CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_1, DIO_CMD_FLICKING);

	CEagleSVCLib::GetInstance()->Set_PTR_Config();

	int nOrgX = 40;

				 //1234567890123456789012345678901234567890	- 40 line
	szTemp.Format("----------------------------------------");
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	// 1. Date / Time
	szTemp.Format("DATE / TIME : %02d/%02d/%04d %02d:%02d:%02d", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	// 2. Terminal ID
	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Option.strTerminal_ID;
	strTemp.TrimRight();

	if (strTemp.IsEmpty())
	{
		szTemp.Format("TERMINAL ID : (EMPTY)");
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Info, _T("PRT - %S"), szTemp);
	}
	else
	{
		//szTemp.Format("TERMINAL ID : %S", strTemp);
		szTemp = "TERMINAL ID : ";
		szTemp += strTemp;
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);

		szTemp.Replace("%", "%%");
		LOG(Info, _T("PRT - %S"), szTemp);
	}

	// 3. Serial Number
	szTemp.Format("SERIAL NO. : %S", CEagleDataManager::GetInstance()->m_Config.m_Option.strMachine_Serial_Number);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	// 4. Network Type
#if (NETWORK_OFFLINE_MODE)
	szTemp.Format("NETWORK TYPE : OFFLINE");
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);
#else
	szTemp.Format("NETWORK TYPE : %S", strNetworkType);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	if (strNetworkType == _T("TCP/IP"))
	{
		// TLS Option
		szTemp.Format("TLS : %S", GET_ENDISABLE_STRING(CEagleDataManager::GetInstance()->m_Config.m_Host.strUseTLS));
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Info, _T("PRT - %S"), szTemp);
	}

	// 5. PROTOCOL
	szTemp.Format("PROTOCOL : %S", CEagleDataManager::GetInstance()->m_Config.m_Host.strProtocolType);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	// 6. Protocol Option
	if (CEagleDataManager::GetInstance()->m_Config.m_Host.strProtocolType == _T("VISA"))
	{
		szTemp.Format("  ENQ : %S", GET_REQUIREDORNOT_STRING(CEagleDataManager::GetInstance()->m_Config.m_Host.strUseENQ));
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Info, _T("PRT - %S"), szTemp);

		szTemp.Format("  EOT : %S", GET_OPTION_STRING(CEagleDataManager::GetInstance()->m_Config.m_Host.strUseEOT));
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Info, _T("PRT - %S"), szTemp);
	}

	// 7. CRC
	szTemp.Format("CRC : %S", GET_ENDISABLE_STRING(CEagleDataManager::GetInstance()->m_Config.m_Host.strCrc_flg));
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

#endif

	szTemp.Format("----------------------------------------");
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);


	int nTitleIndex = 1;

	szTemp.Format("%d. VERSION", nTitleIndex++);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);


	CStringArray strarrVersionInfo;

	strarrVersionInfo.RemoveAll();
	strTemp.Empty();

	strTemp = Get_SW_VersionInfo();

	CUtil::ParsingStringToStringArray(strTemp, AP_DELEMITER, strarrVersionInfo);

	//for(int i=0; i<(strarrVersionInfo.GetCount() -1); i++)
	for(int i=0; i<(strarrVersionInfo.GetCount()); i++)
	{
		switch(i)
		{
		case 0:
			// 1. Software Main Version
			szTemp.Format("MAIN : %S", strarrVersionInfo[i]);
			break;

			// OS Version
		case 1:
			szTemp.Format("OS   : %S", strarrVersionInfo[i]);
			break;

		case 2:
			// 2. CDM Firmware Version
			szTemp.Format("[FW] CASH DISPENSER : %S", strarrVersionInfo[i]);			
			break;

		case 3:
			// 3. IDC Firmware Version
			szTemp.Format("[FW] CARD READER : %S", strarrVersionInfo[i]);			
			break;
		
		case 4:
			// 4. EPP Firmware Version
			szTemp.Format("[FW] PINPAD : %S", strarrVersionInfo[i]);			
			break;

		case 5:
			// 5. PTR Firmware Version
			szTemp.Format("[FW] PRINTER : %S", strarrVersionInfo[i]);			
			break;

		case 6:
			// 6. SIU Firmware Version
			szTemp.Format("[FW] SIU : %S", strarrVersionInfo[i]);			
			break;

		default:
			break;
		}

		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Info, _T("PRT - %S"), szTemp);
	}

	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, "", nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);

	szTemp.Format("%d. DEVICE PARAMETERS", nTitleIndex++);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	// CARD READER
	BYTE byRecvBuff[MAX_DATA_ARRAY_SIZE] = {0,};
	CString strLatchOption;
	BOOL bResult = CDEV_Manager::GetInstance()->m_DEV_CDR.ReadLatch(byRecvBuff);
	if(FALSE == bResult)
	{
		szTemp.Format("UNKNOWN");
	}
	else
	{
		if(0x30 == byRecvBuff[0])
			strLatchOption = _T("0");
		else
			strLatchOption = _T("1");
		szTemp.Format("CARD READER LATCH : %S", GET_ENDISABLE_STRING(strLatchOption));
	}
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	// CASH DISPENSER CBX # Bill / Denomination
	int nCbxCount = _ttoi(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Count);
	for (i=0; i<nCbxCount; i++)
	{
		szTemp.Format("CBX #%d NUMBER OF BILLS : %S", i+1, CUtil::ConvertFromValueToAmountwithCent(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Item_Count[i], FALSE));
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Info, _T("PRT - %S"), szTemp);

		szTemp.Format("CBX #%d DENOMINATION : %S%S", i+1, CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Denomination[i], FALSE));
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Info, _T("PRT - %S"), szTemp);
	}

	// SPEAKER VOLUME
	szTemp.Format("SOUND VOLUME : %d", CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Option.strSound_Volume));
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	// IP Information
	if (strNetworkType == _T("TCP/IP"))
	{
		// TERMINAL IP
		szTemp.Format("DHCP : %S", GET_ENDISABLE_STRING(CEagleDataManager::GetInstance()->m_Config.m_Host.strUseDHCP));
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Info, _T("PRT - %S"), szTemp);

		CString strIP, strSubnet, strGateway, strDNS;

		if (CEagleDataManager::GetInstance()->m_Config.m_Host.strUseDHCP == S_ENABLE)
		{
			CUtil::GetDHCP_IP(strIP);
			CUtil::GetDHCP_SubnetMask(strSubnet);
			CUtil::GetDHCP_GateWay(strGateway);
			CUtil::GetDHCP_DNS(strDNS);
		}
		else
		{
			strIP = CEagleDataManager::GetInstance()->m_Config.m_Host.strTerminalip;
			strSubnet = CEagleDataManager::GetInstance()->m_Config.m_Host.strSubnet;
			strGateway = CEagleDataManager::GetInstance()->m_Config.m_Host.strGateway;
			strDNS = CEagleDataManager::GetInstance()->m_Config.m_Host.strDNS;
		}

		szTemp.Format("IP : %S", strIP);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Info, _T("PRT - %S"), szTemp);

		szTemp.Format("SUBNET MASK : %S", strSubnet);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Info, _T("PRT - %S"), szTemp);

		szTemp.Format("GATEWAY : %S", strGateway);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Info, _T("PRT - %S"), szTemp);

		szTemp.Format("DNS : %S", strDNS);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Info, _T("PRT - %S"), szTemp);
	}
	else
	{
		// Setup String
		szTemp = "MODEM SETUP STRING : ";
		szTemp += CEagleDataManager::GetInstance()->m_Config.m_Host.strSetup_String;
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);

		szTemp.Replace("%", "%%");
		LOG(Info, _T("PRT - %S"), szTemp);
	}

	// V1.0.2.4 2018.07.04 - Reboot Time 추가
	szTemp.Format("REBOOT CYCLE : DAILY");
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	szTemp.Format("REBOOT TIME : %S:%S", CEagleDataManager::GetInstance()->m_Config.m_Option.strRebootTime.Left(2), CEagleDataManager::GetInstance()->m_Config.m_Option.strRebootTime.Right(2));
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, "", nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);

	szTemp.Format("%d. HOST PARAMETERS", nTitleIndex++);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	// 3. Communication header
	szTemp.Format("COMMUNICATION HEADER : %S", GET_ENDISABLE_STRING(CEagleDataManager::GetInstance()->m_Config.m_Host.strCommunication_header));
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	// 4. Communication ID
	szTemp.Format("COMMUNICATION ID : %S", CEagleDataManager::GetInstance()->m_Config.m_Host.strCommunicationID);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);

	szTemp.Replace("%", "%%");
	LOG(Info, _T("PRT - %S"), szTemp);

	// 5. STATUS MONITORING
	szTemp.Format("STATUS MONITORING : %S", GET_ENDISABLE_STRING(CEagleDataManager::GetInstance()->m_Config.m_Host.strAtm_status_monitoring));
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	// 6. HEARTBEAT
	szTemp.Format("HEARTBEAT : %S", GET_ENDISABLE_STRING(CEagleDataManager::GetInstance()->m_Config.m_Host.strHeartbeat_message));
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Error, _T("PRT - %S"), szTemp);

	szTemp.Format("HEARTBEAT FREQUENCY : %d MINUTES", CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Host.strHeartbeat_frequency));
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

#if !(NETWORK_OFFLINE_MODE)
	if (strNetworkType == _T("TCP/IP"))
	{
		// IP인지 URL인지 점검
		BOOL bUseURL = FALSE;
		CString strHostInfo;

		strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strHostip;
		strTemp.TrimRight();
		strTemp.Replace(_T("."), _T(""));

		if (CUtil::IsNumeric(strTemp) == TRUE)	// IP Base
			bUseURL = FALSE;
		else
			bUseURL = TRUE;

		// Host Address
		strHostInfo = CEagleDataManager::GetInstance()->m_Config.m_Host.strHostip;
		strHostInfo.TrimRight();

		if (strHostInfo.IsEmpty())
		{
			szTemp.Format("PRIMARY HOST ADDRESS : (EMPTY)");
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Info, _T("PRT - %S"), szTemp);
		}
		else
		{
			if (bUseURL == TRUE)
			{
				szTemp.Format("PRIMARY HOST ADDRESS :");
				CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
				CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
				LOG(Info, _T("PRT - %S"), szTemp);

				szTemp.Format("%S", strHostInfo);
				CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
				CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
				LOG(Info, _T("PRT - %S"), szTemp);
			}
			else
			{
				szTemp.Format("PRIMARY HOST ADDRESS : %S", strHostInfo);
				CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
				CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
				LOG(Info, _T("PRT - %S"), szTemp);
			}
		}

		szTemp.Format("PRIMARY HOST PORT : %d", CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Host.strPort));
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Info, _T("PRT - %S"), szTemp);

		strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strHostip;
		strTemp.TrimRight();
		strTemp.Replace(_T("."), _T(""));

		if (CUtil::IsNumeric(strTemp) == TRUE)	// IP Base
			bUseURL = FALSE;
		else
			bUseURL = TRUE;

		strHostInfo = CEagleDataManager::GetInstance()->m_Config.m_Host.strBackup_Hostip;
		strHostInfo.TrimRight();

		if (strHostInfo.IsEmpty())
		{
			szTemp.Format("BACKUP HOST ADDRESS : (EMPTY)");
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Info, _T("PRT - %S"), szTemp);
		}
		else
		{
			if (bUseURL == TRUE)
			{
				szTemp.Format("BACKUP HOST ADDRESS :");
				CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
				CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
				LOG(Info, _T("PRT - %S"), szTemp);

				szTemp.Format("%S", strHostInfo);
				CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
				CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
				LOG(Info, _T("PRT - %S"), szTemp);
			}
			else
			{
				szTemp.Format("BACKUP HOST ADDRESS : %S", strHostInfo);
				CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
				CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
				LOG(Info, _T("PRT - %S"), szTemp);
			}
		}
		
		szTemp.Format("BACKUP HOST PORT : %d", CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Host.strBackupPort));
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Info, _T("PRT - %S"), szTemp);
	}
	else
	{
		// Phone Number
		strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strPrimary_phone_number;
		strTemp.TrimRight();

		if (strTemp.IsEmpty())
		{
			szTemp.Format("PRIMARY HOST PHONE NUMBER : (EMPTY)");
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Info, _T("PRT - %S"), szTemp);
		}
		else
		{
			szTemp.Format("PRIMARY PHONE NUMBER : %S", strTemp);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Info, _T("PRT - %S"), szTemp);
		}

		strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strBackup_phone_number;
		strTemp.TrimRight();

		if (strTemp.IsEmpty())
		{
			szTemp.Format("BACKUP PHONE NUMBER : (EMPTY)");
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Info, _T("PRT - %S"), szTemp);
		}
		else
		{
			szTemp.Format("BACKUP PHONE NUMBER : %S", strTemp);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Info, _T("PRT - %S"), szTemp);
		}
	}
#endif

	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, "", nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);

	// EMV AID LIST
	szTemp.Format("EMV - AID LIST(%d)", CDEV_Manager::GetInstance()->m_DEV_CDR.m_sEMV_Config_Data.nTotalAidListCount);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	// EMV AID LIST
	for(int i=0; i<CDEV_Manager::GetInstance()->m_DEV_CDR.m_sEMV_Config_Data.nTotalAidListCount; i++)
	{
		szTemp.Format("(%d) %7.7S : %s", i+1, GET_ENDISABLE_INT(CDEV_Manager::GetInstance()->m_DEV_CDR.m_sEMV_Config_Data.m_sEMV_Config[i].AidUse), CDEV_Manager::GetInstance()->m_DEV_CDR.m_sEMV_Config_Data.m_sEMV_Config[i].AidName);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Info, _T("PRT - %S"), szTemp);
	}

	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, "", nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);

	// 4. Option Information
	szTemp.Format("%d. OPTION PARAMETERS", nTitleIndex++);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	// Language
	szTemp.Format("ENGLISH LANGUAGE: %S", GET_ENDISABLE_STRING(CEagleDataManager::GetInstance()->m_Config.m_Option.strLansupport[0]));
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	szTemp.Format("SPANISH LANGUAGE : %S", GET_ENDISABLE_STRING(CEagleDataManager::GetInstance()->m_Config.m_Option.strLansupport[1]));
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	szTemp.Format("FRENCH LANGUAGE : %S", GET_ENDISABLE_STRING(CEagleDataManager::GetInstance()->m_Config.m_Option.strLansupport[2]));
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	// Transaction Type
	szTemp.Format("BALANCE TRANSACTION : %S", GET_ENDISABLE_STRING(CEagleDataManager::GetInstance()->m_Config.m_Option.strbalance_Enable));
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	szTemp.Format("TRANSFER TRANSACTION : %S", GET_ENDISABLE_STRING(CEagleDataManager::GetInstance()->m_Config.m_Option.strtransfer_Enable));
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	// Account Type
	szTemp.Format("SAVINGS ACCOUNT : %S", GET_ENDISABLE_STRING(CEagleDataManager::GetInstance()->m_Config.m_Option.strSaving_account));
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	szTemp.Format("CREDIT ACCOUNT : %S", GET_ENDISABLE_STRING(CEagleDataManager::GetInstance()->m_Config.m_Option.strCredit_account));
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	szTemp.Format("FAST CASH L1 : %S%S", CURRENCY_SYMBOL, CEagleDataManager::GetInstance()->m_Config.m_Option.strFast_amount[0]);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	szTemp.Format("FAST CASH L2 : %S%S", CURRENCY_SYMBOL, CEagleDataManager::GetInstance()->m_Config.m_Option.strFast_amount[1]);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	szTemp.Format("FAST CASH L3 : %S%S", CURRENCY_SYMBOL, CEagleDataManager::GetInstance()->m_Config.m_Option.strFast_amount[2]);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	szTemp.Format("FAST CASH R1 : %S%S", CURRENCY_SYMBOL, CEagleDataManager::GetInstance()->m_Config.m_Option.strFast_amount[3]);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	szTemp.Format("FAST CASH R2 : %S%S", CURRENCY_SYMBOL, CEagleDataManager::GetInstance()->m_Config.m_Option.strFast_amount[4]);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	szTemp.Format("FAST CASH R3 : %S%S", CURRENCY_SYMBOL, CEagleDataManager::GetInstance()->m_Config.m_Option.strFast_amount[5]);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	szTemp.Format("MAX WITHDRAWAL AMOUNT : %S%S", CURRENCY_SYMBOL, CEagleDataManager::GetInstance()->m_Config.m_Option.strMax_withdrawal_amount);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	// BIN LIST COUNT 추가
	szTemp.Format("BIN LIST COUNT : %d", CEagleDataManager::GetInstance()->m_Config.m_Iso.nTotalCount);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	szTemp.Format("SURCHARGE TYPE : %S", GET_SURCHARGETYPE_STRING(CEagleDataManager::GetInstance()->m_Config.m_Option.strSurcharge_type));
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	// Surcharge Amount는 Type에 상관없이 인자하도록 수정
	szTemp.Format("SURCHARGE AMOUNT : %S%S", CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(CEagleDataManager::GetInstance()->m_Config.m_Option.strWithdrawalSurcharge_amount));
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	if (CEagleDataManager::GetInstance()->m_Config.m_Option.strSurcharge_type == _T("1"))
	{
		szTemp.Format("METHOD : %S", GET_PERCENTAGEMETHOD_STRING(CEagleDataManager::GetInstance()->m_Config.m_Option.strWithdrawal_PercentSurcharge_Option));
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Info, _T("PRT - %S"), szTemp);

		szTemp.Format("PERCENTAGE : %S", CUtil::ConvertFromValueToAmountwithCent(CEagleDataManager::GetInstance()->m_Config.m_Option.strSurcharge_percentage));
		LOG(Info, _T("PRT - %S"), szTemp);
		szTemp += "%";
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	}

	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Option.strSurcharge_owner;
	strTemp.TrimLeft();
	strTemp.TrimRight();

	if (strTemp.IsEmpty())
	{
		szTemp.Format("TERMINAL OWNER : (EMPTY)");
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Info, _T("PRT - %S"), szTemp);
	}
	else
	{
		szTemp = "TERMINAL OWNER : ";
		szTemp += strTemp;
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		
		szTemp.Replace("%", "%%");
		LOG(Info, _T("PRT - %S"), szTemp);
	}

	// ADV Option
	szTemp.Format("AD INTERVAL : %d SEC", CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_ADS.strAdver_Interval_time));
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	for(int i=0; i< SUPPORT_ADVERTISE; i++)
	{
		szTemp.Format("AD %d SCREEN : %S", i+1, GET_ENDISABLE_STRING(CEagleDataManager::GetInstance()->m_Config.m_ADS.strAdver_Screen_en[i]));
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Info, _T("PRT - %S"), szTemp);
	}

	// MESSAGE
	BOOL bExistMsg = FALSE;

	for(i=0; i<4; i++)
	{
		strTemp = CEagleDataManager::GetInstance()->m_Config.m_Message.strMarketing_message[i];
		strTemp.TrimRight();

		if (strTemp.IsEmpty() == FALSE)
		{
			bExistMsg = TRUE;
			break;
		}
	}

	if (bExistMsg == TRUE)
	{
		szTemp.Format("MARKETING MESSAGE :");
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Info, _T("PRT - %S"), szTemp);

		for(i=0; i<4; i++)
		{
			szTemp = CEagleDataManager::GetInstance()->m_Config.m_Message.strMarketing_message[i];
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			szTemp.Replace("%", "%%");
			LOG(Info, _T("PRT - %S"), szTemp);
		}		
	}
	else
	{
		szTemp.Format("MARKETING MESSAGE : (EMPTY)");
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Info, _T("PRT - %S"), szTemp);
	}

	bExistMsg = FALSE;

	for(i=0; i<4; i++)
	{
		strTemp = CEagleDataManager::GetInstance()->m_Config.m_Message.strLocation_message[i];
		strTemp.TrimRight();

		if (strTemp.IsEmpty() == FALSE)
		{
			bExistMsg = TRUE;
			break;
		}
	}

	if (bExistMsg == TRUE)
	{
		szTemp.Format("LOCATION MESSAGE :");
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Error, _T("PRT - %S"), szTemp);

		for(i=0; i<4; i++)
		{
			szTemp = CEagleDataManager::GetInstance()->m_Config.m_Message.strLocation_message[i];
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);

			szTemp.Replace("%", "%%");
			LOG(Info, _T("PRT - %S"), szTemp);
		}		
	}
	else
	{
		szTemp.Format("LOCATION MESSAGE : (EMPTY)");
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Info, _T("PRT - %S"), szTemp);
	}

	bExistMsg = FALSE;

	for(i=0; i<3; i++)
	{
		strTemp = CEagleDataManager::GetInstance()->m_Config.m_Message.strAttraction_message[i];
		strTemp.TrimRight();

		if (strTemp.IsEmpty() == FALSE)
		{
			bExistMsg = TRUE;
			break;
		}
	}

	if (bExistMsg == TRUE)
	{
		szTemp.Format("ATTRACT MESSAGE :");
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Info, _T("PRT - %S"), szTemp);

		for(i=0; i<3; i++)
		{
			szTemp = CEagleDataManager::GetInstance()->m_Config.m_Message.strAttraction_message[i];
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);

			szTemp.Replace("%", "%%");
			LOG(Info, _T("PRT - %S"), szTemp);
		}		
	}
	else
	{
		szTemp.Format("ATTRACT MESSAGE : (EMPTY)");
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Info, _T("PRT - %S"), szTemp);
	}

	bExistMsg = FALSE;

	for(i=0; i<3; i++)
	{
		strTemp = CEagleDataManager::GetInstance()->m_Config.m_Message.strFarewell_message[i];
		strTemp.TrimRight();

		if (strTemp.IsEmpty() == FALSE)
		{
			bExistMsg = TRUE;
			break;
		}
	}

	if (bExistMsg == TRUE)
	{
		szTemp.Format("FAREWELL MESSAGE :");
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Info, _T("PRT - %S"), szTemp);

		for(i=0; i<3; i++)
		{
			szTemp = CEagleDataManager::GetInstance()->m_Config.m_Message.strFarewell_message[i];
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);

			szTemp.Replace("%", "%%");
			LOG(Info, _T("PRT - %S"), szTemp);
		}		
	}
	else
	{
		szTemp.Format("FAREWELL MESSAGE : (EMPTY)");
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Info, _T("PRT - %S"), szTemp);
	}
	/////////////////////

	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, "", nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);

	// 5. TASK Information
	szTemp.Format("%d. TASK PARAMETERS", nTitleIndex++);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	szTemp.Format("SCHEDULE DAY CLOSE : %S", GET_ENDISABLE_STRING(CEagleDataManager::GetInstance()->m_Config.m_Option.strScheduleDayClose));
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	szTemp.Format("SCHEDULE DAY CLOSE TIME : %S:%S", CEagleDataManager::GetInstance()->m_Config.m_Option.strDayCloseTime.Left(2), CEagleDataManager::GetInstance()->m_Config.m_Option.strDayCloseTime.Right(2));
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);

	// 6. AMS Information
	szTemp.Format("%d. AMS PARAMETERS", nTitleIndex++);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	szTemp.Format("AMS : %S", GET_ENDISABLE_STRING(CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_en_flg));
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	szTemp.Format("AMS STATUS SEND : %S", GET_ENDISABLE_STRING(CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_Status_en_flg));
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	if (strNetworkType == _T("TCP/IP"))
	{
		// AMS ADDRESS
		strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_IP_Address;
		strTemp.TrimRight();

		if (strTemp.IsEmpty())
		{
			szTemp.Format("AMS IP ADDRESS : (EMPTY)");
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Info, _T("PRT - %S"), szTemp);
		}
		else
		{
			szTemp.Format("AMS IP ADDRESS : %S", strTemp);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Info, _T("PRT - %S"), szTemp);
		}

		szTemp.Format("AMS PORT : %d", CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_Port));
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
		LOG(Info, _T("PRT - %S"), szTemp);

	}
	else
	{
		// AMS Phone Number
		strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_primary_num;
		strTemp.TrimRight();

		if (strTemp.IsEmpty())
		{
			szTemp.Format("AMS PHONE NUMBER : (EMPTY)");
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Info, _T("PRT - %S"), szTemp);
		}
		else
		{
			szTemp.Format("AMS PHONE NUMBER : %S", strTemp);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
			CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
			LOG(Info, _T("PRT - %S"), szTemp);
		}
	}

	//// Schedule Journal
	szTemp.Format("UPLOAD JOURNAL : %S", GET_ENDISABLE_STRING(CEagleDataManager::GetInstance()->m_Config.m_Host.strSchedule_journal_flg));
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	szTemp.Format("UPLOAD JOURNAL COUNT : %d HOUR", CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Host.strSchedule_journal_count));
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, "", nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);

	// 7. Journal Information
	szTemp.Format("%d. JOURNAL PARAMETERS", nTitleIndex++);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	strTemp.Format(_T("%d"), CDEV_Manager::GetInstance()->m_DEV_EJL.GetLastJournalNo());
	szTemp.Format("CURRENT JOURNAL COUNT : %S", CUtil::ConvertFromValueToAmountwithCent(strTemp, FALSE));
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, "", nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);

	// 8. PINPAD Information
	szTemp.Format("%d. PINPAD PARAMETERS", nTitleIndex++);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	// PINPAD STATE
	int nActivatedStatus = CDEV_Manager::GetInstance()->m_DEV_CREPP.GetDeviceStatus();
	szTemp.Format("PINPAD STATE : %S", GET_EPP_ACTIVATE__STRING(nActivatedStatus));
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	// MASTER KCV
	CString strKeyKCV;
	strKeyKCV = CDEV_Manager::GetInstance()->m_DEV_CREPP.Get_KeyCheckValue(MASTER_KEYNAME);

	if (strKeyKCV.IsEmpty() == FALSE)
		strTemp.Format(_T("%s %s"), strKeyKCV.Left(2), strKeyKCV.Mid(2,2));
	else
		strTemp.Format(_T("EMPTY"));

	szTemp.Format("MASTER KEY CHECKSUM : %S", strTemp);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	strKeyKCV = CDEV_Manager::GetInstance()->m_DEV_CREPP.Get_KeyCheckValue(WORKING_KEYNAME);

	if (strKeyKCV.IsEmpty() == FALSE)
		strTemp.Format(_T("%s %s"), strKeyKCV.Left(2), strKeyKCV.Mid(2,2));
	else
		strTemp.Format(_T("EMPTY"));

	szTemp.Format("PIN KEY CHECKSUM : %S", strTemp);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_Textout(EAGLE_PRT_TYPE_USB, (char*)(LPCSTR)szTemp, nOrgX, 1, 1, EAGLE_PRT_FONT_TYPE_STANDARD, EAGLE_PRT_FONT_STYLE_NORMAL);
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_FeedLine(EAGLE_PRT_TYPE_USB);
	LOG(Info, _T("PRT - %S"), szTemp);

	// Paper Cut
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_CutPaper(EAGLE_PRT_TYPE_USB, 1, 0);

	// PTR Quiry Status Stop
	CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_QuiryStatusStart();

	// 프린터 램프 OFF
	CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_1, DIO_CMD_OFF);

	if (CEagleSVCLib::GetInstance()->IsPTR_Available(TRUE) == FALSE)
		return FALSE;

	return TRUE;
}


/** **********************************************************
*	@brief		ErrorCode
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_ErrorCode()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	BOOL bFoundErrorCode = FALSE;
	CString strScreenInform;
	CString strTemp, strTemp2;
	CString strErrorCode, strErrorDesc, strErrorRecovery;
	ERRCODEINFO	m_ErrorcodeInfo;
	CStringArray	strarrayTemp;
	int		i=0;

	m_nCurrentFocusIndex = 1;

	strarrayTemp.RemoveAll();

	// 처음 화면 표시시에는 에러가 있는 경우 해당 에러코드 표시
	// 없는 경우에는 첫번째 에러코드 표시
	if (CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetError == TRUE)
	{
		strErrorCode = CEagleDataManager::GetInstance()->m_Client_Info.m_strErrorCode;

		LOG(Info, _T("Current Error Code : %s - try to search error code from DB"), strErrorCode);

		memset(&m_ErrorcodeInfo, 0, sizeof(m_ErrorcodeInfo));

		if (CDEV_Manager::GetInstance()->m_DEV_EJL.GetErrorCodeInfo(strErrorCode, &m_ErrorcodeInfo) == TRUE)
		{
			strErrorCode.Format(_T("%S"), m_ErrorcodeInfo.err_code);
			
			strarrayTemp.RemoveAll();
			strErrorDesc.Empty();

			strTemp.Format(_T("%S"), m_ErrorcodeInfo.err_desc);
			CUtil::ParsingStringToStringArray(strTemp, ERROR_DB_LINEFEED_DELEMITER, strarrayTemp);

			for(i=0; i< strarrayTemp.GetCount(); i++)
			{
				strErrorDesc += strarrayTemp[i];
				strErrorDesc += SCR_LINEFEED_DELEMITER;
			}


			strarrayTemp.RemoveAll();
			strErrorRecovery.Empty();

			strTemp.Format(_T("%S"), m_ErrorcodeInfo.err_recovery);
			CUtil::ParsingStringToStringArray(strTemp, ERROR_DB_LINEFEED_DELEMITER, strarrayTemp);

			for(i=0; i< strarrayTemp.GetCount(); i++)
			{
				strErrorRecovery += strarrayTemp[i];
				strErrorRecovery += SCR_LINEFEED_DELEMITER;
			}

			bFoundErrorCode = TRUE;
		}
		else
		{
			strErrorCode = _T("J00004");	// Search Fail로 처리
			LOG(Error, _T("Can not fine error code (%s)"), strErrorCode);

			memset(&m_ErrorcodeInfo, 0, sizeof(m_ErrorcodeInfo));

			if (CDEV_Manager::GetInstance()->m_DEV_EJL.GetErrorCodeInfo(strErrorCode, &m_ErrorcodeInfo) == TRUE)
			{
				strErrorCode.Format(_T("%S"), m_ErrorcodeInfo.err_code);

				strarrayTemp.RemoveAll();
				strErrorDesc.Empty();

				strTemp.Format(_T("%S"), m_ErrorcodeInfo.err_desc);
				CUtil::ParsingStringToStringArray(strTemp, ERROR_DB_LINEFEED_DELEMITER, strarrayTemp);

				for(i=0; i< strarrayTemp.GetCount(); i++)
				{
					strErrorDesc += strarrayTemp[i];
					strErrorDesc += SCR_LINEFEED_DELEMITER;
				}


				strarrayTemp.RemoveAll();
				strErrorRecovery.Empty();

				strTemp.Format(_T("%S"), m_ErrorcodeInfo.err_recovery);
				CUtil::ParsingStringToStringArray(strTemp, ERROR_DB_LINEFEED_DELEMITER, strarrayTemp);

				for(i=0; i< strarrayTemp.GetCount(); i++)
				{
					strErrorRecovery += strarrayTemp[i];
					strErrorRecovery += SCR_LINEFEED_DELEMITER;
				}

				bFoundErrorCode = TRUE;
			}
			else
			{
				LOG(Error, _T("Can not fine error code (%s)"), strErrorCode);
			}
		}
	}
	else
	{
		// 정상인 경우 Error Code 처음값인 C00001로 표시
		strErrorCode = _T("C00001");
		memset(&m_ErrorcodeInfo, 0, sizeof(m_ErrorcodeInfo));

		if (CDEV_Manager::GetInstance()->m_DEV_EJL.GetErrorCodeInfo(strErrorCode, &m_ErrorcodeInfo) == TRUE)
		{
			strarrayTemp.RemoveAll();
			strErrorDesc.Empty();

			strTemp.Format(_T("%S"), m_ErrorcodeInfo.err_desc);
			CUtil::ParsingStringToStringArray(strTemp, ERROR_DB_LINEFEED_DELEMITER, strarrayTemp);

			for(i=0; i< strarrayTemp.GetCount(); i++)
			{
				strErrorDesc += strarrayTemp[i];
				strErrorDesc += SCR_LINEFEED_DELEMITER;
			}


			strarrayTemp.RemoveAll();
			strErrorRecovery.Empty();

			strTemp.Format(_T("%S"), m_ErrorcodeInfo.err_recovery);
			CUtil::ParsingStringToStringArray(strTemp, ERROR_DB_LINEFEED_DELEMITER, strarrayTemp);

			for(i=0; i< strarrayTemp.GetCount(); i++)
			{
				strErrorRecovery += strarrayTemp[i];
				strErrorRecovery += SCR_LINEFEED_DELEMITER;
			}

			bFoundErrorCode = TRUE;
		}
		else
		{
			LOG(Error, _T("Can not fine error code (%s)"), strErrorCode);
		}
	}

	if (bFoundErrorCode == FALSE)
	{
		strErrorCode = ERROR_UNKNOWN_ERRORCODE;
		strErrorDesc = _T("Unknown error code");
		strErrorRecovery = _T("Contact your system manager");
	}

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (bShowScreen == TRUE)
		{
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();
			strScreenInform.Empty();

			// 1. Error Code
			strScreenInform = strErrorCode;
			strScreenInform += SCR_CMD_DELIMITER;

			// 2. Error description
			strScreenInform += strErrorDesc;
			strScreenInform += SCR_CMD_DELIMITER;

			// 3. Error recovery
			strScreenInform += strErrorRecovery;
			strScreenInform += SCR_CMD_DELIMITER;

			Operator_DisplayScreen(OP_SCREEN_ERRORCODE, bKeyEnable, strScreenInform);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_ErrorCode - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_REPORT;
				break;
			}

			// Touch로 Focus를 주기 위한 로직
			CString strTouchName = _T("SEARCHERRORCODE_TOUCH");
			if (strTouchName.CompareNoCase(m_strScrOutData.Left(strTouchName.GetLength())) == 0)
			{
				m_nCurrentFocusIndex = CUtil::StringToInt(m_strScrOutData.Right(1));
				m_strScrOutData = _T("SEARCHERRORCODE");
			}
			///////////////////////////////////////////////////

			// Next Action
			if (m_strScrOutData == _T("SEARCHERRORCODE"))
			{
				if (Operator_Input_Text(_T("SEARCH ERROR CODE"), _T(""), m_strScrOutData, 6, UPPERCASE_MODE) == TRUE)
				{
					m_strScrOutData.TrimLeft();
					m_strScrOutData.TrimRight();

					LOG(Info, _T("SELECT SEARCH ERROR CODE - %s"), m_strScrOutData);

					Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);

					memset(&m_ErrorcodeInfo, 0, sizeof(m_ErrorcodeInfo));
					if (CDEV_Manager::GetInstance()->m_DEV_EJL.GetErrorCodeInfo(m_strScrOutData, &m_ErrorcodeInfo) == TRUE)
					{
						strErrorCode.Format(_T("%S"), m_ErrorcodeInfo.err_code);

						strarrayTemp.RemoveAll();
						strErrorDesc.Empty();

						strTemp.Format(_T("%S"), m_ErrorcodeInfo.err_desc);
						CUtil::ParsingStringToStringArray(strTemp, ERROR_DB_LINEFEED_DELEMITER, strarrayTemp);

						for(i=0; i< strarrayTemp.GetCount(); i++)
						{
							strErrorDesc += strarrayTemp[i];
							strErrorDesc += SCR_LINEFEED_DELEMITER;
						}

						strarrayTemp.RemoveAll();
						strErrorRecovery.Empty();

						strTemp.Format(_T("%S"), m_ErrorcodeInfo.err_recovery);
						CUtil::ParsingStringToStringArray(strTemp, ERROR_DB_LINEFEED_DELEMITER, strarrayTemp);

						for(i=0; i< strarrayTemp.GetCount(); i++)
						{
							strErrorRecovery += strarrayTemp[i];
							strErrorRecovery += SCR_LINEFEED_DELEMITER;
						}

						bFoundErrorCode = TRUE;
					}
					else
					{
						LOG(Info, _T("CAN NOT FIND ERROR CODE"));
						Operator_Processing_Screen(_T("CAN NOT FIND ERROR CODE"));
						CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
					}
				}

				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("PREV"))
			{
				// Device쪽 함수가 구현이 안되어 기능 미구현
				bShowScreen = TRUE;			
			}
			else if (m_strScrOutData == _T("NEXT"))
			{
				// Device쪽 함수가 구현이 안되어 기능 미구현
				bShowScreen = TRUE;			
			}
			else
			{
				// 비정상적 값인 경우에는 OP MAIN으로 이동
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
		}

		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}
}


/** **********************************************************
*	@brief		SW Version 취득 함수
*	@retval		Version 정보
************************************************************/
CString CSVC_Manager::Get_SW_VersionInfo()
{
	CString strResult, strTemp;
	int nMajor = 0, nMinor = 0;
	BOOL bResult = FALSE;
	CString strBLVer, strKernelVer;

	CString strProductVersion;
	strProductVersion = CDEV_Manager::GetInstance()->m_strAP_Version;

	// 1. Software Main Version
#if (NETWORK_OFFLINE_MODE)
	strTemp.Format(_T("D%s"), strProductVersion);
#else
	strTemp.Format(_T("V%s"), strProductVersion);
#endif

	strResult = strTemp;
	strResult += AP_DELEMITER;

	// OS Version (V 01(Dummy). 1(Bootloader). 11(Kernel)
	//strBLVer = CUtil::String_GetRegistry(REGISTRY_OS_INFO_KEY, REGISTRY_BOOTLOADER_VALUE_NAME);
	strKernelVer = CUtil::String_GetRegistry(REGISTRY_OS_INFO_KEY, REGISTRY_KERNEL_VALUE_NAME);

	strKernelVer.Replace(_T("v"), _T("V"));

	strTemp.Format(_T("%s"), strKernelVer);

	strResult += strTemp;
	strResult += AP_DELEMITER;

	// 2. CDM Firmware Version
#if (EMULATION_CDM_DEVICE)
	strTemp = _T("N/A");
#else
	strTemp.Format(_T("V%s"), CDEV_Manager::GetInstance()->m_DEV_CDM.m_strWDM_FW_Version);
#endif
	strResult += strTemp;
	strResult += AP_DELEMITER;	


	// 3. IDC firmware version (Interface 없음)
#if (EMULATION_CDR_DEVICE)
	strTemp = _T("N/A");
#else
	// 통신 장애시 Delay를 없애기 위해 로직 수정 (Initialize시에 다시 Version Read하도록 함)
	strTemp = CDEV_Manager::GetInstance()->m_DEV_CDR.m_strCDR_FW_Version;
#endif
	strResult += strTemp;
	strResult += AP_DELEMITER;

	// 4. EPP firmware version (Interface 없음)
#if (EMULATION_EPP_DEVICE)
	strTemp = _T("N/A");
#else
	strTemp.Empty();
	//CDEV_Manager::GetInstance()->m_DEV_CREPP.GetVersionInfo(&strTemp);
	strTemp = CDEV_Manager::GetInstance()->m_DEV_CREPP.m_strREPP_FW_Version;
#endif
	strResult += strTemp;
	strResult += AP_DELEMITER;

	// 5. PTR firmware version 
#if (EMULATION_PTR_DEVICE)
	strTemp = _T("N/A");
#else
	strTemp.Format(_T("V%s"), CDEV_Manager::GetInstance()->m_DEV_PRT.m_strPRT_FW_Version);
#endif
	strResult += strTemp;
	strResult += AP_DELEMITER;

	// 6. SIU firmware version
#if (EMULATION_SIU_DEVICE)
	strTemp = _T("N/A");
#else
	strTemp = CDEV_Manager::GetInstance()->m_DEV_DIO.m_strDIO_FW_Version;
#endif

	strResult += strTemp;
	strResult += AP_DELEMITER;

	return strResult;
}

