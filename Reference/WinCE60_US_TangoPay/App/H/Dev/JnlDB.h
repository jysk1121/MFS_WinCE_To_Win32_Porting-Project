#ifndef __NH_JNL_DB_H__
#define __NH_JNL_DB_H__

//#include ".\Common\MB2500DLL.h"				// [#573] NH AIREAT 2009.10.26 SW NVRAM
#include ".\Common\CmnLib.h"

#define EJ_BLOCK_SIZE			(1024*32)						// E-JNL BLOCK SIZE.
#define EJ_SAVE_BLOCK_INDEX		(0)								// E-JNL Save Block Index in EJ Block.
#define EJ_SEARCH_BLOCK_INDEX	(1)								// E-JNL Search Block Index in EJ Block.
#define EJ_DATA_SIZE			(EJ_BLOCK_SIZE-(8*4))			// E-JNL BLOCK의 DATA SIZE.
#define EJ_LOGGING_MAX			(100)							// 한개의 파일에 Logging 할수 있는 최대 개수.
#define EJ_INDEX_INIT			(-1)							// E-JNL 초기 Index.
#define EJ_INDEX_START			(1)								// E-JNL 시작 Index.
#define EJ_INDEX_MAX			(9999)							// 최대 E-JNL Index.
#define EJ_FILE_INDEX_MAX		(EJ_INDEX_MAX/EJ_LOGGING_MAX)	// 최대 File Index.

#define EJ_JNL_DELIMITER		((char)0x1C)					// E-JNL간의 구분자.

#define MAPPING_TRY_MAX			(2)								// Memroy Mapping Try Count

#define EJ_HEADER_SIZE			(38)							// E-Journal Header Size.
#define EJ_MAX_DATA_SIZE		(1024)							// a E-Journal Max Data size 9999


//////////////////////////////////////
// JNL DB Version
#define JNLDB_VERSION_START		(0x56010000)
//#define JNLDB_VERSION			(0x56010000)					// first, BaseDef_XX.h로 이동.

#pragma pack(1)

//////////////////////////////////////
// E-JNL 저장용 구조체
typedef struct tag_EJSaveBlock
{
	INT32	nVersionInfo;
	INT32	nStatus;							// 0 - Logging, 1 - Saving, 2 - End to save, 3 - Block Initialing
	INT32	nFirstIndex;						// E-JNL start index of logging.
	INT32	nLastIndex;							// E-JNL last index of logging.
	INT32	nLastPrintIndex;					// E-JNL Last index of print.
	INT32	nLastRMSUploadIndex;				// E-JNL last index of RMS Upload.
	INT32	nLastHostUploadIndex;				// E-JNL last index of Host upload.
	INT32	nNextDataOffset;					// E-JNL pointer of EJData for Next logging.

	CHAR	EJData[EJ_DATA_SIZE];				// E-JNL Data.

} EJSaveBlock, *LPEJSaveBlock;

//////////////////////////////////////
// E-JNL 검색용 구조체
typedef struct tag_EJSearchBlock
{
	INT32	nLoadedFileIndex;					// E-JNL index of load file. 00 - 99. -1 : unload.
	INT32	nStartIndex;						// E-JNL start index in load file.
	INT32	nEndIndex;							// E-JNL end index in load file.
	INT32	nReserve1; 							// Reserved
	INT32	nReserve2;							// Reserved
	INT32	nReserve3;							// Reserved
	INT32	nReserve4;							// Reserved
	INT32	nReserve5;							// Reserved

	CHAR	EJData[EJ_DATA_SIZE];				// E-JNL Data.

} EJSearchBlock, *LPEJSearchBlock;

//////////////////////////////////////
// Data Offset Search.
typedef struct tag_SearchDataOffset
{
	INT32	nJnlSize;
	INT32	nJnlIndex;
	INT32	nStackYear;							// [#508] AIREAT 2009.02.27 : Search by date 정보 추가
	INT32	nStackMonth;						// [#508] AIREAT 2009.02.27 : Search by date 정보 추가
	INT32	nStackDay;							// [#508] AIREAT 2009.02.27 : Search by date 정보 추가
	INT32	nStackHour;							// [#508] AIREAT 2009.02.27 : Search by date 정보 추가
	INT32	nStackMin;							// [#508] AIREAT 2009.02.27 : Search by date 정보 추가
	INT32	nStackSec;							// [#508] AIREAT 2009.02.27 : Search by date 정보 추가
	CHAR	*pData;
} SearchDataOffset, *LPSearchDataOffset;

#pragma pack()

/////////////////////////////////////
// Upload Type
typedef enum
{
	UP_PRINT = 0,
	UP_RMS,
	UP_HOST,
	UP_TYPE_MAX

} UP_TYPE;

