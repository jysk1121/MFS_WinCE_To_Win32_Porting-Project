#include "StdAfx.h"
#include ".\..\..\H\Tran\CPin4Data.h"
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

const CString TEST_TRANSACTION_AMOUNT = L"20.00";
const CString TEST_TRANSACTION_PHONE  = L"8009999999";
const CString TEST_TRANSACTION_PIN    = CString(ZERO4);
const CString TEST_TRANSACTION_CODE   = CString(ZERO4);

CPin4Data::CPin4Data()
{
	m_pPublicKey = NULL;
	m_pPrivateKey = m_pSymmetricKey = NULL;
	m_nLenSymmetricKey = m_nLenPrivateKey = 0;
	ResetSetting();
	m_strDayTotalDateTime = m_strDayTotalDispCount = m_strDayTotalDispAmt = L"";
	ResetTransactionData();
}

CPin4Data::~CPin4Data()
{
	// [#2515] US Justin 2017.11.14 MasterCard Cash Pickup - Prestaging
	if(m_pPublicKey !=  NULL)		delete [] m_pPublicKey;
	if(m_pPrivateKey != NULL)		delete [] m_pPrivateKey;
	if(m_pSymmetricKey != NULL)		delete [] m_pSymmetricKey;
	// End of [#2515]
}

void CPin4Data::ResetSetting()
{
	m_bPin4Available = FALSE;
	m_strPin4Logo = m_strPin4Logo_Button	= _T("");		
	m_strHostIP = m_strHostPort = m_strHostSSL = m_strHostProcess = m_strCommId = _T("");		// [#2524] Justin Download Certificate  (remove m_strHostSSLCert) // [#2431] US Justin 2016.06.13 Add PAI CommId

	// [#2515] US Justin 2017.11.14 MasterCard Cash Pickup - Prestaging
	m_strAuthHostIP = m_strAuthHostPort = m_strAuthHostSSL = m_strAuthHostProcess = m_strAuthClientID = _T("");
	if(m_pPublicKey !=  NULL)		delete [] m_pPublicKey;
	if(m_pPrivateKey != NULL)		delete [] m_pPrivateKey;
	if(m_pSymmetricKey != NULL)		delete [] m_pSymmetricKey;
	m_pPublicKey = NULL;
	m_pPrivateKey = m_pSymmetricKey = NULL;
	m_nLenSymmetricKey = m_nLenPrivateKey = 0;
	// End of [#2515]

	m_strMaxDispenseLimit = _T("");				// [#2433] US Justin 2016.06.23 Add MaxDispenseLimit
}

// [#RWC6-57] US William 2019.09.18 PAI MCCP Key presence
/**
 * Sets the transaction data to the data used for the test transaction. The test transaction
 * is used to obtain the MCCP symmetric key
 */
void CPin4Data::SetTestTransactionSettings()
{
	this->m_strUSPhoneNumber	= TEST_TRANSACTION_PHONE;
	this->m_strPin4Code			= TEST_TRANSACTION_PIN;
	this->m_strSecretPin		= TEST_TRANSACTION_CODE;
	this->m_strTranAmount		= TEST_TRANSACTION_AMOUNT;
	this->m_nTransactionStep	= PIN4_TRAN_GETTOKEN;
}

void CPin4Data::ResetTotal()
{
	NHDEBUG(DBG_INFO, (L"CPin4Data::ResetTotal()\n") );
	SaveDispenseResult(TRUE);
}

BOOL CPin4Data::ReadSettingAndDispenseHistory()
{
	NHDEBUG(DBG_INFO, (L"CPin4Data::ReadSettingAndDispenseHistory()\n") );
	m_bPin4Available = FALSE;

	CString strPath;
	CFile cf;

	///////////////////////////////////////////////////////////////
	// Read Setting
	///////////////////////////////////////////////////////////////
	strPath.Format(L"%s\\%s", ATM_DATA_PATH, PIN4_SETTING_FILE);
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
		if( strName.CompareNoCase(L"HostIP") == 0 )					m_strHostIP = strChildData;
		else if( strName.CompareNoCase(L"HostPort") == 0 )			m_strHostPort = strChildData;
		else if( strName.CompareNoCase(L"UseSSL") == 0 )			m_strHostSSL = strChildData;
		else if( strName.CompareNoCase(L"Process") == 0 )			m_strHostProcess = strChildData;
		//else if( strName.CompareNoCase(L"SSLCert") == 0 )			m_strHostSSLCert = strChildData;		// [#2524] US Justin PIN4 Download Certificate
		else if( strName.CompareNoCase(L"CommId") == 0 )			m_strCommId = strChildData;				// [#2431] US Justin PAI 2016.06.13 Add CommID for Pin4
		else if( strName.CompareNoCase(L"MaxDispenseLimit") == 0 )	m_strMaxDispenseLimit = strChildData;	// [#2433] US Justin PAI 2016.06.23 Add MaxDispenseLimit		
		else if( strName.CompareNoCase(L"Pin4Logo") == 0 )			m_strPin4Logo = strChildData;			
		else if( strName.CompareNoCase(L"Pin4Logo_Button") == 0 )	m_strPin4Logo_Button = strChildData;			
		// [#2515] US Justin 2017.11.14 PIN4 Prestaging
		else if( strName.CompareNoCase(L"Auth_HostIP") == 0 )		m_strAuthHostIP = strChildData;
		else if( strName.CompareNoCase(L"Auth_HostPort") == 0 )		m_strAuthHostPort = strChildData;
		else if( strName.CompareNoCase(L"Auth_UseSSL") == 0 )		m_strAuthHostSSL = strChildData;
		else if( strName.CompareNoCase(L"Auth_Process") == 0 )		m_strAuthHostProcess = strChildData;
		else if( strName.CompareNoCase(L"Auth_ClientID") == 0 )		m_strAuthClientID = strChildData;
		// End of [#2515]
	}

	if( xml.OutOfElem() == FALSE )
	{
		NHDEBUG(DBG_INFO, (L"[FAIL OUT OF <Setting>]\n"));
		ResetSetting();
		return FALSE;
	}

	if( (m_strHostIP.GetLength()>0)			&& (m_strHostPort.GetLength()>0)	&& (m_strHostSSL.GetLength()>0) && 
		(m_strHostProcess.GetLength()>0)	&& (m_strCommId.GetLength()>0) )	// [#2431] US Justin PAI 2016.06.13 Add CommID for Pin4
	{
		// [#2515] US Justin 2017.11.14 PIN4 Prestaging
		BOOL bValidData = TRUE;

		#if (APP_PRESTAGIN_PIN4)	// Check Authentication server as well for Prestaging.

			MoveCredentialData2ATM2Folder();									// [#2568] US Justin 2018.07.30 Move PIN4 Credential from ATM/DATA to ATM2/DATA

			bValidData = FALSE;

			if( (m_strAuthHostIP.GetLength()>0)		&& (m_strAuthHostPort.GetLength()>0)&& (m_strAuthHostSSL.GetLength()>0) && 
				(m_strAuthHostProcess.GetLength()>0)&& (m_strAuthClientID.GetLength()>0) )	
			{
				NHDEBUG(DBG_INFO, (L"PIN4 PRE Staging Mode\n"));
				bValidData = FALSE;

				// Read Public Key.....
				CFile datFile;
				CFileException e;
				CString strFileName;

				strFileName.Format(L"%s\\%s", ATM_DATA_PATH, PIN4_PUBLIC_KEY_FILE);
				if(datFile.Open(strFileName, CFile::modeRead, &e))
				{
					if(m_pPublicKey !=  NULL)		
						delete [] m_pPublicKey;
					long nSize = (long) datFile.GetLength();
					m_pPublicKey = new char[nSize+1];
					memset(m_pPublicKey, NULL, nSize+1);

					if( datFile.Read(m_pPublicKey, nSize) == nSize)
					{
						NHDEBUG(DBG_INFO, (L"PIN4 Public Key File=[%s], size=[%d] Loading OK\n", strFileName, nSize ));
						bValidData = TRUE;
					}
					else 
					{
						delete [] m_pPublicKey;
						m_pPublicKey = NULL;
					}						
					datFile.Close();
				}

				if(bValidData != TRUE)
					NHDEBUG(DBG_INFO, (L"PIN4 Public Key File=[%s] Loading ERROR... ERROR ERROR\n", strFileName ));

				// Read Private Key ...
				if(bValidData == TRUE)
				{
					strFileName.Format(L"%s\\%s", ATM2_DATA_PATH, PIN4_PRIVATE_KEY_FILE);		// [#2568] US Justin 2018.07.30 Move PIN4 Credential from ATM/DATA to ATM2/DATA
					if(datFile.Open(strFileName, CFile::modeRead, &e))
					{
						if(m_pPrivateKey !=  NULL)	
						{
							delete [] m_pPrivateKey;
							m_nLenPrivateKey = 0;
						}
						long nSize =  (long) datFile.GetLength();
						m_pPrivateKey = new unsigned char[nSize+1];
						memset(m_pPrivateKey, NULL, nSize+1);

						if( (nSize>2) && (datFile.Read(m_pPrivateKey, nSize)==nSize) )
						{
							m_nLenPrivateKey = nSize;
							NHDEBUG(DBG_INFO, (L"PIN4 Private Key File=[%s], size=[%d] Loading OK\n", strFileName, nSize ));
						}
						else
						{
							NHDEBUG(DBG_INFO, (L"PIN4 Private Key File=[%s] Loading ERROR\n", strFileName ));
							delete [] m_pPrivateKey;
							m_pPrivateKey = NULL;
							m_nLenPrivateKey = 0;
						}						
						datFile.Close();
					}
				}

				// Read Symmetric Key...
				if(bValidData == TRUE)
				{
					strFileName.Format(L"%s\\%s", ATM2_DATA_PATH, PIN4_SYMMETRIC_KEY_FILE);		// [#2568] US Justin 2018.07.30 Move PIN4 Credential from ATM/DATA to ATM2/DATA
					if(datFile.Open(strFileName, CFile::modeRead, &e))
					{
						if(m_pSymmetricKey !=  NULL)
						{
							delete [] m_pSymmetricKey;
							m_nLenSymmetricKey = 0;
						}

						long nSize =  (long) datFile.GetLength();
						m_pSymmetricKey = new unsigned char[nSize+1];
						memset(m_pSymmetricKey, NULL, nSize+1);

						if( (nSize>2) && (datFile.Read(m_pSymmetricKey, nSize)==nSize) )
						{
							m_nLenSymmetricKey = nSize;
							NHDEBUG(DBG_INFO, (L"PIN4 Symmetric Key File=[%s], size=[%d] Loading OK\n", strFileName, nSize ));
						}
						else
						{
							NHDEBUG(DBG_INFO, (L"PIN4 Symmetric Key File=[%s] Loading ERROR\n", strFileName ));
							delete [] m_pSymmetricKey;
							m_pSymmetricKey = NULL;
							m_nLenSymmetricKey = 0;
						}						
						datFile.Close();
					}
				}
			}
		#endif

		if(bValidData)
		{
			m_bPin4Available = TRUE;
			// [#2433] US Justin 2016.06.23 Add MaxDispenseLimit
			int nMaxDispenseLimit = Asc2Int(m_strMaxDispenseLimit);
			if( (nMaxDispenseLimit<100) || (nMaxDispenseLimit>1000) )
				m_strMaxDispenseLimit = L"500";
			// End of [#2433]
		}
		// End of [#2515
	}
	else
	{
		NHDEBUG(DBG_INFO, (L"[One or more paremeters are missing]\n"));
		ResetSetting();
		return FALSE;
	}
	
	///////////////////////////////////////////////////////////////
	// Read Dispense History
	///////////////////////////////////////////////////////////////
	strPath.Format(L"%s\\%s", ATM2_DATA_PATH, PIN4_DISPENSERESULT_FILE);
	if( cf.Open( strPath, CFile::modeRead) == FALSE )
	{
		NHDEBUG(DBG_INFO, (L"[FAIL TO OPEN(%s)]\n", strPath));
		SaveDispenseResult(TRUE);
		return FALSE;
	}

	nFileLen = (int)cf.GetLength();
	unsigned char* pBuffer1 = new unsigned char[nFileLen + 2];		// Allocate buffer for binary file data
	if( pBuffer1 == NULL )
	{
		NHDEBUG(DBG_INFO, (L"[FAIL TO NEW BUFFER(%d)]\n", nFileLen + 2));
		cf.Close();
		return FALSE;
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
		NHDEBUG(DBG_INFO, (L"[Dispense Information is wrong or not set]\n"));
		SaveDispenseResult(TRUE);
		return FALSE;
	}

	BOOL bReadSuccessful = TRUE;

	// DayTotal Date and Time
	CString strTemp = arrTemp.GetAt(0);
	NHDEBUG(DBG_INFO, (L"   m_strDayTotalDateTime = [%s]\n", strTemp));
	if(strTemp.GetLength()==14)		m_strDayTotalDateTime = strTemp;
	else							bReadSuccessful = FALSE;

	// DayTotal Dispense Count
	if(bReadSuccessful)
	{
		strTemp = arrTemp.GetAt(1);
		NHDEBUG(DBG_INFO, (L"   m_strDayTotalDispCount = [%s]\n", strTemp));
		if(Asc2Int(strTemp)>=0)			m_strDayTotalDispCount = strTemp;
		else							bReadSuccessful = FALSE;
	}

	// DayTotal Total Dispense Amount
	if(bReadSuccessful)
	{
		strTemp = arrTemp.GetAt(2);
		NHDEBUG(DBG_INFO, (L"   m_strDayTotalDispAmt = [%s]\n", strTemp));
		if(Asc2Int(strTemp)>=0)			m_strDayTotalDispAmt = strTemp;
		else							bReadSuccessful = FALSE;
	}

	if(!bReadSuccessful)
	{
		NHDEBUG(DBG_INFO, (L"[ERROR on Dispense Result Data]\n"));
		SaveDispenseResult(TRUE);
		return FALSE;
	}

	NHDEBUG(DBG_INFO, (L"CPin4Data::ReadDispenseHistory () RETURN TRUE \n") );
	return TRUE;
}

