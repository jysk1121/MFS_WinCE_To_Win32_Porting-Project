#pragma once

#include "EagleConfigDefine.h"

class CEagleConfig
{
public:
	// 持失切
	CEagleConfig(void);

	// 社瑚切
	~CEagleConfig(void);

	BOOL Load();
	BOOL LoadDevice();
	BOOL LoadSystem();
	BOOL LoadHost();
	BOOL LoadIso();
	BOOL LoadMessage();
	BOOL LoadOption();
	BOOL LoadAds();
	BOOL LoadTotalInfo();
	BOOL LoadLastTransInfo();
	BOOL LoadCbxInfo();
	BOOL GetDispensingFlag();

	CString LoadHostData(CString strKey);

	BOOL Save();
	BOOL SaveDevice();
	BOOL SaveSystem();
	BOOL SaveHost();
	BOOL SaveIso();
	BOOL SaveMessage();
	BOOL SaveOption();
	BOOL SaveAds();
	BOOL SaveTotalInfo();
	BOOL SaveLastTransInfo();
	BOOL SaveCbxInfo();
	BOOL ClearDispenseFlag();

	BOOL SaveSystemData(CString strKey, CString strValue);
	BOOL SavePasswordData(CString strKey, CString strValue);
	BOOL SaveHostData(CString strKey, CString strValue);
	BOOL SaveAMSData(CString strKey, CString strValue);
	BOOL SaveOptionData(CString strKey, CString strValue);
	BOOL SaveMessageData(CString strKey, CString strValue);
	BOOL SaveLastTransInfoData(CString strKey, CString strValue);
	BOOL SaveDeviceData(CString strKey, CString strValue);
	BOOL SaveCBXInfoData(CString strKey, CString strValue);

	CString LoadSystemData(CString strKey, CString strValue);

	EAGLE_CONFIG_DEVICE		m_Device;
	EAGLE_CONFIG_SYSTEM		m_System;
	EAGLE_CONFIG_HOST		m_Host;
	EAGLE_CONFIG_ISO		m_Iso;
	EAGLE_CONFIG_MESSAGE	m_Message;
	EAGLE_CONFIG_OPTION		m_Option;
	EAGLE_CONFIG_ADS		m_ADS;
	EAGLE_TOTAL_INFO		m_TotalInfo;
	EAGLE_LAST_TRANS_INFO	m_LastTransInfo;
	EAGLE_CBX_INFO			m_CbxInfo;
};

