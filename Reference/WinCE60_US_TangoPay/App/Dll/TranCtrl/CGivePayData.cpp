#include "StdAfx.h"
#include ".\..\..\H\Tran\CGivePayData.h"
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

//[#2513] US Justin 2017.11.02 GivePay Giftcard Purchase
#if(APP_GPAY_GIFTCARD_PURCHASE)
 CGivePayData::CGivePayData()
{
	m_strNextDailyConfig = L"";			// [#2574]
	m_strCPDayTotalDateTime = m_strCPDayTotalDispCount = m_strCPDayTotalDispAmt = L"";	
	ResetSetting();
	ResetGPTransactionData(TRUE);
}

CGivePayData::~CGivePayData()
{
}

////////////////////////////////////////////////////////////
// Setting Function
void CGivePayData::ResetSetting()
{
	m_bGivePayAvailable = m_bGivePayEnrolled = FALSE;
	m_strHostIP = _T("");
	m_strPSK = m_strClientID = m_strClientSecret = _T("");;
	m_strHostPort = L"443";		// Default
}

BOOL CGivePayData::ReadSettingAndDispenseHistory()
{
	NHDEBUG(DBG_INFO, (L"CGivePayData::ReadSettingAndDispenseHistory()\n") );
	m_bGivePayAvailable = m_bGivePayEnrolled = FALSE;
	
	CString strPath;
	CFile cf;

	///////////////////////////////////////////////////////////////
	// Read Setting
	///////////////////////////////////////////////////////////////
	strPath.Format(L"%s\\%s", ATM_DATA_PATH, GPAY_SETTING_FILE);
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

	CString strDecPSK, strEncPSK;		// [#2566] NH Justin 2018.07.19 Encrypt GivePay 
	strDecPSK = strEncPSK = L"";		// [#2566] NH Justin 2018.07.19 Encrypt GivePay 

	while( xml.FindChildElem() )
	{
		CString strName = xml.GetChildTagName();
		if( strName.IsEmpty() == TRUE )
			continue;
		CString strChildData = xml.GetChildData();
		NHDEBUG(DBG_INFO, (L" XML ChildNode [%s] = [%s]\n", strName, strChildData ));
		if(		 strName.CompareNoCase(L"HostIP") == 0 )				m_strHostIP			= strChildData;
		else if( strName.CompareNoCase(L"HostPort") == 0 )				m_strHostPort		= strChildData;
		// [#2566] NH Justin 2018.07.19 Encrypt GivePay 
		//else if( strName.CompareNoCase(L"PSK") == 0 )					m_strPSK			= strChildData;
		else if( strName.CompareNoCase(L"PSKey") == 0 )					strEncPSK			= strChildData;
		else if( strName.CompareNoCase(L"PSKey_Initial_Value") == 0 )	strDecPSK			= strChildData;
		// End of [#2566]
		else if( strName.CompareNoCase(L"TaeURL") == 0 )				m_strTaeURL			= strChildData;		// [#RWC6-44] GivePay AutoEnrollment QR Code Display
		else if( strName.CompareNoCase(L"TaeKey") == 0 )				m_strTaeKey			= strChildData;		// [#RWC6-44] GivePay AutoEnrollment QR Code Display
		else if( strName.CompareNoCase(L"ClientID") == 0 )				m_strClientID		= strChildData;
		else if( strName.CompareNoCase(L"ClientSecret") == 0 )			m_strClientSecret	= strChildData;
	}

	if( xml.OutOfElem() == FALSE )
	{
		NHDEBUG(DBG_INFO, (L"[FAIL OUT OF <Setting>]\n"));
		ResetSetting();
		return FALSE;
	}

	// [#2566] NH Justin 2018.07.19 Encrypt GivePay 
	if( strEncPSK.GetLength() > 0)
		m_strPSK = ModifyPreSharedKey(strEncPSK, FALSE);
	else if(strDecPSK.GetLength() > 0)
	{
		m_strPSK = strDecPSK;
		if( (m_strHostIP.GetLength()>0)	&& (m_strHostPort.GetLength()>0) )
			SaveSetting();
	}

	if( (m_strHostIP.GetLength()>0)	&& (m_strHostPort.GetLength()>0) && (m_strPSK.GetLength()>0) )
	{
		m_bGivePayAvailable = TRUE;
		NHDEBUG(DBG_INFO, (L"GivePay Service Available\n") );
		if( (m_strClientID.GetLength()>0)	&& (m_strClientSecret.GetLength()>0) )
		{
			m_bGivePayEnrolled = TRUE;
			NHDEBUG(DBG_INFO, (L"GivePay Enrolled\n") );
		}
	}
	else
	{
		NHDEBUG(DBG_INFO, (L"[One or more paremeters are missing]\n"));
		ResetSetting();
		return FALSE;
	}

	// Create Image Folder
	strPath.Format( _T("%s\\Img"), GPAY_DATA_FOLDER);
	CreateDirectory(strPath, NULL);
		
	return ReadDispenseHistory();
}

BOOL CGivePayData::SaveSetting()
{
	NHDEBUG(DBG_INFO, (L"GivePay SaveSetting()\n") );

	CString strData, strPath, strTemp;
	CFile cf;

	strPath.Format(L"%s\\%s", ATM_DATA_PATH, GPAY_SETTING_FILE);
	NHDEBUG(DBG_INFO, (L"GivePay SETTING FILE = [%s]\n", strPath) );

	if( cf.Open( strPath, CFile::modeWrite | CFile::modeCreate ) == FALSE  )
	{
		NHDEBUG(DBG_INFO, (L"[FAIL TO OPEN(%s)]\n", strPath));
		return FALSE;
	}

	strData  = L"<GivePay>\r\n";
	strData += L"<Setting>\r\n";
	if(m_strHostIP.GetLength()>0)			{	strData += L"<HostIP>"			+ m_strHostIP							+ L"</HostIP>\r\n";			}
	if(m_strHostPort.GetLength()>0)			{	strData += L"<HostPort>"		+ m_strHostPort							+ L"</HostPort>\r\n";		}
	//if(m_strPSK.GetLength()>0)			{	strData += L"<PSK>"				+ m_strPSK								+ L"</PSK>\r\n";			}		// [#2566] NH Justin 2018.07.19 Encrypt GivePay 
	if(m_strPSK.GetLength()>0)				{	strData += L"<PSKey>"			+ ModifyPreSharedKey(m_strPSK, TRUE)	+ L"</PSKey>\r\n";			}		// [#2566] NH Justin 2018.07.19 Encrypt GivePay 
	if(m_strTaeURL.GetLength()>0)			{	strData += L"<TaeURL>"			+ m_strTaeURL							+ L"</TaeURL>\r\n";		}		// [#RWC6-44] GivePay AutoEnrollment QR Code Display
	if(m_strTaeKey.GetLength()>0)			{	strData += L"<TaeKey>"			+ m_strTaeKey							+ L"</TaeKey>\r\n";		}		// [#RWC6-44] GivePay AutoEnrollment QR Code Display
	if(m_strClientID.GetLength()>0)			{	strData += L"<ClientID>"		+ m_strClientID							+ L"</ClientID>\r\n";		}
	if(m_strClientSecret.GetLength()>0)		{	strData += L"<ClientSecret>"	+ m_strClientSecret						+ L"</ClientSecret>\r\n";	}

	strData += L"</Setting>\r\n";
	strData += L"</GivePay>\r\n";
	NHDEBUG(DBG_INFO, (L"GivePay Setting =\n%s", strData) );

	BYTE pBuffer[4096];
	memset(pBuffer, NULL, 4096);
	WideToMulti((LPSTR)pBuffer, strData, strData.GetLength());
	cf.Write( pBuffer, strData.GetLength() );
	cf.Close();

	m_bGivePayAvailable = m_bGivePayEnrolled = FALSE;
	if( (m_strHostIP.GetLength()>0)			&& (m_strHostPort.GetLength()>0) && (m_strPSK.GetLength()>0) )
	{
		m_bGivePayAvailable = TRUE;
		NHDEBUG(DBG_INFO, (L"GivePay Service Available\n") );
		if( (m_strClientID.GetLength()>0)	&& (m_strClientSecret.GetLength()>0) )
		{
			m_bGivePayEnrolled = TRUE;
			NHDEBUG(DBG_INFO, (L"GivePay Enrolled\n") );
		}
	}
	NHDEBUG(DBG_INFO, (L"GivePay SaveSetting() OK    RETURN TRUE\n") );
	return TRUE;
}

// [#2557] US Justin 2018.06.21
CString CGivePayData::GetGPImageFileName(int nID)
{
	CString strRtn;
	strRtn.Format(L"%s\\", GPAY_DATA_FOLDER);

	// [#2574] US Justin GivePay Enhancement3
	/*
	if(nID==GPAY_IMG_ICON_SELECTED_CARD)
		strRtn = GetImageFileName( GetJSONNodeValue(m_strSelectedCard, L"imageUri") );
	*/

	if(nID==GPAY_IMG_ICON_SELECTED_CARD)
	{
		CString strSelCardUri = GetJSONNodeValue(m_strSelectedCard, L"imageUri");
		if( IsImageFileExist(strSelCardUri) )
			strRtn = GetImageFileName(strSelCardUri);
		else
		{
			if(m_nGPServiceType == GPAY_SERVICE_GIFTCARD) 
				strRtn = GetGPImageFileName(GPAY_IMG_ICON_GENERAL_GIFT);
			else
				strRtn = GetGPImageFileName(GPAY_IMG_ICON_GENERAL_WIRELESS);
		}
	}
	// End of [#2574]
	else if(nID==GPAY_IMG_ICON_SELECTED_GREET )
		strRtn = GetImageFileName( GetJSONNodeValue(m_strSelectedGreeting, L"imageUri") );
	else
	{
		int nWidth = GetSystemMetrics(SM_CXSCREEN);
		int nHeight = GetSystemMetrics(SM_CYSCREEN);

		if( nWidth == 640 )			strRtn += L"640_480\\";
		else if( nWidth == 800 )	strRtn += L"800_600\\";
		else if( nWidth == 1024 )	
		{
			if( nHeight == 768 )	strRtn += L"1024_768\\";
			else					strRtn += L"1024_600\\";
		}

		switch(nID)
		{
			case GPAY_IMG_LOGO:						strRtn += L"LOGO";				break;
			case GPAY_IMG_OL_MAIN:					strRtn += L"MM";				break;
			case GPAY_IMG_OL_UPSELL:				strRtn += L"UPSELL";			break;
			case GPAY_IMG_OL_GC_CATEGORY:			strRtn += L"GC_CAT";			break;
			case GPAY_IMG_OL_GC_SUB_CATEGORY:		strRtn += L"GC_SCAT";			break;
			case GPAY_IMG_OL_GC_CARD_SELECTION:		strRtn += L"GC_SEL";			break;
			case GPAY_IMG_OL_GC_AMOUNT:				strRtn += L"GC_AMT_INPUT";		break;
			case GPAY_IMG_OL_GC_ORDER_CONFIRM:		strRtn += L"GC_CRFM_PRCHS";		break;
			case GPAY_IMG_OL_GC_SMS_FEE_CONFIRM:	strRtn += L"GC_CRFM_SMS";		break;
			case GPAY_IMG_OL_GC_DIGITAL_GREET:		strRtn += L"GC_DG";				break;
			case GPAY_IMG_OL_GC_RECIPENT_PHONE:		strRtn += L"GC_REC_INPUT";		break;
			case GPAY_IMG_OL_GC_SENDER_PHONE:		strRtn += L"GC_SEND_INPUT";		break;
			case GPAY_IMG_OL_WL_SELECTION:			strRtn += L"WC_CHS";			break;
			case GPAY_IMG_OL_WL_AMOUNT:				strRtn += L"WC_AMT_INPUT";		break;
			case GPAY_IMG_OL_WL_SMS_FEE_CONFIRM:	strRtn += L"WC_CRFM_SMS";		break;
			case GPAY_IMG_OL_WL_PHONE:				strRtn += L"WC_MDN_INPUT";		break;
			case GPAY_IMG_OL_ZIPCODE:				strRtn += L"GC_ZIP";			break;
			case GPAY_IMG_OL_THANK:					strRtn += L"THANK";				break;
			case GPAY_IMG_OL_WAIT:					strRtn += L"WAIT";				break;
			case GPAY_IMG_OL_TAC:					strRtn += L"TAC";				break;		// [#2574]
			case GPAY_IMG_ICON_GENERAL_GIFT:		strRtn += L"PLACEHOLDER_G";		break;		// [#2574]
			case GPAY_IMG_ICON_GENERAL_WIRELESS:	strRtn += L"PLACEHOLDER_W";		break;		// [#2574]
			case GPAY_IMG_OL_NEEDTIME:				strRtn += L"NEEDTIME";			break;		// [#2574]
			default:								strRtn  = L"";					break;
		}
		if(strRtn.GetLength()>0)
			strRtn += m_sLanguageExtension + _T(".png");		// Language Extension is not set When "LOGO" is called, 
	}

	NHDEBUG(DBG_INFO, (L"GivePay Image Name Return, requested id[%d] = [%s]\n", nID, strRtn));
	return strRtn;
}

