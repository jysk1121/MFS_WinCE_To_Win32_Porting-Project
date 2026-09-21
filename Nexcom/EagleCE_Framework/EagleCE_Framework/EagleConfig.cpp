#include "StdAfx.h"
#include "EagleConfig.h"
#include "EagleConfigDefine.h"
#include "IniFile.h"
#include "Util.h"

CEagleConfig::CEagleConfig(void)
{
}

CEagleConfig::~CEagleConfig(void)
{
}


BOOL CEagleConfig::Load()
{
	if (FALSE == LoadAds())
	{
		return FALSE;
	}

	if (FALSE == LoadIso())
	{
		return FALSE;
	}

	if (FALSE == LoadCbxInfo())
	{
		return FALSE;
	}

	if (FALSE == LoadDevice())
	{
		return FALSE;
	}

	if (FALSE == LoadHost())
	{
		return FALSE;
	}

	if (FALSE == LoadLastTransInfo())
	{
		return FALSE;
	}

	if (FALSE == LoadMessage())
	{
		return FALSE;
	}

	if (FALSE == LoadOption())
	{
		return FALSE;
	}

	if (FALSE == LoadSystem())
	{
		return FALSE;
	}

	if (FALSE == LoadTotalInfo())
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CEagleConfig::LoadDevice()
{
	// 파일 경로 취득
	CString strPath = _T("");

	//strPath.Format(_T("%s%s"), CUtil::GetAppPath(), DEVICE_CFG_PATH_NAME);
	strPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH, DEVICE_CFG_PATH_NAME);

	if (CUtil::IsExistFile(strPath) == FALSE)
	{
		// Default Folder에서 File Copy하도록 로직 보완
		CString strDefaultPath = _T("");
		strDefaultPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_DEFAULT_PATH, DEVICE_CFG_PATH_NAME);
		CopyFile(strDefaultPath, strPath, FALSE);
		CUtil::Sleep_Wait(100);
	}

	// INI 파일 열기
	CIniFile iniFile(strPath);


	// [DEVICE_PORT]
	m_Device.strPortNumber_CDM = iniFile.ReadString(_T("DEVICE_PORT"), _T("CDM"), _T(""));

	m_Device.strPortNumber_CDR = iniFile.ReadString(_T("DEVICE_PORT"), _T("CDR"), _T(""));

	m_Device.strPortNumber_DIO = iniFile.ReadString(_T("DEVICE_PORT"), _T("DIO"), _T(""));

	m_Device.strPortNumber_EPP = iniFile.ReadString(_T("DEVICE_PORT"), _T("EPP"), _T(""));

	m_Device.strPortNumber_SHT = iniFile.ReadString(_T("DEVICE_PORT"), _T("SHT"), _T(""));

	m_Device.strPortNumber_PTR = iniFile.ReadString(_T("DEVICE_PORT"), _T("PTR"), _T(""));

	m_Device.strPortNumber_MUB = iniFile.ReadString(_T("DEVICE_PORT"), _T("MUB"), _T(""));

	
	// [DEVICE_EPP]
	m_Device.strType_EPP = iniFile.ReadString(_T("DEVICE_EPP"), _T("EPP_TYPE"), _T("1"));

	// [DEVICE_CDR]
	m_Device.strEMV_Enable = iniFile.ReadString(_T("DEVICE_CDR"), _T("EMV"), _T("1"));

	// [DEVICE_CAM]
	m_Device.strCAM_Enable = iniFile.ReadString(_T("DEVICE_CAM"), _T("CAM"), _T("0"));


	return TRUE;
}

BOOL CEagleConfig::LoadSystem()
{
	// 파일 경로 취득
	CString strPath = _T("");

	//strPath.Format(_T("%s%s"), CUtil::GetAppPath(), OP_SYSTEM_CFG_PATH_NAME);
	strPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH, SYSTEM_CFG_PATH_NAME);

	if (CUtil::IsExistFile(strPath) == FALSE)
	{
		// Default Folder에서 File Copy하도록 로직 보완
		CString strDefaultPath = _T("");
		strDefaultPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_DEFAULT_PATH, SYSTEM_CFG_PATH_NAME);
		CopyFile(strDefaultPath, strPath, FALSE);
		CUtil::Sleep_Wait(100);
	}

	// INI 파일 열기
	CIniFile iniFile(strPath);


	// [PASSWORD]
	BYTE byTemp1[32] = { 0, };
	BYTE byTemp2[32] = { 0, };
	BYTE byKey_16[32] = { 0, };
	CString strTemp, strDecryptedData, strEncryptedData;

	// Key Information
	CUtil::ConvertStringToHex(PASSWORD_KEY_32, byKey_16);

	memset(byTemp1, 0, sizeof(byTemp1));
	memset(byTemp2, 0, sizeof(byTemp2));

	strEncryptedData.Empty();
	strDecryptedData.Empty();

	strTemp = iniFile.ReadString(_T("PASSWORD"), _T("admin_pwd"), _T("222222"));
	strTemp.TrimLeft();
	strTemp.TrimRight();

 	if (strTemp.GetLength() == 6)	// Encrypted되지 않은 Data인 경우에는 Encrypted해서 저장
	{
		strTemp.Format(_T("06%s"), strTemp);
		sprintf((char*)byTemp1, "%S", strTemp);

		CUtil::EncryptKeyData(byTemp1, strTemp.GetLength(), byTemp2, byKey_16);
		strEncryptedData = CUtil::ConvertHexToString(byTemp2, 8);

		iniFile.WriteString(_T("PASSWORD"), _T("admin_pwd"), strEncryptedData);

		// 원래의 Password를 Memory에 Load
		m_System.strAdministrator_password = strTemp;
	}
	else
	{
		// Decrypted해서 Memory에 Load하도록 처리
		CUtil::ConvertStringToHex(strTemp, byTemp1);
		CUtil::DecryptKeyData(byTemp1, 8, byTemp2, byKey_16);

		strTemp.Format(_T("%S"), byTemp2);
		m_System.strAdministrator_password = strTemp.Right(6);
	}

	memset(byTemp1, 0, sizeof(byTemp1));
	memset(byTemp2, 0, sizeof(byTemp2));

	strEncryptedData.Empty();
	strDecryptedData.Empty();

	strTemp = iniFile.ReadString(_T("PASSWORD"), _T("master_pwd"), _T("111111"));
	strTemp.TrimLeft();
	strTemp.TrimRight();

 	if (strTemp.GetLength() == 6)	// Encrypted되지 않은 Data인 경우에는 Encrypted해서 저장
	{
		strTemp.Format(_T("06%s"), strTemp);
		sprintf((char*)byTemp1, "%S", strTemp);

		CUtil::EncryptKeyData(byTemp1, strTemp.GetLength(), byTemp2, byKey_16);
		strEncryptedData = CUtil::ConvertHexToString(byTemp2, 8);

		iniFile.WriteString(_T("PASSWORD"), _T("master_pwd"), strEncryptedData);

		// 원래의 Password를 Memory에 Load
		m_System.strMaster_password = strTemp;
	}
	else
	{
		// Decrypted해서 Memory에 Load하도록 처리
		CUtil::ConvertStringToHex(strTemp, byTemp1);
		CUtil::DecryptKeyData(byTemp1, 8, byTemp2, byKey_16);

		strTemp.Format(_T("%S"), byTemp2);
		m_System.strMaster_password = strTemp.Right(6);
	}

	memset(byTemp1, 0, sizeof(byTemp1));
	memset(byTemp2, 0, sizeof(byTemp2));

	strEncryptedData.Empty();
	strDecryptedData.Empty();

	strTemp = iniFile.ReadString(_T("PASSWORD"), _T("replenish_pwd"), _T("333333"));
	strTemp.TrimLeft();
	strTemp.TrimRight();

 	if (strTemp.GetLength() == 6)	// Encrypted되지 않은 Data인 경우에는 Encrypted해서 저장
	{
		strTemp.Format(_T("06%s"), strTemp);
		sprintf((char*)byTemp1, "%S", strTemp);

		CUtil::EncryptKeyData(byTemp1, strTemp.GetLength(), byTemp2, byKey_16);
		strEncryptedData = CUtil::ConvertHexToString(byTemp2, 8);

		iniFile.WriteString(_T("PASSWORD"), _T("replenish_pwd"), strEncryptedData);

		// 원래의 Password를 Memory에 Load
		m_System.strReplenishment_password = strTemp;
	}
	else
	{
		// Decrypted해서 Memory에 Load하도록 처리
		CUtil::ConvertStringToHex(strTemp, byTemp1);
		CUtil::DecryptKeyData(byTemp1, 8, byTemp2, byKey_16);

		strTemp.Format(_T("%S"), byTemp2);
		m_System.strReplenishment_password = strTemp.Right(6);
	}

	memset(byTemp1, 0, sizeof(byTemp1));
	memset(byTemp2, 0, sizeof(byTemp2));

	strEncryptedData.Empty();
	strDecryptedData.Empty();

	strTemp = iniFile.ReadString(_T("PASSWORD"), _T("ams_pwd"), _T("111111"));
	strTemp.TrimLeft();
	strTemp.TrimRight();

 	if (strTemp.GetLength() == 6)	// Encrypted되지 않은 Data인 경우에는 Encrypted해서 저장
	{
		strTemp.Format(_T("06%s"), strTemp);
		sprintf((char*)byTemp1, "%S", strTemp);

		CUtil::EncryptKeyData(byTemp1, strTemp.GetLength(), byTemp2, byKey_16);
		strEncryptedData = CUtil::ConvertHexToString(byTemp2, 8);

		iniFile.WriteString(_T("PASSWORD"), _T("ams_pwd"), strEncryptedData);

		// 원래의 Password를 Memory에 Load
		m_System.strRms_Password = strTemp;
	}
	else
	{
		// Decrypted해서 Memory에 Load하도록 처리
		CUtil::ConvertStringToHex(strTemp, byTemp1);
		CUtil::DecryptKeyData(byTemp1, 8, byTemp2, byKey_16);

		strTemp.Format(_T("%S"), byTemp2);
		m_System.strRms_Password = strTemp.Right(6);
	}

	m_System.strBackUpLogIndex = iniFile.ReadString(_T("SYSTEM"), _T("backup_log_index"), _T("0"));

	m_System.strSequence_Number = iniFile.ReadString(_T("SYSTEM"), _T("sequence_no"), _T("0000"));

	m_System.strLogMgr_Version = iniFile.ReadString(_T("SYSTEM"), _T("log_mgr_version"), _T("00.00.00"));

	m_System.strCaptureImageFile_Index = iniFile.ReadString(_T("SYSTEM"), _T("backup_captureimage_index"), _T("0"));

	return TRUE;
}