// [#2568] US Justin 2018.07.30 Move PIN4 Credential from ATM/DATA to ATM2/DATA
void CPin4Data::MoveCredentialData2ATM2Folder()
{
	NHDEBUG(DBG_INFO, (L"CPin4Data::MoveCredentialData2ATM2Folder() \n") );

	CString	strPathData, strPathATM2;
	BOOL	bSuccess;

	// Move Symmetric Key
	strPathData.Format(L"%s\\%s", ATM_DATA_PATH,  PIN4_SYMMETRIC_KEY_FILE);
	strPathATM2.Format(L"%s\\%s", ATM2_DATA_PATH, PIN4_SYMMETRIC_KEY_FILE);
	if(IsExistFile(strPathData))
	{
		DeleteFile(strPathATM2);
		bSuccess = MoveFile(strPathData, strPathATM2);
		NHDEBUG(DBG_INFO, (L"  Symmetric Key Moved [%s] => [%s] : Result=[%d]\n", strPathData, strPathATM2, bSuccess ) );
	}

	// Move Private Key
	strPathData.Format(L"%s\\%s", ATM_DATA_PATH,  PIN4_PRIVATE_KEY_FILE);
	strPathATM2.Format(L"%s\\%s", ATM2_DATA_PATH, PIN4_PRIVATE_KEY_FILE);
	if(IsExistFile(strPathData))
	{
		DeleteFile(strPathATM2);
		bSuccess = MoveFile(strPathData, strPathATM2);
		NHDEBUG(DBG_INFO, (L"  Private Key Moved [%s] => [%s] : Result=[%d]\n", strPathData, strPathATM2, bSuccess ) );
	}

	// Move Certificate File
	strPathData.Format(L"%s\\%s", ATM_DATA_PATH,  PIN4_CERTIFICATE_FILE);
	strPathATM2.Format(L"%s\\%s", ATM2_DATA_PATH, PIN4_CERTIFICATE_FILE);
	if(IsExistFile(strPathData))
	{
		DeleteFile(strPathATM2);
		bSuccess = MoveFile(strPathData, strPathATM2);
		NHDEBUG(DBG_INFO, (L"  Certificate File Moved [%s] => [%s] : Result=[%d]\n", strPathData, strPathATM2, bSuccess ) );
	}
}
// End of [#2568]

// [#2455] US Justin
BOOL CPin4Data::SavePin4Setting()
{
	NHDEBUG(DBG_INFO, (L"CPin4Data::SavePin4Setting() \n"));

	BYTE pBuffer[4096];
	CString strTemp, strData, strPath;

	CFile cf;
	strPath.Format(L"%s\\%s", ATM_DATA_PATH, PIN4_SETTING_FILE);
	if( cf.Open( strPath, CFile::modeWrite | CFile::modeCreate ) == FALSE  )
	{
		NHDEBUG(DBG_INFO, (L"[FAIL TO OPEN(%s)]\n", strPath));
		return FALSE;
	}

	strData  = L"<HalCash_Pin4>\r\n";
	strData += L"<Setting>\r\n";
	strData += L"<HostIP>"				+ m_strHostIP			+ L"</HostIP>\r\n";
	strData += L"<HostPort>"			+ m_strHostPort			+ L"</HostPort>\r\n";
	strData += L"<UseSSL>"				+ m_strHostSSL			+ L"</UseSSL>\r\n";
	strData += L"<Process>"				+ m_strHostProcess		+ L"</Process>\r\n";
	strData += L"<CommId>"				+ m_strCommId			+ L"</CommId>\r\n";
	strData += L"<MaxDispenseLimit>"	+ m_strMaxDispenseLimit	+ L"</MaxDispenseLimit>\r\n";
	strData += L"<Pin4Logo>"  			+ m_strPin4Logo			+ L"</Pin4Logo>\r\n";
	strData += L"<Pin4Logo_Button>"		+ m_strPin4Logo_Button	+ L"</Pin4Logo_Button>\r\n";

	// [#2515] US Justin 2017.11.14 PIN4 Prestaging
	if(!m_strAuthHostIP.IsEmpty())
		strData += L"<Auth_HostIP>"			+ m_strAuthHostIP		+ L"</Auth_HostIP>\r\n";
	if(!m_strAuthHostPort.IsEmpty())
		strData += L"<Auth_HostPort>"		+ m_strAuthHostPort		+ L"</Auth_HostPort>\r\n";
	if(!m_strAuthHostSSL.IsEmpty())
		strData += L"<Auth_UseSSL>"			+ m_strAuthHostSSL		+ L"</Auth_UseSSL>\r\n";
	if(!m_strAuthHostProcess.IsEmpty())
		strData += L"<Auth_Process>"		+ m_strAuthHostProcess	+ L"</Auth_Process>\r\n";
	if(!m_strAuthClientID.IsEmpty())
		strData += L"<Auth_ClientID>"		+ m_strAuthClientID		+ L"</Auth_ClientID>\r\n";
	// End of [#2515]

	strData += L"</Setting>\r\n";
	strData += L"</HalCash_Pin4>\r\n";

	memset(pBuffer, NULL, 4096);
	WideToMulti((LPSTR)pBuffer, strData, strData.GetLength());
	cf.Write( pBuffer, strData.GetLength() );
	cf.Close();

	/*
	// [#2515] US Justin 2017.11.14 PIN4 Prestaging ... SAVE PUBLIC AND PRIVATE KEY (PEM FORMAT)
	if (bSaveAll == TRUE)
	{
		// Save Public Key
		if(m_pPublicKey !=  NULL)
		{
			strPath.Format(L"%s\\%s", ATM_DATA_PATH, PIN4_PUBLIC_KEY_FILE);
			if( cf.Open( strPath, CFile::modeWrite | CFile::modeCreate ) == FALSE  )
			{
				NHDEBUG(DBG_INFO, (L"[FAIL TO OPEN(%s)]\n", strPath));
				return FALSE;
			}
			cf.Write( m_pPublicKey, strlen(m_pPublicKey));
			cf.Close();
		}

		// Save Private Key
		if(m_pPrivateKey !=  NULL)
		{
			strPath.Format(L"%s\\%s", ATM2_DATA_PATH, PIN4_PRIVATE_KEY_FILE);
			if( cf.Open( strPath, CFile::modeWrite | CFile::modeCreate ) == FALSE  )
			{
				NHDEBUG(DBG_INFO, (L"[FAIL TO OPEN(%s)]\n", strPath));
				return FALSE;
			}
			cf.Write( m_pPrivateKey, m_nLenPrivateKey);
			cf.Close();
		}
		// End of [#2515]

		// Save Symmetric Key
		if(m_pSymmetricKey !=  NULL)
		{
			strPath.Format(L"%s\\%s", ATM2_DATA_PATH, PIN4_SYMMETRIC_KEY_FILE);
			if( cf.Open( strPath, CFile::modeWrite | CFile::modeCreate ) == FALSE  )
			{
				NHDEBUG(DBG_INFO, (L"[FAIL TO OPEN(%s)]\n", strPath));
				return FALSE;
			}
			cf.Write( m_pSymmetricKey, m_nLenSymmetricKey);
			cf.Close();
		}
	}
	// End of [#2515]
	*/

	return TRUE;
}
// End of [#2455]

