#include "stdafx.h"
#include ".\Tran\CashDepotService.h"
#include ".\Common\NHDbgApi.h"

void cd_json_object_set_cstring(CString value, JSON_Object *root, const char *name)
{
	int len = value.GetLength() + 1;
	char *s = new char[len]();
	WideToMulti(s, value, len);
	json_object_dotset_string(root, name, s);
	delete[] s;
}

void cd_json_array_set_cstring(CString value, JSON_Array *root)
{
	int len = value.GetLength() + 1;
	char *s = new char[len]();
	WideToMulti(s, value, len);
	json_array_append_string(root, s);
	delete[] s;
}

bool cd_json_parse_response_data(CString data, JSON_Value **rootValue, JSON_Object **object)
{
	CStringA json(data);

	*rootValue = json_parse_string(json.GetBuffer());
	*object = json_value_get_object(*rootValue);

	return rootValue != NULL;
}

double cd_json_cstring_set_float(CString value)
{
	int len = value.GetLength() + 1;
	char *s = new char[len]();
	WideToMulti(s, value, len);
	double d = atof(s);
	delete[] s;

	return d;
}

//
// Standard Flows
//

CString CCashDepotService::SerializeConfigurations(const CDConfigurations &configs)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);

	cd_json_object_set_cstring(configs.Bin, root_object, "bin");
	cd_json_object_set_cstring(configs.Timestamp, root_object, "tTimeStamp");

	char *serialized = json_serialize_to_string(root_value);
	CString message(serialized);

	json_free_serialized_string(serialized);
	json_value_free(root_value);

	return message;
}

bool CCashDepotService::DeserializeConfigurationsResponse(CString data, CDConfigurationsResponse &response)
{
	JSON_Value *rootValue = NULL;
	JSON_Object *object = NULL;
	JSON_Object *dat = NULL;
	JSON_Object *config = NULL;

	if (!cd_json_parse_response_data(data, &rootValue, &object))
		return false;

	dat = json_object_get_object(object, "data");

	if (dat == NULL)
		return false;

	config = json_object_get_object(dat, "config");

	if (config == NULL)
		return false;

	response.RbiStatus = json_object_get_boolean(config, "rbiStatus");
	response.CryptoStatus = json_object_get_boolean(config, "cryptoStatus");
	response.Surcharges = (double)json_object_get_number(config, "surcharges");
	response.AdminFees = (double)json_object_get_number(config, "adminFees");
	response.MaxWithdrawalAmount = (int)json_object_get_number(config, "maxWithdrawalAmount");
	response.AdvertisementId = CString(json_object_get_string(config, "advertisementId"));

	json_value_free(rootValue);

	return true;
}

//
/// Cryptocurrency Flows
//

bool CCashDepotService::DeserializeProvidersArray(JSON_Array *providers, CDProviders &providerList)
{
	JSON_Object *object = NULL;
	CDProvider temp;

	if (providers == NULL)
		return false;

	size_t count = json_array_get_count(providers);

	for (size_t i = 0; i < count; i++)
	{
		object = json_array_get_object(providers, i);

		if (!DeserializeProvider(object, temp))
			continue;

		providerList.AddTail(temp);
	}

	return true;
}

bool CCashDepotService::DeserializeProvider(JSON_Object *object, CDProvider &provider)
{
	if (object == NULL)
		return false;

	provider.Provider = CString(json_object_get_string(object, "provider"));
	provider.ProviderCode = CString(json_object_get_string(object, "providerCode"));

	return true;
}

bool CCashDepotService::DeserializeErrorsArray(JSON_Array *errors, CDErrors &errorsList)
{
	JSON_Object *object = NULL;
	CDError temp;

	if (errors == NULL)
		return false;

	size_t count = json_array_get_count(errors);

	for (size_t i = 0; i < count; i++)
	{
		object = json_array_get_object(errors, i);

		if (!DeserializeError(object, temp))
			continue;

		errorsList.AddTail(temp);
	}

	return true;
}

bool CCashDepotService::DeserializeError(JSON_Object *object, CDError &error)
{
	if (object == NULL)
		return false;

	error.Code = CString(json_object_get_string(object, "code"));
	error.Field = CString(json_object_get_string(object, "field"));
	error.Detail = CString(json_object_get_string(object, "detail"));
	error.Error = CString(json_object_get_string(object, "error"));

	return true;
}

