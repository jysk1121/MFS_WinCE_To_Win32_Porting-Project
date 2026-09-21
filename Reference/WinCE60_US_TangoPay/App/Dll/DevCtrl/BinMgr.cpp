// BinMgr.cpp: implementation of the CBinMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\..\H\Dev\BinMgr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


#define DB_DEBUG(X,Y)	RETAILMSG(X,Y);
#define ON				1


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------
 CLASS    NAME: CBinMgr
 FUNCTION NAME: CBinMgr()
 RETURN TYPE  : 
 PARAMETER    :
 DESCRIPTION  : NVRAM에 메모리를 주소 번지를 얻어옴 
-------------------------------------------------------------------*/
CBinMgr::CBinMgr()
{
	m_pBinBlock = NULL;

	m_pBinBlock = (LPBinBlock)GetRsvdAddr();

	InitBinVariable();

	if (!m_pBinBlock)
	{
		DB_DEBUG(ON, (_T("[CBinMgr::MappingBinBlock] memory mapping failed. BinBloack(0x%X)\n"), m_pBinBlock));
		return;		// KSK 2009.9.9 Codesonar 지적사항 대책
	}

	//메모리에 쓰레기 데이타가 있을 경우 초기화 한다 )
	if ( m_pBinBlock->nReservedNVRAMVersionInfo < RESEVED_AREA_START_VERSION || m_pBinBlock->nReservedNVRAMVersionInfo > RESEVED_AREA_VERSION)
	{
		DB_DEBUG(ON, (_T("[CBinMgr::CBinMgr()] Garbage data existed\n"), m_pBinBlock));
		// [#573] NH AIREAT 2009.10.09 SW NVRAM
		//memset(m_pBinBlock, 0x00, 256*1024);
		//m_pBinBlock->nReservedNVRAMVersionInfo = RESEVED_AREA_VERSION;
		NvramSetMemory(m_pBinBlock, 0x00, 256*1024);
		NvramCopyDword((LPDWORD)&m_pBinBlock->nReservedNVRAMVersionInfo, (DWORD)RESEVED_AREA_VERSION);
		// end of [#573]
	}

	if ( m_pBinBlock->wBinTotalCount == 0)
		m_wCurrentBinIndex = 0;
	else
		m_wCurrentBinIndex = 0;

	m_wCurrentBinPage = 1;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CBinMgr
 FUNCTION NAME: ~CBinMgr()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 할당 된 메모리 해제함 . 
-------------------------------------------------------------------*/
CBinMgr::~CBinMgr()
{
	if ( m_pBinBlock )
	{
		FreeRsvdAddr(m_pBinBlock);
		m_pBinBlock = NULL;
	}
	InitBinVariable();

}

/*-------------------------------------------------------------------
 CLASS    NAME: CBinMgr
 FUNCTION NAME: DeleteAllList()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : BINLIST에 할당된 메모리 초기화 함 . 
     Allow Only List Bin Eanble도 초기화 시킴 
	 (다 지우고 해당 항목만 켜 있으면 거래가 안 됨)
-------------------------------------------------------------------*/
void CBinMgr::DeleteAllList()
{
//		memset(m_pBinBlock, 0x00, sizeof(BinBlock));
	// [#573] NH AIREAT 2009.10.09 SW NVRAM
	//m_pBinBlock->wBinTotalCount = 0;
	//memset(m_pBinBlock->BinList, 0x00, sizeof(m_pBinBlock->BinList));
	NvramCopyWord(&m_pBinBlock->wBinTotalCount, 0);
	NvramSetMemory(m_pBinBlock->BinList, 0x00, sizeof(m_pBinBlock->BinList));
	// end of [#573]
}

/*-------------------------------------------------------------------
 CLASS    NAME: CBinMgr
 FUNCTION NAME: GetAllowOnlyBinEnable()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/

WORD CBinMgr::GetAllowOnlyBinEnable()
{
	return m_pBinBlock->wAllowOnlyBinEnable;
}
/*-------------------------------------------------------------------
 CLASS    NAME: CBinMgr
 FUNCTION NAME: SetAllowOnlyBinEnable()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CBinMgr::SetAllowOnlyBinEnable(WORD value)
{
	//m_pBinBlock->wAllowOnlyBinEnable = value;
	NvramCopyWord(&m_pBinBlock->wAllowOnlyBinEnable, value);	// [#573] NH AIREAT 2009.10.09 SW NVRAM
}

/*-------------------------------------------------------------------
 CLASS    NAME: CBinMgr
 FUNCTION NAME: InsertBinProperties()
 RETURN TYPE  : 
 PARAMETER    : BIN Number, Action, Value
 DESCRIPTION  : 기존 BIN LIST에 추가함 ( BIN NUMBER, ACTION순으로 정렬하여 추가함 )
-------------------------------------------------------------------*/
BOOL CBinMgr::InsertBinProperties(BinProperties newBin)
{
	int i;

	if ( (m_pBinBlock->wBinTotalCount - m_wViolationCount +1) >  BIN_MAXIMUN_COUNT )
		return FALSE;

	if ( m_wViolationCount > 0)
	{
		for ( int i = m_wViolationCount -1; i >= 0; i--)
		{
			DeleteBinProperites(m_wViolationIndex[i], FALSE);
			if ( m_wViolationIndex[i] < m_wCurrentBinIndex )
				m_wCurrentBinIndex--;
		}
	}

	WORD wInsertIndex = 0;
	BOOL bFoundIndex = FALSE;
	//먼저 해당 위치를 찾는다. 
	for (i = 0; i < m_pBinBlock->wBinTotalCount; i++)
	{
		int nCompare =  memcmp( m_pBinBlock->BinList[i].BinNo, newBin.BinNo, BIN_NO_SIZE);
		 
		if (nCompare  == 0) //동일 BIN일 경우
		{
			if ( m_pBinBlock->BinList[i].wAction > newBin.wAction)//동일 BIN이 들어올 경우 
			{
				wInsertIndex = i;
				bFoundIndex = TRUE;
				break;
			}
			else if ( m_pBinBlock->BinList[i].wAction == newBin.wAction)
				return FALSE;

		}
		else if ( nCompare > 0) //현재 BIN보다 클 경우 해당 INDEX에 넣는다. 
		{
			wInsertIndex = i;
			bFoundIndex = TRUE;
			break;
		}
	}
	if ( bFoundIndex == FALSE)
		wInsertIndex = m_pBinBlock->wBinTotalCount;

//RETAILMSG(1, (_T("NEW BIN =>Index=[%d], BinNo =[%S], Action = [%d], Value = [%d], bFound=[%d]\n"), 
//		 wInsertIndex, newBin.BinNo, newBin.wAction, newBin.dwValue, bFoundIndex ));

	//중간에 삽입 될 경우  해당 INDEX로 부터 뒤로 한칸씩 밀고. 
	for ( i = m_pBinBlock->wBinTotalCount; i > wInsertIndex; i--)
	{
		// [#573] NH AIREAT 2009.10.09 SW NVRAM
		NvramCopyMemory((void*)m_pBinBlock->BinList[i].BinNo,  (void*)m_pBinBlock->BinList[i-1].BinNo, BIN_NO_SIZE);
		NvramCopyWord(&m_pBinBlock->BinList[i].wAction, m_pBinBlock->BinList[i-1].wAction);
		NvramCopyDword(&m_pBinBlock->BinList[i].dwValue, m_pBinBlock->BinList[i-1].dwValue);
		// end of [#573]
	}
	
	//해당 INDEX에 추가될 BIN 삽입하고 
	// [#573] NH AIREAT 2009.10.09 SW NVRAM
	NvramCopyMemory(&m_pBinBlock->BinList[wInsertIndex], &newBin, sizeof(BinProperties));
	// end of [#573]

	//m_pBinBlock->wBinTotalCount++;
	NvramCopyWord(&m_pBinBlock->wBinTotalCount, (WORD)(m_pBinBlock->wBinTotalCount+1));		// [#573] NH AIREAT 2009.10.09 SW NVRAM

	//Current Index를 Insert Index로 넣어준다. 
	m_wCurrentBinIndex = wInsertIndex;


	WORD wPage = m_wCurrentBinIndex / BIN_COUNT_PER_PAGE;
	m_wCurrentBinPage = ++wPage;

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CBinMgr
 FUNCTION NAME: ModifyBinProperites()
 RETURN TYPE  : 
 PARAMETER    : Index BIN Number, Action, Value
 DESCRIPTION  : 기존 Index를 수정하고 위배 되는 항목은 삭제함 
-------------------------------------------------------------------*/
void CBinMgr::ModifyBinProperites(WORD wModifyIndex, BinProperties editBin)
{
	//해당 Index에 BIN을 저장한다. 
	// [#573] NH AIREAT 2009.10.09 SW NVRAM
	NvramCopyMemory( m_pBinBlock->BinList[wModifyIndex].BinNo,  editBin.BinNo , BIN_NO_SIZE);
	NvramCopyWord(&m_pBinBlock->BinList[wModifyIndex].wAction, editBin.wAction);
	NvramCopyDword(&m_pBinBlock->BinList[wModifyIndex].dwValue, editBin.dwValue);
	// end of [#573]

	m_wCurrentBinIndex = wModifyIndex;

	//충돌 되는 BIN이 있으면 해당 내용 삭제함 
	if (m_wViolationCount > 0)
	{
		for ( int i = m_wViolationCount -1; i >= 0; i--)
		{
			DeleteBinProperites(m_wViolationIndex[i], FALSE);
			if ( m_wViolationIndex[i] < m_wCurrentBinIndex )
				m_wCurrentBinIndex--;
		}
	}
	WORD wPage = m_wCurrentBinIndex / BIN_COUNT_PER_PAGE;
	m_wCurrentBinPage = ++wPage;

}

/*-------------------------------------------------------------------
 CLASS    NAME: CBinMgr
 FUNCTION NAME: DeleteBinProperites()
 RETURN TYPE  : 
 PARAMETER    : Index BIN Number
 DESCRIPTION  : 항목을 삭제하고 Current Index를 조정한다. 
-------------------------------------------------------------------*/
void CBinMgr::DeleteBinProperites(WORD wDeleteIndex, BOOL bIndexUpdate)
{
	for ( int i = wDeleteIndex; i < m_pBinBlock->wBinTotalCount-1; i++)
	{
		// [#573] NH AIREAT 2009.10.09 SW NVRAM
		NvramCopyMemory( m_pBinBlock->BinList[i].BinNo,  m_pBinBlock->BinList[i+1].BinNo, BIN_NO_SIZE);
		NvramCopyWord(&m_pBinBlock->BinList[i].wAction, m_pBinBlock->BinList[i+1].wAction);
		NvramCopyDword(&m_pBinBlock->BinList[i].dwValue, m_pBinBlock->BinList[i+1].dwValue);
		// end of [#573]
	}
	//m_pBinBlock->wBinTotalCount--;
	NvramCopyWord(&m_pBinBlock->wBinTotalCount, (WORD)(m_pBinBlock->wBinTotalCount-1));			// [#573] NH AIREAT 2009.10.09 SW NVRAM

	// [#573] NH AIREAT 2009.10.09 SW NVRAM
	NvramSetMemory(  m_pBinBlock->BinList[m_pBinBlock->wBinTotalCount].BinNo, 0x00, sizeof(BIN_NO_SIZE));
	NvramCopyWord(&m_pBinBlock->BinList[m_pBinBlock->wBinTotalCount].wAction, 0);
	NvramCopyDword(&m_pBinBlock->BinList[m_pBinBlock->wBinTotalCount].dwValue, 0);
	// end of [#573]

	if ( bIndexUpdate)
	{
		if ( m_wCurrentBinIndex == m_pBinBlock->wBinTotalCount)
			m_wCurrentBinIndex--;
	}

}
/*-------------------------------------------------------------------
 CLASS    NAME: CBinMgr
 FUNCTION NAME: GetCurrentBinListPage()
 RETURN TYPE  : 
 PARAMETER    : Index BIN Number, Action, Value
 DESCRIPTION  : 기존 Index를 수정하고 위배 되는 항목은 삭제함 
-------------------------------------------------------------------*/
WORD CBinMgr::GetCurrentBinListPage(LPBinProperties pBinProperties)
{
	WORD wBinCount = 0;
	
	WORD wStartIndex = (m_wCurrentBinPage *BIN_COUNT_PER_PAGE) - BIN_COUNT_PER_PAGE;
	WORD wEndIndex = m_wCurrentBinPage *BIN_COUNT_PER_PAGE;

	for ( int i = wStartIndex; i < wEndIndex; i++)
	{
		if ( i == m_pBinBlock->wBinTotalCount) break;

		m_CurrentPageBinList[wBinCount++] = m_pBinBlock->BinList[i];
	}
	
	memcpy(pBinProperties, m_CurrentPageBinList, sizeof(BinProperties) *BIN_COUNT_PER_PAGE);

	return wBinCount; 
}
/*-------------------------------------------------------------------
 CLASS    NAME: CBinMgr
 FUNCTION NAME: GetTotalBinCount()
 RETURN TYPE  : Total BIN Count
 PARAMETER    : 
 DESCRIPTION  : NVRAM에 저장된 BIN LIST Total Count를 리턴함 
-------------------------------------------------------------------*/
WORD CBinMgr::GetTotalBinCount()
{
	return m_pBinBlock->wBinTotalCount;
}
/*-------------------------------------------------------------------
 CLASS    NAME: CBinMgr
 FUNCTION NAME: GetAnotherProperties()
 RETURN TYPE  : Other Properties Count
 PARAMETER    : 현재 BIN과 Other Properties가 저장될 BIN Properties 배열 
 DESCRIPTION  : 동일 BIN NUMBER로 지정된 다른 BIN 들이 있는지 검색하여 
              Count와 그 배열을 리터함 
-------------------------------------------------------------------*/
WORD CBinMgr::GetAnotherProperties(BinProperties curBin, LPBinProperties lpProperties)
{
	WORD wBinCount = 0;
	memset(m_AnotherPropertiesList, 0x00, sizeof(BinProperties) * BIN_COUNT_PER_PAGE);
	memset(m_wAnotherIndex, 0x00, sizeof(WORD) * BIN_COUNT_PER_PAGE);
	
	for ( int i = 0 ; i < m_pBinBlock->wBinTotalCount; i++)
	{
		if ( m_wEditBinFlag == TRUE && m_wEditIndex == i)
			continue;
		else
		{
			BinProperties targetBin = m_pBinBlock->BinList[i];

			if ( memcmp(curBin.BinNo, targetBin.BinNo, BIN_NO_SIZE) == 0)
			{
				m_AnotherPropertiesList[wBinCount] = targetBin;
				m_wAnotherIndex[wBinCount] = i;
				wBinCount++;
			}
		}
	}

	memcpy(lpProperties, m_AnotherPropertiesList, sizeof(BinProperties) * BIN_COUNT_PER_PAGE);

	return wBinCount;
}
/*-------------------------------------------------------------------
 CLASS    NAME: CBinMgr
 FUNCTION NAME: GetViolationProperties()
 RETURN TYPE  : BIN과 위배되는 항목 COUNT 
 PARAMETER    : 현재 BIN 정보, 위배되는 항목을 저장하기 위한 배열, 
                리스트를 신규 잘성할 것인지 기존값 활용할 것인지 여부 
 DESCRIPTION  : 등록 되려는 BIN과 위배되는 항목이 있는지 체크함
-------------------------------------------------------------------*/
WORD CBinMgr::GetViolationProperties(BinProperties curBin, LPBinProperties lpProperties, BOOL bUpdate)
{
	if ( bUpdate)
	{
		m_wViolationCount  = 0;
		memset(m_ViolationPropertiesList , 0x00, sizeof(BinProperties) * BIN_COUNT_PER_PAGE);
		memset(m_wViolationIndex , 0x00, sizeof(WORD) * BIN_COUNT_PER_PAGE);
	
		for ( int i = 0 ; i < m_pBinBlock->wBinTotalCount; i++)
		{
			if ( m_wEditBinFlag == TRUE && m_wEditIndex == i) //같은 INDEX는 SKIP
				continue;
			else
			{
				BinProperties targetBin = m_pBinBlock->BinList[i];

				if ( memcmp(curBin.BinNo, targetBin.BinNo, BIN_NO_SIZE) == 0)
				{
					if ( IsVoilated(curBin, targetBin) == TRUE)
					{
						m_ViolationPropertiesList[m_wViolationCount] = targetBin;
						m_wViolationIndex[m_wViolationCount] = i;
						m_wViolationCount++;
					}
				}
			}
		}
	}
	memcpy(lpProperties, m_ViolationPropertiesList, sizeof( BinProperties) *BIN_COUNT_PER_PAGE);

	return 	m_wViolationCount;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CBinMgr
 FUNCTION NAME: MakeBinPrintData()
 RETURN TYPE  : Print 할 데이타 편집함 
 PARAMETER    : Print할 페이지. 
 DESCRIPTION  : 페이지 별로 프린트할 데이타를 편집함  
-------------------------------------------------------------------*/
CString CBinMgr::MakeBinPrintData(WORD pageno)
{
	WORD wStartIndex = (pageno *BIN_COUNT_PER_PAGE) - BIN_COUNT_PER_PAGE;
	WORD wEndIndex = pageno *BIN_COUNT_PER_PAGE;

	if ( wEndIndex >  m_pBinBlock->wBinTotalCount)
		wEndIndex =  m_pBinBlock->wBinTotalCount ;

	CString	strPrintData, strTemp, strAmount, strAction;

	for ( int i = wStartIndex; i < wEndIndex; i++)
	{
		if ( m_pBinBlock->BinList[i].wAction == ACTION_WITHDRAWAL_SURCHARGE
			|| m_pBinBlock->BinList[i].wAction == ACTION_BALANCE_SURCHARGE )
		{
			strAmount.Format(L"%d", m_pBinBlock->BinList[i].dwValue);
			strAction.Format(L"%s%5s", GetCurrencySymbol(), MakeMoneyCent(strAmount));	// KSK 2011.04.13 Bug Fix
		}
		else
			strAction = L"";

		strTemp.Format(L"%4d %-10S %-17s", 
			i+1,  m_pBinBlock->BinList[i].BinNo, GetActionString(m_pBinBlock->BinList[i].wAction) );

		strPrintData += strTemp;
		strPrintData += strAction;

		if ( i != (wEndIndex -1))
			strPrintData += "||";

	}
	return strPrintData;
}
/*-------------------------------------------------------------------
 CLASS    NAME: CBinMgr
 FUNCTION NAME: MakeHeaderPrintData()
 RETURN TYPE  : Print 할 데이타 Header 편집함 
 PARAMETER    : 현재 프린트 하는 Terminal Number
 DESCRIPTION  : 제목, 현재 날짜.  
-------------------------------------------------------------------*/
CString CBinMgr::MakeHeaderPrintData(CString strTerminalID)
{
	CString	strPrintData = L"", strTemp;

	int index = 0;

	strTemp = L"========================================||";
	strPrintData = strTemp;
	strTemp = L"          * PRINT BIN LIST *            ||";
	strPrintData += strTemp;
	strTemp = L"========================================||";
	strPrintData += strTemp;

	SYSTEMTIME localTime;
	::GetLocalTime(&localTime);
	strTemp.Format(L"TODAY              = %s %2.2d:%2.2d:%2.2d||", 
					GetCmnLocalDate(localTime.wMonth, localTime.wDay, localTime.wYear),
					localTime.wHour, localTime.wMinute, localTime.wSecond);
	strPrintData += strTemp;

	strTemp.Format(L"TERMINAL #         = %s||", strTerminalID);
	strPrintData += strTemp;


	strTemp = L"--------------------------------------------||";
	strPrintData += strTemp;

	strTemp.Format(L"ALLOW ONLY LISTED BIN : %s||", (m_pBinBlock->wAllowOnlyBinEnable == 0 ? L"DISABLE": L"ENABLE"));
	strPrintData += strTemp;

	strTemp.Format(L"TOTAL BIN COUNT       : %d ||", m_pBinBlock->wBinTotalCount);
	strPrintData += strTemp;
	strTemp = L"--------------------------------------------||";
	strPrintData += strTemp;

	return strPrintData;
}


/*-------------------------------------------------------------------
 CLASS    NAME: CBinMgr
 FUNCTION NAME: MoveCurrentPage()
 RETURN TYPE  : 페이지 이동 여부 
 PARAMETER    : Move할 Page 단위 -1, 이전 Page, 1이면 다음 페이지 -5, 5
 DESCRIPTION  : 현재 Page에서 입력된 PAge 단위로 이동하여 출력하고 
                 이동 불가능 할 경우 FALSE를 리턴함 
-------------------------------------------------------------------*/
BOOL CBinMgr::MoveCurrentPage(int MovePage)
{
	WORD wPage = m_pBinBlock->wBinTotalCount / BIN_COUNT_PER_PAGE;
	WORD wModuler = m_pBinBlock->wBinTotalCount % BIN_COUNT_PER_PAGE;
	if ( wModuler != 0) wPage++;

	if ( wPage == 0) wPage =1;

	WORD wTotalPage = wPage;

	if ( MovePage > 0)
	{
		if ( m_wCurrentBinPage == wTotalPage ) //마지막 페이지 
		{
			return FALSE;
		}
		else
		{
			if ( m_wCurrentBinPage + MovePage  <= wTotalPage)
			{
				m_wCurrentBinPage += MovePage;
			}
			else
			{
				m_wCurrentBinPage = wTotalPage;
			}
		}
	}
	else
	{
		if ( m_wCurrentBinPage == 1) //첫번째 페이지 
		{
			return FALSE;
		}
		else
		{
			if ( m_wCurrentBinPage + MovePage >= 1)
			{
				m_wCurrentBinPage += MovePage;
			}
			else
			{
				m_wCurrentBinPage = 1;
			}
		}
	}
	//Current Index도 바꿔야 하나? 

	m_wCurrentBinIndex = (m_wCurrentBinPage-1 )*BIN_COUNT_PER_PAGE;
	//이건 좀 생각해 보고 나서 

	return TRUE;
}
/*-------------------------------------------------------------------
 CLASS    NAME: CBinMgr
 FUNCTION NAME: GetTotalPage()
 RETURN TYPE  : 총 출력 가능한 페이지 Count  
 PARAMETER    : 
 DESCRIPTION  : 10개씩 출력 할 때 가능한 Total Page Count Returen
-------------------------------------------------------------------*/
WORD CBinMgr::GetTotalPage()
{
	WORD wPage = 1;
	
	if ( m_pBinBlock->wBinTotalCount > 0)
	{
		wPage = m_pBinBlock->wBinTotalCount / BIN_COUNT_PER_PAGE;
		WORD wModuler = m_pBinBlock->wBinTotalCount % BIN_COUNT_PER_PAGE;
		if ( wModuler != 0) wPage++;
	}
	return wPage;
}
/*-------------------------------------------------------------------
 CLASS    NAME: CBinMgr
 FUNCTION NAME: GetActionString()
 RETURN TYPE  : ACTION String 
 PARAMETER    : 
 DESCRIPTION  : 명세표 출력을 위해 ACTION 별 String을 리턴함 
-------------------------------------------------------------------*/
CString CBinMgr::GetActionString(WORD action)
{
	CString strText= L"UNDEFINED ACTION  ";

	switch(action)
	{
	case ACTION_BLOCK_SURCHARGE:
		strText = L"BLOCK SURCHARGE   "; break;
	case ACTION_WITHDRAWAL_SURCHARGE:
		strText = L"WITHDRAWAL CHARGE "; break;
	case ACTION_BALANCE_SURCHARGE:
		strText = L"BALANCE SURCHARGE "; break;
	case ACTION_ACCEPT_CARD:
		strText = L"ACCEPT CARD       "; break;
	case ACTION_DECLINE_CARD:
		strText = L"DECLINE CARD      "; break;
	case ACTION_AWARD_COUPON:
		strText = L"AWARD COUPON      "; break;
	case ACTION_PIN_CHANGE:
		strText = L"ALLOW PIN CHANGE  "; break;
	default:
		strText= L"UNDEFINED ACTION  "; break;
	}

	return strText;
}
/*-------------------------------------------------------------------
 CLASS    NAME: CBinMgr
 FUNCTION NAME: InitBinVariable()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : BIN LIST 화면 진입시 검색에 필요한 데이타를 초기화를 위해 사용  
-------------------------------------------------------------------*/
void CBinMgr::InitBinVariable()
{
	//해당 항목 모두 초기화 함 
	memset (&m_ViolationPropertiesList, 0x00, sizeof(BinProperties) *BIN_COUNT_PER_PAGE);
	memset (&m_AnotherPropertiesList, 0x00, sizeof(BinProperties) *BIN_COUNT_PER_PAGE);
	memset (&m_CurrentPageBinList, 0x00, sizeof(BinProperties) *BIN_COUNT_PER_PAGE);
	memset (&m_CurrentProperties, 0x00, sizeof(BinProperties));
	memset (&m_EditProperties, 0x00, sizeof(BinProperties));
	memset (m_SearchBinNo, 0x00, BIN_NO_SIZE);
	m_wAnotherPropertiesCount= 0;
	m_wCurrentBinIndex = 0;
	m_wCurrentBinPage = 1;
	m_wEditIndex = 0;
	m_wEditBinFlag = 0;
	m_wViolationCount = 0;
	memset( &m_wViolationIndex, 0x00, sizeof(WORD) * BIN_COUNT_PER_PAGE);
	memset( &m_wAnotherIndex, 0x00, sizeof(WORD) * BIN_COUNT_PER_PAGE);
}
/*-------------------------------------------------------------------
 CLASS    NAME: CBinMgr
 FUNCTION NAME: SetEditBinInformation()
 RETURN TYPE  : 
 PARAMETER    : BIN을 Edit할지 여부, Edit할 Index 
 DESCRIPTION  : BIN LIST 화면 진입시 검색에 필요한 데이타를 초기화함  
-------------------------------------------------------------------*/
void CBinMgr::SetEditBinInformation(BOOL bEdit, WORD wIndex)
{
	if ( bEdit ) //기존 BIN Edit
	{
		m_wEditBinFlag = 1;
		m_wEditIndex = wIndex;
		m_wCurrentBinIndex = wIndex; 

		memcpy( m_EditProperties.BinNo, m_pBinBlock->BinList[wIndex].BinNo, BIN_NO_SIZE);
		m_EditProperties.wAction = m_pBinBlock->BinList[wIndex].wAction;
		m_EditProperties.dwValue = m_pBinBlock->BinList[wIndex].dwValue;

		m_CurrentProperties =  m_EditProperties;
	}
	else  //신규 등록
	{
		m_wEditBinFlag = 0;
		m_wEditIndex = 0;
		memset( &m_EditProperties , 0x00, sizeof(BinProperties));
		memset( &m_CurrentProperties , 0x00, sizeof(BinProperties));
	}
}
/*-------------------------------------------------------------------
 CLASS    NAME: CBinMgr
 FUNCTION NAME: GetCurrentPageNumber()
 RETURN TYPE  : Current Page 번호 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
WORD CBinMgr::GetCurrentPageNumber()
{
	return m_wCurrentBinPage;
}
/*-------------------------------------------------------------------
 CLASS    NAME: CBinMgr
 FUNCTION NAME: GetEditBinFlag()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 현재 편집중인 BIN이 Add인지 Edit인지 Flag
-------------------------------------------------------------------*/
BOOL CBinMgr::GetEditBinFlag()
{
	if ( m_wEditBinFlag == 0)
		return FALSE;
	else
		return TRUE;
}
/*-------------------------------------------------------------------
 CLASS    NAME: CBinMgr
 FUNCTION NAME: GetEditBinInformation()
 RETURN TYPE  : Edit 인지 ADD 인지 여부 
 PARAMETER    : 현재 편집중인 BIN인 BIN인 INDEX 및 BIN PROPERTY
 DESCRIPTION  : 현재 편집중인 BIN인 BIN인 INDEX 및 BIN PROPERTY 전달 달해줌 
-------------------------------------------------------------------*/
BOOL CBinMgr::GetEditBinInformation(WORD *wIndex, LPBinProperties lpProperties)
{
	if ( m_wEditBinFlag == 0)
	{
		memcpy(lpProperties, &m_EditProperties, sizeof(BinProperties));
		return FALSE;
	}
	else
	{
		*wIndex = m_wEditIndex;
		memcpy(lpProperties, &m_EditProperties, sizeof(BinProperties));
		return TRUE;
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: CBinMgr
 FUNCTION NAME: SetEditBinInformationBinNo()
 RETURN TYPE  :  
 PARAMETER    : 현재 편집중인 BIN No
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CBinMgr::SetEditBinInformationBinNo(char *szBin)
{
	memcpy(m_EditProperties.BinNo, szBin, BIN_NO_SIZE);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CBinMgr
 FUNCTION NAME: SetEditBinInformationBinNo()
 RETURN TYPE  :  
 PARAMETER    : 현재 편집중인 BIN의 ACTION
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CBinMgr::SetEditBinInformationAction(WORD wAction)
{
	m_EditProperties.wAction = wAction;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CBinMgr
 FUNCTION NAME: SetEditBinInformationValue()
 RETURN TYPE  :  
 PARAMETER    : 현재 편집중인 BIN의 VALUE
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CBinMgr::SetEditBinInformationValue(DWORD dwValue)
{
	m_EditProperties.dwValue = dwValue; 
}

/*-------------------------------------------------------------------
 CLASS    NAME: CBinMgr
 FUNCTION NAME: GetSeachBinNumber()
 RETURN TYPE  :  
 PARAMETER    : 현재 Search 명령을 수행중인 BIN NO 얻어옴 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CBinMgr::GetSeachBinNumber(char *szBin)
{
	memcpy(szBin, m_SearchBinNo, BIN_NO_SIZE);	
}

/*-------------------------------------------------------------------
 CLASS    NAME: CBinMgr
 FUNCTION NAME: SetSearchBinNumber()
 RETURN TYPE  :  
 PARAMETER    : 현재 Search 명령을 수행중인 BIN NO 셋팅 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CBinMgr::SetSearchBinNumber(char *szBin)
{
	memcpy(m_SearchBinNo, szBin, BIN_NO_SIZE);
}
/*-------------------------------------------------------------------
 CLASS    NAME: CBinMgr
 FUNCTION NAME: SearchNextBinNumber()
 RETURN TYPE  :  
 PARAMETER    :  
 DESCRIPTION  : Next Search Index에서 부터 설정된 BIN NO로 
              검색하여 그 검색되었는지 여부를 전달함. 
-------------------------------------------------------------------*/
BOOL CBinMgr::SearchNextBinNumber()
{
	BOOL bFound = FALSE;

	if ( m_pBinBlock->wBinTotalCount == 0) return bFound;

	for ( int i = 0; i < m_pBinBlock->wBinTotalCount; i++)
	{
		if ( memcmp(m_pBinBlock->BinList[i].BinNo, m_SearchBinNo, strlen(m_SearchBinNo)) == 0)
		{
			m_wCurrentBinIndex = i;
			bFound = TRUE;
			break;
		}
	}
	if ( bFound )
	{
		WORD wPage = m_wCurrentBinIndex / BIN_COUNT_PER_PAGE;
		m_wCurrentBinPage = ++wPage;
	}
	return bFound;
}
/*-------------------------------------------------------------------
 CLASS    NAME: CBinMgr
 FUNCTION NAME: GetViolationBinCount()
 RETURN TYPE  :  
 PARAMETER    :  
 DESCRIPTION  : 입력하려는 BIN과 충돌되는 BIN COUNT를 리턴함. 
-------------------------------------------------------------------*/
WORD CBinMgr::GetViolationBinCount()
{
	return m_wViolationCount;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CBinMgr
 FUNCTION NAME: GetViolationBinCount()
 RETURN TYPE  :  
 PARAMETER    :  
 DESCRIPTION  : CURRENT BIN과 TARGET BIN이 충돌 여부 Check  
-------------------------------------------------------------------*/
BOOL CBinMgr::IsVoilated(BinProperties curBin, BinProperties targetBin)
{
	BOOL bVoilated = FALSE;

	switch( curBin.wAction)
	{
	case ACTION_BLOCK_SURCHARGE:
		{
			switch( targetBin.wAction)
			{
			case ACTION_WITHDRAWAL_SURCHARGE:
			case ACTION_BALANCE_SURCHARGE:
			case ACTION_DECLINE_CARD:
			case ACTION_BLOCK_SURCHARGE:
				bVoilated = TRUE;
				break;
			}
		}
		break;
	case ACTION_WITHDRAWAL_SURCHARGE:
		{
			switch( targetBin.wAction)
			{
			case ACTION_BLOCK_SURCHARGE:
			case ACTION_WITHDRAWAL_SURCHARGE:
			case ACTION_DECLINE_CARD:
				bVoilated = TRUE;
				break;
			}
		}
		break;
	case ACTION_BALANCE_SURCHARGE:
		{
			switch( targetBin.wAction)
			{
			case ACTION_BLOCK_SURCHARGE:
			case ACTION_BALANCE_SURCHARGE:
			case ACTION_DECLINE_CARD:
				bVoilated = TRUE;
				break;
			}
		}
		break;
	case ACTION_ACCEPT_CARD: //[#545] SOOK 2009.09.01 
		{
			switch( targetBin.wAction)
			{
			case ACTION_ACCEPT_CARD:
			case ACTION_DECLINE_CARD:
				bVoilated = TRUE;
				break;
			}
		}
		break;
	case ACTION_AWARD_COUPON:
		{
			switch( targetBin.wAction)
			{
			case ACTION_AWARD_COUPON:
			case ACTION_DECLINE_CARD:
				bVoilated = TRUE;
				break;
			}
		}
		break;
	case ACTION_PIN_CHANGE:
		{
			switch( targetBin.wAction)
			{
			case ACTION_PIN_CHANGE: //end of [#545]
			case ACTION_DECLINE_CARD:
				bVoilated = TRUE;
				break;
			}
		}
		break;
	case ACTION_DECLINE_CARD:
		{
			switch( targetBin.wAction)
			{
			case ACTION_BLOCK_SURCHARGE:
			case ACTION_WITHDRAWAL_SURCHARGE:
			case ACTION_BALANCE_SURCHARGE:
			case ACTION_ACCEPT_CARD:
			case ACTION_AWARD_COUPON:
			case ACTION_PIN_CHANGE:
			case ACTION_DECLINE_CARD:
				bVoilated = TRUE;
				break;
			}
		}
		break;
	}
	return bVoilated;
}
/*-------------------------------------------------------------------
 CLASS    NAME: CBinMgr
 FUNCTION NAME: MakeTransactionBinList()
 RETURN TYPE  : 
 PARAMETER    : 현재 BIN 정보, 위배되는 항목을 저장하기 위한 배열, 
                리스트를 신규 잘성할 것인지 기존값 활용할 것인지 여부 
 DESCRIPTION  : 거래시 해당 BIN이 존재하는지 여부 확인하는 
-------------------------------------------------------------------*/
WORD CBinMgr::MakeTransactionBinList(CString strIDCTrack2)
{
	memset(m_TransactionBinPropertiesList, 0x00, sizeof( BinProperties)*BIN_COUNT_PER_PAGE);
	m_wTransactionBinCount = 0;

	CString strBinList;
	BOOL bExist = FALSE;
	WORD wBinLength = 0;
	for ( int i = 0; i < m_pBinBlock->wBinTotalCount; i++)
	{
		strBinList.Format(L"%S", m_pBinBlock->BinList[i].BinNo);
		strBinList.TrimRight();
		if ( bExist == FALSE)
		{
			if( strBinList == strIDCTrack2.Left(strBinList.GetLength()) )
			{
				wBinLength = strBinList.GetLength();
				bExist = TRUE;

				m_TransactionBinPropertiesList[m_wTransactionBinCount++] = m_pBinBlock->BinList[i];
			}
		}
		else
		{
			if ( strBinList == strIDCTrack2.Left(wBinLength))
			{
				m_TransactionBinPropertiesList[m_wTransactionBinCount++] = m_pBinBlock->BinList[i];
			}
		}
	}

	return m_wTransactionBinCount;
}
/*-------------------------------------------------------------------
 CLASS    NAME: CBinMgr
 FUNCTION NAME: GetTransactionBinProperties()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 해당 ACTION의 존재 여부와 Value 값을 얻어옴 
-------------------------------------------------------------------*/
BOOL CBinMgr::GetTransactionBinProperties(WORD action, DWORD *value)
{
	BOOL bExist = FALSE;

	if ( m_wTransactionBinCount > 0) 
	{
		for ( int i = 0; i < m_wTransactionBinCount; i++)
		{
			if ( action == m_TransactionBinPropertiesList[i].wAction )
			{
				*value = m_TransactionBinPropertiesList[i].dwValue;
				bExist = TRUE;
				break;
			}
		}
	}
	return bExist;
}
/*-------------------------------------------------------------------
 CLASS    NAME: CBinMgr
 FUNCTION NAME: GetBinPropertesByIndex()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Index에 해당하는 BIN Properties를 얻어옴  
-------------------------------------------------------------------*/
BOOL CBinMgr::GetBinPropertesByIndex(WORD wIndex, LPBinProperties lpBin)
{
	if ( wIndex >= m_pBinBlock->wBinTotalCount ) return FALSE;

	memcpy( lpBin, &m_pBinBlock->BinList[wIndex], sizeof(BinProperties));
	return TRUE;
}

void CBinMgr::ClearBinList()
{
	//m_pBinBlock->wAllowOnlyBinEnable = 0x00;
	NvramCopyWord(&m_pBinBlock->wAllowOnlyBinEnable, 0);		// [#573] NH AIREAT 2009.10.09 SW NVRAM
	DeleteAllList();
}

BOOL CBinMgr::InsertBinPropertiesWithoutSorting(BinProperties newBin)
{
	if ( m_pBinBlock->wBinTotalCount < BIN_MAXIMUN_COUNT)
	{
		// [#573] NH AIREAT 2009.10.09 SW NVRAM
		//m_pBinBlock->BinList[m_pBinBlock->wBinTotalCount++] = newBin;
		NvramCopyMemory(&m_pBinBlock->BinList[m_pBinBlock->wBinTotalCount], &newBin, sizeof(BinProperties));
		NvramCopyWord(&m_pBinBlock->wBinTotalCount, (WORD)(m_pBinBlock->wBinTotalCount+1));
		// end of [#573]
		return TRUE;
	}
	else
		return FALSE;
	
}

// [#2046] AU KJW 2011.04.19
/*-------------------------------------------------------------------
 CLASS    NAME: CBinMgr
 FUNCTION NAME: GetBinBlock()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 파라미터 포인터로 BinBlock을 복사함.
-------------------------------------------------------------------*/
BOOL CBinMgr::GetBinBlock(LPBinBlock pBinBlock)
{
	if( pBinBlock == NULL || m_pBinBlock == NULL )
	{
		DB_DEBUG(ON, (_T("[CBinMgr::GetBinBlock()::MappingBinBlock] memory mapping failed. BinBloack(0x%X)\n"), m_pBinBlock));
		return FALSE;
	}

	if( m_pBinBlock->wBinTotalCount > BIN_MAXIMUN_COUNT )
		return FALSE;

	memcpy( &pBinBlock->nReservedNVRAMVersionInfo, &m_pBinBlock->nReservedNVRAMVersionInfo, sizeof(pBinBlock->nReservedNVRAMVersionInfo) );
	memcpy( &pBinBlock->wAllowOnlyBinEnable, &m_pBinBlock->wAllowOnlyBinEnable, sizeof(pBinBlock->wAllowOnlyBinEnable) );
	memcpy( &pBinBlock->wBinTotalCount, &m_pBinBlock->wBinTotalCount, sizeof(pBinBlock->wBinTotalCount) );
	for( int i = 0 ; i < m_pBinBlock->wBinTotalCount ; i++ )
	{
		memcpy( &pBinBlock->BinList[i], &m_pBinBlock->BinList[i], sizeof(pBinBlock->BinList[0]) );
	}
	memcpy( pBinBlock->reserved, m_pBinBlock->reserved, sizeof(pBinBlock->reserved) );

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CBinMgr
 FUNCTION NAME: SetBinBlock(LPBinBlock pBinBlock)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 파라미터 포인터의 값으로 내부 BinBlock을 덮어씀.
-------------------------------------------------------------------*/
BOOL CBinMgr::SetBinBlock(LPBinBlock pBinBlock)
{
	if( pBinBlock == NULL || m_pBinBlock == NULL )
	{
		DB_DEBUG(ON, (_T("[CBinMgr::SetBinBlock()::MappingBinBlock] memory mapping failed. BinBloack(0x%X)\n"), m_pBinBlock));
		return FALSE;
	}

	if( pBinBlock->wBinTotalCount > BIN_MAXIMUN_COUNT )
		return FALSE;

	InitBinVariable();
	//NvramCopyMemory( m_pBinBlock, pBinBlock, sizeof(BinBlock) );
	NvramCopyMemory( &m_pBinBlock->nReservedNVRAMVersionInfo, &pBinBlock->nReservedNVRAMVersionInfo, sizeof(m_pBinBlock->nReservedNVRAMVersionInfo) );
	NvramCopyMemory( &m_pBinBlock->wAllowOnlyBinEnable, &pBinBlock->wAllowOnlyBinEnable, sizeof(m_pBinBlock->wAllowOnlyBinEnable) );
	NvramCopyMemory( &m_pBinBlock->wBinTotalCount, &pBinBlock->wBinTotalCount, sizeof(m_pBinBlock->wBinTotalCount) );
	for( int i = 0 ; i < pBinBlock->wBinTotalCount ; i++ )
	{
		NvramCopyMemory( &m_pBinBlock->BinList[i], &pBinBlock->BinList[i], sizeof(m_pBinBlock->BinList[0]) );
	}
	NvramCopyMemory( m_pBinBlock->reserved, pBinBlock->reserved, sizeof(m_pBinBlock->reserved) );

	return TRUE;
}
// end of [#2046]
// [#2487] AU KSK 2017.06.08
/*-------------------------------------------------------------------
CLASS    NAME: CBinMgr
FUNCTION NAME: IsExistRegisterSurcharge
RETURN TYPE  : 
PARAMETER    : None
DESCRIPTION  : 등록된 출금 / 조회 수수료가 등록되어져 있는지 검색
-------------------------------------------------------------------*/
BOOL CBinMgr::IsExistRegisterSurcharge()
{
	if( m_pBinBlock == NULL )
	{
		DB_DEBUG(ON, (_T("[CBinMgr::IsExistRegisterSurcharge()::MappingBinBlock] memory mapping failed. BinBloack(0x%X)\n"), m_pBinBlock));
		return FALSE;
	}

	if ( m_pBinBlock->wBinTotalCount <= 0 )
		return FALSE;

	if( m_pBinBlock->wBinTotalCount > BIN_MAXIMUN_COUNT )
		return FALSE;

	for ( int i = 0 ; i < m_pBinBlock->wBinTotalCount; i++)
	{
		BinProperties targetBin = m_pBinBlock->BinList[i];
		if (targetBin.wAction == ACTION_WITHDRAWAL_SURCHARGE || targetBin.wAction == ACTION_BALANCE_SURCHARGE || targetBin.wAction == ACTION_BLOCK_SURCHARGE)
		{
			// Block 및 수수료가 0인 경우에는 수수료를 미부과하므로 $0.00 표시로 처리
			NVDump('O', 'D', "76", L"", L"BIN SURCH DETECT");
			return TRUE;
		}
	}

	NVDump('O', 'D', "76", L"", L"NO BIN SURCH");

	return FALSE;
}
// end of [#2487]

// [#2487] AU KSK 2017.06.08
/*-------------------------------------------------------------------
 CLASS    NAME: CBinMgr
 FUNCTION NAME: GetMinimumSurcharge
 RETURN TYPE  : 
 PARAMETER    : None
 DESCRIPTION  : 등록된 출금/조회 수수료의 최소 금액 검색
-------------------------------------------------------------------*/
int CBinMgr::GetMinimumSurcharge()
{
	CString strTemp;
	BOOL bFindRegisterSurcharge = FALSE;
	int nMinimumSurcharge = 0;

	if( m_pBinBlock == NULL )
	{
		DB_DEBUG(ON, (_T("[CBinMgr::GetMinimumSurcharge()::MappingBinBlock] memory mapping failed. BinBloack(0x%X)\n"), m_pBinBlock));
		return -1;
	}

	if ( m_pBinBlock->wBinTotalCount <= 0 )
		return -1;

	if( m_pBinBlock->wBinTotalCount > BIN_MAXIMUN_COUNT )
		return -1;

	for ( int i = 0 ; i < m_pBinBlock->wBinTotalCount; i++)
	{
		BinProperties targetBin = m_pBinBlock->BinList[i];
		if (targetBin.wAction == ACTION_WITHDRAWAL_SURCHARGE || targetBin.wAction == ACTION_BALANCE_SURCHARGE || targetBin.wAction == ACTION_BLOCK_SURCHARGE)
		{
			if (bFindRegisterSurcharge == FALSE)
			{
				if (targetBin.wAction == ACTION_BLOCK_SURCHARGE)
					nMinimumSurcharge = 0;
				else
					nMinimumSurcharge = targetBin.dwValue;

				// 최초에 찾은 Surcharge값을 Set
				bFindRegisterSurcharge = TRUE;

			}

			if (targetBin.dwValue < (DWORD)nMinimumSurcharge)
				nMinimumSurcharge = targetBin.dwValue;
		}
	}

	if (bFindRegisterSurcharge == FALSE)	// Bin List에 Surcharge가 미 등록된 Case
	{
		NVDump('O', 'D', "77", L"", L"NO BIN SURCH");
		return -2;
	}

	strTemp.Format(L"MIN SUR:%d", nMinimumSurcharge);
	NVDump('O', 'D', "76", L"", strTemp);
	return nMinimumSurcharge;	// 찾은 최소 Surcharge 금액을 return
}
// end of [#2487]

// [#2487] AU KSK 2017.06.08
/*-------------------------------------------------------------------
 CLASS    NAME: CBinMgr
 FUNCTION NAME: GetMaximumSurcharge
 RETURN TYPE  : 
 PARAMETER    : None
 DESCRIPTION  : 등록된 출금/조회 수수료의 최대 금액 검색
-------------------------------------------------------------------*/
int CBinMgr::GetMaximumSurcharge()
{
	CString strTemp;
	BOOL bFindRegisterSurcharge = FALSE;
	int nMaximumSurcharge = 0;

	if( m_pBinBlock == NULL )
	{
		DB_DEBUG(ON, (_T("[CBinMgr::GetMaximumSurcharge()::MappingBinBlock] memory mapping failed. BinBloack(0x%X)\n"), m_pBinBlock));
		return -1;
	}

	if ( m_pBinBlock->wBinTotalCount <= 0 )
		return -1;

	if( m_pBinBlock->wBinTotalCount > BIN_MAXIMUN_COUNT )
		return -1;

	for ( int i = 0 ; i < m_pBinBlock->wBinTotalCount; i++)
	{
		BinProperties targetBin = m_pBinBlock->BinList[i];
		if (targetBin.wAction == ACTION_WITHDRAWAL_SURCHARGE || targetBin.wAction == ACTION_BALANCE_SURCHARGE || targetBin.wAction == ACTION_BLOCK_SURCHARGE)
		{
			if (bFindRegisterSurcharge == FALSE)
			{
				if (targetBin.wAction == ACTION_BLOCK_SURCHARGE)
					nMaximumSurcharge = 0;
				else
					nMaximumSurcharge = targetBin.dwValue;

				// 최초에 찾은 Surcharge값을 Set
				bFindRegisterSurcharge = TRUE;
			}

			if (targetBin.dwValue > (DWORD)nMaximumSurcharge)
				nMaximumSurcharge = targetBin.dwValue;
		}
	}

	if (bFindRegisterSurcharge == FALSE)	// Bin List에 Surcharge가 미 등록된 Case
	{
		NVDump('O', 'D', "78", L"", L"NO BIN SURCH");
		return -2;
	}

	strTemp.Format(L"MAX SUR:%d", nMaximumSurcharge);
	NVDump('O', 'D', "76", L"", strTemp);
	return nMaximumSurcharge;	// 찾은 최소 Surcharge 금액을 return
}
// end of [#2487]