// [#2566] NH Justin 2018.07.19 Encrypt GivePay 
CString CGivePayData::ModifyPreSharedKey(CString sInput, BOOL bEncrypt)
{
	// Random Symmetric Key......
	unsigned char symkey[32] = { 
		0x2f, 0xc1, 0x06, 0xc5, 0x84, 0x2c, 0x2d, 0x3f, 0xfd, 0x33, 0x05, 0x13, 0xf0, 0x3e, 0xf8, 0x94,
		0x2d, 0x1f, 0x61, 0x6a, 0x65, 0xac, 0x40, 0xb2,	0x08, 0x2f, 0x8e, 0x2b, 0xc0, 0x1a, 0x54, 0xb4 };

	CString strReturn = L"";
	if(bEncrypt==TRUE)
	{
		strReturn = Encrypt_And_Base64Encode(sInput, symkey, 2);
		NHDEBUG(DBG_INFO, (L"GivePay Encrypt PSK [%s] => [%s]\n", sInput, strReturn));
	}
	else
	{
		unsigned char result[4098] = {};
		int nConverted = Base64Decode_And_Decrypt(sInput, symkey, 2, result);
		strReturn = CString(result);
		strReturn = strReturn.Left(nConverted);
		NHDEBUG(DBG_INFO, (L"GivePay Decrypt PSK [%s] => [%s]\n", sInput, strReturn));
	}
	return strReturn;
}
// End of [#2566]

BOOL CGivePayData::GetKeyandIVFromSharedKeyAndSalt(CString strPSK, CString strSalt, unsigned char*key, unsigned char*iv)
{
	// Assign pre-shared key
	char* preSharedKey = new char[strPSK.GetLength() + 1];
	memset(preSharedKey, NULL, strPSK.GetLength() + 1);
	WideToMulti( preSharedKey, strPSK, strPSK.GetLength());

	// Assign Salt
	char* salt = new char[strSalt.GetLength() + 1];
	memset(salt, NULL, strSalt.GetLength() + 1);
	WideToMulti( salt, strSalt, strSalt.GetLength());

	BOOL bSuccess = TRUE;
	// Get Key
	if (!GetKeyFromSharedKey(key, preSharedKey, strPSK.GetLength(), (unsigned char*) salt, strSalt.GetLength(), 1000, 32, 0))
	{
		NHDEBUG(DBG_INFO, (L" GetKeyFromSharedKey => GET KEY ERROR )\n"));
		bSuccess = FALSE;
	}
	// Get IV
	if (bSuccess)
	{
		if (!GetKeyFromSharedKey(iv, preSharedKey, strPSK.GetLength(), (unsigned char*) salt, strSalt.GetLength(), 1000, 16, 32))
		{
			NHDEBUG(DBG_INFO, (L" GetKeyFromSharedKey => GET IV ERROR )\n"));
			bSuccess = FALSE;
		}
	}
	delete[] salt;
	delete[] preSharedKey;
	return bSuccess;
}

CString CGivePayData::EncryptTerminalID(CString strPSK, CString strSalt, CString strTID)
{
	CString strEncTID = L"";
	NHDEBUG(DBG_INFO, (L"GivePay EncryptTerminalID(PSK=[%s], Salt=[%s], TID=[%s])\n",  strPSK, strSalt, strTID));
	unsigned char key[32] = {};
	unsigned char iv[16] = {};

	if(GetKeyandIVFromSharedKeyAndSalt(strPSK, strSalt, key, iv))
	{
		NHDEBUG(DBG_INFO, (L"GivePay Get Key and IV OK. NOW ENCRYPT TID\n"));
		strEncTID = Encrypt_And_Base64Encode(strTID, key, 3, iv);  
	}
	NHDEBUG(DBG_INFO, (L"GivePay EncryptTerminalID =[%s]\n", strEncTID));
	return strEncTID;
}

CString CGivePayData::DecryptCredential(CString strPSK, CString strSalt, CString strCredential)
{
	CString strDecCredential = L"";
	NHDEBUG(DBG_INFO, (L"GivePay DecryptCredential(PSK=[%s], Salt=[%s], strCredential(first10)=[%s])\n",  strPSK, strSalt, strCredential.Left(10) ) );
	unsigned char key[32] = {};
	unsigned char iv[16] = {};

	if(	GetKeyandIVFromSharedKeyAndSalt(strPSK, strSalt, key, iv) )
	{
		NHDEBUG(DBG_INFO, (L"GivePay Get Key and IV OK. NOW ENCRYPT TID\n"));
		unsigned char decrypted[4098] = {};
		memset(decrypted, NULL, 4098);
		int nConverted = Base64Decode_And_Decrypt(strCredential, key, 3, decrypted, iv);
		if(nConverted > 0)
		{
			NHDEBUG(DBG_INFO, (L" Decryption OK, Decrypted Credential Length = [%d]\n", nConverted));
			strDecCredential.Format(L"%S", decrypted);
			strDecCredential = strDecCredential.Left(nConverted);
		}
	}
	NHDEBUG(DBG_INFO, (L"GivePay DecryptCredential return [%s]\n",  strDecCredential ) );
	return strDecCredential;
}

CString CGivePayData::GenerateRandomUUID()
{
	// 123e4567-e89b-12d3-a456-426655440000
	// Format 8 - 4 - 4 - 4 - 12
	// xxxxxxxx-xxxx-Mxxx-Nxxx-xxxxxxxxxxxx
	// M: Version (4:Random UUID)
	// N: Variant (Range 0x8000 - 0bfff)

	CString strUUID = L"";
	CString strTemp;
	for(int i=0; i<16; i++)
	{
		// add 4 "-" 
		if( (i==4)||(i==6)||(i==8)||(i==10) )
			strUUID += L"-";

		// Generate 16 Random Numbers
		BYTE btRand = rand() % 256;

		// Adjust Version and Variant
		if(i==6)		btRand = ((btRand & 0x0f) | 0x40);	// Version : Make Version "4" (Random)
		else if(i==8)	btRand = ((btRand & 0x3f) + 0x80);	// Variant : Range 0x80 ~ 0xbf

		// Combine UUID
		strTemp.Format(L"%02x", btRand);
		strUUID += strTemp;
	}
	NHDEBUG(DBG_INFO, (L"GivePay Random UUID = [%s]\n",  strUUID ) );
	return strUUID;
}

CString CGivePayData::MakeFullName(CString sFirstName, CString sLastName)
{
	CString strReturn = L"";
	if( (sFirstName.GetLength()>0) || (sLastName.GetLength()>0) )
	{
		if (sFirstName.GetLength()>0)
			strReturn += sFirstName;
		if((sFirstName.GetLength()>0) && (sLastName.GetLength()>0) )
			strReturn += L" ";
		if (sLastName.GetLength()>0)
			strReturn += sLastName;
	}
	return strReturn;
}

BOOL CGivePayData::ParseAndFillGPItemArray(CString strItems, CStringArray* arrGPStr)
{
	NHDEBUG(DBG_INFO, (L"Parse GivePay Items\n") );

	// Empty Category
	arrGPStr->RemoveAll();

	// [#J009] US Justin Bug Fix : AP Crash on GivePay Image Downloading
	UINT nDataLen = strItems.GetLength();
	UINT i = 0;
	while(1)
	{
		if(i>=nDataLen)
			break;

		int nNumCurlBrace =0;
		BOOL bStartCollecting = FALSE;
		CString sTempBuffer = L"";

		while(1)
		{
			CString sCheck = strItems.GetAt(i++);;
			if( sCheck == "{" )
			{
				nNumCurlBrace++;
				if (nNumCurlBrace==1)	bStartCollecting = TRUE;
				else					sTempBuffer += sCheck;
			}
			else if( sCheck == "}" )
			{
				nNumCurlBrace--;
				if (nNumCurlBrace==0 )
				{
					arrGPStr->Add(sTempBuffer);
					NHDEBUG(DBG_INFO, (L" Found Items [%d] : First500=[%s], Length=[%d]\n", arrGPStr->GetCount(), sTempBuffer.Left(500), sTempBuffer.GetLength() ) );
					break;
				}
				else 
					sTempBuffer += sCheck;
			}
			else if(bStartCollecting == TRUE)
				sTempBuffer += sCheck;

			if(i>=nDataLen)
				break;
		}	
	} 
	// End of [J009]

	NHDEBUG(DBG_INFO, (L"Parsed Category = [%d]\n", arrGPStr->GetCount() ) );
	if( arrGPStr->GetCount() >0 )
		return TRUE;
	return FALSE;
}

int CGivePayData::FillGPButtonItems(int nType, int nPage, CString* strBtn, CString*strImage, int*nNextPage, int nMaxBtnNum)
{
	NHDEBUG(DBG_INFO, (L"Fill GivePay Button Items, Type=[%d], Page=[%d]\n", nType, nPage ) );

	int nMAXBTN = nMaxBtnNum;
	int nFilledBtn = 0;
	*nNextPage = 0;

	CString strOneCategory, strID, strName, strImageFile, strTemp;
	int nHasChildren;
	
	CStringArray* parrStr = NULL;
	if(     nType==GPAY_ID_CATEGORY)		parrStr = &m_arrCategory;
	else if(nType==GPAY_ID_SUBCATEGORY)		parrStr = &m_arrSubCategory;
	else if(nType==GPAY_ID_CARDS)			parrStr = &m_arrCards;
	else if(nType==GPAY_ID_GREETINGS)		parrStr = &m_arrGreetings;
	else if(nType==GPAY_ID_WIRELESS)		parrStr = &m_arrWireless;

	if( (parrStr!=NULL) && (parrStr->GetSize()>(nPage*nMAXBTN)) )
	{
		int nButtonId = 0;
		for(int i=0; i<nMAXBTN; i++)
		{
			if( parrStr->GetSize() > ( nPage*nMAXBTN+i) )
			{
				nHasChildren = 0;
				strOneCategory = parrStr->GetAt(nPage*nMAXBTN+i);

				// check children and replace with dummy value to get ID and Name.
				strTemp = GetJSONNodeValue(strOneCategory, L"children" );
				if(strTemp.GetLength() > 10)
				{
					nHasChildren = 1;
					strOneCategory.Replace(strTemp, L"_CHILDREN_ITEMS_");
				}

				// Get ID and Name
				strID = GetJSONNodeValue(strOneCategory, L"id");
				if(nType==GPAY_ID_WIRELESS)			strName = GetJSONNodeValue(strOneCategory, L"displayName");
				else if(nType==GPAY_ID_GREETINGS)	strName = GetJSONNodeValue(strOneCategory, L"name");	// [#2574] US Justin GivePay Enhancement3 :
				else								strName = GetJSONNodeValue(strOneCategory, L"name");
				strName.Replace( L"&", L"&&");
				strName.Replace(L"\\r\\n", L"\n");	// Make Multi Lines

				// Image File Name			
				// [#2574] US Justin GivePay Enhancement3 : Replace image with generic image if file does not exist
				//strImageFile = GetImageFileName( GetJSONNodeValue(strOneCategory, L"imageUri") );
				strImageFile = L"";
				CString strImageURI = GetJSONNodeValue(strOneCategory, L"imageUri");
				if(IsImageFileExist(strImageURI) )
					strImageFile = GetImageFileName(strImageURI);
				else
				{
					if(nType==GPAY_ID_CARDS)
						strImageFile = GetGPImageFileName(GPAY_IMG_ICON_GENERAL_GIFT);
					else if(nType==GPAY_ID_WIRELESS)
						strImageFile = GetGPImageFileName(GPAY_IMG_ICON_GENERAL_WIRELESS);
				}
				// End of [#2574]				

				if( (strID.GetLength()>0) && (strName.GetLength()>0) )
				{
					if(strImageFile.GetLength() > 0)
					{
						strImage[nFilledBtn] = strImageFile;
						NHDEBUG(DBG_INFO, (L"Image File[%d] = [%s]\n", nFilledBtn, strImageFile ));
					}
					strBtn[nFilledBtn++].Format(L"%d%02d%06d%s", nHasChildren, (nPage*nMAXBTN+i), Asc2Int(strID), strName);						
					NHDEBUG(DBG_INFO, (L"Fill Button[%d] with [%s]\n", nFilledBtn-1, strBtn[nFilledBtn-1]) );
				}
			}
			else
				break;
		}
		if( parrStr->GetSize() > ((nPage+1)*nMAXBTN) )
			*nNextPage = 1;
	}
	return nFilledBtn;
}

BOOL CGivePayData::FillGPCardFixedAmount(CString* strBtn, int nButtonNum)
{
	NHDEBUG(DBG_INFO, (L"Fill GivePay Card Fixed Amount Button Values, Number of Button = [%d]\n", nButtonNum) );

	int nFilledBtn = 0;

	CString strFixedAmounts = GetJSONNodeValue(m_strSelectedCard, L"fixedAmounts");
	NHDEBUG(DBG_INFO, (L"Received fixedAmounts = [%s]\n", strFixedAmounts));

	if( strFixedAmounts.GetLength() < 1 )
	{
		NHDEBUG(DBG_INFO, (L"This Card Does not support Fixed Amount\n"));
		return FALSE;
	}

	CStringArray strTempArray;
	SplitString(strFixedAmounts, ",", strTempArray);	
	for(int i=0; i<	strTempArray.GetSize(); i++)
	{
		int nCentValue = Dollar2Cent(strTempArray.GetAt(i));
		if(nCentValue >0)
		{
			strBtn[nFilledBtn++].Format(L"%s%0.2f", GetCurrencySymbol(), (0.01*nCentValue+0.001) );
			NHDEBUG(DBG_INFO, (L"Fill Button[%d] with [%s]\n", nFilledBtn-1, strBtn[nFilledBtn-1]) );
			if(nFilledBtn == nButtonNum)
				break;
		}
	}

	if(nFilledBtn>0)
		return TRUE;
	return FALSE;
}

