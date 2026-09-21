// BinMgr.h: interface for the CBinMgr class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __BIN_MGR_H__
#define __BIN_MGR_H__

//#include ".\Common\MB2500DLL.h"					// [#573] NH AIREAT 2009.10.26 SW NVRAM
#include ".\Common\CmnLib.h"

#define BIN_BLOCK_SIZE	(1024*128)
#define BIN_MAXIMUN_COUNT	8000
#define BIN_NO_SIZE			10
#define BIN_COUNT_PER_PAGE	10
#define RESEVED_AREA_START_VERSION	(0x78010000)


#if (_WIN32_WCE >= 0x600)	// [#584] NH KSK 2009.12.02
#pragma pack(1)
#endif

//////////////////////////////////////
// Multi-Funtion BIN 저장용 구조체
typedef struct tab_BinProperties
{
	CHAR	BinNo[BIN_NO_SIZE];
	WORD	wAction;
	DWORD	dwValue;
} BinProperties, *LPBinProperties;


typedef struct tag_BinBlock
{
	INT32	nReservedNVRAMVersionInfo;
	WORD	wAllowOnlyBinEnable;
	WORD	wBinTotalCount;

	BinProperties BinList[BIN_MAXIMUN_COUNT]; 
	char	reserved[96*16 -8];
//	char	reserved[192*16 -8];

} BinBlock, *LPBinBlock;

#if (_WIN32_WCE >= 0x600)	// [#584] NH KSK 2009.12.02
#pragma pack()
#endif

class AFX_CLASS_EXPORT CBinMgr  
{
public:
	CBinMgr();
	virtual ~CBinMgr();

private:	
	LPBinBlock m_pBinBlock;

	//총 64K중 16 * 4000개만 지원하고 나머지 1536 BYTE는 나머지 작업을 위해 사용 16 *96
	BinProperties m_CurrentPageBinList[BIN_COUNT_PER_PAGE];
	BinProperties m_AnotherPropertiesList[BIN_COUNT_PER_PAGE];
	BinProperties m_ViolationPropertiesList[BIN_COUNT_PER_PAGE];
	BinProperties m_TransactionBinPropertiesList[BIN_COUNT_PER_PAGE];

	//나머지중 30개는 조회를 위해서 사용함  (16 * 56)
	BinProperties m_EditProperties;
	BinProperties m_CurrentProperties; 

	//나머지중 30개는 조회를 위해서 사용함  (16 * 54)
	WORD	m_wViolationIndex[BIN_COUNT_PER_PAGE];
	WORD    m_wAnotherIndex[BIN_COUNT_PER_PAGE];
	WORD	m_wCurrentBinIndex;
	WORD	m_wCurrentBinPage;
	WORD	m_wViolationCount; 
	WORD	m_wAnotherPropertiesCount;

	WORD	m_wEditBinFlag;

	WORD	m_wEditIndex;
	WORD	m_wTransactionBinCount;
	char	m_SearchBinNo[BIN_NO_SIZE+1];

private:
	BOOL IsVoilated(BinProperties curBin, BinProperties targetBin);

public:
	BOOL InsertBinPropertiesWithoutSorting(BinProperties newBin);
	void SetLastTransactionCode(WORD TranCode);
	WORD GetLastTransactionCode();
	void ClearBinList();
	BOOL GetBinPropertesByIndex(WORD wIndex, LPBinProperties lpBin);
	BOOL GetTransactionBinProperties(WORD action, DWORD* value);
	WORD MakeTransactionBinList(CString strIDCTrack2);
	WORD GetViolationBinCount();
	BOOL SearchNextBinNumber();
	void SetSearchBinNumber(char * szBin);
	void GetSeachBinNumber(char * szBin);
	void SetEditBinInformationValue(DWORD dwValue);
	void SetEditBinInformationAction(WORD action);
	void SetEditBinInformationBinNo(char * szBin);
	BOOL GetEditBinInformation(WORD *wIndex, LPBinProperties lpProperties);
	BOOL GetEditBinFlag();
	WORD GetCurrentPageNumber();
	void SetEditBinInformation(BOOL bEdit, WORD wIndex = 0 );
	void InitBinVariable();
	CString GetActionString(WORD action);
	WORD GetTotalPage();
	BOOL MoveCurrentPage(int MovePage);
	CString MakeHeaderPrintData(CString strTerminalID);
	CString MakeBinPrintData(WORD pageno);
	WORD GetViolationProperties(BinProperties curBin, LPBinProperties lpProperties, BOOL bUpdate = FALSE);
	WORD GetAnotherProperties(BinProperties curBin, LPBinProperties lpProperties);
	WORD GetTotalBinCount();
	WORD GetCurrentBinListPage(LPBinProperties lpProperties);
	void DeleteBinProperites(WORD index, BOOL bIndexUpdate = TRUE);
	void ModifyBinProperites(WORD index, BinProperties editBin);
	BOOL InsertBinProperties(BinProperties newBin);
	void SetAllowOnlyBinEnable(WORD value);
	WORD GetAllowOnlyBinEnable();
	void DeleteAllList();

	// [#2046] AU KJW 2011.04.19
	BOOL GetBinBlock(LPBinBlock pBinBlock);
	BOOL SetBinBlock(LPBinBlock pBinBlock);
	// end of [#2046]

	// [#2487] AU KSK 2017.06.08
	BOOL IsExistRegisterSurcharge();
	int GetMinimumSurcharge();
	int GetMaximumSurcharge();
	// end of [#2487]
};

#endif __BIN_MGR_H__
