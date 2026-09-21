#include "stdafx.h"

#include ".\Common\CmnLib.h"
#include ".\Tran\B4UService.h"

//#define NH_DEBUG	

#include ".\Common\NHDbgApi.h"

#define	DBG_CALL		1
#define DBG_INFO		1

#define B4U_QR_CODE_FILE ATM_DATA_PATH _T("\\B4UQr.png") // \\ATM\\Data\\B4UQr.png

#define JSON_NULL L"null"

// 128 KB buffer for HTTP requests
#define HTTP_BUFFER_SIZE (128 * 1024)

// [RWC6-117] Update JSON implementation
void json_set_string_b4u(CString value, JSON_Object *root, const char * name) 
{
	int len = value.GetLength() + 1;
	char * s = new char[len]();
	WideToMulti(s, value, len);
	json_object_dotset_string(root, name, s);
	delete [] s;
}
// end of [RWC6-117]

CB4UService::CB4UService(CString host)
{
	this->httpClient = new HTTP(host);
}

CB4UService::~CB4UService(void)
{
	delete this->httpClient;
}

BOOL CB4UService::RequestNewSession(B4UIni ini, B4USession& session) 
{
	NHDEBUG(DBG_INFO, (L"RequestNewSession\r\n"));

	// [#RWC6-77] Bitload4U Updated API and Enhancement
	CString requestJson;
	BOOL success;
	CString response;

	if (session.Type == B4U_SESS_TYPE_STAGE)
	{
		requestJson = CB4UService::SerializeSessionRequest(ini, session);
		response = this->PostData(ini.PostRoute, requestJson, session.IsCallbackNeeded, session.TerminalId, session.Type, ini, &success);
	}
	else if (session.Type == B4U_SESS_TYPE_REPORT)
	{
		requestJson = CB4UService::FinalizeSessionStatus(session);
		response = this->PostData(ini.PostRouteReport, requestJson, session.IsCallbackNeeded, session.TerminalId, session.Type, ini, &success);
	}
	else
	{
		NHERROR((L"Bad session step\r\n"));
		return false;
	}
	
	if (!session.IsCallbackNeeded)
	{
		NHERROR((L"Callback(GET) not required, returns true\r\n"));
		return true;
	}
	// end of [#RWC6-77]
	
	if (!success)
	{
		NHERROR((L"failed\r\n"));
		return false;
	}
	
	if (!CB4UService::DeserializeB4USession(response, session))
	{
		NHERROR((L"failed parsing\r\n"));
		return false;
	}

	return true;
}

BOOL CB4UService::GetSessionStatus(B4USession& session, B4USessionType sessionType, B4UIni ini, B4USessionStatus& sessionStatus)
{
	NHDEBUG(DBG_CALL, (L"Getting trx status for %s\r\n", session.TransactionId));

	BOOL success;
	CString	revRoute;

	if (session.Type == B4U_SESS_TYPE_PRESTAGE)
	{
		revRoute.Format(L"%s", ini.GetRoutePrestage);
	}
	else
	{
		CString	EncryptedTransactionID = EncryptB4UValue(session.TransactionId);
		EncryptedTransactionID.Replace('+','.');
		EncryptedTransactionID.Replace('/','_');
		EncryptedTransactionID.Replace('=','-');
		revRoute.Format(L"%s/%s", ini.GetRoute, EncryptedTransactionID);
	}

	CString response = this->GetData(revRoute, L"", sessionType, session.TerminalId, ini, sessionStatus, &success);
	if (!success)
	{
		NHERROR((L"failed\r\n"));
		return false;
	}

	if ((session.Type == B4U_SESS_TYPE_PRESTAGE) && (sessionStatus.Status == B4U_PRE_RECEVIED))
	{
		sessionStatus.ErrorIsSet = false;
		sessionStatus.Error.Code = 0;
		sessionStatus.Error.Description = L"";
	}

	if ((session.Type == B4U_SESS_TYPE_STAGE) && (sessionStatus.Status == B4U_NOT_PAID))
	{
		sessionStatus.ErrorIsSet = false;
		sessionStatus.Error.Code = 0;
		sessionStatus.Error.Description = L"";
		return true;
	}

	if (sessionStatus.HasError())
	{
		NHERROR((L"B4U request failed with error code %d - %s", sessionStatus.Error.Code, sessionStatus.Error.Description));
		return false;
	}

	if (!CB4UService::DeserializeB4USessionStatus(response, session.Currency, session.Type, sessionStatus))
	{
		NHERROR((L"failed parsing\r\n"));
		return false;
	}

	return true;
}

