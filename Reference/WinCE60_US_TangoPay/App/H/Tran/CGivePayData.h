#ifndef __CGIVEPAYDATA_H__
#define __CGIVEPAYDATA_H__

// [#2513] US Justin 2017.11.02 
#if(APP_GPAY_GIFTCARD_PURCHASE) 

#define GPAY_SERVICE_CARD_PURCHASE		0
#define GPAY_SERVICE_CARD_SELL			0x0001
#define GPAY_SERVICE_DONATION			0x0002
#define GPAY_SERVICE_ALL				0x0003

#define GPAY_DATA_FOLDER				 ATM_DATA_PATH _T("\\GivePay")

#define GPAY_SETTING_FILE				_T("GivePayConfig.xml")
#define GPAY_TRANRESULT_FILE			_T("GivePayTransactionResult.dat")

#define GPAY_TRAN_NOTSTART				0
#define GPAY_TRAN_GETTOKEN				1
#define GPAY_TRAN_DOWNLOAD_FEES			2
#define GPAY_TRAN_DOWNLOAD_CATEGORY		3
#define GPAY_TRAN_DOWNLOAD_CARDS		4
#define GPAY_TRAN_DOWNLOAD_GREETINGS	5
#define GPAY_TRAN_DOWNLOAD_WIRELESS		6

#define GPAY_TRAN_PURCHASECARD_STAGE	7
#define GPAY_TRAN_PURCHASECARD_APPROVAL	8
#define GPAY_TRAN_PURCHASECARD_REVERSAL	9

#define GPAY_TRAN_ENROLLMENT			10
#define GPAY_TRAN_DOWNLOAD_IMAGES		11

#define GPAY_ID_CATEGORY				1
#define GPAY_ID_SUBCATEGORY				2
#define GPAY_ID_CARDS					3
#define GPAY_ID_GREETINGS				4
#define GPAY_ID_WIRELESS				5

#define GPAY_SERVICE_NONE				0
#define GPAY_SERVICE_GIFTCARD			1
#define GPAY_SERVICE_WIRELESS			2

#define GPAY_PHONETYPE_SENDER			0
#define GPAY_PHONETYPE_RECEIPIENT		1
#define GPAY_PHONETYPE_WIRELESS			2

// [#2574] US Justin GivePay Enhancement3
#define	GPAY_DOWNLOAD_OP_ENROLL			0
#define GPAY_DOWNLOAD_TRANINFO			1
#define GPAY_DOWNLOAD_CMN_IMAGE			2
#define	GPAY_DOWNLOAD_ALL				10
// End of [#2547]

// [#2557] US Justin 2018.06.21
#define	GPAY_IMG_LOGO					0		// LOGO.png
#define	GPAY_IMG_OL_MAIN				1		// MM.png
#define	GPAY_IMG_OL_UPSELL				2		// UPSELL.png
#define	GPAY_IMG_OL_GC_CATEGORY			10		// GC_CAT.png
#define	GPAY_IMG_OL_GC_SUB_CATEGORY		11		// GC_SCAT.png
#define	GPAY_IMG_OL_GC_CARD_SELECTION	12		// GC_SEL.png
#define	GPAY_IMG_OL_GC_AMOUNT			13		// GC_AMT_INPUT.png
#define	GPAY_IMG_OL_GC_ORDER_CONFIRM	14		// GC_CRFM_PRCHS.png
#define	GPAY_IMG_OL_GC_SMS_FEE_CONFIRM	15		// GC_CRFM_SMS.png
#define	GPAY_IMG_OL_GC_DIGITAL_GREET	16		// GC_DG.png
#define	GPAY_IMG_OL_GC_RECIPENT_PHONE	17		// GC_REC_INPUT.png
#define	GPAY_IMG_OL_GC_SENDER_PHONE		18		// GC_SEND_INPUT.png
#define	GPAY_IMG_OL_WL_SELECTION		30		// WC_CHS.png
#define	GPAY_IMG_OL_WL_AMOUNT			31		// WC_AMT_INPUT.png
#define	GPAY_IMG_OL_WL_SMS_FEE_CONFIRM	32		// WC_CRFM_SMS.png
#define	GPAY_IMG_OL_WL_PHONE			33		// WC_MDN_INPUT.png
#define	GPAY_IMG_OL_ZIPCODE				40		// GC_ZIP.png
#define	GPAY_IMG_OL_THANK				41		// THANK.png
#define	GPAY_IMG_OL_WAIT				42		// WAIT.png
#define	GPAY_IMG_OL_TAC					43		// TAC.png
#define GPAY_IMG_OL_NEEDTIME			44		// NEEDTIME.png

