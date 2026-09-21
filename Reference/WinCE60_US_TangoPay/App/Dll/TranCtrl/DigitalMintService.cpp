#include "stdafx.h"
#include ".\Tran\DigitalMintService.h"
#include ".\Common\NHDbgApi.h"
#include ".\Common\Crypto.h"

#ifndef UNDER_CE
	#include <math.h>
#endif

#define CLIENT_TYPE_HEADER			L"X-Client-Type: NHATM"
#define CLIENT_VERION_HEADER		L"X-Client-Version: v0.1"

#define TIMESTAMP_HEADER			L"DigitalMint-Timestamp"
#define DEVICEID_HEADER				L"DigitalMint-DeviceID"
#define AUTH_HEADER					L"DigitalMint-Auth"
#define APIKEY_HEADER				L"DigitalMint-Key"
#define SIGNATURE_HEADER			L"DigitalMint-Signature"
#define PLATFORM_HEADER				L"DigitalMint-Platform"

#define AUTHTYPE_HMACV1				L"HMACv1"

#define VERSION_ROUTE				L"/v2"
#define GREET_ROUTE					L"/v2/greet"
#define GREET_ENROLL_ROUTE			L"/v2/greet/enroll"
#define PIN_RECONFIRM_ROUTE			L"/v2/greet/phone/reconfirm"
#define PIN_RESET_ROUTE				L"/v2/greet/pin/reset"
#define ACCOUNT_LIMITS_ROUTE		L"/v2/accounts/%s/limits"
#define QUOTES_ROUTE				L"/v2/quotes/%s"
#define QUOTES_ACCOUNT_ROUTE		L"/v2/quotes/%s?account=%s"
#define PRODUCTS_ROUTE				L"/v2/platforms/%s/products"
#define INTIATE_TRANSACTION_ROUTE	L"/v2/transactions"
#define TRANSACTION_ROUTE			L"/v2/transactions/%s"
#define KIOSK_STATE_ROUTE			L"/v2/kiosk/state"
#define KIOSK_EVENT_ROUTE			L"/v2/kiosk/event"
#define ATM_REGISTRATION_ROUTE		L"/v2/register/atm/%s"
#define SIDECAR_REGISTRATION_ROUTE	L"/v2/register/device/%s"
#define TOS_ROUTE					L"/v2/accounts/%s/terms-of-service"

CDigitalMintService::CDigitalMintService(DMConfiguration config, CDigitalMintConfigurationManager configManager, HTTP *http)
{
	isMyHttp = false;
	this->http = http;
	this->config = config;
	this->configManager = &configManager;
	this->transactionMode = DMTXNMODE_UNREGISTERED;
}

bool CDigitalMintService::Greet(const DMGreeting &greeting, DMGreetingResponse &response)
{
	NHDBG((L"Performing customer greet API call\r\n"));

	CString requestData = this->SerializeGreet(greeting);

	bool success;
	HttpResponse httpResponse = this->MakeRequest(GREET_ROUTE, requestData, HTTP_PUT, true, &success);

	if (!success)
	{
		NHERROR((L"Failed to make greet request\r\n"));
		return false;
	}

	if (!this->DeserializeGreetResponse(httpResponse.ResponseData, response))
	{
		NHERROR((L"Failed to deserialize greet result\r\n"));
		return false;
	}

	return true;
}

bool CDigitalMintService::GreetCode(const DMGreeting &greeting, DMGreetingResponse &response, bool put /*= false*/)
{
	NHDBG((L"Performing customer greet code API call\r\n"));

	CString requestData = this->SerializeGreet(greeting);

	bool success;
	HttpResponse httpResponse;

	if (put)
	{
		httpResponse = this->MakeRequest(GREET_ROUTE, requestData, HTTP_PUT, true, &success);
	}
	else
	{
		httpResponse = this->MakeRequest(GREET_ROUTE, requestData, HTTP_POST, true, &success);
	}

	if (!success)
	{
		NHERROR((L"Failed to make greet code request\r\n"));
		return false;
	}

	if (!this->DeserializeGreetResponse(httpResponse.ResponseData, response))
	{
		NHERROR((L"Failed to deserialize greet code result\r\n"));
		return false;
	}

	return true;
}