// [#2574] US Justin GivePay Enhancement3
BOOL CGivePayData::FillGPCardMinMaxAmount(CStringArray* arrAmtRange)
{
	NHDEBUG(DBG_INFO, (L"Fill GivePay Card Minimum and Maximum Range\n") );
	arrAmtRange->RemoveAll();

	CString strTemp;
	CString strRanges = GetJSONNodeValue(m_strSelectedCard, L"variableAmountRanges");
	NHDEBUG(DBG_INFO, (L" Original Min Max String = [%s]\n", strRanges ) );

	while(1)
	{
		if( strRanges.GetLength() < 23 )		// {"minimum":,"maximum":}
			break;

		int nMINAMT = Dollar2Cent( GetJSONNodeValue(strRanges, L"minimum") );	// Find First Match
		int nMAXAMT = Dollar2Cent( GetJSONNodeValue(strRanges, L"maximum") );	// Find First Match
		if( (nMINAMT>0) || (nMAXAMT>0) || (nMINAMT<=nMAXAMT) )
		{
			strTemp.Format(L"%06d%06d", nMINAMT, nMAXAMT);
			arrAmtRange->Add(strTemp);
			NHDEBUG(DBG_INFO, (L" FOUND AMOUNT RANGE[%d] = [%s]\n", arrAmtRange->GetSize(), strTemp ) );
		}

		int nLoc = strRanges.Find(L"},");
		if(nLoc >=0)	
		{
			strRanges = strRanges.Mid(nLoc+2);
			NHDEBUG(DBG_INFO, (L" New Min Max String = [%s]\n", strRanges ) );
		}
		else			
			break;
	}

	strTemp = L"";
	for(int i=0; i<arrAmtRange->GetSize(); i++)
		strTemp += arrAmtRange->GetAt(i) + L" ";
	NHDEBUG(DBG_INFO, (L"FOUND RANGES (Before Combine) = [%s]\n", strTemp) );
	
	if(arrAmtRange->GetSize()>1)
		CombineMultiRanges(arrAmtRange);

	strTemp = L"";
	for(int i=0; i<arrAmtRange->GetSize(); i++)
		strTemp += arrAmtRange->GetAt(i) + L" ";
	NHDEBUG(DBG_INFO, (L"Combined RANGES = [%s]\n", strTemp) );

	if(arrAmtRange->GetSize()>0)
		return TRUE;
	else
		return FALSE;
}

BOOL CGivePayData::CombineMultiRanges(CStringArray* arrAmtRange, CString* strBtn, int nBtnNum)
{
	NHDEBUG(DBG_INFO, (L"CGivePayData::CombineMultiRanges()\n") );

	CStringArray arrNewRange;
	CString strFoundRange, strTemp;
	while(1)
	{
		if(arrAmtRange->GetSize()<=0)
			break;

		strFoundRange = L"";

		// Find the first range
		int nSmallest = 0;
		strFoundRange = arrAmtRange->GetAt(0);
		int nMin = Asc2Int(strFoundRange.Left(6));
		for(int i=1; i<arrAmtRange->GetSize(); i++)
		{
			strTemp = arrAmtRange->GetAt(i);
			if( Asc2Int(strTemp.Left(6)) < nMin )
			{
				nSmallest = i;
				strFoundRange = strTemp;
				nMin = Asc2Int(strFoundRange.Left(6));
			}
		}

		// Remove the first Range from the original array
		arrAmtRange->RemoveAt(nSmallest);
		NHDEBUG(DBG_INFO, (L"First Range = Range[%d], [%s]\n", nSmallest, strFoundRange) );

		// Compare the found range with all other range.
		// if overlapped, adjust the found range and remove the item from the original list.
		// Continue above steps until there is no overlapped item.
		while(1)
		{
			if(arrAmtRange->GetSize()<=0)
			{
				NHDEBUG(DBG_INFO, (L"No range remained in the original list. Add Range [%s]\n", strFoundRange) );
				arrNewRange.Add(strFoundRange);
				break;
			}

			// Check if ranges are overlapped or not
			BOOL bCombined = FALSE;
			int nMinRange = Asc2Int(strFoundRange.Left(6));
			int nMaxRange = Asc2Int(strFoundRange.Right(6));
			int nCompare;
			for(nCompare=0; nCompare<arrAmtRange->GetSize(); nCompare++)
			{
				strTemp = arrAmtRange->GetAt(nCompare);
				if( Asc2Int(strTemp.Left(6)) <= nMaxRange)
				{
					bCombined = TRUE;
					break;
				}
			}
		
			if(bCombined)
			{
				// Merge two ranges and remove the found range from the original list
				strTemp = arrAmtRange->GetAt(nCompare);
				arrAmtRange->RemoveAt(nCompare);

				int nMinComp = Asc2Int(strTemp.Left(6));
				int nMaxComp = Asc2Int(strTemp.Right(6));
				int nNewMax = nMaxComp > nMaxRange ? nMaxComp :nMaxRange ; 
				strFoundRange.Format(L"%06d%06d",nMinRange, nNewMax);
				NHDEBUG(DBG_INFO, (L"Found overapped range: Org[%d - %d], Found[%d - %d] => Combined = [%s]\n", nMinRange, nMaxRange,nMinComp, nMaxComp, strFoundRange) );
			}
			else
			{
				NHDEBUG(DBG_INFO, (L"No Overlapped range found, Add Range [%s]\n", strFoundRange) );
				arrNewRange.Add(strFoundRange);
				break;
			}
		}
	}

	strTemp = L"";
	for(int i=0; i<arrNewRange.GetSize(); i++)
		strTemp += arrNewRange.GetAt(i) + L" ";
	NHDEBUG(DBG_INFO, (L"Result of Combining Ranges = [%s]\n", strTemp) );

	// Combine Individual Button Amounts
	if( (strBtn != NULL) && (nBtnNum>0) )
	{
		for(int i=0; i<nBtnNum; i++)
		{
			if( strBtn[i].GetLength() > 1 )
			{
				strTemp = strBtn[i].Left(1);
				if(strTemp == GetCurrencySymbol())
				{
					int nCompareAmt = Dollar2Cent(strBtn[i].Mid(1));
					if( !IsAmountIncludedInRange(&arrNewRange, nCompareAmt) )
					{
						NHDEBUG(DBG_INFO, (L"[%d] is not included in the ranges => Adding Range\n", nCompareAmt) );
						strTemp.Format(L"%06d%06d",nCompareAmt, nCompareAmt);
						arrNewRange.Add(strTemp);
					}
					else 
						NHDEBUG(DBG_INFO, (L"[%d] is included in the ranges\n", nCompareAmt) );
				}
			}
		}
	}

	// copy 
	arrAmtRange->RemoveAll();
	strTemp = L"";
	for(int i=0; i<arrNewRange.GetSize(); i++)
	{
		arrAmtRange->Add( arrNewRange.GetAt(i) );
		strTemp += arrNewRange.GetAt(i) + L" ";
	}
	NHDEBUG(DBG_INFO, (L"Return Final Combined Range [%s]\n", strTemp) );
	return TRUE;
}

BOOL CGivePayData::IsAmountIncludedInRange(CStringArray* arrAmtRange, int nAmount)
{
	for(int i=0; i<arrAmtRange->GetSize(); i++)
	{
		CString strOneRange = arrAmtRange->GetAt(i);
		int nMinAmt = Asc2Int(strOneRange.Left(6));
		int nMaxAmt = Asc2Int(strOneRange.Right(6));

		if((nAmount>=nMinAmt) && (nAmount<=nMaxAmt) )
			return TRUE;
	}
	return FALSE;
}

CString CGivePayData::GetAvailableAmountRange(CStringArray* arrAmtRange)
{
	CString strReturn = "";
	CString strTemp;
	for(int i=0; i<arrAmtRange->GetSize(); i++)
	{
		CString strOneRange = arrAmtRange->GetAt(i);
		int nMinAmt = Asc2Int(strOneRange.Left(6));
		int nMaxAmt = Asc2Int(strOneRange.Right(6));

		if(nMinAmt == nMaxAmt)
			strTemp.Format(L"%s%0.2f", GetCurrencySymbol(), (0.01*nMinAmt+0.0001) );
		else
			strTemp.Format(L"%s%0.2f-%s%0.2f", GetCurrencySymbol(), (0.01*nMinAmt+0.0001), GetCurrencySymbol(), (0.01*nMaxAmt+0.0001) );

		if(strReturn.GetLength() > 0)
			strReturn += L", ";
		strReturn += strTemp;
	}
	return strReturn;
}
// End of [#2574]
													
// [#J002] US Justin
void CGivePayData::CollectCardInformation(int nCategoryID)
{
	NHDEBUG(DBG_INFO, (L"Collect Card Information (CategoryID = %d)\n", nCategoryID) );
	CString strTemp;
	strTemp.Format(L"%06d", nCategoryID);
	int i;
	for(i=0; i<m_arrCards.GetSize(); i++)
		m_arrFullCardList.Add(strTemp+m_arrCards.GetAt(i));
	NHDEBUG(DBG_INFO, (L"[%d] CARD INFORMATION ARE ADDED ====> TOTAL COLLECTED CARD NUMBER =[%d]\n", i, m_arrFullCardList.GetSize() ));
}

int CGivePayData::GetSelectedCardInformation(int nCategoryID)
{
	NHDEBUG(DBG_INFO, (L"Get Selected Card Information (CategoryID = %d)\n", nCategoryID) );
	m_arrCards.RemoveAll();
	int nAdded = 0;
	CString strTemp;
	for(int i=0; i<m_arrFullCardList.GetSize(); i++)
	{
		strTemp = m_arrFullCardList.GetAt(i);
		if(strTemp.GetLength()>6)
		{
			if( Asc2Int(strTemp.Left(6)) == nCategoryID )
			{
				nAdded ++;
				m_arrCards.Add(strTemp.Mid(6));
			}
		}
	}
	NHDEBUG(DBG_INFO, (L"==> [%d] CARD INFORMATION ARE FILLED)\n", nAdded));
	return nAdded;
}
// End of [#J002] 

BOOL CGivePayData::ParseAndFillGPSubCategory(int nCatID)
{
	NHDEBUG(DBG_INFO, (L"Fill GivePay Sub Category\n") );

	if(nCatID>=m_arrCategory.GetSize() )
	{
		NHDEBUG(DBG_INFO, (L"Invalid Category ID\n") );
		return FALSE;
	}

	CString strSubCategory = m_arrCategory.GetAt(nCatID);
	m_arrSubCategory.RemoveAll();								// [#2557]
	if( !ParseAndFillGPItemArray(strSubCategory, &m_arrSubCategory) )
	{
		NHDEBUG(DBG_INFO, (L"Parsing Sub Catetory Failure \n") );
		return FALSE;
	}
	return TRUE;
}

////////////////////////////////////////////////////////////
// Transaction Function
// [#J006]
BOOL CGivePayData::CanMakeGPTransaction(BOOL bPiggyBackTran)
{
	BOOL bReturn = FALSE;
	if( (m_bGivePayAvailable == TRUE) && (m_bGivePayEnrolled == TRUE) &&
		(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_GIVEPAY_ENABLE_DISABLE) == ENABLE) &&											
		(MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) != NETWORK_DIALUP) &&
		(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) != L"NH1500SE") )
	{
		int nAppearOption = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_GIVEPAY_UPSELL);
		if( bPiggyBackTran == TRUE)
		{
			//if( (nAppearOption == RMS_GIVEPAY_PIGGYBACK) || (nAppearOption == RMS_GIVEPAY_BOTH) ) // [#RWC6-1]
			if( (nAppearOption == GIVEPAY_APPEAR_PIGGYBACK) || (nAppearOption == GIVEPAY_APPEAR_BOTH) )
				bReturn = TRUE;
		}
		else
		{
			//if( (nAppearOption == RMS_GIVEPAY_INDIVIDUAL) || (nAppearOption == RMS_GIVEPAY_BOTH) ) // [#RWC6-1]
			if( (nAppearOption == GIVEPAY_APPEAR_INDIVIDUAL) || (nAppearOption == GIVEPAY_APPEAR_BOTH) )
				bReturn = TRUE;
		}
	}
	return bReturn;
}
// End of [#J006]

void CGivePayData::ResetGPTransactionData(BOOL bEraseAll)		// [#2557]
{
	if(bEraseAll == TRUE)
	{
		m_strGPToken = m_strGPTokenType = m_strGPTokenExpire = _T("");	
		m_strDataExpire = _T("");
		m_arrCategory.RemoveAll();
		m_arrGreetings.RemoveAll();
		m_arrWireless.RemoveAll();
		m_strCardFee = m_strWirelessFee = m_strGreetingFee = _T("");
	}
	m_sLanguageExtension = _T("");
	m_nTransactionStep = GPAY_TRAN_NOTSTART;
	m_nGPCategoryId = 0;
	m_nGPServiceType = GPAY_SERVICE_GIFTCARD;

	m_arrSubCategory.RemoveAll();
	m_arrCards.RemoveAll();
	m_strSelectedCard = m_strSelectedGreeting = m_strGPTranAmount = _T("");
	m_strGPPhoneNumRcv = m_strGPPhoneNumSnd = m_strGPZipCode = _T("");	
	m_strGPHolderFName = m_strGPHolderLName = m_strGPPan = m_strGPCardExp = _T("");

	m_strStg_OrderId = m_strStg_ProductName = _T("");
	m_arrStg_ExtraFee.RemoveAll();
	m_strStg_UnitPrice = m_strStg_TaxAmt = m_strStg_FeeAmt = m_strStg_GifAmt = m_strStg_SubTotal = m_strStg_GrandTotal = _T("");

	m_strStg_Snd_FName = m_strStg_Snd_LName = m_strStg_Snd_FullName = m_strStg_Snd_Phone = _T("");
	m_strStg_Rcv_FName = m_strStg_Rcv_LName = m_strStg_Rcv_FullName = m_strStg_Rcv_Phone = _T("");

	m_strAuth_TrDateTime = _T("");

	m_strErrorCode = m_strErrorDesc = _T("");
}

