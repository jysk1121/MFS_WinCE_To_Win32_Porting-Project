#pragma once

#include "stdafx.h"

#include ".\Net\HTTP.h"
#include ".\TangoPayTypes.h"
#include ".\Common\TangoPayConfigurationManager.h"

#include <parson/parson.h>
#pragma comment(lib, "parson.lib")


// [RWC6-676] SKKim 2024.03.27 created header file for TangoPay

//TODO::REPLACE ALL of the bool. with bool. 
void	json_set_string(CString value, JSON_Object *root, const char * name);

//What:	Talks to MoniView to get enhacned bin information
class AFX_CLASS_EXPORT TangoPayService
{
	public:
		///============================
		//== Constructor/Destructor  ==
		//=============================
		explicit TangoPayService(HTTP* http);


		CString HMACSHA256(unsigned char *key, int keylen, CString data);

		//================
		//==  Commands  ==
		//================
		// TangoPay Identifier
		bool ResetAPIKey(CString strAPIKey);			// only test function

		bool GetPublicKeyRequest(CString strApiKey, TangoPayGetPublicKeyResponse& response);

		bool GetTerminaljwtRequest(TangoPayGetTerminaljwtRequest request, TangoPayGetTerminaljwtResponse& response);

		// Transaction API
		bool GetUserRequest(TangoPayGetUserRequest request, TangoPayGetUserResponse& response);
		bool PrestagingVerifyRequest(TangoPayPreStagingVerifyRequest request, TangoPayPreStagingVerifyResponse& response);
		bool StagingValidationRequest(TangoPayStagingValidationRequest request, TangoPayStagingValidationResponse& response);
		bool CommitRequest(TangoPaySendMoneyCommitRequest request, TangoPaySendMoneyCommitResponse& response);

	private:
		//============
		//==  Data  ==
		//============
		HTTP* _http;

		//=========================
		//==  Internal Commands  ==
		//=========================
		CString			CreateRoute(CString format, CString host);
		HttpResponse	MakeRequest(int requestType, CString route, CString headers, CString data, HTTP_METHOD method, bool* success);
		bool			HandleRequestFailure(bool rVal, CString requestType);
		bool			HandleDeserializationResult(bool result, CString requestType);

		void SaveRequest(int requestType, BYTE* request, int length);
		void SaveResponse(int requestType, CString responseData);

		//============
		//==  TODO  ==
		//= Create a separate object to handle the deserialization. 
		//= Own an instance of that here. 
		//= Remove all of the de-serialization code from this object.
		//============
		/*
		 * Serialization
		*/
		CString SerializeGetTerminalJWTRequest(TangoPayGetTerminaljwtRequest request);
		CString SerializePrestageVerifyRequest(TangoPayPreStagingVerifyRequest request);
		CString SerializePrestageValidationRequest(TangoPayStagingValidationRequest request);
		CString SerializeCommitRequest(TangoPaySendMoneyCommitRequest request);

		/*
		* DeSerialization
		*/
		bool DeserializeResetApiKeyResponse(CString data);
		bool DeserializeGetPublicKeyResponse(CString data, TangoPayGetPublicKeyResponse& response);
		bool DeserializeGetTerminalJWTResponse(CString data, TangoPayGetTerminaljwtResponse& response);
		bool DeserializeGetUserResponse(CString data, TangoPayGetUserResponse& response);

		bool DeserializePrestageVerifyResponse(CString data, TangoPayPreStagingVerifyResponse& response);
		void DeserializeReservatiosArray(JSON_Array* arr_reservations, Reservations& reservationsData);

		bool DeserializePrestageValidationResponse(CString data, TangoPayStagingValidationResponse& response);
		void DeserializeDetailSendAmountsArray(JSON_Array* arr_SendAmounts, DetailSendAmounts& SendAmountsData);
		void DeserializeDetailReceiveAmountsArray(JSON_Array* arr_ReceiveAmounts, DetailEstimatedReceiveAmounts& ReceiveAmountsData);
		void DeserializeDetailDisclosuresArray(JSON_Array* arr_Disclosures, sDisclosures& DisclosuresData);

		bool DeserializeCommitResponse(CString data, TangoPaySendMoneyCommitResponse& response);
		void DeserializeAgentReceiptsArray(JSON_Array* arr_AgentReceipts, sAgentReceipts& AgentReceiptsData);
		void DeserializeConsumerReceiptsArray(JSON_Array* arr_ConsumerReceipts, sConsumerReceipts& ConsumerReceiptsData);

		/* Helper functions for the serialization */
		void	JsonArrayToCString(CString value, JSON_Array* root);
		double	CStringToDouble(CString value);
		void	JsonObjectToCString(CString value, JSON_Object* root, const char* name);
		void	CStringToJsonObject(CString data, JSON_Value** rootValue, JSON_Object** object);
		void	CharToUniCodeCString(const char *chSrcValue, CString& strDestValue);

		/*
		* End Serialization
		*/

};