// matrix.cpp : implementation file
//

#include "stdafx.h"
#include "matrix.h"
#include ".\Common\CommDef.h"
#include ".\Common\ConstDef.h"

#include ".\Common\StructDef.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
// CMatrix

// ----------------------------------------------------------------------------
// CONSTRUCTOR
// ----------------------------------------------------------------------------
CMatrix::CMatrix()
{
	m_nDeviceID			= DEV_NONE;
	m_strDeviceName.Empty();
	m_nMethodCount		= 0;
	m_nEventCount		= 0;
	m_nCurrMethod		= -1;
	
	m_nppMasterMatrix	= NULL;
	m_nppEventMatrix	= NULL;

}

// ----------------------------------------------------------------------------
// DESTRUCTOR
// 1. 프로그램 종료시 Memory Leak이 발생된다. 
//    원인 : 2차원배열이 명백하게 해제되지 않는다. 원인규명이 필요하다.
//           주석처리한 두번째 배열삭제시 에러가 발생되며 이부분이 제대로 제거되지 않는다.
// ----------------------------------------------------------------------------
CMatrix::~CMatrix()
{
	int index = 0;

	if (m_nppMasterMatrix != NULL)
	{
		for (index = 0; index<m_nEventCount;index++)
		{
			delete [] m_nppMasterMatrix[index];
		}

		delete [] m_nppMasterMatrix;
	}

	if (m_nppEventMatrix != NULL)
	{
		for (index = 0; index<m_nEventCount;index++)
			delete [] m_nppEventMatrix[index];

		delete [] m_nppEventMatrix;
	}

	// memory leak발생됨...디버깅 요망
	if (m_ppMatrix != NULL)
	{
		for (index = 0; index<m_nEventCount;index++)
			delete [] m_ppMatrix[index];

		m_ppMatrix = NULL;									// 이문장 제거가능하도록 디버깅요망[????? 필히 디버깅필요]
		delete [] m_ppMatrix;
	}
}


///////////////////////////////////////////////////////////////////////////////
// CMatrix message handlers

// ----------------------------------------------------------------------------
// 디바이스의 ID와 이름을 설정한다.
// ----------------------------------------------------------------------------
void CMatrix::SetDeviceInfo(int nDevice, char* szName)
{
	m_nDeviceID = nDevice;										// DeviceID
	m_strDeviceName = szName;									// DeviceName
}

// ----------------------------------------------------------------------------
// 디바이스의 ID를 조회한다.
// ----------------------------------------------------------------------------
int CMatrix::GetDeviceID()
{
	return m_nDeviceID;
}

// ----------------------------------------------------------------------------
// 디바이스의 이름을 조회한다.
// ----------------------------------------------------------------------------
CString CMatrix::GetDeviceName()
{
	CString strResult("");

	strResult = m_strDeviceName;

	return strResult;
}

// ----------------------------------------------------------------------------
// 디바이스의 메소드 갯수를 설정한다.
// ----------------------------------------------------------------------------
void CMatrix::SetMethodCount(int nCount)
{
	m_nMethodCount = nCount;
}

// ----------------------------------------------------------------------------
// 디바이스의 이벤트 갯수를 설정한다.
// ----------------------------------------------------------------------------
void CMatrix::SetEventCount(int nCount)
{
	m_nEventCount = nCount;
}

// ----------------------------------------------------------------------------
// 디바이스의 메소드 갯수를 조회한다.
// ----------------------------------------------------------------------------
int	CMatrix::GetMethodCount()
{
	return m_nMethodCount;
}

// ----------------------------------------------------------------------------
// 디바이스의 이벤트 갯수를 조회한다
// ----------------------------------------------------------------------------
int CMatrix::GetEventCount()
{
	return m_nEventCount;
}

