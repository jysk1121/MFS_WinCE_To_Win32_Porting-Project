#ifndef __CPAYPALCCADATA_H__
#define __CPAYPALCCADATA_H__

// [#2446] US Justin 2016.09.29 Implement Paypal CCA
#if(APP_PAYDIANT_CCA) 

#define PAYPAL_FILE_LOGO_BUTTON			0

#define PAYPAL_SETTING_FILE				_T("PaypalCCAConfig.xml")
#define PAYPAL_DISPENSERESULT_FILE		_T("PaypalCCADispenseResult.dat")

#define PAYPAL_TRAN_NOTSTART			0
#define PAYPAL_TRAN_GETTOKEN			1
#define PAYPAL_TRAN_GETCUSTOMERINFO		2
#define PAYPAL_TRAN_GETWITHDRAWALTICKET	3
#define PAYPAL_TRAN_UPDATESURCHARGE		4
#define PAYPAL_TRAN_HOSTAPPROVAL		5
#define PAYPAL_TRAN_UPDATETRAN			6
#define PAYPAL_TRAN_HOSTREVERSAL		7

#define PAYPAL_TRAN_RELEASETOKEN		91
#define PAYPAL_TRAN_CANCEL_CUSINFO		92
#define PAYPAL_TRAN_CANCEL_WITHTICKET	93
//#define PAYPAL_TRAN_FULLREVERSAL		94

#define PAYPAL_UPDATETRAN_DENIED		0
#define PAYPAL_UPDATETRAN_DISPENSED		1

// [#2449] Justin Paydiant Online
#define UPDATE_CREDENTIAL_NONE			0x00
#define UPDATE_CREDENTIAL_KEY			0x01
#define UPDATE_CREDENTIAL_LOCATIONID	0x02
#define UPDATE_CREDENTIAL_PROVIDERID	0x04
#define UPDATE_CREDENTIAL_ATMID			0x08
#define UPDATE_CREDENTIAL_HOSTURL		0x10
#define UPDATE_CREDENTIAL_COMPLETE		0x1f
// End of [#2449]

// [#2522] US Justin 2017.12.18 Update Process Surcharge
#define PAYPAL_SURCHARGE_TYPE_ATM		0
#define PAYPAL_SURCHARGE_TYPE_PROCESSOR	1
// End of [#2522]

class AFX_EXT_CLASS CPaypalCCAData
{
public:
	CPaypalCCAData();
	virtual ~CPaypalCCAData();

	////////////////////////////////////////////////////////////
	// Setting Data
	BOOL	m_bPaypalCCAAvailable;
	CString m_strHostPort, m_strHostSSL, m_strHostProcess;	
	CString m_strPPLogo_Button;
	CString m_strATMKey, m_strLocationID, m_strProviderID, m_strATMID, m_strHostIP;
	CString m_strSoapHeader, m_strATMCredential;
	CString m_strDownATMKey, m_strDownLocationID, m_strDownProviderID, m_strDownATMID, m_strDownHostIP;		// [#2449] Justin Paydiant Online

	void	ResetSetting();
	BOOL	ReadSettingAndDispenseHistory();
	void	ResetDownloadCredential();								// [#2449] Justin Paydiant Online		
	BOOL	AssignPaydiantCredential(int nType, CString sValue);	// [#2449] Justin Paydiant Online
	BOOL	SaveDownloadPaydiantCredential();						// [#2449] Justin Paydiant Online
	CString	GetPaypalCCAFileName(int nType);

	////////////////////////////////////////////////////////////
	// Transaction Data
	BOOL	m_bUserCancel;											// [#2526] US Justin 2018.01.12 Paypal - Continue Transaction if User Cancel (Release token) Failure
	int		m_nTransactionStep;
	CString m_strAccessToken, m_strATMTrRefID, m_strPaydiantRefID;
	CString m_strCustomerInfo;
	CString m_strPaymentTender, m_strPaymentInstURi, m_strPaymentInstID;
	CString m_strCW_CardNum, m_strCW_CardExpDate; //, m_strCW_Currency;		// [#2466] US Justin 2017.01.18 Paydiant Certification. Ignore Currency Code
	CString m_strReqAmount, m_strATMSurcharge, m_strProcSurcharge, m_strTotalAmount, m_strTranDispensed;	// 40.00, 2.50, 1.50, 41.50, 20.00		// [#2522] US Justin 2017.10.18 Update Surcharge
	CString m_strPaydiantCode, m_strPaydiantDesc, m_strExternalCode, m_strExternalDesc, m_strErrorCode, m_strErrorDesc;

	void	ResetTransactionData();
	CString	MakeRequestMessage(LPCTSTR sExtraData);
	CString	MakeDemoResponseMessage();
	BOOL	ParseReceivedData(CString strReceived);
	BOOL	SetErrorCodeAndDesc(CString sResCode, CString strDesc = L"");	
	void	UpdateSurcharge(int nSurchargeAmt, int nSurchargeType);	// [#2522] US Justin 2017.12.18 Update Processor Surcharge
	void	UpdateExternalStatusCode(CString strCode, CString strATMCode, CString strATMCodeDesc);

	////////////////////////////////////////////////////////////
	// Day Total Data
	CString m_strPPDayTotalDateTime, m_strPPDayTotalDispCount, m_strPPDayTotalDispAmt;
	void	ResetTotal();
	BOOL	ReadDispenseHistory();
	BOOL	SaveDispenseResult(BOOL bReset=FALSE);
	void	RecodeDispenseResult(long nAmount /* Cent */);
	CString GetDaytotalJournalData();	

	////////////////////////////////////////////////////////////
	// Handling Data
	CString GetNodeContents(CString strSource, CString strFind, CString &strXMLNS, BOOL bMainNode=FALSE);
	CString GetAdditionalDataValue(CString strSource, CString strXMLNS, CString strKey);
};
#endif
// End of [#2446]

#endif //__CPAYPALCCADATA_H__