bool CCashDepotService::DeserializeDisclosuresArray(JSON_Array *disclosures, CDDisclosures &disclosuresList)
{
	JSON_Object *object = NULL;
	CDDisclosure temp;

	if (disclosures == NULL)
		return false;

	size_t count = json_array_get_count(disclosures);

	for (size_t i = 0; i < count; i++)
	{
		object = json_array_get_object(disclosures, i);
		
		if (!DeserializeDisclosure(object, temp))
			continue;

		disclosuresList.AddTail(temp);
	}

	return true;
}

bool CCashDepotService::DeserializeDisclosure(JSON_Object *object, CDDisclosure &disclosure)
{
	if (object == NULL)
		return false;

	disclosure.Id = CString(json_object_get_string(object, "id"));

	return true;
}

bool CCashDepotService::DeserializeCryptoCurrencyArray(JSON_Array *cryptoCurrencies, CDCryptoCurrencies &cryptoCurrenciesList, CString currency)
{
	JSON_Object *object = NULL;
	CDCryptoCurrency temp;

	if (cryptoCurrencies == NULL)
		return false;

	size_t count = json_array_get_count(cryptoCurrencies);

	for (size_t i = 0; i < count; i++)
	{
		object = json_array_get_object(cryptoCurrencies, i);
		
		if (!DeserializeCryptoCurrency(object, temp, currency))
			continue;

		cryptoCurrenciesList.AddTail(temp);
	}

	return true;
}

bool CCashDepotService::DeserializeCryptoCurrency(JSON_Object *object, CDCryptoCurrency &cryptoCurrency, CString currency)
{
	if (object == NULL)
		return false;

	CString coinType = CString(json_object_get_string(object, "coinType"));
	cryptoCurrency.CoinType = coinType;
	cryptoCurrency.DisabledMessage = CString(json_object_get_string(object, "disabledMessage"));
	cryptoCurrency.BuyEnabled = !!json_object_get_boolean(object, "buyEnabled");
	cryptoCurrency.SellEnabled = !!json_object_get_boolean(object, "sellEnabled");
	cryptoCurrency.DisabledMessageCode = (int)json_object_get_number(object, "disabledMessageCode");
	cryptoCurrency.AllowConcurrentSellTransaction = !!json_object_get_boolean(object, "allowConcurrentSellTransaction");
	cryptoCurrency.DisplayName = CString(json_object_get_string(object, "displayName"));
	cryptoCurrency.Currency.Format(L"%s-%s", coinType, currency);

	return true;
}

bool CCashDepotService::DeserializeCryptoCurrencyPricesArray(JSON_Array *dataArray, CDCryptoCurrencyPriceResponse &response, CString selectedCurrency)
{
	JSON_Object *object = NULL;

	if (dataArray == NULL)
		return false;

	size_t count = json_array_get_count(dataArray);

	for (size_t i = 0; i < count; i++)
	{
		object = json_array_get_object(dataArray, i);
		
		if (!DeserializeCryptoCurrencyPrice(object, response, selectedCurrency))
			return false;
	}

	return true;
}

bool CCashDepotService::DeserializeCryptoCurrencyPrice(JSON_Object *object, CDCryptoCurrencyPriceResponse &response, CString selectedCurrency)
{
	if (object == NULL)
		return false;

	CString pair = CString(json_object_get_string(object, "pair"));

	if (pair != selectedCurrency)
		return false;

	response.Id = CString(json_object_get_string(object, "id"));
	response.Currency = CString(json_object_get_string(object, "currency"));
	response.Price = (double)json_object_get_number(object, "price");
	response.ExpirationTime = (int)json_object_get_number(object, "expirationTime");
	response.Pair = pair;
	response.CoinType = CString(json_object_get_string(object, "coinType"));
	response.DisplayName = CString(json_object_get_string(object, "displayName"));
	
	if (!DeserializeCryptoCurrencyPriceRates(object, response))
		return false;

	return true;
}

bool CCashDepotService::DeserializeCryptoCurrencyPriceRates(JSON_Object *object, CDCryptoCurrencyPriceResponse &response)
{
	JSON_Object *obj = NULL;

	if (object == NULL)
		return false;

	obj = json_object_dotget_object(object, "rates");

	if (obj == NULL)
		return false;

	response.Rates.Fiat = CString(json_object_get_string(obj, "fiat"));
	response.Rates.Crypto = CString(json_object_get_string(obj, "crypto"));

	return true;
}

