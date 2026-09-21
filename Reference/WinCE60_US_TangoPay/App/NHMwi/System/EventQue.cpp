// EventQue.cpp : INI파일에 대한 기본적인 제어를 수행하도록 한다.

#include "stdafx.h"
#include "EventQue.h"
#include ".\Common\CommDef.h"
#include "..\H\Common\CmnLib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////

CEventQue::CEventQue()
{
}

CEventQue::~CEventQue()
{
}
///////////////////////////////////////////////////////////////////////////////
void CEventQue::Init(LPCSTR szOwnerName /* = "NAUTILUS" */)
{
	m_strOwnerName = szOwnerName; 
	ResetQue();
}

void CEventQue::ResetQue()
{
	LPEVENTDATA		pEventData = NULL;							// 2004.11.05
	
	while(!m_EventQue.IsEmpty())
	{
		pEventData	= (LPEVENTDATA)m_EventQue.RemoveHead();
		if (pEventData == NULL) break;
		delete pEventData;
		pEventData = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////
BOOL CEventQue::IsEmpty()
{
	return m_EventQue.IsEmpty();
}

///////////////////////////////////////////////////////////////////////////////
int CEventQue::GetEventCount()
{
	return m_EventQue.GetCount();
}

///////////////////////////////////////////////////////////////////////////////
BOOL CEventQue::PutData(char* szType, char* szName, char* szEvent, char* szValue)
{
	if (strlen(szEvent) == 0)	return FALSE;

	LPEVENTDATA	pEventData = new EVENTDATA;
	pEventData->strEventType	= szType;
	pEventData->strQueOwner		= szName;
	if (pEventData->strEventType.CompareNoCase(L"EVENT_DEVICE") == 0)
	{
		pEventData->strEventName	= szEvent;
		pEventData->strEventLastCmd = szValue;
		pEventData->strDataName		= "";
		pEventData->strDataValue	= "";
	}
	else
	{
		pEventData->strEventName	= "";
		pEventData->strEventLastCmd = "";
		pEventData->strDataName		= szEvent;
		pEventData->strDataValue	= szValue;
	}

	m_EventQue.AddTail(pEventData);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
BOOL CEventQue::GetData(char* szType, char* szName, char* szEvent, char* szValue)
{
	LPEVENTDATA		pEventData = NULL;//new EVENTDATA;
	POSITION		Pos = m_EventQue.GetHeadPosition();
	POSITION		OldPos = NULL;
	OldPos = Pos;
	if (Pos == NULL)
	{
		szType = "";	szName = "";	szEvent = "";	szValue = "";
		return FALSE;
	}

	pEventData = (LPEVENTDATA)m_EventQue.GetNext(Pos);

	if (pEventData == NULL)
	{
		szType = "";	szName = "";	szEvent = "";	szValue = "";
		return FALSE;
	}
	
// 2006.03.14 for WinCE
//	WideToMulti(szType, pEventData->strEventType.GetBuffer(0), pEventData->strEventType.GetLength()*2);		// AIREAT 20100211
//	pEventData->strEventType.ReleaseBuffer();																// AIREAT 20100211
//	WideToMulti(szName, pEventData->strQueOwner.GetBuffer(0), pEventData->strQueOwner.GetLength()*2);		// AIREAT 20100211
//	pEventData->strQueOwner.ReleaseBuffer();																// AIREAT 20100211
	WideToMulti(szType, pEventData->strEventType, pEventData->strEventType.GetLength()*2);
	WideToMulti(szName, pEventData->strQueOwner, pEventData->strQueOwner.GetLength()*2);

//////////////////////////////////////////////////////////////////////////

	if (pEventData->strEventType.CompareNoCase(L"EVENT_DEVICE") == 0)
	{
// 2006.03.14 for WinCE
//		WideToMulti(szEvent, pEventData->strEventName.GetBuffer(0), pEventData->strEventName.GetLength()*2);		// AIREAT 20100211
//		pEventData->strEventName.ReleaseBuffer();																	// AIREAT 20100211
//		WideToMulti(szValue, pEventData->strEventLastCmd.GetBuffer(0), pEventData->strEventLastCmd.GetLength()*2);	// AIREAT 20100211
//		pEventData->strEventLastCmd.ReleaseBuffer();																// AIREAT 20100211
		WideToMulti(szEvent, pEventData->strEventName, pEventData->strEventName.GetLength()*2);
		WideToMulti(szValue, pEventData->strEventLastCmd, pEventData->strEventLastCmd.GetLength()*2);
//////////////////////////////////////////////////////////////////////////
	}
	else
	{
// 2006.03.14 for WinCE
//		WideToMulti(szEvent, pEventData->strDataName.GetBuffer(0), pEventData->strDataName.GetLength()*2);
//		pEventData->strDataName.ReleaseBuffer();
//		WideToMulti(szValue, pEventData->strDataValue.GetBuffer(0), pEventData->strDataValue.GetLength()*2);
//		pEventData->strDataValue.ReleaseBuffer();
		WideToMulti(szEvent, pEventData->strDataName, pEventData->strDataName.GetLength()*2);			// AIREAT 20100211
		WideToMulti(szValue, pEventData->strDataValue, pEventData->strDataValue.GetLength()*2);			// AIREAT 20100211
//////////////////////////////////////////////////////////////////////////
	}
	m_EventQue.RemoveAt(OldPos);

	delete pEventData;											// 2004.11.05

	return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
BOOL CEventQue::CheckDeviceDataExist(char* szType, char* szName, char* szEvent, char* szValue)
{
	BOOL retVal = FALSE;

	// 장치디바이스 큐일경우에만 하단을 처리한다.
	if (strcmp(szType, (LPCSTR)MWI_EVENT_DEVICE) != 0)	return FALSE;

	LPEVENTDATA		pEventData = NULL;
	POSITION		Pos = m_EventQue.GetHeadPosition();
	if (Pos == NULL) return FALSE;
	
	CString strEvent = CString(szEvent);

	while(true)
	{
		pEventData = (LPEVENTDATA)m_EventQue.GetNext(Pos);

		if (pEventData == NULL) 
			break;

		if (pEventData->strEventName.CompareNoCase(strEvent) == 0)
		{
			retVal = TRUE;
			break;
		}

		if (Pos == NULL)
			break;
	}

	return retVal;
}

///////////////////////////////////////////////////////////////////////////////
// 에러메시지가 검지될때까지 이벤트를 펌핑한다.
// 에러이벤트가 검지되면 펌핑을 그만두고 TRUE를 리턴하고,
// 에러이벤트가 검지되지 않으면 끝까지 펌핑하고 TRUE를 리턴한다.
BOOL CEventQue::PumpingEventToError()
{
	BOOL retVal = FALSE;

	LPEVENTDATA		pEventData = NULL;
	POSITION		Pos = m_EventQue.GetHeadPosition();
	POSITION		OldPos = NULL;
	OldPos = Pos;

	if (Pos == NULL) return FALSE;
	
	while(true)
	{
		pEventData = (LPEVENTDATA)m_EventQue.GetNext(Pos);

		if (pEventData == NULL) 
		{
			retVal = TRUE;
			break;
		}

// 2006.03.14 for WinCE
// (LPCTSTR) -> _T(...)
		if ((pEventData->strEventName.CompareNoCase(_T("DeviceError")) == 0)	||
			(pEventData->strEventName.CompareNoCase(_T("FatalError")) == 0)	)
		{
			retVal = TRUE;
			break;
		}

		// 이벤트를 삭제한다.
		m_EventQue.RemoveAt(OldPos);
		OldPos = Pos;

		delete pEventData;										// 2004.11.05
		
		if (Pos == NULL)
		{
			retVal = TRUE;
			break;
		}
	}

	return retVal;
}