// [#2557] US Justin 2018.06.12 
BOOL CGivePayData::IsGPParameterOld()
{
	if( !m_strDataExpire.IsEmpty() && !m_arrCategory.IsEmpty()    && !m_arrGreetings.IsEmpty()   && !m_arrWireless.IsEmpty() && 
		!m_strCardFee.IsEmpty()    && !m_strWirelessFee.IsEmpty() && !m_strGreetingFee.IsEmpty() )
	{
		CString sTE = m_strDataExpire;
		if(sTE.GetLength() == 14)
		{
			CTime CurTime = CTime::GetCurrentTime();
			CTime ExpTime( Asc2Int(sTE.Left(4)), Asc2Int(sTE.Mid(4,2)), Asc2Int(sTE.Mid(6,2)), Asc2Int(sTE.Mid(8,2)), Asc2Int(sTE.Mid(10,2)), Asc2Int(sTE.Mid(12,2)) );
			if( ExpTime > CurTime)
			{
				NHDEBUG(DBG_CALL, (L"GP Parameters are still valid. Expire at [%s]\n", sTE));
				return FALSE;
			}
		}
	}
	
	m_strDataExpire = _T("");
	m_arrCategory.RemoveAll();
	m_arrGreetings.RemoveAll();
	m_arrWireless.RemoveAll();
	m_strCardFee = m_strWirelessFee = m_strGreetingFee = _T("");
	return TRUE;
}

void CGivePayData::UpdateGPParameterExpire(int nExpSec)
{
	if(nExpSec < 180) // Minimum 3 minutes
		nExpSec = 180;

	CTimeSpan tokenSpan(nExpSec);
	CTime tmExp = CTime::GetCurrentTime() + tokenSpan;
	m_strDataExpire.Format( L"%04d%02d%02d%02d%02d%02d", tmExp.GetYear(), tmExp.GetMonth(), tmExp.GetDay(), tmExp.GetHour(), tmExp.GetMinute(), tmExp.GetSecond());
	NHDEBUG(DBG_INFO, (L"GivePay Parameters Expire at [%s]\n", m_strDataExpire));
}
// End of [#2557]
				
// [#2574] US Justin GivePay Enhancement3, Add Download Config Time
BOOL CGivePayData::NeedDailyUpdate()
{
	BOOL bNeedToDownload = TRUE;

	CString sNC = m_strNextDailyConfig;
	if(sNC.GetLength() == 14)
	{
		CTime CurTime = CTime::GetCurrentTime();
		CTime ExpTime( Asc2Int(sNC.Left(4)), Asc2Int(sNC.Mid(4,2)), Asc2Int(sNC.Mid(6,2)), Asc2Int(sNC.Mid(8,2)), Asc2Int(sNC.Mid(10,2)), Asc2Int(sNC.Mid(12,2)) );

		if( ExpTime > CurTime )
			bNeedToDownload = FALSE;
	}
	return bNeedToDownload;
}

void CGivePayData::UpdateGPNextDailyUpdateTime()
{
	CTime CurTime = CTime::GetCurrentTime();

	int nDownloadTime = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_GIVEPAY_CONFIG_TIME);
	nDownloadTime = 230;					// [#2574] JUSTIN 2018.10.04 HardCode GivePay Daily Update Time, 2:30

	NHDEBUG(DBG_INFO, (L"GivePay Config Time =  [%d]\n", nDownloadTime));

	int nDownloadHour = (int) (0.01*nDownloadTime);
	int nDownloadMinute = nDownloadTime % 100;
	CTime NextUP(CurTime.GetYear(), CurTime.GetMonth(),  CurTime.GetDay(), nDownloadHour, nDownloadMinute, 0);

	if(CurTime > NextUP)
		NextUP += CTimeSpan(1, 0, 0, 0);	// 1 day later

	m_strNextDailyConfig.Format( L"%04d%02d%02d%02d%02d%02d", NextUP.GetYear(), NextUP.GetMonth(), NextUP.GetDay(), NextUP.GetHour(), NextUP.GetMinute(), NextUP.GetSecond());
	NHDEBUG(DBG_INFO, (L"GivePay Next Daily Update Time =  [%s]\n", m_strNextDailyConfig));
}
// End of [#2574]

CString CGivePayData::GetImageFileName(CString sImageURI)
{
	CString strRtn = L"";

	if(sImageURI.GetLength()<10)
		return strRtn;

	int nLoc = sImageURI.ReverseFind('?');
	if(nLoc<0)		nLoc = sImageURI.GetLength();
	CString strFileName = sImageURI.Left(nLoc);

	nLoc = strFileName.ReverseFind('/');
	strFileName = strFileName.Mid(nLoc+1);

	
	if(strFileName.GetLength() > 0)
		strRtn.Format( _T("%s\\Img\\%s"), GPAY_DATA_FOLDER, strFileName );

	NHDEBUG(DBG_INFO, (L"GivePay Extract Image FileName [%s] => [%s]\n", sImageURI, strRtn));
	return strRtn;	
}

BOOL CGivePayData::ResetImageFiles()
{
	CString strDestFolder;
	strDestFolder.Format( L"%s\\Img", GPAY_DATA_FOLDER);
	ResetGPTransactionData(TRUE);
	NHDEBUG(DBG_INFO, (L"GivePay Reset Image from [%s]\n", strDestFolder ));
	return DeleteFileInDirectory(strDestFolder, _T("*.*"));
}

BOOL CGivePayData::IsImageFileExist(CString sImageURI)
{
	CString strFileName = GetImageFileName(sImageURI);

	BOOL bFileExist = FALSE;
	CFileStatus status;
	if( CFile::GetStatus( strFileName, status ) )
	{
		if(status.m_size>0)
			bFileExist = TRUE;
	}

	NHDEBUG(DBG_CALL, (L"IsImageFileExist(%s) = [%d]\n", strFileName, bFileExist));
	return bFileExist;
}

CString	CGivePayData::MakeGPRequestMessage(CString strTerminalID)
{
	NHDEBUG(DBG_INFO, (L"GivePay MakeRequestMessage(TID=[%s])\n", strTerminalID));
	CString strReturn = _T("");
	CString strTemp;
	if(m_nTransactionStep == GPAY_TRAN_ENROLLMENT)
	{
		// Assign Salt - Random Alpha Numeric (16 bytes)
		srand(GetTickCount());
		m_strGPSalt = _T("");

		// [#RWC6-41] GivePay Need To Fix Duplicates During Registration Process
		for (int i = 0; i < 16; i++)
		{
			strTemp.Format(L"%c", (char)(rand() % 89 + 38) ); // salt value from hexa to ascii
			m_strGPSalt += strTemp;
		}
		// end of [#RWC6-41]

		// Encrypt Terminal ID
		CString strEncTID = EncryptTerminalID(m_strPSK, m_strGPSalt, strTerminalID.TrimRight());

		// Combine Strings
		strReturn += L"{\"encryptedTid\": \"" + strEncTID + L"\", ";
		strReturn += L"\"manufacturer\": \"Nautilus-Hyosung\", ";
		strReturn += L"\"salt\": \"" + m_strGPSalt + L"\"}";
	}	
	else if(m_nTransactionStep == GPAY_TRAN_GETTOKEN)
	{
	// Grant Type
		strReturn += L"grant_type=client_credentials&";
		
		// Client ID (x-www-form-urlencoded)
		strReturn += L"client_id=";								
			strTemp.Format(L"%s",		m_strClientID);
			strTemp.Replace(_T("+"), _T("%2B"));
			strTemp.Replace(_T("/"), _T("%2F"));
			strTemp.Replace(_T("="), _T("%3D"));
		strReturn += strTemp + L"&";

		// Client secret (x-www-form-urlencoded)
		strReturn += L"client_secret=";								
			strTemp.Format(L"%s",		m_strClientSecret);
			strTemp.Replace(_T("+"), _T("%2B"));
			strTemp.Replace(_T("/"), _T("%2F"));
			strTemp.Replace(_T("="), _T("%3D"));
		strReturn += strTemp;	
	}
	else if(m_nTransactionStep == GPAY_TRAN_DOWNLOAD_FEES)			// Return URL
		strReturn.Format(L"api/v1/devices/self/fee-config");
	else if(m_nTransactionStep == GPAY_TRAN_DOWNLOAD_CATEGORY)		// Return URL
		strReturn.Format(L"api/v1/categories");
	else if(m_nTransactionStep == GPAY_TRAN_DOWNLOAD_CARDS)			// Return URL
		strReturn.Format(L"api/v1/cards?categoryId=%d", m_nGPCategoryId);
	else if(m_nTransactionStep == GPAY_TRAN_DOWNLOAD_GREETINGS)		// Return URL
		strReturn.Format(L"api/v1/greetings");
	else if(m_nTransactionStep == GPAY_TRAN_DOWNLOAD_WIRELESS)		// Return URL
		strReturn.Format(L"api/v1/carriers");
	else if( m_nTransactionStep == GPAY_TRAN_PURCHASECARD_STAGE )
	{
		/*
		{
		  "delivery": {
			"deliveryType": "phone",		// phone
			"sender": {
			  "name": "string",				// First Name of sender
			  "surname": "string",			// Last Name of Sender
			  "phoneNumber": "string"		// Phone Number of Sender
			},
			"phoneNumber": "string"			// Phone Number of Recipient
		  },
		  "lineItems": [
			{
			  "skuType": "gift_card",		// gift_card or wireless_rtr
			  "sku": "string",				// Id from Card or Wireless Carrier
			  "gifId": 0,					// ID of Greeting if selected
			  "unitPrice": 0				// Price to Purchase
			}
		  ],
		  "idempotencyString": "string",	// Random GUID

		  /////// Customer can be deleted
		  "customer": {
			"phoneNumber": "string"			// Phone Number of Recipient
		  }
		  //////

		}
		*/

		strReturn += L"{";
		strReturn +=    L"\"delivery\": {";
		strReturn +=       L"\"deliveryType\": \"phone\",";

		// Sender Information (Gift Card Only)
		if(m_nGPServiceType==GPAY_SERVICE_GIFTCARD)
		{
			strReturn +=       L"\"sender\": {";
			if(	m_strGPHolderFName.GetLength() > 0)		strReturn +=    L"\"name\": \""		+ m_strGPHolderFName + L"\",";
			if(	m_strGPHolderLName.GetLength() > 0)		strReturn +=    L"\"surname\": \""	+ m_strGPHolderLName + L"\",";
			strReturn +=          L"\"phoneNumber\": \"+1" + m_strGPPhoneNumSnd + L"\"";
			strReturn +=       L"},";
		}
		strReturn +=       L"\"phoneNumber\": \"+1" + m_strGPPhoneNumRcv + L"\"";
		strReturn +=    L"},";
		strReturn +=    L"\"lineItems\": [{";

		// Service Type
		strReturn +=       L"\"skuType\": ";
		if(m_nGPServiceType==GPAY_SERVICE_GIFTCARD)		strReturn +=       L"\"gift_card\",";
		else											strReturn +=       L"\"wireless_rtr\",";
		
		// Item ID
		strReturn +=       L"\"sku\": \"" + GetJSONNodeValue(m_strSelectedCard, L"id") + L"\",";

		// Greeting ID
		if( m_strSelectedGreeting.GetLength() > 0 )
			strReturn +=       L"\"gifId\": " + GetJSONNodeValue(m_strSelectedGreeting, L"id") + L",";\

		// Transaction Amount
		strReturn +=       L"\"unitPrice\": " + MakeMoneyCent(m_strGPTranAmount);

		strReturn +=    L"}],";
		strReturn +=    L"\"idempotencyString\": \"" + GenerateRandomUUID() + "\"";
		//strReturn +=    L"\"customer\": {";
		//strReturn +=       L"\"phoneNumber\": \"+1" + m_strGPPhoneNumRcv + L"\"";
		//strReturn +=    L"}";
		strReturn += L"}";
	}		
	else if( m_nTransactionStep == GPAY_TRAN_PURCHASECARD_APPROVAL )
	{
		/*
			{
			  "pan": "string",
			  "postalCode": "string",
			  "cvv": "string",
			  "cardHolderName": "string",
			  "expirationMonth": "string",
			  "expirationYear": "string",
			  "idempotencyString": "string"
			}
		*/

		strReturn += L"{";
		
		/*		
		#ifdef NH_DEBUG
			// GivePay Lab Test Only
			strReturn +=    L"\"pan\": \"4111111111111111\",";
		#else
			strReturn +=    L"\"pan\": \"" + m_strGPPan + "\",";				
		#endif	
		*/

		strReturn +=    L"\"pan\": \"" + m_strGPPan + "\",";					
		strReturn +=    L"\"postalCode\": \"" + m_strGPZipCode + L"\",";
		strReturn +=    L"\"amount\": \"" + m_strStg_GrandTotal + L"\",";
		
		// Sender Name
		if( m_strStg_Snd_FullName.GetLength()>0 )
			strReturn +=    L"\"cardHolderName\": \"" + m_strStg_Snd_FullName + L"\",";

		strReturn +=    L"\"expirationMonth\": \"" + m_strGPCardExp.Mid(2,2) + L"\",";
		strReturn +=    L"\"expirationYear\": \"" + m_strGPCardExp.Left(2) + L"\",";
		strReturn +=    L"\"idempotencyString\": \"" + GenerateRandomUUID()+ L"\"";
		strReturn += L"}";
	}
	else if( m_nTransactionStep == GPAY_TRAN_DOWNLOAD_IMAGES )				// Return Host URL
	{
		int nBgLoc = m_sImageURL.Find(L"://");
		if(nBgLoc>=0)		nBgLoc += 3;
		else				nBgLoc = 0;
		strReturn = m_sImageURL.Mid(nBgLoc);

		nBgLoc = strReturn.Find(L"/");
		if(nBgLoc < 0)		nBgLoc = strReturn.GetLength();
		strReturn = strReturn.Left(nBgLoc);
	}

	NHDEBUG(DBG_INFO, (L"GivePay Request (type=%d) Message = \n%s", m_nTransactionStep, strReturn));
	return strReturn;
}

