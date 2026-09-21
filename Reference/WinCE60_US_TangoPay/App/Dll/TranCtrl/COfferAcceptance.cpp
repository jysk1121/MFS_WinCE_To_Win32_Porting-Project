#include "StdAfx.h"
#include ".\..\..\H\Tran\COfferAcceptance.h"
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

// [#2540] NH Justin 2018.03.13 Surcharge and DCC Acceptance Report
COfferAcceptance::COfferAcceptance()
{
	m_strOAStartDateTime = m_strSurchargeAccept = m_strSurchargeDecline = m_strDCCAccept, m_strDCCDecline = L"";
}

COfferAcceptance::~COfferAcceptance()
{
}

BOOL COfferAcceptance::ResetAcceptance()
{
	NHDEBUG(DBG_INFO, (L"COfferAcceptance::ResetAcceptance()\n") );
	return SaveAcceptanceResult(TRUE);
}

BOOL COfferAcceptance::ReadAcceptanceHistory()
{
	NHDEBUG(DBG_INFO, (L"COfferAcceptance::ReadAcceptanceHistory()\n") );
	CString strPath;
	CFile cf;

	///////////////////////////////////////////////////////////////
	// Read Acceptance History
	///////////////////////////////////////////////////////////////
	strPath.Format(L"%s\\%s", ATM2_DATA_PATH, OFFER_ACCEPTANCE_RESULT_FILE);
	if( cf.Open( strPath, CFile::modeRead) == FALSE )
	{
		NHDEBUG(DBG_INFO, (L"[FAIL TO OPEN Acceptance History(%s) => Reset Acceptance Result]\n", strPath));
		SaveAcceptanceResult(TRUE);
		return TRUE;
	}

	int nFileLen = (int)cf.GetLength();
	unsigned char* pBuffer1 = new unsigned char[nFileLen + 2];		// Allocate buffer for binary file data
	if( pBuffer1 == NULL )
	{
		cf.Close();
		NHDEBUG(DBG_INFO, (L"[FAIL TO NEW BUFFER(%d)] => Reset Acceptance Result\n", nFileLen + 2));
		SaveAcceptanceResult(TRUE);
		return TRUE;
	}
	nFileLen = cf.Read( pBuffer1, nFileLen );
	cf.Close();

	CString strAcceptData = (LPCSTR)pBuffer1;
	delete [] pBuffer1;
	NHDEBUG(DBG_INFO, (L"   Acceptance Data =[%s]\n", strAcceptData));

	CStringArray	arrTemp;
	SplitString(strAcceptData, L",", arrTemp);	
	
	// Info  (5) :		Date and Time, Surcharge Accept, Surcharge Decline, DCC Accept, DCC Decline
	if(arrTemp.GetSize() < 5)
	{
		NHDEBUG(DBG_INFO, (L"[Acceptance Information is wrong or not set => Reset Acceptance Result]\n"));
		SaveAcceptanceResult(TRUE);
		return TRUE;
	}

	BOOL bReadSuccessful = TRUE;

	// Date and Time
	CString strTemp = arrTemp.GetAt(0);
	NHDEBUG(DBG_INFO, (L"   m_strOAStartDateTime = [%s]\n", strTemp));
	if(strTemp.GetLength()==14)		m_strOAStartDateTime = strTemp;
	else							bReadSuccessful = FALSE;

	// Surcharge Accept
	if(bReadSuccessful)
	{
		strTemp = arrTemp.GetAt(1);
		NHDEBUG(DBG_INFO, (L"   m_strSurchargeAccept = [%s]\n", strTemp));
		if(Asc2Int(strTemp)>=0)			m_strSurchargeAccept = strTemp;
		else							bReadSuccessful = FALSE;
	}

	// Surcharge Decline
	if(bReadSuccessful)
	{
		strTemp = arrTemp.GetAt(2);
		NHDEBUG(DBG_INFO, (L"   m_strSurchargeDecline = [%s]\n", strTemp));
		if(Asc2Int(strTemp)>=0)			m_strSurchargeDecline = strTemp;
		else							bReadSuccessful = FALSE;
	}

	// DCC Accept
	if(bReadSuccessful)
	{
		strTemp = arrTemp.GetAt(3);
		NHDEBUG(DBG_INFO, (L"   m_strDCCAccept = [%s]\n", strTemp));
		if(Asc2Int(strTemp)>=0)			m_strDCCAccept = strTemp;
		else							bReadSuccessful = FALSE;
	}

	// DCC Decline
	if(bReadSuccessful)
	{
		strTemp = arrTemp.GetAt(4);
		NHDEBUG(DBG_INFO, (L"   m_strDCCDecline = [%s]\n", strTemp));
		if(Asc2Int(strTemp)>=0)			m_strDCCDecline = strTemp;
		else							bReadSuccessful = FALSE;
	}

	if(bReadSuccessful)
		NHDEBUG(DBG_INFO, (L"COfferAcceptance::ReadAcceptanceHistory () SUCCESS\n") );
	else
	{
		NHDEBUG(DBG_INFO, (L"[ERROR on Acceptance Result Data]  => Reset Acceptance Result\n"));
		SaveAcceptanceResult(TRUE);
	}	
	return TRUE;
}

