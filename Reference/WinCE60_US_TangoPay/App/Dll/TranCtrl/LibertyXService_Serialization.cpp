#include "stdafx.h"
#include ".\Tran\LibertyXService.h"
#include ".\Common\NHDbgApi.h"

void json_set_string(CString value, JSON_Object *root, const char * name) 
{
	int len = value.GetLength() + 1;
	char * s = new char[len]();
	WideToMulti(s, value, len);
	json_object_dotset_string(root, name, s);
	delete [] s;
}

void parse_cert_chain_array(const char * name, JSON_Object *response, CList<CString, CString&> &certList)
{
	JSON_Array *chainArray = json_object_get_array(response, name);
	int count = json_array_get_count(chainArray);

	for (int i = 0; i < count; i++)
	{
		certList.AddTail(CString(json_array_get_string(chainArray, i)));
	}
}

void json_set_cash_denoms_list(CashoutMap &cashCounts, const char *name, JSON_Object *request)
{
	char denomTmp[32] = {};
	CString denomination;
	int cashCount;
	POSITION pos = cashCounts.GetStartPosition();

	JSON_Value *count_value = json_value_init_object();
	JSON_Object *count_map = json_value_get_object(count_value);

	while (pos != NULL)
	{
		cashCounts.GetNextAssoc(pos, denomination, cashCount);

		WideToMulti(denomTmp, denomination, sizeof(denomTmp));

		json_object_set_number(count_map, denomTmp, cashCount);

		memset(denomTmp, 0, sizeof(denomTmp));
	}

	json_object_set_value(request, name, count_value);
}

void json_set_surcharge_info(SurchargeMap &surchargeInfos, const char *name, JSON_Object *request) 
{
	char currencyCodeTmp[16] = {};
	LXDispenseSurcharge surcharge;
	CString currencyCode;
	POSITION pos = surchargeInfos.GetStartPosition();
	JSON_Value *json_value;
	JSON_Object *json_object;

	JSON_Value *surcharge_value = json_value_init_object();
	JSON_Object *surcharge_object = json_value_get_object(surcharge_value);

	while (pos != NULL)
	{
		surchargeInfos.GetNextAssoc(pos, currencyCode, surcharge);

		memset(currencyCodeTmp, 0, sizeof(currencyCodeTmp));
		WideToMulti(currencyCodeTmp, currencyCode, currencyCode.GetLength());

		json_value = json_value_init_object();
		json_object = json_value_get_object(json_value);

		json_set_string(surcharge.FeeFlat, json_object, "fee_flat");
		json_object_set_number(json_object, "fee_percent_bps", surcharge.FeePercentage * 100);
		json_set_string(surcharge.SurchargeMode, json_object, "mode");

		json_object_set_value(surcharge_object, currencyCodeTmp, json_value);
	}

	json_object_set_value(request, name, surcharge_value);
}

JSON_Value* CLibertyXService::SerializeCard(LXCard card)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);

	json_set_string(card.Last4, root_object, "last4");
	json_set_string(card.Type, root_object, "type");
	json_set_string(card.Name, root_object, "name");
	json_set_string(card.TID, root_object, "tid");

	if (!card.Authorization.IsEmpty())
	{
		json_set_string(card.Authorization, root_object, "authorization");
	}

	if (!card.BIN.IsEmpty())
	{
		json_set_string(card.BIN, root_object, "bin6");
	}

	if (!card.AtmTID.IsEmpty())
	{
		json_set_string(card.AtmTID, root_object, "atmtid");
	}

	return root_value;
}

JSON_Value*	CLibertyXService::SerializeDispensePingInformation(LXDispensePing &data)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);

	json_set_cash_denoms_list(data.CashDenominations, "cash_denominations", root_object);
	json_set_surcharge_info(data.SurchargeInfos, "surcharge", root_object);

	json_set_string(data.MaxDispense, root_object, "max_dispense");
	json_set_string(data.MaxWithdrawalLimit, root_object, "max_withdrawal_limit");

	return root_value;
}

CString	CLibertyXService::SerializeAuthorization(LXAuthorization authorization)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);

	if (authorization.HasCard)
	{
		JSON_Value* cardJson = SerializeCard(authorization.Card);
		json_object_set_value(root_object, "card", cardJson);
	}

	json_set_string(authorization.Token, root_object, "token");
	json_set_string(authorization.Currency, root_object, "currency");
	json_set_string(config.LocationID, root_object, "location_id");
	json_object_set_number(root_object, "source_amount", authorization.SourceAmount);

	char *serialized = json_serialize_to_string(root_value);
	CString message(serialized);

	json_free_serialized_string(serialized);
	json_value_free(root_value);

	return message;
}

