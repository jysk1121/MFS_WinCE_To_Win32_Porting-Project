#include "StdAfx.h"
#include ".\..\..\H\Tran\CJustCashData.h"
#include ".\..\..\H\Common\Markup.h"

#include <parson/parson.h>			// [RWC6-117] Update JSON implementation
#pragma comment(lib, "parson.lib")	// [RWC6-117] Update JSON implementation

//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------

// #define NH_DEBUG // Debug Message On/Off

#include ".\..\..\H\Common\NHDbgApi.h"

#define DBG_CALL 1
#define DBG_INFO 1

// [#2445] US Justin 2016.09.21 Just.Cash
#if (APP_JUST_CASH)

CJustCashData::CJustCashData()
{
	m_strJCDayTotalDateTime = m_strJCDayTotalDispCount = m_strJCDayTotalDispAmt = L"";
	m_strDCDayTotalDateTime = m_strDCDayTotalDispCount = m_strDCDayTotalDispAmt = L""; // [#2496] US Justin 2017.08.14 Purchasing BitCoin

	ResetSetting();
	ResetTransactionData();
}

CJustCashData::~CJustCashData() {}

////////////////////////////////////////////////////////////
// Setting Function
void CJustCashData::ResetSetting()
{
	m_bJustCashAvailable = FALSE;
	m_bJustCashRegistered = FALSE;
	m_strJCLogo = m_strJCLogo_Button = _T("");
	m_strHostIP = m_strHostPort = m_strHostProcess = _T("");
	m_strHostSSL = L"1";
	m_strUserName = m_strEncryptPWD = m_strDevKey = m_strSessionKey = _T(""); // [#2528] 2018.01.29 Remove m_strEncrpytPIN
	m_transactionMode = JCTXNMODE_ATM;
}

BOOL CJustCashData::ReadSettingAndDispenseHistory()
{
	NHDEBUG(DBG_INFO, (L"CJustCashData::ReadSettingAndDispenseHistory()\n"));

	m_bJustCashAvailable = FALSE;
	m_bJustCashRegistered = FALSE;

	CString strPath, oldPath;
	CFile cf;

	oldPath.Format(L"%s\\%s", ATM_DATA_PATH, JUSTCASH_SETTING_FILE);
	strPath.Format(L"%s\\%s", ATM2_DATA_PATH, JUSTCASH_SETTING_FILE);

	///////////////////////////////////////////////////////////////
	// Move Settings files from /ATM/Data to /ATM2/Data
	///////////////////////////////////////////////////////////////
	bool newConfigsExist = !!IsExistFile(oldPath);

	if (newConfigsExist)
	{
		CopyFile(oldPath, strPath, FALSE);
		DeleteFile(oldPath);

		NHDBG((L"New CoinSquare configuration installed\r\n"));
	}

	///////////////////////////////////////////////////////////////
	// Read Setting
	///////////////////////////////////////////////////////////////
	if (cf.Open(strPath, CFile::modeRead | CFile::typeText) == FALSE)
	{
		NHDEBUG(DBG_INFO, (L"[FAIL TO OPEN(%s)]\n", strPath));
		return FALSE;
	}

	int nFileLen = (int)cf.GetLength();
	unsigned char *pBuffer = new unsigned char[nFileLen + 2]; // Allocate buffer for binary file data

	if (pBuffer == NULL)
	{
		NHDEBUG(DBG_INFO, (L"[FAIL TO NEW BUFFER(%d)]\n", nFileLen + 2));

		cf.Close();

		return FALSE;
	}

	nFileLen = cf.Read(pBuffer, nFileLen);
	cf.Close();

	pBuffer[nFileLen] = '\0';
	pBuffer[nFileLen + 1] = '\0'; // in case 2-byte encoded

	CString strXML(_T(""));

	if (pBuffer[0] == 0xFF && pBuffer[1] == 0xFE) // Windows Unicode file is detected if starts with FEFF
	{
		strXML = (LPCWSTR)(&pBuffer[2]);

		NHDEBUG(DBG_CALL, (L"[File starts with hex FFFE, assumed to be wide char format.]\n"));
	}
	else
	{
		strXML = (LPCSTR)pBuffer;
	}

	delete[] pBuffer;

	if (strXML.GetLength() < nFileLen / 2 - 20) // If it is too short, assume it got truncated due to non-text content
	{
		NHDEBUG(DBG_INFO, (L"[Error converting file to string (may contain binary data)]\n"));
		return FALSE;
	}

	CMarkup xml;

	if (xml.SetDoc(strXML) == FALSE)
	{
		NHDEBUG(DBG_INFO, (L"[FAIL TO PARSE XML]\n"));
		return FALSE;
	}

	if (xml.FindChildElem(_T("Setting")) == FALSE)
	{
		NHDEBUG(DBG_INFO, (L"[FAIL TO FIND <Setting>]\n"));
		return FALSE;
	}

	if (xml.IntoElem() == FALSE)
	{
		NHDEBUG(DBG_INFO, (L"[FAIL TO INTO <Setting>]\n"));
		return FALSE;
	}

	while (xml.FindChildElem())
	{
		CString strName = xml.GetChildTagName();

		if (strName.IsEmpty() == TRUE)
			continue;

		CString strChildData = xml.GetChildData();

		NHDEBUG(DBG_INFO, (L" XML ChildNode [%s] = [%s]\n", strName, strChildData));

		if		(strName.CompareNoCase(L"HostIP") == 0)					m_strHostIP			= strChildData;
		else if (strName.CompareNoCase(L"HostPort") == 0)				m_strHostPort		= strChildData;
		else if (strName.CompareNoCase(L"Process") == 0)				m_strHostProcess	= strChildData;
		else if (strName.CompareNoCase(L"JustCashLogo") == 0)			m_strJCLogo			= strChildData;
		else if (strName.CompareNoCase(L"JustCashLogo_Button") == 0)	m_strJCLogo_Button	= strChildData;
		else if (strName.CompareNoCase(L"username") == 0)				m_strUserName		= strChildData;
		else if (strName.CompareNoCase(L"password") == 0)				m_strEncryptPWD		= strChildData;
//		else if (strName.CompareNoCase(L"encrypt_pin") == 0)			m_strEncrpytPIN		= strChildData; // [#2528] 2018.01.29 Remove m_strEncrpytPIN
		else if (strName.CompareNoCase(L"device_key") == 0)				m_strDevKey			= strChildData;
		else if (strName.CompareNoCase(L"session_key") == 0)			m_strSessionKey		= strChildData;
		else if (strName.CompareNoCase(L"transaction_mode") == 0)		m_transactionMode	= Asc2Int(strChildData);
	}

	if (xml.OutOfElem() == FALSE)
	{
		NHDEBUG(DBG_INFO, (L"[FAIL OUT OF <Setting>]\n"));

		ResetSetting();

		return FALSE;
	}

	if ((m_strHostIP.GetLength() > 0) && (m_strHostPort.GetLength() > 0) && (m_strHostProcess.GetLength() > 0)
		&& (m_strUserName.GetLength() > 0) && (m_strEncryptPWD.GetLength() > 0) && (m_strDevKey.GetLength() > 0)) // [#2528] 2018.01.29 Remove m_strEncrpytPIN
	{
		m_bJustCashAvailable = TRUE;

		if (m_strSessionKey.GetLength() > 0)
			m_bJustCashRegistered = TRUE;
	}
	else
	{
		NHDEBUG(DBG_INFO, (L"[One or more paremeters are missing]\n"));

		ResetSetting();

		return FALSE;
	}

	return ReadDispenseHistory();
}