// [#2515] US Justin 2017.11.14 PIN4 Prestaging ... SAVE PUBLIC AND PRIVATE KEY (PEM FORMAT)
BOOL CPin4Data::AssignProcessorPrivateKey(unsigned char* key, int nLen)
{
	NHDEBUG(DBG_INFO, (L"CPin4Data::AssignProcessorPrivateKey( Length = [%d]) \n", nLen ));

	if(m_pPrivateKey !=  NULL)
	{
		delete [] m_pPrivateKey;
		m_nLenPrivateKey = 0;
	}

	m_pPrivateKey = new unsigned char[nLen + 1];
	memcpy( m_pPrivateKey, key, nLen);
	m_pPrivateKey[nLen] = 0;
	m_nLenPrivateKey = nLen;

	CFile cf;
	CString  strPath;
	strPath.Format(L"%s\\%s", ATM2_DATA_PATH, PIN4_PRIVATE_KEY_FILE);		// [#2568] US Justin 2018.07.30 Move PIN4 Credential from ATM/DATA to ATM2/DATA
	if( cf.Open( strPath, CFile::modeWrite | CFile::modeCreate ) == FALSE  )
	{
		NHDEBUG(DBG_INFO, (L"[FAIL TO OPEN(%s)]\n", strPath));
		return FALSE;
	}
	cf.Write( m_pPrivateKey, m_nLenPrivateKey);
	cf.Close();
	return TRUE;
}

BOOL CPin4Data::AssignSymmetricKey(unsigned char* key, int nLen)
{
	NHDEBUG(DBG_INFO, (L"CPin4Data::AssignSymmetricKey(Length = [%d]) \n", nLen ));

	if(m_pSymmetricKey !=  NULL)
	{
		delete [] m_pSymmetricKey;
		m_nLenSymmetricKey = 0;
	}

	m_pSymmetricKey = new unsigned char[nLen + 1];
	memcpy(m_pSymmetricKey, key, nLen);
	m_pSymmetricKey[nLen] = 0;
	m_nLenSymmetricKey = nLen;

	CFile cf;
	CString  strPath;
	strPath.Format(L"%s\\%s", ATM2_DATA_PATH, PIN4_SYMMETRIC_KEY_FILE);		// [#2568] US Justin 2018.07.30 Move PIN4 Credential from ATM/DATA to ATM2/DATA
	if( cf.Open( strPath, CFile::modeWrite | CFile::modeCreate ) == FALSE  )
	{
		NHDEBUG(DBG_INFO, (L"[FAIL TO OPEN(%s)]\n", strPath));
		return FALSE;
	}
	cf.Write( m_pSymmetricKey, m_nLenSymmetricKey);
	cf.Close();
	return TRUE;
}
// End of [#2515]

// [#2524] US Justin 2018.1.2 PIN4 Use Certificate
BOOL CPin4Data::AssignX509Certificate(unsigned char* cert, int nLen)
{
	NHDEBUG(DBG_INFO, (L"CPin4Data::AssignX509Certificate( Length = [%d]) \n", nLen ));

	BYTE* pCertData = NULL;
	int nCertLen = nLen;
		
	// PEM FORMATTED CERTIFICATE
	if( cert[0]=='-' && cert[1]=='-' && cert[2]=='-' && cert[3]=='-' && cert[4]=='-' && 
		cert[5]=='B' && cert[6]=='E' && cert[7]=='G' && cert[8]=='I' && cert[9]=='N' )
	{
		NHDEBUG(DBG_INFO, (L"[Certificate is PEM Formatted]\n"));
		pCertData = new BYTE[nLen+1];
		memcpy(pCertData, cert, nLen);
		nCertLen = nLen;
	}
	// DER FORMATTED CERFICATE => CONVERT TO PEM FORMATTED CERTIFICATE
	else			
	{
		NHDEBUG(DBG_INFO, (L"[Certificate is DER Formatted => CONVERT TO PEM FORMATTED CERTIFICATE]\n"));

		// Convert Binary data to BASE64 Data
		BYTE* pBase64Data = new BYTE[nLen*2];
		memset(pBase64Data, NULL, nLen*2);
		int nDataSize = base64_encode( (char*) cert, nLen, (char*) pBase64Data, nLen*2);
		CString strBase64Cert = CString (pBase64Data);
		strBase64Cert = strBase64Cert.Left(nDataSize);
		delete [] pBase64Data;

		// Convert to Formatted String
		CString strFormattedCert = L"-----BEGIN CERTIFICATE-----\r\n";
		BOOL bStopChanging = FALSE;
		do
		{
			if( strBase64Cert.GetLength() > 64 )
			{
				strFormattedCert += strBase64Cert.Left(64) + L"\r\n";
				strBase64Cert = strBase64Cert.Mid(64);
			}
			else
				bStopChanging = TRUE;
		} while(bStopChanging == FALSE);

		if(strBase64Cert.GetLength()>0)
			strFormattedCert += strBase64Cert + L"\r\n";
		strFormattedCert += L"-----END CERTIFICATE-----";

		pCertData = new BYTE[strFormattedCert.GetLength() + 1];
		memset(pCertData, NULL, strFormattedCert.GetLength() +1 );
		WideToMulti( (char*) pCertData, strFormattedCert, strFormattedCert.GetLength());
		nCertLen = strFormattedCert.GetLength();
	}

	CFile cf;
	CString  strPath;
	strPath.Format(L"%s\\%s", ATM2_DATA_PATH, PIN4_CERTIFICATE_FILE);		// [#2568] US Justin 2018.07.30 Move PIN4 Credential from ATM/DATA to ATM2/DATA
	if( cf.Open( strPath, CFile::modeWrite | CFile::modeCreate ) == FALSE  )
	{
		NHDEBUG(DBG_INFO, (L"[FAIL TO OPEN(%s)]\n", strPath));
		delete [] pCertData;
		return FALSE;
	}
	cf.Write( pCertData, nCertLen);
	cf.Close();

	delete [] pCertData;
	return TRUE;
}
// End of [#2524]
								
// [#2563] US Justin 2018.07.12 Reset cached PIN4
void CPin4Data::RemoveDownloadCredentials()
{
	CString  strPath;

	// Remove Private Key
	strPath.Format(L"%s\\%s", ATM2_DATA_PATH, PIN4_PRIVATE_KEY_FILE);		// [#2568] US Justin 2018.07.30 Move PIN4 Credential from ATM/DATA to ATM2/DATA
	DeleteFile(strPath);

	if(m_pPrivateKey !=  NULL)	
		delete [] m_pPrivateKey;
	m_pPrivateKey = NULL;
	m_nLenPrivateKey = 0;
	NHDEBUG(DBG_INFO, (L"Private Key file and buffer are removed\n" ));

	// Remove Symmetric Key
	strPath.Format(L"%s\\%s", ATM2_DATA_PATH, PIN4_SYMMETRIC_KEY_FILE);		// [#2568] US Justin 2018.07.30 Move PIN4 Credential from ATM/DATA to ATM2/DATA
	DeleteFile(strPath);

	if(m_pSymmetricKey !=  NULL)
		delete [] m_pSymmetricKey;
	m_pSymmetricKey = NULL;
	m_nLenSymmetricKey = 0;
	NHDEBUG(DBG_INFO, (L"Symmetric Key file and buffer are cleared\n" ));

	// Remove Certificate
	strPath.Format(L"%s\\%s", ATM2_DATA_PATH, PIN4_CERTIFICATE_FILE);		// [#2568] US Justin 2018.07.30 Move PIN4 Credential from ATM/DATA to ATM2/DATA
	DeleteFile(strPath);
	NHDEBUG(DBG_INFO, (L"Certificate file is cleared\n" ));
}

// [#J003] US Justin 2018.11.12 Mastercard Cash Pickup - Add OP Menu for removing Sym Key, Add an indicator for Sym Key Existance
BOOL CPin4Data::HaveSymmetricKey()
{
	if( (m_nLenSymmetricKey <=0) || (m_pSymmetricKey == NULL) )
		return false;
	return true;
}
// End of [#J003]