bool CLibertyXService::DeserializeAuthorizationResponse(CString data, LXAuthorizationResponse &response)
{
	JSON_Value *rootValue;
	JSON_Object *auth;

	int len = data.GetLength() + 1;
	char * json = new char[len]();
	WideToMulti(json, data, len);

	rootValue = json_parse_string(json);
	auth = json_value_get_object(rootValue);

	response.Message = CString(json_object_get_string(auth, "msg"));
	response.MessageCustomer = CString(json_object_get_string(auth, "msg_customer"));
	response.PaymentId = CString(json_object_get_string(auth, "payment_id"));
	response.Status = CString(json_object_get_string(auth, "status"));

	memset(json, 0, len);
	delete [] json;
	json_value_free(rootValue);

	return true;
}

CString	CLibertyXService::SerializeCancellation(LXCancel cancel)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);
	
	json_set_string(cancel.PaymentId, root_object, "payment_id");

	if (cancel.HasCard)
	{
		JSON_Value* cardJson = SerializeCard(cancel.Card);
		json_object_set_value(root_object, "card", cardJson);	
	}

	if (cancel.HasError)
	{
		json_set_string(cancel.Error.Code, root_object, "error.code");
		json_set_string(cancel.Error.Message, root_object, "error.message");
	}

	char *serialized = json_serialize_to_string(root_value);
	CString message(serialized);

	json_free_serialized_string(serialized);
	json_value_free(root_value);

	return message;
}

bool CLibertyXService::DeserializeCancellationResponse(CString data, LXCancelResponse &response)
{
	return true;
}

CString	CLibertyXService::SerializeExecute(LXExecute execute)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);

	json_set_string(execute.PaymentId, root_object, "payment_id");
		
	if (execute.HasCard)
	{
		JSON_Value* cardJson = SerializeCard(execute.Card);
		json_object_set_value(root_object, "card", cardJson);
	}

	char *serialized = json_serialize_to_string(root_value);
	CString message(serialized);

	json_free_serialized_string(serialized);
	json_value_free(root_value);

	return message;
}

bool CLibertyXService::DeserializeExecuteResponse(CString data, LXExecuteResponse &response)
{
	JSON_Value *rootValue;
	JSON_Object *exec;

	int len = data.GetLength() + 1;
	char * json = new char[len]();
	WideToMulti(json, data, len);

	rootValue = json_parse_string(json);
	exec = json_value_get_object(rootValue);

	response.Message = CString(json_object_get_string(exec, "msg"));
	response.MessageCustomer = CString(json_object_get_string(exec, "msg_customer"));
	response.ErrorType = CString(json_object_get_string(exec, "error_type"));
	response.Status = CString(json_object_get_string(exec, "status"));

	memset(json, 0, len);
	delete [] json;
	json_value_free(rootValue);

	return true;
}

CString CLibertyXService::SerializePing(LXPing &ping)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);

	json_set_string(ping.LocationID, root_object, "location_id");
	json_set_string(ping.TID, root_object, "tid");
	json_set_string(ping.SoftwareVersion, root_object, "software_version");

	if (ping.HasDispenseInfo) 
	{
		JSON_Value* dispenseJson = SerializeDispensePingInformation(ping.DispenseInfo);
		json_object_set_value(root_object, "dispense", dispenseJson);
	}

	if (!ping.AtmTID.IsEmpty())
	{
		json_set_string(ping.AtmTID, root_object, "atmtid");
	}

	char *serialized = json_serialize_to_string(root_value);
	CString message(serialized);

	json_free_serialized_string(serialized);
	json_value_free(root_value);

	return message;
}

bool CLibertyXService::DeserializePingResponse(CString data, LXPingResponse &pong)
{
	JSON_Value *rootValue;
	JSON_Object *response;

	int len = data.GetLength() + 1;
	char * json = new char[len]();
	WideToMulti(json, data, len);

	rootValue = json_parse_string(json);
	response = json_value_get_object(rootValue);

	pong.Status = CString(json_object_get_string(response, "status"));
	pong.Message = CString(json_object_get_string(response, "msg"));
	pong.MessageCustomer = CString(json_object_get_string(response, "msg_customer"));

	memset(json, 0, len);
	delete [] json;
	json_value_free(rootValue);

	return true;
}

CString CLibertyXService::SerializeDispenseAuthorization(LXDispenseAuthorization &authorization)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);

	json_set_string(authorization.Currency, root_object, "currency");
	json_set_string(authorization.Token, root_object, "token");
	json_set_string(authorization.LocationId, root_object, "location_id");
	json_set_string(authorization.AuthorizationNonce, root_object, "authorize_nonce");
	json_set_string(authorization.MaxDispense, root_object, "max_dispense");

	json_object_set_boolean(root_object, "test", authorization.Test);

	json_set_cash_denoms_list(authorization.CashDenominations, "cash_denominations", root_object);

	char *serialized = json_serialize_to_string(root_value);
	CString message(serialized);

	json_free_serialized_string(serialized);
	json_value_free(root_value);

	return message;
}

