#include "StdAfx.h"
#include ".\..\..\H\Tran\CPayPalCCAData.h"
#include ".\..\..\H\Common\Markup.h"

#include <parson/parson.h>			// [RWC6-117] Update JSON implementation
#pragma comment(lib, "parson.lib")	// [RWC6-117] Update JSON implementation

//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------

//#define NH_DEBUG		// Debug Message On/Off

#include ".\..\..\H\Common\NHDbgApi.h"

#define	DBG_CALL		1
#define DBG_INFO		1

// [#2446] US Justin 2016.09.29 Paypal CCA
#if(APP_PAYDIANT_CCA)

 CPaypalCCAData::CPaypalCCAData()
{
	ResetSetting();
	ResetTransactionData();
	ResetDownloadCredential();		// [#2449] Justin Paydiant Online
}

CPaypalCCAData::~CPaypalCCAData()
{
}

////////////////////////////////////////////////////////////
// Setting Function
void CPaypalCCAData::ResetSetting()
{
	m_strHostPort = L"443";						// [#2449] Assign Default Value	
	m_strHostSSL = L"1";						// [#2449] Assign Default Value
	m_strPPLogo_Button = L"PaypalLogo.jpg";		// [#2449] Assign Default Value
	//m_strHostProcess = L"atm-gateway";		// [#2449] Assign Default Value

	m_bPaypalCCAAvailable = FALSE;
	m_strATMKey = m_strLocationID = m_strProviderID = m_strATMID = m_strHostIP = m_strHostProcess = _T("");
	m_strSoapHeader = m_strATMCredential = _T("");
	m_strPPDayTotalDateTime = m_strPPDayTotalDispCount = m_strPPDayTotalDispAmt = L"";
}

BOOL CPaypalCCAData::ReadSettingAndDispenseHistory()
{
	NHDEBUG(DBG_INFO, (L"m_bPaypalCCAAvailable::ReadSettingAndDispenseHistory()\n") );
	m_bPaypalCCAAvailable = FALSE;

	CString strPath;
	CFile cf;

	///////////////////////////////////////////////////////////////
	// Read Setting
	///////////////////////////////////////////////////////////////
	strPath.Format(L"%s\\%s", ATM_DATA_PATH, PAYPAL_SETTING_FILE);
	if( cf.Open( strPath, CFile::modeRead | CFile::typeText) == FALSE )
	{
		NHDEBUG(DBG_INFO, (L"[FAIL TO OPEN(%s)]\n", strPath));
		return FALSE;
	}
	int nFileLen = (int)cf.GetLength();

	unsigned char* pBuffer = new unsigned char[nFileLen + 2];				// Allocate buffer for binary file data
	if( pBuffer == NULL )
	{
		NHDEBUG(DBG_INFO, (L"[FAIL TO NEW BUFFER(%d)]\n", nFileLen + 2));
		cf.Close();
		return FALSE;
	}

	nFileLen = cf.Read( pBuffer, nFileLen );
	cf.Close();

	pBuffer[nFileLen] = '\0';
	pBuffer[nFileLen+1] = '\0';												// in case 2-byte encoded
	CString strXML(_T(""));
	if ( pBuffer[0] == 0xFF && pBuffer[1] == 0xFE )							// Windows Unicode file is detected if starts with FEFF	
	{
		strXML = (LPCWSTR)(&pBuffer[2]);
		NHDEBUG(DBG_CALL, (L"[File starts with hex FFFE, assumed to be wide char format.]\n"));
	}
	else
	{
		strXML = (LPCSTR)pBuffer;
	}
	delete [] pBuffer;

	if ( strXML.GetLength() < nFileLen / 2 - 20 )							// If it is too short, assume it got truncated due to non-text content	
	{
		NHDEBUG(DBG_INFO, (L"[Error converting file to string (may contain binary data)]\n"));
		return FALSE;
	}

	CMarkup xml;
	if( xml.SetDoc( strXML ) == FALSE )
	{
		NHDEBUG(DBG_INFO, (L"[FAIL TO PARSE XML]\n"));
		return FALSE;
	}
	if( xml.FindChildElem(_T("Setting")) == FALSE )
	{
		NHDEBUG(DBG_INFO, (L"[FAIL TO FIND <Setting>]\n"));
		return FALSE;
	}
	if( xml.IntoElem() == FALSE )
	{
		NHDEBUG(DBG_INFO, (L"[FAIL TO INTO <Setting>]\n"));
		return FALSE;
	}
	while( xml.FindChildElem() )
	{
		CString strName = xml.GetChildTagName();
		if( strName.IsEmpty() == TRUE )
			continue;
		CString strChildData = xml.GetChildData();
		NHDEBUG(DBG_INFO, (L" XML ChildNode [%s] = [%s]\n", strName, strChildData ));
		if(		 strName.CompareNoCase(L"HostIP") == 0 )				m_strHostIP = strChildData;
		else if( strName.CompareNoCase(L"Process") == 0 )				m_strHostProcess = strChildData;			
		else if( strName.CompareNoCase(L"ATMKey") == 0 )				m_strATMKey = strChildData;			
		else if( strName.CompareNoCase(L"ATMLocationID") == 0 )			m_strLocationID = strChildData;			
		else if( strName.CompareNoCase(L"ATMProviderID") == 0 )			m_strProviderID = strChildData;			
		else if( strName.CompareNoCase(L"ATMID") == 0 )					m_strATMID = strChildData;			
	}

	if( xml.OutOfElem() == FALSE )
	{
		NHDEBUG(DBG_INFO, (L"[FAIL OUT OF <Setting>]\n"));
		ResetSetting();
		return FALSE;
	}

	if( (m_strHostIP.GetLength()>0)	&& (m_strATMKey.GetLength()>0)	&& (m_strLocationID.GetLength()>0) && (m_strProviderID.GetLength()>0) && (m_strATMID.GetLength()>0) )
	{
		// Make ATM Credential
		CString strTemp;
		strTemp.Format(L"<atm:atmId>%s</atm:atmId>\r\n",m_strATMID);						m_strATMCredential  = strTemp;
		strTemp.Format(L"<atm:atmKey>%s</atm:atmKey>\r\n",m_strATMKey);						m_strATMCredential += strTemp;
		strTemp.Format(L"<atm:atmLocationId>%s</atm:atmLocationId>\r\n",m_strLocationID);	m_strATMCredential += strTemp;
		strTemp.Format(L"<atm:atmProviderId>%s</atm:atmProviderId>\r\n",m_strProviderID);	m_strATMCredential += strTemp;
		NHDEBUG(DBG_INFO, (L"ATM Credential = \r\n%s\n", m_strATMCredential));

		// Make Soap common Header
		m_strSoapHeader = L"<?xml version=\"1.0\"?>\r\n";
		m_strSoapHeader += L"<soapenv:Envelope xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:atm=\"http://www.paydiant.com/atm-gateway\">\r\n";
		m_strSoapHeader += L"<soapenv:Header/>\r\n";
		m_strSoapHeader += L"<soapenv:Body>\r\n";
		NHDEBUG(DBG_INFO, (L"SOAP COMMON HEADER = \r\n%s", m_strSoapHeader));

		m_bPaypalCCAAvailable = TRUE;
		NHDEBUG(DBG_INFO, (L"Read config file OK\n"));
	}
	else
	{
		NHDEBUG(DBG_INFO, (L"[One or more paremeters are missing]\n"));
		ResetSetting();
		return FALSE;
	}
	return ReadDispenseHistory();
}

// [#2449] Justin Paydiant Online
void CPaypalCCAData::ResetDownloadCredential()
{
	NHDEBUG(DBG_INFO, (L"PayPal ResetDownloadCredential()\n") );
	m_strDownATMKey = m_strDownLocationID = m_strDownProviderID = m_strDownATMID = m_strDownHostIP = L"";
}

BOOL CPaypalCCAData::AssignPaydiantCredential(int nType, CString sValue)
{
	NHDEBUG(DBG_INFO, (L"PayPal AssignPaydiantCredential(%x, %s)\n", nType, sValue) );
	if		(nType == UPDATE_CREDENTIAL_KEY)			m_strDownATMKey = sValue;
	else if (nType == UPDATE_CREDENTIAL_LOCATIONID)		m_strDownLocationID = sValue;
	else if (nType == UPDATE_CREDENTIAL_PROVIDERID)		m_strDownProviderID = sValue;
	else if (nType == UPDATE_CREDENTIAL_ATMID)			m_strDownATMID = sValue;
	else if (nType == UPDATE_CREDENTIAL_HOSTURL)		m_strDownHostIP = sValue;
	else if (nType == UPDATE_CREDENTIAL_COMPLETE)
	{
		BOOL bSaveCred = FALSE;
		if( SaveDownloadPaydiantCredential() == TRUE )
		{
			if( ReadSettingAndDispenseHistory() )
			{
				bSaveCred = TRUE;
				NHDEBUG(DBG_INFO, (L"PAYPAL DOWNLOAD SETTING OK\n") );
			}
		}
		ResetDownloadCredential();

		if(bSaveCred)			
		{
			NHDEBUG(DBG_INFO, (L"PAYPAL DOWNLOAD SETTING OK\n") );
			return TRUE;
		}

		NHDEBUG(DBG_INFO, (L"!!!!!!!!!! PAYPAL DOWNLOAD SETTING ERROR !!!!!!!!!!\n") );
		return FALSE;
	}
	return TRUE;
}