BOOL CJustCashData::SaveSetting()
{
	NHDEBUG(DBG_INFO, (L"JustCash SaveSetting()\n"));

	CString strData, strPath, strTemp;
	CFile cf;

	strPath.Format(L"%s\\%s", ATM2_DATA_PATH, JUSTCASH_SETTING_FILE);

	if (cf.Open(strPath, CFile::modeWrite | CFile::modeCreate) == FALSE)
	{
		NHDEBUG(DBG_INFO, (L"[FAIL TO OPEN(%s)]\n", strPath));
		return FALSE;
	}

	strData = L"<JustCash>\r\n";
	strData += L"<Setting>\r\n";

	if (m_strHostIP.GetLength() > 0)		{	strData += L"<HostIP>"				+ m_strHostIP					+ L"</HostIP>\r\n";					}
	if (m_strHostPort.GetLength() > 0)		{	strData += L"<HostPort>"			+ m_strHostPort					+ L"</HostPort>\r\n";				}
	if (m_strHostProcess.GetLength() > 0)	{	strData += L"<Process>"				+ m_strHostProcess				+ L"</Process>\r\n";				}
	if (m_strJCLogo.GetLength() > 0)		{	strData += L"<JustCashLogo>"		+ m_strJCLogo					+ L"</JustCashLogo>\r\n";			}
	if (m_strJCLogo_Button.GetLength() > 0)	{	strData += L"<JustCashLogo_Button>"	+ m_strJCLogo_Button			+ L"</JustCashLogo_Button>\r\n";	}
	if (m_strUserName.GetLength() > 0)		{	strData += L"<username>"			+ m_strUserName					+ L"</username>\r\n";				}
	if (m_strEncryptPWD.GetLength() > 0)	{	strData += L"<password>"			+ m_strEncryptPWD				+ L"</password>\r\n";				}
//	if (m_strEncrpytPIN.GetLength() > 0)	{	strData += L"<encrypt_pin>"			+ m_strEncrpytPIN				+ L"</encrypt_pin>\r\n";			} // [#2528] 2018.01.29 Remove m_strEncrpytPIN
	if (m_strDevKey.GetLength() > 0)		{	strData += L"<device_key>"			+ m_strDevKey					+ L"</device_key>\r\n";				}
	if (m_strSessionKey.GetLength() > 0)	{	strData += L"<session_key>"			+ m_strSessionKey				+ L"</session_key>\r\n";			}
												strData += L"<transaction_mode>"	+ Int2Asc(m_transactionMode)	+ L"</transaction_mode>\r\n";

	strData += L"</Setting>\r\n";
	strData += L"</JustCash>\r\n";

	NHDEBUG(DBG_INFO, (L"JustCash Setting =\n%s", strData));

	BYTE pBuffer[4096];
	memset(pBuffer, NULL, 4096);

	WideToMulti((LPSTR)pBuffer, strData, strData.GetLength());

	cf.Write(pBuffer, strData.GetLength());
	cf.Close();

	NHDEBUG(DBG_INFO, (L"JustCash SaveSetting() OK    RETURN TRUE\n"));

	return TRUE;
}

CString CJustCashData::GetJustCashFileName(int nType)
{
	CString sRtn = L"";

	if (nType == JUSTCASH_FILE_LOGO_BUTTON)
		sRtn.Format(L"%s\\%s", ATM_DATA_PATH, m_strJCLogo_Button);
	else if (nType == JUSTCASH_FILE_LOGO_GENERAL)
		sRtn.Format(L"%s\\%s", ATM_DATA_PATH, m_strJCLogo);

	return sRtn;
}

CString CJustCashData::GetHashCode()
{
	CString strData, strTemp;

	// last 10 digit of device key
	strTemp.Format(L"          %s", m_strDevKey);
	strData = strTemp.Right(10);

	// first 5 of pcode
	strTemp.Format(L"%s     ", m_strCashCode);
	strData += strTemp.Left(5);

	// Convert the wide string
	char *pMsgBody = new char[strData.GetLength() + 1]();
	WideToMulti(pMsgBody, strData, strData.GetLength());

	// SHA 256 of Message
	CString strSHAEnc = SHA_256_Encrypt((unsigned char *)pMsgBody, strData.GetLength(), 0);

	NHDEBUG(DBG_INFO, (L"SHA256 : [%S] => [%s]\n", pMsgBody, strSHAEnc));

	delete[] pMsgBody;

	return strSHAEnc;
}

BOOL CJustCashData::IsSidecarConfigured()
{
	return (m_transactionMode == JCTXNMODE_BOTH || m_transactionMode == JCTXNMODE_SIDECAR)
		&& m_bJustCashAvailable
		&& m_bJustCashRegistered;
}

BOOL CJustCashData::IsAtmConfigured()
{
	return (m_transactionMode == JCTXNMODE_BOTH || m_transactionMode == JCTXNMODE_SIDECAR || m_transactionMode == JCTXNMODE_ATM)
		&& m_bJustCashAvailable
		&& m_bJustCashRegistered;
}

CString CJustCashData::GetTransactionMode()
{
	switch (m_transactionMode)
	{
	case JCTXNMODE_ATM:
		return L"Debit";
	case JCTXNMODE_SIDECAR:
		return L"Cash";
	case JCTXNMODE_BOTH:
		return L"Both";
	default:
		return L"Unknown";
	}
};

////////////////////////////////////////////////////////////
// Transaction Function
void CJustCashData::ResetTransactionData()
{
	m_bIsOnUsTransaction = FALSE;
	m_nTransactionStep = JUSTCASH_CARDLESS_TRAN_NOTSTART;
	m_strCashCode = m_strTID = m_strTranAmount = m_strTrackData = m_strApprovalCode = _T("");
	m_strTranDispensed = _T("0.00");
	m_strErrorCode = m_strErrorDesc = _T("");

	m_strCurrencyType = m_strDCPrice = m_strDCQuantity = m_strDCUnitPrice = m_strDCCommit = _T("");												// [#2496] US Justin 2017.08.14
	m_strBTPublicKey = m_strBTPrivateKey = _T("");																								// [#2503]
	m_strETWalletFile = m_strETPassPhase = m_strETAddress = m_strWalletImage = _T("");															// [#2503] US Justin 2017.09.05

	m_strPhoneNum = m_strTxId = m_strCode = m_strAmount = m_strMinAmount = m_strMaxAmount = m_strServerMessage = m_strVaultBalance = _T("");	// [#RWC6-477] US ryan.payton 2022.11.21 Purchase Bitcoin by Cash
}