bool CDigitalMintService::GreetReconfirm(const DMGreeting &greeting, DMGreetingResponse &response)
{
	NHDBG((L"Performing customer greet reconfirm API call\r\n"));

	CString requestData = this->SerializeGreet(greeting);

	bool success;
	HttpResponse httpResponse = this->MakeRequest(PIN_RECONFIRM_ROUTE, requestData, HTTP_POST, true, &success);

	if (!success)
	{
		NHERROR((L"Failed to make greet reconfirm request\r\n"));
		return false;
	}

	if (!this->DeserializeGreetResponse(httpResponse.ResponseData, response))
	{
		NHERROR((L"Failed to deserialize greet reconfirm result\r\n"));
		return false;
	}

	return true;
}

bool CDigitalMintService::GreetReset(const DMGreeting &greeting, DMGreetingResponse &response)
{
	NHDBG((L"Performing customer greet reset API call\r\n"));

	CString requestData = this->SerializeGreet(greeting);

	bool success;
	HttpResponse httpResponse = this->MakeRequest(PIN_RESET_ROUTE, requestData, HTTP_POST, true, &success);

	if (!success)
	{
		NHERROR((L"Failed to make greet reset request\r\n"));
		return false;
	}

	if (!this->DeserializeGreetResponse(httpResponse.ResponseData, response))
	{
		NHERROR((L"Failed to deserialize greet reset result\r\n"));
		return false;
	}

	return true;
}

bool CDigitalMintService::GreetEnroll(const DMEnrollmentRequest &enrollment, DMEnrollmentResponse &response)
{
	NHDBG((L"Performing customer greet enroll API call\r\n"));

	CString requestData = this->SerializeEnrollment(enrollment);

	bool success;
	HttpResponse httpResponse = this->MakeRequest(GREET_ENROLL_ROUTE, requestData, HTTP_PUT, true, &success);

	if (!success)
	{
		NHERROR((L"Failed to make greet request\r\n"));
		return false;
	}

	if (!this->DeserializeEnrollmentResponse(httpResponse.ResponseData, response))
	{
		NHERROR((L"Failed to deserialize greet enroll result\r\n"));
		return false;
	}

	return true;
}

bool CDigitalMintService::GetAccountLimits(CString accountHash, DMAccountLimitsResponse &response)
{
	NHDBG((L"Performing account limits API call\r\n"));

	bool success;

	CString route;
	route.Format(ACCOUNT_LIMITS_ROUTE, accountHash);

	HttpResponse httpResponse = this->MakeRequest(route, L"", HTTP_GET, true, &success);

	if (!success)
	{
		NHERROR((L"Failed to make account limits request\r\n"));
		return false;
	}

	if (!this->DeserializeAccountLimitsResponse(httpResponse.ResponseData, response))
	{
		NHERROR((L"Failed to deserialize account limits result\r\n"));
		return false;
	}

	return true;
}

bool CDigitalMintService::GetProducts(DMProductsResponse &response)
{
	NHDBG((L"Performing get products API call\r\n"));

	bool success;

	CString route;
	route.Format(PRODUCTS_ROUTE, configManager->PlatformHash(config));

	HttpResponse httpResponse = this->MakeRequest(route, L"", HTTP_GET, true, &success);

	if (!success)
	{
		NHERROR((L"Failed to make products request\r\n"));
		return false;
	}

	if (!this->DeserializeProductsResponse(httpResponse.ResponseData, response))
	{
		NHERROR((L"Failed to deserialize products result\r\n"));
		return false;
	}

	return true;
}

bool CDigitalMintService::InitiateTransaction(const DMInitiateTransactionRequest &request, DMTransactionResponse &response)
{
	NHDBG((L"Performing transaction initiation API call\r\n"));

	CString requestData = this->SerializeIntiateTransactionRequest(request);

	bool success;
	HttpResponse httpResponse = this->MakeRequest(INTIATE_TRANSACTION_ROUTE, requestData, HTTP_PUT, true, &success);

	if (!success)
	{
		NHERROR((L"Failed to make initiate transaction request\r\n"));
		return false;
	}

	if (!this->DeserializeTransactionStateResponse(httpResponse.ResponseData, response))
	{
		NHERROR((L"Failed to deserialize transaction state result\r\n"));
		return false;
	}

	return true;
}

