#include "stdafx.h"

//#define NH_DEBUG		// Debug Message On/Off
#include ".\Common\NHDbgApi.h"
#include ".\Tran\TangoPayService.h"
#include ".\Common\Crypto.h"
#include <openssl/rand.h>

#ifndef UNDER_CE
#include <math.h>
#endif


// [RWC6-676] SKKim 2024.03.27 created header file for TangoPay

// define EndPoint
#define ENDPOINT_RESET_APIKEY			L"/api/apikey/reset"

#define ENDPOINT_GET_PUBLICKEY			L"/api/key/generate/public/user"
#define ENDPOINT_GET_TERMINAL_JWT		L"/api/token/generate"
#define ENDPOINT_GET_USER				L"/atm/tangopayback/api/usermanagement/user"
#define ENDPOINT_PRESTAGE_VERIFY		L"/atm/tangopayback/api/prestaging/verify"
#define ENDPOINT_STAGE_VALIDATION		L"/atm/moneygram/api/send-money/staging/validation"
#define ENDPOINT_COMMIT					L"/atm/moneygram/api/send-money/commit"


//==============================
//==  Constructor/Destructor  ==
//==============================
TangoPayService::TangoPayService(HTTP* http)
{
	_http = http;
}

//=======================
//==  Public Commands  ==
//=======================
/*
Terminal Identity API
*/

bool TangoPayService::ResetAPIKey(CString strAPIKey)
{
	NHDBG((L"ResetAPIKey API call(ApiKey:%s)\r\n", strAPIKey));
	bool success = false;

	//Prep the route
	CString route = ENDPOINT_RESET_APIKEY;

	route.AppendFormat(L"/%s", strAPIKey);

	HttpResponse httpResponse = MakeRequest(RESET_APIKEY, route, L"", L"", HTTP_POST, &success);

	success = DeserializeResetApiKeyResponse(httpResponse.ResponseData);

	return HandleDeserializationResult(success, L"ResetApiKeyRequest");
}

bool TangoPayService::GetPublicKeyRequest(CString strApiKey, TangoPayGetPublicKeyResponse& response)
{
	NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_GET_PUBLICKEY, NVP4_NONE, L"G_PUB_KEY");
	NHDBG((L"GetPublicKeyRequest API call\r\n"));
	bool success = false;

	//Prep the route
	CString route = ENDPOINT_GET_PUBLICKEY;

	CString headers;
	headers.AppendFormat(L"ApiKey:%s\r\n", strApiKey);

	//Send and receive
	HttpResponse httpResponse = MakeRequest(GET_PUBLICKEY, route, headers, L"", HTTP_POST, &success);

	// [RWC6-676] Start SKKim 2024.05.28
	response.resultinfo.strStatusCode = httpResponse.StatusCode;
	response.resultinfo.strStatusMsg = httpResponse.StatusMessage;
	// [RWC6-676] End SKKim 2024.05.28

	if (false == HandleRequestFailure(success, L"GetPublicKeyRequest"))
		return false;

	success = DeserializeGetPublicKeyResponse(httpResponse.ResponseData, response);

	return HandleDeserializationResult(success, L"GetPublicKeyRequest");
}


CString TangoPayService::HMACSHA256(unsigned char *key, int keylen, CString data)
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


bool TangoPayService::GetTerminaljwtRequest(TangoPayGetTerminaljwtRequest request, TangoPayGetTerminaljwtResponse& response)
{
	NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_GET_JWT, L"", L"G_JWT");
	NHDBG((L"GetTerminaljwtRequest API call\r\n"));

	bool success = false;

	//Prep the route
	CString route = ENDPOINT_GET_TERMINAL_JWT;

	CString strJsonBody = SerializeGetTerminalJWTRequest(request);

	//Send and receive
	HttpResponse httpResponse = MakeRequest(GET_TERMINAL_JWT, route, L"", strJsonBody, HTTP_POST, &success);

	// [RWC6-676] Start SKKim 2024.05.28
	response.resultinfo.strStatusCode = httpResponse.StatusCode;
	response.resultinfo.strStatusMsg = httpResponse.StatusMessage;
	// [RWC6-676] End SKKim 2024.05.28

	//Parse results
	if(false == HandleRequestFailure(success, L"G_JWT"))
		return false;

	success = DeserializeGetTerminalJWTResponse(httpResponse.ResponseData, response);

	return HandleDeserializationResult(success, L"G_JWT");
}


/*
Terminal Transaction API functions
*/
bool TangoPayService::GetUserRequest(TangoPayGetUserRequest request, TangoPayGetUserResponse& response)
{
	NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_GET_USER, L"", L"G_USER");
	NHDBG((L"GetUserRequest API call\r\n"));

	bool success = false;

	CString route, countrycodeval;
	route.AppendFormat(ENDPOINT_GET_USER);
	route.AppendFormat(L"/%s", request.phoneNumber);
	countrycodeval.Format(L"?countryCode=%s", request.countryCode);
	countrycodeval.Replace(L"+", L"%%2B");		// +값이 있는 경우 값이 %2B로 code 변경처리해야함
	route.AppendFormat(countrycodeval);

	CString headers;
	//headers.Format(L"Authorization: Bearer %s\r\n", request.eid);
	headers = L"Authorization: Bearer ";
	headers += request.eid;
	headers += L"\r\n";

	//Send and receive
	HttpResponse httpResponse = MakeRequest(GET_USER, route, headers, L"", HTTP_GET, &success);

	// [RWC6-676] Start SKKim 2024.05.28
	response.resultinfo.strStatusCode = httpResponse.StatusCode;
	response.resultinfo.strStatusMsg = httpResponse.StatusMessage;
	// [RWC6-676] End SKKim 2024.05.28

	//Parse results
	if(false == HandleRequestFailure(success, L"G_USER"))
		return false;

	success = DeserializeGetUserResponse(httpResponse.ResponseData, response);

	return HandleDeserializationResult(success, L"G_USER");
}