CString CJustCashData::MakeRequestMessage(CString strTerminalID, CString strErrorCode, CString strLast6Digit) // [#2528] US Justin 2018.01.29 Add Parameter
{
	NHDEBUG(DBG_INFO, (L"JustCash MakeRequestMessage(TID=[%s], Err=[%s]\n", strTerminalID, strErrorCode));
	m_strTID = strTerminalID.TrimRight();

	// Login (OLD)	    https://hyosung.just.cash/jcda/justcash/atm/login?p_username=test03&encrypt_password=jZae727K08KaOmKSgOaGzww%2FXVqGr%2FPKEgIMkjrcbJI%3D&encrypt_pin=1234&pdevice_key=MNJ45TC32TEWLHWU&sessionKey=th1b13gj4qpensrtgl1be58i7pm3u7nig2ild34q
	// LOGIN 2018.01.29 https://secure.just.cash/jcda/justcash/atm/login?username=hyatm8&password=showhy8&deviceKey=HY1818

	// Submit Code	https://secure.just.cash/jcda/justcash/atm/purchase_code?terminal_id=J1&pay_code=12345&sessionKey=8u9f4sttcbai62fdp30n1h3qtq9k4i5fb8rvkro2
	// Tran Update	https://hyosung.just.cash/jcda/justcash/atm/error
	// BitCoin Purchase  https://secure.just.cash/jcda/justcash/atm/btc_purchase?terminal_id=J1&amount=20&sessionKey=8u9f4sttcbai62fdp30n1h3qtq9k4i5fb8rvkro2
	// Bitcoin Commit(OLD) https://secure.just.cash/jcda/justcash/atm/btc_commit?terminal_id=J1&public_key=mhKDEwgN5j3j7GD8DmW29RJnNbjJhm3n5L&is_commit=1&sessionKey=8u9f4sttcbai62fdp30n1h3qtq9k4i5fb8rvkro2
	// Bitcoin Commit	   https://secure.just.cash/jcda/justcash/atm/btc_commit?terminal_id=J1&public_key=mhKDEwgN5j3j7GD8DmW29RJnNbjJhm3n5L&is_commit=1&encrypt_card_hash=dkkdkdkdk&sessionKey=8u9f4sttcbai62fdp30n1h3qtq9k4i5fb8rvkro2
	// Ethereum Purchase https://secure.just.cash/jcda/justcash/eth/coins_purchase?terminal_id=J1&amount=20&sessionKey=8u9f4sttcbai62fdp30n1h3qtq9k4i5fb8rvkro2
	// Ethereum Commit(OLD)   https://secure.just.cash/jcda/justcash/eth/coins_commit?terminal_id=J1&public_key=mhKDEwgN5j3j7GD8DmW29RJnNbjJhm3n5L&is_commit=1&sessionKey=8u9f4sttcbai62fdp30n1h3qtq9k4i5fb8rvkro2
	// Ethereum Commit        https://secure.just.cash/jcda/justcash/eth/coins_commit?terminal_id=J1&public_key=mhKDEwgN5j3j7GD8DmW29RJnNbjJhm3n5L&is_commit=1&encrypt_card_hash=dkkdkdkdk&sessionKey=8u9f4sttcbai62fdp30n1h3qtq9k4i5fb8rvkro2

	CString strReturn = m_strHostProcess; // jcda/justcash
	strReturn += L"/";

	CString strTemp = L"";

	if (m_nTransactionStep == JUSTCASH_SETUP_LOGIN)
	{
		strTemp.Format(L"atm/login?username=%s&password=%s&deviceKey=%s", m_strUserName, m_strEncryptPWD, m_strDevKey); // [#2528] US Justin 2018.01.29 Just.Cash Change Login Method
		// strTemp.Format(L"atm/login?p_username=%s&encrypt_password=%s&encrypt_pin=%s&pdevice_key=%s", m_strUserName, m_strEncryptPWD, m_strEncrpytPIN, m_strDevKey);
	}
	else if (m_nTransactionStep == JUSTCASH_CARDLESS_TRAN_SUBMITCODE)
	{
		strTemp.Format(L"atm/purchase_code?terminal_id=%s&pay_code=%s&sessionKey=%s", m_strTID, m_strCashCode, m_strSessionKey);
	}
	else if (m_nTransactionStep == JUSTCASH_CARDLESS_TRAN_UPDATETRAN)
	{
		CString sDate = GetDate();
		CString sTime = GetTime();

		strTemp.Format(L"atm/error?code=%s&optional_text=", strErrorCode);																					strReturn += strTemp;
		strTemp.Format(L"{pcode:%s,", m_strCashCode);																										strReturn += strTemp;
		strTemp.Format(L"ack_code:%s,", GetHashCode());																										strReturn += strTemp;
		strTemp.Format(L"amt:%s,", m_strTranAmount);																										strReturn += strTemp;
		strTemp.Format(L"actual_amt:%s,", m_strTranDispensed);																								strReturn += strTemp;
		strTemp.Format(L"datetime:%s-%s-%sT%s:%s:%sZ}", sDate.Left(4), sDate.Mid(4, 2), sDate.Mid(6, 2), sTime.Left(2), sTime.Mid(2, 2), sTime.Mid(4, 2));	strReturn += strTemp;
		strTemp.Format(L"&sessionKey=%s", m_strSessionKey);
	}
	// [#2496] US Justin 2017.08.14 Add Bit Coin
	else if (m_nTransactionStep == JUSTCASH_DIGITAL_CUR_PURCHASE)
	{
		CString strCardHash = GetMD5(strLast6Digit); // US William 2020.08.11 Add hash for KYC on purchase call
		// [#2503] US Justin 2017.09.06
		// strTemp.Format(L"atm/btc_purchase?terminal_id=%s&amount=%0.2f&sessionKey=%s", m_strTID, 0.01*Asc2Int(m_strTranAmount), m_strSessionKey);

		if (m_strCurrencyType == JUSTCASH_CURRENCY_BITCOIN)
			strTemp.Format(L"atm/btc_purchase?terminal_id=%s&amount=%0.2f&encrypted_card_hash=%s&sessionKey=%s", m_strTID, 0.01 * Asc2Int(m_strTranAmount), strCardHash, m_strSessionKey);
		else if (m_strCurrencyType == JUSTCASH_CURRENCY_ETHEREUM)
			strTemp.Format(L"eth/coins_purchase?terminal_id=%s&amount=%0.2f&sessionKey=%s", m_strTID, 0.01 * Asc2Int(m_strTranAmount), m_strSessionKey);
		// End of [#2503]
	}
	else if (m_nTransactionStep == JUSTCASH_DIGITAL_CUR_COMMIT)
	{
		// [#2528] US Justin 2018.01.29
		CString strCardHash = GetMD5(strLast6Digit);
		NHDEBUG(DBG_INFO, (L"JustCash Request Card Last 6 Digit MD5 HASH. Last 6 Digit=[%s] => MD5=[%s]\n", strLast6Digit, strCardHash));
		// End of [#2528]

		// [#2503] US Justin 2017.09.06
		// strTemp.Format(L"atm/btc_commit?terminal_id=%s&public_key=%s&is_commit=%s&sessionKey=%s", m_strTID, m_strBTPublicKey, m_strDCCommit, m_strSessionKey);

		if (m_strCurrencyType == JUSTCASH_CURRENCY_BITCOIN)
			strTemp.Format(L"atm/btc_commit?terminal_id=%s&public_key=%s&is_commit=%s&encrypt_card_hash=%s&sessionKey=%s", m_strTID, m_strBTPublicKey, m_strDCCommit, strCardHash, m_strSessionKey); // [#2528] US Justin 2018.01.29
		else if (m_strCurrencyType == JUSTCASH_CURRENCY_ETHEREUM)
			strTemp.Format(L"eth/coins_commit?terminal_id=%s&public_key=%s&is_commit=%s&encrypt_card_hash=%s&sessionKey=%s", m_strTID, m_strETAddress, m_strDCCommit, strCardHash, m_strSessionKey); // [#2528] US Justin 2018.01.29
		// End of [#2503]
	}
	// [#2496]
	// [#RWC6-477] US ryan.payton 2022.11.21 Purchase Bitcoin by Cash
	else if (m_nTransactionStep == JUSTCASH_DIGITAL_CUR_CASH_PHONE)
	{
		strTemp.Format(L"cash/phone?terminal_id=%s&phone=%s&sessionKey=%s", m_strTID, m_strPhoneNum, m_strSessionKey);
	}
	else if (m_nTransactionStep == JUSTCASH_DIGITAL_CUR_CASH_CODE)
	{
		strTemp.Format(L"cash/request?terminal_id=%s&tx_id=%s&code=%s&sessionKey=%s", m_strTID, m_strTxId, m_strCode, m_strSessionKey);
	}
	else if (m_nTransactionStep == JUSTCASH_DIGITAL_CUR_CASH_ADD)
	{
		strTemp.Format(L"cash/amount/add?terminal_id=%s&tx_id=%s&amount=%s&vault_balance=%s&sessionKey=%s", m_strTID, m_strTxId, m_strAmount, m_strVaultBalance, m_strSessionKey);
	}
	else if (m_nTransactionStep == JUSTCASH_DIGITAL_CUR_CASH_COMMIT)
	{
		strTemp.Format(L"cash/commit?terminal_id=%s&tx_id=%s&vault_balance=%s&sessionKey=%s", m_strTID, m_strTxId, m_strVaultBalance, m_strSessionKey);
	}
	else if (m_nTransactionStep == JUSTCASH_DIGITAL_CUR_VAULT_BALANCE)
	{
		strTemp.Format(L"cash/vault/balance?terminal_id=%s&vault_balance=%s&sessionKey=%s", m_strTID, m_strVaultBalance, m_strSessionKey);
	}
	else if (m_nTransactionStep == JUSTCASH_DIGITAL_CUR_INFO)
	{
		strTemp.Format(L"atm/btc_info?terminal_id=%s&sessionKey=%s", m_strTID, m_strSessionKey);
	}
	// End of [#RWC6-477]

	strReturn += strTemp;

	NHDEBUG(DBG_INFO, (L"JustCash Request (type=%d) Message = \n%s", m_nTransactionStep, strReturn));
	return strReturn;
}