BOOL CPin4Data::SaveDispenseResult(BOOL bReset)
{
	NHDEBUG(DBG_INFO, (L"CPin4Data::SaveDispenseResult(%d) \n", bReset));

	CString strPath;
	strPath.Format(L"%s\\%s", ATM2_DATA_PATH, PIN4_DISPENSERESULT_FILE);

	CFile cf;
	if( cf.Open( strPath, CFile::modeCreate|CFile::modeWrite) == FALSE )
	{
		NHDEBUG(DBG_INFO, (L"[FAIL TO OPEN(%s)]\n", strPath));
		return FALSE;
	}

	if(bReset)
	{
		m_strDayTotalDateTime.Format(L"%s%s",GetDate(), GetTime());
		m_strDayTotalDispCount = L"0";
		m_strDayTotalDispAmt = L"0";
	}

	CString strTotalDispenseData = L"";
	strTotalDispenseData += m_strDayTotalDateTime + _T(",");
	strTotalDispenseData += m_strDayTotalDispCount + _T(",");
	strTotalDispenseData += m_strDayTotalDispAmt;

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

void CPin4Data::RecodeDispenseResult(long nAmount)
{
	NHDEBUG(DBG_INFO, (L"CPin4Data::RecodeDispenseResult (Amt=%ld)\n", nAmount) );

	m_strTranDispensed.Format(L"%d", (int) (0.01*nAmount + 0.001) );

	// Update Day Total ... Date Time
	if( m_strDayTotalDateTime.GetLength() != 14)
		m_strDayTotalDateTime.Format(L"%s%s", GetDate(), GetTime() );
	NHDEBUG(DBG_INFO, (L"  m_strDayTotalDateTime = [%s]\n", m_strDayTotalDateTime) );

	// Update Day Total ... Dispense Count
	long nTemp = Asc2Int(m_strDayTotalDispCount) + 1;
	m_strDayTotalDispCount.Format(L"%ld", nTemp);
	NHDEBUG(DBG_INFO, (L"  m_strDayTotalDispCount = [%s]\n", m_strDayTotalDispCount) );

	// Update Day Total ... Dispense Amount
	nTemp = Asc2Int(m_strDayTotalDispAmt) + Asc2Int(m_strTranDispensed);
	m_strDayTotalDispAmt.Format(L"%ld", nTemp);
	NHDEBUG(DBG_INFO, (L"  m_strDayTotalDispAmt = [%s]\n", m_strDayTotalDispAmt) );

	SaveDispenseResult();
}

CString CPin4Data::GetDaytotalJournalData()
{
	NHDEBUG(DBG_INFO, (L"CPin4Data::GetDaytotalJournalData()\n") );

	CString strEJNLData = L"";
	strEJNLData += m_strDayTotalDateTime + _T("^");
	strEJNLData += m_strDayTotalDispCount + _T("^");
	strEJNLData += m_strDayTotalDispAmt;

	NHDEBUG(DBG_INFO, (L"   Journal Data=[%s])\n", strEJNLData) );
	return strEJNLData;
}

CString CPin4Data::GetPin4FileName(int nType)
{
	CString sRtn = L"";

	if(nType==PIN4_FILE_LOGO_BUTTON)
		sRtn.Format(L"%s\\%s",ATM_DATA_PATH, m_strPin4Logo_Button);
	else if(nType==PIN4_FILE_LOGO_TOPCONER)
		sRtn.Format(L"%s\\%s",ATM_DATA_PATH, m_strPin4Logo);
	else if(nType==PIN4_FILE_SSLCERT)
	{
		// [#2524] US Justin 2018.01.02 Download Certificate
		/* 
		if( m_strHostSSLCert.GetLength()>0)		sRtn.Format(L"%s\\%s",ATM_DATA_PATH, m_strHostSSLCert);
		else									sRtn = _T("0");
		*/

		#if(APP_PRESTAGIN_PIN4)	
			sRtn.Format(L"%s\\%s%c%s\\%s", ATM2_DATA_PATH, PIN4_CERTIFICATE_FILE, FIELD_DELIMITER, ATM2_DATA_PATH, PIN4_PRIVATE_KEY_FILE);		// [#2568] US Justin 2018.07.30 Move PIN4 Credential from ATM/DATA to ATM2/DATA
			//sRtn = _T("0");		// Disable Host Certificate - USE ONLY FOR DEBUG PURPOSE
		#else
			sRtn = _T("0");
		#endif

		// End of [#2524]
	}
	return sRtn;	
}

void CPin4Data::ResetTransactionData()
{
	m_nTransactionStep = PIN4_TRAN_NONE;								// [#2515] US Justin 2017.11.14 MasterCard Cash Pickup - Prestaging	
	m_strP4AccessToken = m_strVCN = m_strExpirationYYMM = _T("");		// [#2515] US Justin 2017.11.14 MasterCard Cash Pickup - Prestaging	

	m_strUSPhoneNumber = m_strIntPhoneCountryCode = m_strIntPhoneNumber = _T("");
	m_strPin4Code = m_strSecretPin = _T("");
	//m_strTranSeqNo = m_strTID = _T("");
	m_strTranSeqNo = _T("");
	m_strTranAmount = m_strTranDispensed = _T("");
	m_strTranResCode = m_strTranResAuthNum = m_strTranResDate = m_strTranResTime = m_strTranResBizDate = _T("");
	m_strRevResCode = m_strErrorCode = m_strErrorDesc = _T("");
	m_nNeedReversal = 0;
	
	CString strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID);	
	strTemp.TrimRight();

	// [#2515] US Justin
	//m_strTID.Format(L"%015s", strTemp);
	#if(APP_PRESTAGIN_PIN4)
		m_strTID.Format(L"%s", strTemp);		
	#else
		m_strTID.Format(L"%015s", strTemp);		// TID : 15 digit "0" Padding Left "HYOSUNG" => "00000000HYOSUNG" (PHASE1)
	#endif
	// End of [#2515]
}

void CPin4Data::SetTransactionData()
{
	// Basic Information for Receipt
	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTYPE, TRANTYPE_WITHDRAWAL);
	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSFROMACCOUNT, L"CA");
	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTOACCOUNT, L"CA");
	
	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT, m_strTranAmount);
	MemSetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEAMT, L"000");
	MemSetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEFLAG, L"0");
}

// [#2405] US Justin 2016.03.15 HalCash
void CPin4Data::SetJournalRMSOtherMsg(bool bUserCancelMsg) // [#2515]
{
	NHDEBUG(DBG_INFO, (L"CPin4Data::SetJournalRMSOtherMsg / bUserCancelMsg=[%d]\n", bUserCancelMsg ) );
	CString strTemp, strTemp2;
	strTemp2 = _T("");

	// [#2515]
	if(bUserCancelMsg == true)
	{
		strTemp = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSOTHERMSG);
		strTemp.TrimRight();
		if(strTemp.IsEmpty() )
			strTemp2.Format(L"%c", UNIT_DELIMITER);
		strTemp2 += strTemp +  RMS_OTHERMSG_DELIMITER + RMS_OTHERMSG_FID_GENERAL_COMMENT + L"PIN RE-ENTRY USER CANCEL";
	}
	else
	{
		strTemp.Format(L"%c%s%s", UNIT_DELIMITER, RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_PIN4_PHONENUMBER);
		strTemp2 += strTemp;
		if(m_strUSPhoneNumber.GetLength()==10)		strTemp.Format(L"1-%s",  m_strUSPhoneNumber);
		else										strTemp.Format(L"%s-%s", m_strIntPhoneCountryCode, m_strIntPhoneNumber);
		strTemp2 += strTemp;
		strTemp.Format(L"%s%s%s", RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_PIN4_PIN4CODE,    m_strPin4Code);
		strTemp2 += strTemp;
		strTemp.Format(L"%s%s%s", RMS_OTHERMSG_DELIMITER, RMS_OTHERMSG_FID_PIN4_SECRETCODE,  m_strSecretPin);
		strTemp2 += strTemp;
	}
	// End of [#2515]
	NHDEBUG(DBG_CALL, (L"Other Message for Pin4 = [%s]\n", strTemp2));
	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSOTHERMSG, strTemp2);
}