CString CB4UService::DecryptB4UValue(CString encryptedValue, BOOL *success)
{
	CString sDecrypted = L"";
	unsigned char iv[B4U_DecKeyLen] = {};
	int	len = encryptedValue.GetLength()+1;
	unsigned char * cDecrypted = new unsigned char[len]();

	int nDecrypted = Base64Decode_And_Decrypt_AES_128_ECB(encryptedValue, (unsigned char *)B4U_DecKey, 3, cDecrypted, iv);	// int nKeyType 2:Symmetric (without IV) for SecretKey from JCE
	if(nDecrypted > 0)
	{
		sDecrypted.Format(L"%S", cDecrypted);
		sDecrypted = sDecrypted.Left(nDecrypted);
		*success = true;
		NHDEBUG(DBG_INFO, (L" Decryption OK, value=[%s] length=[%d]\n", sDecrypted, nDecrypted));
	}else
	{
		*success = false;
		NHDEBUG(DBG_INFO, (L" Decryption FAILED!!!\n"));
	}

	memset(cDecrypted,0,sizeof(cDecrypted));
	delete [] cDecrypted;

	return sDecrypted;
}


CString CB4UService::EncryptB4UValue(CString originalValue)
{
	NHDEBUG(DBG_INFO, (L"EncryptB4UValue: original [%s]\n", originalValue) );
	unsigned char iv[B4U_DecKeyLen] = {};

	CString strReturn = Encrypt_And_Base64Encode_AES_128_ECB(originalValue, (unsigned char *)B4U_DecKey, 3, iv) ;

	NHDEBUG(DBG_INFO, (L"EncryptB4UValue: return [%s]\n", strReturn) );
	return strReturn;
}

/**
 * Deserialize the json returned by the opportunity registration route
 */
BOOL CB4UService::DeserializeB4USession(CString response, B4USession& session)
{ 
	BOOL	success = false;
	JSON_Value *rootValue;
	JSON_Object *auth;

	int len = response.GetLength() + 1;
	char * json = new char[len]();
	WideToMulti(json, response, len);

	rootValue = json_parse_string(json);
	auth = json_value_get_object(rootValue);

	if (session.Type == B4U_SESS_TYPE_PRESTAGE)
	{
		session.SurchargeAmount = json_object_get_string(auth, "amount");
	}
	else if (session.Type == B4U_SESS_TYPE_STAGE)
	{
		session.QrPath = DecryptB4UValue(CString(json_object_get_string(auth, "qrPath")), &success);
		if(!success) return false;
		
		session.QrData = DecryptB4UValue(CString(json_object_get_string(auth, "qrdata")), &success);
		if(!success) return false;

		session.TransactionId = DecryptB4UValue(CString(json_object_get_string(auth, "transactionId")), &success);
		if(!success) return false;

		session.EncTransactionId = CString(json_object_get_string(auth, "transactionId"));

		session.BitcoinAddr = DecryptB4UValue(CString(json_object_get_string(auth, "bitcoinAddress")), &success);
		if(!success) return false;

		double nBtcAmount = json_object_dotget_number(auth, "bitcoinAmount");
		session.BitcoinAmountDisplay.Format(L"%f", nBtcAmount);
		if (session.BitcoinAmountDisplay.GetLength() > 16) // trims over-sized decimal for receipt/journal
		{
			CString sBtcAmount = session.BitcoinAmountDisplay.Left(16);
			session.BitcoinAmountDisplay.Format(L"%s", sBtcAmount);
		}
	}

	return true;
}

/**
 * Deserialize the json data returned by the txn status route
 */