CString CJustCashData::MakeDemoResponseMessage()
{
	CString strIndicator = m_strCashCode.Left(1);
	CString strSendBody = L"{\r\n";

	if (strIndicator == L"9")	strSendBody += L"\"result\": \"error\",\r\n";	// TRANSACTION FAILURE
	else						strSendBody += L"\"result\": \"ok\",\r\n";		// TRANSACTION OK

	strSendBody += L"\"error\": null,\r\n";
	strSendBody += L"\"data\": ";

	// Make Response
	if (m_nTransactionStep == JUSTCASH_SETUP_LOGIN) // Login (Registering)
	{
		strSendBody += L"{\r\n    \"items\": [\r\n";
		strSendBody += L"{\r\n\"sessionKey\": \"ATMe6c22f6ab59111e681670a9dc08c19bd\"\r\n}";
		strSendBody += L"]\r\n        }\r\n";
	}
	else if (m_nTransactionStep == JUSTCASH_CARDLESS_TRAN_SUBMITCODE) // Submit Code (Transaction)
	{
		CString strTemp;
		strSendBody += L"{\r\n    \"items\": [\r\n{\r\n";
		strSendBody += L"\"amount\": \"20.00\", \r\n";
		strSendBody += L"\"error_text\": \"Cash Code....\", \r\n";
		strSendBody += L"\"error_code\": \"01\", \r\n";
		strSendBody += L"\"user_id\": \"1\", \r\n";
		strSendBody += L"\"atm_type\": \"1\", \r\n";

		if (strIndicator == L"1")	strTemp.Format(L"\"code\": \"%s\" \r\n", GetHashCode());	// ON US TRANSACTION
		else						strTemp = L"\"track\": \"1234567890123456=9912\" \r\n";		// OFF US TRANSACTION

		strSendBody += strTemp;
		strSendBody += L"}\r\n";
		strSendBody += L"          ]\r\n        }\r\n";
	}
	// [#2496] US Justin 2017.8.14 Add Bitcoin
	else if (m_nTransactionStep == JUSTCASH_DIGITAL_CUR_PURCHASE) // Purchase Bitcoin
	{
		CString strTemp;
		strSendBody += L"{\r\n    \"items\": [\r\n{\r\n";

		if (m_strCurrencyType == JUSTCASH_CURRENCY_BITCOIN)
		{
			strTemp.Format(L"\"btcPrice\": \"%0.2f\", \r\n", 0.01 * Asc2Int(m_strTranAmount));
			strSendBody += strTemp;

			strTemp.Format(L"\"btcQuantity\": \"%0.6f\", \r\n", 0.01 * Asc2Int(m_strTranAmount) / 2349.004);
			strSendBody += strTemp;

			strSendBody += L"\"btcWholeUnitPrice\": \"2349.0004\", \r\n";
			strSendBody += L"\"btcPublicKey\": \"ms6mQcjZmHQorm2VKgHRYUjeB5zfYB6bmu\", \r\n";
			strSendBody += L"\"btcPrivateKey\": \"cNrks4dUPPT39HrZv6w3CfSPVga1KmXgrV6eVJjYo3F62XXiH6Fu\" \r\n";
		}
		else if (m_strCurrencyType == JUSTCASH_CURRENCY_ETHEREUM) // [#2503]
		{
			// strTemp.Format(L"\"ethPrice\": \"%0.2f\", \r\n", 0.01*Asc2Int(m_strTranAmount));
			// strSendBody += strTemp;

			strSendBody += L"\"address\": \"0x3c65c6d534386ad40ac5c691dc747c73d1c94fb6\", \r\n";
			strSendBody += L"\"passPhrase\": \"tBYSQJyPfpOrXGoToqqA\", \r\n";

			strTemp.Format(L"\"ethQuantity\": \"%0.4f\", \r\n", 0.01 * Asc2Int(m_strTranAmount) / 250.0);
			strSendBody += strTemp;

			strSendBody += L"\"ethWholeUnitPrice\": \"250.0\", \r\n";
			strSendBody += L"\"qrCodeWalletFile\": \"USE EXISTING FILE\", \r\n";

			strSendBody += L"\"walletFileContent\":{\"address\": \"3c65c6d534386ad40ac5c691dc747c73d1c94fb6\",\"id\": \"d66bbbde-250a-4117-94a5-3f132047c1ff\",";
			strSendBody += L"\"version\": 3,\"crypto\":{\"cipher\": \"aes-128-ctr\",\"ciphertext\": \"1d7633413aeccafb1b15bdab1488da3e26e092fbe9af79aeafc48a40ffd92e68\",";
			strSendBody += L"\"cipherparams\":{\"iv\":\"3706eb0ef87f785c5bfad8f126344ed6\"},\"kdf\": \"scrypt\",\"kdfparams\":{\"dklen\": 32,\"n\": 262144,\"p\": 1,\"r\": 8,";
			strSendBody += L"\"salt\": \"ded22f70a780f4cabe1a66241ef9d42f6db4ba56dfcd13902ea1242cbe177e2c\"},\"mac\": \"de2b0502ca7e68a8d0b0e63b787c7e9a6575ee7e71d69be451b187aa4c3a2157\"}} \r\n";
		}

		strSendBody += L"}\r\n          ]\r\n        }\r\n";
	}
	else if (m_nTransactionStep == JUSTCASH_DIGITAL_CUR_COMMIT) // Commit Bitcoin
	{
		strSendBody += L"{ }\r\n";
	}
	// End of [#2496]
	else if (m_nTransactionStep == JUSTCASH_CARDLESS_TRAN_UPDATETRAN) // Update Transaction for Dispense Error
	{
		strSendBody += L"{ }\r\n";
	}
	else
	{
		strSendBody += L"{ }\r\n";
	}

	strSendBody += L"}\r\n";

	return strSendBody;
}

