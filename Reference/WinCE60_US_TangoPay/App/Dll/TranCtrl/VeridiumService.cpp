#include "stdafx.h"

#include ".\Common\CmnLib.h"
#include ".\Tran\VeridiumService.h"

#define NH_DEBUG

#include ".\Common\NHDbgApi.h"

#define	DBG_CALL		1
#define DBG_INFO		1

#define VERIDIUM_QR_CODE_FILE ATM_DATA_PATH _T("\\VeridiumQr.png")

// The format string for the JSON data posted to get the session status
#define GET_SESS_STATUS_POST_DATA_FORMAT_STRING L"{\"sessionId\":\"%s\",\"bopsSessionId\":\"%s\"}"

#define JSON_NULL L"null"

// 128 KB buffer for HTTP requests
#define HTTP_BUFFER_SIZE (128 * 1024)


CVeridiumService::CVeridiumService(void)
{
	this->httpClient = new HTTP();
}

CVeridiumService::~CVeridiumService(void)
{
	delete this->httpClient;
}

BOOL CVeridiumService::RegisterNewOpportunity(VeridiumSession& session) 
{
	NHDEBUG(DBG_INFO, (L"Registering new opportunity\r\n"));

	BOOL success;
	CString response = this->PostData(VERIDIUM_REGISTER_SESSION_ROUTE, L"", &success);
	if (!success)
	{
		NHERROR((L"failed\r\n"));
		return false;
	}

	if (!CVeridiumService::DeserializeVeridiumSession(response, session))
	{
		NHERROR((L"failed parsing\r\n"));
		return false;
	}

	return true;
}

BOOL CVeridiumService::GetSessionStatus(const GetSessionStatusRequest& request, VeridiumSessionStatus& sessionStatus)
{
	NHDEBUG(DBG_CALL, (L"Getting session status for %s\r\n", request.SessionId));

	// Get the json to POST
	CString requestJson = CVeridiumService::SerializeSessionStatusRequest(request);

	BOOL success;
	CString response = this->PostData(VERIDIUM_GET_SESSION_STATUS_ROUTE, requestJson, &success);
	if (!success)
	{
		NHERROR((L"failed\r\n"));
		return false;
	}

	if (!CVeridiumService::DeserializeVeridiumSessionStatus(response, sessionStatus))
	{
		NHERROR((L"failed parsing\r\n"));
		return false;
	}

	if (sessionStatus.HasError())
	{
		NHERROR((L"Veridium request failed with error code %d - %s", sessionStatus.Error.Code, sessionStatus.Error.Description));
	}

	return true;
}

/**
 * Deserialize the json returned by the opportunity registration route
 */
BOOL CVeridiumService::DeserializeVeridiumSession(CString response, VeridiumSession& session)
{
	session.SessionId = GetJSONNodeValue(response, L"sessionId");
	session.BopsSessionId = GetJSONNodeValue(response, L"bopsSessionId");
	session.SessionQrImage = GetJSONNodeValue(response, L"sessionQrImage");

	return true;
}

/**
 * Deserialize the json data returned by the session status route
 */
BOOL CVeridiumService::DeserializeVeridiumSessionStatus(CString response, VeridiumSessionStatus& status)
{
	status.SessionId = GetJSONNodeValue(response, L"sessionId");
	status.BopsSessionId = GetJSONNodeValue(response, L"bopsSessionId");
	status.UserName = GetJSONNodeValue(response, L"userName");
	status.Status = GetJSONNodeValue(response, L"status");
	status.AmountDisplay = GetJSONNodeValue(response, L"amount");
	status.Amount = Asc2Int(status.AmountDisplay, 4);
	
	CString expirationTime = GetJSONNodeValue(response, L"expiration");
	status.Expiration = Asc2Int(expirationTime, 4);

	CString errorString = GetJSONNodeValue(response, L"error");
	if (errorString != JSON_NULL)
	{
		// Handle the error
		CString errorCode = GetJSONNodeValue(response, L"errorCode");
		status.Error.Code = Asc2Int(errorCode, 4);
		status.Error.Description = GetJSONNodeValue(response, L"errorDescription");
		
		status.ErrorIsSet = true;
	}

	return true;
}

/**
 * Serialize the request data into JSON
 */
CString CVeridiumService::SerializeSessionStatusRequest(const GetSessionStatusRequest& request)
{
	CString data;
	data.Format(GET_SESS_STATUS_POST_DATA_FORMAT_STRING, request.SessionId, request.BopsSessionId);

	return data;
}

CString CVeridiumService::PostData(CString route, CString data, BOOL *success)
{
	HttpResponse response;
	char* dataBuffer = NULL;
	NHDEBUG(DBG_CALL, (L"Posting data\r\n"));
	*success = false;

	// Encode the data, if present
	if (!data.IsEmpty())
	{
		int dataSize = data.GetLength();
		dataBuffer = new char[dataSize + 1];
		WideToMulti(dataBuffer, data, dataSize);
	}

	HTTPERR result = this->httpClient->SendPostRequest(&response, route, VERIDIUM_HOST, (unsigned char*)dataBuffer, L"Content-Type: application/json\r\n");

	// Delete the buffer, if necessary
	if (dataBuffer != NULL) delete [] dataBuffer;

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

BOOL CVeridiumService::SaveQrDataToFile(CString base64Data, CString* filename)
{
	// Open the file handle
	HANDLE hFile = CreateFile(VERIDIUM_QR_CODE_FILE, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		NHDEBUG(DBG_CALL, (L"error creating qr file\n"));
		return false;
	}

	// Length of decoded data is (3 * (nChars / 4)) - nPaddingChars
	long lNewBufferLen = 3 * (base64Data.GetLength() /4);
	unsigned char* buffer = new unsigned char[lNewBufferLen];

	// Convert the base64 data to a char array
	int dataLen = base64Data.GetLength();
	char* charData = new char[dataLen + 1];
	WideToMulti(charData, base64Data, dataLen);

	// Decode the base64 string
	int decodedLength = base64_decode(charData, buffer, lNewBufferLen);

	delete [] charData;

	// Write to the file
	DWORD bytesWritten;
	BOOL ok = WriteFile(hFile, buffer, decodedLength, &bytesWritten, NULL);

	delete [] buffer;

	if (!ok)
	{
		NHDEBUG(DBG_CALL, (L"error writing to file\n"));
	}

	// Close everything out
	CloseHandle(hFile);

	*filename = VERIDIUM_QR_CODE_FILE;
	return ok;
}