bool CCashDepotService::DeserializeProvidersResponse(CString data, CDProvidersResponse &response)
{
	bool success = false;
	JSON_Value *rootValue = NULL;
	JSON_Object *object = NULL;
	JSON_Array *providers = NULL;
	JSON_Array *errors = NULL;

	if (!cd_json_parse_response_data(data, &rootValue, &object))
		goto cleanup;

	response.Message = CString(json_object_get_string(object, "errorMessage"));
	response.Code = (int)json_object_get_number(object, "status");

	errors = json_object_get_array(object, "errors");

	if (errors == NULL)
		goto cleanup;

	if (!DeserializeErrorsArray(errors, response.Errors))
		goto cleanup;

	providers = json_object_get_array(object, "data");

	if (providers == NULL)
		goto cleanup;

	if (!DeserializeProvidersArray(providers, response.Providers))
		goto cleanup;

	success = true;

cleanup:

	if (rootValue != NULL)
		json_value_free(rootValue);

	return success;
}

CString CCashDepotService::SerializeClientToken(const CDClientTokenRequest &request)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);

	cd_json_object_set_cstring(request.ServiceProvider, root_object, "serviceProvider");

	char *serialized = json_serialize_to_string(root_value);
	CString message(serialized);

	json_free_serialized_string(serialized);
	json_value_free(root_value);

	return message;
}

bool CCashDepotService::DeserializeClientTokenResponse(CString data, CDClientTokenResponse &response)
{
	bool success = false;
	JSON_Value *rootValue = NULL;
	JSON_Object *object = NULL;
	JSON_Array *errors = NULL;

	if (!cd_json_parse_response_data(data, &rootValue, &object))
		goto cleanup;

	response.Message = CString(json_object_get_string(object, "errorMessage"));
	response.Code = (int)json_object_get_number(object, "status");

	errors = json_object_get_array(object, "errors");

	if (errors == NULL)
		goto cleanup;

	if (!DeserializeErrorsArray(errors, response.Errors))
		goto cleanup;

	response.AccessToken = CString(json_object_get_string(object, "accessToken"));

	success = true;

cleanup:

	if (rootValue != NULL)
		json_value_free(rootValue);

	return success;
}

CString CCashDepotService::SerializeSendCode(const CDSendCodeRequest &request)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);

	if (!request.Token.IsEmpty())
		cd_json_object_set_cstring(request.Token, root_object, "Token");

	if (!request.TerminalId.IsEmpty())
		cd_json_object_set_cstring(request.TerminalId, root_object, "atmId");

	if (!request.ServiceProvider.IsEmpty())
		cd_json_object_set_cstring(request.ServiceProvider, root_object, "ServiceProvider");

	if (!request.Phone.IsEmpty())
		cd_json_object_set_cstring(request.Phone, root_object, "phone");

	char *serialized = json_serialize_to_string(root_value);
	CString message(serialized);

	json_free_serialized_string(serialized);
	json_value_free(root_value);

	return message;
}

bool CCashDepotService::DeserializeSendCodeResponse(CString data, CDSendCodeResponse &response)
{
	bool success = false;
	JSON_Value *rootValue = NULL;
	JSON_Object *object = NULL;
	JSON_Array *errors = NULL;

	if (!cd_json_parse_response_data(data, &rootValue, &object))
		goto cleanup;

	response.Message = CString(json_object_get_string(object, "errorMessage"));
	response.Code = (int)json_object_get_number(object, "status");

	errors = json_object_get_array(object, "errors");

	if (errors == NULL)
		goto cleanup;

	if (!DeserializeErrorsArray(errors, response.Errors))
		goto cleanup;

	success = true;

cleanup:

	if (rootValue != NULL)
		json_value_free(rootValue);

	return success;
}

CString CCashDepotService::SerializeVerifyCode(const CDVerifyCodeRequest &request)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);

	if (!request.Token.IsEmpty())
		cd_json_object_set_cstring(request.Token, root_object, "token");

	if (!request.TerminalId.IsEmpty())
		cd_json_object_set_cstring(request.TerminalId, root_object, "atmId");

	if (!request.ServiceProvider.IsEmpty())
		cd_json_object_set_cstring(request.ServiceProvider, root_object, "serviceProvider");

	if (!request.Phone.IsEmpty())
		cd_json_object_set_cstring(request.Phone, root_object, "phone");

	if (!request.Code.IsEmpty())
		cd_json_object_set_cstring(request.Code, root_object, "code");

	char *serialized = json_serialize_to_string(root_value);
	CString message(serialized);

	json_free_serialized_string(serialized);
	json_value_free(root_value);

	return message;
}

