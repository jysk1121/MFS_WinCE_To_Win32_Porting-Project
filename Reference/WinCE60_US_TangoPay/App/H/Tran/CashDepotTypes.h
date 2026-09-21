#pragma once

#include "stdafx.h"

//
// Standard Flows
//

/**
 * GetConfigurations request
 */
typedef struct _cd_configurations
{
	// The bin
	CString Bin;

	// The timestamp
	CString Timestamp;
} CDConfigurations, *LPCDConfigurations;

/**
 * Response from the GetConfigurations request
 */
typedef struct _cd_configurations_response
{
	bool	RbiStatus;
	bool	CryptoStatus;
	double	Surcharges;
	double	AdminFees;
	int		MaxWithdrawalAmount;
	CString	AdvertisementId;

	_cd_configurations_response::_cd_configurations_response()
	{
		Reset();
	};

	void Reset()
	{
		RbiStatus = true;
		CryptoStatus = false;
		Surcharges = 0.0;
		AdminFees = 0.0;
		MaxWithdrawalAmount = 0;
		AdvertisementId = L"";
	};
} CDConfigurationsResponse, *LPCDConfigurationsResponse;

//
/// Cryptocurrency Flows
//

#define CD_ERROR_FUNCTION bool HasError() const { return StatusCode != CDR_OK || Errors.GetCount() > 0; };

/**
 * The response code type
 */
typedef enum
{
	CDR_None,
	CDR_OK = 200
} CDResponseType;

/**
 * Represents an error
 */
typedef struct _cd_error
{
	CString Code;
	CString Field;
	CString Detail;
	CString Error;

	_cd_error::_cd_error()
	{
		Reset();
	};

	void Reset()
	{
		Code.Empty();
		Field.Empty();
		Detail.Empty();
		Error.Empty();
	}
} CDError, *LPCDError;

typedef CList<CDError> CDErrors;

/**
 * CryptoResponseBase
 */
typedef struct _cd_crypto_base_response
{
	// 0 for success. Number greater than 1 for failure.
	int Code;

	// Empty for success. This field may contain error message on failure.
	CString Message;

	// Empty for success. This field may contain list of error message on failure.
	CDErrors Errors;

	_cd_crypto_base_response::_cd_crypto_base_response()
	{
		Reset();
	};

	CString GetMessage()
	{
		if (!Message.IsEmpty())
		{
			return Message;
		}
		else
		{
			if (Errors.GetCount() > 0)
			{
				POSITION first = Errors.FindIndex(0);
				CDError error = Errors.GetAt(first);
				return error.Detail;
			}
			else
			{
				return "";
			}
		}
	}

	CString GetError()
	{
		if (Errors.GetCount() > 0)
		{
			POSITION first = Errors.FindIndex(0);
			CDError error = Errors.GetAt(first);
			return error.Error;
		}

		return "";
	}

	void Reset()
	{
		Code = CDR_None;
		Message = L"";
		Errors.RemoveAll();
	}
} CDCryptoBaseResponse, *LPCDCryptoBaseResponse;

/**
 * Represents a crypto provider
 */
typedef struct _cd_provider
{
	CString Provider;
	CString ProviderCode;

	_cd_provider::_cd_provider()
	{
		Reset();
	};

	void Reset()
	{
		Provider.Empty();
		ProviderCode.Empty();
	}
} CDProvider, *LPCDProvider;

typedef CList<CDProvider> CDProviders;

/**
 * Response from the GetProviders request
 */
typedef struct _cd_providers_response : CDCryptoBaseResponse
{
	int StatusCode;
	CString StatusMessage;

	CList<CDProvider> Providers;

	_cd_providers_response::_cd_providers_response()
	{
	};

	CD_ERROR_FUNCTION
} CDProvidersResponse, *LPCDProvidersResponse;

/**
 * Client Token request
 */
typedef struct _cd_client_token_request
{
	// Service provider selected by the customer
	CString ServiceProvider;
} CDClientTokenRequest, *LPCDClientTokenRequest;

/**
 * Response from the Client Token request
 */
typedef struct _cd_client_token_response : CDCryptoBaseResponse
{
	int StatusCode;
	CString StatusMessage;

	CString AccessToken;

	_cd_client_token_response::_cd_client_token_response()
	{
	};

	CD_ERROR_FUNCTION
} CDClientTokenResponse, *LPCDClientTokenResponse;

/**
 * Send Code request
 */