BOOL CPaypalCCAData::SaveDownloadPaydiantCredential()
{
	NHDEBUG(DBG_INFO, (L"PayPal SavePaydiantCredential()\n") );
	if( (m_strDownATMKey.GetLength()<=0) || (m_strDownLocationID.GetLength()<=0) || (m_strDownProviderID.GetLength()<=0) || 
		(m_strDownATMID.GetLength()<=0)  || (m_strDownHostIP.GetLength()<=0) )
	{
		NHDEBUG(DBG_INFO, (L"Incomplete Informatio... RETURN FALSE\n") );
		return FALSE;
	}

	CString strData, strPath;
	CFile cf;

	strPath.Format(L"%s\\%s", ATM_DATA_PATH, PAYPAL_SETTING_FILE);
	if( cf.Open( strPath, CFile::modeWrite | CFile::modeCreate ) == FALSE  )
	{
		NHDEBUG(DBG_INFO, (L"[FAIL TO OPEN(%s)]\n", strPath));
		return FALSE;
	}

	// Remove https:// or http://
	int nLoc= m_strDownHostIP.Find(L"://");
	if(nLoc>=0)
		m_strDownHostIP = m_strDownHostIP.Mid(nLoc+3);


	// Find Process from IP
	CString strHostProcess = L"";
	nLoc = m_strDownHostIP.Find(L"/");
	if(nLoc>0)
	{
		strHostProcess = m_strDownHostIP.Mid(nLoc+1);
		m_strDownHostIP = m_strDownHostIP.Left(nLoc);
	}

	strData  = L"<PaypalCCA>\r\n";
	strData += L"<Setting>\r\n";
	strData += L"<ATMKey>"			+ m_strDownATMKey		+ L"</ATMKey>\r\n";
	strData += L"<ATMLocationID>"	+ m_strDownLocationID	+ L"</ATMLocationID>\r\n";
	strData += L"<ATMProviderID>"	+ m_strDownProviderID	+ L"</ATMProviderID>\r\n";
	strData += L"<ATMID>"			+ m_strDownATMID		+ L"</ATMID>\r\n";
	strData += L"<HostIP>"			+ m_strDownHostIP		+ L"</HostIP>\r\n";
	strData += L"<Process>"			+ strHostProcess		+ L"</Process>\r\n";
	strData += L"</Setting>\r\n";
	strData += L"</PaypalCCA>\r\n";
	NHDEBUG(DBG_INFO, (L"Paypal Setting =\n%s", strData) );

	BYTE pBuffer[4096];
	memset(pBuffer, NULL, 4096);
	WideToMulti((LPSTR)pBuffer, strData, strData.GetLength());
	cf.Write( pBuffer, strData.GetLength() );
	cf.Close();

	NHDEBUG(DBG_INFO, (L"PayPal SavePaydiantCredential() OK    RETURN TRUE\n") );
	return TRUE;
}
// End of [#2449]

CString CPaypalCCAData::GetPaypalCCAFileName(int nType)
{
	CString sRtn = L"";
	if(nType==PAYPAL_FILE_LOGO_BUTTON)
		sRtn.Format(L"%s\\%s",ATM_DATA_PATH, m_strPPLogo_Button);

	return sRtn;	
}

////////////////////////////////////////////////////////////
// Transaction Function
void CPaypalCCAData::ResetTransactionData()
{
	m_bUserCancel = FALSE;											// [#2526] US Justin 2018.01.12 Paypal - Continue Transaction if User Cancel (Release token) Failure
	m_nTransactionStep = PAYPAL_TRAN_NOTSTART;
	m_strAccessToken = m_strATMTrRefID = m_strPaydiantRefID = _T("");
	m_strCustomerInfo = _T("");
	m_strPaymentTender = m_strPaymentInstURi = m_strPaymentInstID = _T("");
	//m_strCW_CardNum = m_strCW_CardExpDate = m_strCW_Currency = _T("");
	m_strCW_CardNum = m_strCW_CardExpDate = _T("");	// [#2466] US Justin 2017.01.18 Paydiant Certification. Ignore Currency Code
	m_strReqAmount = m_strATMSurcharge = m_strProcSurcharge = m_strTotalAmount = m_strTranDispensed = _T("");		// [#2522] US Justin 
	m_strPaydiantCode = m_strPaydiantDesc = m_strExternalCode = m_strExternalDesc = m_strErrorCode = m_strErrorDesc = _T("");
}

