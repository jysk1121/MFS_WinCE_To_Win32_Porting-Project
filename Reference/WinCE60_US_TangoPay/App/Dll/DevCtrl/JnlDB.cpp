#include "Stdafx.h"
#include ".\Dev\JnlDB.h"

#define DB_DEBUG(X,Y)	RETAILMSG(X,Y);
#define ON				1

EJSearchBlock	CJnlDB::m_EJSearchBlock = {0};

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlDB
 FUNCTION NAME: CJnlDB()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : CJnlDB의 생성자.
				1. 변수 초기화.
				1. Memory Mapping.
-------------------------------------------------------------------*/
CJnlDB::CJnlDB()
{
	int	i;

	m_bInitOK = FALSE;
	m_nCurrentPositionIndex = 0;
	m_pEJSaveBlock = NULL;
	m_pEJSearchBlock = NULL;
	memset(m_arSearchDataOffsets, 0, sizeof(m_arSearchDataOffsets));
	memset(m_bufEJData, 0, sizeof(m_bufEJData));

	m_strJNLRoot.Format(_T("%s\\JNL"), JNLBACKUP_SRC_PATH);

	////////////////////////
	// Memory Mapping
	////////////////////////

	// Get Save Block Pointer
	for (i = 0; i < MAPPING_TRY_MAX; i++)
	{
		m_pEJSaveBlock = (LPEJSaveBlock)GetJNLAddr(EJ_SAVE_BLOCK_INDEX);
		if (m_pEJSaveBlock)
			break;
	}

	// Get Search Block Pointer
	m_pEJSearchBlock = (LPEJSearchBlock)&m_EJSearchBlock;

	// Check Memory Block Pointer
	if (!m_pEJSaveBlock ||!m_pEJSearchBlock)
	{
		DB_DEBUG(ON, (_T("[CJnlDB::MappingJNLBlock] memory mapping failed. SaveBlock(0x%X), SearchBlock(0x%X)\n"), 
							m_pEJSaveBlock, m_pEJSearchBlock));
	}

}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlDB
 FUNCTION NAME: ~CJnlDB()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : CJnlDB의 소멸자.
				1. Memroy Unmapping.