bool TangoPayService::PrestagingVerifyRequest(TangoPayPreStagingVerifyRequest request, TangoPayPreStagingVerifyResponse& response)
{
	NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_PRESTAGE_VERIFY, L"", L"PRE_STAGE");
	NHDBG((L"PrestagingVerifyRequest API call\r\n"));

	bool success = false;

	CString route;
	route = ENDPOINT_PRESTAGE_VERIFY;

	CString headers;
	//headers.Format(L"Authorization: Bearer %s\r\n", request.eid);
	headers = L"Authorization: Bearer ";
	headers += request.eid;
	headers += L"\r\n";


	CString strJsonBody = SerializePrestageVerifyRequest(request);

	//Send and receive
	HttpResponse httpResponse = MakeRequest(PRESTAGING_VERIFY, route, headers, strJsonBody, HTTP_POST, &success);

	// [RWC6-676] Start SKKim 2024.05.28
	response.resultinfo.strStatusCode = httpResponse.StatusCode;
	response.resultinfo.strStatusMsg = httpResponse.StatusMessage;
	// [RWC6-676] End SKKim 2024.05.28

	success = DeserializePrestageVerifyResponse(httpResponse.ResponseData, response);

	return HandleDeserializationResult(success, L"PRE_STAGE");
}

bool TangoPayService::StagingValidationRequest(TangoPayStagingValidationRequest request, TangoPayStagingValidationResponse& response)
{
	NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_STAGE_VALIDATION, L"", L"PRE_STG_VALID");
	NHDBG((L"StagingValidationRequest API call\r\n"));

	bool success = false;

	CString route;
	route = ENDPOINT_STAGE_VALIDATION;

	CString headers;
	//headers.Format(L"Authorization: Bearer %s\r\n", request.eid);
	headers = L"Authorization: Bearer ";
	headers += request.eid;
	headers += L"\r\n";


	CString strJsonBody = SerializePrestageValidationRequest(request);

	//Send and receive
	HttpResponse httpResponse = MakeRequest(PRESTAGING_VALIDATION, route, headers, strJsonBody, HTTP_POST, &success);

	// [RWC6-676] Start SKKim 2024.05.28
	response.resultinfo.strStatusCode = httpResponse.StatusCode;
	response.resultinfo.strStatusMsg = httpResponse.StatusMessage;
	// [RWC6-676] End SKKim 2024.05.28

	success = DeserializePrestageValidationResponse(httpResponse.ResponseData, response);


	return HandleDeserializationResult(success, L"STG_VALID");
}

bool TangoPayService::CommitRequest(TangoPaySendMoneyCommitRequest request, TangoPaySendMoneyCommitResponse& response)
{
	NVDump(NVP1_OTHER, NVDUMP_UNIT2_TANGOPAY, NVP3_TANGPAY_COMMIT, L"", L"COMMIT");
	NHDBG((L"CommitRequest API call\r\n"));

	bool success = false;

	CString route;
	route = ENDPOINT_COMMIT;

	CString headers;
	//headers.Format(L"Authorization: Bearer %s\r\n", request.eid);
	headers = L"Authorization: Bearer ";
	headers += request.eid;
	headers += L"\r\n";

	CString strJsonBody = SerializeCommitRequest(request);

	//Send and receive
	HttpResponse httpResponse = MakeRequest(COMMIT, route, headers, strJsonBody, HTTP_POST, &success);

	// [RWC6-676] Start SKKim 2024.05.28
	response.resultinfo.strStatusCode = httpResponse.StatusCode;
	response.resultinfo.strStatusMsg = httpResponse.StatusMessage;
	// [RWC6-676] End SKKim 2024.05.28

	success = DeserializeCommitResponse(httpResponse.ResponseData, response);

	return HandleDeserializationResult(success, L"COMMIT");

}


//=========================
//==  Internal Commands  ==
//=========================
CString TangoPayService::CreateRoute(CString format, CString host)
{
	return L"";
}

HttpResponse TangoPayService::MakeRequest(int requestType, CString route, CString headers, CString data, HTTP_METHOD method, bool* success)
{
	HttpResponse response;
	NHDBG((L"Making request to %s\r\n", route));

	CString strRequestData;
	char* buffer = NULL;
	char* trace_buff = NULL;
	*success = false;

	CString sheaders;
	//CString sheaders(L"Content-Type: application/json\r\n");

	//if (headers.GetLength() > 0)
	//	sheaders = headers;
	//else
	//	sheaders = L"Accept: text/plain\r\n";
	if (headers.GetLength() > 0)
		//sheaders.AppendFormat(L"%s", headers);
		sheaders = headers;

	sheaders += L"Content-Type: application/json\r\n";

	// make trace file
	CString strTraceData;

	strTraceData += route + L"\r\n";
	strTraceData += L"\r\n";

	strTraceData += sheaders + L"\r\n";
	strTraceData += L"\r\n";

	strTraceData += data + L"\r\n";

	int nTraceDataSize = strTraceData.GetLength();
	trace_buff = new char[nTraceDataSize + 1]();
	WideToMulti(trace_buff, strTraceData, nTraceDataSize);
	SaveRequest(requestType, (unsigned char*)trace_buff, nTraceDataSize+1);

	if (trace_buff != NULL)
		delete [] trace_buff;

	// Encode the data, if present
	if(false == data.IsEmpty())
	{
		int dataSize = data.GetLength();
		buffer = new char[dataSize + 1]();

		WideToMulti(buffer, data, dataSize);
		SaveRequest(requestType, (unsigned char*)buffer, dataSize+1);
	}
	else
	{
		int dataSize = route.GetLength();
		buffer = new char[dataSize + 1]();
		memset(buffer, 0, dataSize);
	}

	if (data.GetLength() > 0)
		NHTRACE((L"%s\r\n", data));

	HttpRequest request;
	
	request.Url = route;
	request.Headers = sheaders;
	request.Data = (unsigned char*)buffer;

	HTTPERR result = _http->SendRequest(method, &response, request);

	// Delete the buffer, if necessary.
	if(NULL != buffer)
		delete[] buffer;

	if(HTTP_NO_ERR != result)
	{
		NHERROR((L"Error in web request. Error code: %d\r\n", result));
		return response;
	}

	SaveResponse(requestType, response.ResponseData);
	NHTRACE((L"%s\r\n", response.ResponseData));

	//*success = true;
	if (response.StatusCode == _T("200"))
		*success = true;
	else
		*success = false;

	return response;
}

bool TangoPayService::HandleRequestFailure(bool rVal, CString requestType)
{
	if(false == rVal)
	{
		NHDBG((L"Failed to HandleRequestFailure (%s) request\r\n", requestType));
	}

	return rVal;
}