CString	CPin4Data::MakeRequestMessage(LPCTSTR sStatus)
{
	NHDEBUG(DBG_INFO, (L"CPin4Data::MakeRequestMessage , Step=[%d]\n", m_nTransactionStep ) );

	CString strTemp;
	CString strSendBody = L"";
	
	// [#2515] US 2017.11.15 Add Prestaging
	if( m_nTransactionStep == PIN4_TRAN_GETTOKEN )
	{
		// Grant Type
		CString strEncrypted;
		strSendBody = L"{\"grant_type\":\"password\",";

		// User name = CommID|TerminalID
		strTemp.Format(L"%s|%s", m_strCommId, m_strTID);
		strSendBody += L"\"username\":\"" + strTemp + L"\",";

		// Password = BASE64( Encryt(Phone Number || Contrycode) )
		if(m_strUSPhoneNumber.GetLength()==10)	strTemp = L"1 " + m_strUSPhoneNumber;		
		else									strTemp = m_strIntPhoneCountryCode + L" " + m_strIntPhoneNumber;

		// Encrypt phone number using symkey or public key if symkey is not avaiable
		if(m_pSymmetricKey != NULL)		strEncrypted = Encrypt_And_Base64Encode(strTemp, (unsigned char*) m_pSymmetricKey, 2) ;		// Password (Phone Number)
		else							strEncrypted = Encrypt_And_Base64Encode(strTemp, (unsigned char*) m_pPublicKey, 1) ;		// Password (Phone Number)
		
		NHDEBUG(DBG_INFO, (L" Password(phoneNumber) Encryp_Base64Encode :[%s] => [%s]\n", strTemp, strEncrypted ));
		strSendBody += L"\"password\":\"" + strEncrypted  + L"\",";

		// Client ID = BASE64( Encryt(ClientID) )
		strTemp.Format(L"%s", m_strAuthClientID);

		// Encrypt Client ID using symkey or public key if symkey is not avaiable
		if(m_pSymmetricKey != NULL)		strEncrypted = Encrypt_And_Base64Encode(strTemp, (unsigned char*) m_pSymmetricKey, 2) ;		// Client ID
		else							strEncrypted = Encrypt_And_Base64Encode(strTemp, (unsigned char*) m_pPublicKey, 1) ;		// Client ID

		NHDEBUG(DBG_INFO, (L" client_id Encryp_Base64Encode :[%s] => [%s]\n", strTemp, strEncrypted ));
		strSendBody += L"\"client_id\":\"" + strEncrypted + L"\"}";
	}
	else if( (m_nTransactionStep == PIN4_TRAN_AUTH_PIN4) || (m_nTransactionStep == PIN4_TRAN_REVERSAL_PIN4) )
	{	
		///////////////////////////////////////////////////////////////////
		// Make Status Monitoring XML Data - without windows new line LF and CR
		CString strStatusMonitoring = L"";
		strTemp = CString(sStatus);
		if(strTemp.GetLength() >= 113)
		{
			strStatusMonitoring += L"<statusMonitoring xmlns=\"http://halcash.com/xmlTypes/triton\">";
			strStatusMonitoring +=		L"<programVersionNumber>";			strStatusMonitoring += strTemp.Mid(0,10);		strStatusMonitoring += L"</programVersionNumber>";
			strStatusMonitoring +=		L"<tableVersionNumber>";			strStatusMonitoring += strTemp.Mid(10,10);		strStatusMonitoring += L"</tableVersionNumber>";
			strStatusMonitoring +=		L"<firmwareVersionNumber>";			strStatusMonitoring += strTemp.Mid(20,10);		strStatusMonitoring += L"</firmwareVersionNumber>";
			strStatusMonitoring +=		L"<alarmChestDoorOpen>";			strStatusMonitoring += strTemp.Mid(30,1);		strStatusMonitoring += L"</alarmChestDoorOpen>";	
			// Top Door Open Status(1) is skipped
			strStatusMonitoring +=		L"<alarmSupervisorActive>";			strStatusMonitoring += strTemp.Mid(32,1);		strStatusMonitoring += L"</alarmSupervisorActive>";		
			strStatusMonitoring +=		L"<receiptPrinterPaperStatus>";		strStatusMonitoring += strTemp.Mid(33,1);		strStatusMonitoring += L"</receiptPrinterPaperStatus>";		
			strStatusMonitoring +=		L"<receiptPrinterRibbonStatus>";	strStatusMonitoring += strTemp.Mid(34,1);		strStatusMonitoring += L"</receiptPrinterRibbonStatus>";		
			strStatusMonitoring +=		L"<journalPrinterPaperStatus>";		strStatusMonitoring += strTemp.Mid(35,1);		strStatusMonitoring += L"</journalPrinterPaperStatus>";		
			strStatusMonitoring +=		L"<journalPrinterRibbonStatus>";	strStatusMonitoring += strTemp.Mid(36,1);		strStatusMonitoring += L"</journalPrinterRibbonStatus>";		
			strStatusMonitoring +=		L"<noteStatusDispenser>";			strStatusMonitoring += strTemp.Mid(37,1);		strStatusMonitoring += L"</noteStatusDispenser>";		
			strStatusMonitoring +=		L"<receiptPrinter>";				strStatusMonitoring += strTemp.Mid(38,1);		strStatusMonitoring += L"</receiptPrinter>";		
			strStatusMonitoring +=		L"<journalPrinter>";				strStatusMonitoring += strTemp.Mid(39,1);		strStatusMonitoring += L"</journalPrinter>";		
			strStatusMonitoring +=		L"<dispenser>";						strStatusMonitoring += strTemp.Mid(40,1);		strStatusMonitoring += L"</dispenser>";		
			strStatusMonitoring +=		L"<communicationsSystem>";			strStatusMonitoring += strTemp.Mid(41,1);		strStatusMonitoring += L"</communicationsSystem>";		
			strStatusMonitoring +=		L"<cardReader>";					strStatusMonitoring += strTemp.Mid(42,1);		strStatusMonitoring += L"</cardReader>";		
			strStatusMonitoring +=		L"<cardsRetained>000</cardsRetained>";		
			strStatusMonitoring +=		L"<electronicSystem>00</electronicSystem>";		
			strStatusMonitoring +=		L"<currentErrorCodeForTerminal>";	strStatusMonitoring += strTemp.Mid(48,3);		strStatusMonitoring += L"</currentErrorCodeForTerminal>";		
			strStatusMonitoring +=		L"<communicationsFailures>";		strStatusMonitoring += strTemp.Mid(51,3);		strStatusMonitoring += L"</communicationsFailures>";		
			strStatusMonitoring +=		L"<cassesetteA>";					
			strStatusMonitoring +=			L"<denomination>";				strStatusMonitoring += strTemp.Mid(54,3);		strStatusMonitoring += L"</denomination>";		
			strStatusMonitoring +=			L"<loaded>";					strStatusMonitoring += strTemp.Mid(57,4);		strStatusMonitoring += L"</loaded>";		
			strStatusMonitoring +=			L"<dispensed>";					strStatusMonitoring += strTemp.Mid(61,4);		strStatusMonitoring += L"</dispensed>";		
			strStatusMonitoring +=			L"<rejected>";					strStatusMonitoring += strTemp.Mid(65,3);		strStatusMonitoring += L"</rejected>";		
			strStatusMonitoring +=		L"</cassesetteA>";		
			strStatusMonitoring +=		L"<cassesetteB>";					
			strStatusMonitoring +=			L"<denomination>";				strStatusMonitoring += strTemp.Mid(68,3);		strStatusMonitoring += L"</denomination>";		
			strStatusMonitoring +=			L"<loaded>";					strStatusMonitoring += strTemp.Mid(71,4);		strStatusMonitoring += L"</loaded>";		
			strStatusMonitoring +=			L"<dispensed>";					strStatusMonitoring += strTemp.Mid(75,4);		strStatusMonitoring += L"</dispensed>";		
			strStatusMonitoring +=			L"<rejected>";					strStatusMonitoring += strTemp.Mid(79,3);		strStatusMonitoring += L"</rejected>";		
			strStatusMonitoring +=		L"</cassesetteB>";		
			strStatusMonitoring +=		L"<cassesetteC>";					
			strStatusMonitoring +=			L"<denomination>";				strStatusMonitoring += strTemp.Mid(82,3);		strStatusMonitoring += L"</denomination>";		
			strStatusMonitoring +=			L"<loaded>";					strStatusMonitoring += strTemp.Mid(85,4);		strStatusMonitoring += L"</loaded>";		
			strStatusMonitoring +=			L"<dispensed>";					strStatusMonitoring += strTemp.Mid(89,4);		strStatusMonitoring += L"</dispensed>";		
			strStatusMonitoring +=			L"<rejected>";					strStatusMonitoring += strTemp.Mid(93,3);		strStatusMonitoring += L"</rejected>";		
			strStatusMonitoring +=		L"</cassesetteC>";		
			strStatusMonitoring +=		L"<cassesetteD>";					
			strStatusMonitoring +=			L"<denomination>";				strStatusMonitoring += strTemp.Mid(96,3);		strStatusMonitoring += L"</denomination>";		
			strStatusMonitoring +=			L"<loaded>";					strStatusMonitoring += strTemp.Mid(99,4);		strStatusMonitoring += L"</loaded>";		
			strStatusMonitoring +=			L"<dispensed>";					strStatusMonitoring += strTemp.Mid(103,4);		strStatusMonitoring += L"</dispensed>";		
			strStatusMonitoring +=			L"<rejected>";					strStatusMonitoring += strTemp.Mid(107,3);		strStatusMonitoring += L"</rejected>";		
			strStatusMonitoring +=		L"</cassesetteD>";		
			strStatusMonitoring +=		L"<totalNotesPurged>";				strStatusMonitoring += strTemp.Mid(110,3);		strStatusMonitoring += L"</totalNotesPurged>";		
			strStatusMonitoring += L"</statusMonitoring>";
		}
		NHDEBUG(DBG_INFO, (L"Pin4 Status Monitoring Data = \r\n%s \n", strStatusMonitoring));		

		///////////////////////////////////////////////////////////////////
		// Request Base
		CString strRequestBase = L"";
		strTemp = MemGetStr(_MEM_FLD_APP_VERSIONINFO, _MEM_VAR_APP_AP_VERSION);
		strRequestBase += L"<requestBase>";
		strRequestBase +=		L"<commId>";				strRequestBase += m_strCommId;			strRequestBase += L"</commId>";		
		strRequestBase +=		L"<idProvider>HYOSUNG</idProvider>";
		strRequestBase +=		L"<termIdentifier>td</termIdentifier>";
		strRequestBase +=		L"<softwareVersion>";		strRequestBase += strTemp.Right(2);		strRequestBase += L"</softwareVersion>";
		strRequestBase +=		L"<encryptMode>2</encryptMode>";
		strRequestBase += L"</requestBase>";
		NHDEBUG(DBG_INFO, (L"Pin4 requestBase Data = \r\n%s \n", strRequestBase));

		///////////////////////////////////////////////////////////////////
		// OPEN SOAP Message BODY 
		CString strSoapMessabeBody = L"";

		// [#2515]
		// Add Soap Body Message <SOAP:BODY> ....</SOAP:BODY>
		#if (APP_PRESTAGIN_PIN4)	// Check Authentication server as well for Prestaging.
			strSoapMessabeBody += L"<SOAP:Body xmlns:SOAP=\"http://schemas.xmlsoap.org/soap/envelope/\" Id=\"Body\">";
		#else
			strSoapMessabeBody += L"<SOAP:Body>";
		#endif
		// End of [#2515]

		///////////////////////////////////////////////////////////////////
		// SOAP BODY : Adding Status Monitoring Field and Request Base
		if( m_nTransactionStep == PIN4_TRAN_AUTH_PIN4 )				strSoapMessabeBody += _T("<tns:processAuthorization");
		else if( m_nTransactionStep == PIN4_TRAN_REVERSAL_PIN4 )	strSoapMessabeBody += _T("<tns:processReversal");

		// [#2515] 
		#if (APP_PRESTAGIN_PIN4)	// Check Authentication server as well for Prestaging.
			strSoapMessabeBody += " xmlns:tns=\"http://servers.webServices.processor.halcash.com/\">";
		#else
			strSoapMessabeBody += " xmlns:tns=\"http://genmega.servers.webServices.processor.halcash.com/\">";
		#endif		
			// End of [#2515]

		// Request Type
		if( m_nTransactionStep == PIN4_TRAN_AUTH_PIN4 )				strSoapMessabeBody += L"<AuthorizationRequest>";
		else if( m_nTransactionStep == PIN4_TRAN_REVERSAL_PIN4 )	strSoapMessabeBody += L"<ReversalRequest>";

		// Terminal ID
		strSoapMessabeBody += L"<termId xmlns=\"http://halcash.com/xmlTypes/triton\">";		
		strSoapMessabeBody += m_strTID;	
		strSoapMessabeBody += L"</termId>";

		// Transacton Code
		if( m_nTransactionStep == PIN4_TRAN_AUTH_PIN4 )
		{
			strSoapMessabeBody += L"<transCode xmlns=\"http://halcash.com/xmlTypes/triton\">80</transCode>";
		} 
		else if( m_nTransactionStep == PIN4_TRAN_REVERSAL_PIN4 )
		{
			strSoapMessabeBody += L"<transCode xmlns=\"http://halcash.com/xmlTypes/triton\">29</transCode>";
		}

		// Sequence Number
		strSoapMessabeBody += L"<seqNum xmlns=\"http://halcash.com/xmlTypes/triton\">";
		strSoapMessabeBody += m_strTranSeqNo;
		strSoapMessabeBody += L"</seqNum>";

		// Adding Status Monitoring
		strSoapMessabeBody += strStatusMonitoring;

		// Adding Request Base
		strSoapMessabeBody += strRequestBase;

		// Amount One and Two
		// Pin4 Allows only FULL REVERSAL
		strSoapMessabeBody +=		L"<amountOne xmlns=\"http://halcash.com/xmlTypes/triton\">";		strSoapMessabeBody += m_strTranAmount;				strSoapMessabeBody += L"</amountOne>";
		strSoapMessabeBody +=		L"<amountTwo xmlns=\"http://halcash.com/xmlTypes/triton\">0</amountTwo>";

		// Secret Code and SMS reference
		strSoapMessabeBody +=		L"<secretCode xmlns=\"http://halcash.com/xmlTypes/triton\">";	strSoapMessabeBody += m_strSecretPin;				strSoapMessabeBody += L"</secretCode>";
		strSoapMessabeBody +=		L"<smsReference xmlns=\"http://halcash.com/xmlTypes/triton\">";	strSoapMessabeBody += m_strPin4Code;				strSoapMessabeBody += L"</smsReference>";

		// Telephone Number
		strSoapMessabeBody +=		L"<telephone prefix=\""; 
		if(m_strUSPhoneNumber.GetLength()==10)
		{
			strSoapMessabeBody +=		L"1\" xmlns=\"http://halcash.com/xmlTypes/triton\">";	
			strSoapMessabeBody +=		m_strUSPhoneNumber;		
		}
		else
		{
			strSoapMessabeBody +=		m_strIntPhoneCountryCode; 
			strSoapMessabeBody +=		L"\">";							
			strSoapMessabeBody +=		m_strIntPhoneNumber;
		}
		strSoapMessabeBody +=		L"</telephone>";

		// Currency
		strSoapMessabeBody +=		L"<transactionCurrency xmlns=\"http://halcash.com/xmlTypes/triton\">USD</transactionCurrency>";

		// Date and Time
		CTime ct = CTime::GetCurrentTime();
		CString strTemp2;
		strTemp2.Format(L"%04d", ct.GetYear());
		// Date
		strTemp.Format(L"%02d%02d%s",  ct.GetMonth(), ct.GetDay(), strTemp2.Right(2) );
		strSoapMessabeBody +=		L"<transactionDate xmlns=\"http://halcash.com/xmlTypes/triton\">";	strSoapMessabeBody += strTemp;	strSoapMessabeBody += L"</transactionDate>";
		// Time
		strTemp.Format(L"%02d%02d%02d",  ct.GetHour(), ct.GetMinute(), ct.GetSecond() );
		strSoapMessabeBody +=		L"<transactionTime xmlns=\"http://halcash.com/xmlTypes/triton\">";	strSoapMessabeBody += strTemp;	strSoapMessabeBody += L"</transactionTime>";

		NHDEBUG(DBG_INFO, (L"Pin4 strSoapMessabeBody Data(without status monitoring and request base) = \r\n%s \n", strSoapMessabeBody.Left(512)));
		
		// Request Type
		if( m_nTransactionStep == PIN4_TRAN_AUTH_PIN4 )				strSoapMessabeBody += L"</AuthorizationRequest>";
		else if( m_nTransactionStep == PIN4_TRAN_REVERSAL_PIN4 )	strSoapMessabeBody += L"</ReversalRequest>";

		if( m_nTransactionStep == PIN4_TRAN_AUTH_PIN4 )				strSoapMessabeBody += L"</tns:processAuthorization>";
		else if( m_nTransactionStep == PIN4_TRAN_REVERSAL_PIN4 )	strSoapMessabeBody += L"</tns:processReversal>";

		///////////////////////////////////////////////////////////////////
		// CLOSE SOAP Message BODY 
		strSoapMessabeBody += L"</SOAP:Body>";

		///////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////
		// MAKING WHOLE REQUEST MESSAGE
		///////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////
		// Request Message - Type Definition
		//strSendBody = L"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\r\n";		
		strSendBody = L"";		

		// Soap Envelope
		strSendBody += L"<SOAP:Envelope xmlns:SOAP=\"http://schemas.xmlsoap.org/soap/envelope/\" ";
		strSendBody += L"xmlns:wsse=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\" ";
		strSendBody += L"xmlns:wsu=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd\">";

		// Soap Header with signature (ONLY FOR Prestaging Mode)
		#if(APP_PRESTAGIN_PIN4)

			// SOAP Header with signature
			/*
			<SOAP:Header>
			<SOAP-SEC:Security
				  xmlns:SOAP-SEC="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd" SOAP:mustUnderstand="1">
				 <wsse:BinarySecurityToken EncodingType="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-soap-message-security-1.0#Base64Binary" wsu:Id="binarytoken" ValueType="http://www.pin4.com#CustomToken">a3NzczA5aHIzODFyIyghISlf</wsse:BinarySecurityToken>
				 <ds:Signature xmlns:ds="http://www.w3.org/2000/09/xmldsig#">
					<ds:SignedInfo xmlns:ds=\"http://www.w3.org/2000/09/xmldsig#\">		
						<ds:CanonicalizationMethod Algorithm="http://www.w3.org/2001/10/xml-exc-c14n#"></ds:CanonicalizationMethod>
						<ds:SignatureMethod Algorithm="http://www.w3.org/2001/04/xmldsig-more#rsa-sha256"></ds:SignatureMethod>
						<ds:Reference URI="#Body">
							<ds:Transforms>
                            <ds:Transform Algorithm="http://www.w3.org/2001/10/xml-exc-c14n#">
                                <ec:InclusiveNamespaces xmlns:ec="http://www.w3.org/2001/10/xml-exc-c14n#" PrefixList="ser"></ec:InclusiveNamespaces>
                            </ds:Transform>
							</ds:Transforms>
							<ds:DigestMethod Algorithm="http://www.w3.org/2001/04/xmlenc#sha256"></ds:DigestMethod>
							<ds:DigestValue>9x0mZhajy9dHKuIXh7bm0khuC7M=</ds:DigestValue>
						</ds:Reference>
					</ds:SignedInfo>
				<ds:SignatureValue>CiKztXFr2HnE1ul1S5OrJpiYCV46MJ9jEiDaU7AkAiCsgkTDxAhzyA==</ds:SignatureValue>
				<ds:KeyInfo>
                ////// OLD<wsse:SecurityTokenReference xmlns:wsse="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd" xmlns:wsu="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd">
                <wsse:SecurityTokenReference>
                <wsse:Reference URI="#binarytoken" ValueType="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-x509-token-profile-1.0#X509v3"></wsse:Reference>
                </wsse:SecurityTokenReference>
				</ds:KeyInfo>
				</ds:Signature>
			</SOAP-SEC:Security>
			</SOAP:Header>
			*/

			CString strSoapHeader  = L"<SOAP:Header>";
			strSoapHeader += L"<SOAP-SEC:Security ";
			strSoapHeader += L"xmlns:SOAP-SEC=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\" SOAP:mustUnderstand=\"1\">";
			strSoapHeader += L"<wsse:BinarySecurityToken EncodingType=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-soap-message-security-1.0#Base64Binary\" ";
			strSoapHeader +=  L"wsu:Id=\"binarytoken\" ValueType=\"http://www.pin4.com#CustomToken\">" + m_strP4AccessToken + "</wsse:BinarySecurityToken>";
			strSoapHeader += L"<ds:Signature xmlns:ds=\"https://en.wikipedia.org/wiki/XML_Signature\">";

			// Message Body Digest (SHA256) 
			char* pMsgBody = new char[ strSoapMessabeBody.GetLength()+1 ];
			memset(pMsgBody, NULL, strSoapMessabeBody.GetLength()+1 );
			WideToMulti( pMsgBody, strSoapMessabeBody, strSoapMessabeBody.GetLength());
			CString strDigest = SHA_256_Encrypt((unsigned char *)pMsgBody, strSoapMessabeBody.GetLength(), 1);																// [#2524] US Justin 2018.01.03 Add return type
			NHDEBUG(DBG_INFO, (L" SoapBody Length=[%d] / Sha256 Digest = [%s]\n", strSoapMessabeBody.GetLength(), strDigest));
			delete [] pMsgBody;
			CString strSignBlck = L"";			
			strSignBlck += L"<ds:SignedInfo xmlns:ds=\"http://www.w3.org/2000/09/xmldsig#\">";
			strSignBlck += L"<ds:CanonicalizationMethod Algorithm=\"http://www.w3.org/2001/10/xml-exc-c14n#\"></ds:CanonicalizationMethod>";																	  	
			strSignBlck += L"<ds:SignatureMethod Algorithm=\"http://www.w3.org/2001/04/xmldsig-more#rsa-sha256\"></ds:SignatureMethod>";
			strSignBlck += L"<ds:Reference URI=\"#Body\">";
			strSignBlck += L"<ds:Transforms>";
            strSignBlck += L"<ds:Transform Algorithm=\"http://www.w3.org/2001/10/xml-exc-c14n#\">";
			strSignBlck += L"<ec:InclusiveNamespaces xmlns:ec=\"http://www.w3.org/2001/10/xml-exc-c14n#\" PrefixList=\"ser\"></ec:InclusiveNamespaces>";
			strSignBlck += L"</ds:Transform>";
			strSignBlck += L"</ds:Transforms>";
			strSignBlck += L"<ds:DigestMethod Algorithm=\"http://www.w3.org/2001/04/xmlenc#sha256\"></ds:DigestMethod>";
			strSignBlck += L"<ds:DigestValue>" + strDigest + L"</ds:DigestValue>";
			strSignBlck += L"</ds:Reference>";
			strSignBlck += L"</ds:SignedInfo>";
			strSoapHeader += strSignBlck;

			// Signature of Signed Block (Using Private Key)
			CString strSignature = Cal_Signature_And_Base64Encode(strSignBlck, (unsigned char*)m_pPrivateKey, m_nLenPrivateKey, 1);
			NHDEBUG(DBG_INFO, (L" Calculate SoapBody Signature: Length of Org Msg:[%d] => Signature:[%s]\n", strSignBlck.GetLength(), strSignature));
			strSoapHeader += L"<ds:SignatureValue>" + strSignature + "</ds:SignatureValue>";

			strSoapHeader += L"<ds:KeyInfo>";
			strSoapHeader += L"<wsse:SecurityTokenReference>";
			strSoapHeader += L"<wsse:Reference URI=\"#binarytoken\" ValueType=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-x509-token-profile-1.0#X509v3\"></wsse:Reference>";
			strSoapHeader += L"</wsse:SecurityTokenReference>";
			strSoapHeader += L"</ds:KeyInfo>";
			strSoapHeader += L"</ds:Signature>";
			strSoapHeader += L"</SOAP-SEC:Security>";
			strSoapHeader += L"</SOAP:Header>";

			NHDEBUG(DBG_INFO, (L"Soap Header Length = [%d]\n", strSoapHeader.GetLength() ));
			strSendBody += strSoapHeader;
		#endif

		// Soap Body
		strSendBody += strSoapMessabeBody;

		// Soap Envelope
		strSendBody += L"</SOAP:Envelope>";
	}
	// End of [#2515]
	return strSendBody;
}

