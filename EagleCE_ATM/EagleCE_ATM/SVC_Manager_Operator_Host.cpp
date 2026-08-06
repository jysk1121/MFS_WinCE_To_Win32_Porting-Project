#include "stdafx.h"
#include "SVC_Manager.h"

#include "EagleSVCLib.h"
#include "EagleTritonMsg.h"

#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"
#include "../../EagleCE_Framework/EagleCE_Framework/Util.h"
#include "../../EagleCE_Screen/EagleCE_Screen/SCR_Manager.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/** **********************************************************
*	@brief		Host Main
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_Host_Main()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	CString strScreenInform;

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (bShowScreen == TRUE)
		{
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();

			Operator_DisplayScreen(OP_SCREEN_HOST, bKeyEnable, strScreenInform);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_Host_Main - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER || m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}

			// Next Action
			if (m_strScrOutData == _T("NETWORKSETTINGS"))
			{
				m_strNextScrNum = OP_SCREEN_TERMINAL_NETWORK_SETTINGS;
				break;
			}
			else if (m_strScrOutData == _T("KEYMANAGEMENT"))
			{
				m_strNextScrNum = OP_SCREEN_KEY_MANAGEMENT;
				break;
			}
			else if (m_strScrOutData == _T("CONFIGURECONNECTION"))
			{
				m_strNextScrNum = OP_SCREEN_CONFIGURE_CONNECTION;
				break;
			}
			else if (m_strScrOutData == _T("TERMINALMONITORING"))
			{
				m_strNextScrNum = OP_SCREEN_TERMINAL_MONITORING;
				break;
			}
			else if (m_strScrOutData == _T("AIDLIST"))
			{
				m_strNextScrNum = OP_SCREEN_EMV_AID_LIST;
				break;
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
*	@brief		Terminal Network Settings
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_NetworkSettings()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	CString strScreenInform;
	CString strButtonInform;
	CString strTemp, strNetworkType;

	// Focus Setting
	m_nCurrentFocusIndex = 1;

	strTemp = CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType;
	strNetworkType = strTemp.MakeUpper();

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (bShowScreen == TRUE)
		{
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();
			strScreenInform.Empty();

			// Screen Data
			strScreenInform = strNetworkType;
			strScreenInform += SCR_CMD_DELIMITER;

			// 실제 값이 setting되었을 경우에만 Button이 적용되도록

			if (CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType == _T("TCP/IP"))
				strTemp = _T("IP SETTINGS");
			else
				strTemp = _T("MODEM SETTINGS");

			strScreenInform += strTemp;
			strScreenInform += SCR_CMD_DELIMITER;

			Operator_DisplayScreen(OP_SCREEN_TERMINAL_NETWORK_SETTINGS, bKeyEnable, strScreenInform);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_NetworkSettings - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_HOST;
				break;
			}

			// Touch로 Focus를 주기 위한 로직
			if (m_strSelectTouch.CompareNoCase(m_strScrOutData.Left(m_strSelectTouch.GetLength())) == 0)
			{
				m_nCurrentFocusIndex = CUtil::StringToInt(m_strScrOutData.Right(1));
				m_strScrOutData = _T("SELECT");
			}
			///////////////////////////////////////////////////

			// Next Action
			if (m_strScrOutData == _T("DETAILS"))
			{
				//if (strNetworkType == _T("TCP/IP"))
				if (CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType == _T("TCP/IP"))	// UI 구성 변경에 따른 File에 저장된 값을 참조하도록 변경
					m_strNextScrNum = OP_SCREEN_TERMINAL_IP_SETTINGS;
				else
					m_strNextScrNum = OP_SCREEN_TERMINAL_MODEM_SETTINGS;
				break;
			}
			else if (m_strScrOutData == _T("SELECT"))
			{
				if (strNetworkType == _T("TCP/IP"))
					strNetworkType = _T("MODEM");
				else
					strNetworkType = _T("TCP/IP");

				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("SET"))
			{
				CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType = strNetworkType;

				if (CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType == _T("MODEM"))
				{
					// TLS 및 Protocol type 변경
					CEagleDataManager::GetInstance()->m_Config.m_Host.strUseTLS = S_DISABLE;
					CEagleDataManager::GetInstance()->m_Config.m_Host.strProtocolType = _T("VISA");
				}

				CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("network_type"), CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType);
				CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("useTLS"), CEagleDataManager::GetInstance()->m_Config.m_Host.strUseTLS);
				CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("protocol_type"), CEagleDataManager::GetInstance()->m_Config.m_Host.strProtocolType);
				
				Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);

				// Set Network Config
				if (CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType == _T("TCP/IP"))
					CDEV_Manager::GetInstance()->Set_Network_Config();
				else
					CDEV_Manager::GetInstance()->Set_Network_Config(FALSE);

				Operator_Processing_Screen(_T("SUCCESS"));
				CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

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
*	@brief		Modem Settings
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_ModemSettings()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	CString strScreenInform;
	CString strButtonInform;
	CString strTemp, strSetupstring;

	// Focus Setting
	m_nCurrentFocusIndex = 1;

	strSetupstring = CEagleDataManager::GetInstance()->m_Config.m_Host.strSetup_String;

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (bShowScreen == TRUE)
		{
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();
			strScreenInform.Empty();

			// Screen Data
			strScreenInform = strSetupstring;
			strScreenInform += SCR_CMD_DELIMITER;

			Operator_DisplayScreen(OP_SCREEN_TERMINAL_MODEM_SETTINGS, bKeyEnable, strScreenInform);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_ModemSettings - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_TERMINAL_NETWORK_SETTINGS;
				break;
			}

			// Touch로 Focus를 주기 위한 로직
			if (m_strSelectTouch.CompareNoCase(m_strScrOutData.Left(m_strSelectTouch.GetLength())) == 0)
			{
				m_nCurrentFocusIndex = CUtil::StringToInt(m_strScrOutData.Right(1));
				m_strScrOutData = _T("SELECT");
			}
			///////////////////////////////////////////////////

			if (m_strScrOutData == _T("SELECT"))
			{
				if (Operator_Input_Text(_T("SETUP STRING"), strSetupstring, m_strScrOutData, 20, UPPERCASE_MODE) == TRUE)
				{
					m_strScrOutData.TrimLeft();
					m_strScrOutData.TrimRight();

					strSetupstring = m_strScrOutData;
				}

				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("SET"))
			{
				CEagleDataManager::GetInstance()->m_Config.m_Host.strSetup_String = strSetupstring;
				CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("setup_string"), CEagleDataManager::GetInstance()->m_Config.m_Host.strSetup_String);

				Operator_Processing_Screen(_T("SUCCESS"));
				CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

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
*	@brief		IP Settings
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_IPSettings()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	CString strScreenInform;
	CString strButtonInform;
	CString strTemp, strDHCP, strIP, strSubnet, strGateway, strDNS;
	int nMaxFocusIndex = 5;
	CStringArray	strArray;
	int	nTempFocusIndex = 0;

	// Focus Setting
	m_nCurrentFocusIndex = 1;

	strDHCP = CEagleDataManager::GetInstance()->m_Config.m_Host.strUseDHCP;

	if (strDHCP == S_ENABLE)
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

	LOG(Info, _T("Proc_Operator_IPSettings - Start"));

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (bShowScreen == TRUE)
		{
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();
			strScreenInform.Empty();

			// Screen Data
			strScreenInform = GET_ENDISABLE_STRING(strDHCP);
			strScreenInform += SCR_CMD_DELIMITER;

			strScreenInform += strIP;
			strScreenInform += SCR_CMD_DELIMITER;

			strScreenInform += strSubnet;
			strScreenInform += SCR_CMD_DELIMITER;

			strScreenInform += strGateway;
			strScreenInform += SCR_CMD_DELIMITER;

			strScreenInform += strDNS;
			strScreenInform += SCR_CMD_DELIMITER;

			Operator_DisplayScreen(OP_SCREEN_TERMINAL_IP_SETTINGS, bKeyEnable, strScreenInform);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// IP 정보를 가져오지 못했을 경우에 대한 처리 추가
		// 현재 설정이 DHCP이고 User가 DHCP를 변경하지 않을 경우에만 갱신하도록 함
		if (CEagleDataManager::GetInstance()->m_Config.m_Host.strUseDHCP == S_ENABLE)
		{
			if (strDHCP == S_ENABLE)
			{
				CUtil::GetDHCP_IP(strTemp);

				if (strIP != strTemp)
				{
					strIP = strTemp;
					CUtil::GetDHCP_SubnetMask(strSubnet);
					CUtil::GetDHCP_GateWay(strGateway);
					CUtil::GetDHCP_DNS(strDNS);

					bShowScreen = TRUE;
					continue;
				}
			}
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_IPSettings - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_TERMINAL_NETWORK_SETTINGS;
				break;
			}

			// Touch로 Focus를 주기 위한 로직
			if (m_strSelectTouch.CompareNoCase(m_strScrOutData.Left(m_strSelectTouch.GetLength())) == 0)
			{
				nTempFocusIndex = CUtil::StringToInt(m_strScrOutData.Right(1));

				if( (nTempFocusIndex != 1) && (strDHCP == S_ENABLE) )
				{
					Operator_Processing_Screen(OP_GUIDE_TOUCH_INVALID);
					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

					bShowScreen = TRUE;
					continue;
				}

				m_nCurrentFocusIndex = nTempFocusIndex;
				m_strScrOutData = _T("SELECT");
			}
			///////////////////////////////////////////////////

			// Next Action
			if (m_strScrOutData == _T("PREV"))
			{
				m_nCurrentFocusIndex--;

				if (strDHCP == S_ENABLE)
				{
					m_nCurrentFocusIndex = 1;
				}
				else
				{
					if (m_nCurrentFocusIndex < 1)
						m_nCurrentFocusIndex = nMaxFocusIndex;
				}

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("NEXT"))
			{
				m_nCurrentFocusIndex++;

				if (strDHCP == S_ENABLE)
				{
					m_nCurrentFocusIndex = 1;
				}
				else
				{
					if (m_nCurrentFocusIndex > nMaxFocusIndex)
						m_nCurrentFocusIndex = 1;
				}

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("SELECT"))
			{
				// Index를 가지고 몇 번째 값을 설정해야 하는지 AP에서 확인한다.
				switch(m_nCurrentFocusIndex)
				{
				case 1:
					{
						if (strDHCP == S_ENABLE)
							strDHCP = S_DISABLE;
						else
							strDHCP = S_ENABLE;
					}
					break;

				case 2:	// IP
					{
						if (Operator_Input_Number(_T("IP"), strIP, m_strScrOutData, 15) == TRUE)
						{
							m_strScrOutData.TrimLeft();
							m_strScrOutData.TrimRight();
							strArray.RemoveAll();

							LOG(Info, _T("IP Return value : %s"), m_strScrOutData);
							// Validation Check
							CUtil::ParsingStringToStringArray(m_strScrOutData, _T("."), strArray);

							LOG(Info, _T("Parsing Count : %d"), strArray.GetCount());

							if (strArray.GetCount() == 4)
							{
								BOOL bInvalid = FALSE;

								for(int i=0; i<strArray.GetCount(); i++)
								{
									int nTemp = CUtil::StringToInt(strArray[i]);

									if ( nTemp < 0 || nTemp > 255)
									{
										bInvalid = TRUE;
										break;
									}
								}

								if (bInvalid == TRUE)
								{
									Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
									CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
								}
								else
								{
									strIP = m_strScrOutData;
								}
							}
							else
							{
								Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
								CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
							}
						}
					}
					break;

				case 3:	// SUBNET
					{
						if (Operator_Input_Number(_T("SUBNET"), strSubnet, m_strScrOutData, 15) == TRUE)
						{
							m_strScrOutData.TrimLeft();
							m_strScrOutData.TrimRight();
							strArray.RemoveAll();

							LOG(Info, _T("SubNet Return value : %s"), m_strScrOutData);
							// Validation Check
							CUtil::ParsingStringToStringArray(m_strScrOutData, _T("."), strArray);

							LOG(Info, _T("Parsing Count : %d"), strArray.GetCount());

							if (strArray.GetCount() == 4)
							{
								BOOL bInvalid = FALSE;

								for(int i=0; i<strArray.GetCount(); i++)
								{
									int nTemp = CUtil::StringToInt(strArray[i]);

									if ( nTemp < 0 || nTemp > 255)
									{
										bInvalid = TRUE;
										break;
									}
								}

								if (bInvalid == TRUE)
								{
									Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
									CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
								}
								else
								{
									strSubnet = m_strScrOutData;
								}
							}
							else
							{
								Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
								CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
							}
						}
					}
					break;

				case 4:	// GATEWAY
					{
						if (Operator_Input_Number(_T("GATEWAY"), strGateway, m_strScrOutData, 15) == TRUE)
						{
							m_strScrOutData.TrimLeft();
							m_strScrOutData.TrimRight();
							strArray.RemoveAll();

							LOG(Info, _T("GateWay Return value : %s"), m_strScrOutData);
							// Validation Check
							CUtil::ParsingStringToStringArray(m_strScrOutData, _T("."), strArray);

							LOG(Info, _T("Parsing Count : %d"), strArray.GetCount());

							if (strArray.GetCount() == 4)
							{
								BOOL bInvalid = FALSE;

								for(int i=0; i<strArray.GetCount(); i++)
								{
									int nTemp = CUtil::StringToInt(strArray[i]);

									if ( nTemp < 0 || nTemp > 255)
									{
										bInvalid = TRUE;
										break;
									}
								}

								if (bInvalid == TRUE)
								{
									Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
									CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
								}
								else
								{
									strGateway = m_strScrOutData;
								}
							}
							else
							{
								Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
								CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
							}
						}
					}
					break;

				case 5:	// DNS
					{
						if (Operator_Input_Number(_T("DNS"), strDNS, m_strScrOutData, 15) == TRUE)
						{
							m_strScrOutData.TrimLeft();
							m_strScrOutData.TrimRight();
							strArray.RemoveAll();

							LOG(Info, _T("DNS Return value : %s"), m_strScrOutData);
							// Validation Check
							CUtil::ParsingStringToStringArray(m_strScrOutData, _T("."), strArray);

							LOG(Info, _T("Parsing Count : %d"), strArray.GetCount());

							if (strArray.GetCount() == 4)
							{
								BOOL bInvalid = FALSE;

								for(int i=0; i<strArray.GetCount(); i++)
								{
									int nTemp = CUtil::StringToInt(strArray[i]);

									if ( nTemp < 0 || nTemp > 255)
									{
										bInvalid = TRUE;
										break;
									}
								}

								if (bInvalid == TRUE)
								{
									Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
									CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
								}
								else
								{
									strDNS = m_strScrOutData;
								}
							}
							else
							{
								Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
								CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
							}
						}
					}
					break;

				default:
					break;
				}

				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("SET"))
			{
				Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);

				CEagleDataManager::GetInstance()->m_Config.m_Host.strUseDHCP = strDHCP;
				CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("useDHCP"), CEagleDataManager::GetInstance()->m_Config.m_Host.strUseDHCP);

				// IP가 Static인 경우에만 설정된 IP정보를 저장
				if (strDHCP == S_DISABLE)
				{
					CEagleDataManager::GetInstance()->m_Config.m_Host.strTerminalip = strIP;
					CEagleDataManager::GetInstance()->m_Config.m_Host.strSubnet = strSubnet;
					CEagleDataManager::GetInstance()->m_Config.m_Host.strGateway = strGateway;
					CEagleDataManager::GetInstance()->m_Config.m_Host.strDNS = strDNS;

					CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("terminal_ip"), CEagleDataManager::GetInstance()->m_Config.m_Host.strTerminalip);
					CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("subnet"), CEagleDataManager::GetInstance()->m_Config.m_Host.strSubnet);
					CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("gateway"), CEagleDataManager::GetInstance()->m_Config.m_Host.strGateway);
					CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("DNS"), CEagleDataManager::GetInstance()->m_Config.m_Host.strDNS);
				}

				CDEV_Manager::GetInstance()->Set_Network_Config();

				// IP가 DHCP인 경우는 Network 설정 후에 IP 정보를 취득해서 화면 갱신하도록 함
				if (strDHCP == S_ENABLE)
				{
					strIP.Empty();
					CUtil::GetDHCP_IP(strIP);
					CEagleDataManager::GetInstance()->m_Config.m_Host.strTerminalip = strIP;

					strSubnet.Empty();
					CUtil::GetDHCP_SubnetMask(strSubnet);
					CEagleDataManager::GetInstance()->m_Config.m_Host.strSubnet = strSubnet;

					strGateway.Empty();
					CUtil::GetDHCP_GateWay(strGateway);
					CEagleDataManager::GetInstance()->m_Config.m_Host.strGateway = strGateway;

					strDNS.Empty();
					CUtil::GetDHCP_DNS(strDNS);
					CEagleDataManager::GetInstance()->m_Config.m_Host.strDNS = strDNS;

					CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("terminal_ip"), CEagleDataManager::GetInstance()->m_Config.m_Host.strTerminalip);
					CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("subnet"), CEagleDataManager::GetInstance()->m_Config.m_Host.strSubnet);
					CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("gateway"), CEagleDataManager::GetInstance()->m_Config.m_Host.strGateway);
					CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("DNS"), CEagleDataManager::GetInstance()->m_Config.m_Host.strDNS);
				}

				Operator_Processing_Screen(_T("SUCCESS"));
				CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

				m_nCurrentFocusIndex = 1;
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

	LOG(Info, _T("Proc_Operator_IPSettings - End"));
}