bool TangoPayService::HandleDeserializationResult(bool result, CString requestType)
{
	if(false == result)
	{
		NHDBG((L"Failed to HandleDeserializationResult (%s)\r\n", requestType));
	}
	return result;
}

void TangoPayService::SaveRequest(int requestType, BYTE* request, int length)
{
	//////////////////////////////////////////////////////////////////////////////////////////////////
	// Save request message to file (overwite previous transaction. Leave the latest transaction only)
	CString strLogFileName;
	strLogFileName.Format(_T("%s\\TangoPayService_Req%02d.dat"), ATM_DATA_PATH, requestType);
	NHDBG((L"LOG FILE = [%s]\n", strLogFileName));

	CFile cf;
	bool opened = (cf.Open(strLogFileName, CFile::modeWrite | CFile::modeCreate | CFile::typeBinary) == 1);

	if(FALSE == opened)
	{
		NHDBG((L"[FAIL TO OPEN(%s)]\n", strLogFileName));
	}
	else
	{
		cf.Write(request, length);
		cf.Close();
	}
	// Save request message to file (overwite previous transaction. Leave the latest transaction only)
}

//TODO:: Make these into general use functions. Replace all occurrances in the AP with the general one.
void TangoPayService::SaveResponse(int requestType, CString responseData)
{
	///////////////////////////////////////////////////////////////////////////////////////////////////
	// Save response message to file (overwite previous transaction. Leave the latest transaction only)
	CString strLogFileName;
	strLogFileName.Format(_T("%s\\TangoPay_Res%02d.dat"), ATM_DATA_PATH, requestType);
	NHDBG((L"LOG FILE = [%s]\n", strLogFileName));

	CFile cf;
	bool opened = (cf.Open(strLogFileName, CFile::modeWrite | CFile::modeCreate | CFile::typeBinary) == 1);
	if(FALSE == opened)
		NHDBG((L"[FAIL TO OPEN(%s)]\n", strLogFileName));
	else
	{
		int nSize = responseData.GetLength();
		char *pBuffer = new char[nSize+1];
		memset(pBuffer, 0, nSize+1);
		WideToMulti( pBuffer, responseData, nSize);
		cf.Write(pBuffer, nSize );
		delete [] pBuffer;
		cf.Close();
	}
	// Save response message to file (overwite previous transaction. Leave the latest transaction only)
	///////////////////////////////////////////////////////////////////////////////////////////////////
}

//=====================================================
//==  PRIVATE. TangoPayService functions SERIALIZATION  ==
//=====================================================
CString TangoPayService::SerializeGetTerminalJWTRequest(TangoPayGetTerminaljwtRequest request)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);

	json_set_string(request.Id, root_object, "Id");
	json_set_string(request.Key, root_object, "Key");
	json_set_string(request.Nonce, root_object, "Nonce");
	json_set_string(request.CheckSum, root_object, "CheckSum");

	char *serialized = json_serialize_to_string(root_value);
	CString data(serialized);

	json_free_serialized_string(serialized);
	json_value_free(root_value);

	return data;
}


CString TangoPayService::SerializePrestageVerifyRequest(TangoPayPreStagingVerifyRequest request)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);

	json_set_string(request.PhoneNumber, root_object, "phoneNumber");
	json_set_string(request.CountryCode, root_object, "countryCode");
	json_set_string(request.OtpCode, root_object, "otpCode");
	json_set_string(request.OtpSeed, root_object, "otpSeed");

	char *serialized = json_serialize_to_string(root_value);
	CString data(serialized);

	json_free_serialized_string(serialized);
	json_value_free(root_value);

	return data;
}

CString TangoPayService::SerializePrestageValidationRequest(TangoPayStagingValidationRequest request)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);

	json_set_string(request.MachineId, root_object, "MachineId");
	json_set_string(request.PartnerId, root_object, "PartnerId");
	json_set_string(request.TransactionId, root_object, "TransactionId");
	json_set_string(request.ConfirmationNumber, root_object, "ConfirmationNumber");
	json_set_string(request.PrestagingId, root_object, "PrestagingId");

	char *serialized = json_serialize_to_string(root_value);
	CString data(serialized);

	json_free_serialized_string(serialized);
	json_value_free(root_value);

	return data;
}

CString TangoPayService::SerializeCommitRequest(TangoPaySendMoneyCommitRequest request)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);

	json_set_string(request.TransactionId, root_object, "TransactionId");
	json_set_string(request.MgiTransactionSessionId, root_object, "MgiTransactionSessionId");

	char *serialized = json_serialize_to_string(root_value);
	CString data(serialized);

	json_free_serialized_string(serialized);
	json_value_free(root_value);

	return data;
}


#define JSON_NAME_RESULT			"result"
#define JSON_NAME_ERROR_OBJ			"error"
#define JSON_NAME_ERROR_CODE		"code"
#define JSON_NAME_ERROR_MSG			"message"

//=====================================================
//==  PRIVATE. TangoPayService functions DESERIALIZATION  ==
//=====================================================
bool TangoPayService::DeserializeResetApiKeyResponse(CString data)
{
	bool rVal = false;
	CString strTemp;

	JSON_Value* rootValue = NULL;
	JSON_Object* object = NULL;
	JSON_Object* Error_Object = NULL;

	CStringToJsonObject(data, &rootValue, &object);
	if(NULL != rootValue)
	{
		rVal =	(json_object_get_boolean(object, JSON_NAME_RESULT) == 1);	// JSON_NAME_RESULT

		// Error Object
		{
			Error_Object = json_object_get_object(object, JSON_NAME_ERROR_OBJ);	// JSON_NAME_ERROR_OBJ
			if (Error_Object != NULL)
			{
				//strTemp =		(CString)(json_object_get_string(Error_Object, "Code"));			// JSON_NAME_ERROR_CODE
				//strTemp =	(CString)(json_object_get_string(Error_Object, "Message"));				// JSON_NAME_ERROR_MSG
				CharToUniCodeCString(json_object_get_string(Error_Object, JSON_NAME_ERROR_CODE), strTemp);
				CharToUniCodeCString(json_object_get_string(Error_Object, JSON_NAME_ERROR_MSG), strTemp);
			}
			else
			{
				NHDBG((L"DeserializeResetApiKeyResponse - Error Object is null\r\n"));
			}
		}
	}
	else
	{
			NHDBG((L"DeserializeResetApiKeyResponse - Json Value Object is null\r\n"));
	}

	return rVal;
}