bool CCashDepotService::DeserializeVerifyCodeResponse(CString data, CDVerifyCodeResponse &response)
{
	bool success = false;
	JSON_Value *rootValue = NULL;
	JSON_Object *object = NULL;
	JSON_Array *errors = NULL;
	JSON_Object *customerLimits = NULL;

	if (!cd_json_parse_response_data(data, &rootValue, &object))
		goto cleanup;

	response.Message = CString(json_object_get_string(object, "errorMessage"));
	response.Code = (int)json_object_get_number(object, "status");

	errors = json_object_get_array(object, "errors");

	if (errors == NULL)
		goto cleanup;

	if (!DeserializeErrorsArray(errors, response.Errors))
		goto cleanup;

	response.JWT = CString(json_object_get_string(object, "jwt"));
	response.CustomerId = CString(json_object_get_string(object, "customerId"));
	response.TermsOfService = CString(json_object_get_string(object, "termsOfService"));

	customerLimits = json_object_get_object(object, "customerLimits");

	if (customerLimits == NULL)
		goto cleanup;

	response.CustomerLimits.TierOne = (int)json_object_get_number(customerLimits, "tierOne");
	response.CustomerLimits.TierTwo = (int)json_object_get_number(customerLimits, "tierTwo");
	response.CustomerLimits.CurrentLimit = (double)json_object_get_number(customerLimits, "currentLimit");

	success = true;

cleanup:

	if (rootValue != NULL)
		json_value_free(rootValue);

	return success;
}

CString CCashDepotService::SerializeDisclosure(const CDDisclosureRequest &request)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);

	if (!request.Token.IsEmpty())
		cd_json_object_set_cstring(request.Token, root_object, "token");

	if (!request.TerminalId.IsEmpty())
		cd_json_object_set_cstring(request.TerminalId, root_object, "atmId");

	if (!request.ServiceProvider.IsEmpty())
		cd_json_object_set_cstring(request.ServiceProvider, root_object, "serviceProvider");

	char *serialized = json_serialize_to_string(root_value);
	CString message(serialized);

	json_free_serialized_string(serialized);
	json_value_free(root_value);

	return message;
}

bool CCashDepotService::DeserializeDisclosureResponse(CString data, CDDisclosureResponse &response)
{
	bool success = false;
	JSON_Value *rootValue = NULL;
	JSON_Object *object = NULL;
	JSON_Array *errors = NULL;
	JSON_Array *disclosures = NULL;

	if (!cd_json_parse_response_data(data, &rootValue, &object))
		goto cleanup;

	response.Message = CString(json_object_get_string(object, "errorMessage"));
	response.Code = (int)json_object_get_number(object, "status");

	errors = json_object_get_array(object, "errors");

	if (errors == NULL)
		goto cleanup;

	if (!DeserializeErrorsArray(errors, response.Errors))
		goto cleanup;

	disclosures = json_object_get_array(object, "data");

	if (disclosures == NULL)
		goto cleanup;

	if (!DeserializeDisclosuresArray(disclosures, response.Disclosures))
		goto cleanup;

	success = true;

cleanup:

	if (rootValue != NULL)
		json_value_free(rootValue);

	return success;
}

CString CCashDepotService::SerializeDisclosureAccept(const CDDisclosureAcceptRequest &request)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);
	JSON_Value *root_disclosures = json_value_init_array();
	JSON_Array *disclosures = json_value_get_array(root_disclosures);

	if (!request.Token.IsEmpty())
		cd_json_object_set_cstring(request.Token, root_object, "token");

	if (!request.TerminalId.IsEmpty())
		cd_json_object_set_cstring(request.TerminalId, root_object, "atmId");

	if (!request.ServiceProvider.IsEmpty())
		cd_json_object_set_cstring(request.ServiceProvider, root_object, "serviceProvider");

	if (!request.UserToken.IsEmpty())
		cd_json_object_set_cstring(request.UserToken, root_object, "userToken");

	if (!request.CustomerId.IsEmpty())
		cd_json_object_set_cstring(request.CustomerId, root_object, "customerId");

	size_t count = request.Disclosures.GetCount();

	if (count > 0)
	{
		for (size_t i = 0; i < count; i++)
		{
			POSITION position = request.Disclosures.FindIndex(i);
			CDDisclosure disclosure = request.Disclosures.GetAt(position);
			cd_json_array_set_cstring(disclosure.Id, disclosures);
		}
	}

	json_object_dotset_value(root_object, "disclosures", root_disclosures);

	char *serialized = json_serialize_to_string(root_value);
	CString message(serialized);

	json_free_serialized_string(serialized);
	json_value_free(root_value);

	return message;
}