BOOL CJustCashData::ParseReceivedData(CString strReceived)
{
	NHDEBUG(DBG_INFO, (L"JustCash ParseReceivedData, TransactionStep = [%d]\n", m_nTransactionStep));
	int err = 0;
	JSON_Value *rootValue;
	JSON_Object *json;
	JSON_Array *items;
	JSON_Object *item;
	JSON_Object *error;

	// Split response string at the double line break (the beginning of content)
	int contentIndex = strReceived.Find(L"\r\n\r\n") + 4;
	CString content = strReceived.Mid(contentIndex);

	int responseLen = content.GetLength();
	char *jsonData = new char[responseLen + 1]();
	WideToMulti(jsonData, content, responseLen);

#ifdef _DEBUG
	if (content.GetLength() <= 1024)
	{
		NHDEBUG(DBG_CALL, (L"PARSE Response, data = \n%s\n", content));
	}
	else
	{
		NHDEBUG(DBG_CALL, (L"PARSE Response, data, Length = [%d]\n%s\n...................\n%s\n", content.GetLength(), content.Left(450), content.Right(450)));
	}
#endif

	// Parse the JSON
	rootValue = json_parse_string(jsonData);
	json = json_value_get_object(rootValue);

	// Check "error"
	if (m_nTransactionStep == JUSTCASH_CARDLESS_TRAN_SUBMITCODE)
	{
		items = json_object_dotget_array(json, "data.items");
		item = json_array_get_object(items, 0);

		m_strServerMessage = CString(json_object_get_string(item, "error_text"));
		NHDEBUG(DBG_INFO, (L"PARSE Response, server_message=[%s]\n", m_strServerMessage));
	}
	else
	{
		error = json_object_dotget_object(json, "error");
		m_strServerMessage = CString(json_object_get_string(error, "server_message"));
		NHDEBUG(DBG_INFO, (L"PARSE Response, server_message=[%s]\n", m_strServerMessage));
	}

	// Check "result"
	CString strValue = CString(json_object_get_string(json, "result"));
	NHDEBUG(DBG_INFO, (L"PARSE Response, result=[%s]\n", strValue));

	if (strValue.CompareNoCase(L"ok") != 0)
	{
		err = 901;
		goto cleanup;
	}

	if ((m_nTransactionStep == JUSTCASH_SETUP_LOGIN)				|| // OP Registering
		(m_nTransactionStep == JUSTCASH_CARDLESS_TRAN_SUBMITCODE)	|| // Submit Code : Cardless Transaction
		(m_nTransactionStep == JUSTCASH_DIGITAL_CUR_PURCHASE)		|| // Bitcoin or Ethereum Purchase : Purchasing Digital Currency
		// [#RWC6-477] US ryan.payton 2022.11.21 Purchase Bitcoin by Cash
		(m_nTransactionStep == JUSTCASH_DIGITAL_CUR_CASH_PHONE)		||
		(m_nTransactionStep == JUSTCASH_DIGITAL_CUR_CASH_CODE)		||
		(m_nTransactionStep == JUSTCASH_DIGITAL_CUR_CASH_ADD)		||
		(m_nTransactionStep == JUSTCASH_DIGITAL_CUR_CASH_COMMIT)	||
		(m_nTransactionStep == JUSTCASH_DIGITAL_CUR_VAULT_BALANCE)	||
		(m_nTransactionStep == JUSTCASH_DIGITAL_CUR_INFO)
		// End of [#RWC6-477]
	)
	{
		// Check for "data" path
		if (json_object_dothas_value_of_type(json, "data", JSONObject) == 0)
		{
			// No "data" path
			err = 902;
			goto cleanup;
		}

		if (m_nTransactionStep == JUSTCASH_SETUP_LOGIN)
		{
			// Just extract the session key and call it good!
			if (json_object_dothas_value_of_type(json, "data.sessionKey", JSONString) == 0)
			{
				err = 908;
				goto cleanup;
			}

			m_strSessionKey = CString(json_object_dotget_string(json, "data.sessionKey"));
			NHDEBUG(DBG_INFO, (L"PARSE Response, sessionKey=[%s]\n", m_strSessionKey));

			if (SaveSetting() != TRUE)
			{
				m_strSessionKey = L"";

				err = 909;
				goto cleanup;
			}

			m_bJustCashRegistered = TRUE;
		}
		else if (m_nTransactionStep == JUSTCASH_CARDLESS_TRAN_SUBMITCODE)
		{
			items = json_object_dotget_array(json, "data.items");
			item = json_array_get_object(items, 0);

			m_strTranAmount = CString(json_object_get_string(item, "amount"));
			NHDEBUG(DBG_INFO, (L"PARSE Response, amount=[%s]\n", m_strTranAmount));

			CString strErrorText = CString(json_object_get_string(item, "error_text"));
			NHDEBUG(DBG_INFO, (L"PARSE Response, error_text=[%s]\n", strErrorText));

			CString strErrorCode = CString(json_object_get_string(item, "error_code"));
			NHDEBUG(DBG_INFO, (L"PARSE Response, error_code=[%s]\n", strErrorCode));

			m_strTrackData = CString(json_object_get_string(item, "track"));
			NHDEBUG(DBG_INFO, (L"PARSE Response, track=[%s]\n", m_strTrackData));

			if (m_strTranAmount.GetLength() <= 0)
			{
				err = 904;
				goto cleanup;
			}
			else if (Dollar2Cent(m_strTranAmount) <= 0)
			{
				err = 904;
				goto cleanup;
			}

			CStringArray strArrayTrack;
			SplitString(m_strTrackData, L"=", strArrayTrack);
			if (strArrayTrack.GetSize() < 2)
			{
				err = 907;
				goto cleanup;
			}

			CString sAccountNum = strArrayTrack.GetAt(0);
			if (sAccountNum.GetLength() < 15)
			{
				err = 907;
				goto cleanup;
			}

			NHDEBUG(DBG_INFO, (L"PARSE Response : SUCCESSFUL... OFF US TRANSACTION\n"));
			m_bIsOnUsTransaction = FALSE;
		}
		else if (m_nTransactionStep == JUSTCASH_DIGITAL_CUR_PURCHASE)
		{
			items = json_object_dotget_array(json, "data.items");
			item = json_array_get_object(items, 0);

			if (m_strCurrencyType == JUSTCASH_CURRENCY_BITCOIN)
			{
				// Bitcoin Price (ex. $20.0)
				m_strDCPrice = CString(json_object_get_string(item, "btcPrice"));
				NHDEBUG(DBG_INFO, (L"PARSE Response, btcPrice=[%s]\n", m_strDCPrice));
				if (m_strDCPrice.GetLength() <= 0)
				{
					err = 921;
					goto cleanup;
				}

				// Bitcoin quantity
				m_strDCQuantity = CString(json_object_get_string(item, "btcQuantity"));
				NHDEBUG(DBG_INFO, (L"PARSE Response, btcQuantity=[%s]\n", m_strDCQuantity));
				if (m_strDCQuantity.GetLength() <= 0)
				{
					err = 922;
					goto cleanup;
				}

				// Bitcoin unit price
				m_strDCUnitPrice = CString(json_object_get_string(item, "btcWholeUnitPrice"));
				NHDEBUG(DBG_INFO, (L"PARSE Response, btcWholeUnitPrice=[%s]\n", m_strDCUnitPrice));
				if (m_strDCUnitPrice.GetLength() <= 0)
				{
					err = 923;
					goto cleanup;
				}

				// Bitcoin Public key
				m_strBTPublicKey = CString(json_object_get_string(item, "btcPublicKey"));
				NHDEBUG(DBG_INFO, (L"PARSE Response, btcPublicKey=[%s]\n", m_strBTPublicKey));
				if (m_strBTPublicKey.GetLength() <= 0)
				{
					err = 924;
					goto cleanup;
				}

				// Bitcoin Private key
				m_strBTPrivateKey = CString(json_object_get_string(item, "btcPrivateKey"));
				NHDEBUG(DBG_INFO, (L"PARSE Response, btcPrivateKey=[%s]\n", m_strBTPrivateKey));
				if (m_strBTPrivateKey.GetLength() <= 0)
				{
					err = 925;
					goto cleanup;
				}

				NHDEBUG(DBG_INFO, (L"PARSE Response : SUCCESSFUL... Bitcoin Purchase\n"));
			}
			else
			{
				err = 920; // Undefined Digital Currency
				goto cleanup;
			}
		}
		// [#RWC6-477] US ryan.payton 2022.11.21 Purchase Bitcoin by Cash
		else if (m_nTransactionStep == JUSTCASH_DIGITAL_CUR_CASH_PHONE)
		{
			items = json_object_dotget_array(json, "data.items");
			item = json_array_get_object(items, 0);

			// Transaction Id
			m_strTxId = CString(json_object_get_string(item, "tx_id"));
			NHDEBUG(DBG_INFO, (L"PARSE Response, tx_id=[%s]\n", m_strTxId));
			if (m_strTxId.GetLength() <= 0)
			{
				err = 910;
				goto cleanup;
			}
		}
		else if (m_nTransactionStep == JUSTCASH_DIGITAL_CUR_CASH_CODE)
		{
			items = json_object_dotget_array(json, "data.items");
			item = json_array_get_object(items, 0);

			// Bitcoin unit price
			m_strDCUnitPrice = CString(json_object_get_string(item, "btcWholeUnitPrice"));
			NHDEBUG(DBG_INFO, (L"PARSE Response, btcWholeUnitPrice=[%s]\n", m_strDCUnitPrice));
			if (m_strDCUnitPrice.GetLength() <= 0)
			{
				err = 923;
				goto cleanup;
			}

			// Minimum Amount
			m_strMinAmount = CString(json_object_get_string(item, "min_amount"));
			NHDEBUG(DBG_INFO, (L"PARSE Response, min_amount=[%s]\n", m_strMinAmount));
			if (m_strMinAmount.GetLength() <= 0)
			{
				err = 911;
				goto cleanup;
			}

			// Maximum Amount
			CString strMaxAmount = CString(json_object_get_string(item, "max_amount"));
			NHDEBUG(DBG_INFO, (L"PARSE Response, max_amount=[%s]\n", strMaxAmount));

			if (strMaxAmount.GetLength() <= 0)
			{
				err = 912;
				goto cleanup;
			}

			// Fix for formatting
			CStringArray strSplit;
			SplitString(strMaxAmount, '.', strSplit);

			m_strMaxAmount = strSplit[0];
			NHDEBUG(DBG_INFO, (L"FORMAT Response, max_amount=[%s]\n", m_strMaxAmount));

			if (m_strMaxAmount.GetLength() <= 0)
			{
				err = 912;
				goto cleanup;
			}

			// Bitcoin Public key
			m_strBTPublicKey = CString(json_object_get_string(item, "btcPublicKey"));
			NHDEBUG(DBG_INFO, (L"PARSE Response, btcPublicKey=[%s]\n", m_strBTPublicKey));
			if (m_strBTPublicKey.GetLength() <= 0)
			{
				err = 924;
				goto cleanup;
			}

			// Bitcoin Private key
			m_strBTPrivateKey = CString(json_object_get_string(item, "btcPrivateKey"));
			NHDEBUG(DBG_INFO, (L"PARSE Response, btcPrivateKey=[%s]\n", m_strBTPrivateKey));
			if (m_strBTPrivateKey.GetLength() <= 0)
			{
				err = 925;
				goto cleanup;
			}
		}
		else if (m_nTransactionStep == JUSTCASH_DIGITAL_CUR_CASH_ADD)
		{
			items = json_object_dotget_array(json, "data.items");
			item = json_array_get_object(items, 0);

			// Bitcoin quantity
			m_strDCQuantity = CString(json_object_get_string(item, "btcAmount"));
			NHDEBUG(DBG_INFO, (L"PARSE Response, btcAmount=[%s]\n", m_strDCQuantity));
			if (m_strDCQuantity.GetLength() <= 0)
			{
				err = 922;
				goto cleanup;
			}
		}
		else if (m_nTransactionStep == JUSTCASH_DIGITAL_CUR_INFO)
		{
			items = json_object_dotget_array(json, "data.items");
			item = json_array_get_object(items, 0);

			// Minimum Amount
			m_strMinAmount = CString(json_object_get_string(item, "min_amount"));
			NHDEBUG(DBG_INFO, (L"PARSE Response, min_amount=[%s]\n", m_strMinAmount));
			if (m_strMinAmount.GetLength() <= 0)
			{
				err = 911;
				goto cleanup;
			}
		}
		// End of [#RWC6-477]
	}

cleanup:

	memset(jsonData, 0, responseLen);
	delete[] jsonData;
	json_value_free(rootValue);

	if (err != 0)
	{
		CString errorCode;
		errorCode.Format(L"%d", err);
		SetErrorCodeAndDesc(errorCode);
		return FALSE;
	}

	NHDEBUG(DBG_INFO, (L"JustCash ParseReceivedData return TRUE\n"));
	return TRUE;
}