bool TangoPayService::DeserializeGetPublicKeyResponse(CString data, TangoPayGetPublicKeyResponse& response)
{
	bool rVal = false;
	JSON_Value* rootValue = NULL;
	JSON_Object* object = NULL;
	JSON_Object* Error_Object = NULL;

	CStringToJsonObject(data, &rootValue, &object);
	if(NULL != rootValue)
	{
		response.resultinfo.Result =		(json_object_get_boolean(object, JSON_NAME_RESULT) == 1);	// JSON_NAME_RESULT


		// Error Object
		{
			Error_Object = json_object_get_object(object, JSON_NAME_ERROR_OBJ);	// JSON_NAME_ERROR_OBJ
			if (Error_Object != NULL)
			{
				//response.resultinfo.ErrCode =		(CString)(json_object_get_string(Error_Object, "Code"));	// JSON_NAME_ERROR_CODE
				//response.resultinfo.ErrMessage =	(CString)(json_object_get_string(Error_Object, "Message"));	// JSON_NAME_ERROR_MSG
				CharToUniCodeCString(json_object_get_string(Error_Object, JSON_NAME_ERROR_CODE), response.resultinfo.ErrCode);
				CharToUniCodeCString(json_object_get_string(Error_Object, JSON_NAME_ERROR_MSG), response.resultinfo.ErrMessage);
			}
			else
			{
				NHDBG((L"DeserializeGetPublicKeyResponse - Error Object is null\r\n"));
			}
		}

		response.PublicKey =	(CString)json_object_get_string(object, "publicKey");
		response.CheckSum =		(CString)json_object_get_string(object, "checkSum");
	}

	if(rootValue != NULL)
		json_value_free(rootValue);

	if (response.resultinfo.Result == true)
		rVal = true;

	return rVal;
}

bool TangoPayService::DeserializeGetTerminalJWTResponse(CString data, TangoPayGetTerminaljwtResponse& response)
{
	bool rVal = false;
	JSON_Value* rootValue = NULL;
	JSON_Object* object = NULL;
	JSON_Object* Error_Object = NULL;

	CStringToJsonObject(data, &rootValue, &object);
	if(NULL != rootValue)
	{
		response.resultinfo.Result =		(json_object_get_boolean(object, JSON_NAME_RESULT) == 1);	// JSON_NAME_RESULT

		// Error Object
		{
			Error_Object = json_object_get_object(object, JSON_NAME_ERROR_OBJ);	// JSON_NAME_ERROR_OBJ
			if (Error_Object != NULL)
			{
				//response.resultinfo.ErrCode =		(CString)json_object_get_string(Error_Object, "Code");		// JSON_NAME_ERROR_CODE
				//response.resultinfo.ErrMessage =	(CString)json_object_get_string(Error_Object, "Message");	// JSON_NAME_ERROR_MSG
				CharToUniCodeCString(json_object_get_string(Error_Object, JSON_NAME_ERROR_CODE), response.resultinfo.ErrCode);
				CharToUniCodeCString(json_object_get_string(Error_Object, JSON_NAME_ERROR_MSG), response.resultinfo.ErrMessage);
			}
			else
			{
				NHDBG((L"DeserializeGetTerminalJWTResponse - Error Object is null\r\n"));
			}
		}

		response.AccessToken =	(CString)json_object_get_string(object, "accessToken");
		response.Nonce =		(CString)json_object_get_string(object, "nonce");
		response.Signature =	(CString)json_object_get_string(object, "signature");
	}

	if(rootValue != NULL)
		json_value_free(rootValue);

	if (response.resultinfo.Result == true)
	{
		rVal = true;
	}
	else
	{
		NHDBG((L"DeserializeGetTerminalJWTResponse - Result : false\r\n"));
		NHDBG((L"DeserializeGetTerminalJWTResponse - Error Code : %s\r\n", response.resultinfo.ErrCode));
		NHDBG((L"DeserializeGetTerminalJWTResponse - Error Message : %s\r\n", response.resultinfo.ErrMessage));
	}

	return rVal;
}

bool TangoPayService::DeserializeGetUserResponse(CString data, TangoPayGetUserResponse& response)
{
	bool rVal = false;
	JSON_Value* rootValue = NULL;
	JSON_Object* object = NULL;
	JSON_Object* Error_Object = NULL;

	CStringToJsonObject(data, &rootValue, &object);
	if(NULL != rootValue)
	{
		response.resultinfo.Result =		(json_object_get_boolean(object, JSON_NAME_RESULT) == 1);

		// Error Object - 문서상에는 첫글자가 대문자이지만 실제 내려오는 값은 소문자임
		{
			Error_Object = json_object_get_object(object, JSON_NAME_ERROR_OBJ);
			if (Error_Object != NULL)
			{
				//response.resultinfo.ErrCode =		(CString)json_object_get_string(Error_Object, JSON_NAME_ERROR_CODE);
				//response.resultinfo.ErrMessage =	(CString)json_object_get_string(Error_Object, JSON_NAME_ERROR_MSG);
				CharToUniCodeCString(json_object_get_string(Error_Object, JSON_NAME_ERROR_CODE), response.resultinfo.ErrCode);
				CharToUniCodeCString(json_object_get_string(Error_Object, JSON_NAME_ERROR_MSG), response.resultinfo.ErrMessage);
			}
			else
			{
				NHDBG((L"DeserializeGetUserResponse - Error Object is null\r\n"));
			}
		}

		response.retCode =		(CString)json_object_get_string(object, "retCode");
		response.retMsg =		(CString)json_object_get_string(object, "retMsg");
		response.userId =		(CString)json_object_get_string(object, "userId");
	}

	if(rootValue != NULL)
		json_value_free(rootValue);

	if (response.resultinfo.Result == true)
	{
		rVal = true;
	}
	else
	{
		NHDBG((L"DeserializeGetUserResponse - Result : false\r\n"));
		NHDBG((L"DeserializeGetUserResponse - Error Code : %s\r\n", response.resultinfo.ErrCode));
		NHDBG((L"DeserializeGetUserResponse - Error Message : %s\r\n", response.resultinfo.ErrMessage));
		NHDBG((L"DeserializeGetUserResponse - Return Code : %s\r\n", response.retCode));
		NHDBG((L"DeserializeGetUserResponse - Return Message : %s\r\n", response.retMsg));
	}

	return rVal;
}

