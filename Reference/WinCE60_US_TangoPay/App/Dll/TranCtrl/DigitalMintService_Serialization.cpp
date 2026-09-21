#include "stdafx.h"
#include ".\Tran\DigitalMintService.h"
#include ".\Common\NHDbgApi.h"

void json_object_set_cstring(CString value, JSON_Object *root, const char *name)
{
	int len = value.GetLength() + 1;
	char *s = new char[len]();
	WideToMulti(s, value, len);
	json_object_dotset_string(root, name, s);
	delete[] s;
}

bool json_parse_response_data(CString data, JSON_Value **rootValue, JSON_Object **object)
{
	CStringA json(data);

	*rootValue = json_parse_string(json.GetBuffer());
	*object = json_value_get_object(*rootValue);

	return rootValue != NULL;
}

JSON_Value *CDigitalMintService::SerializeStateMap(DMStateMap map)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);

	if (map.HasBillState)
	{
		json_object_set_boolean(root_object, "bill_state", map.BillState);
	}

	if (map.HasScannerState)
	{
		json_object_set_boolean(root_object, "scanner_state", map.ScannerState);
	}

	if (map.HasCameraState)
	{
		json_object_set_boolean(root_object, "camera_state", map.CameraState);
	}

	if (map.HasOnline)
	{
		json_object_set_boolean(root_object, "online", map.Online);
	}

	return root_value;
}

bool CDigitalMintService::DeserializeProduct(JSON_Object *object, DMProduct &product)
{
	if (object == NULL)
	{
		return false;
	}

	product.Hash = CString(json_object_get_string(object, "Hash"));
	product.Symbol = CString(json_object_get_string(object, "Symbol"));
	product.Description = CString(json_object_get_string(object, "Description"));
	product.Markup = json_object_get_number(object, "Markup");

	return true;
}

bool CDigitalMintService::DeserializeProductArray(JSON_Array *products, CList<DMProduct> &productList)
{
	JSON_Object *object = NULL;
	DMProduct temp;
	if (products == NULL)
	{
		return false;
	}

	size_t count = json_array_get_count(products);
	for (size_t i = 0; i < count; i++)
	{
		object = json_array_get_object(products, i);
		if (!DeserializeProduct(object, temp))
		{
			continue;
		}

		productList.AddTail(temp);
	}

	return true;
}

CString CDigitalMintService::SerializeGreet(const DMGreeting &greeting)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);

	json_object_set_cstring(greeting.PhoneNumber, root_object, "PhoneNumber");

	if (!greeting.ConfirmCode.IsEmpty())
	{
		json_object_set_cstring(greeting.ConfirmCode, root_object, "ConfirmCode");
	}

	if (!greeting.Pin.IsEmpty())
	{
		json_object_set_cstring(greeting.Pin, root_object, "PIN");
	}

	char *serialized = json_serialize_to_string(root_value);
	CString message(serialized);

	json_free_serialized_string(serialized);
	json_value_free(root_value);

	return message;
}