BOOL CB4UService::DeserializeB4USessionStatus(CString response, CString requestedCurrency, B4USessionType sessionType, B4USessionStatus& status)
{
	JSON_Value *rootValue;
	JSON_Object *auth;

	int len = response.GetLength() + 1;
	char * json = new char[len]();
	WideToMulti(json, response, len);

	rootValue = json_parse_string(json);
	auth = json_value_get_object(rootValue);

	if (sessionType == B4U_SESS_TYPE_STAGE)
	{
		status.TransactionId	= CString(json_object_get_string(auth, "transaction_id"));
		status.Status			= CString(json_object_get_string(auth, "status"));
		status.Confirmation		= CString(json_object_get_string(auth, "confirmation"));
		status.SatoshiValue		= CString(json_object_get_string(auth, "satoshi_value"));
		status.EuroValue		= CString(json_object_get_string(auth, "euro_value"));
		status.Usdvalue			= CString(json_object_get_string(auth, "usd_value"));
		status.TransactionHash	= CString(json_object_get_string(auth, "transaction_hash"));
		status.EquivalentAmount = CString(json_object_get_string(auth, "equivalent_Amount"));
		status.CurrencyFromServer = CString(json_object_get_string(auth, "requested_Currency"));

		if ( (status.TransactionId.GetLength() <= 0) || (status.Status.GetLength() <= 0) ||
			(requestedCurrency.MakeUpper() != status.CurrencyFromServer.MakeUpper()) )
		{
			return false;
		}
	}
	else if (sessionType == B4U_SESS_TYPE_PRESTAGE)
	{
		status.FeeFromServer		= CString(json_object_get_string(auth, "amount"));
		status.CurrencyFromServer	= CString(json_object_get_string(auth, "currency"));
	}
	
	CString errorString	= CString(json_object_get_string(auth, "error"));
	if (errorString != JSON_NULL)
	{
		CString errorCode			= CString(json_object_get_string(auth, "errorCode"));
		status.Error.Code			= Asc2Int(errorCode, 4);
		status.Error.Description	= CString(json_object_get_string(auth, "errorDescription"));
	}

	return true;
}

CString CB4UService::SerializeSessionRequest(B4UIni ini, const B4USession& request)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);

	CString EncryptedMerchantID = EncryptB4UValue(ini.MerchantId);
	CString	EncryptedTransactionID = EncryptB4UValue(request.TransactionId);
	CString sAmount;
	sAmount.Format(L"%d", request.Amount);
	
	json_set_string_b4u(EncryptedMerchantID, root_object, "merchant_id");
	json_set_string_b4u(EncryptedTransactionID, root_object, "transactionId");
	json_set_string_b4u(request.Currency, root_object, "currency");
	json_set_string_b4u(sAmount, root_object, "amount");

	char *serialized = json_serialize_to_string(root_value);
	CString data(serialized);
	
	json_free_serialized_string(serialized);
	json_value_free(root_value);

	return data;
}

// [#RWC6-77] Bitload4U Updated API and Enhancement
CString CB4UService::FinalizeSessionStatus(const B4USession& session)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);

	CString EncryptedTerminalID = EncryptB4UValue(session.TerminalId);

	json_set_string_b4u(session.EncTransactionId, root_object, "transaction_id");
	json_set_string_b4u(session.SequenceNumber, root_object, "sequence");
	json_set_string_b4u(EncryptedTerminalID, root_object, "terminalNumber");
	json_set_string_b4u(session.Disbursed, root_object, "amount_disbursed");
	json_set_string_b4u(session.ReqDateTime, root_object, "transactionDateTime");
	json_set_string_b4u(session.RequestedAmount, root_object, "amount_requested");
	json_set_string_b4u(session.DispensedAmount, root_object, "amount_dispensed");
	json_set_string_b4u(session.Currency, root_object, "requested_Currency");
	json_set_string_b4u(session.EquivalentAmount, root_object, "equivalent_Amount");

	char *serialized = json_serialize_to_string(root_value);
	CString data(serialized);
	
	json_free_serialized_string(serialized);
	json_value_free(root_value);

	return data;
}
// end of [#RWC6-77]