void CJustCashData::SetErrorCodeAndDesc(CString sResCode, LPCTSTR sErrText)
{
	m_strErrorCode.Format(L"DJ%03s00", sResCode);
	// 1234567890123456789012345678901234567890

	if		(sResCode == "901") m_strErrorDesc = L"Denied by JC host";
	else if (sResCode == "902") m_strErrorDesc = L"JC resp. data Error";
	else if (sResCode == "903") m_strErrorDesc = L"JC resp. items Error";
	else if (sResCode == "904") m_strErrorDesc = L"Invalid Amount";
	else if (sResCode == "905") m_strErrorDesc = L"Invalid Approval data";
	else if (sResCode == "906") m_strErrorDesc = L"Approval Code Error";
	else if (sResCode == "907") m_strErrorDesc = L"Invalid Account Number";
	else if (sResCode == "908") m_strErrorDesc = L"Invalid SessingKey";
	else if (sResCode == "909") m_strErrorDesc = L"Saving Error - Setting";
	// [#RWC6-477] US ryan.payton 2022.11.21 Purchase Bitcoin by Cash
	else if (sResCode == "910") m_strErrorDesc = L"Cash Transaction Id Error";
	else if (sResCode == "911") m_strErrorDesc = L"Minimum Amount Error";
	else if (sResCode == "912") m_strErrorDesc = L"Maximum Amount Error";
	// End of [#RWC6-477]
	// [#2496] US Justin 2017.08.15 Add Bitcoin
	else if (sResCode == "920") m_strErrorDesc = L"Undefined Digital Currency";
	else if (sResCode == "921") m_strErrorDesc = L"Bitcoin Price Error";
	else if (sResCode == "922") m_strErrorDesc = L"Bitcoin Quantity Error";
	else if (sResCode == "923") m_strErrorDesc = L"Bitcoin Unit Price Error";
	else if (sResCode == "924") m_strErrorDesc = L"Bitcoin Public Key Error";
	else if (sResCode == "925") m_strErrorDesc = L"Bitcoin Private Key Error";
	else if (sResCode == "930") m_strErrorDesc = L"Bitcoin Printer Error";
	// End of [#2496]
	// [#2503]
	else if (sResCode == "931") m_strErrorDesc = L"Ethereum Address Error";
	else if (sResCode == "932") m_strErrorDesc = L"Ethereum PassPhase Error";
	else if (sResCode == "933") m_strErrorDesc = L"Ethereum Quantity Error";
	else if (sResCode == "934") m_strErrorDesc = L"Ethereum Unit Price Error";
	else if (sResCode == "935") m_strErrorDesc = L"Ethereum Wallet Conents Error";
	// End of [#2503]
	else						m_strErrorDesc = L"Transaction Error";

	if (sErrText != NULL)		m_strErrorDesc = CString(sErrText);

	NHDEBUG(DBG_INFO, (L"SET ERROR CODE: ORG CODE=[%s], ATM CODE=[%s], DESC=[%s]\n", sResCode, m_strErrorDesc, m_strErrorDesc));
	NVDump('O', 'C', "00", m_strErrorCode, L"JustCash ERR");
}