bool CDigitalMintService::SetWalletAddress(const CString transactionHash, const DMScanWalletRequest &request, DMTransactionResponse &response)
{
	NHDBG((L"Performing set wallet address API call\r\n"));

	bool success;

	CString route;
	route.Format(TRANSACTION_ROUTE, transactionHash);

	CString requestData = this->SerializeScanWalletRequest(request);
	HttpResponse httpResponse = this->MakeRequest(route, requestData, HTTP_POST, true, &success);

	if (!success)
	{
		NHERROR((L"Failed to make scan wallet address request\r\n"));
		return false;
	}

	if (!this->DeserializeTransactionStateResponse(httpResponse.ResponseData, response))
	{
		NHERROR((L"Failed to deserialize transaction state result\r\n"));
		return false;
	}

	return true;
}

bool CDigitalMintService::GetQuote(const CString selectedCoin, DMQuoteResponse &response)
{
	return GetQuote(selectedCoin, L"", response);
}

bool CDigitalMintService::GetQuote(const CString selectedCoin, const CString accountId, DMQuoteResponse &response)
{
	NHDBG((L"Performing quotes API call\r\n"));

	bool success;

	CString route;

	if (accountId == L"")
	{
		route.Format(QUOTES_ROUTE, selectedCoin);
	}
	else
	{
		route.Format(QUOTES_ACCOUNT_ROUTE, selectedCoin, accountId);
	}

	HttpResponse httpResponse = this->MakeRequest(route, L"", HTTP_GET, true, &success);

	if (!success)
	{
		NHERROR((L"Failed to make quotes request\r\n"));
		return false;
	}

	if (!this->DeserializeQuoteResponse(httpResponse.ResponseData, response))
	{
		NHERROR((L"Failed to deserialize quotes result\r\n"));
		return false;
	}

	return true;
}

bool CDigitalMintService::SendTOS(const CString accountId)
{
	NHDBG((L"Performing TOS API call\r\n"));

	bool success;

	CString route;
	route.Format(TOS_ROUTE, accountId);

	HttpResponse httpResponse = this->MakeRequest(route, L"", HTTP_GET, true, &success);

	if (!success)
	{
		NHERROR((L"Failed to make TOS request\r\n"));
		return false;
	}

	return true;
}

bool CDigitalMintService::CollectPayment(const CString transactionHash, const DMCollectPaymentRequest &request, DMTransactionResponse &response)
{
	NHDBG((L"Performing collect payment API call\r\n"));

	bool success;

	CString route;
	route.Format(TRANSACTION_ROUTE, transactionHash);

	CString requestData = this->SerializeCollectPaymentRequest(request);
	HttpResponse httpResponse = this->MakeRequest(route, requestData, HTTP_POST, true, &success);

	if (!success)
	{
		NHERROR((L"Failed to make collect payment request\r\n"));
		return false;
	}

	if (!this->DeserializeTransactionStateResponse(httpResponse.ResponseData, response))
	{
		NHERROR((L"Failed to deserialize transaction state result\r\n"));
		return false;
	}

	return true;
}

bool CDigitalMintService::CompleteTransaction(const CString transactionHash, const DMCompleteTransactionRequest &request, DMTransactionResponse &response)
{
	NHDBG((L"Performing complete transaction API call\r\n"));

	bool success;

	CString route;
	route.Format(TRANSACTION_ROUTE, transactionHash);

	CString requestData = this->SerializeCompleteTransactionRequest(request);
	HttpResponse httpResponse = this->MakeRequest(route, requestData, HTTP_POST, true, &success);

	if (!success)
	{
		NHERROR((L"Failed to make complete transaction request\r\n"));
		return false;
	}

	if (!this->DeserializeTransactionStateResponse(httpResponse.ResponseData, response))
	{
		NHERROR((L"Failed to deserialize transaction state result\r\n"));
		return false;
	}

	return true;
}