bool CCashDepotService::DeserializeDisclosureAcceptResponse(CString data, CDDisclosureAcceptResponse &response)
{
	bool success = false;
	JSON_Value *rootValue = NULL;
	JSON_Object *object = NULL;
	JSON_Array *errors = NULL;

	if (!cd_json_parse_response_data(data, &rootValue, &object))
		goto cleanup;

	response.Message = CString(json_object_get_string(object, "errorMessage"));
	response.Code = (int)json_object_get_number(object, "status");

	errors = json_object_get_array(object, "errors");

	if (errors == NULL)
		goto cleanup;

	if (!DeserializeErrorsArray(errors, response.Errors))
		goto cleanup;

	response.DisclosureToken = CString(json_object_dotget_string(object, "disclosureToken"));

	success = true;

cleanup:

	if (rootValue != NULL)
		json_value_free(rootValue);

	return success;
}

CString CCashDepotService::SerializeDailyLimit(const CDDailyLimitRequest &request)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);

	if (!request.Token.IsEmpty())
		cd_json_object_set_cstring(request.Token, root_object, "token");

	if (!request.ServiceProvider.IsEmpty())
		cd_json_object_set_cstring(request.ServiceProvider, root_object, "serviceProvider");

	if (!request.DisclosureToken.IsEmpty())
		cd_json_object_set_cstring(request.DisclosureToken, root_object, "disclosureToken");

	char *serialized = json_serialize_to_string(root_value);
	CString message(serialized);

	json_free_serialized_string(serialized);
	json_value_free(root_value);

	return message;
}

bool CCashDepotService::DeserializeDailyLimitResponse(CString data, CDDailyLimitResponse &response)
{
	CString strTmp;
	bool success = false;
	JSON_Value *rootValue = NULL;
	JSON_Object *object = NULL;
	JSON_Array *errors = NULL;

	if (!cd_json_parse_response_data(data, &rootValue, &object))
		goto cleanup;

	response.Message = CString(json_object_get_string(object, "errorMessage"));
	response.Code = (int)json_object_get_number(object, "status");

	errors = json_object_get_array(object, "errors");

	if (errors == NULL)
		goto cleanup;

	if (!DeserializeErrorsArray(errors, response.Errors))
		goto cleanup;

	response.Currency = CString(json_object_get_string(object, "currency"));

	strTmp = CString(json_object_get_string(object, "value"));
	response.Value = cd_json_cstring_set_float(strTmp);

	success = true;

cleanup:

	if (rootValue != NULL)
		json_value_free(rootValue);

	return success;
}

CString CCashDepotService::SerializeCryptoCurrency(const CDCryptoCurrencyRequest &request)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);

	if (!request.Token.IsEmpty())
		cd_json_object_set_cstring(request.Token, root_object, "token");

	if (!request.TerminalId.IsEmpty())
		cd_json_object_set_cstring(request.TerminalId, root_object, "atmId");

	if (!request.ServiceProvider.IsEmpty())
		cd_json_object_set_cstring(request.ServiceProvider, root_object, "serviceProvider");

	if (!request.DisclosureToken.IsEmpty())
		cd_json_object_set_cstring(request.DisclosureToken, root_object, "disclosureToken");

	char *serialized = json_serialize_to_string(root_value);
	CString message(serialized);

	json_free_serialized_string(serialized);
	json_value_free(root_value);

	return message;
}

bool CCashDepotService::DeserializeCryptoCurrencyResponse(CString data, CDCryptoCurrencyResponse &response, CString currency)
{
	bool success = false;
	JSON_Value *rootValue = NULL;
	JSON_Object *object = NULL;
	JSON_Array *errors = NULL;
	JSON_Array *cryptoCurrencies = NULL;

	if (!cd_json_parse_response_data(data, &rootValue, &object))
		goto cleanup;

	response.Message = CString(json_object_get_string(object, "errorMessage"));
	response.Code = (int)json_object_get_number(object, "status");

	errors = json_object_get_array(object, "errors");

	if (errors == NULL)
		goto cleanup;

	if (!DeserializeErrorsArray(errors, response.Errors))
		goto cleanup;

	cryptoCurrencies = json_object_get_array(object, "data");

	if (cryptoCurrencies == NULL)
		goto cleanup;

	if (!DeserializeCryptoCurrencyArray(cryptoCurrencies, response.CryptoCurrencies, currency))
		goto cleanup;

	success = true;

cleanup:

	if (rootValue != NULL)
		json_value_free(rootValue);

	return success;
}

