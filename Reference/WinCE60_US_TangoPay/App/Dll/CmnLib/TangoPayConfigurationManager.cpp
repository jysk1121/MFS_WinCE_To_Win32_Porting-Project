#include "stdafx.h"
#include ".\Common\CmnLib.h"
#include ".\Common\TangoPayConfigurationManager.h"
#include ".\Common\NHDbgApi.h"

// // [RWC6-676] Start SKKim 2024.04.05 added new files
#define SECTION_SERVICE_INFO			"Service_Info"
#define KEY_SERVICE_ENABLE				"Enable"
#define KEY_PARTNERID					"PartnerID"

#define SECTION_SERVER_INFO				"Server_Info"
#define KEY_IDENTIFY_SERVER_URL			"Identify_URL"
#define KEY_IDENTIFY_SERVER_PORT		"Identify_Port"
#define KEY_TRANSACTION_SERVER_URL		"Transaction_URL"
#define KEY_TRANSACTION_SERVER_PORT		"Transaction_Port"

#define SECTION_PUBLICKEY_INFO			"PublicKey_Info"
#define KEY_APIKEY						"ApiKey"
#define KEY_PUBLICKEY					"PublicKey"
#define KEY_PUBLICKEY_CHECKSUM			"CheckSum"

#define SECTION_ETC_INFO				"Etc_Info"
#define DOWNLOAD_APP_INFO				"Download_App_Info"




CTangoPayConfigurationManager::CTangoPayConfigurationManager(void) {}


CTangoPayConfigurationManager::~CTangoPayConfigurationManager(void) {}


void CTangoPayConfigurationManager::CheckConfigData()
{
	FILE *fp = NULL;
	DWORD dwFileWriten = 0;
	CString filename(TANGOPAY_CONFIG_INI);
	CStringA strTemp;

	TangoPayConfiguration config;

	if ( (fp = _tfopen(filename, _T("r"))) == NULL)
	{
		// file 미 존재시 default file 생성
		if ( (fp = _tfopen(filename, _T("wb+"))) != NULL)
		{
			// Create Service_Info Section
			strTemp.Format("[%s]\r\n", SECTION_SERVICE_INFO);
			dwFileWriten = fwrite(strTemp.GetBuffer(), sizeof(char), strTemp.GetLength(), fp);

			strTemp.Format("%s=%s\r\n", KEY_SERVICE_ENABLE, "false");
			dwFileWriten = fwrite(strTemp.GetBuffer(), sizeof(char), strTemp.GetLength(), fp);

			strTemp.Format("%s=%S\r\n", KEY_PARTNERID, config.strPartnerID);
			dwFileWriten = fwrite(strTemp.GetBuffer(), sizeof(char), strTemp.GetLength(), fp);

			// Create Server_Info Section
			strTemp.Format("[%s]\r\n", SECTION_SERVER_INFO);
			dwFileWriten = fwrite(strTemp.GetBuffer(), sizeof(char), strTemp.GetLength(), fp);

			strTemp.Format("%s=%S\r\n", KEY_IDENTIFY_SERVER_URL, config.strIdentifyServerURL);
			dwFileWriten = fwrite(strTemp.GetBuffer(), sizeof(char), strTemp.GetLength(), fp);

			strTemp.Format("%s=%d\r\n", KEY_IDENTIFY_SERVER_PORT, config.nIdentifyPort);
			dwFileWriten = fwrite(strTemp.GetBuffer(), sizeof(char), strTemp.GetLength(), fp);

			strTemp.Format("%s=%S\r\n", KEY_TRANSACTION_SERVER_URL, config.strTransactionServerURL);
			dwFileWriten = fwrite(strTemp.GetBuffer(), sizeof(char), strTemp.GetLength(), fp);

			strTemp.Format("%s=%d\r\n", KEY_TRANSACTION_SERVER_PORT, config.nTransactionPort);
			dwFileWriten = fwrite(strTemp.GetBuffer(), sizeof(char), strTemp.GetLength(), fp);

			// Create PublicKey_Info Section
			strTemp.Format("[%s]\r\n", SECTION_PUBLICKEY_INFO);
			dwFileWriten = fwrite(strTemp.GetBuffer(), sizeof(char), strTemp.GetLength(), fp);

			strTemp.Format("%s=%S\r\n", KEY_APIKEY, config.strApiKey);
			dwFileWriten = fwrite(strTemp.GetBuffer(), sizeof(char), strTemp.GetLength(), fp);

			strTemp.Format("%s=%S\r\n", KEY_PUBLICKEY_CHECKSUM,config.strPK_CheckSum);
			dwFileWriten = fwrite(strTemp.GetBuffer(), sizeof(char), strTemp.GetLength(), fp);

			// PublicKey에 \r\n이 포함되어 중간에 있는 경우 ini file 검색 로직에 문제가 생김
			//strTemp.Format("%s=%S\r\n", KEY_PUBLICKEY, config.strPublicKey);
			//dwFileWriten = fwrite(strTemp.GetBuffer(), sizeof(char), strTemp.GetLength(), fp);

			strTemp.Format("[%s]\r\n", SECTION_ETC_INFO);
			dwFileWriten = fwrite(strTemp.GetBuffer(), sizeof(char), strTemp.GetLength(), fp);

			// [RWC6-676] test code
			strTemp.Format("%s=%s\r\n", DOWNLOAD_APP_INFO, "https://pay.dev.tangopay.com");
			dwFileWriten = fwrite(strTemp.GetBuffer(), sizeof(char), strTemp.GetLength(), fp);

			fflush(fp);
			fclose(fp);
		}
	}
	else
	{
		fclose(fp);
	}
}