BOOL CEagleConfig::LoadHost()
{
	// 파일 경로 취득
	CString strPath = _T("");

	//strPath.Format(_T("%s%s"), CUtil::GetAppPath(), OP_HOST_CFG_PATH_NAME);
	strPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH, HOST_CFG_PATH_NAME);

	// File이 존재하지 않을 경우 Default 처리 추가
	if (CUtil::IsExistFile(strPath) == FALSE)
	{
		CString strDefaultPath;
		strDefaultPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_DEFAULT_PATH, HOST_CFG_PATH_NAME);
		CopyFile(strDefaultPath, strPath, FALSE);
		CUtil::Sleep_Wait(100);
	}


	// INI 파일 열기
	CIniFile iniFile(strPath);

	// [HOST]
	m_Host.strNetworkType = iniFile.ReadString(_T("HOST"), _T("network_type"), _T("TCP/IP"));

	m_Host.strProtocolType = iniFile.ReadString(_T("HOST"), _T("protocol_type"), _T("STANDARD"));

	m_Host.strUseENQ = iniFile.ReadString(_T("HOST"), _T("useENQ"), _T("1"));

	m_Host.strUseEOT = iniFile.ReadString(_T("HOST"), _T("useEOT"), _T("1"));

	m_Host.strHostip = iniFile.ReadString(_T("HOST"), _T("host_ip"), _T("0.0.0.0"));

	m_Host.strBackup_Hostip = iniFile.ReadString(_T("HOST"), _T("backup_host_ip"), _T("0.0.0.0"));

	m_Host.strPort = iniFile.ReadString(_T("HOST"), _T("port"), _T("0"));

	m_Host.strBackupPort = iniFile.ReadString(_T("HOST"), _T("backup_port"), _T("0"));

	m_Host.strUseDHCP = iniFile.ReadString(_T("HOST"), _T("useDHCP"), _T("0"));

	m_Host.strTerminalip = iniFile.ReadString(_T("HOST"), _T("terminal_ip"), _T("0.0.0.0"));

	m_Host.strSubnet = iniFile.ReadString(_T("HOST"), _T("subnet"), _T("0.0.0.0"));

	m_Host.strGateway = iniFile.ReadString(_T("HOST"), _T("gateway"), _T("0.0.0.0"));

	m_Host.strDNS = iniFile.ReadString(_T("HOST"), _T("DNS"), _T("0.0.0.0"));

	m_Host.strUseTLS = iniFile.ReadString(_T("HOST"), _T("useTLS"), _T("0"));

	m_Host.strUseCurl = iniFile.ReadString(_T("HOST"), _T("use_curl"), _T("0"));

	m_Host.strCommunication_header = iniFile.ReadString(_T("HOST"), _T("comm_header_en"), _T("0"));

	m_Host.strCommunicationID = iniFile.ReadString(_T("HOST"), _T("comm_id"), _T(""));

	m_Host.strPrimary_phone_number = iniFile.ReadString(_T("HOST"), _T("primary_num"), _T(""));

	m_Host.strBackup_phone_number = iniFile.ReadString(_T("HOST"), _T("backup_num"), _T(""));

	m_Host.strPre_dial = iniFile.ReadString(_T("HOST"), _T("predial_en"), _T("0"));

	m_Host.strSetup_String = iniFile.ReadString(_T("HOST"), _T("setup_string"), _T("AT&FE0+MS=V22B"));


	m_Host.strCrc_flg = iniFile.ReadString(_T("HOST"), _T("crc_flg"), _T("0"));

	m_Host.strAtm_status_monitoring = iniFile.ReadString(_T("HOST"), _T("atm_monitor_en"), _T("0"));

	m_Host.strHeartbeat_message = iniFile.ReadString(_T("HOST"), _T("heartbeat_msg_en"), _T("0"));

	m_Host.strHeartbeat_frequency = iniFile.ReadString(_T("HOST"), _T("heartbeat_frequency"), _T("30"));


	m_Host.strRms_en_flg = iniFile.ReadString(_T("AMS"), _T("rms_en"), _T("0"));

	m_Host.strRms_primary_num = iniFile.ReadString(_T("AMS"), _T("primary_num"), _T(""));

	//m_Host.strRms_callback_flg = iniFile.ReadString(_T("AMS"), _T("callback_en"), _T("0"));

	m_Host.strRms_Status_en_flg = iniFile.ReadString(_T("AMS"), _T("rms_status_en"), _T("0"));

	m_Host.strRms_IP_Address = iniFile.ReadString(_T("AMS"), _T("rms_ip_address"), _T(""));

	m_Host.strRms_Port = iniFile.ReadString(_T("AMS"), _T("rms_port"), _T("0"));

	m_Host.strSchedule_journal_flg = iniFile.ReadString(_T("AMS"), _T("schedule_jnl_en"), _T("0"));

	m_Host.strSchedule_journal_count = iniFile.ReadString(_T("AMS"), _T("schedule_jnl_cnt"), _T("10"));	// MAX : 999

	return TRUE;
}

CString CEagleConfig::LoadHostData(CString strKey)
{
	// 파일 경로 취득
	CString strPath = _T("");

	//strPath.Format(_T("%s%s"), CUtil::GetAppPath(), OP_HOST_CFG_PATH_NAME);
	strPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH, HOST_CFG_PATH_NAME);

	// INI 파일 열기
	CIniFile iniFile(strPath);

	return iniFile.ReadString(_T("HOST"), strKey, _T(""));
}

BOOL CEagleConfig::LoadIso()
{
	// 파일 경로 취득
	CString strPath = _T("");

	//strPath.Format(_T("%s%s"), CUtil::GetAppPath(), OP_BINLIST_CFG_PATH_NAME);
	strPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH, BINLIST_CFG_PATH_NAME);

	if (CUtil::IsExistFile(strPath) == FALSE)
	{
		CString strDefaultPath;
		strDefaultPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_DEFAULT_PATH, BINLIST_CFG_PATH_NAME);
		CopyFile(strDefaultPath, strPath, FALSE);
		CUtil::Sleep_Wait(100);
	}

	// INI 파일 열기
	CIniFile iniFile(strPath);
	CString strSecName;

	m_Iso.nTotalCount = 0;

	for(int i=0; i<MAX_BIN_COUNT; i++)
	{
		strSecName.Format(_T("block_iso_%d"), i+1);
		m_Iso.strBLOCK_ISO[i] = iniFile.ReadString(_T("BLOCK_ISO"), strSecName, _T(""));

		m_Iso.strBLOCK_ISO[i].TrimLeft();
		m_Iso.strBLOCK_ISO[i].TrimRight();

		if (m_Iso.strBLOCK_ISO[i].IsEmpty() == FALSE)
			m_Iso.nTotalCount++;
	}

	return TRUE;
}

BOOL CEagleConfig::LoadMessage()
{
	// 파일 경로 취득
	CString strPath = _T("");

	strPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH, MESSAGE_CFG_PATH_NAME);

	if (CUtil::IsExistFile(strPath) == FALSE)
	{
		CString strDefaultPath;
		strDefaultPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_DEFAULT_PATH, MESSAGE_CFG_PATH_NAME);
		CopyFile(strDefaultPath, strPath, FALSE);
		CUtil::Sleep_Wait(100);
	}

	int i=0;
	CString strAppName, strKeyName;

	// INI 파일 열기
	CIniFile iniFile(strPath);

	strAppName = _T("MESSAGE");

	for(i=0; i<4; i++)
	{
		strKeyName.Format(_T("marketing_msg_%d"), i+1);
		m_Message.strMarketing_message[i] = iniFile.ReadString(strAppName, strKeyName, _T(""));
	}

	for(i=0; i<4; i++)
	{
		strKeyName.Format(_T("location_msg_%d"), i+1);
		m_Message.strLocation_message[i] = iniFile.ReadString(strAppName, strKeyName, _T(""));
	}

	for(i=0; i<3; i++)
	{
		strKeyName.Format(_T("attract_msg_%d"), i+1);
		m_Message.strAttraction_message[i] = iniFile.ReadString(strAppName, strKeyName, _T(""));
	}

	for(i=0; i<3; i++)
	{
		strKeyName.Format(_T("farewell_msg_%d"), i+1);
		m_Message.strFarewell_message[i] = iniFile.ReadString(strAppName, strKeyName, _T(""));
	}

	return TRUE;
}