CString CDigitalMintService::SerializeEnrollment(const DMEnrollmentRequest &enrollment)
{
	json_set_escape_slashes(0);
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);

	json_object_set_cstring(enrollment.PhoneNumber, root_object, "PhoneNumber");

	if (!enrollment.ConfirmCode.IsEmpty())
	{
		json_object_set_cstring(enrollment.ConfirmCode, root_object, "ConfirmCode");
	}

	if (!enrollment.FirstName.IsEmpty())
	{
		json_object_set_cstring(enrollment.FirstName, root_object, "FirstName");
	}

	if (!enrollment.LastName.IsEmpty())
	{
		json_object_set_cstring(enrollment.LastName, root_object, "LastName");
	}

	if (!enrollment.SSN.IsEmpty())
	{
		json_object_set_cstring(enrollment.SSN, root_object, "SSN");
	}

	if (!enrollment.Pin.IsEmpty())
	{
		json_object_set_cstring(enrollment.Pin, root_object, "PIN");
	}

	if (!enrollment.Gender.IsEmpty())
	{
		json_object_set_cstring(enrollment.Gender, root_object, "Gender");
	}

	if (!enrollment.DOB.IsEmpty())
	{
		json_object_set_cstring(enrollment.DOB, root_object, "DOB");
	}

	if (!enrollment.DocumentSerial.IsEmpty())
	{
		json_object_set_cstring(enrollment.DocumentSerial, root_object, "DocumentSerial");
	}

	if (!enrollment.DocumentExpiry.IsEmpty())
	{
		json_object_set_cstring(enrollment.DocumentExpiry, root_object, "DocumentExpiry");
	}

	if (!enrollment.DocumentIssue.IsEmpty())
	{
		json_object_set_cstring(enrollment.DocumentIssue, root_object, "DocumentIssue");
	}

	if (!enrollment.DocumentScanData.IsEmpty())
	{
		json_object_set_cstring(enrollment.DocumentScanData, root_object, "DocumentScanData");
	}

	if (!enrollment.DocumentFirstData.IsEmpty())
	{
		json_object_set_cstring(enrollment.DocumentFirstData, root_object, "DocumentFirstData");
	}

	if (!enrollment.Street.IsEmpty())
	{
		json_object_set_cstring(enrollment.Street, root_object, "Street");
	}

	if (!enrollment.City.IsEmpty())
	{
		json_object_set_cstring(enrollment.City, root_object, "City");
	}

	if (!enrollment.StateCode.IsEmpty())
	{
		json_object_set_cstring(enrollment.StateCode, root_object, "StateCode");
	}

	if (!enrollment.CountryCode.IsEmpty())
	{
		json_object_set_cstring(enrollment.CountryCode, root_object, "CountryCode");
	}

	char *serialized = json_serialize_to_string(root_value);
	CString message(serialized);

	json_free_serialized_string(serialized);
	json_value_free(root_value);
	json_set_escape_slashes(1);

	return message;
}

bool CDigitalMintService::DeserializeGreetResponse(CString data, DMGreetingResponse &response)
{
	JSON_Value *rootValue = NULL;
	JSON_Object *object = NULL;

	if (!json_parse_response_data(data, &rootValue, &object))
	{
		return false;
	}

	response.AccountHash = CString(json_object_get_string(object, "AccountHash"));
	response.SessionHash = CString(json_object_get_string(object, "SessionHash"));
	response.Message = CString(json_object_get_string(object, "Message"));
	response.Code = (int)json_object_get_number(object, "Code");

	json_value_free(rootValue);
	return true;
}

bool CDigitalMintService::DeserializeEnrollmentResponse(CString data, DMEnrollmentResponse &response)
{
	JSON_Value *rootValue = NULL;
	JSON_Object *object = NULL;

	if (!json_parse_response_data(data, &rootValue, &object))
	{
		return false;
	}

	response.AccountHash = CString(json_object_get_string(object, "AccountHash"));
	response.SessionHash = CString(json_object_get_string(object, "SessionHash"));
	response.Message = CString(json_object_get_string(object, "Message"));
	response.Code = (int)json_object_get_number(object, "Code");

	json_value_free(rootValue);
	return true;
}

bool CDigitalMintService::DeserializeAccountLimitsResponse(CString data, DMAccountLimitsResponse &response)
{
	JSON_Value *rootValue = NULL;
	JSON_Object *object = NULL;

	if (!json_parse_response_data(data, &rootValue, &object))
	{
		return false;
	}

	response.Code = (int)json_object_get_number(object, "Code");
	response.DailyTransactionLimit = (int)json_object_get_number(object, "DailyTransactionLimit");
	response.DailyVolume = (int)json_object_get_number(object, "DailyVolume");
	response.DailyLimitExceeded = !!json_object_get_boolean(object, "DailyLimitExceeded");
	response.MonthlyTransactionLimit = (int)json_object_get_number(object, "MonthlyTransactionLimit");
	response.MonthlyVolume = (int)json_object_get_number(object, "MonthlyVolume");
	response.MonthlyLimitExceeded = !!json_object_get_boolean(object, "MonthlyLimitExceeded");
	response.PerTransactionLimit = (int)json_object_get_number(object, "PerTransactionLimit");
	response.MaximumCashAllowed = (int)json_object_get_number(object, "MaximumCashAllowed");
	response.RemainingToHitMonthlyLimit = (int)json_object_get_number(object, "RemainingToHitMonthlyLimit");
	response.RemainingToHitDailyLimit = (int)json_object_get_number(object, "RemainingToHitDailyLimit");
	response.After7DaysCanPurchase = (int)json_object_get_number(object, "After7DaysCanPurchase");
	response.After14DaysCanPurchase = (int)json_object_get_number(object, "After14DaysCanPurchase");
	response.NextDayCanPurchase = CString(json_object_get_string(object, "NextDayCanPurchase"));
	response.NextDayCanPurchaseAmount = (int)json_object_get_number(object, "NextDayCanPurchaseAmount");

	json_value_free(rootValue);

	return true;
}