bool TangoPayService::DeserializePrestageVerifyResponse(CString data, TangoPayPreStagingVerifyResponse& response)
{
	bool rVal = false;
	JSON_Value* rootValue = NULL;
	JSON_Object* object = NULL;
	JSON_Object* Error_Object = NULL;
	JSON_Object* prestaingInfo_Object = NULL;
	JSON_Array* array_Reservations = NULL;

	CStringToJsonObject(data, &rootValue, &object);

	if(NULL != rootValue)
	{
		response.resultinfo.Result =		(json_object_get_boolean(object, JSON_NAME_RESULT) == 1);

		// Error Object
		{
			Error_Object = json_object_get_object(object, JSON_NAME_ERROR_OBJ);
			if (Error_Object != NULL)
			{
				//response.resultinfo.ErrCode =		(CString)json_object_get_string(Error_Object, JSON_NAME_ERROR_CODE);
				//response.resultinfo.ErrMessage =	(CString)json_object_get_string(Error_Object, JSON_NAME_ERROR_MSG);
				CharToUniCodeCString(json_object_get_string(Error_Object, JSON_NAME_ERROR_CODE), response.resultinfo.ErrCode);
				CharToUniCodeCString(json_object_get_string(Error_Object, JSON_NAME_ERROR_MSG), response.resultinfo.ErrMessage);
			}
			else
			{
				NHDBG((L"DeserializePrestageVerifyResponse - Error Object is null\r\n"));
			}
		}

		response.retCode =		(CString)json_object_get_string(object, "retCode");
		response.retMsg =		(CString)json_object_get_string(object, "retMsg");
		response.verify =		(json_object_get_boolean(object, "verify") == 1);

		// PrestaingInfo Object
		{
			prestaingInfo_Object = json_object_get_object(object, "prestagingInfo");
			if (prestaingInfo_Object != NULL)
			{
				response.prestaingInfo_obj.preStagingId =				(CString)json_object_get_string(prestaingInfo_Object, "preStagingId");
				response.prestaingInfo_obj.serviceProviderCd =			(CString)json_object_get_string(prestaingInfo_Object, "serviceProviderCd");
				response.prestaingInfo_obj.serviceTypeCd =				(CString)json_object_get_string(prestaingInfo_Object, "serviceTypeCd");
				response.prestaingInfo_obj.userId =						(CString)json_object_get_string(prestaingInfo_Object, "userId");
				response.prestaingInfo_obj.deliveryOption =				(CString)json_object_get_string(prestaingInfo_Object, "deliveryOption");

				// UniCode로 변환 처리
				//response.prestaingInfo_obj.deliveryOptionDisplayName =	(CString)json_object_get_string(prestaingInfo_Object, "deliveryOptionDisplayName");
				//response.prestaingInfo_obj.receiveCountry =				(CString)json_object_get_string(prestaingInfo_Object, "receiveCountry");
				//response.prestaingInfo_obj.receiveState =				(CString)json_object_get_string(prestaingInfo_Object, "receiveState");
				//response.prestaingInfo_obj.receiveCurrency =			(CString)json_object_get_string(prestaingInfo_Object, "receiveCurrency");
				//response.prestaingInfo_obj.receiverFirstName =			(CString)json_object_get_string(prestaingInfo_Object, "receiverFirstName");
				//response.prestaingInfo_obj.receiverLastName =			(CString)json_object_get_string(prestaingInfo_Object, "receiverLastName");

				CharToUniCodeCString(json_object_get_string(prestaingInfo_Object, "deliveryOptionDisplayName"), response.prestaingInfo_obj.deliveryOptionDisplayName);
				response.prestaingInfo_obj.receiveCountry =				(CString)json_object_get_string(prestaingInfo_Object, "receiveCountry");
				response.prestaingInfo_obj.receiveState =				(CString)json_object_get_string(prestaingInfo_Object, "receiveState");
				response.prestaingInfo_obj.receiveCurrency =			(CString)json_object_get_string(prestaingInfo_Object, "receiveCurrency");
				CharToUniCodeCString(json_object_get_string(prestaingInfo_Object, "receiverFirstName"), response.prestaingInfo_obj.receiverFirstName);
				CharToUniCodeCString(json_object_get_string(prestaingInfo_Object, "receiverLastName"), response.prestaingInfo_obj.receiverLastName);

				array_Reservations = json_object_get_array(prestaingInfo_Object, "reservations");

				// Reservation Array
				if (array_Reservations != NULL)
				{
					DeserializeReservatiosArray(array_Reservations, response.prestaingInfo_obj.reservations_array);
				}
				else
				{
					NHDBG((L"DeserializePrestageVerifyResponse - Reservations Array is null\r\n"));
				}

				response.prestaingInfo_obj.stateName =					(CString)json_object_get_string(prestaingInfo_Object, "stateName");
				response.prestaingInfo_obj.countryName =				(CString)json_object_get_string(prestaingInfo_Object, "countryName");
				response.prestaingInfo_obj.ctn =						(CString)json_object_get_string(prestaingInfo_Object, "ctn");
				response.prestaingInfo_obj.ctnCountryCode =				(CString)json_object_get_string(prestaingInfo_Object, "ctnCountryCode");
				response.prestaingInfo_obj.otpCode =					(CString)json_object_get_string(prestaingInfo_Object, "otpCode");
				response.prestaingInfo_obj.otpExpireTime =				(int)json_object_get_number(prestaingInfo_Object, "otpExpireTime");
				response.prestaingInfo_obj.createDt =					(CString)json_object_get_string(prestaingInfo_Object, "createDt");
				response.prestaingInfo_obj.receiveAgentID=				(CString)json_object_get_string(prestaingInfo_Object, "receiveAgentID");
				response.prestaingInfo_obj.receiveAgentAbbreviation =	(CString)json_object_get_string(prestaingInfo_Object, "receiveAgentAbbreviation");
				response.prestaingInfo_obj.confirmationNumber =			(CString)json_object_get_string(prestaingInfo_Object, "confirmationNumber");
				response.prestaingInfo_obj.transactionId =				(CString)json_object_get_string(prestaingInfo_Object, "transactionId");
				response.prestaingInfo_obj.expired =					(json_object_get_boolean(prestaingInfo_Object, "expired") == 1);
				response.prestaingInfo_obj.isUsed =						(json_object_get_boolean(prestaingInfo_Object, "isUsed") == 1);
				response.prestaingInfo_obj.otpSeed =					(int)json_object_get_number(prestaingInfo_Object, "otpSeed");
				response.prestaingInfo_obj.lat =						(int)json_object_get_number(prestaingInfo_Object, "lat");
				response.prestaingInfo_obj.lng =						(int)json_object_get_number(prestaingInfo_Object, "lng");
				response.prestaingInfo_obj.isRestored =					(json_object_get_boolean(prestaingInfo_Object, "isRestored") == 1);
			}
			else
			{
				NHDBG((L"DeserializePrestageVerifyResponse - prestaingInfo Object is null\r\n"));
			}
		}
	}

	if(rootValue != NULL)
		json_value_free(rootValue);

	if (response.resultinfo.Result == true)
	{
		rVal = true;
	}
	else
	{
		NHDBG((L"DeserializePrestageVerifyResponse - Result : false\r\n"));
		NHDBG((L"DeserializePrestageVerifyResponse - Error Code : %s\r\n", response.resultinfo.ErrCode));
		NHDBG((L"DeserializePrestageVerifyResponse - Error Message : %s\r\n", response.resultinfo.ErrMessage));
		NHDBG((L"DeserializePrestageVerifyResponse - Return Code : %s\r\n", response.retCode));
		NHDBG((L"DeserializePrestageVerifyResponse - Return Message : %s\r\n", response.retMsg));
	}

	return rVal;
}