BOOL CEagleConfig::LoadOption()
{
	// 파일 경로 취득
	CString strPath = _T("");

	strPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH, OPTION_CFG_PATH_NAME);

	if (CUtil::IsExistFile(strPath) == FALSE)
	{
		CString strDefaultPath;
		strDefaultPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_DEFAULT_PATH, OPTION_CFG_PATH_NAME);
		CopyFile(strDefaultPath, strPath, FALSE);
		CUtil::Sleep_Wait(100);
	}	

	// INI 파일 열기
	CIniFile iniFile(strPath);

	m_Option.strSaving_account = iniFile.ReadString(_T("OPTION"), _T("saving_en"), _T("1"));

	m_Option.strCredit_account = iniFile.ReadString(_T("OPTION"), _T("credit_en"), _T("1"));

	m_Option.strtransfer_Enable = iniFile.ReadString(_T("OPTION"), _T("transfer_en"), _T("1"));

	m_Option.strbalance_Enable = iniFile.ReadString(_T("OPTION"), _T("balance_en"), _T("1"));

	m_Option.strFast_amount[0] = iniFile.ReadString(_T("OPTION"), _T("fast_amt_1"), _T("20"));

	m_Option.strFast_amount[1] = iniFile.ReadString(_T("OPTION"), _T("fast_amt_2"), _T("40"));

	m_Option.strFast_amount[2] = iniFile.ReadString(_T("OPTION"), _T("fast_amt_3"), _T("60"));

	m_Option.strFast_amount[3] = iniFile.ReadString(_T("OPTION"), _T("fast_amt_4"), _T("80"));

	m_Option.strFast_amount[4] = iniFile.ReadString(_T("OPTION"), _T("fast_amt_5"), _T("100"));

	m_Option.strFast_amount[5] = iniFile.ReadString(_T("OPTION"), _T("fast_amt_6"), _T("200"));

	m_Option.strMax_withdrawal_amount = iniFile.ReadString(_T("OPTION"), _T("max_withdrawal"), _T("200"));

	m_Option.strSurcharge_type = iniFile.ReadString(_T("OPTION"), _T("surch_type"), _T("0"));

	m_Option.strWithdrawalSurcharge_amount = iniFile.ReadString(_T("OPTION"), _T("withdrawal_surch_amt"), _T("150"));

	m_Option.strSurcharge_percentage = iniFile.ReadString(_T("OPTION"), _T("surch_percent"), _T("100"));

	m_Option.strWithdrawal_PercentSurcharge_Option = iniFile.ReadString(_T("OPTION"), _T("surch_lesser_greater"), _T("0"));

	m_Option.strSurcharge_owner = iniFile.ReadString(_T("OPTION"), _T("surch_owner"), _T(""));

	m_Option.strBalaceSurcharge_amount = iniFile.ReadString(_T("OPTION"), _T("balance_surch_amt"), _T("000"));

	m_Option.strTerminal_ID = iniFile.ReadString(_T("OPTION"), _T("terminal_id"), _T(""));

	m_Option.strMachine_Serial_Number = iniFile.ReadString(_T("OPTION"), _T("machine_no"), _T(""));

	m_Option.strLanguage = iniFile.ReadString(_T("OPTION"), _T("language"), _T("0"));

	m_Option.strLansupport[0] = iniFile.ReadString(_T("OPTION"), _T("English"), _T(""));

	m_Option.strLansupport[1] = iniFile.ReadString(_T("OPTION"), _T("Spanish"), _T(""));

	m_Option.strLansupport[2] = iniFile.ReadString(_T("OPTION"), _T("French"), _T(""));

	m_Option.strScheduleDayClose = iniFile.ReadString(_T("OPTION"), _T("schedule_dayclose_en"), _T("0"));

	m_Option.strDayCloseTime = iniFile.ReadString(_T("OPTION"), _T("dayclose_time"), _T("0000"));

	m_Option.strDayCloseYear = iniFile.ReadString(_T("OPTION"), _T("dayclose_year"), _T("2018"));

	m_Option.strDayCloseMonth = iniFile.ReadString(_T("OPTION"), _T("dayclose_month"), _T("01"));

	m_Option.strDayCloseDay = iniFile.ReadString(_T("OPTION"), _T("dayclose_day"), _T("01"));

	m_Option.strSound_Volume = iniFile.ReadString(_T("OPTION"), _T("sound_val"), _T("5"));

	m_Option.strRebootYear = iniFile.ReadString(_T("OPTION"), _T("reboot_year"), _T("2000"));

	m_Option.strRebootMonth = iniFile.ReadString(_T("OPTION"), _T("reboot_month"), _T("01"));

	m_Option.strRebootDay = iniFile.ReadString(_T("OPTION"), _T("reboot_day"), _T("01"));

	m_Option.strRebootTime = iniFile.ReadString(_T("OPTION"), _T("reboot_newtime"), _T(""));	// V1.0.2.4 2018.07.03 - OP메뉴에서 설정 가능함에 따라 추가함.

	return TRUE;
}


BOOL CEagleConfig::LoadAds()
{
	// 파일 경로 취득
	CString strPath = _T("");

	strPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH, ADS_CFG_PATH_NAME);

	if (CUtil::IsExistFile(strPath) == FALSE)
	{
		CString strDefaultPath;
		strDefaultPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_DEFAULT_PATH, ADS_CFG_PATH_NAME);
		CopyFile(strDefaultPath, strPath, FALSE);
		CUtil::Sleep_Wait(100);
	}

	// INI 파일 열기
	CIniFile iniFile(strPath);

	// [AD]
	m_ADS.strAdver_Interval_time = iniFile.ReadString(_T("AD"), _T("ad_set_interval_time"), _T("5"));

	m_ADS.strAdver_Screen_en[0] = iniFile.ReadString(_T("AD"), _T("ad_screen1_en"), _T("0"));

	m_ADS.strAdver_Screen_en[1] = iniFile.ReadString(_T("AD"), _T("ad_screen2_en"), _T("0"));

	m_ADS.strAdver_Screen_en[2] = iniFile.ReadString(_T("AD"), _T("ad_screen3_en"), _T("0"));

	m_ADS.strAdver_Screen_en[3] = iniFile.ReadString(_T("AD"), _T("ad_screen4_en"), _T("0"));

	m_ADS.strAdver_Screen_en[4] = iniFile.ReadString(_T("AD"), _T("ad_screen5_en"), _T("0"));

	m_ADS.strAdver_Screen_en[5] = iniFile.ReadString(_T("AD"), _T("ad_screen6_en"), _T("0"));

	return TRUE;
}


BOOL CEagleConfig::LoadTotalInfo()
{
	// 파일 경로 취득
	CString strPath = _T("");

	strPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH, TOTAL_CFG_PATH_NAME);

	if (CUtil::IsExistFile(strPath) == FALSE)
	{
		CString strDefaultPath;
		strDefaultPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_DEFAULT_PATH, TOTAL_CFG_PATH_NAME);
		CopyFile(strDefaultPath, strPath, FALSE);
		CUtil::Sleep_Wait(100);
	}


	// INI 파일 열기
	CIniFile iniFile(strPath);


	m_TotalInfo.Withdrawal_Count = iniFile.ReadString(_T("TOTAL"), _T("withdrawal_count"), _T("0"));

	m_TotalInfo.BalanceInquiry_Count = iniFile.ReadString(_T("TOTAL"), _T("balanceinquiry_count"), _T("0"));

	m_TotalInfo.Transfer_Count = iniFile.ReadString(_T("TOTAL"), _T("transfer_count"), _T("0"));

	m_TotalInfo.Withdrawal_Amount = iniFile.ReadString(_T("TOTAL"), _T("withdrawal_amount"), _T("0"));

	return TRUE;
}