bool CDigitalMintService::DeserializeQuote(CString data, DMQuoteResponse &response)
{
	JSON_Value *rootValue = NULL;
	JSON_Object *object = NULL;

	if (!json_parse_response_data(data, &rootValue, &object))
	{
		return false;
	}

	response.Buy = CString(json_object_get_string(object, "Buy"));
	response.Sell = CString(json_object_get_string(object, "Sell"));
	response.Hash = CString(json_object_get_string(object, "Hash"));
	response.MaxQuantity = (int)json_object_get_number(object, "MaxQuantity");
	response.Expiry = CString(json_object_get_string(object, "Expiry"));
	response.Bid = CString(json_object_get_string(object, "Bid"));
	response.Ask = CString(json_object_get_string(object, "Ask"));

	json_value_free(rootValue);

	return true;
}

bool CDigitalMintService::DeserializeProductsResponse(CString data, DMProductsResponse &response)
{
	bool success = false;
	JSON_Value *rootValue = NULL;
	JSON_Object *object = NULL;
	JSON_Array *products = NULL;

	if (!json_parse_response_data(data, &rootValue, &object))
	{
		goto cleanup;
	}

	products = json_object_get_array(object, "Data");
	if (products == NULL)
	{
		goto cleanup;
	}

	if (!DeserializeProductArray(products, response.Products))
	{
		goto cleanup;
	}

	success = true;

cleanup:

	if (rootValue != NULL)
	{
		json_value_free(rootValue);
	}

	return success;
}

CString CDigitalMintService::SerializeIntiateTransactionRequest(const DMInitiateTransactionRequest &request)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);

	json_object_set_cstring(request.SessionHash, root_object, "SessionHash");
	json_object_set_cstring(request.Symbol, root_object, "Symbol");
	json_object_set_cstring(request.Side, root_object, "Side");

	char *serialized = json_serialize_to_string(root_value);
	CString message(serialized);

	json_free_serialized_string(serialized);
	json_value_free(root_value);

	return message;
}

bool CDigitalMintService::DeserializeTransactionStateResponse(CString data, DMTransactionResponse &response)
{
	JSON_Value *rootValue = NULL;
	JSON_Object *object = NULL;

	if (!json_parse_response_data(data, &rootValue, &object))
	{
		return false;
	}

	response.Code = (int)json_object_get_number(object, "Code");
	response.Status = CString(json_object_get_string(object, "Status"));
	response.Message = CString(json_object_get_string(object, "Message"));
	response.TransactionHash = CString(json_object_get_string(object, "TransactionHash"));
	response.Timestamp = CString(json_object_get_string(object, "Timestamp"));
	response.AccountHash = CString(json_object_get_string(object, "AccountHash"));
	response.SessionHash = CString(json_object_get_string(object, "SessionHash"));
	response.AccountName = CString(json_object_get_string(object, "AccountName"));
	response.Amount = json_object_get_number(object, "Amount");
	response.Product = CString(json_object_get_string(object, "Product"));
	response.Expiry = CString(json_object_get_string(object, "Expiry"));
	response.DefaultWallet = CString(json_object_get_string(object, "DefaultWallet"));
	response.WithdrawalAddress = CString(json_object_get_string(object, "WithdrawalAddress"));

	json_value_free(rootValue);

	return true;
}