CString	CPaypalCCAData::MakeRequestMessage(LPCTSTR sExtraData)
{
	// Request Message Header
	CString strTemp;
	CString strSendBody = m_strSoapHeader;

	// Request Type
	if      ( m_nTransactionStep == PAYPAL_TRAN_GETTOKEN)				strSendBody += L"<atm:ObtainCashAccessTokenRequest>\r\n";
	else if ( m_nTransactionStep == PAYPAL_TRAN_GETCUSTOMERINFO)		strSendBody += L"<atm:RetrieveTransactionMetadataRequest>\r\n";
	else if ( m_nTransactionStep == PAYPAL_TRAN_GETWITHDRAWALTICKET)	strSendBody += L"<atm:RetrieveTransactionMetadataRequest>\r\n";
	else if ( m_nTransactionStep == PAYPAL_TRAN_UPDATESURCHARGE)		strSendBody += L"<atm:UpdateTransactionRequest>\r\n";
	else if ( m_nTransactionStep == PAYPAL_TRAN_UPDATETRAN)				strSendBody += L"<atm:UpdateTransactionRequest>\r\n";
	else if ( m_nTransactionStep == PAYPAL_TRAN_RELEASETOKEN)			strSendBody += L"<atm:ReleaseCashAccessTokenRequest>\r\n";
	else if ( m_nTransactionStep == PAYPAL_TRAN_CANCEL_CUSINFO)			strSendBody += L"<atm:UpdateTransactionRequest>\r\n";
	else if ( m_nTransactionStep == PAYPAL_TRAN_CANCEL_WITHTICKET)		strSendBody += L"<atm:UpdateTransactionRequest>\r\n";

	//else if ( m_nTransactionStep == PAYPAL_TRAN_FULLREVERSAL)			strSendBody += L"<atm:UpdateTransactionRequest>\r\n";

	// ATM Credential
	strSendBody += m_strATMCredential;
	
	// Access Token
	if ( m_nTransactionStep != PAYPAL_TRAN_GETTOKEN )
	{
		strTemp.Format(L"<atm:cashAccessToken>%s</atm:cashAccessToken>\r\n",m_strAccessToken);
		strSendBody += strTemp;
	}

	//Paydiant Transaction Ref. ID
	if( (m_nTransactionStep == PAYPAL_TRAN_GETWITHDRAWALTICKET)	|| (m_nTransactionStep == PAYPAL_TRAN_UPDATESURCHARGE)	|| 
		(m_nTransactionStep == PAYPAL_TRAN_UPDATETRAN)			|| (m_nTransactionStep == PAYPAL_TRAN_CANCEL_CUSINFO)	|| 
		(m_nTransactionStep == PAYPAL_TRAN_CANCEL_WITHTICKET)   )
	{
		strTemp.Format(L"<atm:paydiantTransactionReferenceId>%s</atm:paydiantTransactionReferenceId>\r\n",m_strPaydiantRefID);
		strSendBody += strTemp;
	}

	// Atm Transaction Ref. ID
	if(	(m_nTransactionStep == PAYPAL_TRAN_UPDATESURCHARGE)		|| (m_nTransactionStep == PAYPAL_TRAN_UPDATETRAN)		||
		(m_nTransactionStep == PAYPAL_TRAN_CANCEL_WITHTICKET)	)
	{
		strTemp.Format(L"<atm:atmTransactionReferenceId>%s</atm:atmTransactionReferenceId>\r\n",m_strATMTrRefID);
		strSendBody += strTemp;
	}

	// [#2522] US Justin 
	BOOL bIsSurchargeChanged = FALSE;
	//if( m_strATMSurcharge != m_strProcSurcharge )
	if(Dollar2Cent(m_strATMSurcharge) != Dollar2Cent(m_strProcSurcharge) )
		bIsSurchargeChanged = TRUE;
	// End of [#2522]
	
	////////////////////////////////////////////////////////////////////////////////
	// Main Message
	if(m_nTransactionStep == PAYPAL_TRAN_UPDATESURCHARGE)
	{
		strSendBody += L"<atm:cashAccessTicket>\r\n";
		strTemp.Format(L"<atm:amount>%s</atm:amount>\r\n",m_strTotalAmount);								strSendBody += strTemp;
		strTemp.Format(L"<atm:surchargeAmount>%s</atm:surchargeAmount>\r\n",m_strATMSurcharge);				strSendBody += strTemp;
		strTemp.Format(L"<atm:subTotal>%s</atm:subTotal>\r\n",m_strReqAmount);								strSendBody += strTemp;
		strTemp.Format(L"<atm:cashAccessTransactionType>WITHDRAWAL</atm:cashAccessTransactionType>\r\n");	strSendBody += strTemp;		// [#2522] US Justin Added
		strSendBody += L"</atm:cashAccessTicket>\r\n";
	}
	else if( (m_nTransactionStep == PAYPAL_TRAN_CANCEL_WITHTICKET) || (m_nTransactionStep == PAYPAL_TRAN_UPDATETRAN) )
	{
		if (m_nTransactionStep == PAYPAL_TRAN_CANCEL_WITHTICKET)
		{
			strSendBody += L"<atm:posProcessedPaymentResult>\r\n";				// [#2522]
	
			strTemp.Format(L"<atm:amount>%s</atm:amount>\r\n",m_strTotalAmount);							strSendBody += strTemp;
			// [#2466] US Justin 2017.01.27 Certification Issue Fix
			//strTemp.Format(L"<atm:paymentStatus>SUBMIT_PROCESSED_PAYMENTS</atm:paymentStatus>\r\n");		strSendBody += strTemp;
			//strTemp.Format(L"<atm:paymentStatusCode>250</atm:paymentStatusCode>\r\n");					strSendBody += strTemp;
			strTemp.Format(L"<atm:paymentStatus>%s</atm:paymentStatus>\r\n", m_strPaydiantDesc);			strSendBody += strTemp;
			strTemp.Format(L"<atm:paymentStatusCode>%s</atm:paymentStatusCode>\r\n", m_strPaydiantCode);	strSendBody += strTemp;
			// End of [#2466]
		}
		else	// PAYPAL_TRAN_UPDATETRAN
		{
			int nAmountDispense = Dollar2Cent(m_strTranDispensed);
			int nAmountRequest  = Dollar2Cent(m_strReqAmount);
			if( nAmountDispense > nAmountRequest )				nAmountDispense = nAmountRequest;		// Block Over dispensing

			// [#2522] US Justin 2017.12.18
			if( bIsSurchargeChanged == TRUE)
			{
				strSendBody += L"<atm:cashAccessTicket>\r\n";
				strTemp.Format(L"<atm:amount>%s</atm:amount>\r\n",m_strTotalAmount);								strSendBody += strTemp;
				strTemp.Format(L"<atm:surchargeAmount>%s</atm:surchargeAmount>\r\n",m_strProcSurcharge);			strSendBody += strTemp;
				strTemp.Format(L"<atm:subTotal>%s</atm:subTotal>\r\n",m_strReqAmount);								strSendBody += strTemp;
				strTemp.Format(L"<atm:cashAccessTransactionType>WITHDRAWAL</atm:cashAccessTransactionType>\r\n");	strSendBody += strTemp;
				strSendBody += L"</atm:cashAccessTicket>\r\n";
			}
			// End of [#2522]

			strSendBody += L"<atm:posProcessedPaymentResult>\r\n";				// [#2522]
			if( (nAmountDispense>0) && (nAmountDispense<nAmountRequest ) )		// Partial Dispense
			{
				nAmountDispense += Dollar2Cent(m_strProcSurcharge);
				strTemp.Format(L"<atm:amount>%0.2f</atm:amount>\r\n", 0.01*nAmountDispense);					strSendBody += strTemp;
				strTemp.Format(L"<atm:paymentStatus>PARTIAL_PAYMENT_SUCCESS</atm:paymentStatus>\r\n");			strSendBody += strTemp;
				strTemp.Format(L"<atm:paymentStatusCode>204</atm:paymentStatusCode>\r\n");						strSendBody += strTemp;
			}
			else																// Full Dispense
			{
				strTemp.Format(L"<atm:amount>%s</atm:amount>\r\n", m_strTotalAmount);							strSendBody += strTemp;
				strTemp.Format(L"<atm:paymentStatus>%s</atm:paymentStatus>\r\n", m_strPaydiantDesc);			strSendBody += strTemp;
				strTemp.Format(L"<atm:paymentStatusCode>%s</atm:paymentStatusCode>\r\n", m_strPaydiantCode);	strSendBody += strTemp;
			}
		}

		// [#2449] US Justin
		if( m_nTransactionStep == PAYPAL_TRAN_UPDATETRAN )
		{
			strTemp.Format(L"<atm:paymentTransactionUri>%s</atm:paymentTransactionUri>\r\n",m_strATMTrRefID);							strSendBody += strTemp;
			strTemp.Format(L"<atm:externalPaymentStatusCode>%s</atm:externalPaymentStatusCode>\r\n",m_strExternalCode);					strSendBody += strTemp;
			strTemp.Format(L"<atm:externalPaymentStatusDescription>%s</atm:externalPaymentStatusDescription>\r\n",m_strExternalDesc);	strSendBody += strTemp;
		}
		strTemp.Format(L"<atm:currencyCode>USD</atm:currencyCode>\r\n");								strSendBody += strTemp;
		// End of [#2449]

		strTemp.Format(L"<atm:paymentInstrument>\r\n");													strSendBody += strTemp;
			strTemp.Format(L"<atm:paymentTender>\r\n");													strSendBody += strTemp;
			strSendBody += m_strPaymentTender;
			strTemp.Format(L"</atm:paymentTender>\r\n");												strSendBody += strTemp;
			strTemp.Format(L"<atm:paymentInstrumentUri>%s</atm:paymentInstrumentUri>\r\n",m_strPaymentInstURi);		strSendBody += strTemp;
			strTemp.Format(L"<atm:paymentInstrumentId>%s</atm:paymentInstrumentId>\r\n",m_strPaymentInstID);		strSendBody += strTemp;
		strTemp.Format(L"</atm:paymentInstrument>\r\n");										strSendBody += strTemp;

		strSendBody += L"</atm:posProcessedPaymentResult>\r\n";
	}

	// Request element and Type
	if ( m_nTransactionStep == PAYPAL_TRAN_GETTOKEN)
	{
		strSendBody += L"</atm:ObtainCashAccessTokenRequest>\r\n";
	}
	else if	( m_nTransactionStep == PAYPAL_TRAN_GETCUSTOMERINFO)		
	{
		strSendBody += L"<atm:dataElementTypes>CUSTOMER_INFORMATION</atm:dataElementTypes>\r\n";
		strSendBody += L"</atm:RetrieveTransactionMetadataRequest>\r\n";
	}
	else if ( m_nTransactionStep == PAYPAL_TRAN_GETWITHDRAWALTICKET)
	{
		strSendBody += L"<atm:dataElementTypes>CASH_ACCESS_TICKET</atm:dataElementTypes>\r\n";
		strSendBody += L"</atm:RetrieveTransactionMetadataRequest>\r\n";
	}
	else if ( m_nTransactionStep == PAYPAL_TRAN_UPDATESURCHARGE)		
	{
		strSendBody += L"<atm:transactionFlowRules>UPDATE_TICKET</atm:transactionFlowRules>\r\n";
		strSendBody += L"</atm:UpdateTransactionRequest>\r\n";
	}
	else if ( m_nTransactionStep == PAYPAL_TRAN_UPDATETRAN)
	{
		strSendBody += L"<atm:transactionFlowRules>SUBMIT_PROCESSED_PAYMENTS</atm:transactionFlowRules>\r\n";
		// [#2522] US Justin 
		if( bIsSurchargeChanged == TRUE)
			strSendBody += L"<atm:transactionFlowRules>UPDATE_TICKET</atm:transactionFlowRules>\r\n";
		// End of [#2522]
		strSendBody += L"</atm:UpdateTransactionRequest>\r\n";
	}
	else if ( m_nTransactionStep == PAYPAL_TRAN_RELEASETOKEN)
	{
		strSendBody += L"</atm:ReleaseCashAccessTokenRequest>\r\n";
	}
	else if ( m_nTransactionStep == PAYPAL_TRAN_CANCEL_CUSINFO)		
	{
		strSendBody += L"<atm:transactionFlowRules>CANCEL_TRANSACTION</atm:transactionFlowRules>\r\n";
		strSendBody += L"</atm:UpdateTransactionRequest>\r\n";
	}
	else if ( m_nTransactionStep == PAYPAL_TRAN_CANCEL_WITHTICKET)		
	{
		strSendBody += L"<atm:transactionFlowRules>SUBMIT_PROCESSED_PAYMENTS</atm:transactionFlowRules>\r\n";
		strSendBody += L"</atm:UpdateTransactionRequest>\r\n";
	}
	//else if ( m_nTransactionStep == PAYPAL_TRAN_FULLREVERSAL)		
	//{
	//	strSendBody += L"<atm:transactionFlowRules>FULL_REVERSE_TRANSACTION</atm:transactionFlowRules>\r\n";
	//	strSendBody += L"</atm:UpdateTransactionRequest>\r\n";
	//}
	
	strSendBody += _T("</soapenv:Body>\r\n</soapenv:Envelope>\r\n");
	return strSendBody;
}