bool CTangoPayConfigurationManager::GetConfiguration(TangoPayConfiguration &config) const
{
    CStringA filename(TANGOPAY_CONFIG_INI);
    return LoadConfigurationFromIni(filename.GetBuffer(), config);

    return false;
}

bool CTangoPayConfigurationManager::SaveConfiguration(const TangoPayConfiguration &config) const
{
    CStringA filename(TANGOPAY_CONFIG_INI);
    return SaveConfigurationToIni(filename.GetBuffer(), config);
}


//
// Private Functions
//

bool CTangoPayConfigurationManager::LoadConfigurationFromIni(char *filename, TangoPayConfiguration &config) const
{
    char *buffer = NULL;
    bool failed = false;
    int res = 0;

	res = GetIniString(filename, SECTION_SERVICE_INFO, KEY_SERVICE_ENABLE, &buffer);
	failed |= res == -1;

	if (res == 1)
	{
		config.bEnable = strcmp(buffer, "true") == 0;
		delete[] buffer;
	}

	res = GetIniString(filename, SECTION_SERVICE_INFO, KEY_PARTNERID, &buffer);
	failed |= res == -1;

	if (res == 1)
	{
		config.strPartnerID = CString(buffer);
		delete[] buffer;
	}


    res = GetIniString(filename, SECTION_SERVER_INFO, KEY_IDENTIFY_SERVER_URL, &buffer);
    failed |= res == -1;

    if (res == 1)
    {
        config.strIdentifyServerURL = CString(buffer);
        delete[] buffer;
    }

	res = GetIniString(filename, SECTION_SERVER_INFO, KEY_IDENTIFY_SERVER_PORT, &buffer);
	failed |= res == -1;

	if (res == 1)
	{
		config.nIdentifyPort = Asc2Int(CString(buffer));
		delete[] buffer;
	}

	res = GetIniString(filename, SECTION_SERVER_INFO, KEY_TRANSACTION_SERVER_URL, &buffer);
	failed |= res == -1;

	if (res == 1)
	{
		config.strTransactionServerURL = CString(buffer);
		delete[] buffer;
	}

	res = GetIniString(filename, SECTION_SERVER_INFO, KEY_TRANSACTION_SERVER_PORT, &buffer);
	failed |= res == -1;

	if (res == 1)
	{
		config.nTransactionPort = Asc2Int(CString(buffer));
		delete[] buffer;
	}


	res = GetIniString(filename, SECTION_PUBLICKEY_INFO, KEY_APIKEY, &buffer);
	failed |= res == -1;

	if (res == 1)
	{
		config.strApiKey = CString(buffer);
		delete[] buffer;
	}

	res = GetIniString(filename, SECTION_PUBLICKEY_INFO, KEY_PUBLICKEY_CHECKSUM, &buffer);
	failed |= res == -1;

	if (res == 1)
	{
		config.strPK_CheckSum = CString(buffer);
		delete[] buffer;
	}

	// PublicKey에 \r\n이 포함되어 중간에 있는 경우 ini file 검색 로직에 문제가 생겨 로직 변경함
	//res = GetIniString(filename, SECTION_PUBLICKEY_INFO, KEY_PUBLICKEY, &buffer);
	//failed |= res == -1;

	//if (res == 1)
	//{
	//	config.strPublicKey = CString(buffer);
	//	delete[] buffer;
	//}
	FILE *fp = NULL;
	char read_buff[1024] = { 0, };

	if ( (fp = _tfopen(TANGOPAY_PUBLIC_KEY, _T("rb"))) != NULL)
	{
		fseek(fp, 0, SEEK_SET);
		fread(read_buff, sizeof(char), sizeof(read_buff), fp);
		fclose(fp);

		config.strPublicKey = (CString)read_buff;
	}

	res = GetIniString(filename, SECTION_ETC_INFO, DOWNLOAD_APP_INFO, &buffer);
	failed |= res == -1;

	if (res == 1)
	{
		config.strDownloadAppInfo = CString(buffer);
		delete[] buffer;
	}

    return !failed;
}