#define	GPAY_IMG_ICON_SELECTED_CARD		50		// Selected card image
#define	GPAY_IMG_ICON_SELECTED_GREET	51		// Selected Greet image

#define	GPAY_IMG_ICON_GENERAL_GIFT		52		// General Gift Card Image		// [#2574] US Justin GivePay Enhancement3
#define	GPAY_IMG_ICON_GENERAL_WIRELESS	53		// General Wireless Card Image	// [#2574] US Justin GivePay Enhancement3
// End of [#2557]

#define GPAY_KEYIN_TIME_OUT				30		// [#2574] US Justin GivePay Enhancement3


class AFX_EXT_CLASS CGivePayData
{
public:
	CGivePayData();
	virtual ~CGivePayData();

	////////////////////////////////////////////////////////////
	// Setting Data
	BOOL	m_bGivePayAvailable, m_bGivePayEnrolled;
	CString m_strHostIP, m_strHostPort, m_strTaeURL;					// [#RWC6-44] GivePay AutoEnrollment QR Code Display
	CString m_strPSK, m_strClientID, m_strClientSecret, m_strTaeKey;	// [#RWC6-44] GivePay AutoEnrollment QR Code Display
	CString m_strGPSalt;	// Enrollment Variable

	void	ResetSetting();
	BOOL	ReadSettingAndDispenseHistory();
	BOOL	SaveSetting();

	CString GetGPImageFileName(int nID);										// [#2557] US Justin 2018.06.21

	////////////////////////////////////////////////////////////
	// Transaction Data
	int		m_nTransactionStep, m_nGPCategoryId, m_nGPServiceType;
	CString m_sLanguageExtension;
	CString m_sImageURL;

	CString	m_strGPToken, m_strGPTokenType, m_strGPTokenExpire;					// Get Token Variables	[#2557]
	CString m_strDataExpire, m_strNextDailyConfig;								// [#2557] US Justin 2018.06.12 Data Expiration	// [#2574]
	CString m_strCardFee, m_strWirelessFee, m_strGreetingFee;					// Fees - Configuration (Dollar : 20.00)
	CStringArray m_arrCategory, m_arrGreetings, m_arrWireless;					// Received Items

	CStringArray m_arrFullCardList;												// [#J002] US Justin 2018.10.25 Downloading Detailed Card List
	CStringArray m_arrSubCategory, m_arrCards;
	CString	m_strSelectedCard, m_strSelectedGreeting, m_strGPTranAmount;		// Selected Items (JSON STRING) and Transaction Amount(Unit:Cent. 2500 = $25.00)
	CString m_strGPPhoneNumRcv, m_strGPPhoneNumSnd, m_strGPZipCode;				// Entered 
	CString m_strGPHolderFName, m_strGPHolderLName, m_strGPPan, m_strGPCardExp;						// Card (Payment) information

	////////////////////////////////////////////////////////////
	// Staging Result.... UNIT $DD.CC (13.24)
	CString m_strStg_OrderId, m_strStg_ProductName;
	CStringArray m_arrStg_ExtraFee;
	CString m_strStg_UnitPrice, m_strStg_TaxAmt, m_strStg_FeeAmt, m_strStg_GifAmt,m_strStg_SubTotal, m_strStg_GrandTotal;
	CString m_strStg_Snd_FName, m_strStg_Snd_LName, m_strStg_Snd_FullName, m_strStg_Snd_Phone;
	CString m_strStg_Rcv_FName, m_strStg_Rcv_LName, m_strStg_Rcv_FullName, m_strStg_Rcv_Phone;