BOOL CEagleConfig::LoadLastTransInfo()
{
	// 파일 경로 취득
	CString strPath = _T("");

	strPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH, LASTTRANSINFO_CFG_PATH_NAME);

	if (CUtil::IsExistFile(strPath) == FALSE)
	{
		CString strDefaultPath;
		strDefaultPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_DEFAULT_PATH, LASTTRANSINFO_CFG_PATH_NAME);
		CopyFile(strDefaultPath, strPath, FALSE);
		CUtil::Sleep_Wait(100);
	}


	// INI 파일 열기
	CIniFile iniFile(strPath);


	m_LastTransInfo.m_strTrack1Data = iniFile.ReadString(_T("LASTTRANSINFO"), _T("Track1Data"), _T(""));

	m_LastTransInfo.m_strTrack2Data = iniFile.ReadString(_T("LASTTRANSINFO"), _T("Track2Data"), _T(""));

	m_LastTransInfo.m_strTrack3Data = iniFile.ReadString(_T("LASTTRANSINFO"), _T("Track3Data"), _T(""));

	m_LastTransInfo.m_strShowCardNumber = iniFile.ReadString(_T("LASTTRANSINFO"), _T("showcardnumber"), _T(""));

	m_LastTransInfo.m_strTransType = iniFile.ReadString(_T("LASTTRANSINFO"), _T("TransactionType"), _T(""));

	m_LastTransInfo.m_strFromAccountType = iniFile.ReadString(_T("LASTTRANSINFO"), _T("FromAccountType"), _T(""));

	m_LastTransInfo.m_strToAccountType = iniFile.ReadString(_T("LASTTRANSINFO"), _T("ToAccountType"), _T(""));

	m_LastTransInfo.m_strRequestAmount = iniFile.ReadString(_T("LASTTRANSINFO"), _T("RequestAmount"), _T(""));

	m_LastTransInfo.m_strDispenseAmount = iniFile.ReadString(_T("LASTTRANSINFO"), _T("DispenseAmount"), _T(""));

	m_LastTransInfo.m_strSurchargeAmount = iniFile.ReadString(_T("LASTTRANSINFO"), _T("SurchargeAmount"), _T(""));

	m_LastTransInfo.m_strTransferAmount = iniFile.ReadString(_T("LASTTRANSINFO"), _T("TransferAmount"), _T(""));

	m_LastTransInfo.m_strAutorizationNumber = iniFile.ReadString(_T("LASTTRANSINFO"), _T("AutorizationNumber"), _T(""));

	m_LastTransInfo.m_strTransactionDate = iniFile.ReadString(_T("LASTTRANSINFO"), _T("TransactionDate"), _T(""));

	m_LastTransInfo.m_strTransactionTime = iniFile.ReadString(_T("LASTTRANSINFO"), _T("TransactionTime"), _T(""));
	
	m_LastTransInfo.m_strBusinessDate = iniFile.ReadString(_T("LASTTRANSINFO"), _T("BusinessDate"), _T(""));	

	m_LastTransInfo.m_strAvailableBalance = iniFile.ReadString(_T("LASTTRANSINFO"), _T("AvailableBalance"), _T(""));

	m_LastTransInfo.m_strBalance = iniFile.ReadString(_T("LASTTRANSINFO"), _T("Balance"), _T(""));

	m_LastTransInfo.m_strDispensedCount = iniFile.ReadString(_T("LASTTRANSINFO"), _T("Dispensed_Cnt"), _T(""));

	m_LastTransInfo.m_strRejectedCount = iniFile.ReadString(_T("LASTTRANSINFO"), _T("Rejected_Cnt"), _T(""));

	m_LastTransInfo.m_nIsReversal = CUtil::StringToInt(iniFile.ReadString(_T("LASTTRANSINFO"), _T("IsReversal"), _T("0")));

	m_LastTransInfo.m_strReasonforReversal = iniFile.ReadString(_T("LASTTRANSINFO"), _T("ReasonforReversal"), _T("0"));	

	m_LastTransInfo.m_nProtocol_processing_state = CUtil::StringToInt(iniFile.ReadString(_T("LASTTRANSINFO"), _T("Protocol_processing_state"), _T("0")));

	m_LastTransInfo.m_bSaveEJLforTransRecord = CUtil::StringToBOOL(iniFile.ReadString(_T("LASTTRANSINFO"), _T("IsSaveEJLTransRecord"), _T("0")));

	// EMV Information
	m_LastTransInfo.m_str_AID = iniFile.ReadString(_T("LASTTRANSINFO"), _T("EMV_AID"), _T(""));

	m_LastTransInfo.m_str_AID_Label = iniFile.ReadString(_T("LASTTRANSINFO"), _T("EMV_AID_LABEL"), _T(""));

	m_LastTransInfo.m_strTLV_57 = iniFile.ReadString(_T("LASTTRANSINFO"), _T("EMV_TLV_57"), _T(""));

	m_LastTransInfo.m_strTLV_5A = iniFile.ReadString(_T("LASTTRANSINFO"), _T("EMV_TLV_5A"), _T(""));

	m_LastTransInfo.m_strTLV_82 = iniFile.ReadString(_T("LASTTRANSINFO"), _T("EMV_TLV_82"), _T(""));

	m_LastTransInfo.m_strTLV_8A = iniFile.ReadString(_T("LASTTRANSINFO"), _T("EMV_TLV_8A"), _T(""));

	m_LastTransInfo.m_strTLV_95 = iniFile.ReadString(_T("LASTTRANSINFO"), _T("EMV_TLV_95"), _T(""));

	m_LastTransInfo.m_strTLV_9A = iniFile.ReadString(_T("LASTTRANSINFO"), _T("EMV_TLV_9A"), _T(""));

	m_LastTransInfo.m_strTLV_9C = iniFile.ReadString(_T("LASTTRANSINFO"), _T("EMV_TLV_9C"), _T(""));

	m_LastTransInfo.m_strTLV_5F2A = iniFile.ReadString(_T("LASTTRANSINFO"), _T("EMV_TLV_5F2A"), _T(""));

	m_LastTransInfo.m_strTLV_5F34 = iniFile.ReadString(_T("LASTTRANSINFO"), _T("EMV_TLV_5F34"), _T(""));

	m_LastTransInfo.m_strTLV_9F02 = iniFile.ReadString(_T("LASTTRANSINFO"), _T("EMV_TLV_9F02"), _T(""));

	m_LastTransInfo.m_strTLV_9F03 = iniFile.ReadString(_T("LASTTRANSINFO"), _T("EMV_TLV_9F03"), _T(""));

	m_LastTransInfo.m_strTLV_9F10 = iniFile.ReadString(_T("LASTTRANSINFO"), _T("EMV_TLV_9F10"), _T(""));

	m_LastTransInfo.m_strTLV_9F18 = iniFile.ReadString(_T("LASTTRANSINFO"), _T("EMV_TLV_9F18"), _T(""));

	m_LastTransInfo.m_strTLV_9F1A = iniFile.ReadString(_T("LASTTRANSINFO"), _T("EMV_TLV_9F1A"), _T(""));

	m_LastTransInfo.m_strTLV_9F26 = iniFile.ReadString(_T("LASTTRANSINFO"), _T("EMV_TLV_9F26"), _T(""));

	m_LastTransInfo.m_strTLV_9F27 = iniFile.ReadString(_T("LASTTRANSINFO"), _T("EMV_TLV_9F27"), _T(""));

	m_LastTransInfo.m_strTLV_9F33 = iniFile.ReadString(_T("LASTTRANSINFO"), _T("EMV_TLV_9F33"), _T(""));

	m_LastTransInfo.m_strTLV_9F35 = iniFile.ReadString(_T("LASTTRANSINFO"), _T("EMV_TLV_9F35"), _T(""));

	m_LastTransInfo.m_strTLV_9F36 = iniFile.ReadString(_T("LASTTRANSINFO"), _T("EMV_TLV_9F36"), _T(""));

	m_LastTransInfo.m_strTLV_9F37 = iniFile.ReadString(_T("LASTTRANSINFO"), _T("EMV_TLV_9F37"), _T(""));

	m_LastTransInfo.m_strTLV_9F39 = iniFile.ReadString(_T("LASTTRANSINFO"), _T("EMV_TLV_9F39"), _T(""));

	m_LastTransInfo.m_strTLV_9F41 = iniFile.ReadString(_T("LASTTRANSINFO"), _T("EMV_TLV_9F41"), _T(""));

	m_LastTransInfo.m_strTLV_DF05 = iniFile.ReadString(_T("LASTTRANSINFO"), _T("EMV_TLV_DF05"), _T(""));
	
	// CST Information for current transaction
	m_LastTransInfo.m_strCST_Reject_Info[0] = iniFile.ReadString(_T("LASTTRANSINFO"), _T("CST1_Reject_Info"), _T(""));

	m_LastTransInfo.m_strCST_Reject_Info[1] = iniFile.ReadString(_T("LASTTRANSINFO"), _T("CST2_Reject_Info"), _T(""));

	m_LastTransInfo.m_strCST_Reject_Info[2] = iniFile.ReadString(_T("LASTTRANSINFO"), _T("CST3_Reject_Info"), _T(""));

	m_LastTransInfo.m_strCST_Reject_Info[3] = iniFile.ReadString(_T("LASTTRANSINFO"), _T("CST4_Reject_Info"), _T(""));
	
	m_LastTransInfo.m_strCST_Remain_Cnt = iniFile.ReadString(_T("LASTTRANSINFO"), _T("CST_Remain_Count"), _T(""));

	return TRUE;
}