CString	CGivePayData::MakeDemoResponseMessage()
{
	CString strSendBody  = L"{";
	
	// Make Response
	if(m_nTransactionStep == GPAY_TRAN_ENROLLMENT)					// Enrollment
	{
		strSendBody += L"\"encryptedClientCredentials\":\"NO_DECRYPT_ETyAge1BXJ:UK0HZSEvzGJPzpvonMTKqG\"},\"success\":true";
	}	
	else if(m_nTransactionStep == GPAY_TRAN_GETTOKEN)				// Get Token
	{
		strSendBody += L"\"access_token\":\"eyJhbGciOiJodHRwOi8vd3d3LnczLm9yZy8yM\",";
		strSendBody += L"\"expires_in\":3600,";
		strSendBody += L"\"token_type\":\"Bearer\"";
	}
	else if(m_nTransactionStep == GPAY_TRAN_DOWNLOAD_FEES)		// Download Fees
	{
		strSendBody += L"\"result\":{";
		strSendBody +=			    L"\"id\": 21,";
		strSendBody +=			    L"\"giftCardFee\": 2.12,";
		strSendBody +=			    L"\"wirelessFee\": 2.23,";
		strSendBody +=			    L"\"greetingFee\": 1.45";
		strSendBody +=L"},\"targetUrl\":null,\"success\":true";
	}
	else if(m_nTransactionStep == GPAY_TRAN_DOWNLOAD_CATEGORY)		// Download Category
	{
		strSendBody += L"\"result\":[{\"id\":2,\"imageUri\":null,\"children\":[";
		strSendBody +=					L"{\"id\":10,\"children\":[],\"name\":\"Casual Dining\"},";
		strSendBody +=					L"{\"id\":3,\"children\":[],\"name\":\"Steakhouses\"},";
		strSendBody +=					L"{\"id\":9,\"children\":[],\"name\":\"Fast Casual\"},";
		strSendBody +=					L"{\"id\":4,\"children\":[],\"name\":\"Seafood\"},";
		strSendBody +=					L"{\"id\":8,\"children\":[],\"name\":\"Italian\"}],";
		strSendBody +=					L"\"name\":\"Restaurants\"},";
		strSendBody +=			    L"{\"id\":1,\"imageUri\":null,\"children\":[],\"name\":\"Retailers\"},";
		strSendBody +=			    L"{\"id\":6,\"imageUri\":null,\"children\":[],\"name\":\"Experiences\"}";
		strSendBody +=L"],\"targetUrl\":null,\"success\":true";
	}
	else if(m_nTransactionStep == GPAY_TRAN_DOWNLOAD_GREETINGS)		// Download Greetings
	{
		strSendBody += L"\"result\":{\"items\":[";
		strSendBody +=		L"{\"id\":3,\"lastModificationTime\":\"0001-01-01T00:00:00\",\"name\":\"Happy Birthday!\",\"defaultMessage\":\"Happy Birthday!\",\"unitPrice\":0.0,\"enabled\":true},";
    	strSendBody +=		L"{\"id\":1,\"lastModificationTime\":\"0001-01-01T00:00:00\",\"name\":\"Congratulations!\",\"defaultMessage\":\"Congratulations!\",\"unitPrice\":0.0,\"enabled\":true},";
    	strSendBody +=		L"{\"id\":6,\"lastModificationTime\":\"0001-01-01T00:00:00\",\"name\":\"We <3 ATMIA\",\"defaultMessage\":\"We <3 ATMIA\",\"unitPrice\":0.0,\"enabled\":true},";
		strSendBody +=		L"{\"id\":4,\"lastModificationTime\":\"0001-01-01T00:00:00\",\"name\":\"Happy Mother's Day!\",\"defaultMessage\":\"Happy Mother's Day!\",\"unitPrice\":0.0,\"enabled\":true},";
    	strSendBody +=		L"{\"id\":5,\"lastModificationTime\":\"2018-02-12T20:49:04.21\",\"name\":\"Valentine's Day!\",\"defaultMessage\":\"Happy Valentine's Day!\",\"unitPrice\":0.0,\"enabled\":true},";
    	strSendBody +=		L"{\"id\":2,\"lastModificationTime\":\"2018-03-12T21:12:33.330714\",\"name\":\"Buena Suerte\",\"defaultMessage\":\"Buena Suerte\",\"unitPrice\":0.0,\"enabled\":true}";
    	strSendBody += L"]},\"targetUrl\":null,\"success\":true,\"error\":null,\"unAuthorizedRequest\":false,\"__abp\":true";
	}
	else if(m_nTransactionStep == GPAY_TRAN_DOWNLOAD_WIRELESS)		// Download Wireless Carriers
	{
		strSendBody += L"\"result\":{\"items\":[";
		strSendBody +=		L"{\"id\":1,\"fixedAmounts\":[],\"variableAmountRanges\":[{\"minimum\":10.0,\"maximum\":50.0}],\"displayName\":\"AT&T\",\"additionalConvenienceFee\":0.0,\"enabled\":true},";
		strSendBody +=		L"{\"id\":2,\"fixedAmounts\":[],\"variableAmountRanges\":[{\"minimum\":10.0,\"maximum\":50.0}],\"displayName\":\"T-Mobile\",\"additionalConvenienceFee\":0.0,\"enabled\":true},";
		strSendBody +=		L"{\"id\":3,\"fixedAmounts\":[],\"variableAmountRanges\":[{\"minimum\":1.0,\"maximum\":50.0}],\"displayName\":\"Cricket\",\"additionalConvenienceFee\":0.0,\"enabled\":true},";
		strSendBody +=		L"{\"id\":4,\"fixedAmounts\":[],\"variableAmountRanges\":[{\"minimum\":1.0,\"maximum\":50.0}],\"displayName\":\"Virgin Mobile\",\"additionalConvenienceFee\":0.0,\"enabled\":true}";
    	strSendBody += L"]},\"targetUrl\":null,\"success\":true,\"error\":null,\"unAuthorizedRequest\":false,\"__abp\":true";
	}
	else if(m_nTransactionStep == GPAY_TRAN_DOWNLOAD_CARDS)			// Download Cards
	{
		strSendBody += L"\"result\":[";

		if( m_nGPCategoryId==10 || m_nGPCategoryId==3 || m_nGPCategoryId==9 || m_nGPCategoryId==4 || m_nGPCategoryId==8 )
		{
			strSendBody +=    L"{\"name\":\"Applebee's\",\"cardCategory\":null,\"imageUri\":\"https://app.giftango.com/GPCGraphics/CIR_000424_02.png\",\"vendorCount\":1,";
			strSendBody +=      L"\"termsAndConditions\":[\"Terms and Conditions: This gift card is usable up to balance only to purchase goods or services at any Applebee's Grill in the U.S. and Canada or through applebees.com.\"],";
			strSendBody +=      L"\"fixedAmounts\":[],";
			strSendBody +=      L"\"variableAmountRanges\":[{\"minimum\":5.0,\"maximum\":50.0}],";
			strSendBody +=      L"\"lastModificationTime\":\"2018-03-22T20:30:38.344481\",";
			strSendBody +=      L"\"feeAmount\":0.0,";
			strSendBody +=      L"\"enabled\":true,";
			strSendBody +=      L"\"id\":2},";
			strSendBody +=    L"{\"name\":\"Brinker\",\"cardCategory\":null,\"imageUri\":\"https://app.giftango.com/GPCGraphics/CIR_000034_00.png\",\"vendorCount\":1,";
			strSendBody +=      L"\"termsAndConditions\":[null],";
			strSendBody +=      L"\"fixedAmounts\":[],";
			strSendBody +=      L"\"variableAmountRanges\":[{\"minimum\":5.0,\"maximum\":50.0}],";
			strSendBody +=      L"\"lastModificationTime\":\"2018-03-22T20:30:38.344482\",";
			strSendBody +=      L"\"feeAmount\":0.0,";
			strSendBody +=      L"\"enabled\":true,";
			strSendBody +=      L"\"id\":3},";
			strSendBody +=    L"{\"name\":\"Chili's\",\"cardCategory\":null,\"imageUri\":\"https://app.giftango.com/GPCGraphics/CIR_000069_04.png\",\"vendorCount\":1,";
			strSendBody +=      L"\"termsAndConditions\":[null],";
			strSendBody +=      L"\"fixedAmounts\":[],";
			strSendBody +=      L"\"variableAmountRanges\":[{\"minimum\":5.0,\"maximum\":50.0}],";
			strSendBody +=      L"\"lastModificationTime\":\"2018-03-22T20:30:38.344485\",";
			strSendBody +=      L"\"feeAmount\":0.0,";
			strSendBody +=      L"\"enabled\":true,";
			strSendBody +=      L"\"id\":5}";
		}
		else if( m_nGPCategoryId==1 )
		{
			strSendBody +=    L"{\"name\":\"Build-A-Bear\",\"cardCategory\":null,\"imageUri\":\"https://app.giftango.com/GPCGraphics/CIR_000048_08.png\",\"vendorCount\":2,";
	 		strSendBody +=      L"\"termsAndConditions\":[\"Use of this eGift Card constitutes acceptance of the following terms: Your Bear Bucks eGift Card can be redeemed for merchandise at participating Build-A-Bear Workshop locations in the United States and Puerto Rico, or online at buildabear.com. To use, present this eGift Card at the time of purchase. Purchase amount, plus applicable taxes, will be deducted from available balance until the value reaches zero. eGift Cards are non-refundable, cannot be exchanged for cash and cannot be replaced if lost or stolen, except as otherwise required by law. eGift Cards can be purchased for a minimum of $5 and the maximum of $150. Build-A-Bear Workshop, Inc. \"],";
	 		strSendBody +=      L"\"fixedAmounts\":[50.0,25.0],";
	 		strSendBody +=      L"\"variableAmountRanges\":[],";
	 		strSendBody +=      L"\"lastModificationTime\":\"2018-03-22T20:30:38.344484\",";
	 		strSendBody +=      L"\"feeAmount\":0.0,";
	 		strSendBody +=      L"\"enabled\":true,";
	 		strSendBody +=      L"\"id\":4},";
			strSendBody +=    L"{\"name\":\"Overstock.com\",\"cardCategory\":null,\"imageUri\":\"https://app.giftango.com/GPCGraphics/CIR_000058_00.png\",\"vendorCount\":1,";
	 		strSendBody +=     L"\"termsAndConditions\":[\"Gift Cards cannot be accepted as a payment method on our auctions, cars and real estate sites. Gift Cards are not exchangeable for cash. If your order exceeds the amount on your gift card(s), you must use another form of payment for the difference. Gift Cards cannot be purchased or used by international customers, shipped outside the continental U.S., or used to pay for international orders or products shipped outside the continental U.S. Check your balance by clicking here (https://www.overstock.com/gift-cards/check-gift-card-balance.html).\"],";
	 		strSendBody +=     L"\"fixedAmounts\":[],";
	 		strSendBody +=     L"\"variableAmountRanges\":[{\"minimum\":5.0,\"maximum\":50.0}],";
	 		strSendBody +=     L"\"lastModificationTime\":\"2018-03-22T20:30:38.344489\",";
	 		strSendBody +=     L"\"feeAmount\":0.0,\"enabled\":true,\"id\":11}";
		}
		else if( m_nGPCategoryId==6 )
		{
	 		strSendBody +=     L"{\"name\":\"Hotels.com\",\"cardCategory\":null,\"imageUri\":\"https://app.giftango.com/GPCGraphics/CIR_000988_00.png\",\"vendorCount\":1,";
	  		strSendBody +=       L"\"termsAndConditions\":[\"Terms and Conditions\nUsable up to balance only for new hotel bookings at www.hotels.com/gc. Only one gift card can be redeemed per online booking, but multiple gift cards may be combined into one gift card by visiting the balance transfer tab found at www.hotels.com/gcbalance. Not redeemable at hotel locations or if you choose the Pay at Hotel option online. Not redeemable toward existing bookings. Not redeemable for cash unless required by law. Not reloadable. Safeguard the card. Not replaced if lost or stolen. CARDCO CXXXIX, Inc. is the card issuer and sole obligor to card owner. CARDCO may delegate its issuer obligations to an assignee, without recourse. If delegated, the assignee, and not CARDCO, will be sole obligor to card owner. Card is distributed by Hotels.com. However, Hotels.com is not an obligor to card owner. Purchase, use or acceptance of card constitutes acceptance of these terms. For balance inquires visit www.hotels.com/gcbalance or call US Toll Free 1-888-999-4468.\"],";
	  		strSendBody +=       L"\"fixedAmounts\":[],";
	  		strSendBody +=       L"\"variableAmountRanges\":[{\"minimum\":10.0,\"maximum\":50.0}],";
	  		strSendBody +=       L"\"lastModificationTime\":\"2018-03-22T20:30:38.344486\",";
	  		strSendBody +=       L"\"feeAmount\":0.0,\"enabled\":true,";
	  		strSendBody +=       L"\"id\":6}";
		}
		strSendBody += L"], \"targetUrl\":null, \"success\":true,\"error\":null,\"unAuthorizedRequest\":false,\"__abp\":true";
	}
	else if( (m_nTransactionStep == GPAY_TRAN_PURCHASECARD_STAGE) || (m_nTransactionStep == GPAY_TRAN_PURCHASECARD_APPROVAL) )
	{
		int nTotalAmt = Asc2Int(m_strGPTranAmount);;
		if(m_nGPServiceType==GPAY_SERVICE_GIFTCARD)		nTotalAmt += Dollar2Cent(m_strCardFee);
		else											nTotalAmt += Dollar2Cent(m_strWirelessFee);
		if( m_strSelectedGreeting.GetLength() > 0 )		nTotalAmt += Dollar2Cent(m_strGreetingFee);

		strSendBody += L"\"result\":{";
        strSendBody +=		L"\"id\":138,";
        strSendBody +=		L"\"taxAmount\":0.0,";
        strSendBody +=		L"\"feeAmount\":";
		if(m_nGPServiceType==GPAY_SERVICE_GIFTCARD)		strSendBody += m_strCardFee;
		else											strSendBody += m_strWirelessFee;
		strSendBody +=		L",";
        strSendBody +=		L"\"grandTotal\":" + MakeMoneyCent(Int2Asc(nTotalAmt)) + L",";
        strSendBody +=		L"\"sender\":";
        strSendBody +=		L"{";

		if(	m_strGPHolderFName.GetLength() > 0)		strSendBody +=    L"\"name\": \""		+ m_strGPHolderFName + L"\",";
		if(	m_strGPHolderLName.GetLength() > 0)		strSendBody +=    L"\"surname\": \""	+ m_strGPHolderLName + L"\",";

        strSendBody +=			L"\"id\":67";
        strSendBody +=		L"}";
		strSendBody += L"},\"targetUrl\":null,\"success\":true,\"error\":null,\"unAuthorizedRequest\":false,\"__abp\":true";
	}	

	strSendBody += L"}";
	return strSendBody;
}