CString CB4UService::PostData(CString route, CString data, BOOL IsCallbackNeeded, CString terminalID, B4USessionType sessionType, B4UIni ini, BOOL *success)
{
	HttpResponse response;
	char* dataBuffer = NULL;
	NHDEBUG(DBG_CALL, (L"Posting data\r\n")); 
	*success = false;

	NHDEBUG(DBG_CALL, (L"data=[%s]", data));
	// Encode the data, if present
	if (!data.IsEmpty())
	{
		int dataSize = data.GetLength();
		dataBuffer = new char[dataSize + 1]();
		WideToMulti(dataBuffer, data, dataSize);
	}

	CString header;

	// [#RWC6-115] B4U: API Changes
	if (sessionType == B4U_SESS_TYPE_REPORT)
	{
		CString EncryptedMerchantID = EncryptB4UValue(ini.MerchantId);
		header.Format(L"Content-Type: application/json\r\nAccept: application/json\r\nmerchant_id: %s\r\n", EncryptedMerchantID);
	}
	else
	{
		CString EncryptedTerminalID = EncryptB4UValue(terminalID);
		header.Format(L"Content-Type: application/json\r\nAccept: application/json\r\nterminal_id: %s\r\n", EncryptedTerminalID);
	}
	// end of [#RWC6-115]

	HttpRequest request;
	request.Url = route;
	request.Headers = header;
	request.Data = (unsigned char *)dataBuffer;
	HTTPERR result = this->httpClient->SendPostRequest(&response, request);
	
	// Delete the buffer, if necessary
	if (dataBuffer != NULL) delete [] dataBuffer;

	if (!IsCallbackNeeded)
	{
		NHERROR((L"Callback(GET) not required, returns true\r\n"));
		*success = true;
		return _T("");
	}

	if (result != HTTP_NO_ERR)
	{
		NHERROR((L"Error in web request. Error code: %d\r\n", result));
		return _T("");
	}

	// If the request failed
	if (!response.StatusCodeIsSuccessful())
	{
		NHERROR((L"Error in web request. Status code not 200\r\n"));
		return _T("");
	}

	*success = true;
	return response.ResponseData;
}

CString CB4UService::GetData(CString route, CString data, B4USessionType sessionType, CString terminalID, B4UIni ini, B4USessionStatus& status, BOOL *success)
{
	HttpResponse response;
	NHDEBUG(DBG_CALL, (L"Getting data\r\n"));
	*success = false;

	NHDEBUG(DBG_CALL, (L"data=[%s]\r\n", data));
	
	CString header;
	CString	EncryptedMerchantID = EncryptB4UValue(ini.MerchantId);
	if (sessionType == B4U_SESS_TYPE_PRESTAGE)
	{
		header.Format(L"Content-Type: application/json\r\nAccept: application/json\r\nmerchant_id: %s\r\n", EncryptedMerchantID);
	}
	else
	{
		CString	EncryptedSecretID = EncryptB4UValue(ini.SecretId);
		header.Format(L"Content-Type: application/json\r\nAccept: application/json\r\nmerchant_id: %s\r\nsecret_id: %s\r\n", EncryptedMerchantID, EncryptedSecretID);
	}

	HttpRequest request;
	request.Url = route;
	request.Headers = header;
	request.UseCache = false;
	HTTPERR result = this->httpClient->SendGetRequest(&response, request);
	if (result != HTTP_NO_ERR)
	{
		NHERROR((L"Error in web request. Error code: %d\r\n", result));
		NHERROR((L"Error in web request. StatusCode=[%s],\r\nStatusMessage=[%s],\r\nResponseData=[%s]\r\n", response.StatusCode, response.StatusMessage, response.ResponseData ));
		return _T("");
	}

	if ( (response.StatusCode == L"401") && (response.ResponseData.Find(B4U_NOT_PAID) >= 0) )
	{	
		status.Status = B4U_NOT_PAID;
		*success = true;
		return response.ResponseData;
	}

	// If the request failed
	if (!response.StatusCodeIsSuccessful())
	{
		NHERROR((L"Error in web request. Status code not 200\r\n"));
		return _T("");
	}

	if (sessionType == B4U_SESS_TYPE_PRESTAGE)
	{
		status.Status = B4U_PRE_RECEVIED;
	}
	else
	{
		status.Status = B4U_CONFIRMED;
	}

	*success = true;
	return response.ResponseData;
}

BOOL CB4UService::GenerateQRCode(CString qrData, CString filename)
{
	NHDEBUG(DBG_INFO, (L"CBitload4UDat::GenerateQRCode()\n"));

	// Delete Existing QR Image
	DeleteFile(filename);

	MakeQRCodeImageFile(qrData, filename);

	CCeFileFind finder;
	BOOL bFound = FALSE;
	bFound = finder.FindFile(filename);
	finder.Close();

	if (!bFound)
	{
		NHDEBUG(DBG_INFO, (L"CBitload4UData::GenerateQRCode QR Not Found!!!\n"));
		return FALSE;
	}

	NHDEBUG(DBG_INFO, (L"CBitload4UDat::GenerateQRCode All Good!\n"));
	return TRUE;
}