CString	CPaypalCCAData::MakeDemoResponseMessage()
{
	CString strXMLNS = L"ns2";
	CString strTemp;
	// Request Message Header
	CString strReceiveBody = L"<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n";
	strReceiveBody += L"<SOAP-ENV:Header/>\r\n";
	strReceiveBody += L"<SOAP-ENV:Body>\r\n";

	// Response Type
	strTemp.Format(L"<%s:",strXMLNS); 
	if      ( m_nTransactionStep == PAYPAL_TRAN_GETTOKEN)				strTemp += L"ObtainCashAccessTokenResponse";
	else if ( m_nTransactionStep == PAYPAL_TRAN_GETCUSTOMERINFO)		strTemp += L"RetrieveTransactionMetadataResponse";
	else if ( m_nTransactionStep == PAYPAL_TRAN_GETWITHDRAWALTICKET)	strTemp += L"RetrieveTransactionMetadataResponse";
	else if ( m_nTransactionStep == PAYPAL_TRAN_UPDATESURCHARGE)		strTemp += L"UpdateTransactionResponse";
	else if ( m_nTransactionStep == PAYPAL_TRAN_UPDATETRAN)				strTemp += L"UpdateTransactionResponse";
	else if ( m_nTransactionStep == PAYPAL_TRAN_RELEASETOKEN)			strTemp += L"ReleaseCashAccessTokenResponse";
	else if ( m_nTransactionStep == PAYPAL_TRAN_CANCEL_CUSINFO)			strTemp += L"UpdateTransactionResponse";
	else if ( m_nTransactionStep == PAYPAL_TRAN_CANCEL_WITHTICKET)		strTemp += L"UpdateTransactionResponse";
	//else if ( m_nTransactionStep == PAYPAL_TRAN_FULLREVERSAL)			strTemp += L"UpdateTransactionResponse";

	strReceiveBody += strTemp;
	strTemp.Format(L" xmlns:%s=\"http://www.paydiant.com/atm-gateway\">\r\n", strXMLNS);
	strReceiveBody += strTemp;

	// Credential
	strTemp.Format(L"<%s:atmId>%s</%s:atmId>\r\n", strXMLNS, m_strATMID, strXMLNS);							strReceiveBody += strTemp;
	strTemp.Format(L"<%s:atmLocationId>%s</%s:atmLocationId>\r\n", strXMLNS, m_strLocationID, strXMLNS);	strReceiveBody += strTemp;
	strTemp.Format(L"<%s:atmProviderId>%s</%s:atmProviderId>\r\n", strXMLNS, m_strProviderID, strXMLNS);	strReceiveBody += strTemp;

	CString strInputValue;
	// Token
	if( m_nTransactionStep == PAYPAL_TRAN_GETTOKEN )			strInputValue = L"23kb2ad";
	else														strInputValue = m_strAccessToken;
	strTemp.Format(L"<%s:cashAccessToken>%s</%s:cashAccessToken>\r\n", strXMLNS, strInputValue,strXMLNS);
	strReceiveBody += strTemp;

	// Paydiant Transaction reference ID
	if ( m_nTransactionStep == PAYPAL_TRAN_GETCUSTOMERINFO)		strInputValue = L"PPP";	
	else														strInputValue = m_strPaydiantRefID;	
	if ( (m_nTransactionStep!=PAYPAL_TRAN_GETTOKEN)&&(m_nTransactionStep!=PAYPAL_TRAN_RELEASETOKEN)	)
	{
		strTemp.Format(L"<%s:paydiantTransactionReferenceId>%s</%s:paydiantTransactionReferenceId>\r\n", strXMLNS, strInputValue, strXMLNS);
		strReceiveBody += strTemp;
	}

	// ATM Transacton reference ID
	if ( m_nTransactionStep == PAYPAL_TRAN_GETWITHDRAWALTICKET)	strInputValue = L"ATM";	
	else														strInputValue = m_strATMTrRefID;	
	if( (m_nTransactionStep!=PAYPAL_TRAN_GETTOKEN)			&& (m_nTransactionStep!=PAYPAL_TRAN_RELEASETOKEN) && 
		(m_nTransactionStep!=PAYPAL_TRAN_GETCUSTOMERINFO)	&& (m_nTransactionStep!=PAYPAL_TRAN_CANCEL_CUSINFO) )
	{
		strTemp.Format(L"<%s:atmTransactionReferenceId>%s</%s:atmTransactionReferenceId>\r\n", strXMLNS, strInputValue, strXMLNS);
		strReceiveBody += strTemp;
	}

	// Main Response
	CString strXMLNSCol = strXMLNS + L":";
	if ( m_nTransactionStep == PAYPAL_TRAN_GETCUSTOMERINFO)	
	{
		strTemp = L"<ns2:customer>\r\n";
			strTemp += L"<ns2:customerId>7oxysoYiRlRx</ns2:customerId>\r\n";
			strTemp += L"<ns2:customerUri>auto001:entity:customer#xxx-000-x09x</ns2:customerUri>\r\n";
			strTemp += L"<ns2:firstName>Paul</ns2:firstName>\r\n";
			strTemp += L"<ns2:lastName>Casey</ns2:lastName>\r\n";
			strTemp += L"<ns2:address1>N/A</ns2:address1>\r\n";
			strTemp += L"<ns2:address2>N/A</ns2:address2>\r\n";
			strTemp += L"<ns2:city>N/A</ns2:city>\r\n";
			strTemp += L"<ns2:zipCode>N/A</ns2:zipCode>\r\n";
			strTemp += L"<ns2:state>N/A</ns2:state>\r\n";
			strTemp += L"<ns2:country>N/A</ns2:country>\r\n";
			strTemp += L"<ns2:phone>2101115522</ns2:phone>\r\n";
			strTemp += L"<ns2:email>2172621056@mobileregistration.com</ns2:email>\r\n";
		strTemp += L"</ns2:customer>\r\n";
		strTemp += L"<ns2:currentTransactionStatus>ACTIVE</ns2:currentTransactionStatus>\r\n";
		strTemp += L"<ns2:dataAccessResults>\r\n";
			strTemp += L"<ns2:dataAvailable>true</ns2:dataAvailable>\r\n";
			strTemp += L"<ns2:dataAccessTimeOut>false</ns2:dataAccessTimeOut>\r\n";
			strTemp += L"<ns2:customerDidNotProvideData>false</ns2:customerDidNotProvideData>\r\n";
			strTemp += L"<ns2:dataElementType>CUSTOMER_INFORMATION</ns2:dataElementType>\r\n";
		strTemp += L"</ns2:dataAccessResults>\r\n";
		strTemp += L"<ns2:nextTransactionFlowRuleSpecifications>\r\n";
			strTemp += L"<ns2:expectsCheckoutToken>false</ns2:expectsCheckoutToken>\r\n";
			strTemp += L"<ns2:expectsPaydiantReferenceId>true</ns2:expectsPaydiantReferenceId>\r\n";
			strTemp += L"<ns2:stepNumber>2</ns2:stepNumber>\r\n";
			strTemp += L"<ns2:stepOptional>true</ns2:stepOptional>\r\n";
			strTemp += L"<ns2:transactionFlowRule>UPDATE_TICKET</ns2:transactionFlowRule>\r\n";
			strTemp += L"<ns2:transactionType>WITHDRAWAL</ns2:transactionType>\r\n";
		strTemp += L"</ns2:nextTransactionFlowRuleSpecifications>\r\n";

		strTemp.Replace(L"ns2:", strXMLNSCol);
		strReceiveBody += strTemp;
	}
	else if ( m_nTransactionStep == PAYPAL_TRAN_GETWITHDRAWALTICKET)	
	{
		strTemp = L"<ns2:customerSelectedPaymentInstrument>\r\n";
			strTemp += L"<ns2:paymentTender>\r\n";
				strTemp += L"<ns2:accountTypeName>DEBIT_CARD_ACCOUNT</ns2:accountTypeName>\r\n";
				strTemp += L"<ns2:accountTypeUri>paydiant:payment-account-management.accounttype#Debit</ns2:accountTypeUri>\r\n";
				strTemp += L"<ns2:networkTypeName>NYCE</ns2:networkTypeName>\r\n";
				strTemp += L"<ns2:networkTypeUri>paydiant:payment-account-management.networktype#NYCE</ns2:networkTypeUri>\r\n";
			strTemp += L"</ns2:paymentTender>\r\n";
			strTemp += L"<ns2:additionalData>\r\n";
				strTemp += L"<ns2:key>STATE</ns2:key>\r\n";
				strTemp += L"<ns2:value>Georgia</ns2:value>\r\n";
			strTemp += L"</ns2:additionalData>\r\n";
				strTemp += L"<ns2:additionalData>\r\n";
				strTemp += L"<ns2:key>ZIP_CODE</ns2:key>\r\n";
				strTemp += L"<ns2:value>12345</ns2:value>\r\n";
			strTemp += L"</ns2:additionalData>\r\n";
			strTemp += L"<ns2:additionalData>\r\n";
				strTemp += L"<ns2:key>HOLDER_NAME</ns2:key>\r\n";
				strTemp += L"<ns2:value>Mr. MobileGateway Automation</ns2:value>\r\n";
			strTemp += L"</ns2:additionalData>\r\n";
			strTemp += L"<ns2:additionalData>\r\n";
				strTemp += L"<ns2:key>COUNTRY</ns2:key>\r\n";
				strTemp += L"<ns2:value>US</ns2:value>\r\n";
			strTemp += L"</ns2:additionalData>\r\n";
			strTemp += L"<ns2:additionalData>\r\n";
				strTemp += L"<ns2:key>MOBILE_NUMBER</ns2:key>\r\n";
				strTemp += L"<ns2:value>2101115522</ns2:value>\r\n";
			strTemp += L"</ns2:additionalData>\r\n";
			strTemp += L"<ns2:additionalData>\r\n";
				strTemp += L"<ns2:key>NICK_NAME</ns2:key>\r\n";
				strTemp += L"<ns2:value>Mr. MobileGateway's ATM</ns2:value>\r\n";
			strTemp += L"</ns2:additionalData>\r\n";
			strTemp += L"<ns2:additionalData>\r\n";
				strTemp += L"<ns2:key>EXPIRY_DATE</ns2:key>\r\n";
				strTemp += L"<ns2:value>05-2022</ns2:value>\r\n";
			strTemp += L"</ns2:additionalData>\r\n";
			strTemp += L"<ns2:additionalData>\r\n";
				strTemp += L"<ns2:key>IMAGE_PATH</ns2:key>\r\n";
				strTemp += L"<ns2:value>Cards/Defaults/NYCE-card.png</ns2:value>\r\n";
			strTemp += L"</ns2:additionalData>\r\n";
			strTemp += L"<ns2:additionalData>\r\n";
				strTemp += L"<ns2:key>ADDRESS_LINE1</ns2:key>\r\n";
				strTemp += L"<ns2:value>37 Schmackel Lane</ns2:value>\r\n";
			strTemp += L"</ns2:additionalData>\r\n";
			strTemp += L"<ns2:additionalData>\r\n";
				strTemp += L"<ns2:key>EMAIL</ns2:key>\r\n";
				strTemp += L"<ns2:value>2172621056@mobileregistration.com</ns2:value>\r\n";
			strTemp += L"</ns2:additionalData>\r\n";
			strTemp += L"<ns2:additionalData>\r\n";
				strTemp += L"<ns2:key>CARD_NUMBER</ns2:key>\r\n";
				strTemp += L"<ns2:value>7999996698717894</ns2:value>\r\n";
			strTemp += L"</ns2:additionalData>\r\n";
			strTemp += L"<ns2:additionalData>\r\n";
				strTemp += L"<ns2:key>CITY</ns2:key>\r\n";
				strTemp += L"<ns2:value>Wellesley</ns2:value>\r\n";
			strTemp += L"</ns2:additionalData>\r\n";
			strTemp += L"<ns2:additionalData>\r\n";
				strTemp += L"<ns2:key>MASKED_CARD_NUMBER</ns2:key>\r\n";
				strTemp += L"<ns2:value>xxxxxxxxxxxx7894</ns2:value>\r\n";
			strTemp += L"</ns2:additionalData>\r\n";
			strTemp += L"<ns2:additionalData>\r\n";
				strTemp += L"<ns2:key>CURRENCY_CODE</ns2:key>\r\n";
				strTemp += L"<ns2:value>USD</ns2:value>\r\n";
			strTemp += L"</ns2:additionalData>\r\n";
			strTemp += L"<ns2:paymentInstrumentUri>auto001:accountmenagement:cardaccount#xx00zz</ns2:paymentInstrumentUri>\r\n";
			strTemp += L"<ns2:paymentInstrumentId>23kuOWaKOoPw</ns2:paymentInstrumentId>\r\n";
		strTemp += L"</ns2:customerSelectedPaymentInstrument>\r\n";
		strTemp += L"<ns2:cashAccessTicket>\r\n";
			strTemp += L"<ns2:amount>20.0</ns2:amount>\r\n";
			strTemp += L"<ns2:subTotal>20.0</ns2:subTotal>\r\n";
			strTemp += L"<ns2:cashAccessTransactionType>WITHDRAWAL</ns2:cashAccessTransactionType>\r\n";
		strTemp += L"</ns2:cashAccessTicket>\r\n";
		strTemp += L"<ns2:currentTransactionStatus>PROCESSING</ns2:currentTransactionStatus>\r\n";
		strTemp += L"<ns2:transactionDetails>\r\n";
			strTemp += L"<ns2:totalAmountPaid>20.0</ns2:totalAmountPaid>\r\n";
		strTemp += L"</ns2:transactionDetails>\r\n";
		strTemp += L"<ns2:dataAccessResults>\r\n";
			strTemp += L"<ns2:dataAvailable>true</ns2:dataAvailable>\r\n";
			strTemp += L"<ns2:dataAccessTimeOut>false</ns2:dataAccessTimeOut>\r\n";
			strTemp += L"<ns2:customerDidNotProvideData>false</ns2:customerDidNotProvideData>\r\n";
			strTemp += L"<ns2:dataElementType>CASH_ACCESS_TICKET</ns2:dataElementType>\r\n";
		strTemp += L"</ns2:dataAccessResults>\r\n";
		strTemp += L"<ns2:nextTransactionFlowRuleSpecifications>\r\n";
			strTemp += L"<ns2:expectsCheckoutToken>false</ns2:expectsCheckoutToken>\r\n";
			strTemp += L"<ns2:expectsPaydiantReferenceId>true</ns2:expectsPaydiantReferenceId>\r\n";
			strTemp += L"<ns2:stepNumber>2</ns2:stepNumber>\r\n";
			strTemp += L"<ns2:stepOptional>true</ns2:stepOptional>\r\n";
			strTemp += L"<ns2:transactionFlowRule>UPDATE_TICKET</ns2:transactionFlowRule>\r\n";
			strTemp += L"<ns2:transactionType>WITHDRAWAL</ns2:transactionType>\r\n";
		strTemp += L"</ns2:nextTransactionFlowRuleSpecifications>\r\n";

		strTemp.Replace(L"ns2:", strXMLNSCol);
		strReceiveBody += strTemp;
	}
	else if ( m_nTransactionStep == PAYPAL_TRAN_UPDATESURCHARGE)	
	{
		strTemp = L"<ns2:customerConfirmation>true</ns2:customerConfirmation>\r\n";
		strTemp += L"<ns2:transactionDetails>\r\n";
			strTemp += L"<ns2:totalAmountPaid>0.0</ns2:totalAmountPaid>\r\n";
		strTemp += L"</ns2:transactionDetails>\r\n";
		strTemp += L"<ns2:nextTransactionFlowRuleSpecifications>\r\n";
			strTemp += L"<ns2:expectsCheckoutToken>false</ns2:expectsCheckoutToken>\r\n";
			strTemp += L"<ns2:expectsPaydiantReferenceId>true</ns2:expectsPaydiantReferenceId>\r\n";
			strTemp += L"<ns2:stepNumber>3</ns2:stepNumber>\r\n";
			strTemp += L"<ns2:stepOptional>false</ns2:stepOptional>\r\n";
			strTemp += L"<ns2:transactionFlowRule>SUBMIT_PROCESSED_PAYMENTS</ns2:transactionFlowRule>\r\n";
			strTemp += L"<ns2:transactionType>WITHDRAWAL</ns2:transactionType>\r\n";
		strTemp += L"</ns2:nextTransactionFlowRuleSpecifications>\r\n";

		strTemp.Replace(L"ns2:", strXMLNSCol);
		strReceiveBody += strTemp;
	}
	else if ( m_nTransactionStep == PAYPAL_TRAN_UPDATETRAN)	
	{
		strTemp = L"<ns2:transactionDetails>\r\n";
			strTemp += L"<ns2:totalAmountPaid>12.0</ns2:totalAmountPaid>\r\n";
			strTemp += L"<ns2:processedPaymentResult>\r\n";
				strTemp += L"<ns2:paydiantTenderId>8j5xxcBocEpm</ns2:paydiantTenderId>\r\n";
				strTemp += L"<ns2:amount>12.0</ns2:amount>\r\n";
				strTemp += L"<ns2:paymentStatus>Approved</ns2:paymentStatus>\r\n";
				strTemp += L"<ns2:paymentStatusCode>200</ns2:paymentStatusCode>\r\n";
				strTemp += L"<ns2:paymentTransactionUri>Z1234X</ns2:paymentTransactionUri>\r\n";
				strTemp += L"<ns2:externalPaymentStatusCode>200</ns2:externalPaymentStatusCode>\r\n";
				strTemp += L"<ns2:externalPaymentStatusDescription>Success</ns2:externalPaymentStatusDescription>\r\n";
				strTemp += L"<ns2:currencyCode>USD</ns2:currencyCode>\r\n";
				strTemp += L"<ns2:paymentInstrument>\r\n";
					strTemp += L"<ns2:paymentTender>\r\n";
						strTemp += L"<ns2:accountTypeName>DEBIT_CARD_ACCOUNT</ns2:accountTypeName>\r\n";
						strTemp += L"<ns2:accountTypeUri>paydiant:payment-account-management.accounttype#Debit</ns2:accountTypeUri>\r\n";
						strTemp += L"<ns2:networkTypeName>NYCE</ns2:networkTypeName>\r\n";
						strTemp += L"<ns2:networkTypeUri>paydiant:payment-account-management.networktype#NYCE</ns2:networkTypeUri>\r\n";
					strTemp += L"</ns2:paymentTender>\r\n";
					strTemp += L"<ns2:additionalData>\r\n";
						strTemp += L"<ns2:key>MASKED_CARD_NUMBER</ns2:key>\r\n";
						strTemp += L"<ns2:value>xxxxxxxxxxxx7894</ns2:value>\r\n";
					strTemp += L"</ns2:additionalData>\r\n";
					strTemp += L"<ns2:additionalData>\r\n";
						strTemp += L"<ns2:key>NICK_NAME</ns2:key>\r\n";
						strTemp += L"<ns2:value>Mr. MobileGateway's ATM</ns2:value>\r\n";
					strTemp += L"</ns2:additionalData>\r\n";
					strTemp += L"<ns2:paymentInstrumentUri>auto001:accountmenagement:cardaccount#xx00xx</ns2:paymentInstrumentUri>\r\n";
					strTemp += L"<ns2:paymentInstrumentId>zz1234zz</ns2:paymentInstrumentId>\r\n";
				strTemp += L"</ns2:paymentInstrument>\r\n";
			strTemp += L"</ns2:processedPaymentResult>\r\n";
		strTemp += L"</ns2:transactionDetails>\r\n";

		strTemp.Replace(L"ns2:", strXMLNSCol);
		strReceiveBody += strTemp;
	}

	// Response Code
	strTemp =  L"<ns2:status>Success</ns2:status>\r\n";
	strTemp += L"<ns2:statusCode>100</ns2:statusCode>\r\n";
	strTemp += L"<ns2:statusDetails>Success</ns2:statusDetails>\r\n";
	strTemp.Replace(L"ns2:", strXMLNSCol);
	strReceiveBody += strTemp;

	// Closing
	strTemp.Format(L"</%s:",strXMLNS); 
	if      ( m_nTransactionStep == PAYPAL_TRAN_GETTOKEN)				strTemp += L"ObtainCashAccessTokenResponse";
	else if ( m_nTransactionStep == PAYPAL_TRAN_GETCUSTOMERINFO)		strTemp += L"RetrieveTransactionMetadataResponse";
	else if ( m_nTransactionStep == PAYPAL_TRAN_GETWITHDRAWALTICKET)	strTemp += L"RetrieveTransactionMetadataResponse";
	else if ( m_nTransactionStep == PAYPAL_TRAN_UPDATESURCHARGE)		strTemp += L"UpdateTransactionResponse";
	else if ( m_nTransactionStep == PAYPAL_TRAN_UPDATETRAN)				strTemp += L"UpdateTransactionResponse";
	else if ( m_nTransactionStep == PAYPAL_TRAN_RELEASETOKEN)			strTemp += L"ReleaseCashAccessTokenResponse";
	else if ( m_nTransactionStep == PAYPAL_TRAN_CANCEL_CUSINFO)			strTemp += L"UpdateTransactionResponse";
	else if ( m_nTransactionStep == PAYPAL_TRAN_CANCEL_WITHTICKET)		strTemp += L"UpdateTransactionResponse";
	//else if ( m_nTransactionStep == PAYPAL_TRAN_FULLREVERSAL)			strTemp += L"UpdateTransactionResponse";
	strReceiveBody += strTemp;
	strReceiveBody += L">\r\n";
	strReceiveBody += L"</SOAP-ENV:Body>\r\n";
	strReceiveBody += L"</SOAP-ENV:Envelope>\r\n";

	return strReceiveBody;
}