BOOL CGivePayData::ParseGPReceivedData(CString strReceived, BYTE* pBuffer, int nLength)
{
	NHDEBUG(DBG_INFO, (L"GivePay ParseGPReceivedData, TransactionStep = [%d], Data Length=[%d]\n", m_nTransactionStep, nLength));
	
	try		// [#J009] US Justin Bug Fix : AP Crash on GivePay Image Downloading
	{
		CString strDataValue = L"";

		if(m_nTransactionStep == GPAY_TRAN_GETTOKEN) 
		{
			// Extract "access_token"
			m_strGPToken = GetJSONNodeValue(strReceived, L"access_token");
			if(m_strGPToken.GetLength() <=0)
			{
				SetErrorCodeAndDesc(L"911");					
				return FALSE;
			}

			// Extract "expires_in"
			strDataValue = GetJSONNodeValue(strReceived, L"expires_in");
			if(strDataValue.GetLength() <=0)
			{
				SetErrorCodeAndDesc(L"912");					
				return FALSE;
			}

			// [#2557] US Justin 2018.06.12 Token Expiration
			int nTokenExpire = Asc2Int(strDataValue);
			if( nTokenExpire < 180)			// Margin 180 seconds (3 minutes for making a transaction)
			{
				SetErrorCodeAndDesc(L"912");					
				return FALSE;
			}
			CTimeSpan tokenSpan(nTokenExpire);
			CTime tmExp = CTime::GetCurrentTime() + tokenSpan;
			m_strGPTokenExpire.Format( L"%04d%02d%02d%02d%02d%02d", tmExp.GetYear(), tmExp.GetMonth(), tmExp.GetDay(), tmExp.GetHour(), tmExp.GetMinute(), tmExp.GetSecond());
			NHDEBUG(DBG_INFO, (L"GivePay Token Expire at [%s]\n", m_strGPTokenExpire));
			// End of [#2557]

			// Extract "token_type"
			m_strGPTokenType = GetJSONNodeValue(strReceived, L"token_type");
			if(m_strGPTokenType.GetLength() <=0)
			{
				SetErrorCodeAndDesc(L"913");					
				return FALSE;
			}
		}
		else if(m_nTransactionStep == GPAY_TRAN_DOWNLOAD_IMAGES) 
		{
			CString strFileName = GetImageFileName(m_sImageURL);
			if( (strFileName.GetLength()>4) && (nLength>4) )
			{
				BOOL bIsPNG = TRUE;
				CString strFExt = strFileName.Right(3);
				strFExt.MakeLower();
				if(strFExt.CompareNoCase(L"jpg") == 0)
					bIsPNG = FALSE;

				NHDEBUG(DBG_CALL, (L"FILE = [%s] (isPNG = [%d])\n", strFileName, bIsPNG));

				int i;
				for (i = 0; i < (nLength-4); i++)
				{
					if(bIsPNG)
					{
						if ((pBuffer[i] == 0x89) && (pBuffer[i + 1] == 'P') && (pBuffer[i + 2] == 'N') && (pBuffer[i + 3] == 'G'))
							break;
					}
					else
					{
						if ((pBuffer[i] == 0XFF) && (pBuffer[i + 1] == 0XD8) && (pBuffer[i + 2] == 0XFF))
							break;
					}
				}

				// the beginning of file (i) is found
				if( i < (nLength-4) )
				{
					NHDEBUG(DBG_CALL, (L"Beginning of file [%d] is found. \n", i));

					// [#2574] US Justin GivePay Enhancement3 Validate File Size
					CString strHttpHeader = L"";
					// [#J009] US Justin Bug Fix : AP Crash on GivePay Image Downloading
					for(int hdr=0; hdr<i; hdr++)	
						strHttpHeader += CString((char)pBuffer[hdr]);
					// End of [#J009]
					strHttpHeader.MakeUpper();
					int nLoc = strHttpHeader.Find(L"CONTENT-LENGTH");
					NHDEBUG(DBG_CALL, (L"Header=[%s]\nContent Length Location = [%d]\n", strHttpHeader, nLoc) );

					CString strFileSize = L"";
					CString strTemp;
					if(nLoc>=0)
					{
						for(int k=(nLoc+14); k<strHttpHeader.GetLength(); k++)
						{
							strTemp = strHttpHeader.Mid(k,1);
							NHDEBUG(DBG_CALL, (L"Size-Byte of Header = [%s]\n", strTemp) );
							if(	IsNum(strTemp) )
								strFileSize += strTemp;
							else if(strFileSize.GetLength() > 0)
								break;
						}
					}
					int nSizeOnHeader = Asc2Int(strFileSize);
					int nReceivedSize = nLength - i;
					NHDEBUG(DBG_CALL, (L"File Size on Header = [%d], Received data size = [%d] \n", nSizeOnHeader, nReceivedSize) );			
					if( (nReceivedSize>=(int)(0.95*nSizeOnHeader) ) && (nReceivedSize<=(int)(1.05*nSizeOnHeader)) )		// Preventing displaying partially downloaded image
					{
						CFile cf;
						if( cf.Open( strFileName, CFile::modeWrite | CFile::modeCreate | CFile::typeBinary ) == FALSE )
							NHDEBUG(DBG_CALL, (L"[FAIL TO OPEN(%s)]\n", strFileName));
						else
						{
							cf.Write(pBuffer + i, nReceivedSize );
							cf.Close();
							NHDEBUG(DBG_CALL, (L"IMAGE FILE(%s) SAVED SUCCESSFULLY]\n", strFileName));
						}
					}
					else
						NHDEBUG(DBG_CALL, (L"ERROR - Sizes are different  \n") );
					// end of [#2574]
				}
				else
					NHDEBUG(DBG_CALL, (L"ERROR.. NO FILE HEADER FOUND. Total size=[%d]. i=[%d]\n", nLength, i));
			}
			else
				NHDEBUG(DBG_CALL, (L"ERROR.. INVALID IMAGE FILE NAME [%s] or DATA LENGTH IS SHORT[%d]\n", strFileName, nLength));
		}
		else
		{
			// Extract and Check "success"
			strDataValue = GetJSONNodeValue(strReceived, L"success");
			if(strDataValue.GetLength() <=0)
			{
				SetErrorCodeAndDesc(L"901");					
				return FALSE;
			}
			if(strDataValue.CompareNoCase(L"TRUE") != 0)
			{
				SetErrorCodeAndDesc(L"902");					
				return FALSE;
			}

			// Extract and Check "result"
			strDataValue = GetJSONNodeValue(strReceived, L"result");
			if(strDataValue.GetLength() <=0)
			{
				SetErrorCodeAndDesc(L"903");					
				return FALSE;
			}

			if(m_nTransactionStep == GPAY_TRAN_ENROLLMENT) 
			{
				// Extract "encryptedClientCredentials"
				strDataValue = GetJSONNodeValue(strReceived, L"encryptedClientCredentials");
				if(strDataValue.GetLength() <=0)
				{
					SetErrorCodeAndDesc(L"906");					
					return FALSE;
				}

				CString strFirst11 = strDataValue.Left(11);
				if(strFirst11 == L"NO_DECRYPT_")	strDataValue = strDataValue.Mid(11);
				else								strDataValue = DecryptCredential(m_strPSK, m_strGPSalt, strDataValue);

				CStringArray	arrTemp;
				SplitString(strDataValue, L":", arrTemp);	
				if( arrTemp.GetSize() != 2 )
				{
					SetErrorCodeAndDesc(L"907");					
					return FALSE;
				}

				m_strClientID = arrTemp.GetAt(0);
				m_strClientSecret = arrTemp.GetAt(1);
				SaveSetting();
			}	
			else if( m_nTransactionStep == GPAY_TRAN_DOWNLOAD_FEES )
			{
				m_strCardFee = GetJSONNodeValue(strDataValue, L"giftCardFee");
				m_strWirelessFee = GetJSONNodeValue(strDataValue, L"wirelessFee");
				m_strGreetingFee = GetJSONNodeValue(strDataValue, L"greetingFee");
				NHDEBUG(DBG_INFO, (L"Fees : CardFee=[%s], WirelessFee=[%s], GreetingFee=[%s]\n", m_strCardFee, m_strWirelessFee, m_strGreetingFee));
			}
			else if( m_nTransactionStep == GPAY_TRAN_DOWNLOAD_CATEGORY )
			{
				if(!ParseAndFillGPItemArray(strDataValue, &m_arrCategory) )
				{
					SetErrorCodeAndDesc(L"921");					
					return FALSE;
				}
			}
			else if( m_nTransactionStep == GPAY_TRAN_DOWNLOAD_CARDS )
			{
				CString strTemp;
				// Pack cardCategory
				while(1)
				{
					strTemp = GetJSONNodeValue(strDataValue, L"cardCategory" );
					if(strTemp.GetLength() > 20)
						strDataValue.Replace(strTemp, L"_CARD_CATEGORY_");
					else
						break;
				}

				if(!ParseAndFillGPItemArray(strDataValue, &m_arrCards) )
				{
					SetErrorCodeAndDesc(L"922");					
					return FALSE;
				}
			}	
			else if( m_nTransactionStep == GPAY_TRAN_DOWNLOAD_GREETINGS )
			{
				if(!ParseAndFillGPItemArray(strDataValue, &m_arrGreetings) )
				{
					SetErrorCodeAndDesc(L"923");					
					return FALSE;
				}
			}		
			else if( m_nTransactionStep == GPAY_TRAN_DOWNLOAD_WIRELESS )
			{
				if(!ParseAndFillGPItemArray(strDataValue, &m_arrWireless) )
				{
					SetErrorCodeAndDesc(L"924");					
					return FALSE;
				}
			}		
			else if( (m_nTransactionStep == GPAY_TRAN_PURCHASECARD_STAGE) || (m_nTransactionStep == GPAY_TRAN_PURCHASECARD_APPROVAL) )		// Similar response. Same check points
			{	
				if(m_nTransactionStep == GPAY_TRAN_PURCHASECARD_STAGE)
				{
					// Initialize Buffer
					m_strStg_OrderId = m_strStg_ProductName = _T("");
					m_arrStg_ExtraFee.RemoveAll();
					m_strStg_UnitPrice = m_strStg_TaxAmt = m_strStg_FeeAmt = m_strStg_GifAmt = m_strStg_SubTotal = m_strStg_GrandTotal = _T("");
					m_strStg_Snd_FName = m_strStg_Snd_LName = m_strStg_Snd_FullName = m_strStg_Snd_Phone = _T("");
					m_strStg_Rcv_FName = m_strStg_Rcv_LName = m_strStg_Rcv_FullName = m_strStg_Rcv_Phone = _T("");
				
					// Extract Sender Name
					CString strCustomerInfo = GetJSONNodeValue(strDataValue, L"sender");
					if( strCustomerInfo.GetLength() > 5 )
					{
						m_strStg_Snd_FName = GetJSONNodeValue(strCustomerInfo, L"name");
						m_strStg_Snd_LName = GetJSONNodeValue(strCustomerInfo, L"surname");
						m_strStg_Snd_Phone = GetJSONNodeValue(strCustomerInfo, L"phoneNumber");

						strDataValue.Replace(strCustomerInfo, L"_sender_CONTENTS_");
					}

					// Assign sender name when no name returned
					if ( (m_strStg_Snd_FName.GetLength()<=0) && (m_strStg_Snd_LName.GetLength()<=0) )
					{
						m_strStg_Snd_FName = m_strGPHolderFName;
						m_strStg_Snd_LName = m_strGPHolderLName;
					}

					// Make Sender FULL NAME
					m_strStg_Snd_FullName = MakeFullName(m_strStg_Snd_FName, m_strStg_Snd_LName);
					NHDEBUG(DBG_INFO, (L"Sender FirstName=[%s], LastName=[%s], FullName=[%s]\n", m_strStg_Snd_FName, m_strStg_Snd_LName, m_strStg_Snd_FullName) );

					// Extract Recipient Name
					strCustomerInfo = GetJSONNodeValue(strDataValue, L"customer");
					if( strCustomerInfo.GetLength() > 5 )
					{
						m_strStg_Rcv_FName = GetJSONNodeValue(strCustomerInfo, L"name");
						m_strStg_Rcv_LName = GetJSONNodeValue(strCustomerInfo, L"surname");

						strDataValue.Replace(strCustomerInfo, L"_customer_CONTENTS_");
					}

					// Make Recipient FULL NAME
					m_strStg_Rcv_FullName = MakeFullName(m_strStg_Rcv_FName, m_strStg_Rcv_LName);
					NHDEBUG(DBG_INFO, (L"Recipient FirstName=[%s], LastName=[%s], FullName=[%s]\n", m_strStg_Rcv_FName, m_strStg_Rcv_LName, m_strStg_Rcv_FullName) );
				}
				else 
					m_strAuth_TrDateTime = _T("");

				// Pack Sub Items
				CString strSearch = GetJSONNodeValue(strDataValue, L"device" );
				if(strSearch.GetLength() > 5)		
					strDataValue.Replace(strSearch, L"_device_CONTENTS_");
				
				strSearch = GetJSONNodeValue(strDataValue, L"delivery" );
				if(strSearch.GetLength() > 5)		
				{
					m_strStg_Rcv_Phone = GetJSONNodeValue(strSearch, L"phoneNumber");
					NHDEBUG(DBG_INFO, (L"delivery.phoneNumber=[%s]\n", m_strStg_Rcv_Phone));
					strDataValue.Replace(strSearch, L"_delivery_CONTENTS_");
				}

				strSearch = GetJSONNodeValue(strDataValue, L"payments" );
				if(strSearch.GetLength() > 5)		
					strDataValue.Replace(strSearch, L"_payments_CONTENTS_");

				strSearch = GetJSONNodeValue(strDataValue, L"activity" );
				if(strSearch.GetLength() > 5)		
					strDataValue.Replace(strSearch, L"_activity_CONTENTS_");

				strSearch = GetJSONNodeValue(strDataValue, L"lineItems" );
				if(strSearch.GetLength() > 5)		
				{
					// Extract Product Name, Unit Price, Gif Price, and Extra Fees
					CString strLineItem = strSearch;
					NHDEBUG(DBG_INFO, (L"LineItems=[%s]\n", strLineItem));

					CString strExtraFee = GetJSONNodeValue(strLineItem, L"extraFees" );
					if( strExtraFee.GetLength() > 5)
					{
						ParseAndFillGPItemArray(strExtraFee, &m_arrStg_ExtraFee);
						strLineItem.Replace(strExtraFee, L"_extraFees_CONTENTS_");
					}
					NHDEBUG(DBG_INFO, (L"extra Fees : Number = [%d]\n", m_arrStg_ExtraFee.GetSize() ) );

					m_strStg_ProductName = GetJSONNodeValue(strLineItem, L"productName" );
					m_strStg_UnitPrice = GetJSONNodeValue(strLineItem, L"unitPrice" );
					NHDEBUG(DBG_INFO, (L"productName=[%s], unitPrice=[%s]\n", m_strStg_ProductName, m_strStg_UnitPrice));

					if(m_nGPServiceType == GPAY_SERVICE_GIFTCARD)
					{
						m_strStg_GifAmt = GetJSONNodeValue(strLineItem, L"gifPrice" );
						NHDEBUG(DBG_INFO, (L"gifPrice=[%s], lineTotal=[%s]\n", m_strStg_GifAmt, m_strStg_SubTotal));
					}
					strDataValue.Replace(strSearch, L"_lineItems_CONTENTS_");
				}

				strSearch = GetJSONNodeValue(strDataValue, L"customer" );
				if(strSearch.GetLength() > 5)		
					strDataValue.Replace(strSearch, L"_customer_CONTENTS_");

				// Get Information and Validate
				m_strStg_OrderId  = GetJSONNodeValue(strDataValue, L"id");
				m_strStg_TaxAmt	= GetJSONNodeValue(strDataValue, L"salesTaxAmount");
				m_strStg_FeeAmt	= GetJSONNodeValue(strDataValue, L"feeAmount");
				m_strStg_SubTotal = GetJSONNodeValue(strDataValue, L"subTotal" );
				m_strStg_GrandTotal = GetJSONNodeValue(strDataValue, L"grandTotal");
				NHDEBUG(DBG_INFO, (L"id = [%s], Tax=[%s], Fee=[%s], subTotal = [%s], Total=[%s]\n", m_strStg_OrderId, m_strStg_TaxAmt, m_strStg_FeeAmt, m_strStg_SubTotal, m_strStg_GrandTotal));

				// Check Order ID
				if( m_strStg_OrderId.GetLength() < 0 )
				{
					SetErrorCodeAndDesc(L"931");					
					return FALSE;
				}

				// Check Total Amount
				if( Dollar2Cent(m_strStg_GrandTotal) < Asc2Int(m_strGPTranAmount) )
				{
					SetErrorCodeAndDesc(L"932");					
					return FALSE;
				}

				if(m_nTransactionStep == GPAY_TRAN_PURCHASECARD_APPROVAL)
					m_strAuth_TrDateTime = GetJSONNodeValue(strDataValue, L"creationTime");		// 2018-04-06T18:20:54.51409
			}
		}
	}
	catch (CException* ex)
	{
		CString strTemp;
		strTemp.Format(L"GPAY EXCPT, STEP[%d]", m_nTransactionStep);
		NVDump('O', 'D', "00", L"99999", strTemp);

		NHDEBUG(DBG_INFO, (L"=========================================\n"));
		NHDEBUG(DBG_INFO, (L" \n"));
		NHDEBUG(DBG_INFO, (L" \n"));
		NHDEBUG(DBG_INFO, (L"EXCEPTION... GPAY PARSE DATA, STEP[%d]\n", m_nTransactionStep));
		NHDEBUG(DBG_INFO, (L" \n"));
		NHDEBUG(DBG_INFO, (L" \n"));
		NHDEBUG(DBG_INFO, (L"=========================================\n"));
		SetErrorCodeAndDesc(L"920");
		return FALSE;
	}

	NHDEBUG(DBG_INFO, (L"GivePay ParseGPReceivedData return TRUE\n"));
	return TRUE;
}