BOOL CEagleConfig::LoadCbxInfo()
{
	// 파일 경로 취득
	CString strPath = _T("");

	strPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH, CBXINFO_CFG_PATH_NAME);

	if (CUtil::IsExistFile(strPath) == FALSE)
	{
		CString strDefaultPath;
		strDefaultPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_DEFAULT_PATH, CBXINFO_CFG_PATH_NAME);
		CopyFile(strDefaultPath, strPath, FALSE);
		CUtil::Sleep_Wait(100);
	}

	// INI 파일 열기
	CIniFile iniFile(strPath);


	// [REJECT_INFO]
	for (int nCnt = 0; nCnt < 4; nCnt++)
	{
		CString strNumber = _T("");
		strNumber.Format(_T("%d"), nCnt + 1);

		// long, double, skew, near, more, thin, short, width, testdisp, total
		m_CbxInfo.strCBX_Reject_Reason[nCnt] = iniFile.ReadString(_T("REJECT_INFO"), _T("cbx_reject_reason_") + strNumber, _T("0 0 0 0 0 0 0 0 0 0"));
	}

	// [CBX_INFO]
	m_CbxInfo.strCBX_Count = iniFile.ReadString(_T("CBX_INFO"), _T("cbx_cnt"), _T(""));

	for (int nCnt = 0; nCnt < 4; nCnt++)
	{
		CString strNumber = _T("");
		strNumber.Format(_T("%d"), nCnt + 1);

		m_CbxInfo.strCBX_Use[nCnt] = iniFile.ReadString(_T("CBX_INFO"), _T("cbx_use_") + strNumber, _T("0"));

		m_CbxInfo.strCBX_Load_Count[nCnt] = iniFile.ReadString(_T("CBX_INFO"), _T("cbx_load_count_") + strNumber, _T("0"));

		m_CbxInfo.strCBX_Item_Count[nCnt] = iniFile.ReadString(_T("CBX_INFO"), _T("cbx_item_count_") + strNumber, _T("0"));

		m_CbxInfo.strCBX_Dispense_Count[nCnt] = iniFile.ReadString(_T("CBX_INFO"), _T("cbx_dispense_count_") + strNumber, _T("0"));

		m_CbxInfo.strCBX_Reject_Count[nCnt] = iniFile.ReadString(_T("CBX_INFO"), _T("cbx_reject_count_") + strNumber, _T("0"));

		m_CbxInfo.strCBX_Denomination[nCnt] = iniFile.ReadString(_T("CBX_INFO"), _T("cbx_denomination_") + strNumber, _T("0"));
	}

	m_CbxInfo.strRBN_Item_Count = iniFile.ReadString(_T("CBX_INFO"), _T("rbn_item_count"), _T("0"));


	return TRUE;
}


BOOL CEagleConfig::GetDispensingFlag()
{
	// 파일 경로 취득
	CString strPath = _T("");

	//strPath.Format(_T("%s"), DISPENSE_CFG_PATH_NAME);
	strPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH, DISPENSE_CFG_PATH_NAME);

	if (CUtil::IsExistFile(strPath) == FALSE)
	{
		CString strDefaultPath;
		strDefaultPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_DEFAULT_PATH, DISPENSE_CFG_PATH_NAME);
		CopyFile(strDefaultPath, strPath, FALSE);
		CUtil::Sleep_Wait(100);
	}

	// INI 파일 열기
	CIniFile iniFile(strPath);

	return CUtil::StringToBOOL(iniFile.ReadString(_T("DISPENSEINFO"), _T("IsSendDispense"), _T("0")));
}

BOOL CEagleConfig::ClearDispenseFlag()
{
	// 파일 경로 취득
	CString strPath = _T("");

	strPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH, DISPENSE_CFG_PATH_NAME);

	// INI 파일 열기
	CIniFile iniFile(strPath);

	iniFile.WriteString(_T("DISPENSEINFO"), _T("IsSendDispense"), _T("0"));

	return TRUE;
}


BOOL CEagleConfig::Save()
{
	if (FALSE == SaveDevice())
	{
		return FALSE;
	}

	if (FALSE == SaveSystem())
	{
		return FALSE;
	}

	if (FALSE == SaveHost())
	{
		return FALSE;
	}

	if (FALSE == SaveIso())
	{
		return FALSE;
	}

	if (FALSE == SaveMessage())
	{
		return FALSE;
	}

	if (FALSE == SaveOption())
	{
		return FALSE;
	}

	if (FALSE == SaveAds())
	{
		return FALSE;
	}

	if (FALSE == SaveTotalInfo())
	{
		return FALSE;
	}

	if (FALSE == SaveLastTransInfo())
	{
		return FALSE;
	}

	if (FALSE == SaveCbxInfo())
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CEagleConfig::SaveDevice()
{
	// 파일 경로 취득
	CString strPath = _T("");

	strPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH, DEVICE_CFG_PATH_NAME);

	// INI 파일 열기
	CIniFile iniFile(strPath);

	// [DEVICE_CAM]
	iniFile.WriteString(_T("DEVICE_CAM"), _T("CAM"), m_Device.strCAM_Enable);

	return TRUE;
}


BOOL CEagleConfig::SaveSystem()
{
	// 파일 경로 취득
	CString strPath = _T("");

	strPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH, SYSTEM_CFG_PATH_NAME);


	// INI 파일 열기
	CIniFile iniFile(strPath);

	// Password를 Encrytion해서 File 저장하도록 수정
	BYTE byTemp1[32] = { 0, };
	BYTE byTemp2[32] = { 0, };
	BYTE byKey_16[32] = { 0, };
	CString strTemp, strEncryptedData;

	// Key Information
	CUtil::ConvertStringToHex(PASSWORD_KEY_32, byKey_16);

	memset(byTemp1, 0, sizeof(byTemp1));
	memset(byTemp2, 0, sizeof(byTemp2));

	strTemp.Empty();
	strEncryptedData.Empty();

	strTemp.Format(_T("06%s"), m_System.strAdministrator_password);
	sprintf((char*)byTemp1, "%S", strTemp);

	CUtil::EncryptKeyData(byTemp1, strTemp.GetLength(), byTemp2, byKey_16);
	strEncryptedData = CUtil::ConvertHexToString(byTemp2, 8);

	// [PASSWORD]
	iniFile.WriteString(_T("PASSWORD"), _T("admin_pwd"), strEncryptedData);

	memset(byTemp1, 0, sizeof(byTemp1));
	memset(byTemp2, 0, sizeof(byTemp2));

	strTemp.Empty();
	strEncryptedData.Empty();

	strTemp.Format(_T("06%s"), m_System.strMaster_password);
	sprintf((char*)byTemp1, "%S", strTemp);

	CUtil::EncryptKeyData(byTemp1, strTemp.GetLength(), byTemp2, byKey_16);
	strEncryptedData = CUtil::ConvertHexToString(byTemp2, 8);

	iniFile.WriteString(_T("PASSWORD"), _T("master_pwd"), strEncryptedData);

	memset(byTemp1, 0, sizeof(byTemp1));
	memset(byTemp2, 0, sizeof(byTemp2));

	strTemp.Empty();
	strEncryptedData.Empty();

	strTemp.Format(_T("06%s"), m_System.strReplenishment_password);
	sprintf((char*)byTemp1, "%S", strTemp);

	CUtil::EncryptKeyData(byTemp1, strTemp.GetLength(), byTemp2, byKey_16);
	strEncryptedData = CUtil::ConvertHexToString(byTemp2, 8);

	iniFile.WriteString(_T("PASSWORD"), _T("replenish_pwd"), strEncryptedData);

	memset(byTemp1, 0, sizeof(byTemp1));
	memset(byTemp2, 0, sizeof(byTemp2));

	strTemp.Empty();
	strEncryptedData.Empty();

	strTemp.Format(_T("06%s"), m_System.strRms_Password);
	sprintf((char*)byTemp1, "%S", strTemp);

	CUtil::EncryptKeyData(byTemp1, strTemp.GetLength(), byTemp2, byKey_16);
	strEncryptedData = CUtil::ConvertHexToString(byTemp2, 8);

	iniFile.WriteString(_T("PASSWORD"), _T("ams_pwd"), strEncryptedData);


	iniFile.WriteString(_T("SYSTEM"), _T("backup_log_index"), m_System.strBackUpLogIndex);

	iniFile.WriteString(_T("SYSTEM"), _T("sequence_no"), m_System.strSequence_Number);

	iniFile.WriteString(_T("SYSTEM"), _T("log_mgr_version"), m_System.strLogMgr_Version);

	iniFile.WriteString(_T("SYSTEM"), _T("backup_captureimage_index"), m_System.strCaptureImageFile_Index);

	return TRUE;
}