BOOL CPaypalCCAData::ParseReceivedData(CString strReceived)
{
	BOOL bRtn = FALSE;

	CString strMainDataNode = L"";
	if      ( m_nTransactionStep == PAYPAL_TRAN_GETTOKEN)				strMainDataNode += L"ObtainCashAccessTokenResponse";
	else if ( m_nTransactionStep == PAYPAL_TRAN_GETCUSTOMERINFO)		strMainDataNode += L"RetrieveTransactionMetadataResponse";
	else if ( m_nTransactionStep == PAYPAL_TRAN_GETWITHDRAWALTICKET)	strMainDataNode += L"RetrieveTransactionMetadataResponse";
	else if ( m_nTransactionStep == PAYPAL_TRAN_UPDATESURCHARGE)		strMainDataNode += L"UpdateTransactionResponse";
	else if ( m_nTransactionStep == PAYPAL_TRAN_UPDATETRAN)				strMainDataNode += L"UpdateTransactionResponse";
	else if ( m_nTransactionStep == PAYPAL_TRAN_RELEASETOKEN)			strMainDataNode += L"ReleaseCashAccessTokenResponse";
	else if ( m_nTransactionStep == PAYPAL_TRAN_CANCEL_CUSINFO)			strMainDataNode += L"UpdateTransactionResponse";
	else if ( m_nTransactionStep == PAYPAL_TRAN_CANCEL_WITHTICKET)		strMainDataNode += L"UpdateTransactionResponse";
	//else if ( m_nTransactionStep == PAYPAL_TRAN_FULLREVERSAL)			strMainDataNode += L"UpdateTransactionResponse";

	if(strMainDataNode.GetLength()<=0)
		return SetErrorCodeAndDesc(L"910");
	
	// Get Main Contents
	CString strMainContents, strXMLNS;
	strMainContents = GetNodeContents(strReceived, strMainDataNode, strXMLNS, TRUE);
	if( strMainContents.GetLength() <= 0 )
		return SetErrorCodeAndDesc(L"911");
	if(strXMLNS.GetLength()>0)
		strXMLNS += L":";
	NHDEBUG(DBG_INFO, (L"   XMLNS=[%s]\n", strXMLNS) );

	// Check Credential
	CString strTemp, strTemp2;
	CString strRcvAtmID = GetNodeContents(strMainContents, strXMLNS + L"atmId", strTemp2);
	CString strRcvLocID = GetNodeContents(strMainContents, strXMLNS + L"atmLocationId", strTemp2);
	CString strRcvPrvID = GetNodeContents(strMainContents, strXMLNS + L"atmProviderId", strTemp2);
	if( (strRcvAtmID.GetLength() <= 0)	|| (strRcvLocID.GetLength() <= 0)	|| (strRcvPrvID.GetLength()<=0) ||
		(strRcvAtmID != m_strATMID)		|| (strRcvLocID != m_strLocationID) || (strRcvPrvID != m_strProviderID) )
		return SetErrorCodeAndDesc(L"912");
	NHDEBUG(DBG_INFO, (L"   Check Credential OK\n") );

	// Read Status Code ("100" is successful)
	CString strStatusCode = GetNodeContents(strMainContents, strXMLNS + L"statusCode", strTemp2);
	NHDEBUG(DBG_INFO, (L"   Status Code = [%s]\n", strStatusCode) );

	// Check Status Code
	if(strStatusCode.GetLength() != 3)
		return SetErrorCodeAndDesc(L"919");
	if(strStatusCode != L"100")
	{
		CString strStatusDetails = GetNodeContents(strMainContents, strXMLNS + L"statusDetails", strTemp2);
		return SetErrorCodeAndDesc(strStatusCode, strStatusDetails);
	}
	NHDEBUG(DBG_INFO, (L"   Pass Status Code checking\n") );

	/////////////////////////////////////////////////////
	// Release Token
	if( m_nTransactionStep == PAYPAL_TRAN_RELEASETOKEN )
	{
		NHDEBUG(DBG_INFO, (L"   Release Token OK return\n") );
		return TRUE;
	}

	// Read Token
	strTemp = GetNodeContents(strMainContents, strXMLNS + L"cashAccessToken", strTemp2);
	NHDEBUG(DBG_INFO, (L"   Access Token = [%s]\n", strTemp) );
	if( strTemp.GetLength() <= 0 )
		return SetErrorCodeAndDesc(L"913");

	/////////////////////////////////////////////////////
	// Get Token : Save Token
	if( m_nTransactionStep == PAYPAL_TRAN_GETTOKEN )
	{
		m_strAccessToken = strTemp;
		NHDEBUG(DBG_INFO, (L"   GET TOKEN OK return\n") );
		return TRUE;
	}

	// Check Token
	if( m_strAccessToken != strTemp)
		return SetErrorCodeAndDesc(L"914");
	NHDEBUG(DBG_INFO, (L"   Token Matched\n") );

	// Read paydiant Transaction ref. ID
	strTemp = GetNodeContents(strMainContents, strXMLNS + L"paydiantTransactionReferenceId", strTemp2);
	NHDEBUG(DBG_INFO, (L"   Paydiant Transaction Ref. ID = [%s]\n", strTemp) );
	if( strTemp.GetLength() <= 0 )
		return SetErrorCodeAndDesc(L"915");

	/////////////////////////////////////////////////////
	// Get Customer Information
	if( m_nTransactionStep == PAYPAL_TRAN_GETCUSTOMERINFO )
	{
		// Assign Paydiant Reference ID
		m_strPaydiantRefID = strTemp;

		// Check Customer Information
		m_strCustomerInfo = GetNodeContents(strMainContents, strXMLNS + L"customer", strTemp2);
		NHDEBUG(DBG_INFO, (L"  Customer Info =\n%s\n", m_strCustomerInfo) );
		if(m_strCustomerInfo.GetLength()<=0)
			return SetErrorCodeAndDesc(L"930");

		NHDEBUG(DBG_INFO, (L"   Received Customer information OK return\n") );
		return TRUE;
	}

	// Check paydiant Transaction ref. ID
	if(m_strPaydiantRefID != strTemp)
		return SetErrorCodeAndDesc(L"916");
	NHDEBUG(DBG_INFO, (L"   Paydiant Transaction Ref. ID  Matched\n") );

	/////////////////////////////////////////////////////
	// Cancel Transaction (Customer Info)
	if( m_nTransactionStep == PAYPAL_TRAN_CANCEL_CUSINFO )		
	{
		NHDEBUG(DBG_INFO, (L"   Cancel Tran (Customer Info) OK return\n") );
		return TRUE;
	}

	// Read ATM Transaction ref. ID
	strTemp = GetNodeContents(strMainContents, strXMLNS + L"atmTransactionReferenceId", strTemp2);
	NHDEBUG(DBG_INFO, (L"   ATM Transaction Ref. ID = [%s]\n", strTemp) );
	if( strTemp.GetLength() <= 0 )
		return SetErrorCodeAndDesc(L"917");

	/////////////////////////////////////////////////////
	// Get Cash Withdrawal Ticket
	if( m_nTransactionStep == PAYPAL_TRAN_GETWITHDRAWALTICKET )
	{
		m_strATMTrRefID = strTemp;

		// customerSelectedPaymentInstrument 
		CString strPaymentInst = GetNodeContents(strMainContents, strXMLNS + L"customerSelectedPaymentInstrument", strTemp2);
		if(strPaymentInst.GetLength()<=0)
			return SetErrorCodeAndDesc(L"940");
		NHDEBUG(DBG_INFO, (L"  Received customerSelectedPaymentInstrument \n") );

		// Payment Tender
		m_strPaymentTender = GetNodeContents(strPaymentInst, strXMLNS + L"paymentTender", strTemp2);
		if(m_strPaymentTender.GetLength()<=0)
			return SetErrorCodeAndDesc(L"941");
		if( strXMLNS.GetLength() > 1)
			m_strPaymentTender.Replace(strXMLNS, L"atm:");
		else
		{
			m_strPaymentTender.Replace(L"</", L"*****");
			m_strPaymentTender.Replace(L"<", L"<atm:");	
			m_strPaymentTender.Replace(L"*****", L"</atm:");
		}
		NHDEBUG(DBG_INFO, (L"  Payment tender (converted)=\n%s\n", m_strPaymentTender) );

		// paymentInstrumentUri, m_strPaymentInstID
		m_strPaymentInstURi = GetNodeContents(strPaymentInst, strXMLNS + L"paymentInstrumentUri", strTemp2);
		m_strPaymentInstID  = GetNodeContents(strPaymentInst, strXMLNS + L"paymentInstrumentId", strTemp2);
		NHDEBUG(DBG_INFO, (L"  paymentInstrumentUri=[%s]\n", m_strPaymentInstURi) );
		NHDEBUG(DBG_INFO, (L"  paymentInstrumentId=[%s]\n", m_strPaymentInstID) );
		if( (m_strPaymentInstURi.GetLength()<=0)||(m_strPaymentInstID.GetLength()<=0) )
			return SetErrorCodeAndDesc(L"942");

		// Card Number
		m_strCW_CardNum		= GetAdditionalDataValue(strPaymentInst, strXMLNS, L"CARD_NUMBER");
		m_strCW_CardExpDate = GetAdditionalDataValue(strPaymentInst, strXMLNS, L"EXPIRY_DATE");
		//m_strCW_Currency	= GetAdditionalDataValue(strPaymentInst, strXMLNS, L"CURRENCY_CODE");		// [#2466] US Justin 2017.01.18 Paydiant Certification. Ignore Currency Code
		NHDEBUG(DBG_INFO, (L"  CARD_NUMBER=[%s]\n", m_strCW_CardNum) );
		NHDEBUG(DBG_INFO, (L"  EXPIRY_DATE=[%s]\n", m_strCW_CardExpDate) );
		//NHDEBUG(DBG_INFO, (L"  CURRENCY_CODE=[%s]\n", m_strCW_Currency) );							// [#2466] US Justin 2017.01.18 Paydiant Certification. Ignore Currency Code
		if( m_strCW_CardNum.GetLength() < 12 )
			return SetErrorCodeAndDesc(L"943");
		if( m_strCW_CardExpDate.GetLength() != 7 )
			return SetErrorCodeAndDesc(L"944");

		// [#2466] US Justin 2017.01.18 Paydiant Certification. Ignore Currency Code
		/*
		if( m_strCW_Currency != L"USD" )
			return SetErrorCodeAndDesc(L"945");
		*/
		// End of [#2466]

		// Cash Withdrwal Ticket, Requested Amount
		CString strCashWithTicket = GetNodeContents(strMainContents, strXMLNS + L"cashAccessTicket", strTemp2);
		NHDEBUG(DBG_INFO, (L"  cashAccessTicket=\n%s\n", strCashWithTicket) );
		if(strCashWithTicket.GetLength()<=0)
			return SetErrorCodeAndDesc(L"946");
		m_strReqAmount = GetNodeContents(strCashWithTicket, strXMLNS + L"amount", strTemp2);
		NHDEBUG(DBG_INFO, (L"  AMOUNT=[%s]\n", m_strReqAmount) );
		if( m_strReqAmount.GetLength()<=0 )
			return SetErrorCodeAndDesc(L"947");

		// Assign Default Surcharge and Total Amount
		m_strATMSurcharge = m_strProcSurcharge = L"0";				// [#2522] US Justin 		
		m_strTotalAmount = m_strReqAmount;
		NVDump('O', 'C', "00", m_strReqAmount, L"Paypal Amt");

		NHDEBUG(DBG_INFO, (L"   Get withdwaral ticket OK return\n") );
		return TRUE;
	}

	// Check ATM Transaction ref. ID
	if(m_strATMTrRefID != strTemp)
		return SetErrorCodeAndDesc(L"918");
	NHDEBUG(DBG_INFO, (L"   ATM Transaction Ref. ID  Matched\n") );
	
	if( m_nTransactionStep == PAYPAL_TRAN_UPDATESURCHARGE )		// Update Surcharge
	{
		// customer Confirmation
		strTemp = GetNodeContents(strMainContents, strXMLNS + L"customerConfirmation", strTemp2);
		NHDEBUG(DBG_INFO, (L"   customerConfirmation = [%s]\n", strTemp) );
		if( strTemp.CompareNoCase(L"true") != 0 )
			return SetErrorCodeAndDesc(L"920");	

		NHDEBUG(DBG_INFO, (L"   Updated Surcharge OK return\n") );
	}
	else if( m_nTransactionStep == PAYPAL_TRAN_UPDATETRAN )	
		NHDEBUG(DBG_INFO, (L"   Update Transaction OK return\n") );
	else if( m_nTransactionStep == PAYPAL_TRAN_CANCEL_WITHTICKET )
		NHDEBUG(DBG_INFO, (L"   Cancel after WithdrawalTicket OK return\n") );
	//else if( m_nTransactionStep == PAYPAL_TRAN_FULLREVERSAL )	
	//	NHDEBUG(DBG_INFO, (L"   Full Reversal OK return\n") );
	
	return TRUE;
}