/** **********************************************************
*	@brief		Key Manangement
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_KeyManagement()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	CString strScreenInform;
	CString strButtonInform;

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (bShowScreen == TRUE)
		{
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();
			strScreenInform.Empty();
			strButtonInform.Empty();

			// MASTER인 경우에만 Key 주입 가능하도록 권한 변경
			if (GetOperatorMode() == EAGLE_ATM_SVC_MASTER)
				strButtonInform = _T("on");
			else
				strButtonInform = _T("off");
			strButtonInform += SCR_CMD_DELIMITER;

			// Screen Data
			Operator_DisplayScreen(OP_SCREEN_KEY_MANAGEMENT, bKeyEnable, strScreenInform, strButtonInform);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_KeyManagement - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_HOST;
				break;
			}

			// Next Action
			if (m_strScrOutData == _T("DOWNLOADKEYS"))
			{
				if (Operator_Confirm_Screen(_T("DOWNLOAD KEYS"), _T(""), _T("MASTER KEY MUST BE INJECTED")) == TRUE)
				{
					Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);
					
					CEagleTritonMsg::GetInstance()->Triton_MakeHostMsg(EAGLE_TRAN_CODE_DOWNLOAD_CONFIG);

					if (CEagleSVCLib::GetInstance()->SendRecvHost(EAGLE_TRAN_CODE_DOWNLOAD_CONFIG) == SUCCESS)
					{
						Operator_Processing_Screen(_T("SUCCESS"));
					}
					else
					{
						if (CEagleSVCLib::GetInstance()->Get_ErrorCode().Left(2) == _T("Z3"))
							CEagleSVCLib::GetInstance()->Clear_Error();

						Operator_Processing_Screen(_T("FAILED"));
					}

					// 수동으로 Download Keys를 수행한 경우에는 최초 수행한 걸로 Flag를 설정함.
					m_bExecDownloadKeysToHost_PowerOn = TRUE;

					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
				}

				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("MASTERKEYSMANAGEMENT"))
			{
				m_strNextScrNum = OP_SCREEN_MASTERKEYS_MANAGEMENT;
				break;
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
*	@brief		Master Keys Management
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_MasterKeysManagement()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	CString strScreenInform;
	CString strButtonInform;

	CString strTemp;
	int nActivatedStatus = 0;
	BOOL bPrevRemoveSensorState = FALSE;

	m_nActiveCmd = 0;

	// Removal Sensor 변경시 실시간 화면 갱신을 위해 로직 보완
	bPrevRemoveSensorState = CDEV_Manager::GetInstance()->m_DEV_CREPP.m_bRemovalSensorStatus;

	// Get Activation Status시 Delay로 인해 Processing Message 표시
	Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (bShowScreen == TRUE)
		{
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();
			strScreenInform.Empty();
			strButtonInform.Empty();

			// EPP Statue Check
			nActivatedStatus = CDEV_Manager::GetInstance()->m_DEV_CREPP.GetDeviceStatus();

			LOG(Info, _T("CURRENT ACTIVESTATUS : %d (%s)"), nActivatedStatus, CDEV_Manager::GetInstance()->m_DEV_CREPP.GetLastError());

			strScreenInform += GET_EPP_ACTIVATE__STRING(nActivatedStatus);
			strScreenInform += SCR_CMD_DELIMITER;

			if (bPrevRemoveSensorState == TRUE)
				strScreenInform += _T("NORMAL");
			else
				strScreenInform += _T("ERROR");
			strScreenInform += SCR_CMD_DELIMITER;

			if (nActivatedStatus == EPP_STATE_REMOVED)
			{
				// REMOVED 상태에서는 PREACTIVATED 와 ACTIVATED로 전환 가능
				strButtonInform += _T("off");
				strButtonInform += SCR_CMD_DELIMITER;
				strButtonInform += _T("off");
				strButtonInform += SCR_CMD_DELIMITER;
				strButtonInform += _T("on");
				strButtonInform += SCR_CMD_DELIMITER;
				strButtonInform += _T("on");
				strButtonInform += SCR_CMD_DELIMITER;
			}
			else if (nActivatedStatus == EPP_STATE_PREACTIVATED)
			{
				// PREACTIVATED 상태에서는 ACTIVATED만 가능
				strButtonInform += _T("off");
				strButtonInform += SCR_CMD_DELIMITER;
				strButtonInform += _T("off");
				strButtonInform += SCR_CMD_DELIMITER;
				strButtonInform += _T("off");
				strButtonInform += SCR_CMD_DELIMITER;
				strButtonInform += _T("on");
				strButtonInform += SCR_CMD_DELIMITER;
			}
			else if (nActivatedStatus == EPP_STATE_ACTIVATED)
			{
				// ACTIVATED 상태에서는 PREACTIVATED만 가능
				strButtonInform += _T("on");
				strButtonInform += SCR_CMD_DELIMITER;
				strButtonInform += _T("on");
				strButtonInform += SCR_CMD_DELIMITER;
				strButtonInform += _T("on");
				strButtonInform += SCR_CMD_DELIMITER;
				strButtonInform += _T("off");
				strButtonInform += SCR_CMD_DELIMITER;
			}
			else
			{
				strButtonInform += _T("off");
				strButtonInform += SCR_CMD_DELIMITER;
				strButtonInform += _T("off");
				strButtonInform += SCR_CMD_DELIMITER;
				strButtonInform += _T("off");
				strButtonInform += SCR_CMD_DELIMITER;
				strButtonInform += _T("off");
				strButtonInform += SCR_CMD_DELIMITER;
			}

			// Screen Data
			Operator_DisplayScreen(OP_SCREEN_MASTERKEYS_MANAGEMENT, bKeyEnable, strScreenInform, strButtonInform);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// Removal Sensor 상태 변경시 화면 갱신 처리
		if (bPrevRemoveSensorState != CDEV_Manager::GetInstance()->m_DEV_CREPP.m_bRemovalSensorStatus)
		{
			bPrevRemoveSensorState = CDEV_Manager::GetInstance()->m_DEV_CREPP.m_bRemovalSensorStatus;
			bShowScreen = TRUE;
			continue;
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_MasterKeysManagement - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_KEY_MANAGEMENT;
				break;
			}

			// Next Action
			if (m_strScrOutData == _T("KCVINFO"))
			{
				m_strNextScrNum = OP_SCREEN_MASTER_KEY_KCV_INFO;
				break;			
			}
			else if (m_strScrOutData == _T("INJECTMASTERKEY"))
			{
				// Confirmation Notice
				if (Operator_Confirm_Screen(_T("CONFIRMATION"), _T(""), _T("THE INJECTED MASTER KEY WILL BE DELETED AUTOMATICALLY.")) == TRUE)
				{
					// Remove Master Key
					CDEV_Manager::GetInstance()->m_DEV_CREPP.Remove_RegisteredMasterKey(MASTER_KEYNAME);

					CDEV_Manager::GetInstance()->SaveEJL_SetTextValue(text_field, _T("CLEARED MASTER KEY"));

					m_strNextScrNum = OP_SCREEN_INJECT_MASTER_KEY;
					break;
				}

				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("ACTIVATION"))
			{
				if (nActivatedStatus == EPP_STATE_REMOVED)
				{
					// Removed 상태에서 Activated로 가기 위해서는 NONCE / ID / ACTIVATION CODE 필요함
					m_nActiveCmd = CMD_DEVICE_ACTIVATE;

					m_strNextScrNum = OP_SCREEN_PINPAD_ACTIVATION;
					break;
				}
				else if (nActivatedStatus == EPP_STATE_PREACTIVATED)
				{
					// USER ID는 의미가 없으므로 Confirm 화면으로 대체

					 // PreActivated 상태에서 Activated로 가기 위해서는 ID만 입력하면 됨
					//if (Operator_Input_Text(_T("INPUT USER ID"), _T(""), m_strScrOutData, 8, UPPERCASE_MODE) == TRUE)
					//{
					//	m_strScrOutData.TrimLeft();
					//	m_strScrOutData.TrimRight();

					//	if (m_strScrOutData.IsEmpty() == FALSE)
					//	{
					//		if (CDEV_Manager::GetInstance()->m_DEV_CREPP.SetActivate(CMD_DEVICE_ACTIVATE, m_strScrOutData, _T("")) == TRUE)
					//		{
					//			Operator_Processing_Screen(_T("SUCCESS"));
					//			CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
					//		}
					//		else
					//		{
					//			strTemp.Format(_T("FAILED (%s)"), CDEV_Manager::GetInstance()->m_DEV_CREPP.GetLastError());
					//			LOG(Error, _T("Pre Activation is failed (%s"), strTemp);

					//			Operator_Processing_Screen(strTemp);
					//			CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
					//		}
					//	}
					//	else
					//	{
					//		Operator_Processing_Screen(_T("PLEASE INPUT USER ID"));
					//		CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
					//	}

					//}
					if (Operator_Confirm_Screen(_T("CONFIRMATION"), _T("DO YOU WANT TO 'ACTIVATE' PINPAD?")) == TRUE)
					{
						// Operator ID는 "12345678"로 Fix 처리
						if (CDEV_Manager::GetInstance()->m_DEV_CREPP.SetActivate(CMD_DEVICE_ACTIVATE, _T("12345678"), _T("")) == TRUE)
						{
							Operator_Processing_Screen(_T("SUCCESS"));
							CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
						}
						else
						{
							strTemp.Format(_T("FAILED (%s)"), CDEV_Manager::GetInstance()->m_DEV_CREPP.GetLastError());
							LOG(Error, _T("Activation is failed (%s"), strTemp);

							Operator_Processing_Screen(strTemp);
							CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
						}					
					}

					bShowScreen = TRUE;
				}
			}
			else if (m_strScrOutData == _T("PREACTIVATION"))
			{
				m_nActiveCmd = CMD_DEVICE_PREACTIVATE;

				m_strNextScrNum = OP_SCREEN_PINPAD_ACTIVATION;
				break;
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
*	@brief		PINPAD Activation
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_PNPAD_Activation()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	CString strEPP_SerialNo, strEPP_NonCE, strUserID = _T("12345678"), strActivationCode, strTemp;
	CString strScreenInform;
	CString strButtonInform;
	//int nMaxFocusIndex = 3;

	strEPP_SerialNo = CDEV_Manager::GetInstance()->m_DEV_CREPP.GetEPPSerialNo();

	// NONCE Value는 8자리만 가져오도록 로직 수정
	strTemp = CDEV_Manager::GetInstance()->m_DEV_CREPP.GetEPPNonCEValue();
	strEPP_NonCE = strTemp.Left(8);

	m_nCurrentFocusIndex = 4;

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (bShowScreen == TRUE)
		{
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();
			strScreenInform.Empty();

			strScreenInform += strEPP_SerialNo;
			strScreenInform += SCR_CMD_DELIMITER;

			strScreenInform += strEPP_NonCE;
			strScreenInform += SCR_CMD_DELIMITER;

			strScreenInform += strUserID;
			strScreenInform += SCR_CMD_DELIMITER;

			strScreenInform += strActivationCode;
			strScreenInform += SCR_CMD_DELIMITER;

			// TITLE
			if (m_nActiveCmd == CMD_DEVICE_PREACTIVATE)
				strScreenInform += _T("PREACTIVATION");
			else
				strScreenInform += _T("ACTIVATION");
			strScreenInform += SCR_CMD_DELIMITER;

			// Screen Data
			Operator_DisplayScreen(OP_SCREEN_PINPAD_ACTIVATION, bKeyEnable, strScreenInform);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_PNPAD_Activation - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_MASTERKEYS_MANAGEMENT;
				break;
			}

			// Touch로 Focus를 주기 위한 로직
			if (m_strSelectTouch.CompareNoCase(m_strScrOutData.Left(m_strSelectTouch.GetLength())) == 0)
			{
				m_nCurrentFocusIndex = CUtil::StringToInt(m_strScrOutData.Right(1));
				m_strScrOutData = _T("SELECT");
			}
			///////////////////////////////////////////////////

			// Next Action
			if (m_strScrOutData == _T("PREV"))
			{
				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("NEXT"))
			{
				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("SELECT"))
			{
				// Index를 가지고 몇 번째 값을 설정해야 하는지 AP에서 확인한다.
				switch(m_nCurrentFocusIndex)
				{
				case 4:	// Activation Code
					{
						//if (Operator_Input_Text(_T("ACTIVATION CODE"), strActivationCode, m_strScrOutData, 8, UPPERCASE_MODE) == TRUE)
						if (Operator_Input_HexaDecimal(_T("ACTIVATION CODE"), strActivationCode, m_strScrOutData, 8) == TRUE)
						{
							m_strScrOutData.TrimLeft();
							m_strScrOutData.TrimRight();

							strActivationCode = m_strScrOutData;
						}
					}
					break;

				default:
					break;
				}

				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("SET"))
			{
				if (strActivationCode.IsEmpty())
				{
					Operator_Processing_Screen(_T("PLEASE INPUT ACTIVATION CODE"));
					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
				}
				else if (strActivationCode.GetLength() != 8)
				{
					Operator_Processing_Screen(_T("PLEASE INPUT 8 DIGIT CODE"));
					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);					
				}
				else
				{
					// Execute Activation
					if (CDEV_Manager::GetInstance()->m_DEV_CREPP.SetActivate(m_nActiveCmd, strUserID,strActivationCode) == TRUE)
					{
						Operator_Processing_Screen(_T("SUCCESS"));
						CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

						m_strNextScrNum = OP_SCREEN_MASTERKEYS_MANAGEMENT;
						break;
					}

					strTemp.Format(_T("FAILED (%s)"), CDEV_Manager::GetInstance()->m_DEV_CREPP.GetLastError());
					Operator_Processing_Screen(strTemp);
					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

					LOG(Error, _T("Activation is failed (%s"), CDEV_Manager::GetInstance()->m_DEV_CREPP.GetLastError());
				}

				strActivationCode.Empty();
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
*	@brief		Inject Master Key
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_InjectMasterKey()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	CString strScreenInform;
	CString strButtonInform;
	CString strTemp;

	m_nInjectKeySequence = READY_INJECT_KEY_PART_A;

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (bShowScreen == TRUE)
		{
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();
			strScreenInform.Empty();
			strButtonInform.Empty();

			m_nCurrentFocusIndex = 0;

			if (m_nInjectKeySequence == READY_INJECT_KEY_PART_A)
			{
				// Idle Screen
				strScreenInform += _T("PRESS 'START ENTRY' BUTTON TO ENTER PART A");
				strScreenInform += SCR_CMD_DELIMITER;

				// Button Control
				strButtonInform += _T("off");
				strButtonInform += SCR_CMD_DELIMITER;
				strButtonInform += _T("on");
				strButtonInform += SCR_CMD_DELIMITER;

				// Screen Data (Normal Key Enable)
				Operator_DisplayScreen(OP_SCREEN_INJECT_MASTER_KEY, bKeyEnable, strScreenInform, strButtonInform);
			}
			else if (m_nInjectKeySequence == START_INJECT_PART_A)
			{
				// Screen Information
				strScreenInform += _T("ENTER PART A");
				strScreenInform += SCR_CMD_DELIMITER;

				// Button Control
				strButtonInform += _T("off");
				strButtonInform += SCR_CMD_DELIMITER;
				strButtonInform += _T("off");
				strButtonInform += SCR_CMD_DELIMITER;

				// Set Focus
				m_nCurrentFocusIndex = 2;

				// Screen Data
				Operator_DisplayScreen(OP_SCREEN_INJECT_MASTER_KEY, bKeyEnable, strScreenInform, strButtonInform, _T(""), TRUE);
			}
			else if (m_nInjectKeySequence == IMPORT_INJECT_PART_A)
			{
				// Screen Information
				strScreenInform += _T("PRESS 'IMPORT' BUTTON TO IMPORT PART A");
				strScreenInform += SCR_CMD_DELIMITER;

				// Button Control
				strButtonInform += _T("on");
				strButtonInform += SCR_CMD_DELIMITER;
				strButtonInform += _T("off");
				strButtonInform += SCR_CMD_DELIMITER;

				bKeyEnable = TRUE;

				// Screen Data
				Operator_DisplayScreen(OP_SCREEN_INJECT_MASTER_KEY, bKeyEnable, strScreenInform, strButtonInform);
			}
			else if (m_nInjectKeySequence == READY_INJECT_KEY_PART_B)
			{
				// Screen Information
				strScreenInform += _T("PRESS 'START ENTRY' BUTTON TO ENTER PART B");
				strScreenInform += SCR_CMD_DELIMITER;

				// Button Control
				strButtonInform += _T("off");
				strButtonInform += SCR_CMD_DELIMITER;
				strButtonInform += _T("on");
				strButtonInform += SCR_CMD_DELIMITER;

				bKeyEnable = TRUE;

				// Screen Data
				Operator_DisplayScreen(OP_SCREEN_INJECT_MASTER_KEY, bKeyEnable, strScreenInform, strButtonInform);
			}

			else if (m_nInjectKeySequence == START_INJECT_PART_B)
			{
				// Screen Information
				strScreenInform += _T("ENTER PART B");
				strScreenInform += SCR_CMD_DELIMITER;

				// Button Control
				strButtonInform += _T("off");
				strButtonInform += SCR_CMD_DELIMITER;
				strButtonInform += _T("off");
				strButtonInform += SCR_CMD_DELIMITER;

				// Set Focus
				m_nCurrentFocusIndex = 2;

				// Screen Data
				Operator_DisplayScreen(OP_SCREEN_INJECT_MASTER_KEY, bKeyEnable, strScreenInform, strButtonInform, _T(""), TRUE);
			}
			else if (m_nInjectKeySequence == IMPORT_INJECT_PART_B)
			{
				// Screen Information
				strScreenInform += _T("PRESS 'IMPORT' BUTTON TO IMPORT PART B");
				strScreenInform += SCR_CMD_DELIMITER;

				// Button Control
				strButtonInform += _T("on");
				strButtonInform += SCR_CMD_DELIMITER;
				strButtonInform += _T("off");
				strButtonInform += SCR_CMD_DELIMITER;

				bKeyEnable = TRUE;

				// Screen Data
				Operator_DisplayScreen(OP_SCREEN_INJECT_MASTER_KEY, bKeyEnable, strScreenInform, strButtonInform);
			}


			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_InjectMasterKey - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_EXIT)
			{
				CrypteraEPP_StopClearTextMode();

				m_strNextScrNum = OP_SCREEN_MASTERKEYS_MANAGEMENT;
				break;
			}
			else if (m_strScrOutData == S_TIMEOVER)
			{
				CrypteraEPP_StopClearTextMode();

				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}

			// Next Action
			if (m_strScrOutData == _T("STARTENTRY"))
			{
				// Pin Mode Key Enable
				CrypteraEPP_StopClearTextMode();

				if (CDEV_Manager::GetInstance()->m_DEV_CREPP.StartEntry_Master_PartKey() == FALSE)
				{
					// Pin Mode Disable
					CrypteraEPP_StopClearTextMode();

					strTemp.Format(_T("EXECUTE IS FAILED(%s)\nPLEASE TRY AGAIN"), CDEV_Manager::GetInstance()->m_DEV_CREPP.GetLastError());
					LOG(Error, _T("StartEntry_Master_PartKey is failed (%s"), strTemp);

					Operator_Processing_Screen(_T(""), TRUE, strTemp);

					// 진행 중 실패시 PART A / B가 꼬일 수 있으므로 처음부터 다시 시도하도록 이전 화면으로 전환
					m_strNextScrNum = OP_SCREEN_MASTERKEYS_MANAGEMENT;
					break;
				}

				if (m_nInjectKeySequence == READY_INJECT_KEY_PART_A)
				{
					m_nInjectKeySequence = START_INJECT_PART_A;
				}
				else if (m_nInjectKeySequence == READY_INJECT_KEY_PART_B)
				{
					m_nInjectKeySequence = START_INJECT_PART_B;
				}
				else
				{
					// Sequence Error
					Operator_Processing_Screen(_T("SEQUENCE ERROR"));
					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

					m_strNextScrNum = OP_SCREEN_MASTERKEYS_MANAGEMENT;
					break;
				}

				bShowScreen = TRUE;
				
			}
			else if (m_strScrOutData == _T("IMPORTKEY"))
			{
				if (CDEV_Manager::GetInstance()->m_DEV_CREPP.Store_Master_PartKey() == FALSE)
				{
					strTemp.Format(_T("EXECUTE IS FAILED(%s)\nPLEASE TRY AGAIN"), CDEV_Manager::GetInstance()->m_DEV_CREPP.GetLastError());
					LOG(Error, _T("Store_Master_PartKey is failed (%s"), strTemp);

					Operator_Processing_Screen(_T(""), TRUE, strTemp);

					m_strNextScrNum = OP_SCREEN_MASTERKEYS_MANAGEMENT;
					break;
				}

				if (m_nInjectKeySequence == IMPORT_INJECT_PART_A)
				{
					m_nInjectKeySequence = READY_INJECT_KEY_PART_B;
				}
				else if (m_nInjectKeySequence == IMPORT_INJECT_PART_B)
				{
					// Final Store Execute
					if (CDEV_Manager::GetInstance()->m_DEV_CREPP.Store_Master_FinalKey() == FALSE)
					{
						strTemp.Format(_T("EXECUTE IS FAILED(%s)\nPLEASE TRY AGAIN"), CDEV_Manager::GetInstance()->m_DEV_CREPP.GetLastError());
						LOG(Error, _T("Store_Master_FinalKey is failed (%s"), strTemp);

						Operator_Processing_Screen(_T(""), TRUE, strTemp);

						m_strNextScrNum = OP_SCREEN_MASTERKEYS_MANAGEMENT;
						break;
					}

					// Get KCV
					strTemp = CDEV_Manager::GetInstance()->m_DEV_CREPP.Get_KeyCheckValue(MASTER_KEYNAME);

					LOG(Info, _T("Master KCV (%s"), strTemp);

					CString strMasterKey;
					
					if (strTemp.IsEmpty() == FALSE)
					{
						strMasterKey.Format(_T("MASTER KEY CHECKSUM = %s %s"), strTemp.Left(2), strTemp.Mid(2,2));
						strTemp.Format(_T("SUCCESS (%s %s)"), strTemp.Left(2), strTemp.Mid(2,2));

						CDEV_Manager::GetInstance()->SaveEJL_SetChangeValue(_T("MASTER KEY CHECKSUM = NONE"), strMasterKey);

						Operator_Processing_Screen(_T(""), TRUE, strTemp);

						m_strNextScrNum = OP_SCREEN_MASTERKEYS_MANAGEMENT;
						break;
					}
					else
					{
						strTemp = _T("FAILED");
						Operator_Processing_Screen(_T(""), TRUE, strTemp);
					}

					// Sequence 초기화
					m_nInjectKeySequence = READY_INJECT_KEY_PART_A;
					bKeyEnable = TRUE;
				}

				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("ENTER"))
			{
				if (m_nInjectKeySequence == START_INJECT_PART_A)
					m_nInjectKeySequence = IMPORT_INJECT_PART_A;
				else if (m_nInjectKeySequence == START_INJECT_PART_B)
					m_nInjectKeySequence = IMPORT_INJECT_PART_B;

				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("ERROR"))
			{
				// Pin Mode Disable
				CrypteraEPP_StopClearTextMode();

				Operator_Processing_Screen(_T(""), TRUE, _T("EXECUTE IS FAILED\nPLEASE TRY AGAIN"));

				m_strNextScrNum = OP_SCREEN_MASTERKEYS_MANAGEMENT;
				break;
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

	m_strMasterKey_KCV = CDEV_Manager::GetInstance()->m_DEV_CREPP.Get_KeyCheckValue(MASTER_KEYNAME);

	LOG(Info, _T("CREPP - Update Master Key KCV : %s"), m_strMasterKey_KCV);

	// 초기화
	m_nInjectKeySequence = READY_INJECT_KEY_PART_A;
}


/** **********************************************************
*	@brief		Master Key KCV Info.
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_MasterKey_KCV_Info()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	CString strScreenInform;
	CString strTemp, strMasterKeyKCV, strPINKeyKCV;

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (bShowScreen == TRUE)
		{
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();
			strScreenInform.Empty();

			strMasterKeyKCV = CDEV_Manager::GetInstance()->m_DEV_CREPP.Get_KeyCheckValue(MASTER_KEYNAME);
			strPINKeyKCV = CDEV_Manager::GetInstance()->m_DEV_CREPP.Get_KeyCheckValue(WORKING_KEYNAME);

			// Screen Information
			if (strMasterKeyKCV.IsEmpty() == FALSE)
				strTemp.Format(_T("%s %s"), strMasterKeyKCV.Left(2), strMasterKeyKCV.Mid(2,2));
			else
				strTemp.Format(_T("EMPTY"));

			strScreenInform += strTemp;
			strScreenInform += SCR_CMD_DELIMITER;

			if (strPINKeyKCV.IsEmpty() == FALSE)
				strTemp.Format(_T("%s %s"), strPINKeyKCV.Left(2), strPINKeyKCV.Mid(2,2));
			else
				strTemp.Format(_T("EMPTY"));

			strScreenInform += strTemp;
			strScreenInform += SCR_CMD_DELIMITER;

			// Screen Data
			Operator_DisplayScreen(OP_SCREEN_MASTER_KEY_KCV_INFO, bKeyEnable, strScreenInform);

			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_MasterKey_KCV_Info - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_MASTERKEYS_MANAGEMENT;
				break;
			}
			else if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
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
*	@brief		Configure Connection
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_ConfigureConnection()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	CString strScreenInform;
	CString strButtonInform;
	CString strTemp, strTerminal_ID, strCommHeader, strCommHeader_ID, strCRC, strTLS, strProtocol_Type;
	int nMaxFocusIndex = 6;
	CStringArray	strArray;
	int nTempFocusIndex = 0;

	// Focus Setting
	m_nCurrentFocusIndex = 1;

	strTerminal_ID = CEagleDataManager::GetInstance()->m_Config.m_Option.strTerminal_ID;
	strTerminal_ID.TrimLeft();
	strTerminal_ID.TrimLeft();
	strCommHeader = CEagleDataManager::GetInstance()->m_Config.m_Host.strCommunication_header;
	strCommHeader_ID = CEagleDataManager::GetInstance()->m_Config.m_Host.strCommunicationID;
	strCommHeader_ID.TrimLeft();
	strCommHeader_ID.TrimRight();
	strCRC = CEagleDataManager::GetInstance()->m_Config.m_Host.strCrc_flg;
	strTLS = CEagleDataManager::GetInstance()->m_Config.m_Host.strUseTLS;
	strProtocol_Type = CEagleDataManager::GetInstance()->m_Config.m_Host.strProtocolType;
	strProtocol_Type.MakeUpper();

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (bShowScreen == TRUE)
		{
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();
			strScreenInform.Empty();
			strButtonInform.Empty();

			// Screen Data
			strScreenInform = strTerminal_ID;
			strScreenInform += SCR_CMD_DELIMITER;

			strScreenInform += GET_ENDISABLE_STRING(strCommHeader);
			strScreenInform += SCR_CMD_DELIMITER;

			strScreenInform += strCommHeader_ID;
			strScreenInform += SCR_CMD_DELIMITER;

			strScreenInform += GET_ENDISABLE_STRING(strCRC);
			strScreenInform += SCR_CMD_DELIMITER;

			strScreenInform += GET_ENDISABLE_STRING(strTLS);
			strScreenInform += SCR_CMD_DELIMITER;

			strScreenInform += strProtocol_Type;
			strScreenInform += SCR_CMD_DELIMITER;

			// Button Information
			if (CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType == _T("TCP/IP"))
			{
				strScreenInform += _T("HOST ADDRESS");
			}
			else
			{
				strScreenInform += _T("HOST");
				strScreenInform += SCR_LINEFEED_DELEMITER;
				strScreenInform += _T("PHONE NUMBER");
			}
			strScreenInform += SCR_CMD_DELIMITER;

			// Button Control
			if (CEagleDataManager::GetInstance()->m_Config.m_Host.strProtocolType == _T("VISA"))
				strButtonInform = _T("on");
			else
				strButtonInform = _T("off");

			Operator_DisplayScreen(OP_SCREEN_CONFIGURE_CONNECTION, bKeyEnable, strScreenInform, strButtonInform);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_ConfigureConnection - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_HOST;
				break;
			}

			// Touch로 Focus를 주기 위한 로직
			if (m_strSelectTouch.CompareNoCase(m_strScrOutData.Left(m_strSelectTouch.GetLength())) == 0)
			{
				nTempFocusIndex = CUtil::StringToInt(m_strScrOutData.Right(1));

				if( (3 == nTempFocusIndex) && (S_DISABLE == strCommHeader) )
				{
					Operator_Processing_Screen(OP_GUIDE_TOUCH_INVALID);
					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

					bShowScreen = TRUE;
					continue;
				}
				else if( ((5 == nTempFocusIndex) || (6 == nTempFocusIndex)) && 
						 (CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType == _T("MODEM")) )
				{
					Operator_Processing_Screen(OP_GUIDE_TOUCH_INVALID);
					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

					bShowScreen = TRUE;
					continue;
				}

				m_nCurrentFocusIndex = nTempFocusIndex;
				m_strScrOutData = _T("SELECT");
			}
			///////////////////////////////////////////////////

			// Next Action
			if (m_strScrOutData == _T("PREV"))
			{
				m_nCurrentFocusIndex--;

				// 3번 Comm ID 및 6번은 조건에 따라 다르게 처리

				if (m_nCurrentFocusIndex < 1)
				{
					// 1번인 경우 6번 처리
					if (CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType == _T("TCP/IP"))
						m_nCurrentFocusIndex = nMaxFocusIndex;
					else
						m_nCurrentFocusIndex = nMaxFocusIndex - 2;
				}
				else if (m_nCurrentFocusIndex == 3)
				{
					if (strCommHeader == S_DISABLE)
						m_nCurrentFocusIndex--;
				}

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("NEXT"))
			{
				m_nCurrentFocusIndex++;

				if (m_nCurrentFocusIndex > nMaxFocusIndex)
				{
					m_nCurrentFocusIndex = 1;
				}
				else
				{
					// 3번인 경우 처리
					if (m_nCurrentFocusIndex == 3)
					{
						if (strCommHeader == S_DISABLE)
							m_nCurrentFocusIndex++;
					}
					else if (m_nCurrentFocusIndex >=5)
					{
						if (CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType == _T("MODEM"))
							m_nCurrentFocusIndex = 1;
					}
				}

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("SELECT"))
			{
				// Index를 가지고 몇 번째 값을 설정해야 하는지 AP에서 확인한다.
				switch(m_nCurrentFocusIndex)
				{
				case 1:	// Terminal ID
					{
						if (Operator_Input_Text(_T("TERMINAL ID"), strTerminal_ID, m_strScrOutData, 15, UPPERCASE_MODE) == TRUE)
						{
							m_strScrOutData.TrimLeft();
							m_strScrOutData.TrimRight();

							strTerminal_ID = m_strScrOutData;
						}
					}
					break;

				case 2:	// Comm Header
					{
						if (strCommHeader == S_ENABLE)
							strCommHeader = S_DISABLE;
						else
							strCommHeader = S_ENABLE;
					}
					break;

				case 3:	// Comm Header ID
					{
						if (Operator_Input_Text(_T("COMM HEADER ID"), strCommHeader_ID, m_strScrOutData, 8) == TRUE)
						{
							m_strScrOutData.TrimLeft();
							m_strScrOutData.TrimRight();
							
							strCommHeader_ID = m_strScrOutData;
						}
					}
					break;

				case 4:	// CRC
					{
						if (strCRC == S_ENABLE)
							strCRC = S_DISABLE;
						else
							strCRC = S_ENABLE;
					}
					break;

				case 5:	// TLS
					{
						if (strTLS == S_ENABLE)
							strTLS = S_DISABLE;
						else
							strTLS = S_ENABLE;
					}
					break;

				case 6:	// Protocol Type
					{
						if (strProtocol_Type == _T("VISA"))
							strProtocol_Type = _T("STANDARD");
						else
							strProtocol_Type = _T("VISA");
					}
					break;

				default:
					break;
				}

				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("SET"))
			{
				Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);

				CEagleDataManager::GetInstance()->m_Config.m_Option.strTerminal_ID = strTerminal_ID;
				CEagleDataManager::GetInstance()->m_Config.m_Host.strCommunication_header = strCommHeader;
				CEagleDataManager::GetInstance()->m_Config.m_Host.strCommunicationID = strCommHeader_ID;
				CEagleDataManager::GetInstance()->m_Config.m_Host.strCrc_flg = strCRC;
				CEagleDataManager::GetInstance()->m_Config.m_Host.strUseTLS = strTLS;
				CEagleDataManager::GetInstance()->m_Config.m_Host.strProtocolType = strProtocol_Type;

				CEagleDataManager::GetInstance()->m_Config.SaveOptionData(_T("terminal_id"), CEagleDataManager::GetInstance()->m_Config.m_Option.strTerminal_ID);
				CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("comm_header_en"), CEagleDataManager::GetInstance()->m_Config.m_Host.strCommunication_header);
				CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("comm_id"), CEagleDataManager::GetInstance()->m_Config.m_Host.strCommunicationID);
				CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("crc_flg"), CEagleDataManager::GetInstance()->m_Config.m_Host.strCrc_flg);
				CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("useTLS"), CEagleDataManager::GetInstance()->m_Config.m_Host.strUseTLS);
				CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("protocol_type"), CEagleDataManager::GetInstance()->m_Config.m_Host.strProtocolType);

				CDEV_Manager::GetInstance()->Set_Network_Config(FALSE);

				Operator_Processing_Screen(_T("SUCCESS"));
				CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

				m_nCurrentFocusIndex = 1;
				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("PROTOCOLOPTION"))
			{
				m_strNextScrNum = OP_SCREEN_PROTOCOL_OPTION;
				break;
			}
			else if (m_strScrOutData == _T("HOSTINFO"))
			{
				if (CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType == _T("TCP/IP"))
					m_strNextScrNum = OP_SCREEN_HOST_IP_ADDRESS;
				else
					m_strNextScrNum = OP_SCREEN_HOST_PHONENUMBER;
				break;
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
*	@brief		Protocol Option
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_ProtocolOption()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	CString strScreenInform;
	CString strButtonInform;
	CString strTemp, strUseENQ, strUseEOT;
	int nMaxFocusIndex = 2;

	// Focus Setting
	m_nCurrentFocusIndex = 1;

	strUseENQ = CEagleDataManager::GetInstance()->m_Config.m_Host.strUseENQ;
	strUseEOT = CEagleDataManager::GetInstance()->m_Config.m_Host.strUseEOT;

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (bShowScreen == TRUE)
		{
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();
			strScreenInform.Empty();

			// Screen Data
			strScreenInform = GET_REQUIREDORNOT_STRING(strUseENQ);
			strScreenInform += SCR_CMD_DELIMITER;

			strScreenInform += GET_OPTION_STRING(strUseEOT);
			strScreenInform += SCR_CMD_DELIMITER;

			Operator_DisplayScreen(OP_SCREEN_PROTOCOL_OPTION, bKeyEnable, strScreenInform);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_ProtocolOption - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_CONFIGURE_CONNECTION;
				break;
			}

			// Touch로 Focus를 주기 위한 로직
			if (m_strSelectTouch.CompareNoCase(m_strScrOutData.Left(m_strSelectTouch.GetLength())) == 0)
			{
				m_nCurrentFocusIndex = CUtil::StringToInt(m_strScrOutData.Right(1));
				m_strScrOutData = _T("SELECT");
			}
			///////////////////////////////////////////////////

			// Next Action
			if (m_strScrOutData == _T("PREV"))
			{
				m_nCurrentFocusIndex--;

				// 3번 Comm ID 및 5번은 조건에 따라 다르게 처리

				if (m_nCurrentFocusIndex < 1)
				{
					m_nCurrentFocusIndex = nMaxFocusIndex;
				}

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("NEXT"))
			{
				m_nCurrentFocusIndex++;

				if (m_nCurrentFocusIndex > nMaxFocusIndex)
				{
					m_nCurrentFocusIndex = 1;
				}

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("SELECT"))
			{
				// Index를 가지고 몇 번째 값을 설정해야 하는지 AP에서 확인한다.
				switch(m_nCurrentFocusIndex)
				{
				case 1:	// Use ENQ
					{
						if (strUseENQ == S_ENABLE)
							strUseENQ = S_DISABLE;
						else
							strUseENQ = S_ENABLE;
					}
					break;

				case 2:	// Use EOT
					{
						if (strUseEOT == S_DISABLE)
							strUseEOT = S_ENABLE;
						else if (strUseEOT == S_ENABLE)
							strUseEOT = _T("2");
						else
							strUseEOT = S_DISABLE;
					}
					break;

				default:
					break;
				}

				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("SET"))
			{
				CEagleDataManager::GetInstance()->m_Config.m_Host.strUseENQ = strUseENQ;
				CEagleDataManager::GetInstance()->m_Config.m_Host.strUseEOT = strUseEOT;

				CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("useENQ"), CEagleDataManager::GetInstance()->m_Config.m_Host.strUseENQ);
				CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("useEOT"), CEagleDataManager::GetInstance()->m_Config.m_Host.strUseEOT);

				Operator_Processing_Screen(_T("SUCCESS"));
				CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

				m_nCurrentFocusIndex = 1;
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
*	@brief		Host Phone Number
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_HostPhoneNumber()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	CString strScreenInform;
	CString strButtonInform;
	CString strTemp, strPrimaryNumber, strBackupNumber, strPreDialEnable;
	int nMaxFocusIndex = 3;

	// Focus Setting
	m_nCurrentFocusIndex = 1;

	strPrimaryNumber = CEagleDataManager::GetInstance()->m_Config.m_Host.strPrimary_phone_number;
	strBackupNumber = CEagleDataManager::GetInstance()->m_Config.m_Host.strBackup_phone_number;
	strPreDialEnable = CEagleDataManager::GetInstance()->m_Config.m_Host.strPre_dial;

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (bShowScreen == TRUE)
		{
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();
			strScreenInform.Empty();

			// Screen Data
			strScreenInform = strPrimaryNumber;
			strScreenInform += SCR_CMD_DELIMITER;

			strScreenInform += strBackupNumber;
			strScreenInform += SCR_CMD_DELIMITER;

			strScreenInform += GET_ENDISABLE_STRING(strPreDialEnable);
			strScreenInform += SCR_CMD_DELIMITER;

			Operator_DisplayScreen(OP_SCREEN_HOST_PHONENUMBER, bKeyEnable, strScreenInform);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_HostPhoneNumber - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_CONFIGURE_CONNECTION;
				break;
			}

			// Touch로 Focus를 주기 위한 로직
			if (m_strSelectTouch.CompareNoCase(m_strScrOutData.Left(m_strSelectTouch.GetLength())) == 0)
			{
				m_nCurrentFocusIndex = CUtil::StringToInt(m_strScrOutData.Right(1));
				m_strScrOutData = _T("SELECT");
			}
			///////////////////////////////////////////////////

			// Next Action
			if (m_strScrOutData == _T("PREV"))
			{
				m_nCurrentFocusIndex--;

				if (m_nCurrentFocusIndex < 1)
				{
					m_nCurrentFocusIndex = nMaxFocusIndex;
				}

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("NEXT"))
			{
				m_nCurrentFocusIndex++;

				if (m_nCurrentFocusIndex > nMaxFocusIndex)
				{
					m_nCurrentFocusIndex = 1;
				}

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("SELECT"))
			{
				// Index를 가지고 몇 번째 값을 설정해야 하는지 AP에서 확인한다.
				switch(m_nCurrentFocusIndex)
				{
				case 1:	// Primary number
					{
						if (Operator_Input_Number(_T("PRIMARY NUMBER"), strPrimaryNumber, m_strScrOutData) == TRUE)
						{
							m_strScrOutData.TrimLeft();
							m_strScrOutData.TrimRight();
							strPrimaryNumber = m_strScrOutData;
						}
					}
					break;

				case 2:	// BackUp Number
					{
						if (Operator_Input_Number(_T("BACKUP NUMBER"), strBackupNumber, m_strScrOutData) == TRUE)
						{
							m_strScrOutData.TrimLeft();
							m_strScrOutData.TrimRight();
							strBackupNumber = m_strScrOutData;
						}
					}
					break;

				case 3:	// Use PreDial
					{
						if (strPreDialEnable == S_ENABLE)
							strPreDialEnable = S_DISABLE;
						else
							strPreDialEnable = S_ENABLE;

					}
					break;

				default:
					break;
				}

				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("SET"))
			{
				CEagleDataManager::GetInstance()->m_Config.m_Host.strPrimary_phone_number = strPrimaryNumber;
				CEagleDataManager::GetInstance()->m_Config.m_Host.strBackup_phone_number = strBackupNumber;
				CEagleDataManager::GetInstance()->m_Config.m_Host.strPre_dial = strPreDialEnable;

				CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("primary_num"), CEagleDataManager::GetInstance()->m_Config.m_Host.strPrimary_phone_number);
				CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("backup_num"), CEagleDataManager::GetInstance()->m_Config.m_Host.strBackup_phone_number);
				CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("predial_en"), CEagleDataManager::GetInstance()->m_Config.m_Host.strPre_dial);

				Operator_Processing_Screen(_T("SUCCESS"));
				CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

				m_nCurrentFocusIndex = 1;
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
*	@brief		Host IP address
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_HostIPAddress()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	CString strScreenInform;
	CString strButtonInform;
	CString strTemp, strType, strPrimaryHostIp, strPrimaryHostPort, strBackupHostIp, strBackupHostPort;
	int nMaxFocusIndex = 5;

	// Focus Setting
	m_nCurrentFocusIndex = 1;

	strType = _T("0");	// IP TYPE
	strPrimaryHostIp = CEagleDataManager::GetInstance()->m_Config.m_Host.strHostip;
	strPrimaryHostIp.TrimLeft();
	strPrimaryHostIp.TrimRight();
	strPrimaryHostPort.Format(_T("%d"), CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Host.strPort));

	strBackupHostIp = CEagleDataManager::GetInstance()->m_Config.m_Host.strBackup_Hostip;
	strBackupHostIp.TrimLeft();
	strBackupHostIp.TrimRight();
	strBackupHostPort.Format(_T("%d"), CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Host.strBackupPort));

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (bShowScreen == TRUE)
		{
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();
			strScreenInform.Empty();

			// Screen Data
			if (strType == _T("0"))
				strScreenInform += _T("IP");
			else
				strScreenInform += _T("URL");
			strScreenInform += SCR_CMD_DELIMITER;

			strScreenInform += strPrimaryHostIp;
			strScreenInform += SCR_CMD_DELIMITER;

			strScreenInform += strPrimaryHostPort;
			strScreenInform += SCR_CMD_DELIMITER;

			strScreenInform += strBackupHostIp;
			strScreenInform += SCR_CMD_DELIMITER;

			strScreenInform += strBackupHostPort;
			strScreenInform += SCR_CMD_DELIMITER;

			Operator_DisplayScreen(OP_SCREEN_HOST_IP_ADDRESS, bKeyEnable, strScreenInform);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_HostIPAddress - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_CONFIGURE_CONNECTION;
				break;
			}

			// Touch로 Focus를 주기 위한 로직
			if (m_strSelectTouch.CompareNoCase(m_strScrOutData.Left(m_strSelectTouch.GetLength())) == 0)
			{
				m_nCurrentFocusIndex = CUtil::StringToInt(m_strScrOutData.Right(1));
				m_strScrOutData = _T("SELECT");
			}
			///////////////////////////////////////////////////

			// Next Action
			if (m_strScrOutData == _T("PREV"))
			{
				m_nCurrentFocusIndex--;

				if (m_nCurrentFocusIndex < 1)
				{
					m_nCurrentFocusIndex = nMaxFocusIndex;
				}

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("NEXT"))
			{
				m_nCurrentFocusIndex++;

				if (m_nCurrentFocusIndex > nMaxFocusIndex)
				{
					m_nCurrentFocusIndex = 1;
				}

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("SELECT"))
			{
				// Index를 가지고 몇 번째 값을 설정해야 하는지 AP에서 확인한다.
				switch(m_nCurrentFocusIndex)
				{
				case 1:
					{
						if (strType == _T("0"))
							strType = _T("1");
						else
							strType = _T("0");
					}
					break;

				case 2:	// Primary address
					{
						if (strType == _T("0"))
						{
							if (Operator_Input_Number(_T("PRIMARY IP"), strPrimaryHostIp, m_strScrOutData, 15) == TRUE)
							{
								m_strScrOutData.TrimLeft();
								m_strScrOutData.TrimRight();
								strPrimaryHostIp = m_strScrOutData;
							}
						}
						else
						{
							if (Operator_Input_Text(_T("PRIMARY ADDRESS"), strPrimaryHostIp, m_strScrOutData) == TRUE)
							{
								m_strScrOutData.TrimLeft();
								m_strScrOutData.TrimRight();
								strPrimaryHostIp = m_strScrOutData;
							}
						}
					}
					break;

				case 3:	// Primary port
					{
						CString strGuideText;
						strGuideText.Format(_T("%s %d"), OP_GUIDE_MAX_INPUT_VAL, 65535);

						// Port 정보가 0이하인 경우는 입력을 편하게 하기 위해 Null로 표시하도록 보완 처리
						if (CUtil::StringToInt(strPrimaryHostPort) <= 0)
							strTemp.Empty();
						else
							strTemp = strPrimaryHostPort;

						if (Operator_Input_Number(_T("PRIMARY PORT"), strTemp, m_strScrOutData, 5, NORMAL_TYPE, strGuideText) == TRUE)
						{
							m_strScrOutData.TrimLeft();
							m_strScrOutData.TrimRight();

							int nPort = CUtil::StringToInt(m_strScrOutData);

							if (nPort < 0 || nPort > 65535)	// 0도 입력이 가능하도록 로직 수정
							{
								LOG(Error, _T("Proc_Operator_HostIPAddress - User Key Input Invalid : %d"), nPort);

								Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
								CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
							}
							else
							{
								strPrimaryHostPort.Format(_T("%d"), nPort);
							}
						}
					}
					break;

				case 4:	// Backup address
					{
						if (strType == _T("0"))
						{
							if (Operator_Input_Number(_T("BACKUP IP"), strBackupHostIp, m_strScrOutData, 15) == TRUE)
							{
								m_strScrOutData.TrimLeft();
								m_strScrOutData.TrimRight();
								strBackupHostIp = m_strScrOutData;
							}
						}
						else
						{
							if (Operator_Input_Text(_T("BACKUP ADDRESS"), strBackupHostIp, m_strScrOutData) == TRUE)
							{
								m_strScrOutData.TrimLeft();
								m_strScrOutData.TrimRight();
								strBackupHostIp = m_strScrOutData;
							}
						}
					}
					break;

				case 5:	// Backup port
					{
						CString strGuideText;
						strGuideText.Format(_T("%s %d"), OP_GUIDE_MAX_INPUT_VAL, 65535);

						// Port 정보가 0이하인 경우는 입력을 편하게 하기 위해 Null로 표시하도록 보완 처리
						if (CUtil::StringToInt(strBackupHostPort) <= 0)
							strTemp.Empty();
						else
							strTemp = strBackupHostPort;

						if (Operator_Input_Number(_T("BACKUP PORT"), strTemp, m_strScrOutData, 5, NORMAL_TYPE, strGuideText) == TRUE)
						{
							m_strScrOutData.TrimLeft();
							m_strScrOutData.TrimRight();

							int nPort = CUtil::StringToInt(m_strScrOutData);

							if (nPort < 0 || nPort > 65535)		// 0도 입력이 가능하도록 로직 수정
							{
								LOG(Error, _T("Proc_Operator_HostIPAddress - User Key Input Invalid : %d"), nPort);

								Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
								CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
							}
							else
							{
								strBackupHostPort.Format(_T("%d"), nPort);
							}
						}
					}
					break;

				default:
					break;
				}

				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("SET"))
			{
				// Port 정보가 Null일 경우 0으로 처리하도록 로직 보완
				if (strPrimaryHostPort.IsEmpty())
					strPrimaryHostPort = _T("0");

				if (strBackupHostPort.IsEmpty())
					strBackupHostPort = _T("0");

				CEagleDataManager::GetInstance()->m_Config.m_Host.strHostip = strPrimaryHostIp;
				CEagleDataManager::GetInstance()->m_Config.m_Host.strPort = strPrimaryHostPort;
				CEagleDataManager::GetInstance()->m_Config.m_Host.strBackup_Hostip = strBackupHostIp;
				CEagleDataManager::GetInstance()->m_Config.m_Host.strBackupPort = strBackupHostPort;

				CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("host_ip"), CEagleDataManager::GetInstance()->m_Config.m_Host.strHostip);
				CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("port"), CEagleDataManager::GetInstance()->m_Config.m_Host.strPort);
				CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("backup_host_ip"), CEagleDataManager::GetInstance()->m_Config.m_Host.strBackup_Hostip);
				CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("backup_port"), CEagleDataManager::GetInstance()->m_Config.m_Host.strBackupPort);

				Operator_Processing_Screen(_T("SUCCESS"));
				CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

				m_nCurrentFocusIndex = 1;
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
*	@brief		Terminal Monitoring
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_TerminalMonitoring()
{
	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	CString strScreenInform;
	CString strButtonInform;
	CString strTemp, strStatusMonitoring, strHeartBeatEnable, strHeartBeatFrequency;
	int nMaxFocusIndex = 3;

	// Focus Setting
	m_nCurrentFocusIndex = 1;

	strStatusMonitoring = CEagleDataManager::GetInstance()->m_Config.m_Host.strAtm_status_monitoring;
	strHeartBeatEnable = CEagleDataManager::GetInstance()->m_Config.m_Host.strHeartbeat_message;
	strHeartBeatFrequency.Format(_T("%d"), CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Host.strHeartbeat_frequency));

	while(TRUE)
	{
		CheckService();		// EPP Time시 Start Entry 명령 발행하기 위해 호출

		if (bShowScreen == TRUE)
		{
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();
			strScreenInform.Empty();

			// Screen Data
			strScreenInform = GET_ENDISABLE_STRING(strStatusMonitoring);
			strScreenInform += SCR_CMD_DELIMITER;

			strScreenInform += GET_ENDISABLE_STRING(strHeartBeatEnable);
			strScreenInform += SCR_CMD_DELIMITER;

			strTemp.Format(_T("%s MIN"), strHeartBeatFrequency);
			strScreenInform += strTemp;
			strScreenInform += SCR_CMD_DELIMITER;

			Operator_DisplayScreen(OP_SCREEN_TERMINAL_MONITORING, bKeyEnable, strScreenInform);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		// 입력 Key 확인
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_TerminalMonitoring - User Key Input : %s"), m_strScrOutData);

			// Common Action
			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_HOST;
				break;
			}

			// Touch로 Focus를 주기 위한 로직
			if (m_strSelectTouch.CompareNoCase(m_strScrOutData.Left(m_strSelectTouch.GetLength())) == 0)
			{
				m_nCurrentFocusIndex = CUtil::StringToInt(m_strScrOutData.Right(1));
				m_strScrOutData = _T("SELECT");
			}
			///////////////////////////////////////////////////

			// Next Action
			if (m_strScrOutData == _T("PREV"))
			{
				m_nCurrentFocusIndex--;

				if (m_nCurrentFocusIndex < 1)
				{
					m_nCurrentFocusIndex = nMaxFocusIndex;
				}

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("NEXT"))
			{
				m_nCurrentFocusIndex++;

				if (m_nCurrentFocusIndex > nMaxFocusIndex)
				{
					m_nCurrentFocusIndex = 1;
				}

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("SELECT"))
			{
				// Index를 가지고 몇 번째 값을 설정해야 하는지 AP에서 확인한다.
				switch(m_nCurrentFocusIndex)
				{
				case 1:	// Status Monitoring
					{
						if (strStatusMonitoring == S_ENABLE)
							strStatusMonitoring = S_DISABLE;
						else
							strStatusMonitoring = S_ENABLE;
					}
					break;

				case 2:	// Heart Beat
					{
						if (strHeartBeatEnable == S_ENABLE)
							strHeartBeatEnable = S_DISABLE;
						else
							strHeartBeatEnable = S_ENABLE;
					}
					break;

				case 3:	// Heart Beat Frenquency
					{
						if (Operator_Input_Number(_T("HEARTBEAT FRENQUENCY"), strHeartBeatFrequency, m_strScrOutData, 3) == TRUE)
						{
							m_strScrOutData.TrimLeft();
							m_strScrOutData.TrimRight();

							int nInterval = CUtil::StringToInt(m_strScrOutData);

							if (nInterval < 1 || nInterval > 999)
							{
								Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
								CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
							}
							else
							{
								strHeartBeatFrequency = m_strScrOutData;
							}
						}
					}
					break;

				default:
					break;
				}

				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("SET"))
			{
				CEagleDataManager::GetInstance()->m_Config.m_Host.strAtm_status_monitoring = strStatusMonitoring;
				CEagleDataManager::GetInstance()->m_Config.m_Host.strHeartbeat_message = strHeartBeatEnable;
				CEagleDataManager::GetInstance()->m_Config.m_Host.strHeartbeat_frequency = strHeartBeatFrequency;

				CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("atm_monitor_en"), CEagleDataManager::GetInstance()->m_Config.m_Host.strAtm_status_monitoring);
				CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("heartbeat_msg_en"), CEagleDataManager::GetInstance()->m_Config.m_Host.strHeartbeat_message);
				CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("heartbeat_frequency"), CEagleDataManager::GetInstance()->m_Config.m_Host.strHeartbeat_frequency);

				Operator_Processing_Screen(_T("SUCCESS"));
				CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

				m_nCurrentFocusIndex = 1;
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
*	@brief		EMV AID List
*	@retval		없음
************************************************************/
void CSVC_Manager::Proc_Operator_EMV_AIDList()
{
	BOOL	bShowScreen = TRUE;
	BOOL	bKeyEnable = TRUE;
	CString strScreenInform;
	CString strButtonInform;
	CString strTemp, strTemp2;
	CString	strUseAID[64];		// AID Buffer는 256으로 Define되어져 있으나 실제 20개도 사용 안하여 64개의 배열 생성
	CString strListIndex;
	int nTempFocusIndex = 0;

	int		nMaxFocusIndex = 32;
	int		i = 0, nCurrentPage = 1, nRemainCnt = 0, nTotalPage = 0, nTotalAidCount = 0;
	int		nExistCurPageAidCnt = 0, nTemp = 0;
	BOOL	bResult = FALSE;

	nTotalAidCount = CDEV_Manager::GetInstance()->m_DEV_CDR.m_sEMV_Config_Data.nTotalAidListCount;

	// 초기값 Loading
	for (i=0; i<nTotalAidCount; i++)
	{
		strUseAID[i] = GET_ENDISABLE_INT(CDEV_Manager::GetInstance()->m_DEV_CDR.m_sEMV_Config_Data.m_sEMV_Config[i].AidUse);
	}

	LOG(Info, _T("AID List - Total Count is - %d"), nTotalAidCount);

	nTotalPage = nTotalAidCount / MAX_PAGE_COUNT;
	nRemainCnt = nTotalAidCount % MAX_PAGE_COUNT;

	if (nRemainCnt != 0)
		nTotalPage++;

	if (nTotalAidCount >= MAX_PAGE_COUNT)
		nExistCurPageAidCnt = MAX_PAGE_COUNT;
	else
		nExistCurPageAidCnt = nTotalAidCount;

	LOG(Info, _T("AID List - Total Page : %d"), nTotalPage);

	m_nCurrentFocusIndex = 3;

	while(TRUE) 
	{ 
		CheckService();

		if (bShowScreen == TRUE)
		{  
			strScreenInform.Empty();
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();	
			strButtonInform.Empty();

			// 1. Total Count
			strTemp.Format(_T("%d"), nTotalAidCount);
			strScreenInform += strTemp;
			strScreenInform += SCR_CMD_DELIMITER;

			// 2. Page Information
			strTemp.Format(_T("%d / %d"), nCurrentPage, nTotalPage);
			strScreenInform += strTemp;
			strScreenInform += SCR_CMD_DELIMITER;
			
			i = (nCurrentPage -1) * MAX_PAGE_COUNT;

			for(; i<(MAX_PAGE_COUNT * nCurrentPage); i++)
			{
				if (i < nTotalAidCount)
				{
					// AID En/Disable
					strScreenInform += strUseAID[i];
					strScreenInform += SCR_CMD_DELIMITER;

					// Brand Type
					strTemp.Format(_T("%S"), CDEV_Manager::GetInstance()->m_DEV_CDR.m_sEMV_Config_Data.m_sEMV_Config[i].AidName);
					strScreenInform += strTemp;
					strScreenInform += SCR_CMD_DELIMITER;

					// AID
					strTemp = CUtil::ConvertHexToString(CDEV_Manager::GetInstance()->m_DEV_CDR.m_sEMV_Config_Data.m_sEMV_Config[i].szAid, CDEV_Manager::GetInstance()->m_DEV_CDR.m_sEMV_Config_Data.m_sEMV_Config[i].AidLen);
					strScreenInform += strTemp;
					strScreenInform += SCR_CMD_DELIMITER;
				}
				else
				{
					strScreenInform += _T("");
					strScreenInform += SCR_CMD_DELIMITER;

					strScreenInform += _T("");
					strScreenInform += SCR_CMD_DELIMITER;

					strScreenInform += _T("");
					strScreenInform += SCR_CMD_DELIMITER;
				}
			}

			// Button
			if (nCurrentPage <= 1)
				strButtonInform += _T("off");
			else
				strButtonInform += _T("on");

			strButtonInform += SCR_CMD_DELIMITER;

			nTemp = nTotalAidCount / MAX_PAGE_COUNT;
			if (nTotalAidCount % MAX_PAGE_COUNT)
				nTemp++;

			if (nCurrentPage >= nTemp)
				strButtonInform += _T("off");
			else
				strButtonInform += _T("on");
			strButtonInform += SCR_CMD_DELIMITER;

			Operator_DisplayScreen(OP_SCREEN_EMV_AID_LIST, bKeyEnable, strScreenInform, strButtonInform);	
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_EMV_AIDList - User Key Input : %s"), m_strScrOutData);

			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}

			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_HOST;	
				break;
			}

			// Touch로 Focus를 주기 위한 로직
			if (m_strSelectTouch.CompareNoCase(m_strScrOutData.Left(m_strSelectTouch.GetLength())) == 0)
			{
				nTempFocusIndex = CUtil::StringToInt(m_strScrOutData.Right(2));

				if( nTempFocusIndex > (((nExistCurPageAidCnt-1)*3) + 3) )
					continue;

				m_nCurrentFocusIndex = nTempFocusIndex;
				m_strScrOutData = _T("SELECT");
			}
			///////////////////////////////////////////////////

			// Next Action
			if (m_strScrOutData == _T("PREVPAGE"))
			{
				nCurrentPage--;

				if (nCurrentPage < 1)
					nCurrentPage = 1;

				if ((nCurrentPage * MAX_PAGE_COUNT) > nTotalAidCount)
					nExistCurPageAidCnt = (nTotalAidCount % MAX_PAGE_COUNT);
				else
					nExistCurPageAidCnt = MAX_PAGE_COUNT;

				m_nCurrentFocusIndex = 3; // 초기화
				bShowScreen = TRUE;

			}
			else if (m_strScrOutData == _T("NEXTPAGE"))
			{
				nCurrentPage++;

				if (nCurrentPage >= nTotalPage)
					nCurrentPage = nTotalPage;

				if ((nCurrentPage * MAX_PAGE_COUNT) > nTotalAidCount)
					nExistCurPageAidCnt = (nTotalAidCount % MAX_PAGE_COUNT);
				else
					nExistCurPageAidCnt = MAX_PAGE_COUNT;

				m_nCurrentFocusIndex = 3; // 초기화
				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("PREV"))
			{
				m_nCurrentFocusIndex--;
				m_nCurrentFocusIndex--;
				m_nCurrentFocusIndex--;

				if (m_nCurrentFocusIndex < 3)
				{
					m_nCurrentFocusIndex = ((nExistCurPageAidCnt-1)*3) + 3;
				}

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("NEXT"))
			{ 
				m_nCurrentFocusIndex++;
				m_nCurrentFocusIndex++;
				m_nCurrentFocusIndex++;

				if (m_nCurrentFocusIndex > (((nExistCurPageAidCnt-1)*3) + 3))
					m_nCurrentFocusIndex = 3;

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("SELECT"))
			{
				int nSelectedIndex = 0;
				nSelectedIndex = ((m_nCurrentFocusIndex / 3) - 1) + ((nCurrentPage -1) * MAX_PAGE_COUNT);

				if (strUseAID[nSelectedIndex] == _T("ENABLE"))
					strUseAID[nSelectedIndex] = _T("DISABLE");
				else
					strUseAID[nSelectedIndex] = _T("ENABLE");
				
				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("SET"))
			{
				Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);

				for(i=0; i<nTotalAidCount; i++)
				{
					if (strUseAID[i] == _T("ENABLE"))
						CDEV_Manager::GetInstance()->m_DEV_CDR.m_sEMV_Config_Data.m_sEMV_Config[i].AidUse = 1;
					else
						CDEV_Manager::GetInstance()->m_DEV_CDR.m_sEMV_Config_Data.m_sEMV_Config[i].AidUse = 0;
				}

				// Save And Load
				CDEV_Manager::GetInstance()->m_DEV_CDR.Save_AIDs_InformToFile();

				// ReLoad AID List
				CDEV_Manager::GetInstance()->m_DEV_CDR.Load_AIDs_InformFromFile();

				Operator_Processing_Screen(_T("SUCCESS"));
				CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

				bShowScreen = TRUE;
			}
			else
			{
				// 비정상적 값인 경우에는 OP MAIN으로 이동
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
		}

		Sleep(EAGLE_SLEEP_INTERVAL);
	}
}