BOOL CEagleConfig::SaveHost()
{
	// 파일 경로 취득
	CString strPath = _T("");

	strPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH, HOST_CFG_PATH_NAME);


	// INI 파일 열기
	CIniFile iniFile(strPath);

	iniFile.WriteString(_T("HOST"), _T("network_type"), m_Host.strNetworkType);

	iniFile.WriteString(_T("HOST"), _T("protocol_type"), m_Host.strProtocolType);

	iniFile.WriteString(_T("HOST"), _T("useENQ"), m_Host.strUseENQ);

	iniFile.WriteString(_T("HOST"), _T("useEOT"), m_Host.strUseEOT);

	iniFile.WriteString(_T("HOST"), _T("host_ip"), m_Host.strHostip);

	iniFile.WriteString(_T("HOST"), _T("port"), m_Host.strPort);

	iniFile.WriteString(_T("HOST"), _T("backup_host_ip"), m_Host.strBackup_Hostip);

	iniFile.WriteString(_T("HOST"), _T("backup_port"), m_Host.strBackupPort);

	iniFile.WriteString(_T("HOST"), _T("useDHCP"), m_Host.strUseDHCP);

	iniFile.WriteString(_T("HOST"), _T("terminal_ip"), m_Host.strTerminalip);

	iniFile.WriteString(_T("HOST"), _T("subnet"), m_Host.strSubnet);

	iniFile.WriteString(_T("HOST"), _T("gateway"), m_Host.strGateway);

	iniFile.WriteString(_T("HOST"), _T("DNS"), m_Host.strDNS);

	iniFile.WriteString(_T("HOST"), _T("useTLS"), m_Host.strUseTLS);

	iniFile.WriteString(_T("HOST"), _T("use_curl"), m_Host.strUseCurl);

	iniFile.WriteString(_T("HOST"), _T("comm_header_en"), m_Host.strCommunication_header);

	iniFile.WriteString(_T("HOST"), _T("comm_id"), m_Host.strCommunicationID);

	iniFile.WriteString(_T("HOST"), _T("primary_num"), m_Host.strPrimary_phone_number);

	iniFile.WriteString(_T("HOST"), _T("backup_num"), m_Host.strBackup_phone_number);

	iniFile.WriteString(_T("HOST"), _T("predial_en"), m_Host.strPre_dial);

	iniFile.WriteString(_T("HOST"), _T("setup_string"), m_Host.strSetup_String);

	//iniFile.WriteString(_T("HOST"), _T("baud_rate"), m_Host.strBaud_Rate);

	//iniFile.WriteString(_T("HOST"), _T("connect_timer_val"), m_Host.strConnect_timer);

	iniFile.WriteString(_T("HOST"), _T("crc_flg"), m_Host.strCrc_flg);

	iniFile.WriteString(_T("HOST"), _T("atm_monitor_en"), m_Host.strAtm_status_monitoring);

	iniFile.WriteString(_T("HOST"), _T("heartbeat_msg_en"), m_Host.strHeartbeat_message);

	iniFile.WriteString(_T("HOST"), _T("heartbeat_frequency"), m_Host.strHeartbeat_frequency);

	// [AMS]
	iniFile.WriteString(_T("AMS"), _T("rms_en"), m_Host.strRms_en_flg);

	iniFile.WriteString(_T("AMS"), _T("primary_num"), m_Host.strRms_primary_num);

	iniFile.WriteString(_T("AMS"), _T("rms_status_en"), m_Host.strRms_Status_en_flg);

	iniFile.WriteString(_T("AMS"), _T("rms_ip_address"), m_Host.strRms_IP_Address);

	iniFile.WriteString(_T("AMS"), _T("rms_port"), m_Host.strRms_Port);

	iniFile.WriteString(_T("AMS"), _T("schedule_jnl_en"), m_Host.strSchedule_journal_flg);

	iniFile.WriteString(_T("AMS"), _T("schedule_jnl_cnt"), m_Host.strSchedule_journal_count);

	return TRUE;
}

BOOL CEagleConfig::SaveIso()
{
	// 파일 경로 취득
	CString strPath = _T("");
	CString strSecName;

	strPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH, BINLIST_CFG_PATH_NAME);

	// INI 파일 열기
	CIniFile iniFile(strPath);

	for(int i=0; i<MAX_BIN_COUNT; i++)
	{
		strSecName.Format(_T("block_iso_%d"), i+1);
		iniFile.WriteString(_T("BLOCK_ISO"), strSecName, m_Iso.strBLOCK_ISO[i]);
	}

	return TRUE;
}

BOOL CEagleConfig::SaveMessage()
{
	// 파일 경로 취득
	CString strPath = _T("");

	strPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH, MESSAGE_CFG_PATH_NAME);


	// INI 파일 열기
	CIniFile iniFile(strPath);


	// [OPNTION_MSG]
	// %가 존재하는 경우 file write가 되지 않는 Bug Fix
	int i=0;
	CString strAppName, strKeyName;
	
	strAppName = _T("MESSAGE");

	for(i=0; i<4; i++)
	{
		strKeyName.Format(_T("marketing_msg_%d"), i+1);
		iniFile.WriteString(strAppName, strKeyName, m_Message.strMarketing_message[i]);
	}

	for(i=0; i<4; i++)
	{
		strKeyName.Format(_T("location_msg_%d"), i+1);
		iniFile.WriteString(strAppName, strKeyName, m_Message.strLocation_message[i]);
	}

	for(i=0; i<3; i++)
	{
		strKeyName.Format(_T("attract_msg_%d"), i+1);
		iniFile.WriteString(strAppName, strKeyName, m_Message.strAttraction_message[i]);
	}

	for(i=0; i<3; i++)
	{
		strKeyName.Format(_T("farewell_msg_%d"), i+1);
		iniFile.WriteString(strAppName, strKeyName, m_Message.strFarewell_message[i]);
	}

	return TRUE;
}

BOOL CEagleConfig::SaveOption()
{
	// 파일 경로 취득
	CString strPath = _T("");

	strPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH, OPTION_CFG_PATH_NAME);


	// INI 파일 열기
	CIniFile iniFile(strPath);

	// TRANSACTION
	iniFile.WriteString(_T("OPTION"), _T("saving_en"), m_Option.strSaving_account);

	iniFile.WriteString(_T("OPTION"), _T("credit_en"), m_Option.strCredit_account);

	iniFile.WriteString(_T("OPTION"), _T("transfer_en"), m_Option.strtransfer_Enable);

	iniFile.WriteString(_T("OPTION"), _T("balance_en"), m_Option.strbalance_Enable);

	iniFile.WriteString(_T("OPTION"), _T("fast_amt_1"), m_Option.strFast_amount[0]);

	iniFile.WriteString(_T("OPTION"), _T("fast_amt_2"), m_Option.strFast_amount[1]);

	iniFile.WriteString(_T("OPTION"), _T("fast_amt_3"), m_Option.strFast_amount[2]);

	iniFile.WriteString(_T("OPTION"), _T("fast_amt_4"), m_Option.strFast_amount[3]);

	iniFile.WriteString(_T("OPTION"), _T("fast_amt_5"), m_Option.strFast_amount[4]);

	iniFile.WriteString(_T("OPTION"), _T("fast_amt_6"), m_Option.strFast_amount[5]);

	//iniFile.WriteString(_T("OPTION"), _T("extended_amount_en"), m_Option.strExtended_amount);

	iniFile.WriteString(_T("OPTION"), _T("max_withdrawal"), m_Option.strMax_withdrawal_amount);

	iniFile.WriteString(_T("OPTION"), _T("surch_type"), m_Option.strSurcharge_type);

	iniFile.WriteString(_T("OPTION"), _T("withdrawal_surch_amt"), m_Option.strWithdrawalSurcharge_amount);

	iniFile.WriteString(_T("OPTION"), _T("surch_percent"), m_Option.strSurcharge_percentage);

	iniFile.WriteString(_T("OPTION"), _T("surch_lesser_greater"), m_Option.strWithdrawal_PercentSurcharge_Option);

	iniFile.WriteString(_T("OPTION"), _T("surch_owner"), m_Option.strSurcharge_owner);

	iniFile.WriteString(_T("OPTION"), _T("balance_surch_amt"), m_Option.strBalaceSurcharge_amount);

	iniFile.WriteString(_T("OPTION"), _T("terminal_id"), m_Option.strTerminal_ID);

	iniFile.WriteString(_T("OPTION"), _T("machine_no"), m_Option.strMachine_Serial_Number);

	iniFile.WriteString(_T("OPTION"), _T("language"), m_Option.strLanguage);

	iniFile.WriteString(_T("OPTION"), _T("English"), m_Option.strLansupport[0]);

	iniFile.WriteString(_T("OPTION"), _T("Spanish"), m_Option.strLansupport[1]);

	iniFile.WriteString(_T("OPTION"), _T("French"), m_Option.strLansupport[2]);

	iniFile.WriteString(_T("OPTION"), _T("schedule_dayclose_en"), m_Option.strScheduleDayClose);

	iniFile.WriteString(_T("OPTION"), _T("dayclose_time"), m_Option.strDayCloseTime);

	iniFile.WriteString(_T("OPTION"), _T("dayclose_year"), m_Option.strDayCloseYear);

	iniFile.WriteString(_T("OPTION"), _T("dayclose_month"), m_Option.strDayCloseMonth);

	iniFile.WriteString(_T("OPTION"), _T("dayclose_day"), m_Option.strDayCloseDay);

	iniFile.WriteString(_T("OPTION"), _T("sound_val"), m_Option.strSound_Volume);

	iniFile.WriteString(_T("OPTION"), _T("reboot_year"), m_Option.strRebootYear);

	iniFile.WriteString(_T("OPTION"), _T("reboot_month"), m_Option.strRebootMonth);

	iniFile.WriteString(_T("OPTION"), _T("reboot_day"), m_Option.strRebootDay);

	iniFile.WriteString(_T("OPTION"), _T("reboot_newtime"), m_Option.strRebootTime);	// V1.0.2.4 2018.07.03 - OP메뉴에서 설정 가능함에 따라 추가함.

	return TRUE;
}


BOOL CEagleConfig::SaveAds()
{
	// 파일 경로 취득
	CString strPath = _T("");

	strPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH, ADS_CFG_PATH_NAME);


	// INI 파일 열기
	CIniFile iniFile(strPath);


	// [AD]
	iniFile.WriteString(_T("AD"), _T("ad_set_interval_time"), m_ADS.strAdver_Interval_time);

	iniFile.WriteString(_T("AD"), _T("ad_screen1_en"), m_ADS.strAdver_Screen_en[0]);

	iniFile.WriteString(_T("AD"), _T("ad_screen2_en"), m_ADS.strAdver_Screen_en[1]);

	iniFile.WriteString(_T("AD"), _T("ad_screen3_en"), m_ADS.strAdver_Screen_en[2]);

	iniFile.WriteString(_T("AD"), _T("ad_screen4_en"), m_ADS.strAdver_Screen_en[3]);

	iniFile.WriteString(_T("AD"), _T("ad_screen5_en"), m_ADS.strAdver_Screen_en[4]);

	iniFile.WriteString(_T("AD"), _T("ad_screen6_en"), m_ADS.strAdver_Screen_en[5]);


	return TRUE;
}