typedef struct _cd_send_code_request
{
	// Client Token
	CString Token;

	// Terminal ID
	CString TerminalId;

	// Service provider selected by the customer
	CString ServiceProvider;

	// Phone Number
	CString Phone;
} CDSendCodeRequest, *LPCDSendCodeRequest;

/**
 * Response from the Send Code request
 */
typedef struct _cd_send_code_response : CDCryptoBaseResponse
{
	int StatusCode;
	CString StatusMessage;

	_cd_send_code_response::_cd_send_code_response()
	{
	};

	CD_ERROR_FUNCTION
} CDSendCodeResponse, *LPCDSendCodeResponse;

/**
 * Verify Code request
 */
typedef struct _cd_verify_code_request
{
	// Client Token
	CString Token;

	// Terminal ID
	CString TerminalId;

	// Service provider selected by the customer
	CString ServiceProvider;

	// Phone Number
	CString Phone;

	// Code
	CString Code;
} CDVerifyCodeRequest, *LPCDVerifyCodeRequest;

typedef struct _cd_customer_limits
{
	// Shows the customer limits for Tier one
	int TierOne;

	// Shows the customer limits for Tier two
	int TierTwo;

	// Shows the customer current limits
	double CurrentLimit;
} CDCustomerLimits, *LPCDCustomerLimits;

/**
 * Response from the Verify Code request
 */
typedef struct _cd_verify_code_response : CDCryptoBaseResponse
{
	int StatusCode;
	CString StatusMessage;

	// Jwt verification Token
	CString JWT;

	// Customer's Id
	CString CustomerId;

	// Display's terms of service for Coin Source
	CString TermsOfService;

	// Shows the customer limits
	CDCustomerLimits CustomerLimits;

	_cd_verify_code_response::_cd_verify_code_response()
	{
	};

	CD_ERROR_FUNCTION
} CDVerifyCodeResponse, *LPCDVerifyCodeResponse;

/**
 * Disclosure request
 */
typedef struct _cd_disclosure_request
{
	// Client Token
	CString Token;

	// Terminal ID
	CString TerminalId;

	// Service provider selected by the customer
	CString ServiceProvider;
} CDDisclosureRequest, *LPCDDisclosureRequest;

/**
 * Represents a disclosure
 */
typedef struct _cd_disclosure
{
	CString Id;

	_cd_disclosure::_cd_disclosure()
	{
		Reset();
	};

	void Reset()
	{
		Id.Empty();
	}
} CDDisclosure, *LPCDDisclosure;

typedef CList<CDDisclosure> CDDisclosures;

/**
 * Disclosure response
 */
typedef struct _cd_disclosure_response : CDCryptoBaseResponse
{
	int StatusCode;
	CString StatusMessage;

	// Disclosures
	CDDisclosures Disclosures;

	_cd_disclosure_response::_cd_disclosure_response()
	{
	};

	CD_ERROR_FUNCTION
} CDDisclosureResponse, *LPCDDisclosureResponse;

/**
 * Disclosure Accept request
 */
typedef struct _cd_disclosure_accept_request
{
	// Client Token
	CString Token;

	// Terminal ID
	CString TerminalId;

	// Service provider selected by the customer
	CString ServiceProvider;

	// User Token
	CString UserToken;

	// Customer's Id
	CString CustomerId;

	// Accepted Disclosures
	CDDisclosures Disclosures;
} CDDisclosureAcceptRequest, *LPCDDisclosureAcceptRequest;

/**
 * Disclosure Accept response
 */
typedef struct _cd_disclosure_accept_response : CDCryptoBaseResponse
{
	int StatusCode;
	CString StatusMessage;

	// Disclosure Token
	CString DisclosureToken;

	_cd_disclosure_accept_response::_cd_disclosure_accept_response()
	{
	};

	CD_ERROR_FUNCTION
} CDDisclosureAcceptResponse, *LPCDDisclosureAcceptResponse;

/**
 * Daily Limit request
 */
typedef struct _cd_daily_limit_request
{
	// Client Token
	CString Token;

	// Service provider selected by the customer
	CString ServiceProvider;

	// Disclosure Token
	CString DisclosureToken;
} CDDailyLimitRequest, *LPCDDailyLimitRequest;

/**
 * Daily Limit response
 */
typedef struct _cd_daily_limit_response : CDCryptoBaseResponse
{
	int StatusCode;
	CString StatusMessage;

	CString	Currency;
	int		Value;

	_cd_daily_limit_response::_cd_daily_limit_response()
	{
	};

	CD_ERROR_FUNCTION
} CDDailyLimitResponse, *LPCDDailyLimitResponse;

