#pragma once

#include "stdafx.h"

#define ERROR_FUNCTION bool HasError() const { return Code != DMR_None && (Code < 200 || Code >= 300); };

/**
 * The response code type
 */
typedef enum
{
	DMR_None = 0,

	DMR_OK = 200,

	// Greeting OK. Kiosk should use the greeting API to confirm the customer's phone number
	DMR_GreetingOK = 220,

	// Visually identify customer against provided driver's license
	DMR_VisualDLConfirmationRequired = 221,

	// Customer account exists, but PIN authentication is required for this platform, and the customer has not yet set a PIN. Customer will receive a SMS confirmation code and should enter the confirmation code at the kiosk to confirm their phone number.
	DMR_ConfirmationCodeRequired = 222,

	// Existing customer account confirmation code authentication successful, customer should be prompted to create a PIN.
	DMR_CreatePIN = 223,

	// Customer account exists, PIN validation is required. Customer should be prompted to enter their PIN at the kiosk to verify their account.
	DMR_PINRequired = 224,

	// Account has been created, but is still pending approval and may not yet transact.
	DMR_AccountPendingApproval = 226,

	// Customer must proceed with Web Enrollment before purchasing
	DMR_WebEnrollmentRequired = 228,

	// Visually confirm customer's ID matches DigitalMint information on file
	DMR_VisualIDConfirmationRequired = 229,

	// HTTP Bad Request
	DMR_BadRequest = 400,

	// The credentials are not correct, or the PIN is incorrect
	DMR_Unauthorize = 401,

	// Transaction denied. Customer is restricted to transact.
	DMR_CustomerRestricted = 406,

	// Unprocessable entity (on the enroll this means the account is a duplicate)
	DMR_UnprocessableEntity = 422,

	// Product not found. Invalid symbol or product at location
	DMR_ProductNotFound = 460,

	// Product not active. Product exists but is not enabled at platform
	DMR_ProductNotActive = 461,

	// Platform not active. Platform exists but is not enabled
	DMR_PlatformNotActive = 462,

	// Invalid session. Session exists but desired operation is forbidden in this session
	DMR_InvalidSession = 463,

	// Transaction denied. Customer has reached their daily limit.
	DMR_DailyLimitExceeded = 470,

	// Transaction denied. Customer is not yet approved to transact.
	DMR_CustomerNotApproved = 471,

	// Scan denied. Invalid wallet address. Please re-submit a scan API call with a new wallet address
	DMR_InvalidWalletAddress = 480,

	// Customer has exceeded risk tolerance
	DMR_ExceededRiskTolerance = 530,

	// Customer has reached their daily risk limit
	DMR_ExceededRiskLimit = 531,

	// Customer has reached their individual transaction limit
	DMR_ExceededTransactionLimit = 532,

	// Transaction is in a terminal state
	DMR_TransactionTerminal = 540,

	// Transaction has already been canceled
	DMR_TransactionCancelled = 570
} DMResponseType;

/**
 * Reasons for canceling a transaction
 */
typedef enum
{
	DMCC_CustomerChangedMind = 0,
	DMCC_CustomerEnrollmentIssue = 1,
	DMCC_CustomerQuestionable = 2,
	DMCC_CustomerSigningOut = 3,

	// Use for debit declines
	DMCC_Other = 4,
	DMCC_Training = 10,
	DMCC_LargerPurchaseDesired = 11,
	DMCC_DefaultWalletIncorrect = 15
} DMCancelTransactionReasonCode;

typedef CString DMTransactionSide;
const DMTransactionSide DMSIDE_BUY	= L"buy";
const DMTransactionSide DMSIDE_SELL	= L"sell";

/**
 * Kiosk/ATM+Sidecar HW Events
 */
typedef enum
{
	DMEC_MaintenanceModeEnter,
	DMEC_MaintenanceModeExit,
	DMEC_CashCollection,
	DMEC_KioskRebooted = 6,
	DMEC_CashCassetteRemoved = 10,
	DMEC_CashCassetteInserted,
	DMEC_KioskHomescreen = 18
} DMKioskEventCode;

/**
 * Greeting request
 */
typedef struct _dm_greeting
{

	// Mobile phone number associated with the account
	CString PhoneNumber;

	// The PIN associated with the account
	CString Pin;

	// The code from the customer's MFA
	CString ConfirmCode;

} DMGreeting, *LPDMGreeting;

/**
 * Response from the Greeting request
 */
typedef struct _dm_greeting_response
{
	int		Code;
	CString	Message;
	CString	AccountHash;
	CString	SessionHash;

	_dm_greeting_response::_dm_greeting_response()
	{
		Code = DMR_None;
	};

	ERROR_FUNCTION

} DMGreetingResponse, *LPDMGreetingResponse;

/**
 * Response from the Enrollment request
 */