CString CCashDepotService::SerializeCryptoCurrencyPrice(const CDCryptoCurrencyPriceRequest &request)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);

	if (!request.Token.IsEmpty())
		cd_json_object_set_cstring(request.Token, root_object, "token");

	if (!request.TerminalId.IsEmpty())
		cd_json_object_set_cstring(request.TerminalId, root_object, "atmId");

	if (!request.ServiceProvider.IsEmpty())
		cd_json_object_set_cstring(request.ServiceProvider, root_object, "serviceProvider");

	if (!request.DisclosureToken.IsEmpty())
		cd_json_object_set_cstring(request.DisclosureToken, root_object, "disclosureToken");

	char *serialized = json_serialize_to_string(root_value);
	CString message(serialized);

	json_free_serialized_string(serialized);
	json_value_free(root_value);

	return message;
}

bool CCashDepotService::DeserializeCryptoCurrencyPriceResponse(CString data, CDCryptoCurrencyPriceResponse &response, CString selectedCurrency)
{
	bool success = false;
	JSON_Value *rootValue = NULL;
	JSON_Object *object = NULL;
	JSON_Array *errors = NULL;
	JSON_Array *dataArray = NULL;

	if (!cd_json_parse_response_data(data, &rootValue, &object))
		goto cleanup;

	response.Message = CString(json_object_get_string(object, "errorMessage"));
	response.Code = (int)json_object_get_number(object, "status");

	errors = json_object_get_array(object, "errors");

	if (errors == NULL)
		goto cleanup;

	if (!DeserializeErrorsArray(errors, response.Errors))
		goto cleanup;

	dataArray = json_object_get_array(object, "data");

	if (dataArray == NULL)
		goto cleanup;

	if (!DeserializeCryptoCurrencyPricesArray(dataArray, response, selectedCurrency))
		goto cleanup;

	success = true;

cleanup:

	if (rootValue != NULL)
		json_value_free(rootValue);

	return success;
}

CString CCashDepotService::SerializeSendManualWalletLink(const CDSendManualWalletLinkRequest &request)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);

	if (!request.Token.IsEmpty())
		cd_json_object_set_cstring(request.Token, root_object, "token");

	if (!request.ServiceProvider.IsEmpty())
		cd_json_object_set_cstring(request.ServiceProvider, root_object, "serviceProvider");

	if (!request.DisclosureToken.IsEmpty())
		cd_json_object_set_cstring(request.DisclosureToken, root_object, "disclosureToken");

	if (!request.CoinType.IsEmpty())
		cd_json_object_set_cstring(request.CoinType, root_object, "coinType");

	char *serialized = json_serialize_to_string(root_value);
	CString message(serialized);

	json_free_serialized_string(serialized);
	json_value_free(root_value);

	return message;
}

bool CCashDepotService::DeserializeSendManualWalletLinkResponse(CString data, CDSendManualWalletLinkResponse &response)
{
	bool success = false;
	JSON_Value *rootValue = NULL;
	JSON_Object *object = NULL;
	JSON_Array *errors = NULL;

	if (!cd_json_parse_response_data(data, &rootValue, &object))
		goto cleanup;

	response.Message = CString(json_object_get_string(object, "errorMessage"));
	response.Code = (int)json_object_get_number(object, "status");

	errors = json_object_get_array(object, "errors");

	if (errors == NULL)
		goto cleanup;

	if (!DeserializeErrorsArray(errors, response.Errors))
		goto cleanup;

	success = true;

cleanup:

	if (rootValue != NULL)
		json_value_free(rootValue);

	return success;
}

CString CCashDepotService::SerializeGetManualWalletAddress(const CDGetManualWalletAddressRequest &request)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);

	if (!request.Token.IsEmpty())
		cd_json_object_set_cstring(request.Token, root_object, "token");

	if (!request.DisclosureToken.IsEmpty())
		cd_json_object_set_cstring(request.DisclosureToken, root_object, "disclosureToken");

	char *serialized = json_serialize_to_string(root_value);
	CString message(serialized);

	json_free_serialized_string(serialized);
	json_value_free(root_value);

	return message;
}

bool CCashDepotService::DeserializeGetManualWalletAddressResponse(CString data, CDGetManualWalletAddressResponse &response)
{
	bool success = false;
	JSON_Value *rootValue = NULL;
	JSON_Object *object = NULL;
	JSON_Array *errors = NULL;

	if (!cd_json_parse_response_data(data, &rootValue, &object))
		goto cleanup;

	response.Message = CString(json_object_get_string(object, "errorMessage"));
	response.Code = (int)json_object_get_number(object, "status");

	errors = json_object_get_array(object, "errors");

	if (errors == NULL)
		goto cleanup;

	if (!DeserializeErrorsArray(errors, response.Errors))
		goto cleanup;

	response.WalletAddress = CString(json_object_get_string(object, "walletAddress"));

	success = true;

cleanup:

	if (rootValue != NULL)
		json_value_free(rootValue);

	return success;
}