// ----------------------------------------------------------------------------
// 장치 디바이스별 메소드-이벤트의 마스터메트릭스를 설정한다.
// ----------------------------------------------------------------------------
BOOL CMatrix::SetMasterMatrix(ITEMLIST* ppList)
{
	int index;
	// 기준메트릭스의 포인터를 확인한다.
	if (ppList == NULL)
	{
		return FALSE;
	}

	// 메소드갯수나 이벤트갯수를 확인한다.
	if ((m_nMethodCount <= 0) || (m_nEventCount <= 0))
	{
		return FALSE;
	}

	// 일단 메모리를 할당한다.(갯수만큼)
	// 행열이므로 행을 먼저 할당했는데....이러면 안되는거 같다.
	// ==> 실제로 YES/NO문자열을 가진 오리지널 메트릭스정보를 저장한다.
	// --------------------------------------------------------
	m_ppMatrix = new LPITEMLIST[m_nMethodCount];
	for (int i = 0; i<m_nMethodCount;i++)
		m_ppMatrix[i] = new ITEMLIST[m_nEventCount];

	// 일단 메모리를 할당한다.(갯수만큼)
	// ==> 오리지널 메트릭스정보를 이용하여 0/1의 숫자값으로 변경하여 저장한다.
	// --------------------------------------------------------
	m_nppMasterMatrix = new int*[m_nMethodCount];
	for (index = 0; index<m_nMethodCount;index++)
		m_nppMasterMatrix[index] = new int[m_nEventCount];

	// 일단 메모리를 할당한다.(갯수만큼)
	// ==> 호출하는 메소드에 의해서 설정되고 발생되는 이벤트에 대해서 또다시 설정되는
	//	   메트릭스정보..이를 활용하여 디바이스 구동상태를 확인한다.(For CheckDeviceAction)
	// --------------------------------------------------------
	m_nppEventMatrix = new int*[m_nMethodCount];
	for (index = 0; index<m_nMethodCount;index++)
		m_nppEventMatrix[index] = new int[m_nEventCount];

	// 방법1) 개별처리
	int nProcCount = 0;
	CString strTmp("");
	CString strTmp2("");
	CString strLine("");

	strTmp += SCR_CMD_DELIMITER;
	for (int mIndex=0;mIndex<m_nMethodCount;mIndex++)
	{
		for (int eIndex=0;eIndex<m_nEventCount;eIndex++)
		{
			memset(m_ppMatrix[mIndex][eIndex].szItem, 0x00, 64);
			memcpy(m_ppMatrix[mIndex][eIndex].szItem, ppList[nProcCount].szItem, sizeof(m_ppMatrix[mIndex][eIndex].szItem));
			nProcCount++;

			if (strcmp(m_ppMatrix[mIndex][eIndex].szItem,"YES") == 0)	m_nppMasterMatrix[mIndex][eIndex] = MWI_EVENT_EXIST;
			else m_nppMasterMatrix[mIndex][eIndex] = MWI_EVENT_NOTEXIST;

			strTmp2.Format(L"|%4.4S", m_ppMatrix[mIndex][eIndex].szItem);

			strTmp += strTmp2;
			strLine += "-----";

			// 메소드/이벤트메트릭스에 대한 초기화처리
			m_nppEventMatrix[mIndex][eIndex] = MWI_EVENT_RESET;
		}
		strTmp += "|\n";
		strTmp += strLine + SCR_CMD_DELIMITER;
		strLine.Empty();
	}

	return TRUE;
}

// ----------------------------------------------------------------------------
// 함 수 명 : BOOL CMatrix::ResetMethodEventMatrix(int nMethodIndex)
// DESCRIPT : 이벤트 메트릭스를 초기화한다.(인자가 -1이면 전체초기화, 이외는 라인별..0 Based Index)
// 매개변수 : Index(0 based)
// 반 환 값 : TRUE
// ----------------------------------------------------------------------------
BOOL CMatrix::ResetMethodEventMatrix(int nMethodIndex)
{
	// 해당메소드 인덱스의 이벤트설정정보를 초기화한다.
	if (nMethodIndex >= 0)
	{
		memset(m_nppEventMatrix[nMethodIndex], 0, m_nEventCount);

		// 현재의 메소드인덱스를 저장한다.
		m_nCurrMethod = nMethodIndex;
	}
	else
	{
		memset(m_nppEventMatrix, 0, m_nEventCount*m_nMethodCount);
		m_nCurrMethod = -1;
	}

	return TRUE;
}