BOOL CPaypalCCAData::SetErrorCodeAndDesc(CString sResCode, CString strDesc)
{
	m_strErrorCode.Format(L"DP%03s%02d", sResCode, m_nTransactionStep);
	if(strDesc.GetLength()>1)
		m_strErrorDesc = strDesc.Left(40);
	else
	{   												///////1234567890123456789012345678901234567890
		if(		 sResCode == "900" )		m_strErrorDesc = L"COMMUNICATION ERROR";
		else if( sResCode == "910" )		m_strErrorDesc = L"MAIN NODE ERROR";
		else if( sResCode == "911" )		m_strErrorDesc = L"NO CONTENT";
		else if( sResCode == "912" )		m_strErrorDesc = L"CREDENTIAL ERROR";
		else if( sResCode == "913" )		m_strErrorDesc = L"NO ACCESS TOKEN FOUND";
		else if( sResCode == "914" )		m_strErrorDesc = L"ACCESS TOKEN ERROR";
		else if( sResCode == "915" )		m_strErrorDesc = L"PAYDIANT REF ID NOT FOUND";
		else if( sResCode == "916" )		m_strErrorDesc = L"PAYDIANT REF ID ERROR";
		else if( sResCode == "917" )		m_strErrorDesc = L"ATM TRAN REF ID NOT FOUND";
		else if( sResCode == "918" )		m_strErrorDesc = L"ATM TRAN REF ID ERROR";
		else if( sResCode == "919" )		m_strErrorDesc = L"STATUS CODE ERROR";
		else if( sResCode == "920" )		m_strErrorDesc = L"SURCHARGE REFUSED";
		else if( sResCode == "930" )		m_strErrorDesc = L"NO CUSTOMER INFO";
		else if( sResCode == "940" )		m_strErrorDesc = L"NO SELECTED PAYMENT";
		else if( sResCode == "941" )		m_strErrorDesc = L"NO PAYMENT TENDER";
		else if( sResCode == "942" )		m_strErrorDesc = L"NO PAYMENT INST. URI OR ID";
		else if( sResCode == "943" )		m_strErrorDesc = L"CARD NUMBER ERROR";
		else if( sResCode == "944" )		m_strErrorDesc = L"EXPIRATION DATE ERROR";
		else if( sResCode == "945" )		m_strErrorDesc = L"CURRENCY ERROR";
		else if( sResCode == "946" )		m_strErrorDesc = L"NO CASH WITHDRAWAL TICKET";
		else if( sResCode == "947" )		m_strErrorDesc = L"NO REQUESTED AMOUNT";
		else								m_strErrorDesc = L"TRANSACTION ERROR";
	}

	NHDEBUG(DBG_INFO, (L"SET ERROR CODE: ORG CODE=[%s], ATM CODE=[%s], DESC=[%s]\n", sResCode, m_strErrorDesc, m_strErrorDesc) );
	NVDump('O', 'C', "00", m_strErrorCode, L"Paypal ERR");
	return FALSE;
}