BOOL CB4UService::SaveQrDataToFile(CString base64Data, CString* filename)
{
	*filename = B4U_QR_CODE_FILE;

	// Length of decoded data is (3 * (nChars / 4)) - nPaddingChars
	long lNewBufferLen = 3 * (base64Data.GetLength() /4);
	unsigned char* buffer = new unsigned char[lNewBufferLen];

	// Convert the base64 data to a char array
	int dataLen = base64Data.GetLength();
	char* charData = new char[dataLen + 1];
	WideToMulti(charData, base64Data, dataLen);

	// Decode the base64 string
	int decodedLength = base64_decode(charData, dataLen, buffer, lNewBufferLen);

	// Open the file handle
	HANDLE hFile = CreateFile(B4U_QR_CODE_FILE, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		NHDEBUG(DBG_CALL, (L"error creating qr file\n"));
		return false;
	}

	// Write to the file
	DWORD bytesWritten;
	BOOL ok = WriteFile(hFile, buffer, decodedLength, &bytesWritten, NULL);
	if (!ok)
	{
		NHDEBUG(DBG_CALL, (L"error writing to file\n"));
	}

	delete [] charData;

	// Close everything out
	CloseHandle(hFile);

	return ok;
}

BOOL CB4UService::LoadB4UConfigIni(B4UIni& ini)
{
	CNHReadiniFile	Readini;
	PINIVALUE		pwIniValue;			

	if (!Readini.Open(B4U_INI_FILE, CNHReadiniFile::modeRead, CNHReadiniFile::modeASCII))
		return FALSE;
	
	while (NULL != (pwIniValue = Readini.ReadiniValue()))
	{
		// [HOST_INFO] section
		if (pwIniValue->Section == L"HOST_INFO")
		{
			if (pwIniValue->Key == L"HOST")
			{
				ini.Host = pwIniValue->Values[0];
				NHDEBUG(DBG_INFO, (L"ini.Host = pwIniValue->Values[0] = (%s)\n", pwIniValue->Values[0]));
			}
			else if (pwIniValue->Key == L"MERCHANT_ID")
			{
				ini.MerchantId = pwIniValue->Values[0];
				NHDEBUG(DBG_INFO, (L"ini.MerchantId = pwIniValue->Values[0] = (%s)\n", pwIniValue->Values[0]));
			}
			else if (pwIniValue->Key == L"POST_ROUTE")
			{
				ini.PostRoute = pwIniValue->Values[0]; 
				NHDEBUG(DBG_INFO, (L"ini.PostRoute = pwIniValue->Values[0] = (%s)\n", pwIniValue->Values[0]));
			}
			else if (pwIniValue->Key == L"GET_ROUTE")
			{
				ini.GetRoute = pwIniValue->Values[0];
				NHDEBUG(DBG_INFO, (L"ini.GetRoute = pwIniValue->Values[0] = (%s)\n", pwIniValue->Values[0]));
			}
			else if (pwIniValue->Key == L"SECRET_ID")
			{
				ini.SecretId = pwIniValue->Values[0];
				NHDEBUG(DBG_INFO, (L"ini.SecretId = pwIniValue->Values[0] = (%s)\n", pwIniValue->Values[0]));
			}
			// [#RWC6-77] Bitload4U Updated API and Enhancement
			else if (pwIniValue->Key == L"POST_ROUTE_REPORT")
			{
				ini.PostRouteReport = pwIniValue->Values[0]; 
				NHDEBUG(DBG_INFO, (L"ini.PostRouteReport = pwIniValue->Values[0] = (%s)\n", pwIniValue->Values[0]));
			}
			else if (pwIniValue->Key == L"GET_ROUTE_PRESTAGE")
			{
				ini.GetRoutePrestage = pwIniValue->Values[0]; 
				NHDEBUG(DBG_INFO, (L"ini.GetRoutePrestage = pwIniValue->Values[0] = (%s)\n", pwIniValue->Values[0]));
			}
			// end of [#RWC6-77]
		}
		// [SYSTEM] section
		else if (pwIniValue->Section == L"TRANSACTION_INFO")
		{
			if (pwIniValue->Key == L"CURRENCY")
			{
				ini.Currency = pwIniValue->Values[0];
				NHDEBUG(DBG_INFO, (L"session.Currency = pwIniValue->Values[0] = (%s)\n", pwIniValue->Values[0]));
			}
			else if (pwIniValue->Key == L"FEE")
			{
				ini.FeeDisplay = pwIniValue->Values[0];
				NHDEBUG(DBG_INFO, (L"session.FeeDisplay = pwIniValue->Values[0] = (%s)\n", pwIniValue->Values[0]));
			}
			else if (pwIniValue->Key == L"MINIMUM")
			{
				ini.MinimumDisplay = pwIniValue->Values[0];
				NHDEBUG(DBG_INFO, (L"session.MinimumDisplay = pwIniValue->Values[0] = (%s)\n", pwIniValue->Values[0]));
			}
			else if (pwIniValue->Key == L"QRCODE_TIMEOUT")
			{
				ini.QRCodeTimeout = pwIniValue->Values[0];
				NHDEBUG(DBG_INFO, (L"session.QRCodeTimeout = pwIniValue->Values[0] = (%s)\n", pwIniValue->Values[0]));
			}
		}
	}

	Readini.Close();
	NHDEBUG(DBG_CALL, (L"Readini.Close() done, return=(%d)\n", TRUE));
	return TRUE;
}