void TangoPayService::DeserializeReservatiosArray(JSON_Array* arr_reservations, Reservations& reservationsData)
{
	JSON_Object* object = NULL;
	Reservation temp;

	size_t count = json_array_get_count(arr_reservations);

	for(size_t i = 0; i < count; i++)
	{
		object = json_array_get_object(arr_reservations, i);

		if(NULL == object)
			continue;

		temp.Key	= (CString)json_object_get_string(object, "key");
		temp.Value  = (CString)json_object_get_string(object, "value");

		reservationsData.AddTail(temp);
	}
}


bool TangoPayService::DeserializePrestageValidationResponse(CString data, TangoPayStagingValidationResponse& response)
{
	bool rVal = false;
	JSON_Value* rootValue = NULL;
	JSON_Object* object = NULL;
	JSON_Object* Error_Object = NULL;
	JSON_Object* SendAmountInfo_Object = NULL;
	JSON_Array* array_detailSendAmounts = NULL;
	JSON_Object* ReceiveAmountInfo_Object = NULL;
	JSON_Array* array_detailReceiveAmounts = NULL;

	JSON_Array* array_DisclosuresFirst = NULL;
	JSON_Array* array_DisclosuresSecond = NULL;

	CStringToJsonObject(data, &rootValue, &object);

	if(NULL != rootValue)
	{
		response.resultinfo.Result =		(json_object_get_boolean(object, JSON_NAME_RESULT) == 1);

		// Error Object
		{
			Error_Object = json_object_get_object(object, JSON_NAME_ERROR_OBJ);
			if (Error_Object != NULL)
			{
				//response.resultinfo.ErrCode =		(CString)json_object_get_string(Error_Object, JSON_NAME_ERROR_CODE);
				//response.resultinfo.ErrMessage =	(CString)json_object_get_string(Error_Object, JSON_NAME_ERROR_MSG);
				CharToUniCodeCString(json_object_get_string(Error_Object, JSON_NAME_ERROR_CODE), response.resultinfo.ErrCode);
				CharToUniCodeCString(json_object_get_string(Error_Object, JSON_NAME_ERROR_MSG), response.resultinfo.ErrMessage);
			}
			else
			{
				NHDBG((L"DeserializePrestageValidationResponse - Error Object is null\r\n"));
			}
		}

		response.transactionId =				(CString)json_object_get_string(object, "transactionId");
		response.mgiTransactionSessionId =		(CString)json_object_get_string(object, "mgiTransactionSessionId");

		// Name도 Unicode로 올수 있으므로 Unicode로 변환 처리
		//response.receiverFirstName =			(CString)json_object_get_string(object, "receiverFirstName");
		//response.receiverMiddleName =			(CString)json_object_get_string(object, "receiverMiddleName");
		//response.receiverLastName =				(CString)json_object_get_string(object, "receiverLastName");
		//response.deliveryOption =				(CString)json_object_get_string(object, "deliveryOption");
		//response.deliverOptionDisplayName =		(CString)json_object_get_string(object, "deliverOptionDisplayName");
		CharToUniCodeCString(json_object_get_string(object, "receiverFirstName"), response.receiverFirstName);
		CharToUniCodeCString(json_object_get_string(object, "receiverMiddleName"), response.receiverMiddleName);
		CharToUniCodeCString(json_object_get_string(object, "receiverLastName"), response.receiverLastName);
		response.deliveryOption =				(CString)json_object_get_string(object, "deliveryOption");
		CharToUniCodeCString(json_object_get_string(object, "deliverOptionDisplayName"), response.deliverOptionDisplayName);


		// SendAmountInfo Object
		{
			SendAmountInfo_Object = json_object_get_object(object, "sendAmountInfo");
			if (SendAmountInfo_Object != NULL)
			{
				response.SendAmountInfo_obj.sendAmount =				(CString)json_object_get_string(SendAmountInfo_Object, "sendAmount");
				response.SendAmountInfo_obj.sendCurrency =				(CString)json_object_get_string(SendAmountInfo_Object, "sendCurrency");
				response.SendAmountInfo_obj.totalSendFees =				(CString)json_object_get_string(SendAmountInfo_Object, "totalSendFees");
				response.SendAmountInfo_obj.totalDiscountAmount =		(CString)json_object_get_string(SendAmountInfo_Object, "totalDiscountAmount");
				response.SendAmountInfo_obj.totalSendTaxes =			(CString)json_object_get_string(SendAmountInfo_Object, "totalSendTaxes");
				response.SendAmountInfo_obj.totalAmountToCollect =		(CString)json_object_get_string(SendAmountInfo_Object, "totalAmountToCollect");

				// Detail SendAmounts Array
				array_detailSendAmounts = json_object_get_array(SendAmountInfo_Object, "detailSendAmounts");

				if (array_detailSendAmounts != NULL)
				{
					DeserializeDetailSendAmountsArray(array_detailSendAmounts, response.SendAmountInfo_obj.detailSendAmounts_array);
				}
				else
				{
					NHDBG((L"DeserializePrestageValidationResponse - detailSendAmounts Array is null\r\n"));
				}
			}
			else
			{
				NHDBG((L"DeserializePrestageValidationResponse - SendAmountInfo Object is null\r\n"));
			}
		}

		// ReceiveAmountInfo Object
		{
			ReceiveAmountInfo_Object = json_object_get_object(object, "receiveAmountInfo");
			if (ReceiveAmountInfo_Object != NULL)
			{
				response.ReceiveAmountInfo_obj.receiveAmount =				(CString)json_object_get_string(ReceiveAmountInfo_Object, "receiveAmount");
				response.ReceiveAmountInfo_obj.receiveCurrency =			(CString)json_object_get_string(ReceiveAmountInfo_Object, "receiveCurrency");
				response.ReceiveAmountInfo_obj.validCurrencyIndicator =		(json_object_get_boolean(ReceiveAmountInfo_Object, "validCurrencyIndicator") == 1);
				response.ReceiveAmountInfo_obj.payoutCurrency =				(CString)json_object_get_string(ReceiveAmountInfo_Object, "payoutCurrency");
				response.ReceiveAmountInfo_obj.totalReceiveFees =			(CString)json_object_get_string(ReceiveAmountInfo_Object, "totalReceiveFees");
				response.ReceiveAmountInfo_obj.totalReceiveTaxes =			(CString)json_object_get_string(ReceiveAmountInfo_Object, "totalReceiveTaxes");
				response.ReceiveAmountInfo_obj.totalReceiveAmount =			(CString)json_object_get_string(ReceiveAmountInfo_Object, "totalReceiveAmount");
				response.ReceiveAmountInfo_obj.receiveFeesAreEstimated =	(json_object_get_boolean(ReceiveAmountInfo_Object, "receiveFeesAreEstimated") == 1);
				response.ReceiveAmountInfo_obj.receiveTaxesAreEstimated =	(json_object_get_boolean(ReceiveAmountInfo_Object, "receiveTaxesAreEstimated") == 1);

				// Detail ReceiveAmounts Array
				array_detailReceiveAmounts = json_object_get_array(ReceiveAmountInfo_Object, "detailEstimatedReceiveAmounts");

				if (array_detailReceiveAmounts != NULL)
				{
					DeserializeDetailReceiveAmountsArray(array_detailReceiveAmounts, response.ReceiveAmountInfo_obj.detailEstimatedReceiveAmounts_array);
				}
				else
				{
					NHDBG((L"DeserializePrestageValidationResponse - DeserializeDetailReceiveAmountsArray Array is null\r\n"));
				}
			}
			else
			{
				NHDBG((L"DeserializePrestageValidationResponse - DeserializeDetailReceiveAmountsArray Object is null\r\n"));
			}
		}

		// DisclosuresFirst Array
		array_DisclosuresFirst = json_object_get_array(object, "disclosuresFirst");
		if (array_DisclosuresFirst != NULL)
		{
			DeserializeDetailDisclosuresArray(array_DisclosuresFirst, response.disclosuresFirst_array);
		}
		else
		{
			NHDBG((L"DeserializePrestageValidationResponse - DeserializeDetailDisclosuresFirst Array Array is null\r\n"));
		}

		array_DisclosuresSecond = json_object_get_array(object, "disclosuresSecond");
		if (array_DisclosuresSecond != NULL)
		{
			DeserializeDetailDisclosuresArray(array_DisclosuresSecond, response.disclosuresSecond_array);
		}
		else
		{
			NHDBG((L"DeserializePrestageValidationResponse - DeserializeDetailDisclosuresSecond Array Array is null\r\n"));
		}
	}

	if (response.resultinfo.Result == true)
	{
		rVal = true;
	}
	else
	{
		NHDBG((L"DeserializePrestageValidationResponse - Result : false\r\n"));
		NHDBG((L"DeserializePrestageValidationResponse - Error Code : %s\r\n", response.resultinfo.ErrCode));
		NHDBG((L"DeserializePrestageValidationResponse - Error Message : %s\r\n", response.resultinfo.ErrMessage));
	}

	return rVal;
	
}