bool CTangoPayConfigurationManager::SaveConfigurationToIni(char *filename, const TangoPayConfiguration &config) const
{
    char buffer[2048] = {};

	SetIniString(filename, SECTION_SERVICE_INFO, KEY_SERVICE_ENABLE, config.bEnable ? "true" : "false");

	WideToMulti(buffer, config.strPartnerID, config.strPartnerID.GetLength() + 1);
	SetIniString(filename, SECTION_SERVICE_INFO, KEY_PARTNERID, buffer);
	memset(buffer, 0, sizeof(buffer));


	WideToMulti(buffer, config.strIdentifyServerURL, config.strIdentifyServerURL.GetLength() + 1);
    SetIniString(filename, SECTION_SERVER_INFO, KEY_IDENTIFY_SERVER_URL, buffer);
    memset(buffer, 0, sizeof(buffer));

    sprintf_s(buffer, sizeof(buffer), "%d", config.nIdentifyPort);
    SetIniString(filename, SECTION_SERVER_INFO, KEY_IDENTIFY_SERVER_PORT, buffer);
    memset(buffer, 0, sizeof(buffer));

	WideToMulti(buffer, config.strTransactionServerURL, config.strTransactionServerURL.GetLength() + 1);
	SetIniString(filename, SECTION_SERVER_INFO, KEY_TRANSACTION_SERVER_URL, buffer);
	memset(buffer, 0, sizeof(buffer));

	sprintf_s(buffer, sizeof(buffer), "%d", config.nTransactionPort);
	SetIniString(filename, SECTION_SERVER_INFO, KEY_TRANSACTION_SERVER_PORT, buffer);
	memset(buffer, 0, sizeof(buffer));


	WideToMulti(buffer, config.strApiKey, config.strApiKey.GetLength() + 1);
	SetIniString(filename, SECTION_PUBLICKEY_INFO, KEY_APIKEY, buffer);
	memset(buffer, 0, sizeof(buffer));

	WideToMulti(buffer, config.strPK_CheckSum, config.strPK_CheckSum.GetLength() + 1);
	SetIniString(filename, SECTION_PUBLICKEY_INFO, KEY_PUBLICKEY_CHECKSUM, buffer);
	memset(buffer, 0, sizeof(buffer));

	// PublicKey에 \r\n이 포함되어 중간에 있는 경우 ini file 검색 로직에 문제가 생겨 로직 변경함
	//WideToMulti(buffer, config.strPublicKey, config.strPublicKey.GetLength() + 1);
	//SetIniString(filename, SECTION_PUBLICKEY_INFO, KEY_PUBLICKEY, buffer);
	//memset(buffer, 0, sizeof(buffer));
	FILE *fp = NULL;
	CStringA strTemp(config.strPublicKey);

	// 이전 file 삭제 후 저장
	if ( (fp = _tfopen(TANGOPAY_PUBLIC_KEY, _T("r"))) != NULL)
	{
		fclose(fp);
		fp = NULL;

		::DeleteFile(TANGOPAY_PUBLIC_KEY);
	}

	if (strTemp.GetLength() > 0)
	{
		if ( (fp = _tfopen(TANGOPAY_PUBLIC_KEY, _T("wb+"))) != NULL)
		{
			fwrite(strTemp.GetBuffer(), sizeof(char), strTemp.GetLength(), fp);
			fflush(fp);
			fclose(fp);
		}
	}

	WideToMulti(buffer, config.strDownloadAppInfo, config.strDownloadAppInfo.GetLength() + 1);
	SetIniString(filename, SECTION_ETC_INFO, DOWNLOAD_APP_INFO, buffer);
	memset(buffer, 0, sizeof(buffer));

    return true;
}