typedef struct _dm_enrollment_response
{
	int		Code;
	CString	Message;
	CString	AccountHash;
	CString	SessionHash;

	_dm_enrollment_response::_dm_enrollment_response()
	{
		Code = DMR_None;
	};

	ERROR_FUNCTION

} DMEnrollmentResponse, *LPDMEnrollmentResponse;

/**
 * Enrollment request
 */
typedef struct _dm_enrollment
{
	// Mobile phone number associated with the account
	CString PhoneNumber;

	// The code from the customer's MFA
	CString ConfirmCode;

	// customer first name
	CString FirstName;

	// customer last name
	CString LastName;

	// optional SSN to apply for higher limits. higher limit approval is not instant.
	CString SSN;

	// 4-digit customer PIN
	CString Pin;

	// customer's gender, "m" for male or "f" for female
	CString Gender;

	// customer date of birth in MM/DD/YYYY format
	CString DOB;

	// government document serial number
	CString DocumentSerial;

	// government document expiry date in MM/DD/YYYY format
	CString DocumentExpiry;

	// government document issue date in MM/DD/YYYY format
	CString DocumentIssue;

	// raw barcode scan data
	CString DocumentScanData;

	// base64-encoded Data URI
	CString DocumentFirstData;

	// customer's street address on government document
	CString Street;

	// customer's city on government document
	CString City;

	// two-digit state code, e.g. "IL"
	CString StateCode;

	// two-digit country code, "US" only
	CString CountryCode;
} DMEnrollmentRequest, *LPDMEnrollmentRequest;

/**
 * Account limits information
 */
typedef struct _dm_account_limits_response
{
	int		Code;
	CString	Message;
	CString	Status;

	int		DailyTransactionLimit;
	int		DailyVolume;
	bool	DailyLimitExceeded;

	int		MonthlyTransactionLimit;
	int		MonthlyVolume;
	bool	MonthlyLimitExceeded;

	int		PerTransactionLimit;
	int		MaximumCashAllowed;
	int		RemainingToHitMonthlyLimit;
	int		RemainingToHitDailyLimit;
	int		After7DaysCanPurchase;
	int		After14DaysCanPurchase;
	CString	NextDayCanPurchase;
	int		NextDayCanPurchaseAmount;

	_dm_account_limits_response::_dm_account_limits_response() : NextDayCanPurchase()
	{
		Code = DMR_None;

		DailyTransactionLimit = 0;
		DailyVolume = 0;
		DailyLimitExceeded = false;

		MonthlyTransactionLimit = 0;
		MonthlyVolume = 0;
		MonthlyLimitExceeded = false;

		PerTransactionLimit = 0;
		MaximumCashAllowed = 0;
		RemainingToHitMonthlyLimit = 0;
		RemainingToHitDailyLimit = 0;
		After7DaysCanPurchase = 0;
		After14DaysCanPurchase = 0;
		NextDayCanPurchaseAmount = 0;
	};

	ERROR_FUNCTION

	void Reset()
	{
		Code = DMR_None;
		Message.Empty();
		Status.Empty();

		DailyTransactionLimit = 0;
		DailyVolume = 0;
		DailyLimitExceeded = false;

		MonthlyTransactionLimit = 0;
		MonthlyVolume = 0;
		MonthlyLimitExceeded = false;

		PerTransactionLimit = 0;
		MaximumCashAllowed = 0;
		RemainingToHitMonthlyLimit = 0;
		RemainingToHitDailyLimit = 0;
		After7DaysCanPurchase = 0;
		After14DaysCanPurchase = 0;
		NextDayCanPurchase.Empty();
		NextDayCanPurchaseAmount = 0;
	};

} DMAccountLimitsResponse, *LPDMAccountLimitsResponse;

/**
 * Response containing quotation information for a purchase
 */
typedef struct _dm_quote_response
{
	int		Code;
	CString	Message;
	CString	Status;

	CString	Buy;
	CString	Sell;
	CString	Hash;
	int		MaxQuantity;
	CString	Expiry;
	CString	Bid;
	CString	Ask;

	_dm_quote_response::_dm_quote_response()
	{
		Code = DMR_None;
		MaxQuantity = 0;
	};

	ERROR_FUNCTION

	void Reset()
	{
		Code = DMR_None;
		Message.Empty();
		Status.Empty();

		Buy.Empty();
		Sell.Empty();
		Hash.Empty();
		MaxQuantity = 0;
		Expiry.Empty();
		Bid.Empty();
		Ask.Empty();
	};

} DMQuoteResponse, *LPDMQuoteResponse;

/**
 * Represents a product available for purchase
 */
typedef struct _dm_product
{
	CString	Hash;
	CString	Symbol;
	bool	Live;
	CString	Description;
	double	Markup;
	bool	Buy;
	bool	Sell;

	_dm_product::_dm_product()
	{
		Live = false;
		Markup = 0;
		Buy = false;
		Sell = false;
	};

	void Reset()
	{
		Hash.Empty();
		Symbol.Empty();
		Live = false;
		Description.Empty();
		Markup = 0;
		Buy = false;
		Sell = false;
	}

} DMProduct, *LPDMProduct;

