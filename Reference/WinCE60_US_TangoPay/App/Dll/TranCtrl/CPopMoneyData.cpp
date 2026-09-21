#include "StdAfx.h"
#include ".\..\..\H\Tran\CPopMoneyData.h"
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

// [#2442] US Justin 2016.08.25 PopMoney Day Total
#if(APP_POPMONEY)	// (APP_CUSTOM_PAI) [#2471] US Justin 2017.02.01 Enable Popmoney to all customers
 CPopMoneyData::CPopMoneyData()
{
	m_strPMDayTotalDateTime = m_strPMDayTotalDispCount = m_strPMDayTotalDispAmt = L"";
}

CPopMoneyData::~CPopMoneyData()
{
}

void CPopMoneyData::ResetTotal()
{
	NHDEBUG(DBG_INFO, (L"CPopMoneyData::ResetTotal()\n") );
	SaveDispenseResult(TRUE);
}

BOOL CPopMoneyData::ReadDispenseHistory()
{
	NHDEBUG(DBG_INFO, (L"CPopMoneyData::ReadDispenseHistory()\n") );
	CString strPath;
	CFile cf;

	///////////////////////////////////////////////////////////////
	// Read Dispense History
	///////////////////////////////////////////////////////////////
	strPath.Format(L"%s\\%s", ATM2_DATA_PATH, POPMONEY_DISPENSERESULT_FILE);
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
	NHDEBUG(DBG_INFO, (L"   m_strPMDayTotalDateTime = [%s]\n", strTemp));
	if(strTemp.GetLength()==14)		m_strPMDayTotalDateTime = strTemp;
	else							bReadSuccessful = FALSE;

	// DayTotal Dispense Count
	if(bReadSuccessful)
	{
		strTemp = arrTemp.GetAt(1);
		NHDEBUG(DBG_INFO, (L"   m_strPMDayTotalDispCount = [%s]\n", strTemp));
		if(Asc2Int(strTemp)>=0)			m_strPMDayTotalDispCount = strTemp;
		else							bReadSuccessful = FALSE;
	}

	// DayTotal Total Dispense Amount
	if(bReadSuccessful)
	{
		strTemp = arrTemp.GetAt(2);
		NHDEBUG(DBG_INFO, (L"   m_strPMDayTotalDispAmt = [%s]\n", strTemp));
		if(Asc2Int(strTemp)>=0)			m_strPMDayTotalDispAmt = strTemp;
		else							bReadSuccessful = FALSE;
	}

	if(bReadSuccessful)
		NHDEBUG(DBG_INFO, (L"CPopMoneyData::ReadDispenseHistory () SUCCESS\n") );
	else
	{
		NHDEBUG(DBG_INFO, (L"[ERROR on Dispense Result Data]  => Reset Dispense Result\n"));
		SaveDispenseResult(TRUE);
	}	
	return TRUE;
}

BOOL CPopMoneyData::SaveDispenseResult(BOOL bReset)
{
	NHDEBUG(DBG_INFO, (L"CPopMoneyData::SaveDispenseResult(%d) \n", bReset));

	CString strPath;
	strPath.Format(L"%s\\%s", ATM2_DATA_PATH, POPMONEY_DISPENSERESULT_FILE);

	CFile cf;
	if( cf.Open( strPath, CFile::modeCreate|CFile::modeWrite) == FALSE )
	{
		NHDEBUG(DBG_INFO, (L"[FAIL TO OPEN(%s)]\n", strPath));
		return FALSE;
	}

	if(bReset)
	{
		m_strPMDayTotalDateTime.Format(L"%s%s",GetDate(), GetTime());
		m_strPMDayTotalDispCount = L"0";
		m_strPMDayTotalDispAmt = L"0";
	}

	CString strTotalDispenseData = L"";
	strTotalDispenseData += m_strPMDayTotalDateTime + _T(",");
	strTotalDispenseData += m_strPMDayTotalDispCount + _T(",");
	strTotalDispenseData += m_strPMDayTotalDispAmt;

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

void CPopMoneyData::RecodeDispenseResult(long nAmount /* Cent */)
{
	NHDEBUG(DBG_INFO, (L"CPopMoneyData::RecodeDispenseResult (Amt=%ld)\n", nAmount) );

	// Update Day Total ... Date Time
	if( m_strPMDayTotalDateTime.GetLength() != 14)
		m_strPMDayTotalDateTime.Format(L"%s%s", GetDate(), GetTime() );
	NHDEBUG(DBG_INFO, (L"  m_strPMDayTotalDateTime = [%s]\n", m_strPMDayTotalDateTime) );

	// Update Day Total ... Dispense Count
	long nTemp = Asc2Int(m_strPMDayTotalDispCount) + 1;
	m_strPMDayTotalDispCount.Format(L"%ld", nTemp);
	NHDEBUG(DBG_INFO, (L"  m_strPMDayTotalDispCount = [%s]\n", m_strPMDayTotalDispCount) );

	// Update Day Total ... Dispense Amount
	nTemp = Asc2Int(m_strPMDayTotalDispAmt) + ((int) (0.01*nAmount+0.001));	// Dollar
	m_strPMDayTotalDispAmt.Format(L"%ld", nTemp);
	NHDEBUG(DBG_INFO, (L"  m_strPMDayTotalDispAmt = [%s]\n", m_strPMDayTotalDispAmt) );

	SaveDispenseResult();
}

CString CPopMoneyData::GetDaytotalJournalData()
{
	NHDEBUG(DBG_INFO, (L"CPopMoneyData::GetDaytotalJournalData()\n") );

	CString strEJNLData = L"";
	strEJNLData += m_strPMDayTotalDateTime + _T("^");
	strEJNLData += m_strPMDayTotalDispCount + _T("^");
	strEJNLData += m_strPMDayTotalDispAmt;

	NHDEBUG(DBG_INFO, (L"   Journal Data=[%s])\n", strEJNLData) );
	return strEJNLData;
}
#endif