bool CDigitalMintService::DeserializeQuoteResponse(CString data, DMQuoteResponse &response)
{
	JSON_Value *rootValue = NULL;
	JSON_Object *object = NULL;

	if (!json_parse_response_data(data, &rootValue, &object))
	{
		return false;
	}

	response.Code = (int)json_object_get_number(object, "Code");
	response.Message = CString(json_object_get_string(object, "Message"));
	response.Status = CString(json_object_get_string(object, "Status"));

	response.Buy = CString(json_object_get_string(object, "Buy"));
	response.Sell = CString(json_object_get_string(object, "Sell"));
	response.Hash = CString(json_object_get_string(object, "Hash"));
	response.MaxQuantity = (int)json_object_get_number(object, "MaxQuantity");
	response.Expiry = CString(json_object_get_string(object, "Expiry"));
	response.Bid = CString(json_object_get_string(object, "Bid"));
	response.Ask = CString(json_object_get_string(object, "Ask"));

	json_value_free(rootValue);

	return true;
}

CString CDigitalMintService::SerializeScanWalletRequest(const DMScanWalletRequest &request)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);

	json_object_set_cstring(request.WithdrawalAddress, root_object, "WithdrawalAddress");

	char *serialized = json_serialize_to_string(root_value);
	CString message(serialized);

	json_free_serialized_string(serialized);
	json_value_free(root_value);

	return message;
}

CString CDigitalMintService::SerializeCollectPaymentRequest(const DMCollectPaymentRequest &request)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);

	json_object_set_cstring(request.PaymentCollected, root_object, "PaymentCollected");
	json_object_set_boolean(root_object, "Execute", request.Execute);
	json_object_set_boolean(root_object, "Receipt", request.Receipt);

	char *serialized = json_serialize_to_string(root_value);
	CString message(serialized);

	json_free_serialized_string(serialized);
	json_value_free(root_value);

	return message;
}

CString CDigitalMintService::SerializeCompleteTransactionRequest(const DMCompleteTransactionRequest &request)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);

	json_object_set_boolean(root_object, "Execute", request.Execute);
	json_object_set_boolean(root_object, "Receipt", request.Receipt);

	char *serialized = json_serialize_to_string(root_value);
	CString message(serialized);

	json_free_serialized_string(serialized);
	json_value_free(root_value);

	return message;
}

CString CDigitalMintService::SerializeCancelTransactionRequest(const DMCancelTransactionRequest &request)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);

	json_object_set_number(root_object, "CancelReason", (int)request.CancelCode);

	char *serialized = json_serialize_to_string(root_value);
	CString message(serialized);

	json_free_serialized_string(serialized);
	json_value_free(root_value);

	return message;
}

CString CDigitalMintService::SerializeKioskStateRequest(const DMKioskStateRequest &request)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);

	JSON_Value *statemap = SerializeStateMap(request.StateMap);

	json_object_set_value(root_object, "StateMap", statemap);

	char *serialized = json_serialize_to_string(root_value);
	CString message(serialized);

	json_free_serialized_string(serialized);
	json_value_free(root_value);

	return message;
}

CString CDigitalMintService::SerializeKioskEventRequest(const DMKioskEventRequest &request)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);

	json_object_set_number(root_object, "Event", (int)request.Event);

	char *serialized = json_serialize_to_string(root_value);
	CString message(serialized);

	json_free_serialized_string(serialized);
	json_value_free(root_value);

	return message;
}

bool CDigitalMintService::DeserializeDeviceRegistrationResponse(CString data, DMDeviceRegistrationResponse &response)
{
	JSON_Value *rootValue = NULL;
	JSON_Object *object = NULL;

	if (!json_parse_response_data(data, &rootValue, &object))
	{
		return false;
	}

	response.Code = (int)json_object_get_number(object, "Code");
	response.Message = CString(json_object_get_string(object, "Message"));
	response.Status = CString(json_object_get_string(object, "Status"));
	response.ApiKey = CString(json_object_get_string(object, "APIKey"));
	response.ApiSecret = CString(json_object_get_string(object, "APISecret"));
	response.PlatformHash = CString(json_object_get_string(object, "PlatformHash"));
	response.Environment = CString(json_object_get_string(object, "Environment"));

	json_value_free(rootValue);

	return true;
}

bool CDigitalMintService::DeserializeVersionResponse(CString data, DMVersionResponse &response)
{
	JSON_Value *rootValue = NULL;
	JSON_Object *object = NULL;

	if (!json_parse_response_data(data, &rootValue, &object))
	{
		return false;
	}

	response.Version = CString(json_object_get_string(object, "Version"));

	json_value_free(rootValue);

	return true;
}