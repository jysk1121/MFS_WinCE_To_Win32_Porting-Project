#ifndef __JNL_MGR_H__
#define __JNL_MGR_H__

#include ".\Dev\JnlDB.h"
#include ".\Common\ConstDef.h"
#include ".\Dev\DevDefine.h"
#include "..\Common\ScreenDef.h"						// [#2160] CA KMK 2012.11.12

class CDevCmn;

class AFX_CLASS_EXPORT CJnlMgr
{
public:
	CJnlMgr();
	~CJnlMgr();

public:
	/* Initialize */
	BOOL	Initialize(CWnd *pDevCmn);													// JNL DB를 초기화 한다.

	BOOL	Deinitialize();																// JNL DB를 해제 한다.
	BOOL	ClearJNL();																	// 모든 JNL를 삭제한다.

	/* JNL Save   */
	BOOL Save(CString strKindCode, CString strEJNLData=L"");								// JNL을 저장한다.
	//[#610] SOOK 2009.12.21 Parameter Change Journal 저장 
//#if SUPPORT_CHANGE_PARAMETER_JNL   
	BOOL SaveChangeParameterJnl(int User, CString strParameter, CString oldValue, CString newValue);	
	BOOL SaveChangeParameterJnl(int User, CString strParameter, CStringArray &oldValue, CStringArray &newValue);	
//#endif 
	
	/* JNL Search */
	CString	SearchIndex(UINT32 nJnlIndex);												// JNL Index를 JNL을 검색한다.
	CString MoveFirst();																// First JNL으로 이동한다.
	CString MoveLast();																	// Last JNL으로 이동한다.
	CString MoveNext(UINT32 nSize=1);													// 정해진 크기만큼 떨어진 다음 JNL으로 이동한다.
	CString MovePrev(UINT32 nSize=1);													// 정해진 크기만큼 떨어진 이전 JNL으로 이동한다.

	BOOL	SearchByDate(CTime StartDate, CTime EndDate, CStringArray &Indexs);			// 정해진 기간에 해당하는 모든 저널의 인덱스를 가져온다.		// [#508] AIREAT 2009.02.27 : Search by date Method 추가

	/* Back Up */
	BOOL	BackupJNLData(CString strDestPath);											// 현재 모든 저널 데이터를 주어진 Path에 Copy 한다.
	BOOL	SaveJNLData(CString strFileName, CStringArray &Indexs);						// Indexs에 해당하는 저널을 주어진 파일에 저장한다.				// [#508] AIREAT 2009.02.27 : Save JNL Data Method 추가

	/* JNL Information */
	INT32	GetFirstIndex();															// Journal의 처음 Index를 조회한다.
	INT32	GetLastIndex();																// Journal의 마지막 Index를 조회한다.
	INT32	GetCurrentIndex();															// Current Position의 Journal Index를 조회한다.
	INT32	GetTotalCount();															// Journal의 총 개수를 조회한다.

	INT32	GetJNLStatus();																// Journal 상태를 조회한다.(NORMAL / DOWN) [#462] [NH] KSK 2008.12.09

	/* JNL Upload Methods */
	INT32	GetUploadLastIndex(UP_TYPE Type);											// Upload된 마지막 Journal Index를 조회한다.
	INT32	GetUploadRemainCount(UP_TYPE Type);											// Upload할 Journal Count를 조회한다.
	CString UploadMoveNext(UP_TYPE Type);												// Upload할 다음 Journal를 가져온다.
	BOOL	DoesJnlImageExist(int StackNo, int StackYear, int StackMonth, int StackDay);//[#586] SOOK 2009.12.08 iTM MoniView 연동 부분 추가 
	CString GetJnlImageFileName(int StackNo, int StackYear, int StackMonth, int StackDay);

	INT32	SetUploadedIndex(UP_TYPE Type, INT32 nJnlIndex);							// [#2076] NH KSK 2011.06.29 Upload 성공 확인 후 Set하는 함수 추가
	
	/* JNL Convert */
	CString	ConvertPrintData(int PrintDevice, CString strJnlData);						// JNL Data를 SPR Print Data로 변환한다.