/**
 * Crypto Currency request
 */
typedef struct _cd_crypto_currency_request
{
	// Client Token
	CString Token;

	// Terminal ID
	CString TerminalId;

	// Service provider selected by the customer
	CString ServiceProvider;

	// Disclosure Token
	CString DisclosureToken;
} CDCryptoCurrencyRequest, *LPCDCryptoCurrencyRequest;

/**
 * Represents a cryptocurrency
 */
typedef struct _cd_crypto_currency
{
	CString	CoinType;
	CString	DisabledMessage;
	bool	BuyEnabled;
	bool	SellEnabled;
	int		DisabledMessageCode;
	bool	AllowConcurrentSellTransaction;
	CString	DisplayName;
	CString	Currency;

	_cd_crypto_currency::_cd_crypto_currency()
	{
		Reset();
	};

	void Reset()
	{
		CoinType.Empty();
		DisabledMessage.Empty();
		BuyEnabled = false;
		SellEnabled = false;
		DisabledMessageCode = 0;
		AllowConcurrentSellTransaction = false;
		DisplayName.Empty();
		Currency.Empty();
	}
} CDCryptoCurrency, *LPCDCryptoCurrency;

typedef CList<CDCryptoCurrency> CDCryptoCurrencies;

/**
 * Crypto Currency response
 */
typedef struct _cd_crypto_currency_response : CDCryptoBaseResponse
{
	int StatusCode;
	CString StatusMessage;

	CDCryptoCurrencies CryptoCurrencies;

	_cd_crypto_currency_response::_cd_crypto_currency_response()
	{
	};

	CD_ERROR_FUNCTION
} CDCryptoCurrencyResponse, *LPCDCryptoCurrencyResponse;

/**
 * Crypto Currency Price request
 */
typedef struct _cd_crypto_currency_price_request
{
	// Client Token
	CString Token;

	// Terminal ID
	CString TerminalId;

	// Service provider selected by the customer
	CString ServiceProvider;

	// Disclosure Token
	CString DisclosureToken;
} CDCryptoCurrencyPriceRequest, *LPCDCryptoCurrencyPriceRequest;

/**
 * Represents an error
 */
typedef struct _cd_rates
{
	CString Fiat;
	CString Crypto;

	_cd_rates::_cd_rates()
	{
		Reset();
	};

	void Reset()
	{
		Fiat.Empty();
		Crypto.Empty();
	}
} CDRates, *LPCDRates;

/**
 * Crypto Currency Price response
 */
typedef struct _cd_crypto_currency_price_response : CDCryptoBaseResponse
{
	int StatusCode;
	CString StatusMessage;

	CString	Id;
	CString	Currency;
	double	Price;
	int		ExpirationTime;
	CString	Pair;
	CDRates	Rates;
	CString CoinType;
	CString DisplayName;

	_cd_crypto_currency_price_response::_cd_crypto_currency_price_response()
	{
	};

	CD_ERROR_FUNCTION
} CDCryptoCurrencyPriceResponse, *LPCDCryptoCurrencyPriceResponse;

/**
 * Send Manual Wallet Link request
 */
typedef struct _cd_send_manual_wallet_link_request
{
	// Client Token
	CString Token;

	// Service provider selected by the customer
	CString ServiceProvider;

	// Disclosure Token
	CString DisclosureToken;

	// Coin Type
	CString CoinType;
} CDSendManualWalletLinkRequest, *LPCDSendManualWalletLinkRequest;

/**
 * Send Manual Wallet Link response
 */
typedef struct _cd_send_manual_wallet_link_response : CDCryptoBaseResponse
{
	int StatusCode;
	CString StatusMessage;

	_cd_send_manual_wallet_link_response::_cd_send_manual_wallet_link_response()
	{
	};

	CD_ERROR_FUNCTION
} CDSendManualWalletLinkResponse, *LPCDSendManualWalletLinkResponse;

/**
 * Get Manual Wallet Address request
 */
typedef struct _cd_get_manual_wallet_address_request
{
	// Client Token
	CString Token;

	// Disclosure Token
	CString DisclosureToken;
} CDGetManualWalletAddressRequest, *LPCDGetManualWalletAddressRequest;

/**
 * Get Manual Wallet Address response
 */
typedef struct _cd_get_manual_wallet_address_response : CDCryptoBaseResponse
{
	int StatusCode;
	CString StatusMessage;

	CString WalletAddress;

	_cd_get_manual_wallet_address_response::_cd_get_manual_wallet_address_response()
	{
	};

	CD_ERROR_FUNCTION
} CDGetManualWalletAddressResponse, *LPCDGetManualWalletAddressResponse;