typedef CList<DMProduct> DMProducts;

/**
 * Products response
 */
typedef struct _dm_products_response
{
	int Code;
	CString Message;
	CString Status;

	DMProducts Products;

	_dm_products_response::_dm_products_response()
	{
		Code = 0;
	}

	ERROR_FUNCTION

} DMProductsResponse, *LPDMProductsResponse;

/**
 * Information representing a transaction
 */
typedef struct _dm_initiate_transaction_request
{
	CString SessionHash;
	CString Symbol;
	DMTransactionSide Side;
} DMInitiateTransactionRequest, *LPDMInitiateTransactionRequest;

/**
 * Information representing the state of the transaction
 */
typedef struct _dm_transaction_response
{
	int		Code;
	CString	Message;
	CString	Status;
	int		State;

	CString	TransactionHash;
	CString	TransactionID;
	CString	Timestamp;
	CString	AccountHash;
	CString	SessionHash;
	CString	AccountName;
	double	Price;
	double	Amount;
	CString	Product;
	CString	Expiry;
	CString	DefaultWallet; // DMTODO: Is this used?
	CString	WithdrawalAddress;

	_dm_transaction_response::_dm_transaction_response()
	{
		Code = DMR_None;
		State = 0;
		Price = 0.0;
		Amount = 0.0;
	};

	ERROR_FUNCTION

} DMTransactionResponse, *LPDMTransactionResponse;

/**
 * Represents a wallet address confirmation
 */
typedef struct _dm_scan_wallet_request
{
	CString WithdrawalAddress;

} DMScanWalletRequest, *LPDMScanWalletRequest;

/**
 * Represents payment collection information
 */
typedef struct _dm_collect_payment_request
{
	// Payment amount collected from the customer, usually dollars
	CString PaymentCollected;

	// Execute (complete) transaction with payment collection. Optional flag
	bool Execute;

	// Customer requests a text message receipt. Optional flag
	bool Receipt;

	_dm_collect_payment_request::_dm_collect_payment_request()
	{
		Execute = false;
		Receipt = false;
	};

} DMCollectPaymentRequest, *LPDMCollectPaymentRequest;

/**
 * Completes the transaction
 */
typedef struct _dm_complete_transaction_request
{
	bool Execute;
	bool Receipt;

	_dm_complete_transaction_request::_dm_complete_transaction_request()
	{
		Execute = false;
		Receipt = false;
	};
} DMCompleteTransactionRequest, *LPDMCompleteTransactionRequest;

/**
 * Represents a canceled transaction
 */
typedef struct _dm_cancel_transaction_request
{
	DMCancelTransactionReasonCode CancelCode;

	_dm_cancel_transaction_request::_dm_cancel_transaction_request()
	{
		CancelCode = DMCC_CustomerChangedMind;
	};

} DMCancelTransactionRequest, *LPDMCancelTransactionRequest;

/**
 * Maps of the Kiosk/ATM+Sidecar hardware statuses
 */
typedef struct _dm_statemap
{
	bool BillState;
	bool HasBillState;

	bool ScannerState;
	bool HasScannerState;

	bool CameraState;
	bool HasCameraState;

	bool Online;
	bool HasOnline;

	_dm_statemap::_dm_statemap()
	{
		BillState = false;
		HasBillState = false;
		ScannerState = false;
		HasScannerState = false;
		CameraState = false;
		HasCameraState = false;
		Online = false;
		HasOnline = false;
	}
} DMStateMap, *LPDMStateMap;

/**
 * A request that updates the Kiosk/ATM+Sidecar HW status
 */
typedef struct _dm_kiosk_state_request
{
	DMStateMap StateMap;

	_dm_kiosk_state_request::_dm_kiosk_state_request() : StateMap()
	{
	}

} DMKioskStateRequest, *LPDMKioskStateRequest;

/**
 * A request that updates the Kiosk/ATM+Sidecar HW event
 */
typedef struct _dm_kiosk_event_request
{
	DMKioskEventCode	Event;
	CString				ImageData;

	_dm_kiosk_event_request::_dm_kiosk_event_request()
	{
		Event = DMEC_KioskHomescreen;
		ImageData.Empty();
	}

} DMKioskEventRequest, *LPDMKioskEventRequest;

/**
 * Response for the device registration process
 */
typedef struct _dm_device_reg_response
{
	int		Code;
	CString	Message;
	CString	Status;

	CString	ApiKey;
	CString	ApiSecret;
	CString	PlatformHash;
	CString	Environment;

	_dm_device_reg_response::_dm_device_reg_response()
	{
		Code = DMR_None;
	};

	ERROR_FUNCTION

} DMDeviceRegistrationResponse, *LPDMDeviceRegistrationResponse;

/**
 * Response for the version route
 */
typedef struct _dm_version_response
{
	CString Version;

	_dm_version_response::_dm_version_response(){};

} DMVersionResponse, *LPDMVersionResponse;