bool CDigitalMintService::CancelTransaction(const CString transactionHash, const DMCancelTransactionRequest &request, DMTransactionResponse &response)
{
	NHDBG((L"Performing cancel transaction API call\r\n"));

	bool success;

	CString route;
	route.Format(TRANSACTION_ROUTE, transactionHash);

	CString requestData = this->SerializeCancelTransactionRequest(request);
	HttpResponse httpResponse = this->MakeRequest(route, requestData, HTTP_POST, true, &success);

	if (!success)
	{
		NHERROR((L"Failed to make cancel transaction request\r\n"));
		return false;
	}

	if (!this->DeserializeTransactionStateResponse(httpResponse.ResponseData, response))
	{
		NHERROR((L"Failed to deserialize transaction state result\r\n"));
		return false;
	}

	return true;
}

bool CDigitalMintService::ReportKioskState(const DMKioskStateRequest &request, DMTRANSACTIONMODE txnMode)
{
	NHDBG((L"Performing report kiosk state map API call\r\n"));

	CString requestData = this->SerializeKioskStateRequest(request);

	CString mode = txnMode == DMTXNMODE_ATM ? L"ATM" : L"Sidecar";
	NHDBG((L"%s State Request: %s\r\n", mode, requestData));

	bool success;
	HttpResponse httpResponse = this->MakeRequest(KIOSK_STATE_ROUTE, requestData, HTTP_POST, true, txnMode, &success);

	if (!success)
	{
		NHERROR((L"Failed to make report kiosk state map request\r\n"));
		return false;
	}

	return true;
}

bool CDigitalMintService::ReportKioskEvent(const DMKioskEventRequest &request, DMTRANSACTIONMODE txnMode)
{
	NHDBG((L"Performing report kiosk event API call\r\n"));

	CString requestData = this->SerializeKioskEventRequest(request);

	CString mode = txnMode == DMTXNMODE_ATM ? L"ATM" : L"Sidecar";
	NHDBG((L"%s Event Request: %s\r\n", mode, requestData));

	bool success;
	HttpResponse httpResponse = this->MakeRequest(KIOSK_EVENT_ROUTE, requestData, HTTP_POST, true, txnMode, &success);

	if (!success)
	{
		NHERROR((L"Failed to make report kiosk event request\r\n"));
		return false;
	}

	return true;
}

bool CDigitalMintService::RegisterSidecar(CString code, DMDeviceRegistrationResponse &response)
{
	NHDBG((L"Performing customer registration API call\r\n"));

	bool success;

	CString route;
	route.Format(SIDECAR_REGISTRATION_ROUTE, code);

	HttpResponse httpResponse = this->MakeRequest(route, L"", HTTP_PUT, false, &success);

	if (!success)
	{
		NHERROR((L"Failed to make registration request\r\n"));
		return false;
	}

	if (!this->DeserializeDeviceRegistrationResponse(httpResponse.ResponseData, response))
	{
		NHERROR((L"Failed to deserialize registration result\r\n"));
		return false;
	}

	return true;
}

bool CDigitalMintService::RegisterAtm(CString code, DMDeviceRegistrationResponse &response)
{
	NHDBG((L"Performing customer registration API call\r\n"));

	bool success;

	CString route;
	route.Format(ATM_REGISTRATION_ROUTE, code);

	HttpResponse httpResponse = this->MakeRequest(route, L"", HTTP_PUT, false, &success);

	if (!success)
	{
		NHERROR((L"Failed to make registration request\r\n"));
		return false;
	}

	if (!this->DeserializeDeviceRegistrationResponse(httpResponse.ResponseData, response))
	{
		NHERROR((L"Failed to deserialize registration result\r\n"));
		return false;
	}

	return true;
}

bool CDigitalMintService::GetVersion(DMVersionResponse &response)
{
	NHDBG((L"Performing version API call\r\n"));

	bool success;
	HttpResponse httpResponse = this->MakeRequest(VERSION_ROUTE, L"", HTTP_GET, false, &success);

	if (!success)
	{
		NHERROR((L"Failed to make version request\r\n"));
		return false;
	}

	if (!this->DeserializeVersionResponse(httpResponse.ResponseData, response))
	{
		NHERROR((L"Failed to version result\r\n"));
		return false;
	}

	return true;
}