	// Auth. Result
	CString m_strAuth_TrDateTime;

	////////////////////////////////////////////////////////////
	// Error code and description
	CString m_strErrorCode, m_strErrorDesc;
	BOOL	m_bTempDG911; // TODO need this to be in flow // [#RWC6-1, #2583] US Brandon 2018.12.17 GivePay DG911 fix, WinATM error fix

	// Utility Functions
	CString ModifyPreSharedKey(CString sInput, BOOL bEncrypt);							// [#2566] NH Justin 2018.07.19 Encrypt GivePay 
	BOOL	GetKeyandIVFromSharedKeyAndSalt(CString strPSK, CString Salt, unsigned char*key, unsigned char*iv);
	CString EncryptTerminalID(CString strPSK, CString strSalt, CString strTID);
	CString DecryptCredential(CString strPSK, CString strSalt, CString strCredential);
	CString GenerateRandomUUID();
	CString MakeFullName(CString sFirstName, CString sLastName);

	BOOL	ParseAndFillGPItemArray(CString strItems, CStringArray* arrGPStr);
	BOOL	ParseAndFillGPSubCategory(int nCatID);
	int		FillGPButtonItems(int nType, int nPage, CString* strBtn, CString*strImage, int*nNextPage, int nMaxBtnNum=6);
	BOOL	FillGPCardFixedAmount(CString* strBtn, int nButtonNum);
	BOOL	GetGPCardMinMaxAmount(int& nMinAmt, int& nMaxAmt);
	BOOL	FillGPCardMinMaxAmount(CStringArray* arrAmtRange);										// [#2574]
	BOOL	CombineMultiRanges(CStringArray* arrAmtRange, CString* strBtn = NULL, int nBtnNum=0);	// [#2574]
	BOOL	IsAmountIncludedInRange(CStringArray* arrAmtRange, int nAmount);						// [#2574]
	CString GetAvailableAmountRange(CStringArray* arrAmtRange);										// [#2574]
	void	CollectCardInformation(int nCategoryID);												// [#J002]
	int		GetSelectedCardInformation(int nCategoryID);											// [#J002]

	// Transactional Functions
	BOOL	CanMakeGPTransaction(BOOL bPiggyBackTran);			// [#J006]
	void	ResetGPTransactionData(BOOL bEraseAll = FALSE);		// [#2557]
	BOOL	IsGPParameterOld();									// [#2557]
	void	UpdateGPParameterExpire(int nExpSec);				// [#2557]
	BOOL	NeedDailyUpdate();									// [#2574]
	void	UpdateGPNextDailyUpdateTime();						// [#2574]
	CString GetImageFileName(CString sImageURI);
	BOOL	ResetImageFiles();
	BOOL	IsImageFileExist(CString sImageURI);
	CString	MakeGPRequestMessage(CString strTerminalID);
	CString	MakeDemoResponseMessage();
	BOOL	ParseGPReceivedData(CString strReceived, BYTE* pBuffer, int nLength);
	void	SetErrorCodeAndDesc(CString sResCode, LPCTSTR sErrText=NULL);	

	// Make Transaction receipt or Journal
	CString MakeGPTransactionSummary(BOOL bSuccess, BOOL bPrint, int nMaxColumn);
	CString MakeGPTrJournalData(CString strKindCode);

	// Day Total Data
	CString m_strCPDayTotalDateTime, m_strCPDayTotalDispCount, m_strCPDayTotalDispAmt; // CENT !!  -- Card Purchase

	void	ResetTotal(int nService);
	BOOL	ReadDispenseHistory();
	BOOL	SaveDispenseResult(BOOL bReset=FALSE);
	void	RecodeDispenseResult(long nAmount /* Cent */, int nType);
	CString GetDaytotalJournalData(int nType);	
};
#endif
// End of [#2446]

#endif //__CGIVEPAYDATA_H__