CString	CPin4Data::MakeDemoResponseMessage()
{
	CString strSendBody = L"";

	// [#2515] US 2017.11.15 Add Prestaging
	if( m_nTransactionStep == PIN4_TRAN_GETTOKEN )
	{
		strSendBody  = "{\"token_type\":\"bearer\",";
		strSendBody +=  "\"access_token\":\"FmUoFKHOfCCHICr5uTUuj0nMetvhWTG4KdY2RC4gwO4=\",";
		strSendBody +=  "\"expires_in\":\"600\"}";
	}
	else if( (m_nTransactionStep == PIN4_TRAN_AUTH_PIN4) || (m_nTransactionStep == PIN4_TRAN_REVERSAL_PIN4) )
	{
		// Request Message Header
		strSendBody = L"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n";

		// Request Type
		if( m_nTransactionStep == PIN4_TRAN_AUTH_PIN4 )				strSendBody += L"<AuthorizationResponse>\r\n";
		else if( m_nTransactionStep == PIN4_TRAN_REVERSAL_PIN4 )	strSendBody += L"<ReversalResponse>\r\n";

		// Terminal ID
		strSendBody +=		L"<termId>";		strSendBody += m_strTID;					strSendBody += L"</termId>\r\n";

		// Transacton Code
		if( m_nTransactionStep == PIN4_TRAN_AUTH_PIN4 )				strSendBody +=		L"<transCode>80</transCode>\r\n";
		else if( m_nTransactionStep == PIN4_TRAN_REVERSAL_PIN4 )	strSendBody +=		L"<transCode>29</transCode>\r\n";

		// Sequence Number
		strSendBody +=		L"<seqNum>";		strSendBody += m_strTranSeqNo;				strSendBody += L"</seqNum>\r\n";

		// Response Code 000:Approval / Allothers Declined / 011 => Make reversal
		strSendBody +=		L"<responseCode>";	strSendBody += m_strSecretPin.Left(3);		strSendBody += L"</responseCode>\r\n";

		if( m_nTransactionStep == PIN4_TRAN_AUTH_PIN4 )	
		{
			CString strTemp, strTemp2;

			// Authorization Number
			strSendBody +=		L"<authorizationNum>00001234</authorizationNum>\r\n";

			// Date and Time
			CTime ct = CTime::GetCurrentTime();
			strTemp2.Format(L"%04d", ct.GetYear());
			// Date
			strTemp.Format(L"%02d%02d%s",  ct.GetMonth(), ct.GetDay(), strTemp2.Right(2) );
			strSendBody +=		L"<transactionDate>";	strSendBody += strTemp;	strSendBody += L"</transactionDate>\r\n";
			strSendBody +=		L"<businessDate>";		strSendBody += strTemp;	strSendBody += L"</businessDate>\r\n";
			// Time
			strTemp.Format(L"%02d%02d%02d",  ct.GetHour(), ct.GetMinute(), ct.GetSecond() );
			strSendBody +=		L"<transactionTime>";	strSendBody += strTemp;	strSendBody += L"</transactionTime>\r\n";

			// Amount One and Two
			// Pin4 Allows only FULL REVERSAL
			strSendBody +=		L"<amountOne>";		strSendBody += m_strTranAmount;				strSendBody += L"</amountOne>\r\n";
			strSendBody +=		L"<amountTwo>0</amountTwo>\r\n";
			strSendBody +=		L"<Pan>NO_DECRYPT_5000123412341234</Pan>\r\n";
			strSendBody +=		L"<Expiry>NO_DECRYPT_1299</Expiry>\r\n";
		}
		
		// Request Type
		if( m_nTransactionStep == PIN4_TRAN_AUTH_PIN4 )				strSendBody += L"</AuthorizationResponse>\r\n";
		else if( m_nTransactionStep == PIN4_TRAN_REVERSAL_PIN4 )	strSendBody += L"</ReversalResponse>\r\n";
	}
	// End of [#2515]
	return strSendBody;
}

