// MatrixCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "..\nhmwi.h"
#include "DevCtrl.h"

#include ".\Common\StructDef.h"
#include ".\Common\Commdef.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// 함 수 명 : BOOL CDevCtrl::ResetDeviceStatus()
// DESCRIPT : 장치상태 버퍼를 클리어한다.
// 매개변수 : 없음 
// 반 환 값 : 언제나 TRUE
// ----------------------------------------------------------------------------
BOOL CDevCtrl::ResetDeviceStatus()
{
	LPDATALIST		pData;

	// 고객데이타 필드를 삭제한다.
	while(!m_lstDevStatus.IsEmpty())
	{
		pData	= (LPDATALIST)m_lstDevStatus.RemoveHead();
		if (pData == NULL) break;
		delete pData;
		pData = NULL;
	}

	return TRUE;
}

// ----------------------------------------------------------------------------
// 함 수 명 : int CDevCtrl::GetDeviceStatusCount()
// DESCRIPT : 상태버퍼에 기록되어 있는 상태값의 갯수를 구한다.
// 매개변수 : 없음
// 반 환 값 : 갯수
// ----------------------------------------------------------------------------
int CDevCtrl::GetDeviceStatusCount()
{
	return m_lstDevStatus.GetCount();
}

// ----------------------------------------------------------------------------
// 함 수 명 : BOOL CDevCtrl::CheckDeviceStatusExist(LPCTSTR szName)
// DESCRIPT : 데이타리스트에 해당 데이타가 있는지 확인한다.
// 매개변수 : 데이타 이름
// 반 환 값 : TRUE If Data Exists.
// ----------------------------------------------------------------------------
BOOL CDevCtrl::CheckDeviceStatusExist(LPCTSTR szName)
{
	// 설정할 데이타 리스트가 없다면 뭘 더 확인하나..
	if (m_lstDevStatus.IsEmpty()) return FALSE;

	LPDATALIST	pData	= NULL;
	POSITION	Pos		= NULL;

	// headerPosition으로 이동한다.
	Pos = m_lstDevStatus.GetHeadPosition();

	while(Pos)
	{
		// 데이타를 하나씩 얻어온다.
		pData = (LPDATALIST)m_lstDevStatus.GetNext(Pos);
		if (pData == NULL)	break;

		// DataName이 같은 것이 있나 확인
		if (!pData->strDataName.CompareNoCase(szName))
			return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
// 함 수 명 : BOOL CDevCtrl::AddDeviceStatus(LPCTSTR szDataName, LPCTSTR szDataValue)
// DESCRIPT : 데이타리스트에 테이타를 추가한다.
// 매개변수 : 데이타명, 값
// 반 환 값 : TRUE
// ----------------------------------------------------------------------------
BOOL CDevCtrl::AddDeviceStatus(LPCTSTR szDataName, LPCTSTR szDataValue)
{
	// 같은 이름을 가진 데이타가 있는지 확인하고 없으면 추가하고 있으면 값을 넣는다.
	if (!CheckDeviceStatusExist(szDataName))
	{
		LPDATALIST   pData = new DATALIST;
		pData->strDataName = szDataName;
		pData->strDataValue = szDataValue;

		m_lstDevStatus.AddHead(pData);
	}
	else
	{
		SetDeviceStatus(szDataName, szDataValue);
	}

	return TRUE;
}

// ----------------------------------------------------------------------------
// 함 수 명 : BOOL CDevCtrl::SetDeviceStatus(LPCTSTR szDataName, LPCTSTR szDataValue)
// DESCRIPT : 데이타리스트의 데이타를 저장한다.
// 매개변수 : 데이타명, 값
// 반 환 값 : TRUE if Success
// 기타사항 : 이 함수는 리스트에 데이타의 이름은 있으나 값을 갱신하고자 할 경우에
//			  사용한다. 처음으로 추가되는 데이타라면 이 함수를 사용하지 않는다.
//			  ==> 가급적 이 함수를 직접 콜하여 사용하지 않도록 한다. 
//				  AddUserData를 사용하면 자동으로 검색하여 처리하므로 AddDeviceStatus를 사용하도록.
// ----------------------------------------------------------------------------
BOOL CDevCtrl::SetDeviceStatus(LPCTSTR szDataName, LPCTSTR szDataValue)
{
	// 설정할 데이타 리스트가 없다면 어찌할 방안이 없다.
	if (m_lstDevStatus.IsEmpty()) return FALSE;

	LPDATALIST	pData	= NULL;
	POSITION	Pos		= NULL;

	// headerPosition으로 이동한다.
	Pos = m_lstDevStatus.GetHeadPosition();

	while(Pos)
	{
		// 데이타를 하나씩 얻어온다.
		pData = (LPDATALIST)m_lstDevStatus.GetNext(Pos);
		if (pData == NULL)	return FALSE;

		// DataName이 같다면 값을 설정하고 리턴한다.
		if (!pData->strDataName.CompareNoCase(szDataName))
		{
			pData->strDataValue = szDataValue;
			return TRUE;
		}
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
// 함 수 명 : CString CDevCtrl::GetDeviceStatus(LPCTSTR szDataName)
// DESCRIPT : 장치상태값을 조회한다.
// 매개변수 : 디바이스상태명
// 반 환 값 : 상태명별 상태값
// ----------------------------------------------------------------------------
CString CDevCtrl::GetDeviceStatus(LPCTSTR szDataName)
{
	CString		retString("");

	// 설정할 데이타 리스트가 없다면 어찌할 방안이 없다.
	if (m_lstDevStatus.IsEmpty()) return retString;

	LPDATALIST	pData	= NULL;
	POSITION	Pos		= NULL;

	// headerPosition으로 이동한다.
	Pos = m_lstDevStatus.GetHeadPosition();

	while(Pos)
	{
		// 데이타를 하나씩 얻어온다.
		pData = (LPDATALIST)m_lstDevStatus.GetNext(Pos);
		if (pData == NULL)	break;

		// DataName이 같다면 값을 설정하고 리턴한다.
		if (!pData->strDataName.CompareNoCase(szDataName))
		{
			retString = pData->strDataValue;
			break;
		}
	}

	return retString;
}