BOOL CEagleConfig::SaveTotalInfo()
{
	// 파일 경로 취득
	CString strPath = _T("");

	strPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH, TOTAL_CFG_PATH_NAME);


	// INI 파일 열기
	CIniFile iniFile(strPath);


	iniFile.WriteString(_T("TOTAL"), _T("withdrawal_count"), m_TotalInfo.Withdrawal_Count);

	iniFile.WriteString(_T("TOTAL"), _T("balanceinquiry_count"), m_TotalInfo.BalanceInquiry_Count);

	iniFile.WriteString(_T("TOTAL"), _T("transfer_count"), m_TotalInfo.Transfer_Count);

	iniFile.WriteString(_T("TOTAL"), _T("withdrawal_amount"), m_TotalInfo.Withdrawal_Amount);

	return TRUE;
}


BOOL CEagleConfig::SaveLastTransInfo()
{
	// 파일 경로 취득
	CString strPath = _T("");

	//strPath.Format(_T("%s%s"), CUtil::GetAppPath(), LASTTRANSINFO_CFG_PATH_NAME);
	strPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH, LASTTRANSINFO_CFG_PATH_NAME);


	// INI 파일 열기
	CIniFile iniFile(strPath);


	iniFile.WriteString(_T("LASTTRANSINFO"), _T("Track1Data"), m_LastTransInfo.m_strTrack1Data);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("Track2Data"), m_LastTransInfo.m_strTrack2Data);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("Track3Data"), m_LastTransInfo.m_strTrack3Data);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("showcardnumber"), m_LastTransInfo.m_strShowCardNumber);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("TransactionType"), m_LastTransInfo.m_strTransType);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("FromAccountType"), m_LastTransInfo.m_strFromAccountType);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("ToAccountType"), m_LastTransInfo.m_strToAccountType);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("RequestAmount"), m_LastTransInfo.m_strRequestAmount);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("DispenseAmount"), m_LastTransInfo.m_strDispenseAmount);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("SurchargeAmount"), m_LastTransInfo.m_strSurchargeAmount);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("TransferAmount"), m_LastTransInfo.m_strTransferAmount);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("AutorizationNumber"), m_LastTransInfo.m_strAutorizationNumber);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("TransactionDate"), m_LastTransInfo.m_strTransactionDate);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("TransactionTime"), m_LastTransInfo.m_strTransactionTime);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("BusinessDate"), m_LastTransInfo.m_strBusinessDate);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("AvailableBalance"), m_LastTransInfo.m_strAvailableBalance);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("Balance"), m_LastTransInfo.m_strBalance);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("Dispensed_Cnt"), m_LastTransInfo.m_strDispensedCount);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("Rejected_Cnt"), m_LastTransInfo.m_strRejectedCount);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("IsReversal"), CUtil::IntToString(m_LastTransInfo.m_nIsReversal));

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("ReasonforReversal"), m_LastTransInfo.m_strReasonforReversal);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("Protocol_processing_state"), CUtil::IntToString(m_LastTransInfo.m_nProtocol_processing_state));

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("IsSaveEJLTransRecord"), CUtil::IntToString(m_LastTransInfo.m_bSaveEJLforTransRecord));
	

	// EMV Information
	iniFile.WriteString(_T("LASTTRANSINFO"), _T("EMV_AID"), m_LastTransInfo.m_str_AID);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("EMV_AID_LABEL"), m_LastTransInfo.m_str_AID_Label);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("EMV_TLV_57"), m_LastTransInfo.m_strTLV_57);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("EMV_TLV_5A"), m_LastTransInfo.m_strTLV_5A);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("EMV_TLV_82"), m_LastTransInfo.m_strTLV_82);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("EMV_TLV_95"), m_LastTransInfo.m_strTLV_95);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("EMV_TLV_9A"), m_LastTransInfo.m_strTLV_9A);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("EMV_TLV_9C"), m_LastTransInfo.m_strTLV_9C);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("EMV_TLV_5F2A"), m_LastTransInfo.m_strTLV_5F2A);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("EMV_TLV_5F34"), m_LastTransInfo.m_strTLV_5F34);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("EMV_TLV_9F02"), m_LastTransInfo.m_strTLV_9F02);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("EMV_TLV_9F03"), m_LastTransInfo.m_strTLV_9F03);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("EMV_TLV_9F10"), m_LastTransInfo.m_strTLV_9F10);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("EMV_TLV_9F18"), m_LastTransInfo.m_strTLV_9F18);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("EMV_TLV_9F1A"), m_LastTransInfo.m_strTLV_9F1A);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("EMV_TLV_9F26"), m_LastTransInfo.m_strTLV_9F26);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("EMV_TLV_9F27"), m_LastTransInfo.m_strTLV_9F27);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("EMV_TLV_9F33"), m_LastTransInfo.m_strTLV_9F33);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("EMV_TLV_9F35"), m_LastTransInfo.m_strTLV_9F35);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("EMV_TLV_9F36"), m_LastTransInfo.m_strTLV_9F36);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("EMV_TLV_9F37"), m_LastTransInfo.m_strTLV_9F37);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("EMV_TLV_9F39"), m_LastTransInfo.m_strTLV_9F39);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("EMV_TLV_9F41"), m_LastTransInfo.m_strTLV_9F41);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("EMV_TLV_DF05"), m_LastTransInfo.m_strTLV_DF05);

	// CST Information for current transaction
	iniFile.WriteString(_T("LASTTRANSINFO"), _T("CST1_Reject_Info"), m_LastTransInfo.m_strCST_Reject_Info[0]);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("CST2_Reject_Info"), m_LastTransInfo.m_strCST_Reject_Info[1]);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("CST3_Reject_Info"), m_LastTransInfo.m_strCST_Reject_Info[2]);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("CST4_Reject_Info"), m_LastTransInfo.m_strCST_Reject_Info[3]);

	iniFile.WriteString(_T("LASTTRANSINFO"), _T("CST_Remain_Count"), m_LastTransInfo.m_strCST_Remain_Cnt);



	return TRUE;
}


BOOL CEagleConfig::SaveCbxInfo()
{
	// 파일 경로 취득
	CString strPath = _T("");

	//strPath.Format(_T("%s%s"), CUtil::GetAppPath(), CBXINFO_CFG_PATH_NAME);
	strPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH, CBXINFO_CFG_PATH_NAME);


	// INI 파일 열기
	CIniFile iniFile(strPath);


	// [REJECT_INFO]
	for (int nCnt=0; nCnt<4; nCnt++)
	{
		CString strNumber;
		strNumber.Format(_T("%d"), nCnt + 1);

		iniFile.WriteString(_T("REJECT_INFO"), _T("cbx_reject_reason_") + strNumber, m_CbxInfo.strCBX_Reject_Reason[nCnt]);
	}

	// [CBX_INFO]
	iniFile.WriteString(_T("CBX_INFO"), _T("cbx_cnt"), m_CbxInfo.strCBX_Count);

	for (int nCnt=0; nCnt<4; nCnt++)
	{
		CString strNumber;
		strNumber.Format(_T("%d"), nCnt + 1);

		iniFile.WriteString(_T("CBX_INFO"), _T("cbx_use_") + strNumber, m_CbxInfo.strCBX_Use[nCnt]);

		iniFile.WriteString(_T("CBX_INFO"), _T("cbx_load_count_") + strNumber, m_CbxInfo.strCBX_Load_Count[nCnt]);

		iniFile.WriteString(_T("CBX_INFO"), _T("cbx_item_count_") + strNumber, m_CbxInfo.strCBX_Item_Count[nCnt]);

		iniFile.WriteString(_T("CBX_INFO"), _T("cbx_dispense_count_") + strNumber, m_CbxInfo.strCBX_Dispense_Count[nCnt]);

		iniFile.WriteString(_T("CBX_INFO"), _T("cbx_reject_count_") + strNumber, m_CbxInfo.strCBX_Reject_Count[nCnt]);

		iniFile.WriteString(_T("CBX_INFO"), _T("cbx_denomination_") + strNumber, m_CbxInfo.strCBX_Denomination[nCnt]);
	}

	iniFile.WriteString(_T("CBX_INFO"), _T("rbn_item_count"), m_CbxInfo.strRBN_Item_Count);

	return TRUE;
}


BOOL CEagleConfig::SaveSystemData(CString strKey, CString strValue)
{
	// 파일 경로 취득
	CString strPath = _T("");

	//strPath.Format(_T("%s%s"), CUtil::GetAppPath(), OP_SYSTEM_CFG_PATH_NAME);
	strPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH, SYSTEM_CFG_PATH_NAME);

	// INI 파일 열기
	CIniFile iniFile(strPath);

	iniFile.WriteString(_T("SYSTEM"), strKey, strValue);

	return TRUE;
}


BOOL CEagleConfig::SavePasswordData(CString strKey, CString strValue)
{
	// 파일 경로 취득
	CString strPath = _T("");

	//strPath.Format(_T("%s%s"), CUtil::GetAppPath(), OP_SYSTEM_CFG_PATH_NAME);
	strPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH, SYSTEM_CFG_PATH_NAME);

	// INI 파일 열기
	CIniFile iniFile(strPath);

	iniFile.WriteString(_T("PASSWORD"), strKey, strValue);

	return TRUE;
}