// [#2576] US Justin 2018.09.17 Add Pin4 HTTP Header Parsing Logic 
BOOL CPin4Data::ParseHTTPHeader(CString strReceived, CString &strHeaderCode)	
{
	strHeaderCode = L"910";
	CString strTemp;

	// get the location of HTTP Header
	int nLoc = strReceived.Find(L"HTTP/");
	if (nLoc<0)
		return FALSE;

	CString strHTTPHeader = strReceived.Mid(nLoc + 5);
	nLoc = strHTTPHeader.Find(L" ");						// find spacer between version and status code
	if(nLoc<0)
		return FALSE;
	
	strHTTPHeader = strHTTPHeader.Mid(nLoc+1, 10);			// Check Maximum 10 following characters
	strHeaderCode = L"";
	for(int k=0 ; k<strHTTPHeader.GetLength(); k++)
	{
		strTemp = strHTTPHeader.Mid(k,1);
		if(	IsNum(strTemp) )
			strHeaderCode += strTemp;
		else
			break;
	}

	NHDEBUG(DBG_INFO, (L"FOUND PIN4 HTTP HEADER STATUS CODE = [%s]\n", strHeaderCode));
	int nSttCodeValue = Asc2Int(strHeaderCode);				// Valid Code 100 ~ 599
	if( (nSttCodeValue<100) || (nSttCodeValue>=600) )
	{
		strHeaderCode = L"910";
		return FALSE;
	}

	if( nSttCodeValue != 200 )
	{
		strHeaderCode.Format(L"%03d", nSttCodeValue);
		return FALSE;
	}
	return TRUE;
}
// End of [#2576]