// ----------------------------------------------------------------------------
// 함 수 명 : BOOL CMatrix::SetMethodEventMatrix(LPCTSTR szMethodName)
// DESCRIPT : 해당 메소드별로 발생가능한 이벤트 정보를 설정한다.
// 매개변수 : 메소드 Index
// 반 환 값 : TRUE
// ----------------------------------------------------------------------------
BOOL CMatrix::SetMethodEventMatrix(int nMethodIndex)
{
	int nIndex = 0;

	// 해당메소드 인덱스의 이벤트설정정보를 초기화한다.
	ResetMethodEventMatrix(nMethodIndex);

	// 현메소드 인덱스가 무효한 경우 처리가 불가하다.:2003.05.17 teo
	if (m_nCurrMethod < 0)
	{
		return FALSE;
	}

	// 마스트메트릭스와 비교하여 이벤트메트릭스를 설정한다.
	for(nIndex=0;nIndex < m_nEventCount;nIndex++)
	{
		if (m_nppMasterMatrix[nMethodIndex][nIndex] == MWI_EVENT_EXIST)
			m_nppEventMatrix[nMethodIndex][nIndex] = MWI_EVENT_SETTING;
	}

	return TRUE;
}

// ----------------------------------------------------------------------------
// 함 수 명 : int CMatrix::GetCurrMethodIndex()
// DESCRIPT : 현재수행중인 메소드의 인덱스를 리턴한다.
// 매개변수 : 없음 
// 반 환 값 : 인덱스(하나도 수행하지 않았다면 -1)
// ----------------------------------------------------------------------------
int CMatrix::GetCurrMethodIndex()
{
	return m_nCurrMethod;
}

// ----------------------------------------------------------------------------
// 함 수 명 : BOOL CMatrix::SetEventMatrixValue(int nEventIndex)
// DESCRIPT : 이벤트값을 메트릭스에 설정한다.
// 매개변수 : 
// 반 환 값 : 
// ----------------------------------------------------------------------------
BOOL CMatrix::SetEventMatrixValue(int nEventIndex)
{
	// 인덱스 검증
	if ((nEventIndex < 0) || (nEventIndex >= m_nEventCount))
	{
		return FALSE;
	}
	
	// 해당이벤트가 발생됨을 설정함.
	if (m_nppEventMatrix[m_nCurrMethod][nEventIndex] == MWI_EVENT_SETTING)
	{
		m_nppEventMatrix[m_nCurrMethod][nEventIndex] = MWI_EVENT_OCCUR;
	}

	return TRUE;
}

/**
* Gets the index of event set for given method index
* @returns the event index or -1 if none of events were found
*/
int CMatrix::GetEventIndex(int nMethodIndex)
{
	if ((nMethodIndex < 0) || (nMethodIndex >= m_nMethodCount))
		return -1;

	int nEventValue = -1;
	for (int nEventIdx = 0; nEventIdx < m_nEventCount; nEventIdx++)
	{
		if (m_nppEventMatrix[nMethodIndex][nEventIdx] == MWI_EVENT_OCCUR)
		{
			// TBD: would it be the only event we can find from this loop? should we seek until the end?
			//RETAILMSG(1, (L"Event Found: %d\r\n", nEventIdx));
			nEventValue = nEventIdx;
			break;
		}
	}

	return nEventValue;
}


// ----------------------------------------------------------------------------
// 함 수 명 : 
// DESCRIPT : 
// ----------------------------------------------------------------------------
int CMatrix::GetMasterMatrixValue(int nMethodIndex, int nEventIndex)
{
	if (nMethodIndex >= m_nMethodCount)	return 0;
	if (nEventIndex  >= m_nEventCount)	return 0;
	return m_nppMasterMatrix[nMethodIndex][nEventIndex];
}

// ----------------------------------------------------------------------------
// 함 수 명 : 
// DESCRIPT : 
// ----------------------------------------------------------------------------
int CMatrix::GetEventMatrixValue(int nMethodIndex, int nEventIndex)
{
	if (nMethodIndex >= m_nMethodCount)	return 0;
	if (nEventIndex  >= m_nEventCount)	return 0;
	return m_nppEventMatrix[nMethodIndex][nEventIndex];
}




// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