CString CCashDepotService::SerializeCheckManualWalletAddress(const CDCheckManualWalletAddressRequest &request)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);

	if (!request.Token.IsEmpty())
		cd_json_object_set_cstring(request.Token, root_object, "token");

	if (!request.ServiceProvider.IsEmpty())
		cd_json_object_set_cstring(request.ServiceProvider, root_object, "serviceProvider");

	if (!request.DisclosureToken.IsEmpty())
		cd_json_object_set_cstring(request.DisclosureToken, root_object, "disclosureToken");

	if (!request.CoinType.IsEmpty())
		cd_json_object_set_cstring(request.CoinType, root_object, "coinType");

	if (!request.WalletAddress.IsEmpty())
		cd_json_object_set_cstring(request.WalletAddress, root_object, "walletAddress");

	char *serialized = json_serialize_to_string(root_value);
	CString message(serialized);

	json_free_serialized_string(serialized);
	json_value_free(root_value);

	return message;
}

bool CCashDepotService::DeserializeCheckManualWalletAddressResponse(CString data, CDCheckManualWalletAddressResponse &response)
{
	bool success = false;
	JSON_Value *rootValue = NULL;
	JSON_Object *object = NULL;
	JSON_Array *errors = NULL;

	if (!cd_json_parse_response_data(data, &rootValue, &object))
		goto cleanup;

	response.Message = CString(json_object_get_string(object, "errorMessage"));
	response.Code = (int)json_object_get_number(object, "status");

	errors = json_object_get_array(object, "errors");

	if (errors == NULL)
		goto cleanup;

	if (!DeserializeErrorsArray(errors, response.Errors))
		goto cleanup;

	response.WalletAddress = CString(json_object_get_string(object, "walletAddress"));

	success = true;

cleanup:

	if (rootValue != NULL)
		json_value_free(rootValue);

	return success;
}

CString CCashDepotService::SerializeBuy(const CDBuyRequest &request)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);

	if (!request.Token.IsEmpty())
		cd_json_object_set_cstring(request.Token, root_object, "token");

	if (!request.TerminalId.IsEmpty())
		cd_json_object_set_cstring(request.TerminalId, root_object, "atmId");

	if (!request.ServiceProvider.IsEmpty())
		cd_json_object_set_cstring(request.ServiceProvider, root_object, "serviceProvider");

	if (!request.DisclosureToken.IsEmpty())
		cd_json_object_set_cstring(request.DisclosureToken, root_object, "disclosureToken");

	if (!request.CoinType.IsEmpty())
		cd_json_object_set_cstring(request.CoinType, root_object, "coinType");

	if (!request.WalletAddress.IsEmpty())
		cd_json_object_set_cstring(request.WalletAddress, root_object, "walletAddress");

	if (!request.QuoteId.IsEmpty())
		cd_json_object_set_cstring(request.QuoteId, root_object, "quoteId");

	if (!request.CashTotal.IsEmpty())
		cd_json_object_set_cstring(request.CashTotal, root_object, "cashTotal");

	if (!request.Phone.IsEmpty())
		cd_json_object_set_cstring(request.Phone, root_object, "mobileNumber");

	if (!request.Code.IsEmpty())
		cd_json_object_set_cstring(request.Code, root_object, "code");

	char *serialized = json_serialize_to_string(root_value);
	CString message(serialized);

	json_free_serialized_string(serialized);
	json_value_free(root_value);

	return message;
}

bool CCashDepotService::DeserializeBuyResponse(CString data, CDBuyResponse &response)
{
	bool success = false;
	JSON_Value *rootValue = NULL;
	JSON_Object *object = NULL;
	JSON_Array *errors = NULL;

	if (!cd_json_parse_response_data(data, &rootValue, &object))
		goto cleanup;

	response.Message = CString(json_object_get_string(object, "errorMessage"));
	response.Code = (int)json_object_get_number(object, "status");

	errors = json_object_get_array(object, "errors");

	if (errors == NULL)
		goto cleanup;

	if (!DeserializeErrorsArray(errors, response.Errors))
		goto cleanup;

	response.WalletTransactionId = CString(json_object_get_string(object, "walletTransactionId"));

	success = true;

cleanup:

	if (rootValue != NULL)
		json_value_free(rootValue);

	return success;
}