void CPaypalCCAData::UpdateSurcharge(int nSurchargeAmt, int nSurchargeType)		// [#2522] US Justin
{
	int nInputAmountFull	= Dollar2Cent(m_strReqAmount);
	// [#2522] US Justin 2017.12.18
	// m_strATMSurcharge.Format(L"%0.2f", 0.01*nSurchargeAmt);
	if( nSurchargeType == PAYPAL_SURCHARGE_TYPE_ATM)
		m_strATMSurcharge.Format(L"%0.2f", 0.01*nSurchargeAmt);
	else 
		m_strProcSurcharge.Format(L"%0.2f", 0.01*nSurchargeAmt);
	// End of [#2522]
	m_strTotalAmount.Format(L"%0.2f", 0.01*(nInputAmountFull+nSurchargeAmt) );
}

// [#2449] Mapping ATM Code to Paypay external Code
void CPaypalCCAData::UpdateExternalStatusCode(CString strCode, CString strATMCode, CString strATMCodeDesc)
{
	if(strCode.GetLength() != 2)
	{
		NHDEBUG(DBG_INFO, (L"ERROR UpdateExternalStatusCode(%s), Code Length is not 2 \n", strCode) );
		return;
	}
	m_strExternalCode = strATMCode;
	m_strExternalDesc = strATMCodeDesc;

	// Assign External Status Code
	int nRespCode;
	if	   ( strCode == "00" )	    m_strPaydiantCode = L"200";		// Approved by an ATM host
	else if( strCode == "CE" )	    m_strPaydiantCode = L"241";		// ATM Host communication error
	else if( strCode == "IA" )		m_strPaydiantCode = L"251";		// Invalid Amount received.			// [#2466] US Justin 2017.01.27 
	else if( strCode == "TC" )		m_strPaydiantCode = L"500";		// Transaction Cancelled - General	// [#2466] US Justin 2017.01.27 
	else if( strCode == "DE" )	    m_strPaydiantCode = L"242";		// Dispense Error
	else															// Denied by an ATM Host
	{
		nRespCode = Asc2Int(strCode);
		if(	    nRespCode == 12)	m_strPaydiantCode = L"251";
		else if(nRespCode == 13)	m_strPaydiantCode = L"251";
		else if(nRespCode == 14)	m_strPaydiantCode = L"252";
		else if(nRespCode == 20)	m_strPaydiantCode = L"250";
		else if(nRespCode == 24)	m_strPaydiantCode = L"500";
		else if(nRespCode == 39)	m_strPaydiantCode = L"243";
		else if(nRespCode == 51)	m_strPaydiantCode = L"203";
		else if(nRespCode == 52)	m_strPaydiantCode = L"243";
		else if(nRespCode == 53)	m_strPaydiantCode = L"243";
		else if(nRespCode == 54)	m_strPaydiantCode = L"254";
		else if(nRespCode == 55)	m_strPaydiantCode = L"201";
		else if(nRespCode == 57)	m_strPaydiantCode = L"245";
		else if(nRespCode == 58)	m_strPaydiantCode = L"245";
		else if(nRespCode == 61)	m_strPaydiantCode = L"500";
		else if(nRespCode == 75)	m_strPaydiantCode = L"201";
		else if(nRespCode == 78)	m_strPaydiantCode = L"252";
		else if(nRespCode == 80)	m_strPaydiantCode = L"241";
		else if(nRespCode == 83)	m_strPaydiantCode = L"201";
		else if(nRespCode == 86)	m_strPaydiantCode = L"201";
		else if(nRespCode == 91)	m_strPaydiantCode = L"242";
		else if(nRespCode == 92)	m_strPaydiantCode = L"242";
		else						m_strPaydiantCode = L"500";		// Unlisted Errorcode => "500, Invalid Transaction"
	}

	// Assign External Status Description
	nRespCode = Asc2Int(m_strPaydiantCode);
	if	   ( nRespCode == 200)		m_strPaydiantDesc = L"Approved";
	else if( nRespCode == 201)		m_strPaydiantDesc = L"Declined";
	else if( nRespCode == 203)		m_strPaydiantDesc = L"Insufficient Funds";
	else if( nRespCode == 241)		m_strPaydiantDesc = L"Processing Error";
	else if( nRespCode == 242)		m_strPaydiantDesc = L"Processing Error";
	else if( nRespCode == 243)		m_strPaydiantDesc = L"Transaction Not allowed";
	else if( nRespCode == 245)		m_strPaydiantDesc = L"Transaction Not supported";
	else if( nRespCode == 250)		m_strPaydiantDesc = L"Invalid Inputs";
	else if( nRespCode == 251)		m_strPaydiantDesc = L"Invalid Amount";
	else if( nRespCode == 252)		m_strPaydiantDesc = L"Invalid Card Number";
	else if( nRespCode == 254)		m_strPaydiantDesc = L"Expired Card";
	else if( nRespCode == 500)		m_strPaydiantDesc = L"Invalid Transaction";
	else							m_strPaydiantDesc = L"Invalid Transaction";

	NHDEBUG(DBG_INFO, (L"UpdateExternalStatusCode(%s) => CODE[%s], DESC[%s]\n", strCode, m_strPaydiantCode, m_strPaydiantDesc) );
}
// End of [#2449]

////////////////////////////////////////////////////////////
// Day Total Function
void CPaypalCCAData::ResetTotal()
{
	NHDEBUG(DBG_INFO, (L"CPaypalCCAData::ResetTotal()\n") );
	SaveDispenseResult(TRUE);
}