void TangoPayService::DeserializeDetailSendAmountsArray(JSON_Array* arr_SendAmounts, DetailSendAmounts& SendAmountsData)
{
	JSON_Object* object = NULL;
	DetailSendAmount temp;

	size_t count = json_array_get_count(arr_SendAmounts);

	for(size_t i = 0; i < count; i++)
	{
		object = json_array_get_object(arr_SendAmounts, i);

		if(NULL == object)
			continue;

		temp.amountType	=		(CString)json_object_get_string(object, "amountType");
		temp.amount  =			(CString)json_object_get_string(object, "amount");
		temp.amountCurrency =	(CString)json_object_get_string(object, "amountCurrency");

		SendAmountsData.AddTail(temp);
	}
}

void TangoPayService::DeserializeDetailReceiveAmountsArray(JSON_Array* arr_ReceiveAmounts, DetailEstimatedReceiveAmounts& ReceiveAmountsData)
{
	JSON_Object* object = NULL;
	DetailEstimatedReceiveAmount temp;

	size_t count = json_array_get_count(arr_ReceiveAmounts);

	for(size_t i = 0; i < count; i++)
	{
		object = json_array_get_object(arr_ReceiveAmounts, i);

		if(NULL == object)
			continue;

		temp.amountType	=		(CString)json_object_get_string(object, "amountType");
		temp.amount  =			(CString)json_object_get_string(object, "amount");
		temp.amountCurrency =	(CString)json_object_get_string(object, "amountCurrency");

		ReceiveAmountsData.AddTail(temp);
	}
}