////////////////////////////////////////////////////////////
// Day Total Function
void CJustCashData::ResetTotal(int nService)
{
	NHDEBUG(DBG_INFO, (L"CJustCashData::ResetTotal()\n"));

	// [#2496] US Justin 2017.08.17
	// SaveDispenseResult(TRUE);
	if (nService & JUSTCASH_SERVICE_CARDLESS)
	{
		m_strJCDayTotalDateTime.Format(L"%s%s", GetDate(), GetTime());
		m_strJCDayTotalDispCount = L"0";
		m_strJCDayTotalDispAmt = L"0";
	}

	if (nService & JUSTCASH_SERVICE_DIGITAL_CURRENCY)
	{
		m_strDCDayTotalDateTime.Format(L"%s%s", GetDate(), GetTime());
		m_strDCDayTotalDispCount = L"0";
		m_strDCDayTotalDispAmt = L"0";
	}

	SaveDispenseResult();
	// End of [#2496]
}

BOOL CJustCashData::ReadDispenseHistory()
{
	NHDEBUG(DBG_INFO, (L"CJustCashData::ReadDispenseHistory()\n"));
	CString strPath;
	CFile cf;

	///////////////////////////////////////////////////////////////
	// Read Dispense History
	///////////////////////////////////////////////////////////////
	strPath.Format(L"%s\\%s", ATM2_DATA_PATH, JUSTCASH_DISPENSERESULT_FILE);

	if (cf.Open(strPath, CFile::modeRead) == FALSE)
	{
		NHDEBUG(DBG_INFO, (L"[FAIL TO OPEN Diepsnse History(%s) => Reset Dispense Result]\n", strPath));
		SaveDispenseResult(TRUE);
		return TRUE;
	}

	int nFileLen = (int)cf.GetLength();
	unsigned char *pBuffer1 = new unsigned char[nFileLen + 2]; // Allocate buffer for binary file data

	if (pBuffer1 == NULL)
	{
		cf.Close();
		NHDEBUG(DBG_INFO, (L"[FAIL TO NEW BUFFER(%d)] => Reset Dispense Result\n", nFileLen + 2));
		SaveDispenseResult(TRUE);
		return TRUE;
	}

	nFileLen = cf.Read(pBuffer1, nFileLen);
	cf.Close();

	CString strDispData = (LPCSTR)pBuffer1;
	delete[] pBuffer1;
	NHDEBUG(DBG_INFO, (L"   Dispense Data =[%s]\n", strDispData));

	CStringArray arrTemp;
	SplitString(strDispData, L",", arrTemp);

	// Daytotal Info  (6) :			Date and Time, Dispense Count, Dispense Amount
	//								Date and Time, Dispense Count, Dispense Amount for Digital Currency
	if (arrTemp.GetSize() < 6)
	{
		NHDEBUG(DBG_INFO, (L"[Dispense Information is wrong or not set => Reset Dispense Result]\n"));
		SaveDispenseResult(TRUE);
		return TRUE;
	}

	BOOL bReadSuccessful = TRUE;

	// DayTotal Date and Time
	CString strTemp = arrTemp.GetAt(0);
	NHDEBUG(DBG_INFO, (L"   m_strJCDayTotalDateTime = [%s]\n", strTemp));

	if (strTemp.GetLength() == 14)
		m_strJCDayTotalDateTime = strTemp;
	else
		bReadSuccessful = FALSE;

	// DayTotal Dispense Count
	if (bReadSuccessful)
	{
		strTemp = arrTemp.GetAt(1);
		NHDEBUG(DBG_INFO, (L"   m_strJCDayTotalDispCount = [%s]\n", strTemp));

		if (Asc2Int(strTemp) >= 0)
			m_strJCDayTotalDispCount = strTemp;
		else
			bReadSuccessful = FALSE;
	}

	// DayTotal Total Dispense Amount
	if (bReadSuccessful)
	{
		strTemp = arrTemp.GetAt(2);
		NHDEBUG(DBG_INFO, (L"   m_strJCDayTotalDispAmt = [%s]\n", strTemp));

		if (Asc2Int(strTemp) >= 0)
			m_strJCDayTotalDispAmt = strTemp;
		else
			bReadSuccessful = FALSE;
	}

	// [#2496] US Justin 2017.08.17 BitCoin
	// DayTotal Date and Time
	if (bReadSuccessful)
	{
		strTemp = arrTemp.GetAt(3);
		NHDEBUG(DBG_INFO, (L"   m_strDCDayTotalDateTime = [%s]\n", strTemp));

		if (strTemp.GetLength() == 14)
			m_strDCDayTotalDateTime = strTemp;
		else
			bReadSuccessful = FALSE;
	}

	// DayTotal Dispense Count
	if (bReadSuccessful)
	{
		strTemp = arrTemp.GetAt(4);
		NHDEBUG(DBG_INFO, (L"   m_strDCDayTotalDispCount = [%s]\n", strTemp));

		if (Asc2Int(strTemp) >= 0)
			m_strDCDayTotalDispCount = strTemp;
		else
			bReadSuccessful = FALSE;
	}

	// DayTotal Total Dispense Amount
	if (bReadSuccessful)
	{
		strTemp = arrTemp.GetAt(5);
		NHDEBUG(DBG_INFO, (L"   m_strDCDayTotalDispAmt = [%s]\n", strTemp));

		if (Asc2Int(strTemp) >= 0)
			m_strDCDayTotalDispAmt = strTemp;
		else
			bReadSuccessful = FALSE;
	}
	// End of [#2496]

	if (bReadSuccessful)
	{
		NHDEBUG(DBG_INFO, (L"CJustCashData::ReadDispenseHistory () SUCCESS\n"));
	}
	else
	{
		NHDEBUG(DBG_INFO, (L"[ERROR on Dispense Result Data]  => Reset Dispense Result\n"));
		SaveDispenseResult(TRUE);
	}

	return TRUE;
}