/**
 * Check Manual Wallet Address request
 */
typedef struct _cd_check_manual_wallet_address_request
{
	// Client Token
	CString Token;

	// Service provider selected by the customer
	CString ServiceProvider;

	// Disclosure Token
	CString DisclosureToken;

	// Coin Type
	CString CoinType;

	// Wallet Address
	CString WalletAddress;
} CDCheckManualWalletAddressRequest, *LPCDCheckManualWalletAddressRequest;

/**
 * Check Manual Wallet Address response
 */
typedef struct _cd_check_manual_wallet_address_response : CDCryptoBaseResponse
{
	int StatusCode;
	CString StatusMessage;

	CString WalletAddress;

	_cd_check_manual_wallet_address_response::_cd_check_manual_wallet_address_response()
	{
	};

	CD_ERROR_FUNCTION
} CDCheckManualWalletAddressResponse, *LPCDCheckManualWalletAddressResponse;

/**
 * Buy request
 */
typedef struct _cd_buy_request
{
	// Client Token
	CString Token;

	// Terminal ID
	CString TerminalId;

	// Service provider selected by the customer
	CString ServiceProvider;

	// Disclosure Token
	CString DisclosureToken;

	// Coin Type
	CString CoinType;

	// Wallet Address
	CString WalletAddress;

	// Quote ID
	CString QuoteId;

	// Cash Total
	CString CashTotal;

	// Phone Number
	CString Phone;

	// Code
	CString Code;
} CDBuyRequest, *LPCDBuyRequest;

/**
 * Buy response
 */
typedef struct _cd_buy_response : CDCryptoBaseResponse
{
	int StatusCode;
	CString StatusMessage;

	CString WalletTransactionId;

	_cd_buy_response::_cd_buy_response()
	{
	};

	CD_ERROR_FUNCTION
} CDBuyResponse, *LPCDBuyResponse;

/**
 * Represents a Bill
 */
typedef struct _cd_bill
{
	CString	Name;
	int		Count;
	int		Denomination;
	int		RejectedCount;
	int		Total;
	int		CassetteIndex;
	CString	CassetteType;
	CString	IsoCurrencyCode;

	_cd_bill::_cd_bill()
	{
		Reset();
	};

	void Reset()
	{
		Name.Empty();
		Count = 0;
		Denomination = 0;
		RejectedCount = 0;
		Total = 0;
		CassetteIndex = 0;
		CassetteType.Empty();
		IsoCurrencyCode.Empty();
	}
} CDBill, *LPCDBill;

typedef CList<CDBill> CDBills;

/**
 * Buy Complete request
 */
typedef struct _cd_buy_complete_request
{
	// Client Token
	CString Token;

	// Terminal ID
	CString TerminalId;

	// Service provider selected by the customer
	CString ServiceProvider;

	// Disclosure Token
	CString DisclosureToken;

	// Wallet Address
	CString WalletAddress;

	// Wallet Transaction ID
	CString WalletTransactionId;

	// Quote ID
	CString QuoteId;

	// Cash Total
	CString CashTotal;

	// Code
	CString Code;

	// Bills
	CDBills Bills;
} CDBuyCompleteRequest, *LPCDBuyCompleteRequest;

/**
 * Represents a Receipt
 */
typedef struct _cd_receipt
{
	CString Total;
	CString Crypto;
	CString Fiat;
	CString Id;
	CString CryptoCurrency;
	CString Fee;
	CString NetworkFee;
	CString Timestamp;
	CDRates Rates;

	_cd_receipt::_cd_receipt()
	{
		Reset();
	};

	void Reset()
	{
		Total.Empty();
		Crypto.Empty();
		Fiat.Empty();
		Id.Empty();
		CryptoCurrency.Empty();
		Fee.Empty();
		NetworkFee.Empty();
		Timestamp.Empty();
		Rates.Reset();
	}
} CDReceipt, *LPCDReceipt;

/**
 * Buy Complete response
 */
typedef struct _cd_buy_complete_response : CDCryptoBaseResponse
{
	int StatusCode;
	CString StatusMessage;

	CDReceipt Receipt;

	_cd_buy_complete_response::_cd_buy_complete_response()
	{
	};

	CD_ERROR_FUNCTION
} CDBuyCompleteResponse, *LPCDBuyCompleteResponse;