CString CB4UService::GetB4UDaytotalJournalData(B4UJournal& journal)
{
	NHDEBUG(DBG_INFO, (L"CB4UService::GetB4UDaytotalJournalData()\n") );

	ReadB4UDispenseHistory(journal);

	CString strEJNLData = L"";

	strEJNLData += journal.DayTotalDateTime + _T("^");
	strEJNLData += journal.DayTotalDispCount + _T("^");
	strEJNLData += journal.DayTotalDispAmt;

	NHDEBUG(DBG_INFO, (L"   Journal Data=[%s])\n", strEJNLData) );
	return strEJNLData;
}

BOOL CB4UService::ReadB4UDispenseHistory(B4UJournal& journal)
{
	NHDEBUG(DBG_INFO, (L"CB4UService::ReadB4UDispenseHistory()\n") );

	CString strPath;
	CFile cf;

	strPath.Format(L"%s", B4U_DISPENSERESULT_FILE);
	if( cf.Open( strPath, CFile::modeRead) == FALSE )
	{
		NHDEBUG(DBG_INFO, (L"[FAIL TO OPEN(%s)]\n", strPath));
		CreateB4UDispenseResult(journal);
		return FALSE;
	}

	int nFileLen = (int)cf.GetLength();
	unsigned char* pBuffer1 = new unsigned char[nFileLen + 2];		// Allocate buffer for binary file data
	if( pBuffer1 == NULL )
	{
		NHDEBUG(DBG_INFO, (L"[FAIL TO NEW BUFFER(%d)]\n", nFileLen + 2));
		cf.Close();
		return FALSE;
	}
	nFileLen = cf.Read( pBuffer1, nFileLen );
	cf.Close();

	CString strDispData = (LPCSTR)pBuffer1;
	delete [] pBuffer1;
	NHDEBUG(DBG_INFO, (L"   Dispense Data =[%s]\n", strDispData));

	CStringArray	arrTemp;
	SplitString(strDispData, L",", arrTemp);	
	
	// Daytotal Info  (3) :		Date and Time, Dispense Count, Dispense Amount
	if(arrTemp.GetSize() < 3)
	{
		NHDEBUG(DBG_INFO, (L"[Dispense Information is wrong or not set]\n"));
		SaveB4UDispenseResult(TRUE, journal);
		return FALSE;
	}

	BOOL bReadSuccessful = TRUE;

	// DayTotal Date and Time
	CString strTemp = arrTemp.GetAt(0);
	NHDEBUG(DBG_INFO, (L"   m_strDayTotalDateTime = [%s]\n", strTemp));
	if(strTemp.GetLength()==14)		journal.DayTotalDateTime = strTemp;
	else							bReadSuccessful = FALSE;

	// DayTotal Dispense Count
	if(bReadSuccessful)
	{
		strTemp = arrTemp.GetAt(1);
		NHDEBUG(DBG_INFO, (L"   m_strDayTotalDispCount = [%s]\n", strTemp));
		if(Asc2Int(strTemp)>=0)			journal.DayTotalDispCount = strTemp;
		else							bReadSuccessful = FALSE;
	}

	// DayTotal Total Dispense Amount
	if(bReadSuccessful)
	{
		strTemp = arrTemp.GetAt(2);
		NHDEBUG(DBG_INFO, (L"   m_strDayTotalDispAmt = [%s]\n", strTemp));
		if(Asc2Int(strTemp)>=0)			journal.DayTotalDispAmt = strTemp;
		else							bReadSuccessful = FALSE;
	}

	if(!bReadSuccessful)
	{
		NHDEBUG(DBG_INFO, (L"[ERROR on Dispense Result Data]\n"));
		SaveB4UDispenseResult(TRUE, journal);
		return FALSE;
	}

	NHDEBUG(DBG_INFO, (L"CB4UService::ReadB4UDispenseHistory () RETURN TRUE \n") );
	return TRUE;
}