BOOL COfferAcceptance::SaveAcceptanceResult(BOOL bReset)
{
	NHDEBUG(DBG_INFO, (L"COfferAcceptance::SaveAcceptanceResult(%d) \n", bReset));

	CString strPath;
	strPath.Format(L"%s\\%s", ATM2_DATA_PATH, OFFER_ACCEPTANCE_RESULT_FILE);

	CFile cf;
	if( cf.Open( strPath, CFile::modeCreate|CFile::modeWrite) == FALSE )
	{
		NHDEBUG(DBG_INFO, (L"[FAIL TO OPEN(%s)]\n", strPath));
		return FALSE;
	}

	if(bReset)
	{
		m_strOAStartDateTime.Format(L"%s%s",GetDate(), GetTime());
		m_strSurchargeAccept = m_strSurchargeDecline = m_strDCCAccept = m_strDCCDecline = L"0";
	}

	CString strTotalData = L"";
	strTotalData += m_strOAStartDateTime	+ _T(",");
	strTotalData += m_strSurchargeAccept	+ _T(",");
	strTotalData += m_strSurchargeDecline	+ _T(",");
	strTotalData += m_strDCCAccept			+ _T(",");
	strTotalData += m_strDCCDecline;

	NHDEBUG(DBG_INFO, (L"   AcceptanceResult = [%s]\n",strTotalData));

	UINT nSize = strTotalData.GetLength();

	char *pBuffer = new char[nSize+1];
	memset(pBuffer, 0, nSize+1);
	WideToMulti( pBuffer, strTotalData, nSize);

	cf.Write(pBuffer, nSize );
	delete [] pBuffer;

	cf.Close();
	return TRUE;
}

void COfferAcceptance::IncreaseAcceptanceResult(int nType)
{
	NHDEBUG(DBG_INFO, (L"COfferAcceptance::IncreaseAcceptanceResult (Type=%d)\n", nType) );

	// Update Day Total ... Date Time
	if( m_strOAStartDateTime.GetLength() != 14)
		m_strOAStartDateTime.Format(L"%s%s", GetDate(), GetTime() );
	NHDEBUG(DBG_INFO, (L"  m_strOAStartDateTime = [%s]\n", m_strOAStartDateTime) );

	long nTemp = 0;
	if(nType == OFFER_SURCHARGE_ACCEPT)
	{
		nTemp = Asc2Int(m_strSurchargeAccept) + 1;
		m_strSurchargeAccept.Format(L"%ld", nTemp);
		NHDEBUG(DBG_INFO, (L"  m_strSurchargeAccept = [%s]\n", m_strSurchargeAccept) );
	}
	else if(nType == OFFER_SURCHARGE_DECLINE)
	{
		nTemp = Asc2Int(m_strSurchargeDecline) + 1;
		m_strSurchargeDecline.Format(L"%ld", nTemp);
		NHDEBUG(DBG_INFO, (L"  m_strSurchargeDecline = [%s]\n", m_strSurchargeDecline) );
	}	
	else if(nType == OFFER_DCC_ACCEPT)
	{
		nTemp = Asc2Int(m_strDCCAccept) + 1;
		m_strDCCAccept.Format(L"%ld", nTemp);
		NHDEBUG(DBG_INFO, (L"  m_strDCCAccept = [%s]\n", m_strDCCAccept) );
	}
	else if(nType == OFFER_DCC_DECLINE)
	{
		nTemp = Asc2Int(m_strDCCDecline) + 1;
		m_strDCCDecline.Format(L"%ld", nTemp);
		NHDEBUG(DBG_INFO, (L"  m_strDCCDecline = [%s]\n", m_strDCCDecline) );
	}
	SaveAcceptanceResult();
}

CString COfferAcceptance::GetAcceptanceData()
{
	NHDEBUG(DBG_INFO, (L"COfferAcceptance::GetAcceptanceData()\n") );
	CString strRtnData = L"";
	strRtnData += m_strOAStartDateTime	+ _T("^");
	strRtnData += m_strSurchargeAccept	+ _T("^");
	strRtnData += m_strSurchargeDecline	+ _T("^");
	strRtnData += m_strDCCAccept		+ _T("^");
	strRtnData += m_strDCCDecline;
	NHDEBUG(DBG_INFO, (L"   Acceptance Data=[%s])\n", strRtnData) );
	return strRtnData;
}
// End of [#2540]