// [#2515] US Justin 2017.11.15
int CPin4Data::ParseReceivedData(CString strReceived)
{
	NHDEBUG(DBG_INFO, (L"CPin4Data::ParseReceivedData , Step=[%d]\n", m_nTransactionStep ) );

	CString strTemp;
	unsigned char decrypted[4098] = {};
	int nConverted;

	// [#2576] US Justin 2018.09.17 Add Pin4 HTTP Header Parsing Logic 
	CString strHeaderCode = L"";
	if(!ParseHTTPHeader(strReceived, strHeaderCode))
		return SetErrorCodeAndDesc(strHeaderCode);
	// End of [#2576]

	// [#2515] US 2017.11.15 Add Prestaging
	if( m_nTransactionStep == PIN4_TRAN_GETTOKEN )
	{	
		// Check Symmetric Key => Save
		strTemp = GetJSONNodeValue(strReceived, L"symkey");
		NHDEBUG(DBG_INFO, (L" Received SYMKEY Length = [%d], First 10 digit = [%s]\n", strTemp.GetLength(), strTemp.Left(10) ));
		if(strTemp.GetLength()>0)
		{
			memset(decrypted, NULL, 4098);
			nConverted = Base64Decode_And_Decrypt(strTemp, (unsigned char*) m_pPublicKey, 1, decrypted);		// Symmetric Key
			if(nConverted > 0)
			{
				NHDEBUG(DBG_INFO, (L" Decryption (using Public Key) OK, Decrypted SYMKEY Length = [%d]\n", nConverted));
				AssignSymmetricKey(decrypted, nConverted);
			}
			else
				return SetErrorCodeAndDesc(L"811");
		}

		// Check Processor Private Key => Save
		strTemp = GetJSONNodeValue(strReceived, L"pem");
		NHDEBUG(DBG_INFO, (L" Received Private Key Length = [%d], First 10 digit = [%s]\n", strTemp.GetLength(), strTemp.Left(10) ));
		if(strTemp.GetLength()>0)
		{
			memset(decrypted, NULL, 4098);
			nConverted = Base64Decode_And_Decrypt(strTemp, (unsigned char*) m_pSymmetricKey, 2, decrypted);		// Processor Private Key
			if(nConverted > 0)
			{
				NHDEBUG(DBG_INFO, (L" Decrypted (Using Sym Key) OK, Decrypted Private Key. Length = [%d]\n", nConverted));
				AssignProcessorPrivateKey(decrypted, nConverted);
			}
			else
				return SetErrorCodeAndDesc(L"812");
		}

		// [#2524] 2018.01.02 User Certificate
		// Check X509 Certification => Save
		strTemp = GetJSONNodeValue(strReceived, L"cert");
		NHDEBUG(DBG_INFO, (L" Received X509 Certification Length = [%d], First 10 digit = [%s]\n", strTemp.GetLength(), strTemp.Left(10) ));
		if(strTemp.GetLength()>0)
		{
			memset(decrypted, NULL, 4098);
			nConverted = Base64Decode_And_Decrypt(strTemp, (unsigned char*) m_pSymmetricKey, 2, decrypted);		// Processor Private Key
			if(nConverted > 0)
			{
				NHDEBUG(DBG_INFO, (L" Decrypted (Using Sym Key) OK, Decrypted Certification. Length = [%d]\n", nConverted));
				AssignX509Certificate(decrypted, nConverted);
			}
			else
				return SetErrorCodeAndDesc(L"813");
		}
		// End of [#2524]

		// Check token type
		strTemp = GetJSONNodeValue(strReceived, L"token_type");
		if(strTemp.IsEmpty() )
			return SetErrorCodeAndDesc(L"801");
		NHDEBUG(DBG_INFO, (L" token_type= [%s]\n", strTemp));

		// Check Expires_in
		strTemp = GetJSONNodeValue(strReceived, L"expires_in");
		if(strTemp.IsEmpty() )
			return SetErrorCodeAndDesc(L"802");
		NHDEBUG(DBG_INFO, (L" expires_in= [%s]\n", strTemp));

		// Check access_token
		m_strP4AccessToken = GetJSONNodeValue(strReceived, L"access_token");
		if(m_strP4AccessToken.IsEmpty() )
			return SetErrorCodeAndDesc(L"803");
		NHDEBUG(DBG_INFO, (L" access_token (received)= [%s]\n", m_strP4AccessToken));
	}
	else if( (m_nTransactionStep == PIN4_TRAN_AUTH_PIN4) || (m_nTransactionStep == PIN4_TRAN_REVERSAL_PIN4) )
	{
		int nLoc;

		if( m_nTransactionStep == PIN4_TRAN_AUTH_PIN4 )				strTemp = L"<AuthorizationResponse>";
		else if( m_nTransactionStep == PIN4_TRAN_REVERSAL_PIN4 )	strTemp = L"<ReversalResponse>";

		nLoc = strReceived.Find(strTemp);
		if(nLoc<0)
			return SetErrorCodeAndDesc(L"901");

		// [#2524] US Justin 2017.01.11
		//strReceived = strReceived.Mid(nLoc);
		strReceived = strReceived.Mid(nLoc + strTemp.GetLength());

		CString strXMLNS = L"";
		nLoc = strReceived.Find(L":");
		if(nLoc>0)
		{
			strTemp = strReceived.Left(1);
			if(strTemp == L"<")
			{
				strXMLNS = strReceived.Mid(1, nLoc-1);
				if( (strXMLNS.GetLength()>10) || (strXMLNS.Find(L"<")>=0) || (strXMLNS.Find(L">")>=0) )
					strXMLNS = L"";
			}
		}
		if(strXMLNS.GetLength()>0)
			strXMLNS += L":";
		NHDEBUG(DBG_INFO, (L"XMLNS = [%s]\n", strXMLNS));
		// End of [#2524]

		/////////////// PARSING XML DATA
		// Valid fields.
		CStringArray arrSearch;
		arrSearch.Add(strXMLNS + L"responseCode");
		if( m_nTransactionStep == PIN4_TRAN_AUTH_PIN4 )
		{
			arrSearch.Add(strXMLNS + L"authorizationNum");	
			arrSearch.Add(strXMLNS + L"transactionDate");	
			arrSearch.Add(strXMLNS + L"transactionTime");	
			arrSearch.Add(strXMLNS + L"businessDate");
			// [#2515] US Justin 
			#if(APP_PRESTAGIN_PIN4)		
				arrSearch.Add(strXMLNS + L"Pan");
				arrSearch.Add(strXMLNS + L"Expiry");
			#endif
			// End of [#2515]
		}

		CString strResponseCode = L"";
		CString strTarget, strBegin, strEnd, strValue;
		int nLoc1, nLoc2;
		for(int i=0; i<arrSearch.GetSize(); i++)
		{
			strValue = L"";
			strTarget = arrSearch.GetAt(i);
			strBegin = L"<" + strTarget + L">";
			strEnd = L"</" + strTarget + L">";

			nLoc1 = strReceived.Find(strBegin);
			nLoc2 = strReceived.Find(strEnd);

			if( (nLoc1>0) && (nLoc2>0) && (nLoc2>nLoc1) )
				strValue = strReceived.Mid(nLoc1+strBegin.GetLength(), nLoc2-nLoc1-strBegin.GetLength() );

			if(strValue.GetLength() > 0)
			{
				NHDEBUG(DBG_INFO, (L" XML ChildNode [%s] = [%s]\n", strTarget, strValue ));
				if( strTarget.CompareNoCase(strXMLNS + L"responseCode") == 0 )			
				{
					strResponseCode = strValue;
					if(m_nTransactionStep == PIN4_TRAN_AUTH_PIN4 )				m_strTranResCode = strResponseCode;
					else if( m_nTransactionStep == PIN4_TRAN_REVERSAL_PIN4 )	m_strRevResCode  = strResponseCode;
				}
				else if( strTarget.CompareNoCase(strXMLNS + L"authorizationNum") == 0 )		m_strTranResAuthNum = strValue;
				else if( strTarget.CompareNoCase(strXMLNS + L"transactionDate") == 0 )		m_strTranResDate = strValue;
				else if( strTarget.CompareNoCase(strXMLNS + L"transactionTime") == 0 )		m_strTranResTime = strValue;
				else if( strTarget.CompareNoCase(strXMLNS + L"businessDate") == 0 )			m_strTranResBizDate = strValue;
				else if( strTarget.CompareNoCase(strXMLNS + L"Pan") == 0 )					m_strVCN = strValue;				// [#2515] US Justin 
				else if( strTarget.CompareNoCase(strXMLNS + L"Expiry") == 0 )				m_strExpirationYYMM = strValue;		// [#2515] US Justin 
			}
		}

		// Save Transaction Date and Time, Settlement Date for Receipt
		if(m_nTransactionStep == PIN4_TRAN_AUTH_PIN4 )
		{
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTDATE, m_strTranResDate);
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTTIME, m_strTranResTime);
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSSETTLEDATE, m_strTranResBizDate);
			strTemp.Format( L"          %s", m_strTranResAuthNum);
			m_strTranResAuthNum = strTemp.Right(8);
			// Match with Standard1 format
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTRACEAUDITNUM, m_strTranResAuthNum.Left(6));
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSHOSTNETID, m_strTranResAuthNum.Right(2));
		}

		// Check Status field
		if( strResponseCode.GetLength() != 3)	// Incorrect Response Code
			return SetErrorCodeAndDesc(L"904");

		// Declined response
		if ( strResponseCode.CompareNoCase(L"000") != 0)	
		{
			NHDEBUG(DBG_INFO, (L" Transaction Denied, RESPONSE CODE= [%s]\n", strResponseCode));
			SetErrorCodeAndDesc(strResponseCode);

			if( (m_nTransactionStep == PIN4_TRAN_AUTH_PIN4)&&(strResponseCode.CompareNoCase(L"011")==0) )
				m_nNeedReversal = 1;
			return PIN4_HOST_DENY;
		}

		// [#2515] US Justin Check account number and expiration date
		#if(APP_PRESTAGIN_PIN4)		
		if(m_nTransactionStep == PIN4_TRAN_AUTH_PIN4 )
		{
			if( m_strVCN.IsEmpty() || m_strExpirationYYMM.IsEmpty() )
				return SetErrorCodeAndDesc(L"902");

			CString strFirst11 = m_strVCN.Left(11);
			if(strFirst11 == L"NO_DECRYPT_")	
				m_strVCN = m_strVCN.Mid(11);
			else								
			{
				memset(decrypted, NULL, 4098);
				nConverted = Base64Decode_And_Decrypt(m_strVCN, (unsigned char*)m_pSymmetricKey, 2, decrypted);	// VCN		
				
				if(nConverted > 0)
				{
					m_strVCN = CString (decrypted);
					m_strVCN = m_strVCN.Left(nConverted);
				}
				else
					return SetErrorCodeAndDesc(L"804");
			}
			NHDEBUG(DBG_INFO, (L"Decrypted (using SYM Key) Account Number = [%s]\n", m_strVCN));

			strFirst11 = m_strExpirationYYMM.Left(11);
			if(strFirst11 == L"NO_DECRYPT_")	
				m_strExpirationYYMM = m_strExpirationYYMM.Mid(11);
			else								
			{
				memset(decrypted, NULL, 4098);
				nConverted = Base64Decode_And_Decrypt(m_strExpirationYYMM, (unsigned char*) m_pSymmetricKey, 2, decrypted);	// Expiration Date

				if(nConverted > 0)
				{
					m_strExpirationYYMM = CString (decrypted);
					m_strExpirationYYMM = m_strExpirationYYMM.Left(nConverted);
				}
				else
					return SetErrorCodeAndDesc(L"805");					
			}
			NHDEBUG(DBG_INFO, (L"Decrypted (Using SYM Key) Expiration YYMM= [%s]\n", m_strExpirationYYMM));

			if( (m_strVCN.IsEmpty()) || (m_strVCN.GetLength()>19) || (m_strExpirationYYMM.GetLength() != 4) )
				return SetErrorCodeAndDesc(L"903");	
		}
		#endif
		// End of [#2515]

	}
	// End of [#2515]
	return PIN4_RESULT_OK;
}

 int CPin4Data::SetErrorCodeAndDesc(CString sResCode)
{
	m_strErrorCode.Format(L"DH%03s00", sResCode);
															///////1234567890123456789012345678901234567890
	if(		 sResCode == "002" )		m_strErrorDesc = L"Terminal ID Not Identified.";
	else if( sResCode == "004" )		m_strErrorDesc = L"codes are incorrect or expired";
	else if( sResCode == "008" )		m_strErrorDesc = L"Ineligible Transaction";
	else if( sResCode == "011" )		m_strErrorDesc = L"Inconsistent Status";			// FULL REVERSAL
	else if( sResCode == "014" )		m_strErrorDesc = L"# of attempts exceeded";
	else if( sResCode == "017" )		m_strErrorDesc = L"Invalid Amount";
	else if( sResCode == "018" )		m_strErrorDesc = L"External Decline";
	else if( sResCode == "019" )		m_strErrorDesc = L"System Error";
	else if( sResCode == "204" )		m_strErrorDesc = L"No Order for Phone Number";		// [#2576] US Justin 2018.09.07 
	else if( sResCode == "400" )		m_strErrorDesc = L"Bad Request";					// [#2576] US Justin 2018.09.07 
	else if( sResCode == "403" )		m_strErrorDesc = L"Invalid credentials";			// [#2576] US Justin 2018.09.07 
	else if( sResCode == "500" )		m_strErrorDesc = L"Server error";					// [#2576] US Justin 2018.09.07 
	else if( sResCode == "801" )		m_strErrorDesc = L"No Token Type";					// [#2515] US Justin 2017.11.15
	else if( sResCode == "802" )		m_strErrorDesc = L"No Token Expires In";			// [#2515] US Justin 2017.11.15
	else if( sResCode == "803" )		m_strErrorDesc = L"No Access Token";				// [#2515] US Justin 2017.11.15
	else if( sResCode == "804" )		m_strErrorDesc = L"VCN ERROR";						// [#2515] US Justin
	else if( sResCode == "805" )		m_strErrorDesc = L"Exp. Date ERROR";				// [#2515] US Justin
	else if( sResCode == "811" )		m_strErrorDesc = L"SYM_KEY ERROR";					// [#2515] US Justin 
	else if( sResCode == "812" )		m_strErrorDesc = L"Private Key ERROR";				// [#2515] US Justin 
	else if( sResCode == "813" )		m_strErrorDesc = L"Certificate ERROR";				// [#2524] US Justin 
	else if( sResCode == "900" )		m_strErrorDesc = L"No Response from host";			// [#2515] US Justin 2017.11.15
	else if( sResCode == "901" )		m_strErrorDesc = L"No Response Element";			// [#2515] US Justin 2017.11.15
	else if( sResCode == "902" )		m_strErrorDesc = L"No VCN or Expiration Date";		// [#2515] US Justin 2017.11.15
	else if( sResCode == "903" )		m_strErrorDesc = L"Invalid VCN or Exp. Date";		// [#2515] US Justin 2017.11.15
	else if( sResCode == "904" )		m_strErrorDesc = L"Incorrrect Response Code";		// [#2515] US Justin 2017.11.15
	else if( sResCode == "910" )		m_strErrorDesc = L"Incorrrect HTTP Rest Code";		// [#2576] US Justin 2018.09.07 
	else								m_strErrorDesc = L"Transaction Error";

	return PIN4_RCV_ERROR;
}
// End of [#2405]