BOOL CPaypalCCAData::ReadDispenseHistory()
{
	NHDEBUG(DBG_INFO, (L"CPaypalCCAData::ReadDispenseHistory()\n") );
	CString strPath;
	CFile cf;

	///////////////////////////////////////////////////////////////
	// Read Dispense History
	///////////////////////////////////////////////////////////////
	strPath.Format(L"%s\\%s", ATM2_DATA_PATH, PAYPAL_DISPENSERESULT_FILE);
	if( cf.Open( strPath, CFile::modeRead) == FALSE )
	{
		NHDEBUG(DBG_INFO, (L"[FAIL TO OPEN Diepsnse History(%s) => Reset Dispense Result]\n", strPath));
		SaveDispenseResult(TRUE);
		return TRUE;
	}

	int nFileLen = (int)cf.GetLength();
	unsigned char* pBuffer1 = new unsigned char[nFileLen + 2];		// Allocate buffer for binary file data
	if( pBuffer1 == NULL )
	{
		cf.Close();
		NHDEBUG(DBG_INFO, (L"[FAIL TO NEW BUFFER(%d)] => Reset Dispense Result\n", nFileLen + 2));
		SaveDispenseResult(TRUE);
		return TRUE;
	}
	nFileLen = cf.Read( pBuffer1, nFileLen );
	cf.Close();

	CString strDispData = (LPCSTR)pBuffer1;
	delete [] pBuffer1;
	NHDEBUG(DBG_INFO, (L"   Dispense Data =[%s]\n", strDispData));

	CStringArray	arrTemp;
	SplitString(strDispData, L",", arrTemp);	
	
	// Daytotal Info  (3) :		Date and Time, Dispense Count, Dispense Amount
	if(arrTemp.GetSize() < 3)
	{
		NHDEBUG(DBG_INFO, (L"[Dispense Information is wrong or not set => Reset Dispense Result]\n"));
		SaveDispenseResult(TRUE);
		return TRUE;
	}

	BOOL bReadSuccessful = TRUE;

	// DayTotal Date and Time
	CString strTemp = arrTemp.GetAt(0);
	NHDEBUG(DBG_INFO, (L"   m_strPPDayTotalDateTime = [%s]\n", strTemp));
	if(strTemp.GetLength()==14)		m_strPPDayTotalDateTime = strTemp;
	else							bReadSuccessful = FALSE;

	// DayTotal Dispense Count
	if(bReadSuccessful)
	{
		strTemp = arrTemp.GetAt(1);
		NHDEBUG(DBG_INFO, (L"   m_strPPDayTotalDispCount = [%s]\n", strTemp));
		if(Asc2Int(strTemp)>=0)			m_strPPDayTotalDispCount = strTemp;
		else							bReadSuccessful = FALSE;
	}

	// DayTotal Total Dispense Amount
	if(bReadSuccessful)
	{
		strTemp = arrTemp.GetAt(2);
		NHDEBUG(DBG_INFO, (L"   m_strPPDayTotalDispAmt = [%s]\n", strTemp));
		if(Asc2Int(strTemp)>=0)			m_strPPDayTotalDispAmt = strTemp;
		else							bReadSuccessful = FALSE;
	}

	if(bReadSuccessful)
		NHDEBUG(DBG_INFO, (L"CPaypalCCAData::ReadDispenseHistory () SUCCESS\n") );
	else
	{
		NHDEBUG(DBG_INFO, (L"[ERROR on Dispense Result Data]  => Reset Dispense Result\n"));
		SaveDispenseResult(TRUE);
	}	
	return TRUE;
}

BOOL CPaypalCCAData::SaveDispenseResult(BOOL bReset)
{
	NHDEBUG(DBG_INFO, (L"CPaypalCCAData::SaveDispenseResult(%d) \n", bReset));

	CString strPath;
	strPath.Format(L"%s\\%s", ATM2_DATA_PATH, PAYPAL_DISPENSERESULT_FILE);

	CFile cf;
	if( cf.Open( strPath, CFile::modeCreate|CFile::modeWrite) == FALSE )
	{
		NHDEBUG(DBG_INFO, (L"[FAIL TO OPEN(%s)]\n", strPath));
		return FALSE;
	}

	if(bReset)
	{
		m_strPPDayTotalDateTime.Format(L"%s%s",GetDate(), GetTime());
		m_strPPDayTotalDispCount = L"0";
		m_strPPDayTotalDispAmt = L"0";
	}

	CString strTotalDispenseData = L"";
	strTotalDispenseData += m_strPPDayTotalDateTime + _T(",");
	strTotalDispenseData += m_strPPDayTotalDispCount + _T(",");
	strTotalDispenseData += m_strPPDayTotalDispAmt;

	NHDEBUG(DBG_INFO, (L"   DispenseResult = [%s]\n",strTotalDispenseData));

	UINT nSize = strTotalDispenseData.GetLength();

	char *pBuffer = new char[nSize+1];
	memset(pBuffer, 0, nSize+1);
	WideToMulti( pBuffer, strTotalDispenseData, nSize);

	cf.Write(pBuffer, nSize );
	delete [] pBuffer;

	cf.Close();
	return TRUE;
}

void CPaypalCCAData::RecodeDispenseResult(long nAmount /* Cent */)
{
	NHDEBUG(DBG_INFO, (L"CPaypalCCAData::RecodeDispenseResult (Amt=%ld)\n", nAmount) );

	// Update Day Total ... Date Time
	if( m_strPPDayTotalDateTime.GetLength() != 14)
		m_strPPDayTotalDateTime.Format(L"%s%s", GetDate(), GetTime() );
	NHDEBUG(DBG_INFO, (L"  m_strPPDayTotalDateTime = [%s]\n", m_strPPDayTotalDateTime) );

	// Update Day Total ... Dispense Count
	long nTemp = Asc2Int(m_strPPDayTotalDispCount) + 1;
	m_strPPDayTotalDispCount.Format(L"%ld", nTemp);
	NHDEBUG(DBG_INFO, (L"  m_strPPDayTotalDispCount = [%s]\n", m_strPPDayTotalDispCount) );

	// Update Day Total ... Dispense Amount
	nTemp = Asc2Int(m_strPPDayTotalDispAmt) + ((int) (0.01*nAmount+0.001));	// Dollar
	m_strPPDayTotalDispAmt.Format(L"%ld", nTemp);
	NHDEBUG(DBG_INFO, (L"  m_strPPDayTotalDispAmt = [%s]\n", m_strPPDayTotalDispAmt) );

	SaveDispenseResult();
}

CString CPaypalCCAData::GetDaytotalJournalData()
{
	NHDEBUG(DBG_INFO, (L"CPaypalCCAData::GetDaytotalJournalData()\n") );

	CString strEJNLData = L"";
	strEJNLData += m_strPPDayTotalDateTime + _T("^");
	strEJNLData += m_strPPDayTotalDispCount + _T("^");
	strEJNLData += m_strPPDayTotalDispAmt;

	NHDEBUG(DBG_INFO, (L"   Journal Data=[%s]\n", strEJNLData) );
	return strEJNLData;
}

// Handling data
CString CPaypalCCAData::GetNodeContents(CString strSource, CString strFind, CString &strXMLNS, BOOL bMainNode)
{
	CString strReturn = L"";
	if( (strSource.GetLength() <=0)||(strFind.GetLength() <= 0) )
	{	
		NHDEBUG(DBG_INFO, (L"   GetNodeContents for [%s] Fail ===> 01\n", strFind) );
		return strReturn;
	}

	CString strTarget = strFind;
	if(!bMainNode)
		strTarget += L">";

	// Begin point
	int nLocBgn = strSource.Find(strTarget);
	if(nLocBgn<0)
	{	
		NHDEBUG(DBG_INFO, (L"   GetNodeContents for [%s] Fail ===> 02\n", strFind) );
		return strReturn;
	}
	CString strTemp2 = strSource.Mid( nLocBgn + strTarget.GetLength() );

	// End point
	int nLocEnd = strTemp2.Find(strTarget);
	if(nLocEnd<0)
	{
		NHDEBUG(DBG_INFO, (L"   GetNodeContents for [%s] Fail ===> 03\n", strFind) );
		return strReturn;
	}
	CString strTemp = strTemp2.Left(nLocEnd);

	CString strFoundContent = L"";
	// find XMLNS
	if(bMainNode)
	{
		int nTempLoc = strTemp.Find(L">");
		strFoundContent = strTemp.Mid(nTempLoc+1);
		strXMLNS = L"";
		if(nTempLoc>0)
		{
			strTemp2 = strTemp.Left(nTempLoc);
			nLocBgn = strTemp2.Find(L"xmlns:");
			nLocEnd = strTemp2.Find(L"=");
			if( (nLocBgn>=0) && (nLocEnd>0) && (nLocBgn<nLocEnd) )
			{
				nLocBgn += 6;
				if(	nLocBgn < nLocEnd)
				{
					strXMLNS = strTemp2.Mid(nLocBgn, nLocEnd-nLocBgn);
					NHDEBUG(DBG_INFO, (L"   xmlns =[%s])\n", strXMLNS) );
				}
			}
		}
	}
	else
		strFoundContent = strTemp;

	// find contents Only	
	nLocBgn = strFoundContent.ReverseFind('/');
	strReturn = strFoundContent.Left(nLocBgn-1);
	return strReturn;
}

CString CPaypalCCAData::GetAdditionalDataValue(CString strSource, CString strXMLNS, CString strKey)
{
	CString strReturn = L"";

	CString strSearchKey;
	strSearchKey.Format(L"<%skey>%s</%skey>", strXMLNS, strKey, strXMLNS);
	int nLocKeyBg = strSource.Find(strSearchKey);
	if(nLocKeyBg>=0)
	{
		CString strTemp, strTempBG, strTempED;
		strTemp = strSource.Mid( nLocKeyBg + strSearchKey.GetLength() );
		strTempBG.Format(L"<%svalue>", strXMLNS);
		strTempED.Format(L"</%svalue>", strXMLNS);

		int nLocBG = strTemp.Find(strTempBG);
		int nLocED = strTemp.Find(strTempED);

		if( (nLocBG>=0) && (nLocED>=0) && (nLocED > (nLocBG+strTempBG.GetLength()) ) )
		{
			strReturn = strTemp.Mid(nLocBG+strTempBG.GetLength(), nLocED-(nLocBG+strTempBG.GetLength()) );
			if( strReturn.GetLength()>20 )
				strReturn = L"";

		}
	}
	return strReturn;
}
#endif
// End of [#2446]