//////////////////////////////////////
// E-JNL DB의 관리 Class
class AFX_CLASS_EXPORT CJnlDB
{
public:
	CJnlDB();
	~CJnlDB();

public:	
	/* Initialization Methods */
	BOOL	Initialize();																// JNL DB를 초기화 한다.
	BOOL	Deinitialize();																// JNL DB를 해제 한다.
	BOOL	Clear();																	// 모든 JNL를 삭제한다.

	/* Save Methods */
	BOOL	Write(CString strKindCode, CString strJnlData);								// Journal 데이터를 DB에 저장한다.

	/* Search Methods */
	CString	Search(INT32 nJnlIndex);													// Journal 데이터를 DB에서 검색한다.
	CString MoveFirst();																// 처음 Journal로 이동한다.
	CString MoveLast();																	// 마지막 Journal로 이동한다.
	CString MoveNext(INT32 nSize=1);													// 정해진 크기만큼 떨어진 다음 Journal을 가져온다.
	CString MovePrev(INT32 nSize=1);													// 정해진 크기만큼 떨이진 이전 Journal을 가져온다.

	BOOL	SearchByDate(CTime StartDate, CTime EndDate, CStringArray &Indexs);			// 정해진 기간에 해당하는 모든 저널의 인덱스를 가져온다.		// [#508] AIREAT 2009.02.27 : Search by date Method 추가

	/* Back Up */
	BOOL	BackupJNLData(CString strDestPath);											// 현재 모든 저널 데이터를 주어진 Path에 Copy 한다.
	BOOL	SaveJNLData(CString strFileName, CStringArray &Indexs);						// Indexs에 해당하는 저널을 주어진 파일에 저장한다.				// [#508] AIREAT 2009.02.27 : Save JNL Data Method 추가

	/* Information Methods */
	INT32	GetFirstIndex();															// Journal의 처음 Index를 조회한다.
	INT32	GetLastIndex();																// Journal의 마지막 Index를 조회한다.
	INT32	GetCurrentIndex();															// Current Position의 Journal Index를 조회한다.
	INT32	GetTotalCount();															// Journal의 총 개수를 조회한다.
	INT32	CalcRemainCntToLast(INT32 nBaseIndex);										// 주어진 nBaseIndex에서 Last까지 RemainCount를  계산한다.
	INT32	CalcRemainCntToFirst(INT32 nBaseIndex);										// 주어진 nBaseIndex에서 First까지 RemainCount를  계산한다.


	/* JNL Upload Methods */
	INT32	GetUploadLastIndex(UP_TYPE Type);											// Upload된 마지막 Journal Index를 조회한다.
	INT32	GetUploadRemainCount(UP_TYPE Type);											// Upload할 Journal Count를 조회한다.
	CString UploadMoveNext(UP_TYPE Type);												// Upload할 다음 Journal를 가져온다.

	INT32	SetUploadedIndex(UP_TYPE Type, INT32 nJnlIndex);							// // [#2076] NH KSK 2011.06.29 EJUPLOAD 성공 여부를 위해 함수 추가

	/* Helper Methods */
private:
	BOOL	CheckDBState(INT32 nWriteLength=0);											// JNL DB의 유효성 검사를 수행한다.
	BOOL	IsValidateJnlIndex(INT32 nJnlIndex);										// JNL Index의 유효성을 검사한다.

	INT32	GetFileIndexByJnlIndex(INT32 nJnlIndex);									// JNL Index에 해당하는 File Index를 구한다.
	BOOL	FindFileByFileIndex(INT32 nFileIndex, CString &strFileName);				// File Index에 해당하는 File을 찾는다.
	
	BOOL	SaveEJFileDB(INT32 nJnlIndex);												// EJSaveBlock의 EJ 데이터를 파일로 저장한다.

	INT32	CalcJournalIndex(INT32 nBaseIndex, INT32 nSize);							// BaseIndex +- nSize의 유효한 Journla Index를 계산한다.
	BOOL	QuickSearch(INT32 nJnlIndex, CString &strJnlData);							// Index에 해당하는 Journal을 검색한다.
	BOOL	LoadEJFileDB(INT32 nJnlIndex);												// index에 해당하는 EJ 데이터를 파일 or Save Block으로 부터 로드한다.


	/*	Data.  */
private:
	static EJSearchBlock	m_EJSearchBlock;
	CHAR					m_bufJnlHeader[EJ_HEADER_SIZE+2];
	CHAR					m_bufEJData[EJ_MAX_DATA_SIZE];

	/*	Variable  */
private:
	BOOL				m_bInitOK;									// indicate init success.
	LPEJSaveBlock		m_pEJSaveBlock;								// pointer of EJSaveBlock
	LPEJSearchBlock		m_pEJSearchBlock;							// pointer of EJSearchBlock
	SearchDataOffset	m_arSearchDataOffsets[EJ_LOGGING_MAX];		// E-JNL의 Data Offset -- Search Block

	INT32				m_nCurrentPositionIndex;					// Quick Search의 Current Index						

	CString				m_strJNLRoot;
};

#endif //__NH_JNL_DB_H__