BOOL CB4UService::SaveB4UDispenseResult(BOOL bReset, B4UJournal& journal)
{
	NHDEBUG(DBG_INFO, (L"CB4UService::SaveB4UDispenseResult(%d) \n", bReset));

	CString strPath;
	strPath.Format(L"%s", B4U_DISPENSERESULT_FILE);

	CFile cf;
	if( cf.Open( strPath, CFile::modeCreate|CFile::modeWrite) == FALSE )
	{
		NHDEBUG(DBG_INFO, (L"[FAIL TO OPEN(%s)]\n", strPath));
		return FALSE;
	}

	if(bReset)
	{
		journal.DayTotalDateTime.Format(L"%s%s",GetDate(), GetTime());	// DayTotal fix
		journal.DayTotalDispCount = L"0";								// DayTotal fix
		journal.DayTotalDispAmt = L"0";									// DayTotal fix
	}

	CString strTotalDispenseData = L"";
	strTotalDispenseData += journal.DayTotalDateTime + _T(",");			// DayTotal fix
	strTotalDispenseData += journal.DayTotalDispCount + _T(",");		// DayTotal fix
	strTotalDispenseData += journal.DayTotalDispAmt;					// DayTotal fix

	NHDEBUG(DBG_INFO, (L"   DispenseResult = [%s]\n",strTotalDispenseData));

	UINT nSize = strTotalDispenseData.GetLength();

	char *pBuffer = new char[nSize+1];
	memset(pBuffer, 0, nSize+1);
	WideToMulti( pBuffer, strTotalDispenseData, nSize);

	cf.Write(pBuffer, nSize );
	delete [] pBuffer;

	cf.Close();
	return TRUE;
}

void CB4UService::CreateB4UDispenseResult(B4UJournal& journal)
{
	NHDEBUG(DBG_INFO, (L"CB4UService::CreateB4UDispenseResult() \n"));

	CString strPath;
	strPath.Format(L"%s", B4U_DISPENSERESULT_FILE);

	CFile cf;
	cf.Open(strPath, CFile::modeCreate|CFile:: modeReadWrite);

	journal.DayTotalDateTime.Format(L"%s%s",GetDate(), GetTime());	// DayTotal fix
	journal.DayTotalDispCount = L"0";								// DayTotal fix
	journal.DayTotalDispAmt = L"0";									// DayTotal fix

	CString strTotalDispenseData = L"";
	strTotalDispenseData += journal.DayTotalDateTime + _T(",");		// DayTotal fix
	strTotalDispenseData += journal.DayTotalDispCount + _T(",");	// DayTotal fix
	strTotalDispenseData += journal.DayTotalDispAmt;				// DayTotal fix

	NHDEBUG(DBG_INFO, (L"   DispenseResult = [%s]\n",strTotalDispenseData));

	UINT nSize = strTotalDispenseData.GetLength();

	char *pBuffer = new char[nSize+1];
	memset(pBuffer, 0, nSize+1);
	WideToMulti( pBuffer, strTotalDispenseData, nSize);

	cf.Write(pBuffer, nSize );
	delete [] pBuffer;

	cf.Close();
}