void TangoPayService::DeserializeDetailDisclosuresArray(JSON_Array* arr_Disclosures, sDisclosures& DisclosuresData)
{
	JSON_Object* object = NULL;
	sDisclosure temp;

	size_t count = json_array_get_count(arr_Disclosures);

	for(size_t i = 0; i < count; i++)
	{
		object = json_array_get_object(arr_Disclosures, i);

		if(NULL == object)
			continue;

		// UNICODE값으로 변환 처리
		//temp.fraudWarning	=		(CString)json_object_get_string(object, "fraudWarning");
		//temp.disclosure		 =			(CString)json_object_get_string(object, "disclosure");
		CharToUniCodeCString(json_object_get_string(object, "fraudWarning"), temp.fraudWarning);
		CharToUniCodeCString(json_object_get_string(object, "disclosure"), temp.disclosure);

		DisclosuresData.AddTail(temp);
	}
}

bool TangoPayService::DeserializeCommitResponse(CString data, TangoPaySendMoneyCommitResponse& response)
{
	bool rVal = false;
	JSON_Value* rootValue = NULL;
	JSON_Object* object = NULL;
	JSON_Object* Error_Object = NULL;

	JSON_Array* array_AgentReceipts = NULL;
	JSON_Array* array_ConsumerReceipts = NULL;

	CStringToJsonObject(data, &rootValue, &object);
	if(NULL != rootValue)
	{
		response.resultinfo.Result =		(json_object_get_boolean(object, JSON_NAME_RESULT) == 1);

		// Error Object
		{
			Error_Object = json_object_get_object(object, JSON_NAME_ERROR_OBJ);
			if (Error_Object != NULL)
			{
				//response.resultinfo.ErrCode =		(CString)json_object_get_string(Error_Object, JSON_NAME_ERROR_CODE);
				//response.resultinfo.ErrMessage =	(CString)json_object_get_string(Error_Object, JSON_NAME_ERROR_MSG);
				CharToUniCodeCString(json_object_get_string(Error_Object, JSON_NAME_ERROR_CODE), response.resultinfo.ErrCode);
				CharToUniCodeCString(json_object_get_string(Error_Object, JSON_NAME_ERROR_MSG), response.resultinfo.ErrMessage);
			}
			else
			{
				NHDBG((L"DeserializeCommitResponse - Error Object is null\r\n"));
			}
		}

		response.referenceNumber =				(CString)json_object_get_string(object, "referenceNumber");
		response.referenceNumberTextCode =		(CString)json_object_get_string(object, "referenceNumberTextCode");
		response.referenceNumberText =			(CString)json_object_get_string(object, "referenceNumberText");
		response.referenceNumberConsumerText =	(CString)json_object_get_string(object, "referenceNumberConsumerText");

		// AgentReceipts Array
		array_AgentReceipts = json_object_get_array(object, "agentReceipts");

		if (array_AgentReceipts != NULL)
		{
			DeserializeAgentReceiptsArray(array_AgentReceipts, response.agentreceipts_array);
		}
		else
		{
			NHDBG((L"DeserializeCommitResponse - array_AgentReceipts Array is null\r\n"));
		}


		// Detail ReceiveAmounts Array
		array_ConsumerReceipts = json_object_get_array(object, "consumerReceipts");

		if (array_ConsumerReceipts != NULL)
		{
			DeserializeConsumerReceiptsArray(array_ConsumerReceipts, response.consumerreceipts_array);
		}
		else
		{
			NHDBG((L"DeserializeCommitResponse - DeserializeConsumerReceiptsArray Array is null\r\n"));
		}
	}

	if (response.resultinfo.Result == true)
	{
		rVal = true;
	}
	else
	{
		NHDBG((L"DeserializeCommitResponse - Result : false\r\n"));
		NHDBG((L"DeserializeCommitResponse - Error Code : %s\r\n", response.resultinfo.ErrCode));
		NHDBG((L"DeserializeCommitResponse - Error Message : %s\r\n", response.resultinfo.ErrMessage));
	}

	return rVal;
}

void TangoPayService::DeserializeAgentReceiptsArray(JSON_Array* arr_AgentReceipts, sAgentReceipts& AgentReceiptsData)
{
	JSON_Object* object = NULL;
	AgentReceipt temp;

	size_t count = json_array_get_count(arr_AgentReceipts);

	for(size_t i = 0; i < count; i++)
	{
		//temp.contents = (CString)json_array_get_string(arr_AgentReceipts, i);
		CharToUniCodeCString(json_array_get_string(arr_AgentReceipts, i), temp.contents);

		AgentReceiptsData.AddTail(temp);
	}
}

void TangoPayService::DeserializeConsumerReceiptsArray(JSON_Array* arr_ConsumerReceipts, sConsumerReceipts& ConsumerReceiptsData)
{
	JSON_Object* object = NULL;
	ConsumerReceipt temp;

	size_t count = json_array_get_count(arr_ConsumerReceipts);

	for(size_t i = 0; i < count; i++)
	{
		//temp.contents = (CString)json_array_get_string(arr_ConsumerReceipts, i);
		CharToUniCodeCString(json_array_get_string(arr_ConsumerReceipts, i), temp.contents);
		ConsumerReceiptsData.AddTail(temp);
	}
}


//======================
//== Helper Functions ==
//======================
void TangoPayService::CStringToJsonObject(CString data, JSON_Value** rootValue, JSON_Object** object)
{
	// Start SKKim 2024.04.25 - 대용량 data가 오는 경우 len가 잘못 계산되는 bug fix
	//int len = data.GetLength() + 1;
	//char * json = new char[len]();

	//memset(json, 0, len);
	//WideToMulti(json, data, len);

	int len = 0, converted_cnt = 0;
	char* json = NULL;

	len = WideCharToMultiByte(CP_ACP, 0, data, -1, NULL, NULL, NULL, NULL);
	json = new char[len];

	memset(json, 0, len);
	converted_cnt = WideCharToMultiByte(CP_ACP, 0, data, -1, json, len, NULL, NULL);
	// End SKKim 2024.04.25

	*rootValue = json_parse_string(json);
	*object = json_value_get_object(*rootValue);

	delete [] json;
}

void TangoPayService::CharToUniCodeCString(const char *chSrcValue, CString& strDestValue)
{
	// UNICODE값으로 변환 처리
	int nLen = 0;
	WCHAR* wchTempValue = NULL;

	nLen = MultiByteToWideChar(CP_UTF8, 0, chSrcValue, -1, NULL, NULL);

	wchTempValue = new WCHAR[nLen];
	MultiByteToWideChar(CP_UTF8, 0, chSrcValue, -1, wchTempValue, nLen);

	strDestValue = (CString)wchTempValue;

	delete [] wchTempValue;
}