bool CDigitalMintService::WriteJournalEntry(CJnlMgr *mgr, CString sessionId, CString transactionId, CString accountId, CString selectedCoinSymbol, int transactionAmount)
{
	CString journalData;

    // 01. Terminal Number
    journalData.AppendFormat(L"%s", MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID).Trim());

	// 02. Session ID
	journalData.AppendFormat(L"^%s", sessionId);

	// 03. Transaction ID
	journalData.AppendFormat(L"^%s", transactionId);

	// 04. Account ID
	journalData.AppendFormat(L"^%s", accountId);

	// 05. API URL
	journalData.AppendFormat(L"^%s", configManager->APIUrl(config));

	// 06. Selected Coin Symbol
	journalData.AppendFormat(L"^%s", selectedCoinSymbol);

	// 07. Transaction Mode
	journalData.AppendFormat(L"^%d", transactionMode);

	// 08. Transaction Amount
	int amountInCents = transactionAmount * 100;
	journalData.AppendFormat(L"^%d", amountInCents);

	NHDEBUG(DBG_INFO, (L"Writing Journal info: %s\r\n", journalData));

	return mgr->Save(DIGITALMINT_TXN, journalData);
}

bool CDigitalMintService::WriteErrorJournalEntry(CJnlMgr *mgr, CString sessionId, CString transactionId, CString accountId, CString selectedCoinSymbol, int transactionAmount, CString errorCode)
{
	CString journalData;

    // 01. Terminal ID
    journalData.AppendFormat(L"%s", MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID).Trim());

	// 02. Session ID
	journalData.AppendFormat(L"^%s", sessionId);

	// 03. Transaction ID
	journalData.AppendFormat(L"^%s", transactionId);

	// 04. Account ID
	journalData.AppendFormat(L"^%s", accountId);

	// 05. API URL
	journalData.AppendFormat(L"^%s", configManager->APIUrl(config));

	// 06. Selected Coin Symbol
	journalData.AppendFormat(L"^%s", selectedCoinSymbol);

	// 07. Transaction Mode
	journalData.AppendFormat(L"^%d", transactionMode);

	// 08. Transaction Amount
	int amountInCents = transactionAmount * 100;
	journalData.AppendFormat(L"^%d", amountInCents);

	// 09. Error Code
	if (errorCode.IsEmpty())
	{
		journalData.Append(L"^0000000");
	}
	else
	{
		journalData.AppendFormat(L"^%7.7s", errorCode);
	}

	NHDEBUG(DBG_INFO, (L"Writing Journal info: %s\r\n", journalData));

	return mgr->Save(DIGITALMINT_TXN_ERROR, journalData);
}

void CDigitalMintService::SetTransactionMode(DMTRANSACTIONMODE transactionMode)
{
	this->transactionMode = transactionMode;
}

DMTRANSACTIONMODE CDigitalMintService::GetTransactionMode()
{
	return transactionMode;
}

bool CDigitalMintService::VerifyService()
{
	if (http == NULL)
		return false;

	CString hostname = http->GetHostname();

	if (hostname.IsEmpty())
		return false;

	return true;
}

//
// Private Functions
//

HttpResponse CDigitalMintService::MakeRequest(CString route, CString data, HTTP_METHOD method, bool authenticate, bool *success)
{
	return MakeRequest(route, data, method, authenticate, transactionMode, success);
}