	/* Helper Methods */
private:
	CString ConvertAddCashToPrint(int DescKindCodeIndex, CStringArray &arJnlField);				// ADD CASH JNL -> PRINT DATA
	CString ConvertDayTotalToPrint(int DescKindCodeIndex, CStringArray &arJnlField);			// DAY TOTAL JNL -> PRINT DATA
	CString ConvertCstTotalToPrint(int DescKindCodeIndex, CStringArray &arJnlField);			// CST TOTAL JNL -> PRINT DATA
	CString ConvertCashInTotalToPrint_Demo(int DescKindCodeIndex, CStringArray &arJnlField);	// CASHIN TOTAL JNL -> PRINT DATA	// [#GLDV-3005] US Kook 2022.04.14
	CString ConvertDenoToPrint(int DescKindCodeIndex, CStringArray &arJnlField);				// CHANGE DENOMINATION JNL -> PRINT DATA
	CString ConvertCommMsgToPrint(int DescKindCodeIndex, CStringArray &arJnlField);				// COMMUNICATIUON MESSAGE JNL -> PRINT DATA
	CString ConvertTranToPrint(int DescKindCodeIndex, CStringArray &arJnlField);				// TRANSACTION JNL -> PRINT DATA
	CString ConvertErrorToPrint(int DescKindCodeIndex, CStringArray &arJnlField);				// ERROR JNL -> PRINT DATA
	CString ConvertOperatorToPrint(int DescKindCodeIndex, CStringArray &arJnlField);			// Operator Mode JNL -> PRINT DATA
	CString ConvertPasswordToPrint(int DescKindCodeIndex, CStringArray &arJnlField);			// Change Password JNL -> PRINT DATA
	CString ConvertOperatorAction(int DescKindCodeIndex, CStringArray &arJnlField);				// Change Config JNL -> PRINT DATA		// [#408] AU AIREAT 2008.08.26
	CString ConvertChangeExchangeRate(int DescKindCodeIndex, CStringArray &arJnlField);			// Change Config JNL -> PRINT DATA		// [#408] AU AIREAT 2008.08.26
	CString ConvertUserCancelToPrint(int DescKindCodeIndex, CStringArray &arJnlField);			// User Cancel JNL -> PRINT DATA		// [#529] AU AIREAT 2009.06.02
	CString ConvertDefaultToPrint(int DescKindCodeIndex, CStringArray &arJnlField);				// Default JNL -> PRINT DATA
	CString ConvertTrxEMVDataToPrint(int DescKindCodeIndex, CStringArray &arJnlField);			// [#2115] MX KSK 2012.02.05
	CString ConvertBriefDayTotalToPrint(int DescKindCodeIndex, CStringArray &arJnlField);
	CString ConvertBriefCstTotalToPrint(int DescKindCodeIndex, CStringArray &arJnlField);
	CString ConvertLibertyXToPrint(int DescKindCodeIndex, CStringArray &arJnlField);			// [#RWC6-59] US William 2019.10.28 LibertyX
	CString ConvertLibertyXDispenseToPrint(int DescKindCodeIndex, CStringArray &arJnlField);	// [#RWC6-224] US William 2020.10.14 LibertyX Dispense
	CString ConvertB4UToPrint(int DescKindCodeIndex, CStringArray &arJnlField);					// [#RWC6-188] B4U Log journal entry
	CString ConvertDigitalMintToPrint(int DescKindCodeIndex, CStringArray &arJnlField);			// [#RWC6-399] US ryan.payton 2022.08.02 DigitalMint
	CString ConvertTangoPayToPrint(int DescKindCodeIndex, CStringArray &arJnlField);			// [#RWC6-676] SKKim 2024.05.09
	
	//[#610] SOOK 2009.12.21 Parameter Change Journal 저장 
//#if SUPPORT_CHANGE_PARAMETER_JNL   	
	CString ConvertChangeParameterToPrint (int DescKindCodeIndex, CStringArray &arJnlField);		// Change Parameter -> PRINT DATA
//#endif

	/*	Variable  */
private:
	BOOL			m_bInit;		// Initialize.

// 	CWnd			*m_pDevCmn;		// DevCmn pointer
	CDevCmn			*m_pDevCmn;		// [#2160] CA KMK 2012.11.12 type-casting 간소화

	CJnlDB			m_JnlDB;		// E-JNL DB
	INT32			m_nJNLStatus;	// [#462] [NH] KSK 2008.12.09
};

#endif __JNL_MGR_H__