void CGivePayData::SetErrorCodeAndDesc(CString sResCode, LPCTSTR sErrText)
{
	m_strErrorCode.Format(L"DG%03s00", sResCode);
													///////1234567890123456789012345678901234567890
	if(		 sResCode == "900" )		m_strErrorDesc = L"COMMUNICATION ERROR";
	else if( sResCode == "901" )		m_strErrorDesc = L"Request Denied";
	else if( sResCode == "902" )		m_strErrorDesc = L"Request Denied";
	else if( sResCode == "903" )		m_strErrorDesc = L"No Result Received";
	else if( sResCode == "906" )		m_strErrorDesc = L"No Credential";
	else if( sResCode == "907" )		m_strErrorDesc = L"Credential Error";
	else if( sResCode == "911" )		m_strErrorDesc = L"No Token";
	else if( sResCode == "912" )		m_strErrorDesc = L"No Token Expiration";
	else if( sResCode == "913" )		m_strErrorDesc = L"No Token Type";
	else if( sResCode == "920" )		m_strErrorDesc = L"Data Parsing Error";				// [#J009] US Justin GivePay AP Crash Fix
	else if( sResCode == "921" )		m_strErrorDesc = L"Category Parsing Error";
	else if( sResCode == "922" )		m_strErrorDesc = L"Cards Parsing Error";
	else if( sResCode == "923" )		m_strErrorDesc = L"Greetings Parsing Error";
	else if( sResCode == "924" )		m_strErrorDesc = L"Wireless Carrier Parsing Error";
	else if( sResCode == "931" )		m_strErrorDesc = L"Staging Error (Order ID)";
	else if( sResCode == "932" )		m_strErrorDesc = L"Staging Error (Total Amount)";
	else								m_strErrorDesc = L"Transaction Error";

	if(sErrText!=NULL)					m_strErrorDesc = CString(sErrText);

	NHDEBUG(DBG_INFO, (L"SET ERROR CODE: ORG CODE=[%s], ATM CODE=[%s], DESC=[%s]\n", sResCode, m_strErrorDesc, m_strErrorDesc) );
	NVDump('O', 'C', "00", m_strErrorCode, L"GivePay ERR");
}

CString CGivePayData::MakeGPTransactionSummary(BOOL bSuccess, BOOL bPrint, int nMaxColumn)
{
	CString strLineFeed = L" ";
	CString strSeparator = L"----------------------------------------";
	// 1. Service or Card Name and Transaction Amount
	CString strReturn = GetDistributedString(m_strStg_ProductName, GetCurrencySymbol()+MakeMoneyCent(m_strGPTranAmount), nMaxColumn)  + SCR_RES_DELIMITER;

	// 2. Convenience Fee
	strReturn += GetDistributedString(L"Convenience Fee", GetCurrencySymbol()+MakeMoneyCent(Int2Asc(Dollar2Cent(m_strStg_FeeAmt))), nMaxColumn) + SCR_RES_DELIMITER;

	// 3.4 "Greeting + Tax" OR "Extra Fees"
	int nMaxAdditionalFeeNum = 0;
	if( (m_nGPServiceType == GPAY_SERVICE_GIFTCARD) && (m_strSelectedGreeting.GetLength()>0) && (Dollar2Cent(m_strStg_GifAmt)>0) )
	{
		// Greeting
		strReturn += GetDistributedString(L"Digital Greeting", GetCurrencySymbol()+MakeMoneyCent(Int2Asc(Dollar2Cent(m_strStg_GifAmt))), nMaxColumn) + SCR_RES_DELIMITER;
		if( (Dollar2Cent(m_strStg_TaxAmt) > 0) && (m_arrStg_ExtraFee.GetSize()>0) )
			nMaxAdditionalFeeNum = 1;
	}
	else
	{
		if( m_arrStg_ExtraFee.GetSize()>0)
			nMaxAdditionalFeeNum = 2;
	}
	if(nMaxAdditionalFeeNum>0)
	{
		int nMaxItemNum = m_arrStg_ExtraFee.GetSize();
		if(nMaxItemNum>nMaxAdditionalFeeNum) 
			nMaxItemNum = nMaxAdditionalFeeNum;

		CString strFeeText, strFeeAmount;
		for(int i=0; i<nMaxItemNum; ++i)
		{
			strFeeText = m_arrStg_ExtraFee.GetAt(i);
			strFeeAmount = MakeMoneyCent( Int2Asc( Dollar2Cent(GetJSONNodeValue(strFeeText, L"amount")) ) );
			strReturn += GetDistributedString(GetJSONNodeValue(strFeeText, L"label"), GetCurrencySymbol()+strFeeAmount, nMaxColumn) + SCR_RES_DELIMITER;
		}
	}

	// -------------------------------------
	if(bPrint)
		strReturn += strSeparator + SCR_RES_DELIMITER;

	// 5. Total Charged
	strReturn += GetDistributedString(L"Total Charged to Card", GetCurrencySymbol()+MakeMoneyCent(Int2Asc(Dollar2Cent(m_strStg_GrandTotal))), nMaxColumn) + SCR_RES_DELIMITER;

	// 6. Line Feed
	strReturn += strLineFeed + SCR_RES_DELIMITER;

	// Disclaimer 7,8,9,10,11
	if(bSuccess)
	{
		if(m_nGPServiceType == GPAY_SERVICE_GIFTCARD)
		{
			//                               1234567890123456789012345678901234567890	
			strReturn += GetCenteredString(L"Your Electronic Gift will be Sent via"		,nMaxColumn) + SCR_RES_DELIMITER;
			strReturn += GetCenteredString(L"Text Immediately"							,nMaxColumn) + SCR_RES_DELIMITER;
		}
		else
		{
			//                               1234567890123456789012345678901234567890	
			strReturn += GetCenteredString(L"Payment to your Wireless Account will"		,nMaxColumn) + SCR_RES_DELIMITER;
			strReturn += GetCenteredString(L"be applied within 30 minutes"				,nMaxColumn) + SCR_RES_DELIMITER;
		}

		if(bPrint)
			strReturn += strLineFeed + SCR_RES_DELIMITER;

		//                               1234567890123456789012345678901234567890	
		strReturn += GetCenteredString(L"Delivery times may vary, please allow"   ,nMaxColumn) + SCR_RES_DELIMITER;
		strReturn += GetCenteredString(L"up to 30 minutes for processing. Does"   ,nMaxColumn) + SCR_RES_DELIMITER;
		strReturn += GetCenteredString(L"not include ATM Withdraw Amounts"        ,nMaxColumn) + SCR_RES_DELIMITER;
	}
	else
	{
		// Transaction result and Error Description
		strReturn += CString(L"TRANSACTION FAILED") + SCR_RES_DELIMITER;
		strReturn += m_strErrorDesc + SCR_RES_DELIMITER;
		strReturn += strLineFeed + SCR_RES_DELIMITER;
	}

	if(bPrint)
		strReturn += strLineFeed + SCR_RES_DELIMITER;

	// 12,13                         1234567890123456789012345678901234567890	
	strReturn += GetCenteredString(L"For Support contact 1-833-GIVEPAY or"    ,nMaxColumn) + SCR_RES_DELIMITER;
	strReturn += GetCenteredString(L"visit GivePay.us/support"                ,nMaxColumn);
	return strReturn;
}