CString CCashDepotService::SerializeBuyComplete(const CDBuyCompleteRequest &request)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);
	JSON_Value *root_bills = json_value_init_array();
	JSON_Array *bills = json_value_get_array(root_bills);

	if (!request.Token.IsEmpty())
		cd_json_object_set_cstring(request.Token, root_object, "token");

	if (!request.TerminalId.IsEmpty())
		cd_json_object_set_cstring(request.TerminalId, root_object, "atmId");

	if (!request.ServiceProvider.IsEmpty())
		cd_json_object_set_cstring(request.ServiceProvider, root_object, "serviceProvider");

	if (!request.DisclosureToken.IsEmpty())
		cd_json_object_set_cstring(request.DisclosureToken, root_object, "disclosureToken");

	if (!request.WalletAddress.IsEmpty())
		cd_json_object_set_cstring(request.WalletAddress, root_object, "walletAddress");

	if (!request.WalletTransactionId.IsEmpty())
		cd_json_object_set_cstring(request.WalletTransactionId, root_object, "walletTransactionId");

	if (!request.QuoteId.IsEmpty())
		cd_json_object_set_cstring(request.QuoteId, root_object, "quoteId");

	if (!request.CashTotal.IsEmpty())
		cd_json_object_set_cstring(request.CashTotal, root_object, "cashTotal");

	if (!request.Code.IsEmpty())
		cd_json_object_set_cstring(request.Code, root_object, "code");

	size_t count = request.Bills.GetCount();

	if (count > 0)
	{
		for (size_t i = 0; i < count; i++)
		{
			POSITION pos = request.Bills.FindIndex(i);
			CDBill cdBill = request.Bills.GetAt(pos);
			JSON_Value *root_bill = json_value_init_object();
			JSON_Object *bill = json_value_get_object(root_value);

			cd_json_object_set_cstring(cdBill.Name, bill, "name");
			json_object_set_number(bill, "count", (int)cdBill.Count);
			json_object_set_number(bill, "denomination", (int)cdBill.Denomination);
			json_object_set_number(bill, "rejectedCount", (int)cdBill.RejectedCount);
			json_object_set_number(bill, "total", (int)cdBill.Total);
			json_object_set_number(bill, "cassetteIndex", (int)cdBill.CassetteIndex);
			cd_json_object_set_cstring(cdBill.CassetteType, bill, "cassetteType");
			cd_json_object_set_cstring(cdBill.IsoCurrencyCode, bill, "isoCurrencyCode");

			json_array_append_value(bills, root_bill);
		}

		json_object_dotset_value(root_object, "bills", root_bills);
	}

	char *serialized = json_serialize_to_string(root_value);
	CString message(serialized);

	json_free_serialized_string(serialized);
	json_value_free(root_value);

	return message;
}

bool CCashDepotService::DeserializeBuyCompleteResponse(CString data, CDBuyCompleteResponse &response)
{
	bool success = false;
	JSON_Value *rootValue = NULL;
	JSON_Object *object = NULL;
	JSON_Array *errors = NULL;
	JSON_Object *receipt = NULL;
	JSON_Object *transaction = NULL;
	JSON_Object *rates = NULL;

	if (!cd_json_parse_response_data(data, &rootValue, &object))
		goto cleanup;

	response.Message = CString(json_object_get_string(object, "errorMessage"));
	response.Code = (int)json_object_get_number(object, "status");

	errors = json_object_get_array(object, "errors");

	if (errors == NULL)
		goto cleanup;

	if (!DeserializeErrorsArray(errors, response.Errors))
		goto cleanup;

	receipt = json_object_get_object(object, "receipt");

	if (receipt == NULL)
		return false;

	transaction = json_object_get_object(receipt, "transaction");

	if (transaction == NULL)
		return false;

	response.Receipt.Total = CString(json_object_get_string(transaction, "total"));
	response.Receipt.Crypto = CString(json_object_get_string(transaction, "crypto"));
	response.Receipt.Fiat = CString(json_object_get_string(transaction, "fiat"));
	response.Receipt.Id = CString(json_object_get_string(transaction, "id"));
	response.Receipt.CryptoCurrency = CString(json_object_get_string(transaction, "cryptoCurrency"));
	response.Receipt.Fee = CString(json_object_get_string(transaction, "fee"));
	response.Receipt.NetworkFee = CString(json_object_get_string(transaction, "networkFee"));
	response.Receipt.Timestamp = CString(json_object_get_string(receipt, "timestamp"));

	rates = json_object_get_object(receipt, "rates");

	if (rates == NULL)
		return false;

	response.Receipt.Rates.Fiat = CString(json_object_get_string(rates, "fiat"));
	response.Receipt.Rates.Crypto = CString(json_object_get_string(rates, "crypto"));

	success = true;

cleanup:

	if (rootValue != NULL)
		json_value_free(rootValue);

	return success;
}