BOOL CEagleConfig::SaveHostData(CString strKey, CString strValue)
{
	// 파일 경로 취득
	CString strPath = _T("");

	//strPath.Format(_T("%s%s"), CUtil::GetAppPath(), OP_HOST_CFG_PATH_NAME);
	strPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH, HOST_CFG_PATH_NAME);


	// INI 파일 열기
	CIniFile iniFile(strPath);

	iniFile.WriteString(_T("HOST"), strKey, strValue);

	return TRUE;
}

BOOL CEagleConfig::SaveAMSData(CString strKey, CString strValue)
{
	// 파일 경로 취득
	CString strPath = _T("");

	//strPath.Format(_T("%s%s"), CUtil::GetAppPath(), OP_HOST_CFG_PATH_NAME);
	strPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH, HOST_CFG_PATH_NAME);


	// INI 파일 열기
	CIniFile iniFile(strPath);

	iniFile.WriteString(_T("AMS"), strKey, strValue);

	return TRUE;
}


BOOL CEagleConfig::SaveOptionData(CString strKey, CString strValue)
{
	// 파일 경로 취득
	CString strPath = _T("");

	//strPath.Format(_T("%s%s"), CUtil::GetAppPath(), OP_OPTION_CFG_PATH_NAME);
	strPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH, OPTION_CFG_PATH_NAME);

	// INI 파일 열기
	CIniFile iniFile(strPath);

	iniFile.WriteString(_T("OPTION"), strKey, strValue);

	return TRUE;
}


BOOL CEagleConfig::SaveMessageData(CString strKey, CString strValue)
{
	// 파일 경로 취득
	CString strPath = _T("");

	//strPath.Format(_T("%s%s"), CUtil::GetAppPath(), OP_MESSAGE_CFG_PATH_NAME);
	strPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH, MESSAGE_CFG_PATH_NAME);


	// INI 파일 열기
	CIniFile iniFile(strPath);

	iniFile.WriteString(_T("MESSAGE"), strKey, strValue);

	return TRUE;
}


BOOL CEagleConfig::SaveLastTransInfoData(CString strKey, CString strValue)
{
	// 파일 경로 취득
	CString strPath = _T("");

	//strPath.Format(_T("%s%s"), CUtil::GetAppPath(), LASTTRANSINFO_CFG_PATH_NAME);
	strPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH, LASTTRANSINFO_CFG_PATH_NAME);


	// INI 파일 열기
	CIniFile iniFile(strPath);

	iniFile.WriteString(_T("LASTTRANSINFO"), strKey, strValue);

	return TRUE;
}

BOOL CEagleConfig::SaveDeviceData( CString strKey, CString strValue )
{
	// 파일 경로 취득
	CString strPath = _T("");

	//strPath.Format(_T("%s%s"), CUtil::GetAppPath(), DEVICE_CFG_PATH_NAME);
	strPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH, DEVICE_CFG_PATH_NAME);

	// INI 파일 열기
	CIniFile iniFile(strPath);

	// 현재는 Camera만 Setting하여 CAM Section만 Write하게 Coding함 -> 추후 Section 추가시 추가 Coding 필요
	// [DEVICE_CAM]
	iniFile.WriteString(_T("DEVICE_CAM"), strKey, strValue);

	return TRUE;
}


BOOL CEagleConfig::SaveCBXInfoData( CString strKey, CString strValue )
{
	// 파일 경로 취득
	CString strPath = _T("");

	//strPath.Format(_T("%s%s"), CUtil::GetAppPath(), CBXINFO_CFG_PATH_NAME);
	strPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH, CBXINFO_CFG_PATH_NAME);

	// INI 파일 열기
	CIniFile iniFile(strPath);

	iniFile.WriteString(_T("CBX_INFO"), strKey, strValue);

	return TRUE;
}


CString CEagleConfig::LoadSystemData(CString strKey, CString strValue)
{
	// 파일 경로 취득
	CString strPath = _T("");

	//strPath.Format(_T("%s%s"), CUtil::GetAppPath(), OP_SYSTEM_CFG_PATH_NAME);
	strPath.Format(_T("%s%s\\%s"), CUtil::GetAppPath(), EAGLE_ATM_CONFIG_PATH, SYSTEM_CFG_PATH_NAME);

	// INI 파일 열기
	CIniFile iniFile(strPath);


	// [PASSWORD]
	return iniFile.ReadString(strKey, strValue, _T(""));
}


void EAGLE_TOTAL_INFO::Clear(void)
{
	Withdrawal_Count = _T("0");					// 출금 거래 건수
	BalanceInquiry_Count = _T("0");				// 조회 거래 건수
	Transfer_Count = _T("0");						// 이체 거래 건수
	Withdrawal_Amount = _T("0");					// 출금 총 금액
}

void EAGLE_TOTAL_INFO::SumData(CString strTransType, CString strAmount)
{
	int nCount = 0;
	int nAmount = 0;

	if (strTransType == S_WITHDRAWAL)
	{
		nCount = CUtil::StringToInt(Withdrawal_Count) + 1;
		Withdrawal_Count = CUtil::IntToString(nCount);

		nAmount = (CUtil::StringToInt(Withdrawal_Amount) + CUtil::StringToInt(strAmount));
		Withdrawal_Amount = CUtil::IntToString(nAmount);
	}
	else if (strTransType == S_BALANCEINQUIRY)
	{
		nCount = CUtil::StringToInt(BalanceInquiry_Count) + 1;
		BalanceInquiry_Count = CUtil::IntToString(nCount);
	}
	else if (strTransType == S_TRANSFER)
	{
		nCount = CUtil::StringToInt(Transfer_Count) + 1;
		Transfer_Count = CUtil::IntToString(nCount);
	}
}


void EAGLE_LAST_TRANS_INFO::Clear(void)
{
	m_strTrack1Data.Empty();
	m_strTrack2Data.Empty();
	m_strTrack3Data.Empty();
	m_strShowCardNumber.Empty();				// 명세표 및 저널에 표시 또는 Print할 Card Number
	m_strTransType.Empty();						// Host로 송신한 거래 종류
	m_strFromAccountType.Empty();				// Host로 송신할 계좌 종류
	m_strToAccountType.Empty();					// Host로 송신할 계좌 종류
	m_strRequestAmount.Empty();					// Host로 송신한 요청금액
	m_strDispenseAmount.Empty();				// 방출금액
	m_strSurchargeAmount.Empty();				// Host로 송신 or 수신 받은 Surcharge (수신시 갱신됨)
	m_strTransferAmount.Empty();

	m_strAutorizationNumber.Empty();			// Host로부터 수신 받은 승인번호
	m_strTransactionDate.Empty();				// Host로부터 수신 받은 거래 날짜
	m_strTransactionTime.Empty();				// Host로부터 수신 받은 거래 시간
	m_strBusinessDate.Empty();					// Host로부터 수신 받은 Business Date
	m_strAvailableBalance.Empty();				// Host로부터 수신 받은 Available Balance
	m_strBalance.Empty();						// Host로부터 수신 받은 Balance

	m_strDispensedCount.Empty();
	m_strRejectedCount.Empty();

	m_nIsReversal = 0;
	m_strReasonforReversal.Empty();
	m_nProtocol_processing_state = 0;

	m_bSaveEJLforTransRecord = FALSE;

	m_str_AID.Empty();
	m_str_AID_Label.Empty();

	m_strTLV_9F02.Empty();
	m_strTLV_9F03.Empty();
	m_strTLV_9F26.Empty();
	m_strTLV_82.Empty();
	m_strTLV_8A.Empty();
	m_strTLV_5A.Empty();
	m_strTLV_5F34.Empty();
	m_strTLV_9F36.Empty();
	m_strTLV_9F27.Empty();
	m_strTLV_9F10.Empty();
	m_strTLV_9F39.Empty();
	m_strTLV_9F33.Empty();
	m_strTLV_9F1A.Empty();
	m_strTLV_9F35.Empty();
	m_strTLV_95.Empty();
	m_strTLV_57.Empty();
	m_strTLV_5F2A.Empty();
	m_strTLV_9A.Empty();
	m_strTLV_9F41.Empty();
	m_strTLV_9C.Empty();
	m_strTLV_9F37.Empty();
	m_strTLV_9F18.Empty();
	m_strTLV_DF05.Empty();

	m_strCST_Reject_Info[0].Empty();
	m_strCST_Reject_Info[1].Empty();
	m_strCST_Reject_Info[2].Empty();
	m_strCST_Reject_Info[3].Empty();
	m_strCST_Remain_Cnt.Empty();

}

void EAGLE_CBX_INFO::Clear_Reject_Info()
{
	// Reject Info is clear
	for(int i=0; i<4; i++)
	{
		strCBX_Reject_Reason[i] =  _T("0 0 0 0 0 0 0 0 0 0");	// long, double, skew, near, more, thin, short, width, testdisp, total
	}
}


void EAGLE_CBX_INFO::Clear_CBX_Count()
{
	// CBX All Count is clear
	strRBN_Item_Count = _T("0");

	for(int i=0; i<4; i++)
	{
		strCBX_Load_Count[i] = _T("0");
		strCBX_Item_Count[i] = _T("0");
		strCBX_Dispense_Count[i] = _T("0");
		strCBX_Reject_Count[i] = _T("0");
	}
}