void CB4UService::RecordB4UDispenseResult(long nAmount, B4UJournal& journal)
{
	NHDEBUG(DBG_INFO, (L"CB4UService::RecodeB4UDispenseResult (Amt=%ld)\n", nAmount) );

	CString sTranDispensed = L"";
	sTranDispensed.Format(L"%d", (int) (0.01*nAmount + 0.001) );

	// Update Day Total ... Date Time
	if( journal.DayTotalDateTime.GetLength() != 14)
		journal.DayTotalDateTime.Format(L"%s%s", GetDate(), GetTime() );
	NHDEBUG(DBG_INFO, (L"  m_strDayTotalDateTime = [%s]\n", journal.DayTotalDateTime) );

	// Update Day Total ... Dispense Count
	long nTemp = Asc2Int(journal.DayTotalDispCount) + 1;
	journal.DayTotalDispCount.Format(L"%ld", nTemp);
	NHDEBUG(DBG_INFO, (L"  m_strDayTotalDispCount = [%s]\n", journal.DayTotalDispCount) );

	// Update Day Total ... Dispense Amount
	nTemp = Asc2Int(journal.DayTotalDispAmt) + Asc2Int(sTranDispensed);
	journal.DayTotalDispAmt.Format(L"%ld", nTemp);
	NHDEBUG(DBG_INFO, (L"  m_strDayTotalDispAmt = [%s]\n", journal.DayTotalDispAmt) );

	SaveB4UDispenseResult(FALSE, journal);
}

void CB4UService::ResetTotal()
{
	NHDEBUG(DBG_INFO, (L"CB4UService::ResetTotal()\n") );
	
	CString strPath, strTemp;
	strPath.Format(L"%s", B4U_DISPENSERESULT_FILE);

	CFile cf;
	cf.Open(strPath, CFile::modeCreate|CFile:: modeReadWrite);

	strTemp.Format(L"%s%s,0,0",GetDate(), GetTime());
	NHDEBUG(DBG_INFO, (L"   DispenseResult = [%s]\n",strTemp));

	UINT nSize = strTemp.GetLength();

	char *pBuffer = new char[nSize+1];
	memset(pBuffer, 0, nSize+1);
	WideToMulti( pBuffer, strTemp, nSize);

	cf.Write(pBuffer, nSize );
	delete [] pBuffer;

	cf.Close();
}

BOOL CB4UService::WriteB4UJournalEntry(CJnlMgr *mgr, B4USession& session, B4USessionStatus& status, CString sKindCode)
{
	CString sTemp, sJournalData;

	// 01. TERMINAL NUMBER
	sJournalData.AppendFormat(L"%s", MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID).TrimLeft());

	// 02. SEQUENCE NUMBER
	sJournalData.AppendFormat(L"^%4.4s", MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_SEQUENCENO));

	// 03. TRANSACTION TYPE	
	sJournalData.AppendFormat(L"^%2.2s", TRANTYPE_B4U);

	// 04~07
	sJournalData.AppendFormat(L"^^^^");

	// 08. TRANSACTION HOST DATE
	sJournalData.AppendFormat(L"^%8.8s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTDATE));

	// 09. TRANSACTION HOST TIME
	sJournalData.AppendFormat(L"^%6.6s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTTIME));

	// 10~15
	sJournalData.AppendFormat(L"^^^^");

	// 14. BTC CHARGED
	// 15. TRANSACTION HASH
	if (sKindCode == B4U_TXN)
	{
		sJournalData.AppendFormat(L"^%s", session.BitcoinAmountDisplay);
		sJournalData.AppendFormat(L"^%s", status.TransactionHash);
	}
	else
	{
		sJournalData.AppendFormat(L"^^");
	}

	// 16. TRANSACTION REQUEST AMOUNT
	sJournalData.AppendFormat(L"^%8.8s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT));

	// 17. TRANSACTION DISPENSE AMOUNT
	sJournalData.AppendFormat(L"^%8.8s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT));

	// 18
	sJournalData.AppendFormat(L"^");

	// 20. TRANSACTION RESULT
	if (sKindCode == B4U_TXN)
		sJournalData.AppendFormat(L"^ TRUE");
	else 
		sJournalData.AppendFormat(L"^FALSE");

	// 21. TRANSACTION ERROR CODE
	sJournalData.AppendFormat(L"^");

	// 22
	sJournalData.AppendFormat(L"^");

	NHDEBUG(DBG_INFO, (L"Writing Journal info: %s\r\n", sJournalData));

	return mgr->Save(sKindCode, sJournalData);
}	