BOOL CJustCashData::SaveDispenseResult(BOOL bReset)
{
	NHDEBUG(DBG_INFO, (L"CJustCashData::SaveDispenseResult(%d) \n", bReset));

	CString strPath;
	strPath.Format(L"%s\\%s", ATM2_DATA_PATH, JUSTCASH_DISPENSERESULT_FILE);

	CFile cf;

	if (cf.Open(strPath, CFile::modeCreate | CFile::modeWrite) == FALSE)
	{
		NHDEBUG(DBG_INFO, (L"[FAIL TO OPEN(%s)]\n", strPath));
		return FALSE;
	}

	if (bReset)
	{
		m_strJCDayTotalDateTime.Format(L"%s%s", GetDate(), GetTime());
		m_strJCDayTotalDispCount = L"0";
		m_strJCDayTotalDispAmt = L"0";
		// [#2496] US Justin 2017.08.17 Purchasing BitCoin
		m_strDCDayTotalDateTime.Format(L"%s%s", GetDate(), GetTime());
		m_strDCDayTotalDispCount = L"0";
		m_strDCDayTotalDispAmt = L"0";
		// End of [#2496]
	}

	CString strTotalDispenseData = L"";
	strTotalDispenseData += m_strJCDayTotalDateTime + _T(",");
	strTotalDispenseData += m_strJCDayTotalDispCount + _T(",");
	strTotalDispenseData += m_strJCDayTotalDispAmt;

	// [#2496] US Justin 2017.08.17 Purchasing BitCoin
	strTotalDispenseData += L",";
	strTotalDispenseData += m_strDCDayTotalDateTime + _T(",");
	strTotalDispenseData += m_strDCDayTotalDispCount + _T(",");
	strTotalDispenseData += m_strDCDayTotalDispAmt;
	// End of [#2496]

	NHDEBUG(DBG_INFO, (L"   DispenseResult = [%s]\n", strTotalDispenseData));

	UINT nSize = strTotalDispenseData.GetLength();

	char *pBuffer = new char[nSize + 1];
	memset(pBuffer, 0, nSize + 1);
	WideToMulti(pBuffer, strTotalDispenseData, nSize);

	cf.Write(pBuffer, nSize);
	delete[] pBuffer;

	cf.Close();
	return TRUE;
}

void CJustCashData::RecodeDispenseResult(long nAmount /* Cent */, int nType)
{
	NHDEBUG(DBG_INFO, (L"CJustCashData::RecodeDispenseResult (Amt=%ld)\n", nAmount));

	BOOL bSaveResult = FALSE;

	if (nType == JUSTCASH_SERVICE_CARDLESS)
	{
		// Update Day Total ... Date Time
		if (m_strJCDayTotalDateTime.GetLength() != 14)
			m_strJCDayTotalDateTime.Format(L"%s%s", GetDate(), GetTime());

		NHDEBUG(DBG_INFO, (L"  m_strJCDayTotalDateTime = [%s]\n", m_strJCDayTotalDateTime));

		// Update Day Total ... Dispense Count
		long nTemp = Asc2Int(m_strJCDayTotalDispCount) + 1;
		m_strJCDayTotalDispCount.Format(L"%ld", nTemp);
		NHDEBUG(DBG_INFO, (L"  m_strJCDayTotalDispCount = [%s]\n", m_strJCDayTotalDispCount));

		// Update Day Total ... Dispense Amount
		nTemp = Asc2Int(m_strJCDayTotalDispAmt) + ((int)(0.01 * nAmount + 0.001)); // Dollar
		m_strJCDayTotalDispAmt.Format(L"%ld", nTemp);
		NHDEBUG(DBG_INFO, (L"  m_strJCDayTotalDispAmt = [%s]\n", m_strJCDayTotalDispAmt));
		bSaveResult = TRUE;
	}
	// [#2496] US Justin 2017.08.17 Purchasing BitCoin
	else if (nType == JUSTCASH_SERVICE_DIGITAL_CURRENCY)
	{
		// Update Day Total ... Date Time
		if (m_strDCDayTotalDateTime.GetLength() != 14)
			m_strDCDayTotalDateTime.Format(L"%s%s", GetDate(), GetTime());

		NHDEBUG(DBG_INFO, (L"  m_strDCDayTotalDateTime = [%s]\n", m_strDCDayTotalDateTime));

		// Update Day Total ... Issue Count
		long nTemp = Asc2Int(m_strDCDayTotalDispCount) + 1;
		m_strDCDayTotalDispCount.Format(L"%ld", nTemp);
		NHDEBUG(DBG_INFO, (L"  m_strDCDayTotalDispCount = [%s]\n", m_strDCDayTotalDispCount));

		// Update Day Total ... Dispense Amount
		nTemp = Asc2Int(m_strDCDayTotalDispAmt) + nAmount; // CENT !!!!!!!
		m_strDCDayTotalDispAmt.Format(L"%ld", nTemp);
		NHDEBUG(DBG_INFO, (L"  m_strDCDayTotalDispAmt = [%s]\n", m_strDCDayTotalDispAmt));
		bSaveResult = TRUE;
	}
	// End of [#2496]
	if (bSaveResult)
		SaveDispenseResult();
}

CString CJustCashData::GetDaytotalJournalData(int nType)
{
	NHDEBUG(DBG_INFO, (L"CJustCashData::GetDaytotalJournalData()\n"));

	CString strEJNLData = L"";

	if (nType == JUSTCASH_SERVICE_CARDLESS)
	{
		strEJNLData += m_strJCDayTotalDateTime + _T("^");
		strEJNLData += m_strJCDayTotalDispCount + _T("^");
		strEJNLData += m_strJCDayTotalDispAmt;
	}
	// [#2496] US Justin 2017.08.17 Purchasing BitCoin
	else if (nType == JUSTCASH_SERVICE_DIGITAL_CURRENCY)
	{
		strEJNLData += m_strDCDayTotalDateTime + _T("^");
		strEJNLData += m_strDCDayTotalDispCount + _T("^");
		strEJNLData += m_strDCDayTotalDispAmt;
	}
	// End of [#2496]

	NHDEBUG(DBG_INFO, (L"   Journal Data=[%s])\n", strEJNLData));
	return strEJNLData;
}

#endif
// End of [#2445]