-------------------------------------------------------------------*/
CJnlDB::~CJnlDB()
{
	Deinitialize();

	////////////////////////
	// Memory UnMapping
	////////////////////////

	// Free Save Block Pointer
	if (m_pEJSaveBlock)
	{
		FreeJNLAddr(m_pEJSaveBlock);
		m_pEJSaveBlock = NULL;
	}

	// Free Search Block Pointer
	if (m_pEJSearchBlock)
	{
		m_pEJSearchBlock = NULL;
	}

	DB_DEBUG(ON, (_T("CJNLDB Destructor\n")));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlDB
 FUNCTION NAME: CheckDBState()
 RETURN TYPE  : nonzero : successful
				zero : fail
 PARAMETER    : nWriteLength : JNL Write시 남은 저장 공간 check.
 DESCRIPTION  : JNL DB의 유효성을 검사한다.
				이 함수는 Initialize이 성공한 후에 호출되어야 한다.
-------------------------------------------------------------------*/
BOOL CJnlDB::CheckDBState(INT32 nWriteLength)
{
	// check memory mapping.
	if (!m_pEJSaveBlock || !m_pEJSearchBlock)
	{
		DB_DEBUG(ON, (_T("[CJnlDB::CheckDBState] memory mapping failed. SaveBlock(0x%X), SearchBlock(0x%X)\n"), 
							m_pEJSaveBlock, m_pEJSearchBlock));
		return FALSE;
	}

	// check initialize.
	if (!m_bInitOK)
	{
		DB_DEBUG(ON, (_T("[CJnlDB::CheckDBState] fail Initialize\n")));
		return FALSE;
	}

	// check offset pointer of EJSaveBlock
	if ((0 > (m_pEJSaveBlock->nNextDataOffset + nWriteLength)) ||
		(EJ_DATA_SIZE <= (m_pEJSaveBlock->nNextDataOffset + nWriteLength)))
	{
		DB_DEBUG(ON, (_T("[CJnlDB::CheckDBState] SaveBlock NextOffset Wrong or MAX. NextOffset(%d), Write(%d)\n"), 
							m_pEJSaveBlock->nNextDataOffset, nWriteLength));
		return FALSE;
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlDB
 FUNCTION NAME: GetFileIndexByJnlIndex()
 RETURN TYPE  : -1 : 잘못된 연산
 PARAMETER    : nJnlIndex : E-JNL Index
 DESCRIPTION  : E-JNL Index에 해당하는 File Index를 구한다.
				File Index는 다음의 XX 부분이다.
				   File Name : XX_YYYMMDD.dat
-------------------------------------------------------------------*/
int CJnlDB::GetFileIndexByJnlIndex(INT32 nJnlIndex)
{
	INT32	nFileIndex = -1;

	if (EJ_INDEX_START <= nJnlIndex && nJnlIndex <= EJ_INDEX_MAX)
	{
		nFileIndex = (int)(nJnlIndex / EJ_LOGGING_MAX);
	}

	return nFileIndex;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlDB
 FUNCTION NAME: SaveEJFileDB()
 RETURN TYPE  : nonzero : successful
				zero : fail
 PARAMETER    : nJnlIndex - 마지막 저장된 JNL INDEX
 DESCRIPTION  : EJSaveBlock의 EJ 데이터를 파일로 저장한다.
-------------------------------------------------------------------*/
BOOL CJnlDB::SaveEJFileDB(INT32 nJnlIndex)
{
	FILE	*Stream;
	INT32	nFileIndex = -1;
	DWORD	dwFileWriten = 0;
	CString strSaveFileName;
	CTime CurTime = CTime::GetCurrentTime();

	// Get file index to Save 
	nFileIndex = GetFileIndexByJnlIndex(nJnlIndex);
	if (nFileIndex == -1)
	{
		DB_DEBUG(ON, (_T("[CJnlDB::SaveEJFileDB] Failed. Get FileIndex by JnlIndex(%d)\n"), nJnlIndex));
		return FALSE;
	}

	// delete.
	if (FindFileByFileIndex(nFileIndex, strSaveFileName))
		DeleteFile(strSaveFileName);

	////////////////////////////////////
	// File Write.
	{
		// Get file full name to save
		strSaveFileName.Format(_T("%s\\JNL%2.2d_%4.4d%2.2d%2.2d.dat"), (LPCTSTR)m_strJNLRoot, nFileIndex, 
																   CurTime.GetYear(), CurTime.GetMonth(), 
																   CurTime.GetDay());

		// open file
// 		if ( (Stream = _tfopen(strSaveFileName, _T("w+"))) == NULL)
		if ( (Stream = _tfopen(strSaveFileName, _T("wb"))) == NULL)		// [#2179] CA KMK 2013.02.06 개행문자(\n) file write시 저널 Length 불일치 문제 수정, 0d0a(2byte) -> 0a(1byte)
		{
			DB_DEBUG(ON, (_T("[CJnlDB::SaveEJFileDB] Failed. Create File to Save JNL (%s)\n"), strSaveFileName));
			return FALSE;
		}

		dwFileWriten = fwrite (m_pEJSaveBlock->EJData, sizeof(char), m_pEJSaveBlock->nNextDataOffset, Stream);

		fflush(Stream);

		fclose(Stream);

	}

	if ((INT32)dwFileWriten != m_pEJSaveBlock->nNextDataOffset)
	{
		DB_DEBUG(ON, (_T("[CJnlDB::SaveEJFileDB] Failed. Write amount != Request amount, Req(%d) Write(%d)\n"), 
							m_pEJSaveBlock->nNextDataOffset, dwFileWriten));
		return FALSE;
	}
	
	// Clear EJData in EJSaveBlock
	// [#573] NH AIREAT 2009.10.09 SW NVRAM
	//m_pEJSaveBlock->nNextDataOffset = 0;
	//memset(m_pEJSaveBlock->EJData, 0, EJ_DATA_SIZE);
	NvramCopyDword((LPDWORD)&m_pEJSaveBlock->nNextDataOffset, 0);
	NvramSetMemory(m_pEJSaveBlock->EJData, 0, EJ_DATA_SIZE);
	// end of [#573]
	
	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlDB
 FUNCTION NAME: FindFileByFileIndex()
 RETURN TYPE  : nonzero : successful
				zero : fail
 PARAMETER    : nFileIndex : 찾고자 하는 FileIndex.
				strFileName : FileIndex에 해당하는 파일이름.
 DESCRIPTION  : File Index에 해당하는 File을 찾는다.
-------------------------------------------------------------------*/
BOOL CJnlDB::FindFileByFileIndex(INT32 nFileIndex, CString &strFileName)
{
	WIN32_FIND_DATA	FileData;
	HANDLE			hSearch = INVALID_HANDLE_VALUE;		// [#2022] NH KSK 2011.02.22
	CString			strSearchFile;

	strFileName = _T("");

	// make file name for search.
	strSearchFile.Format(_T("%s\\JNL%2.2d_????????.dat"), m_strJNLRoot, nFileIndex);

	hSearch = FindFirstFile(strSearchFile, &FileData);
	if (hSearch == INVALID_HANDLE_VALUE)
	{
		DB_DEBUG(ON, (_T("[CJnlDB::FindFileByFileIndex] Failed Find File. Root(%s) FileIndex(%d) Search(%s)\n"), 
								m_strJNLRoot, nFileIndex, strSearchFile));
		return FALSE;
	}

	strFileName.Format(_T("%s\\%s"), m_strJNLRoot, FileData.cFileName);

	FindClose(hSearch);

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlDB
 FUNCTION NAME: CalcJournalIndex()
 RETURN TYPE  : nonzero : successful
				zero : fail
 PARAMETER    : 
 DESCRIPTION  : (nBaseIndex +- nSize)의 유효한 Journal Index를 계산한다.
-------------------------------------------------------------------*/
INT32 CJnlDB::CalcJournalIndex(INT32 nBaseIndex, INT32 nSize)
{
	INT32	nCalcJnlIndex;
	INT32	nRemainCount, nRequestSize;


	// Get Remain Count
	if (nSize >= 0)
		nRemainCount = CalcRemainCntToLast(nBaseIndex);
	else
		nRemainCount = CalcRemainCntToFirst(nBaseIndex);

	// Get Request Size.
	nRequestSize = nSize;
	if (nRequestSize < 0)
		nRequestSize *= -1;

	// Validate Request Size.
	if (nRequestSize >= nRemainCount)
		nRequestSize = nRemainCount;

	// Adjust Request Size.
	nCalcJnlIndex = nBaseIndex;
	if (nCalcJnlIndex == EJ_INDEX_INIT)
	{
		if (nSize >= 0)
			nCalcJnlIndex = m_pEJSaveBlock->nFirstIndex;
		else
			nCalcJnlIndex = m_pEJSaveBlock->nLastIndex;
	}
	else
	{
		if (nSize >= 0)
			nCalcJnlIndex += nRequestSize;
		else
			nCalcJnlIndex -= nRequestSize;
	}
	
	// 0 -> MAX
	if (m_pEJSaveBlock->nFirstIndex != EJ_INDEX_INIT && m_pEJSaveBlock->nLastIndex != EJ_INDEX_INIT)
	{
		if (nCalcJnlIndex < EJ_INDEX_START)
			nCalcJnlIndex = (EJ_INDEX_MAX) + nCalcJnlIndex;
		
		// MAX -> 0
		if (nCalcJnlIndex > EJ_INDEX_MAX)
			nCalcJnlIndex = nCalcJnlIndex - (EJ_INDEX_MAX);
	}

	return nCalcJnlIndex;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlDB
 FUNCTION NAME: CalcRemainCntToLast()
 RETURN TYPE  : nonzero : successful
				zero : fail
 PARAMETER    : 
 DESCRIPTION  : 주어진 nBaseIndex에서 Last까지 RemainCount를  계산한다. 
				BaseIndex는 제외이다. (Base = 1, Last = 2, Remain = 1)
				만약 BaseIndex가 EJ_INDEX_INIT(-1)와 같으면 총 갯수를 구한다. 
-------------------------------------------------------------------*/
INT32 CJnlDB::CalcRemainCntToLast(INT32 nBaseIndex)
{
	INT32	nRemainCount = 0;
	BOOL	bTotalCount = FALSE;

	// Index is InitState.
	if (nBaseIndex == EJ_INDEX_INIT)
	{
		nBaseIndex = m_pEJSaveBlock->nFirstIndex;
		if (nBaseIndex != EJ_INDEX_INIT)
			bTotalCount = TRUE;
	}

	// Calc Remain Count to LAST.
	if (m_pEJSaveBlock->nFirstIndex > m_pEJSaveBlock->nLastIndex)
	{
		if (nBaseIndex > m_pEJSaveBlock->nLastIndex)
		{
			nRemainCount = EJ_INDEX_MAX - nBaseIndex;
			nBaseIndex = 0;
		}
	}

	nRemainCount += m_pEJSaveBlock->nLastIndex - nBaseIndex;

	if (bTotalCount == TRUE)
		nRemainCount += 1;

	return nRemainCount;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlDB
 FUNCTION NAME: CalcRemainCntToFirst()
 RETURN TYPE  : nonzero : successful
				zero : fail
 PARAMETER    : 
 DESCRIPTION  : 주어진 nBaseIndex에서 First까지 RemainCount를  계산한다
				BaseIndex는 제외이다. (Base = 2, First = 1, Remain = 1)
				만약 BaseIndex가 EJ_INDEX_INIT(-1)와 같으면 총 갯수를 구한다. 
-------------------------------------------------------------------*/
INT32 CJnlDB::CalcRemainCntToFirst(INT32 nBaseIndex)
{
	INT32	nRemainCount = 0;
	BOOL	bTotalCount = FALSE;

	// Index is InitState.
	if (nBaseIndex == EJ_INDEX_INIT)
	{
		nBaseIndex = m_pEJSaveBlock->nLastIndex;
		if (nBaseIndex != EJ_INDEX_INIT)
			bTotalCount = TRUE;
	}

	// Calc Remain Count.
	if (m_pEJSaveBlock->nFirstIndex > m_pEJSaveBlock->nLastIndex)
	{
		if (nBaseIndex < m_pEJSaveBlock->nFirstIndex)
		{
			nRemainCount = nBaseIndex;
			nBaseIndex = EJ_INDEX_MAX;
		}
	}

	nRemainCount += nBaseIndex - m_pEJSaveBlock->nFirstIndex;

	if (bTotalCount == TRUE)
		nRemainCount += 1;

	return nRemainCount;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlDB
 FUNCTION NAME: IsValidateJnlIndex()
 RETURN TYPE  : nonzero : successful
				zero : fail
 PARAMETER    : 검사하고자 하는 Journal Index
 DESCRIPTION  : Journal Index의 유효성을 검사한다..
-------------------------------------------------------------------*/
BOOL CJnlDB::IsValidateJnlIndex(INT32 nJnlIndex)
{
	if (m_pEJSaveBlock->nFirstIndex > m_pEJSaveBlock->nLastIndex)
	{
		if ((m_pEJSaveBlock->nFirstIndex <= nJnlIndex && nJnlIndex <= EJ_INDEX_MAX) ||
			(EJ_INDEX_START <= nJnlIndex && nJnlIndex <= m_pEJSaveBlock->nLastIndex))
		{
			return TRUE;
		}
	}
	else
	{
		if (m_pEJSaveBlock->nFirstIndex <= nJnlIndex && nJnlIndex <= m_pEJSaveBlock->nLastIndex)
			return TRUE;
	}

	DB_DEBUG(ON, (_T("[CJnlDB::IsValidateJnlIndex] Failed. 1. First(%d), Last(%d), Req(%d)\n"),
						m_pEJSaveBlock->nFirstIndex, m_pEJSaveBlock->nLastIndex, nJnlIndex));
	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlDB
 FUNCTION NAME: QuickSearch()
 RETURN TYPE  : nonzero : successful
				zero : fail
 PARAMETER    : 
 DESCRIPTION  : nJnlIndex에 해당하는 저널을 찾는다..
-------------------------------------------------------------------*/
BOOL CJnlDB::QuickSearch(INT32 nJnlIndex, CString &strJnlData)
{
	INT32 nOffsetIndex;

	strJnlData = _T("");

	// Load JNL DATA
	if (!LoadEJFileDB(nJnlIndex))
	{
		DB_DEBUG(ON, (_T("[CJnlDB::QuickSearch] Failed. LoadEJFileDB(), Index(%d)\n"), nJnlIndex));
		return FALSE;
	}

	nOffsetIndex = nJnlIndex % EJ_LOGGING_MAX;

	if (m_arSearchDataOffsets[nOffsetIndex].pData == NULL)
	{
		DB_DEBUG(ON, (_T("[CJnlDB::QuickSearch] Failed. pData is NULL in Offset array, Index(%d), Index(%d)\n"), nJnlIndex, nOffsetIndex));
		return FALSE;
	}

	// Copy JNL Data
	{
		LPTSTR pStr = strJnlData.GetBuffer(m_arSearchDataOffsets[nOffsetIndex].nJnlSize);
		
		memset((void*)pStr, 0, (m_arSearchDataOffsets[nOffsetIndex].nJnlSize * sizeof(TCHAR)));
		
		MultiByteToWideChar(CP_ACP, 0, (LPCSTR)m_arSearchDataOffsets[nOffsetIndex].pData, m_arSearchDataOffsets[nOffsetIndex].nJnlSize, 
			pStr, ((m_arSearchDataOffsets[nOffsetIndex].nJnlSize -1) * sizeof(TCHAR)));
		
		pStr[m_arSearchDataOffsets[nOffsetIndex].nJnlSize-1] = 0;
		
		strJnlData.ReleaseBuffer();
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlDB
 FUNCTION NAME: LoadEJFileDB()
 RETURN TYPE  : nonzero : successful
				zero : fail
 PARAMETER    : 
 DESCRIPTION  : index에 해당하는 EJ 데이터를 파일 or Save Block으로 부터 로드한다.
-------------------------------------------------------------------*/
BOOL CJnlDB::LoadEJFileDB(INT32 nSearchJnlIndex)
{
	INT32		nFileIndex = -1, nSaveBlockIndex = -1;

	// Get FileIndex of nSearchJnl
	nFileIndex = GetFileIndexByJnlIndex(nSearchJnlIndex);
	if (nFileIndex == -1)
	{
		DB_DEBUG(ON, (_T("[CJnlDB::LoadEJFileDB] Failed. Get FileIndex by JnlIndex(%d)\n"), nSearchJnlIndex));
		return FALSE;
	}

	// Get FileIndex of EJSaveBlock 
	nSaveBlockIndex = GetFileIndexByJnlIndex(m_pEJSaveBlock->nLastIndex);

	// same save block index
	if (nFileIndex == nSaveBlockIndex)
	{
		if ((nFileIndex == m_pEJSearchBlock->nLoadedFileIndex) && 
			(m_pEJSearchBlock->nEndIndex == m_pEJSaveBlock->nLastIndex))
		{
			return TRUE;
		}
	}
	// Already, Loaded in Search Block.
	else if ((nFileIndex == m_pEJSearchBlock->nLoadedFileIndex) && 
			 ((nFileIndex+1) * EJ_LOGGING_MAX -1 == m_pEJSearchBlock->nEndIndex))
	{
		return TRUE;
	}

	// Clear EJSearchBlock
	m_pEJSearchBlock->nLoadedFileIndex = EJ_INDEX_INIT;
	m_pEJSearchBlock->nStartIndex = EJ_INDEX_INIT;
	m_pEJSearchBlock->nEndIndex = EJ_INDEX_INIT;
	memset(m_pEJSearchBlock->EJData, 0, EJ_DATA_SIZE);
	memset(m_arSearchDataOffsets, 0, sizeof(m_arSearchDataOffsets));

	//////////////////////////////////////
	// Load EJ Data From EJSaveBlock
	if (nFileIndex == nSaveBlockIndex)
	{
		memcpy(m_pEJSearchBlock->EJData, m_pEJSaveBlock->EJData, m_pEJSaveBlock->nNextDataOffset);
		
		m_pEJSearchBlock->nLoadedFileIndex = nFileIndex;

		m_pEJSearchBlock->nStartIndex = nFileIndex * EJ_LOGGING_MAX;
		if (nFileIndex == 0)
			m_pEJSearchBlock->nStartIndex += 1;
		m_pEJSearchBlock->nEndIndex = m_pEJSaveBlock->nLastIndex;
	}
	// Load EJ Data From Saved file
	else
	{
		DWORD	nReadByte;
		HANDLE	hLoadFile = INVALID_HANDLE_VALUE;	// [#2022] NH KSK 2011.02.22
		CString	strLoadFileName;

		if (!FindFileByFileIndex(nFileIndex, strLoadFileName))
			return FALSE;

		hLoadFile = CreateFile (strLoadFileName,		// Open XX_YYYYMMDD.dat.
								GENERIC_READ,           // Open for writing
								0,                      // Do not share
								NULL,                   // No security
								OPEN_EXISTING,          // Open or create
								FILE_ATTRIBUTE_NORMAL,  // Normal file
								NULL);                  // No template file
		if (hLoadFile == INVALID_HANDLE_VALUE)
		{
			DB_DEBUG(ON, (_T("[CJnlDB::LoadEJFileDB] Failed. Open File to Search JNL (%s)\n"), strLoadFileName));
			return FALSE;
		}

		nReadByte = 0;
		ReadFile(hLoadFile, m_pEJSearchBlock->EJData, EJ_DATA_SIZE, &nReadByte, NULL);

		CloseHandle(hLoadFile);

		if (nReadByte == 0)
		{
			DB_DEBUG(ON, (_T("[CJnlDB::LoadEJFileDB] Failed. Read Data is 0\n")));
			return FALSE;
		}

		m_pEJSearchBlock->nLoadedFileIndex = nFileIndex;
		
		m_pEJSearchBlock->nStartIndex = nFileIndex * EJ_LOGGING_MAX;
		if (nFileIndex == 0)
			m_pEJSearchBlock->nStartIndex = EJ_INDEX_START;
		m_pEJSearchBlock->nEndIndex = (nFileIndex+1) * EJ_LOGGING_MAX - 1;
	}

	/////////////////////////////////////////
	// E-JNL Validate & Search
	{
		INT32	nLoadingIndex;
		INT32	nHeaderData[9];									// [#508] AIREAT 2009.02.27 : 추출 정보를 배열로 변경
		INT32	nOffsetIndex;

		char	*pEJData = (char*)m_pEJSearchBlock->EJData;
		INT32	offset = 0;
		
		nLoadingIndex = m_pEJSearchBlock->nStartIndex;
		
		while(pEJData - (m_pEJSearchBlock->EJData + EJ_HEADER_SIZE) < EJ_DATA_SIZE)
		{
			// Jump to Next E-JNL
			if (pEJData != m_pEJSearchBlock->EJData)
			{
				if (pEJData[0] != 'X')
					break;
				
				if (pEJData[1] != EJ_JNL_DELIMITER)
					break;

				pEJData += 2;
			}

			memset(nHeaderData, 0, sizeof(nHeaderData));

			// copy Header to buffer
			{
				memset(m_bufJnlHeader, 0, EJ_HEADER_SIZE+2);
				memcpy(m_bufJnlHeader, pEJData, EJ_HEADER_SIZE);
			}

			// Get Header Data
		
			char	*token;
			char	*nextToken;
			int		index = 0;

			token = strtok_s(m_bufJnlHeader, "^", &nextToken);
			while(token != NULL)
			{
				nHeaderData[index] = Asc2Int(CString(token));
				if (index > 8)
					break;

				// Get Next token
				token = strtok_s(NULL, "^", &nextToken);
				index++;
			}
			
			// check Validate. size, index
			if (nLoadingIndex != nHeaderData[2] || nHeaderData[0] <= 0 || nHeaderData[0] > 1000)
				break;

			// Check Stack Date & Time.
			if (IsValidDateTime(nHeaderData[4], nHeaderData[5], nHeaderData[3],
								nHeaderData[6], nHeaderData[7], nHeaderData[8]) != 0)
			{
				nOffsetIndex = nHeaderData[2] % EJ_LOGGING_MAX;
		
				// Set Data Offset.
				m_arSearchDataOffsets[nOffsetIndex].nJnlSize	= nHeaderData[0];	// JNL Size
				m_arSearchDataOffsets[nOffsetIndex].nJnlIndex	= nHeaderData[2];	// Stack Number
				m_arSearchDataOffsets[nOffsetIndex].nStackYear	= nHeaderData[3];	// Stack Year
				m_arSearchDataOffsets[nOffsetIndex].nStackMonth = nHeaderData[4];	// Stack Month
				m_arSearchDataOffsets[nOffsetIndex].nStackDay	= nHeaderData[5];	// Stack Day
				m_arSearchDataOffsets[nOffsetIndex].nStackHour	= nHeaderData[6];	// Stack Hour
				m_arSearchDataOffsets[nOffsetIndex].nStackMin	= nHeaderData[7];	// Stack Minute
				m_arSearchDataOffsets[nOffsetIndex].nStackSec	= nHeaderData[8];	// Stack Second
				m_arSearchDataOffsets[nOffsetIndex].pData		= pEJData;

				// move to Next JNL.
				pEJData += nHeaderData[0];
				nLoadingIndex++;

				continue;
			}

			break;
		}

		// check Data Validate.
		for (int i = m_pEJSearchBlock->nStartIndex; i <= m_pEJSearchBlock->nEndIndex; i++)
		{
			nOffsetIndex = i % EJ_LOGGING_MAX;

			// 제대로 정보가 수집이 되지 않았으면 FALSE이다.
			if (m_arSearchDataOffsets[nOffsetIndex].pData == 0 || m_arSearchDataOffsets[nOffsetIndex].nJnlSize <= 0)
			{
				m_pEJSearchBlock->nLoadedFileIndex = EJ_INDEX_INIT;

				RETAILMSG(1, (L"\nPLEASE ~~ CALL ME CHA CHA.. 82+ 010-7370-8003\n"));
				::MessageBeep(0xFFFFFFFF);
				::MessageBox(NULL, L"CALL Mr CHA.\n", L"VERIFY FAILED", MB_OK);
				DB_DEBUG(ON, (_T("[CJnlDB::LoadEJFileDB] Failed Varify. Start(%d) Cur(%d) End(%d)\n"), 
								(m_pEJSearchBlock->nStartIndex % EJ_LOGGING_MAX), (i % EJ_LOGGING_MAX), (m_pEJSearchBlock->nEndIndex % EJ_LOGGING_MAX)));
				return FALSE;
			}
		}
	}

	return TRUE;
}



////////////////////////////////////////////////////////////////////////
//
//	PUBLIC INTERFACE IMPLEMENTATION.
//
////////////////////////////////////////////////////////////////////////



/*-------------------------------------------------------------------
 CLASS    NAME: CJnlDB
 FUNCTION NAME: Initialize()
 RETURN TYPE  : nonzero : successful
				zero : fail
 PARAMETER    : 
 DESCRIPTION  : JNL DB를 초기화 한다.
-------------------------------------------------------------------*/
BOOL CJnlDB::Initialize()
{
	// Check Memory Block Pointer
	if (!m_pEJSaveBlock || !m_pEJSearchBlock)
		return FALSE;

	//////////////////////////
	// Convert JNL Logging
	//////////////////////////
	if ((m_pEJSaveBlock->nVersionInfo < JNLDB_VERSION_START) || (m_pEJSaveBlock->nVersionInfo > JNLDB_VERSION))
	{
		Clear();
	}
	// [#462] [NH] KSK 2008.12.08
	else if ((JNLDB_VERSION - m_pEJSaveBlock->nVersionInfo) >= 1)	// 0x56010000인 경우
	{
		// 이전 통합 Version 이전 Journal File이 삭제되지 않는 문제 수정
		// 이전 Journal File Delete (JNLDB_VERSION>=0x56010001)
		CCeFileFind finder;
		CString FullFileName;
		BOOL bWorking = FALSE;
		
		FullFileName.Format(_T("%s\\JNL\\*"), JNLBACKUP_SRC_PATH);
		bWorking = finder.FindFile(FullFileName);
		
		while (bWorking)
		{
			bWorking = finder.FindNextFile();				
			FullFileName  = finder.GetFilePath();
			
			if ( FullFileName.Find(_T("\\JNL\\JNL")) == -1)
				DeleteFile(FullFileName);
		}
		
		finder.Close();

		// 0x56010002인 경우 처리는 추후 여기에서 해야함.(JNLDB_VERSION>=0x56010002)
		
		//m_pEJSaveBlock->nVersionInfo = JNLDB_VERSION;	// update journal version
		NvramCopyDword((LPDWORD)&m_pEJSaveBlock->nVersionInfo, JNLDB_VERSION);			// [#573] NH AIREAT 2009.10.09 SW NVRAM

		NVDump('O', 'A', "02", L"", L"JNLDB UPDATE" );	// Log 추가
	}
	// end of [#462]

	//////////////////////////
	// Reset Search Block
	memset(m_pEJSearchBlock, 0, sizeof(EJSearchBlock));
	m_pEJSearchBlock->nLoadedFileIndex = EJ_INDEX_INIT;
	m_pEJSearchBlock->nStartIndex = EJ_INDEX_INIT;
	m_pEJSearchBlock->nEndIndex = EJ_INDEX_INIT;
	memset(m_arSearchDataOffsets, 0, sizeof(m_arSearchDataOffsets));

	m_nCurrentPositionIndex = m_pEJSaveBlock->nLastIndex;

	m_bInitOK = TRUE;

	// check saved file...
	{
		CString strTempJNL;
		if (!QuickSearch(GetFirstIndex(), strTempJNL))
		{
			// 파일이 삭제 되었다고 가정.
			// NVRAM의 JNL 처음 인덱스로 First Index 수정.
			if (QuickSearch(GetLastIndex(), strTempJNL))
			{
				//m_pEJSaveBlock->nFirstIndex = m_pEJSearchBlock->nStartIndex;
				NvramCopyDword((LPDWORD)&m_pEJSaveBlock->nFirstIndex, m_pEJSearchBlock->nStartIndex);		// [#573] NH AIREAT 2009.10.09 SW NVRAM
			}
			else
			{
				//m_pEJSaveBlock->nFirstIndex = m_pEJSaveBlock->nLastIndex;
				NvramCopyDword((LPDWORD)&m_pEJSaveBlock->nFirstIndex, m_pEJSaveBlock->nLastIndex);			// [#573] NH AIREAT 2009.10.09 SW NVRAM
			}

			// check validate LastPrintIndex
			if (!IsValidateJnlIndex(m_pEJSaveBlock->nLastPrintIndex))
			{
				//m_pEJSaveBlock->nLastPrintIndex = EJ_INDEX_INIT;
				NvramCopyDword((LPDWORD)&m_pEJSaveBlock->nLastPrintIndex, EJ_INDEX_INIT);					// [#573] NH AIREAT 2009.10.09 SW NVRAM
			}
			
			// check validate nLastRMSUploadIndex
			if (!IsValidateJnlIndex(m_pEJSaveBlock->nLastRMSUploadIndex))
			{
				//m_pEJSaveBlock->nLastRMSUploadIndex = EJ_INDEX_INIT;
				NvramCopyDword((LPDWORD)&m_pEJSaveBlock->nLastRMSUploadIndex, EJ_INDEX_INIT);				// [#573] NH AIREAT 2009.10.09 SW NVRAM
			}
			
			// check validate nLastHostUploadIndex
			if (!IsValidateJnlIndex(m_pEJSaveBlock->nLastHostUploadIndex))
			{
				//m_pEJSaveBlock->nLastHostUploadIndex = EJ_INDEX_INIT;
				NvramCopyDword((LPDWORD)&m_pEJSaveBlock->nLastHostUploadIndex, EJ_INDEX_INIT);				// [#573] NH AIREAT 2009.10.09 SW NVRAM
			}
		}

		// 마지막으로 이동.
		QuickSearch(GetLastIndex(), strTempJNL);
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlDB
 FUNCTION NAME: Deinitialize()
 RETURN TYPE  : nonzero : successful
				zero : fail
 PARAMETER    : 
 DESCRIPTION  : JNL DB를 해제한다.
-------------------------------------------------------------------*/
BOOL CJnlDB::Deinitialize()
{
	m_bInitOK = FALSE;

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlDB
 FUNCTION NAME: Clear()
 RETURN TYPE  : nonzero : successful
				zero : fail
 PARAMETER    : 
 DESCRIPTION  : 모든 JNL을 삭제한다.
-------------------------------------------------------------------*/
BOOL CJnlDB::Clear()
{
	//////////////////////////
	// Reset Save Block
	if (m_pEJSaveBlock)
	{
		// [#573] NH AIREAT 2009.10.09 SW NVRAM
		//memset(m_pEJSaveBlock, 0, sizeof(EJSaveBlock));
		//m_pEJSaveBlock->nVersionInfo = JNLDB_VERSION;
		//m_pEJSaveBlock->nFirstIndex = EJ_INDEX_INIT;
		//m_pEJSaveBlock->nLastIndex = EJ_INDEX_INIT;
		//m_pEJSaveBlock->nLastPrintIndex = EJ_INDEX_INIT;
		//m_pEJSaveBlock->nLastRMSUploadIndex = EJ_INDEX_INIT;
		//m_pEJSaveBlock->nLastHostUploadIndex = EJ_INDEX_INIT;
		NvramSetMemory(m_pEJSaveBlock, 0, sizeof(EJSaveBlock));
		NvramCopyDword((LPDWORD)&m_pEJSaveBlock->nVersionInfo, JNLDB_VERSION);
		NvramCopyDword((LPDWORD)&m_pEJSaveBlock->nFirstIndex, EJ_INDEX_INIT);
		NvramCopyDword((LPDWORD)&m_pEJSaveBlock->nLastIndex, EJ_INDEX_INIT);
		NvramCopyDword((LPDWORD)&m_pEJSaveBlock->nLastPrintIndex, EJ_INDEX_INIT);
		NvramCopyDword((LPDWORD)&m_pEJSaveBlock->nLastRMSUploadIndex, EJ_INDEX_INIT);
		NvramCopyDword((LPDWORD)&m_pEJSaveBlock->nLastHostUploadIndex, EJ_INDEX_INIT);
		// end of [#573]

		m_nCurrentPositionIndex = m_pEJSaveBlock->nLastIndex;
	}
	
	//////////////////////////
	// Reset Search Block
	if (m_pEJSearchBlock)
	{
		memset(m_pEJSearchBlock, 0, sizeof(EJSearchBlock));
		m_pEJSearchBlock->nLoadedFileIndex = EJ_INDEX_INIT;
		m_pEJSearchBlock->nStartIndex = EJ_INDEX_INIT;
		m_pEJSearchBlock->nEndIndex = EJ_INDEX_INIT;
		memset(m_arSearchDataOffsets, 0, sizeof(m_arSearchDataOffsets));
	}

	//////////////////////////
	// Delete ALL JNL File

	// [#462] [NH] KSK 2008.12.08
	// 통합 Version 이전에 Write된 File을 삭제하지 못하는 Bug Fix
	// Journal File Clear시 관리되는 JNLXX_xxxxxxxx.dat뿐만 아니라 JNL 폴더안에 있는 모든 File을 삭제하도록 수정
//	INT32 nFileIndex;
//	CString strJnlFileName;

//	for (nFileIndex = 0; nFileIndex <= ((INT32)EJ_INDEX_MAX / EJ_LOGGING_MAX); nFileIndex++)
//	{
//		if (FindFileByFileIndex(nFileIndex, strJnlFileName))
//			DeleteFile(strJnlFileName);
//	}

	CString szDestPath;
	szDestPath.Format(_T("%s\\JNL\\"), JNLBACKUP_SRC_PATH);
	
	DeleteFileInDirectory(szDestPath, L"*.*");
	// end of [#462]

	Initialize();
	
	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlDB
 FUNCTION NAME: Write()
 RETURN TYPE  : nonzero : successful
				zero : fail
 PARAMETER    : strKindCode : JNL의 KindCode
				strJnlData  : JNL의 데이터
 DESCRIPTION  : Journal 데이터를 DB에 저장한다.
-------------------------------------------------------------------*/
BOOL CJnlDB::Write(CString strKindCode, CString strJnlData)
{
	CString strJnlSaveFormat;
	INT32	nJNLNextIndex, nJNLSize;
	CTime	CurTime;

	// check validate
	if (!CheckDBState(strJnlData.GetLength()+EJ_HEADER_SIZE))
	{
		DB_DEBUG(ON, (_T("[CJnlDB::Write] Failed. CheckDBState()\n")));
		return FALSE;
	}

	// chekc Data
	if (strKindCode.GetLength() <= 0 || strJnlData.GetLength() <= 0)
	{
		DB_DEBUG(ON, (_T("[CJnlDB::Write] Failed. Param data is 0. kindcode(%d), jnldata(%d)\n"), 
						strKindCode.GetLength(), strJnlData.GetLength()));
		return FALSE;
	}

	////////////////////////////////
	// Save EJSaveBlock to File
	if (m_pEJSaveBlock->nFirstIndex > 0 && ((m_pEJSaveBlock->nLastIndex + 1) % EJ_LOGGING_MAX) == 0)
	{
		if (!SaveEJFileDB(m_pEJSaveBlock->nLastIndex))
		{
			DB_DEBUG(ON, (_T("[CJnlDB::Write] Failed. SaveEJFileDB()\n")));
			return FALSE;
		}
	}

	////////////////////////////////
	// Make Journal Save Format.
	{
		if (m_pEJSaveBlock->nFirstIndex == EJ_INDEX_INIT && m_pEJSaveBlock->nLastIndex == EJ_INDEX_INIT)
		{
			// Is First Logging
			nJNLNextIndex = EJ_INDEX_START;
		}
		else
		{
			// Get JNLIndex
			nJNLNextIndex = m_pEJSaveBlock->nLastIndex + 1;
			
			// MAX -> 0
			if (nJNLNextIndex > EJ_INDEX_MAX)
				nJNLNextIndex = nJNLNextIndex - (EJ_INDEX_MAX);
		}

		// Get JNL Size. -- Header + Data + Delimiter
		nJNLSize = EJ_HEADER_SIZE + strJnlData.GetLength() + 1;
		
		// Get Current Time.
		CurTime = CTime::GetCurrentTime();

		// make Jnl Format.
		strJnlSaveFormat.Format(_T("%4.4d^%3.3s^%8.4d^%4.4d^%2.2d^%2.2d^%2.2d^%2.2d^%2.2d^%s^X%C"),
								nJNLSize, (LPCTSTR)strKindCode, nJNLNextIndex,
								CurTime.GetYear(), CurTime.GetMonth(), CurTime.GetDay(),
								CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond(),
								(LPCTSTR)strJnlData, EJ_JNL_DELIMITER);
	}

	////////////////////////////////
	// Save EJ Data to EJSaveBlock.
	{
		/*
		if (WideCharToMultiByte(CP_ACP, 0, (LPCTSTR)strJnlSaveFormat, -1, 
										(char*)&m_pEJSaveBlock->EJData[m_pEJSaveBlock->nNextDataOffset],
										EJ_DATA_SIZE - m_pEJSaveBlock->nNextDataOffset,
										NULL, NULL) == 0)
		{
			DB_DEBUG(ON, (_T("[CJnlDB::Write] Failed. WideCharToMultiByte() Data(%s), NextOffset(%d)\n"), 
				strJnlSaveFormat, m_pEJSaveBlock->nNextDataOffset));
			return FALSE;
		}
		*/

		int		nWriteByte = 0;

		nWriteByte = WideCharToMultiByte(CP_ACP, 0, (LPCTSTR)strJnlSaveFormat, -1, 
										(LPSTR)&m_bufEJData[0], EJ_MAX_DATA_SIZE,
										NULL, NULL);
		
		nWriteByte -= 1;	// Remove null char
		// 2012.07.31 KMK CodeSonar 대책
// 		if (nWriteByte == 0)
		if (nWriteByte <= 0)
		{
			DB_DEBUG(ON, (_T("[CJnlDB::Write] Failed. WideCharToMultiByte() Data(%s), NextOffset(%d), Err(%d)\n"), 
				strJnlSaveFormat, m_pEJSaveBlock->nNextDataOffset, GetLastError()));
			return FALSE;
		}
		
		NvramCopyMemory((LPVOID)&m_pEJSaveBlock->EJData[m_pEJSaveBlock->nNextDataOffset], (LPVOID)&m_bufEJData[0], nWriteByte);
		
		//m_pEJSaveBlock->nNextDataOffset += strJnlSaveFormat.GetLength();
		//NvramCopyDword((LPDWORD)&m_pEJSaveBlock->nNextDataOffset, (m_pEJSaveBlock->nNextDataOffset + strJnlSaveFormat.GetLength()));		// [#573] NH AIREAT 2009.10.09 SW NVRAM
		NvramCopyDword((LPDWORD)&m_pEJSaveBlock->nNextDataOffset, (m_pEJSaveBlock->nNextDataOffset + nWriteByte));		// [#573] NH AIREAT 2009.10.09 SW NVRAM
	}

	////////////////////////////////
	// Prepare for Next logging.
	{
		// Is First Logging
		if (m_pEJSaveBlock->nFirstIndex == EJ_INDEX_INIT && m_pEJSaveBlock->nLastIndex == EJ_INDEX_INIT)
		{
			//m_pEJSaveBlock->nFirstIndex = EJ_INDEX_START;
			NvramCopyDword((LPDWORD)&m_pEJSaveBlock->nFirstIndex, EJ_INDEX_START);			// [#573] NH AIREAT 2009.10.09 SW NVRAM
		}
		// Move First Index to Next Block
		else if (nJNLNextIndex == m_pEJSaveBlock->nFirstIndex)
		{
			// [#573] NH AIREAT 2009.10.09 SW NVRAM
			//m_pEJSaveBlock->nFirstIndex += EJ_LOGGING_MAX;
			//m_pEJSaveBlock->nFirstIndex = GetFileIndexByJnlIndex(m_pEJSaveBlock->nFirstIndex) * EJ_LOGGING_MAX;
			int	nSaveBlockFirstIndex = m_pEJSaveBlock->nFirstIndex + EJ_LOGGING_MAX;
			int nSaveBlockNewFirstIndex = GetFileIndexByJnlIndex(nSaveBlockFirstIndex) * EJ_LOGGING_MAX;
			NvramCopyDword((LPDWORD)&m_pEJSaveBlock->nFirstIndex, nSaveBlockNewFirstIndex);
			// end of [#573]
			
			if (m_pEJSaveBlock->nFirstIndex <= 0)
			{
				//m_pEJSaveBlock->nFirstIndex = EJ_INDEX_START;
				NvramCopyDword((LPDWORD)&m_pEJSaveBlock->nFirstIndex, EJ_INDEX_START);				// [#573] NH AIREAT 2009.10.09 SW NVRAM
			}
			
			// check validate LastPrintIndex
			if (!IsValidateJnlIndex(m_pEJSaveBlock->nLastPrintIndex))
			{
				//m_pEJSaveBlock->nLastPrintIndex = EJ_INDEX_INIT;
				NvramCopyDword((LPDWORD)&m_pEJSaveBlock->nLastPrintIndex, EJ_INDEX_INIT);			// [#573] NH AIREAT 2009.10.09 SW NVRAM
			}
			
			// check validate nLastRMSUploadIndex
			if (!IsValidateJnlIndex(m_pEJSaveBlock->nLastRMSUploadIndex))
			{
				//m_pEJSaveBlock->nLastRMSUploadIndex = EJ_INDEX_INIT;
				NvramCopyDword((LPDWORD)&m_pEJSaveBlock->nLastRMSUploadIndex, EJ_INDEX_INIT);		// [#573] NH AIREAT 2009.10.09 SW NVRAM
			}
			
			// check validate nLastHostUploadIndex
			if (!IsValidateJnlIndex(m_pEJSaveBlock->nLastHostUploadIndex))
			{
				//m_pEJSaveBlock->nLastHostUploadIndex = EJ_INDEX_INIT;
				NvramCopyDword((LPDWORD)&m_pEJSaveBlock->nLastHostUploadIndex, EJ_INDEX_INIT);		// [#573] NH AIREAT 2009.10.09 SW NVRAM
			}

			//m_pEJSaveBlock->nLastIndex = nJNLNextIndex;
			NvramCopyDword((LPDWORD)&m_pEJSaveBlock->nLastIndex, nJNLNextIndex);					// [#573] NH AIREAT 2009.10.09 SW NVRAM
		}

		//m_pEJSaveBlock->nLastIndex = nJNLNextIndex;
		NvramCopyDword((LPDWORD)&m_pEJSaveBlock->nLastIndex, nJNLNextIndex);						// [#573] NH AIREAT 2009.10.09 SW NVRAM
		m_nCurrentPositionIndex = m_pEJSaveBlock->nLastIndex;
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlDB
 FUNCTION NAME: Search()
 RETURN TYPE  : Journal 데이터.
				Length() > 0 : successful
				         <=  : fail
 PARAMETER    : nJnlIndex : 검색하고자 하는 JNL Index
 DESCRIPTION  : Journal 데이터를 DB에서 검색한다.
-------------------------------------------------------------------*/
CString CJnlDB::Search(INT32 nJnlIndex)
{
	CString strJnlData = _T("");

	// Validate JNL DB
	if (!CheckDBState())
	{
		DB_DEBUG(ON, (_T("[CJnlDB::Search] Failed. CheckDBState()\n")));
		return strJnlData;
	}

	// Validate JNLIndex
	if (!IsValidateJnlIndex(nJnlIndex))
	{
		DB_DEBUG(ON, (_T("[CJnlDB::Search] Failed. IsValidateJnlIndex(), Index(%d)\n"), nJnlIndex));
		return strJnlData;
	}

	// Search Journal
	if (!QuickSearch(nJnlIndex, strJnlData))
	{
		DB_DEBUG(ON, (_T("[CJnlDB::Search] Failed. QuickSearch\n")));
		return strJnlData;
	}

	// update current position
	m_nCurrentPositionIndex = nJnlIndex;


	return strJnlData;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlDB
 FUNCTION NAME: MoveFirst()
 RETURN TYPE  : Journal 데이터.
				Length() > 0 : successful
				         <=  : fail
 PARAMETER    : 
 DESCRIPTION  : 처음 Journal로 이동한다.
-------------------------------------------------------------------*/
CString CJnlDB::MoveFirst()
{
	CString strJnlData = _T("");
	INT32	nSearchIndex;

	// Validate JNL DB
	if (!CheckDBState())
	{
		DB_DEBUG(ON, (_T("[CJnlDB::MoveFirst] Failed. CheckDBState()\n")));
		return strJnlData;
	}

	// Get FirstIndex
	nSearchIndex = GetFirstIndex();

	// Search Journal
	if (!QuickSearch(nSearchIndex, strJnlData))
	{
		DB_DEBUG(ON, (_T("[CJnlDB::MoveFirst] Failed. Search\n")));
		return strJnlData;
	}

	// update current position
	m_nCurrentPositionIndex = nSearchIndex;

	return strJnlData;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlDB
 FUNCTION NAME: MoveLast()
 RETURN TYPE  : Journal 데이터.
				Length() > 0 : successful
				         <=  : fail
 PARAMETER    : 
 DESCRIPTION  : 마지막 Journal로 이동한다.
-------------------------------------------------------------------*/
CString CJnlDB::MoveLast()
{
	CString strJnlData = _T("");
	INT32	nSearchIndex;

	// Validate JNL DB
	if (!CheckDBState())
	{
		DB_DEBUG(ON, (_T("[CJnlDB::MoveLast] Failed. CheckDBState()\n")));
		return strJnlData;
	}

	// Get LastIndex
	nSearchIndex = GetLastIndex();

	// Search Journal
	if (!QuickSearch(nSearchIndex, strJnlData))
	{
		DB_DEBUG(ON, (_T("[CJnlDB::MoveLast] Failed. Search\n")));
		return strJnlData;
	}

	// update current position
	m_nCurrentPositionIndex = nSearchIndex;

	return strJnlData;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlDB
 FUNCTION NAME: MoveNext()
 RETURN TYPE  : Journal 데이터.
				Length() > 0 : successful
				         <=  : fail
 PARAMETER    : nSize : Index의 증가 크기
 DESCRIPTION  : 정해진 크기만큼 떨어진 다음 Journal을 가져온다.
-------------------------------------------------------------------*/
CString CJnlDB::MoveNext(INT32 nSize/* =1 */)
{
	CString strJnlData = _T("");
	INT32	nSearchIndex;

	// Validate JNL DB
	if (!CheckDBState())
	{
		DB_DEBUG(ON, (_T("[CJnlDB::MoveNext] Failed. CheckDBState()\n")));
		return strJnlData;
	}

	// is Last
	if (m_nCurrentPositionIndex == m_pEJSaveBlock->nLastIndex)
	{
		DB_DEBUG(ON, (_T("[CJnlDB::MoveNext] Failed. Already current is Last.\n")));
		return strJnlData;
	}

	// Validate nSize
	if (nSize <= 0 || nSize > EJ_INDEX_MAX)
	{
		DB_DEBUG(ON, (_T("[CJnlDB::MoveNext] Failed. Wrong Param (%d).\n"), nSize));
		return strJnlData;
	}

	// Calc Search Index.
	nSearchIndex = CalcJournalIndex(m_nCurrentPositionIndex, nSize);

	// Search Journal
	if (!QuickSearch(nSearchIndex, strJnlData))
	{
		DB_DEBUG(ON, (_T("[CJnlDB::MoveNext] Failed. Search\n")));
		return strJnlData;
	}

	// update current position
	m_nCurrentPositionIndex = nSearchIndex;

	return strJnlData;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlDB
 FUNCTION NAME: MovePrev()
 RETURN TYPE  : Journal 데이터.
				Length() > 0 : successful
				         <=  : fail
 PARAMETER    : nSize : Index 감소 크기
 DESCRIPTION  : 정해진 크기만큼 떨어진 이전 Journal을 가져온다.
-------------------------------------------------------------------*/
CString CJnlDB::MovePrev(INT32 nSize/* =1 */)
{
	CString strJnlData = _T("");
	INT32	nSearchIndex;

	// Validate JNL DB
	if (!CheckDBState())
	{
		DB_DEBUG(ON, (_T("[CJnlDB::MovePrev] Failed. CheckDBState()\n")));
		return strJnlData;
	}

	// is First
	if (m_nCurrentPositionIndex == m_pEJSaveBlock->nFirstIndex)
	{
		DB_DEBUG(ON, (_T("[CJnlDB::MovePrev] Failed. Already current is First.\n")));
		return strJnlData;
	}

	// Validate nSize
	if (nSize <= 0 || nSize > EJ_INDEX_MAX)
	{
		DB_DEBUG(ON, (_T("[CJnlDB::MovePrev] Failed. Wrong Param (%d).\n"), nSize));
		return strJnlData;
	}

	// Calc Search Index.
	nSearchIndex = CalcJournalIndex(m_nCurrentPositionIndex, (nSize * -1));

	// Search Journal
	if (!QuickSearch(nSearchIndex, strJnlData))
	{
		DB_DEBUG(ON, (_T("[CJnlDB::MovePrev] Failed. Search\n")));
		return strJnlData;
	}

	// update current position
	m_nCurrentPositionIndex = nSearchIndex;

	return strJnlData;
}

// [#508] AIREAT 2009.02.27 : E-JNL Search by date Method 구현
/*-------------------------------------------------------------------
 CLASS    NAME: CJnlDB
 FUNCTION NAME: SearchByDate()
 RETURN TYPE  : if false, failed, otherwise successful.
 PARAMETER    : StartDate : 조회 Start Date
				EndDate   : 조회 End Date
				Indexs	  : 검색된 Index가 저장된다.
 DESCRIPTION  : 정해진 기간에 해당하는 모든 저널의 인덱스를 가져온다.
-------------------------------------------------------------------*/
BOOL CJnlDB::SearchByDate(CTime StartDate, CTime EndDate, CStringArray &Indexs)
{
	INT32	nCurIndex, nOffset;
	CString	strTemp;

	Indexs.RemoveAll();

	// Validate JNL DB
	if (!CheckDBState())
	{
		DB_DEBUG(ON, (_T("[CJnlDB::SearchByDate] Failed. CheckDBState()\n")));
		return FALSE;
	}

	nCurIndex = m_pEJSaveBlock->nFirstIndex;

	while (nCurIndex != m_pEJSaveBlock->nLastIndex && nCurIndex >= EJ_INDEX_START)
	{
		// Load JNL DATA to Search Block
		// if search fail, skip.
		if (LoadEJFileDB(nCurIndex) == TRUE)
		{
			// Search Data
			for (INT32 nIndex = m_pEJSearchBlock->nStartIndex; nIndex <= m_pEJSearchBlock->nEndIndex ; nIndex++)
			{
				nOffset = nIndex % EJ_LOGGING_MAX;
				
				// Get Stack date
				CTime StackDate(m_arSearchDataOffsets[nOffset].nStackYear, m_arSearchDataOffsets[nOffset].nStackMonth, 
								m_arSearchDataOffsets[nOffset].nStackDay,  m_arSearchDataOffsets[nOffset].nStackHour,
								m_arSearchDataOffsets[nOffset].nStackMin,  m_arSearchDataOffsets[nOffset].nStackSec);
				
				if (StartDate <= StackDate && StackDate <= EndDate)
				{
					strTemp.Format(_T("%d"), m_arSearchDataOffsets[nOffset].nJnlIndex);
					Indexs.Add(strTemp);
				}
			}
		}

		nCurIndex = CalcJournalIndex(nCurIndex, EJ_LOGGING_MAX);
	}

	if (Indexs.GetSize() <= 0)
		return FALSE;

	return TRUE;
}
// end of [#508]

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlDB
 FUNCTION NAME: GetFirstIndex()
 RETURN TYPE  : Journal Index
					>= 0 : successful
				    <  0 : fail
 PARAMETER    : 
 DESCRIPTION  : Journal의 처음 Index를 조회한다.
-------------------------------------------------------------------*/
INT32 CJnlDB::GetFirstIndex()
{
	// check validate
	if (!CheckDBState())
	{
		DB_DEBUG(ON, (_T("[CJnlDB::GetFirstIndex] Failed. CheckDBState()\n")));
		return -1;
	}

	return m_pEJSaveBlock->nFirstIndex;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlDB
 FUNCTION NAME: GetLastIndex()
 RETURN TYPE  : Journal Index
					>= 0 : successful
				    <  0 : fail
 PARAMETER    : 
 DESCRIPTION  : Journal의 마지막 Index를 조회한다.
-------------------------------------------------------------------*/
INT32 CJnlDB::GetLastIndex()
{
	// check validate
	if (!CheckDBState())
	{
		DB_DEBUG(ON, (_T("[CJnlDB::GetLastIndex] Failed. CheckDBState()\n")));
		return -1;
	}

	return m_pEJSaveBlock->nLastIndex;;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlDB
 FUNCTION NAME: GetCurrentIndex()
 RETURN TYPE  : Journal Index
					>= 0 : successful
				    <  0 : fail
 PARAMETER    : 
 DESCRIPTION  : Current Position의 Journal Index를 조회한다.
-------------------------------------------------------------------*/
INT32 CJnlDB::GetCurrentIndex()
{
	// check validate
	if (!CheckDBState())
	{
		DB_DEBUG(ON, (_T("[CJnlDB::GetCurrentIndex] Failed. CheckDBState()\n")));
		return -1;
	}

	return m_nCurrentPositionIndex;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlDB
 FUNCTION NAME: GetTotalCount()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Journal의 총 개수를 조회한다..
-------------------------------------------------------------------*/
INT32 CJnlDB::GetTotalCount()
{
	// check validate
	if (!CheckDBState())
	{
		DB_DEBUG(ON, (_T("[CJnlDB::GetCurrentIndex] Failed. CheckDBState()\n")));
		return -1;
	}
	
	return CalcRemainCntToLast(EJ_INDEX_INIT);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlDB
 FUNCTION NAME: GetUploadLastIndex()
 RETURN TYPE  : nonzero : successful
				zero : fail
 PARAMETER    : Upload 항목 (UP_PRINT, UP_RMS, UP_HOST)
 DESCRIPTION  : Upload된 마지막 Journal Index를 조회한다.
-------------------------------------------------------------------*/
INT32 CJnlDB::GetUploadLastIndex(UP_TYPE Type)
{
	// check validate
	if (!CheckDBState())
	{
		DB_DEBUG(ON, (_T("[CJnlDB::GetUploadLastIndex] Failed. CheckDBState()\n")));
		return -2;
	}

	// Get Last Index
	if (Type == UP_PRINT)
		return m_pEJSaveBlock->nLastPrintIndex;
	else if (Type == UP_RMS)
		return m_pEJSaveBlock->nLastRMSUploadIndex;
	else if (Type == UP_HOST)
		return m_pEJSaveBlock->nLastHostUploadIndex;

	DB_DEBUG(ON, (_T("[CJnlDB::GetUploadLastIndex] Failed. Wrong Type(%d)\n"), Type));

	return -2;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlDB
 FUNCTION NAME: GetUploadRemainCount()
 RETURN TYPE  : nonzero : successful
				zero : fail
 PARAMETER    : Upload 항목 (UP_PRINT, UP_RMS, UP_HOST)
 DESCRIPTION  : Upload할 Journal Count를 조회한다.
-------------------------------------------------------------------*/
INT32 CJnlDB::GetUploadRemainCount(UP_TYPE Type)
{
	INT32	nSearchIndex;
	INT32	nRemainCount;

	// check validate
	if (!CheckDBState())
	{
		DB_DEBUG(ON, (_T("[CJnlDB::GetUploadRemainCount] Failed. CheckDBState()\n")));
		return -2;
	}

	// Get Last Index
	if (Type == UP_PRINT)
		nSearchIndex = m_pEJSaveBlock->nLastPrintIndex;
	else if (Type == UP_RMS)
		nSearchIndex = m_pEJSaveBlock->nLastRMSUploadIndex;
	else if (Type == UP_HOST)
		nSearchIndex = m_pEJSaveBlock->nLastHostUploadIndex;
	else
	{
		DB_DEBUG(ON, (_T("[CJnlDB::GetUploadRemainCount] Failed. Wrong Type(%d)\n"), Type));
		return -2;
	}

	// Calc Remain Count.
	nRemainCount = CalcRemainCntToLast(nSearchIndex);

	return nRemainCount;
}

// [#2076] NH KSK 2011.06.29
/*-------------------------------------------------------------------
CLASS    NAME: CJnlDB
FUNCTION NAME: SetUploadedIndex()
RETURN TYPE  : nonzero : successful
zero : fail
PARAMETER    : Upload 항목 (UP_PRINT, UP_RMS, UP_HOST), 설정할 Index (nJnlIndex)
DESCRIPTION  : Upload가 성공한 Index를 설정함
-------------------------------------------------------------------*/
INT32 CJnlDB::SetUploadedIndex(UP_TYPE Type, INT32 nJnlIndex)
{
	if (Type == UP_PRINT)
		m_pEJSaveBlock->nLastPrintIndex = nJnlIndex;
	else if (Type == UP_RMS)
		m_pEJSaveBlock->nLastRMSUploadIndex = nJnlIndex;
	else if (Type == UP_HOST)
		m_pEJSaveBlock->nLastHostUploadIndex = nJnlIndex;

	return 1;
}
// end of [#2076]

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlDB
 FUNCTION NAME: UploadMoveNext()
 RETURN TYPE  : nonzero : successful
				zero : fail
 PARAMETER    : Upload 항목 (UP_PRINT, UP_RMS, UP_HOST)
 DESCRIPTION  : Upload할 다음 Journal를 가져온다.
-------------------------------------------------------------------*/
CString CJnlDB::UploadMoveNext(UP_TYPE Type)
{
	INT32		nSearchIndex;
	CString		strJnlData;

	// check validate
	if (!CheckDBState())
	{
		DB_DEBUG(ON, (_T("[CJnlDB::UploadMoveNext] Failed. CheckDBState()\n")));
		return strJnlData;
	}

	// Get Last Index
	if (Type == UP_PRINT)
		nSearchIndex = m_pEJSaveBlock->nLastPrintIndex;
	else if (Type == UP_RMS)
		nSearchIndex = m_pEJSaveBlock->nLastRMSUploadIndex;
	else if (Type == UP_HOST)
		nSearchIndex = m_pEJSaveBlock->nLastHostUploadIndex;
	else
	{
		DB_DEBUG(ON, (_T("[CJnlDB::UploadMoveNext] Failed. Wrong Type(%d)\n"), Type));
		return strJnlData;
	}

	// Is Last..
	if (nSearchIndex == m_pEJSaveBlock->nLastIndex)
	{
		DB_DEBUG(ON, (_T("[CJnlDB::UploadMoveNext] Failed. no Upload Data. Type(%d).\n"), Type));
		return strJnlData;
	}

	// Calc Search Index.
	nSearchIndex = CalcJournalIndex(nSearchIndex, 1);

	// Search Journal
	if (!QuickSearch(nSearchIndex, strJnlData))
	{
		DB_DEBUG(ON, (_T("[CJnlDB::UploadMoveNext] Failed. Search. Type(%d).\n"), Type));
		return strJnlData;
	}

	// Update Last Index
	if (Type == UP_PRINT)
	{
		//m_pEJSaveBlock->nLastPrintIndex = nSearchIndex;
		NvramCopyDword((LPDWORD)&m_pEJSaveBlock->nLastPrintIndex, nSearchIndex);				// [#573] NH AIREAT 2009.10.09 SW NVRAM
	}
	else if (Type == UP_RMS)
	{
		//m_pEJSaveBlock->nLastRMSUploadIndex = nSearchIndex;
		NvramCopyDword((LPDWORD)&m_pEJSaveBlock->nLastRMSUploadIndex, nSearchIndex);			// [#573] NH AIREAT 2009.10.09 SW NVRAM
	}
	else if (Type == UP_HOST)
	{
		//m_pEJSaveBlock->nLastHostUploadIndex = nSearchIndex;
		NvramCopyDword((LPDWORD)&m_pEJSaveBlock->nLastHostUploadIndex, nSearchIndex);			// [#573] NH AIREAT 2009.10.09 SW NVRAM
	}

	return strJnlData;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CJnlDB
 FUNCTION NAME: BackupJNLData()
 RETURN TYPE  : nonzero : successful
				zero : fail
 PARAMETER    : strDestPath : 백업 경로 폴더.
 DESCRIPTION  : 현재 모든 저널 데이터를 주어진 Path에 Copy 한다.
-------------------------------------------------------------------*/
BOOL CJnlDB::BackupJNLData(CString strDestPath)
{
	FILE			*Stream;
	INT32			nFileIndex = -1;
	WIN32_FIND_DATA	FileData;
	HANDLE			hSearch = INVALID_HANDLE_VALUE;		// [#2022] NH KSK 2011.02.22
	CString			strSrcFileName, strDestFileName, strSaveFileName;
	CTime			CurTime = CTime::GetCurrentTime();

	// check validate
	if (!CheckDBState())
	{
		DB_DEBUG(ON, (_T("[CJnlDB::BackupJNLData] Failed. CheckDBState()\n")));
		return FALSE;
	}

	// check Path.
	if (strDestPath.GetLength() <= 0)
	{
		DB_DEBUG(ON, (_T("[CJnlDB::BackupJNLData] Failed. destination Path wrong. Length is 0\n")));
		return FALSE;
	}

	// Create Update folder in ROOT.
	CreateDirectory(strDestPath, NULL);

	///////////////////////////////////////////////////
	// Delete previous saved file
	strSrcFileName.Format(_T("%s\\*.*"), strDestPath);
	hSearch = FindFirstFile(strSrcFileName, &FileData);
	if (hSearch != INVALID_HANDLE_VALUE)
	{
		while(1)
		{
			strSrcFileName.Format(_T("%s\\%s"), strDestPath, FileData.cFileName);
			
			DeleteFile(strSrcFileName);
			
			if (!FindNextFile(hSearch, &FileData))
				break;
		}
		
		FindClose(hSearch);
	}

	///////////////////////////////////////////////////
	// copy journal file form JNL folder to USB folder
	
	// make file name for search.
	strSrcFileName.Format(_T("%s\\JNL??_????????.dat"), m_strJNLRoot);
	hSearch = FindFirstFile(strSrcFileName, &FileData);
	if (hSearch != INVALID_HANDLE_VALUE)
	{
		BOOL bSuccess = FALSE;

		while(1)
		{
			strSrcFileName.Format(_T("%s\\%s"), m_strJNLRoot, FileData.cFileName);
			strDestFileName.Format(_T("%s\\%s"), strDestPath, FileData.cFileName);
			
			if (!CopyFile(strSrcFileName, strDestFileName, FALSE))
				break;
			
			if (!FindNextFile(hSearch, &FileData))
			{
				bSuccess = TRUE;
				break;
			}
		}
		
		FindClose(hSearch);

		if (!bSuccess)
			return FALSE;
	}

	///////////////////////////////////////////////////
	// copy journal file form save block to USB folder
	{
		nFileIndex = GetFileIndexByJnlIndex(m_pEJSaveBlock->nLastIndex);
		// Get file full name to save
		strSaveFileName.Format(_T("%s\\JNL%2.2d_%4.4d%2.2d%2.2d.dat"), (LPCTSTR)strDestPath, nFileIndex, 
																   CurTime.GetYear(), CurTime.GetMonth(), 
																   CurTime.GetDay());

		// open file
		if ( (Stream = _tfopen(strSaveFileName, _T("w+"))) != NULL)
		{
			int	nNumWrite = 0;

			nNumWrite = fwrite (m_pEJSaveBlock->EJData, sizeof(char), m_pEJSaveBlock->nNextDataOffset, Stream);
			
			fflush(Stream);
			fclose(Stream);

			// [CODESONAR] /* Ignored Return Value (ID: 190) */
			if (nNumWrite != m_pEJSaveBlock->nNextDataOffset)
				return FALSE;
		}
		else
			return FALSE;
	}

	return TRUE;
}

// [#508] AIREAT 2009.02.27 : E-JNL Search by date Method 구현
/*-------------------------------------------------------------------
 CLASS    NAME: CJnlDB
 FUNCTION NAME: SaveJNLData()
 RETURN TYPE  : nonzero : successful
				zero : fail
 PARAMETER    : strFileName : 저장파일 이름 (경로 포함)
				Indexs : 저장할 Index 들
 DESCRIPTION  : Indexs에 해당하는 저널을 주어진 파일에 저장한다.
				- 파일 저장 폴더는 이미 생성되어 있다고 가정한다.
-------------------------------------------------------------------*/
BOOL CJnlDB::SaveJNLData(CString strFileName, CStringArray &Indexs)
{
	FILE	*pStream = NULL;
	INT32	nCurJnlIndex, nOffsetIndex;
	char	*pData;
	INT32	nNumWrite, nNumWritten;
	INT32	nSavedCount = 0;

	// check validate
	if (!CheckDBState())
	{
		DB_DEBUG(ON, (_T("[CJnlDB::SaveJNLData] Failed. CheckDBState()\n")));
		return FALSE;
	}

	// check Path.
	if (strFileName.GetLength() <= 0)
	{
		DB_DEBUG(ON, (_T("[CJnlDB::SaveJNLData] Failed. destination Path wrong. Length is 0\n")));
		return FALSE;
	}

	/////////////////////////////////
	// Save Jnl data
	if (NULL != (pStream = _tfopen(strFileName, _T("w+"))))
	{
		for (int i = 0; i < Indexs.GetSize(); i++)
		{
			nCurJnlIndex = Asc2Int(Indexs[i]);
			
			// Check Validate
			if (IsValidateJnlIndex(nCurJnlIndex) == FALSE)
			{
				DB_DEBUG(ON, (_T("[CJnlDB::SaveJNLData] Failed. IsValidateJnlIndex(), Index(%d)\n"), nCurJnlIndex));
				continue;
			}

			// Load JNL DATA
			if (LoadEJFileDB(nCurJnlIndex) == FALSE)
			{
				DB_DEBUG(ON, (_T("[CJnlDB::SaveJNLData] Failed. LoadEJFileDB(), Index(%d)\n"), nCurJnlIndex));
				continue;
			}

			nOffsetIndex = nCurJnlIndex % EJ_LOGGING_MAX;
			
			// Check Jnl Data
			if (m_arSearchDataOffsets[nOffsetIndex].pData == NULL)
			{
				DB_DEBUG(ON, (_T("[CJnlDB::SaveJNLData] Failed. pData is NULL in Offset array, Index(%d), Index(%d)\n"), nCurJnlIndex, nOffsetIndex));
				continue;
			}

			// write JNL data to File
			pData = m_arSearchDataOffsets[nOffsetIndex].pData;
			nNumWrite =  m_arSearchDataOffsets[nOffsetIndex].nJnlSize + 2;
			nNumWritten = fwrite(pData, sizeof(char), nNumWrite, pStream);
			if (nNumWrite != nNumWritten)
			{
				DB_DEBUG(ON, (_T("[CJnlDB::SaveJNLData] Failed. Written size was wrong. request(%d), result(%d)\n"), nNumWrite, nNumWritten));
				continue;	// 아무 의미 없다. codesonar가 무서워
			}

			nSavedCount++;
		}

		fflush(pStream);
		fclose(pStream);
	}

	if (nSavedCount <= 0)
		return FALSE;

	return TRUE;
}
// end of [#508]