CString CGivePayData::MakeGPTrJournalData(CString strKindCode)
{
	int nStrLen;
	CString strEJNLData, strTemp, strTemp2;

	// 01. TERMINAL NUMBER -- 15 or 8 자리
	strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID);
	strTemp.TrimLeft();
	strEJNLData.Format(L"%s", strTemp);
	
	// 02. SEQUENCE NUMBER
	strTemp.Format(L"^----");
	strEJNLData += strTemp;
	
	// 03. TRANSACTION TYPE
	strTemp.Format(L"^%2.2s", TRANTYPE_GIVEPAY);
	strEJNLData += strTemp;

	// 04. TRANSACTION FROM ACCOUNT		// Credit Card
	strTemp.Format(L"^%2.2s",  L"CR");		
	strEJNLData += strTemp;
	
	// 05. TRANSACTION TO ACCOUNT	
	strTemp = L"^--";
	strEJNLData += strTemp;
	
	// 06. TRANSACTION BANK CODE		// Same as Main Transaction
	strTemp.Format(L"^%4.4s",MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSBANKCODE));
	strEJNLData += strTemp;
	
	// 07. TRANSACTION BANK ACCOUNT		// Fill with "*"
	strTemp2 = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSBANKACCOUNT);
	strTemp = CString('*', strTemp2.GetLength());
	strEJNLData += L"^";
	strEJNLData += strTemp;
	
	// Host Date and Time  2018-04-06T18:20:54.51409
	//                     0123456789012345678901234567890   
	if(m_strAuth_TrDateTime.GetLength()>=19)
	{		 
		// 08. TRANSACTION HOST DATE, MMDDYYYY
		strTemp.Format(L"^%s%s%s", m_strAuth_TrDateTime.Mid(5,2), m_strAuth_TrDateTime.Mid(8,2), m_strAuth_TrDateTime.Left(4));
		strEJNLData += strTemp;
		
		// 09. TRANSACTION HOST TIME, HHMMSS
		strTemp.Format(L"^%s%s%s", m_strAuth_TrDateTime.Mid(11,2), m_strAuth_TrDateTime.Mid(14,2), m_strAuth_TrDateTime.Mid(17,2));
		strEJNLData += strTemp;
	}
	else
	{
		// 08. TRANSACTION HOST DATE
		strTemp.Format(L"^--------");
		strEJNLData += strTemp;
		
		// 09. TRANSACTION HOST TIME
		strTemp.Format(L"^------");
		strEJNLData += strTemp;
	}

	// 10. TRANSACTION AVAIL BALANCE
	strTemp = L"^--------";
	strEJNLData += strTemp;
	
	// 11. TRANSACTION RETRIEVAL NUMBER
	strTemp.Format(L"^------------");
	strEJNLData += strTemp;
	
	// 12. TRANSACTION AUDIT NUMBER : Order Number
	strTemp.Format(L"^%-8.8s", m_strStg_OrderId);
	strEJNLData += strTemp;
	
	// 13. TRANSACTION ROUTING OR COMMMUNICATION ID
	if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE)
		strTemp2.Format(L"^%8.8s", MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_COMMUNICATIONID));		// STANDARD 3
	else
		strTemp2.Format(L"^%6.6s", MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ROUTINGID));					// STANDARD 1,2
	strTemp2.TrimLeft();
	strTemp2.TrimRight();
	strTemp.Format(L"%s", strTemp2);
	strEJNLData += strTemp;
	
	// 14. TRANSACTION SETTLE DATE MMDDYYYY
	if(m_strAuth_TrDateTime.GetLength()>=19)
		strTemp.Format(L"^%s%s%s", m_strAuth_TrDateTime.Mid(5,2), m_strAuth_TrDateTime.Mid(8,2), m_strAuth_TrDateTime.Left(4));
	else
		strTemp.Format(L"^--------");
	strEJNLData += strTemp;
	
	// 15. TRANSACTION SURCHARGE (CONVENIENCE FEE)
	//strTemp.Format(L"^%8.8s",  Int2Asc( Dollar2Cent(m_strStg_GrandTotal) - Dollar2Cent(m_strStg_UnitPrice)) );
	strTemp.Format(L"^%8.8s",  Int2Asc(Dollar2Cent(m_strStg_FeeAmt)) );		// [#J006]
	strEJNLData += strTemp;
	
	// 16. TRANSACTION REQUEST AMOUNT
	strTemp.Format(L"^%8.8s", m_strGPTranAmount);
	strEJNLData += strTemp;
	
	// 17. TRANSACTION DISPENSE AMOUNT
	if (strKindCode == NORMAL_TRX)
		strTemp.Format(L"^%8.8s", m_strGPTranAmount);
	else
		strTemp.Format(L"^%8.8s", L"000");
	strEJNLData += strTemp;
	
	// 18. TRANSACTION REMAIN AMOUNT
	strTemp.Format(L"^%12.12s", L"000");
	strEJNLData += strTemp;
	
	// 19. TRANSACTION PROC COUNT
	strTemp = L"^00";
	strEJNLData += strTemp;
	
	// 20. TRANSACTION RESULT
	if (strKindCode == NORMAL_TRX)		strTemp = L"^ TRUE";
	else								strTemp = L"^FALSE";
	strEJNLData += strTemp;
	
	// 21. TRANSACTION ERROR CODE
	strTemp.Format(L"^%7.7s", m_strErrorCode);
	strEJNLData += strTemp;

	// 22. TRANSACTION CARD DATA		// Same as Main Transaction
	strTemp2 = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSCARDDATA);
	nStrLen =  __min(strTemp2.GetLength()-4, 12);
	if (nStrLen <= 0)
		nStrLen = 0;
	strTemp = L"^";
	strTemp += CString('*', nStrLen);
	strTemp += strTemp2.Right(4);
	strEJNLData += strTemp;
	
	// 23. TRANSACTION NON-CASH VALUE
	strTemp = L"^ 000";
	strEJNLData += strTemp;
	
	// 24. TRANSACTION NON-CASH TYPE
	strTemp = L"^------------";
	strEJNLData += strTemp;
	
	// 25. TRANSACTION OTHER MESSAGE
	strTemp.Format(L"^%c%s%s%s", UNIT_DELIMITER, RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_GENERAL_COMMENT, GetJSONNodeValue(m_strSelectedCard, L"name"));
	if(	m_strSelectedGreeting.GetLength() > 0 )
		strTemp += L",Greeting:" + GetJSONNodeValue(m_strSelectedGreeting, L"defaultMessage");
	strEJNLData += strTemp;

	// 26. SURCHARGE DISPLAY TO CUTOMER
	strTemp = L"^1";
	strEJNLData += strTemp;

	return strEJNLData;
}


////////////////////////////////////////////////////////////
// Day Total Function
void CGivePayData::ResetTotal(int nService)
{
	NHDEBUG(DBG_INFO, (L"CGivePayData::ResetTotal()\n") );

	if(nService & GPAY_SERVICE_CARD_PURCHASE)
	{
		m_strCPDayTotalDateTime.Format(L"%s%s",GetDate(), GetTime());
		m_strCPDayTotalDispCount = L"0";
		m_strCPDayTotalDispAmt = L"0";		
	}

	SaveDispenseResult();
	// End of [#2496]
}

BOOL CGivePayData::ReadDispenseHistory()
{
	NHDEBUG(DBG_INFO, (L"CGivePayData::ReadDispenseHistory()\n") );
	CString strPath;
	CFile cf;

	///////////////////////////////////////////////////////////////
	// Read Dispense History
	///////////////////////////////////////////////////////////////
	strPath.Format(L"%s\\%s", ATM2_DATA_PATH, GPAY_TRANRESULT_FILE);
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
	
	// Daytotal Info  (3) :			Date and Time, Dispense Count, Transaction Amount
	if(arrTemp.GetSize() < 3)
	{
		NHDEBUG(DBG_INFO, (L"[Dispense Information is wrong or not set => Reset Dispense Result]\n"));
		SaveDispenseResult(TRUE);
		return TRUE;
	}

	BOOL bReadSuccessful = TRUE;

	// DayTotal Date and Time
	CString strTemp = arrTemp.GetAt(0);
	NHDEBUG(DBG_INFO, (L"   m_strCPDayTotalDateTime = [%s]\n", strTemp));
	if(strTemp.GetLength()==14)		m_strCPDayTotalDateTime = strTemp;
	else							bReadSuccessful = FALSE;

	// DayTotal Dispense Count
	if(bReadSuccessful)
	{
		strTemp = arrTemp.GetAt(1);
		NHDEBUG(DBG_INFO, (L"   m_strCPDayTotalDispCount = [%s]\n", strTemp));
		if(Asc2Int(strTemp)>=0)			m_strCPDayTotalDispCount = strTemp;
		else							bReadSuccessful = FALSE;
	}

	// DayTotal Total Dispense Amount
	if(bReadSuccessful)
	{
		strTemp = arrTemp.GetAt(2);
		NHDEBUG(DBG_INFO, (L"   m_strCPDayTotalDispAmt = [%s]\n", strTemp));
		if(Asc2Int(strTemp)>=0)			m_strCPDayTotalDispAmt = strTemp;
		else							bReadSuccessful = FALSE;
	}

	if(bReadSuccessful)
		NHDEBUG(DBG_INFO, (L"CGivePayData::ReadDispenseHistory () SUCCESS\n") );
	else
	{
		NHDEBUG(DBG_INFO, (L"[ERROR on Dispense Result Data]  => Reset Dispense Result\n"));
		SaveDispenseResult(TRUE);
	}	
	return TRUE;
}

BOOL CGivePayData::SaveDispenseResult(BOOL bReset)
{
	NHDEBUG(DBG_INFO, (L"CGivePayData::SaveDispenseResult(%d) \n", bReset));

	CString strPath;
	strPath.Format(L"%s\\%s", ATM2_DATA_PATH, GPAY_TRANRESULT_FILE);

	CFile cf;
	if( cf.Open( strPath, CFile::modeCreate|CFile::modeWrite) == FALSE )
	{
		NHDEBUG(DBG_INFO, (L"[FAIL TO OPEN(%s)]\n", strPath));
		return FALSE;
	}

	if(bReset)
	{
		m_strCPDayTotalDateTime.Format(L"%s%s",GetDate(), GetTime());
		m_strCPDayTotalDispCount = L"0";
		m_strCPDayTotalDispAmt = L"0";
	}

	CString strTotalDispenseData = L"";
	strTotalDispenseData += m_strCPDayTotalDateTime + _T(",");
	strTotalDispenseData += m_strCPDayTotalDispCount + _T(",");
	strTotalDispenseData += m_strCPDayTotalDispAmt;

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

void CGivePayData::RecodeDispenseResult(long nAmount /* Cent */, int nType)
{
	NHDEBUG(DBG_INFO, (L"CGivePayData::RecodeDispenseResult (Amt=%ld)\n", nAmount) );

	BOOL bSaveResult = FALSE;
	if(nType == GPAY_SERVICE_CARD_PURCHASE)
	{
		// Update Day Total ... Date Time
		if( m_strCPDayTotalDateTime.GetLength() != 14)
			m_strCPDayTotalDateTime.Format(L"%s%s", GetDate(), GetTime() );
		NHDEBUG(DBG_INFO, (L"  m_strCPDayTotalDateTime = [%s]\n", m_strCPDayTotalDateTime) );

		// Update Day Total ... Issue Count
		long nTemp = Asc2Int(m_strCPDayTotalDispCount) + 1;
		m_strCPDayTotalDispCount.Format(L"%ld", nTemp);
		NHDEBUG(DBG_INFO, (L"  m_strCPDayTotalDispCount = [%s]\n", m_strCPDayTotalDispCount) );

		// Update Day Total ... Dispense Amount
		nTemp = Asc2Int(m_strCPDayTotalDispAmt) + nAmount;	// CENT !!!!!!!
		m_strCPDayTotalDispAmt.Format(L"%ld", nTemp);
		NHDEBUG(DBG_INFO, (L"  m_strDCDayTotalDispAmt = [%s]\n", m_strCPDayTotalDispAmt) );
		bSaveResult = TRUE;
	}

	if(bSaveResult)
		SaveDispenseResult();
}

CString CGivePayData::GetDaytotalJournalData(int nType)
{
	NHDEBUG(DBG_INFO, (L"CGivePayData::GetDaytotalJournalData()\n") );

	CString strEJNLData = L"";
	if(nType == GPAY_SERVICE_CARD_PURCHASE)
	{
		strEJNLData += m_strCPDayTotalDateTime + _T("^");
		strEJNLData += m_strCPDayTotalDispCount + _T("^");
		strEJNLData += m_strCPDayTotalDispAmt;
	}

	NHDEBUG(DBG_INFO, (L"   Journal Data=[%s])\n", strEJNLData) );
	return strEJNLData;
}
#endif
// End of [#2445]