HttpResponse CDigitalMintService::MakeRequest(CString route, CString data, HTTP_METHOD method, bool authenticate, DMTRANSACTIONMODE txMode, bool *success)
{
	HttpResponse response;
	NHDBG((L"Making request to %s\r\n", route));

	char *buffer = NULL;
	*success = false;

	// Encode the data, if present
	if (!data.IsEmpty())
	{
		int dataSize = data.GetLength();
		buffer = new char[dataSize + 1]();

		WideToMulti(buffer, data, dataSize);
	}

	// Create signature
	CString timestamp = GetTimestamp();

	// Add additional request headers
	CString headers(L"Content-Type: application/json\r\n");
	headers.AppendFormat(L"%s\r\n", CLIENT_TYPE_HEADER);
	headers.AppendFormat(L"%s\r\n", CLIENT_VERION_HEADER);
	headers.AppendFormat(L"%s: %s\r\n", TIMESTAMP_HEADER, timestamp);
	headers.AppendFormat(L"%s: %s\r\n", DEVICEID_HEADER, L"1234567");
	headers.AppendFormat(L"%s: %s\r\n", AUTH_HEADER, AUTHTYPE_HMACV1);

	CString signature;

	if (txMode == DMTXNMODE_SIDECAR)
	{
		signature = CreateSignature(route, timestamp, config.SidecarAPIKey, config.SidecarAPISecret);
		headers.AppendFormat(L"%s: %s\r\n", APIKEY_HEADER, config.SidecarAPIKey);
		headers.AppendFormat(L"%s: %s\r\n", PLATFORM_HEADER, config.SidecarPlatformHash);
		headers.AppendFormat(L"%s: %s\r\n", SIGNATURE_HEADER, signature);
	}
	else if (txMode == DMTXNMODE_ATM)
	{
		signature = CreateSignature(route, timestamp, config.AtmAPIKey, config.AtmAPISecret);
		headers.AppendFormat(L"%s: %s\r\n", APIKEY_HEADER, config.AtmAPIKey);
		headers.AppendFormat(L"%s: %s\r\n", PLATFORM_HEADER, config.AtmPlatformHash);
		headers.AppendFormat(L"%s: %s\r\n", SIGNATURE_HEADER, signature);
	}
	else if (authenticate)
	{
		NHERROR((L"Error in setting signature. Transaction mode: %d\r\n", transactionMode));
		return response;
	}

	NHTRACE((L"%s\r\n", data));

	HttpRequest request;
	request.Url = route;
	request.Headers = headers;
	request.Data = (unsigned char *)buffer;

	HTTPERR result = this->http->SendRequest(method, &response, request);

	// Delete the buffer, if necessary
	if (buffer != NULL)
	{
		delete[] buffer;
	}

	if (result != HTTP_NO_ERR)
	{
		NHERROR((L"Error in web request. Error code: %d\r\n", result));
		return response;
	}

	NHTRACE((L"%s\r\n", response.ResponseData));
	*success = true;

	return response;
}

CString CDigitalMintService::GetTimestamp()
{
	SYSTEMTIME time = {};
	GetSystemTime(&time);

	CString nanos;
	nanos.Format(L"%.9f", (double)time.wMilliseconds / 1000);
	nanos = nanos.Mid(2);

	CString datestring;
	datestring.Format(
		L"%d-%02d-%02dT%02d:%02d:%02d.%sZ",
		time.wYear,
		time.wMonth,
		time.wDay,
		time.wHour,
		time.wMinute,
		time.wSecond,
		nanos);

	return datestring;
}

CString CDigitalMintService::CreateSignature(CString path, CString timestamp, CString apikey, CString apiSecret)
{
	CString signatureData;
	int keylen = 0;
	unsigned char *key = alloc_base64_decoded_string(apiSecret, &keylen);

	signatureData.Format(L"%s%s%s", apikey, timestamp, path);
	CString signature = HMACSHA256(key, keylen, signatureData);

	memset(key, 0, keylen);
	delete[] key;

	return signature;
}

CString CDigitalMintService::HMACSHA256(unsigned char *key, int keylen, CString data)
{
	CString base64Output = L"";
	unsigned char *hmaceddata = NULL;
	char *base64data = NULL;
	int base64len = 0;
	CStringA input(data);
	int hmaclen = hmac_data_sha256(NULL, 0, NULL, 0, NULL);
	if (hmaclen == 0)
	{
		goto cleanup;
	}

	// HMAC data
	hmaceddata = new unsigned char[hmaclen]();
	hmac_data_sha256(key, keylen, (unsigned char *)input.GetBuffer(), data.GetLength(), hmaceddata);

	// BASE64 encode the data
	base64len = (int)(ceil(hmaclen / 3.0) * 4);
	base64data = new char[base64len + 1]();
	base64_encode((char *)hmaceddata, hmaclen, base64data, base64len);

	base64Output = base64data;

cleanup:

	if (hmaceddata != NULL)
	{
		memset(hmaceddata, 0, hmaclen);
		delete[] hmaceddata;
	}

	if (base64data != NULL)
	{
		memset(base64data, 0, base64len);
		delete[] base64data;
	}

	return base64Output;
}