bool CLibertyXService::DeserializeDispenseAuthorizationResponse(CString data, LXDispenseAuthorizationResponse &result)
{
	JSON_Value *rootValue;
	JSON_Object *response;

	int len = data.GetLength() + 1;
	char * json = new char[len]();
	WideToMulti(json, data, len);

	rootValue = json_parse_string(json);
	response = json_value_get_object(rootValue);

	result.Status = CString(json_object_get_string(response, "status"));
	result.Message = CString(json_object_get_string(response, "msg"));
	result.MessageCustomer = CString(json_object_get_string(response, "msg_customer"));
	result.AuthorizationNonce = CString(json_object_get_string(response, "authorize_nonce"));
	result.Amount = CString(json_object_get_string(response, "amount"));
	result.Test = json_object_get_boolean(response, "test") == 1;

	memset(json, 0, len);
	delete [] json;
	json_value_free(rootValue);

	return true;
}

CString CLibertyXService::SerializeDispenseExecute(LXDispenseExecute &execute)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);

	json_set_string(execute.AuthorizationNonce, root_object, "authorize_nonce");

	json_object_set_boolean(root_object, "test", execute.Test);

	char *serialized = json_serialize_to_string(root_value);
	CString message(serialized);

	json_free_serialized_string(serialized);
	json_value_free(root_value);

	return message;
}

bool CLibertyXService::DeserializeDispenseExecuteResponse(CString data, LXDispenseExecuteResponse &result)
{
	JSON_Value *rootValue;
	JSON_Object *response;

	int len = data.GetLength() + 1;
	char * json = new char[len]();
	WideToMulti(json, data, len);

	rootValue = json_parse_string(json);
	response = json_value_get_object(rootValue);

	result.Status = CString(json_object_get_string(response, "status"));
	result.Message = CString(json_object_get_string(response, "msg"));
	result.MessageCustomer = CString(json_object_get_string(response, "msg_customer"));
	result.AuthorizationNonce = CString(json_object_get_string(response, "authorize_nonce"));
	result.Test = json_object_get_boolean(response, "test") == 1;

	result.SignatureAlgorithm = CString(json_object_get_string(response, "signature_algorithm"));

	parse_cert_chain_array("certificate_chain_A", response, result.CertChainA);
	parse_cert_chain_array("certificate_chain_B", response, result.CertChainB);
	parse_cert_chain_array("certificate_chain_C", response, result.CertChainC);
	result.SignatureA = CString(json_object_get_string(response, "signature_A"));
	result.SignatureB = CString(json_object_get_string(response, "signature_B"));
	result.SignatureC = CString(json_object_get_string(response, "signature_C"));

	memset(json, 0, len);
	delete [] json;
	json_value_free(rootValue);

	return true;
}

CString CLibertyXService::SerializeDispenseCancel(LXDispenseCancel &cancel)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);

	json_set_string(cancel.AuthorizationNonce, root_object, "authorize_nonce");

	if (cancel.HasError)
	{
		json_set_string(cancel.Error.Code, root_object, "error.code");
		json_set_string(cancel.Error.Message, root_object, "error.message");

		if (cancel.Error.IsDispenseError)
		{
			json_set_string(cancel.Error.DispensedAmount, root_object, "error.data.dispensed");
		}
	}

	char *serialized = json_serialize_to_string(root_value);
	CString message(serialized);

	json_free_serialized_string(serialized);
	json_value_free(root_value);

	return message;
}

bool CLibertyXService::DeserializeDispenseCancelResponse(CString data, LXDispenseCancelResponse &result)
{
	JSON_Value *rootValue;
	JSON_Object *response;

	int len = data.GetLength() + 1;
	char * json = new char[len]();
	WideToMulti(json, data, len);

	rootValue = json_parse_string(json);
	response = json_value_get_object(rootValue);

	result.Status = CString(json_object_get_string(response, "status"));
	result.Message = CString(json_object_get_string(response, "msg"));
	result.MessageCustomer = CString(json_object_get_string(response, "msg_customer"));

	memset(json, 0, len);
	delete [] json;
	json_value_free(rootValue);

	return true;
}

CString CLibertyXService::SerializeDispenseTransactionReport(LXDispenseTransactionReport &report)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);

	json_set_string(report.AuthorizationNonce, root_object, "authorize_nonce");
	json_set_string(report.Status, root_object, "status");

	if (report.HasError)
	{
		json_set_string(report.Error.Code, root_object, "error.code");
		json_set_string(report.Error.Message, root_object, "error.message");
		
		if (report.Error.IsDispenseError)
		{
			json_set_string(report.Error.DispensedAmount, root_object, "error.data.dispensed");
		}
	}

	char *serialized = json_serialize_to_string(root_value);
	CString message(serialized);

	json_free_serialized_string(serialized);
	json_value_free(root_value);

	return message;
}

bool CLibertyXService::DeserializeDispenseTransactionReport(CString data, LXDispenseReportResponse &result)
{
	JSON_Value *rootValue;
	JSON_Object *response;

	int len = data.GetLength() + 1;
	char * json = new char[len]();
	WideToMulti(json, data, len);

	rootValue = json_parse_string(json);
	response = json_value_get_object(rootValue);

	result.Status = CString(json_object_get_string(response, "status"));

	memset(json, 0, len);
	delete [] json;
	json_value_free(rootValue);

	return true;
}
