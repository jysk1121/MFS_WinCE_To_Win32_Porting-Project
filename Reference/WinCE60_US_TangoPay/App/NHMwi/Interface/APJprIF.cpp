// JPRIF.cpp

#include "stdafx.h"
#include "..\NHMWI.h"
#include "..\NHMWICtl.h"
#include "..\NHMWIPpg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


///////////////////////////////////////////////////////////////////////////////////////////////////
// Method : Called From AP
///////////////////////////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// PROTOTYPE : long JprInitialize() 
// DESCRIPT  : Power-On 초기화처리
// PARAMETER : 없음
// RETURN    : 함수수행결과(0-정상수행)
// REMARK	 : 20030814_1.0_1066 : Device Open Proc.
// ----------------------------------------------------------------------------
long CNHMWICtrl::JprInitialize() 
{
	m_nJprExist = ST_NOT_DETECT;

	int nResult = 0;
	if (m_bJprOpened)
	{
		m_pDevCtrl->EvtQReset(DEV_JPR);
		nResult = DevJprCloseConnection();
		if (nResult != R_NORMAL)
		{
			return nResult;
		}
	}
	
	m_bJprOpened = TRUE;
	nResult = DevJprOpenConnection();

	return nResult;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long JprDeinitialize() 
// DESCRIPT  : 세션종료
// PARAMETER : 없음
// RETURN    : 함수수행결과(0-정상수행)
// REMARK	 : 20030814_1.0_1066 : Device Open Proc.
// ----------------------------------------------------------------------------
long CNHMWICtrl::JprDeinitialize() 
{
	m_nJprExist = ST_NOT_DETECT;

	int nResult = 0;
	if (m_bJprOpened)
	{
		m_bJprOpened = FALSE;										
		nResult = DevJprCloseConnection();
	}
	else
	{
		m_bJprOpened = FALSE;										
	}
	return nResult;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : BSTR JprGetErrorCode() 
// DESCRIPT  : 장애코드를 조회한다.
// PARAMETER : 없음
// RETURN    : 장애코드
// ----------------------------------------------------------------------------
BSTR CNHMWICtrl::JprGetErrorCode() 
{
	CString strResult("");
	strResult = DevJprGetErrorCode();
	return strResult.AllocSysString();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long JprClearErrorCode() 
// DESCRIPT  : 저널부의 장애코드를 클리어한다.
// PARAMETER : 없음
// RETURN    : NORMAL(0)
// ----------------------------------------------------------------------------
long CNHMWICtrl::JprClearErrorCode() 
{
	return DevJprClearErrorCode();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : BSTR JprGetSensorInfo() 
// DESCRIPT  : 저널부의 센서정보를 조회한다
// PARAMETER : 없음
// RETURN    : 조회한 센서정보값(String type)
// ----------------------------------------------------------------------------
BSTR CNHMWICtrl::JprGetSensorInfo() 
{
	CString strResult("");
	strResult = DevJprGetSensorInfo();							// 20030719_1.0_1053 : GetSensorInfo
	return strResult.AllocSysString();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long JprGetDeviceStatus() 
// DESCRIPT  : 저널의 장치상태를 조회한다.
// PARAMETER : 없음
// RETURN    : NORMAL(0), DOWN(2)
// ----------------------------------------------------------------------------
long CNHMWICtrl::JprGetDeviceStatus() 
{
	return DevJprGetStatus(1);
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long JprGetPaperStatus() 
// DESCRIPT  : 저널의 용지상태를 조회한다.
// PARAMETER : 없음
// RETURN    : SLIP_NORMAL(0), SLIP_EMPTY_PAPER(1), SLIP_LOW_END(2)
// ----------------------------------------------------------------------------
long CNHMWICtrl::JprGetPaperStatus() 
{
	return DevJprGetStatus(3);
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long JprGetMaterialInfo()  
// DESCRIPT  : 매체잔류정보 조회
// PARAMETER : 없음
// RETURN    : ST_NOT_DETECT(0), ST_SENSOR1(1)-ANY
// ----------------------------------------------------------------------------
long CNHMWICtrl::JprGetMaterialInfo() 
{
	int nResult = ST_NOT_DETECT;

//t	if (m_nJprExist)
//t		nResult = ST_SENSOR1;

	nResult = (m_nJprExist);

	return nResult;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long JprEmbossPrint() 
// DESCRIPT  : 엠보스를 저널에 프린트한다.
// PARAMETER : 없음
// RETURN    : 함수 실행결과(0-정상실행)
// REMARK    : 엠보스를 프린트하기 우해서는 저널부에 데이타로 "Emboss"를 넘겨야 한다.
//	           이는 데이타를 AP에서 받지 않고 MWI에서 처리한다.
// ----------------------------------------------------------------------------
long CNHMWICtrl::JprEmbossPrint() 
{
	m_nJprExist = (ST_SENSOR1 /*| ST_SENSOR2*/);
	SetEventMatrix(DEV_JPR, "EmbossPrint");
	return DevJprEmbossPrint();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long JprPrint(LPCTSTR szPrintData) 
// DESCRIPT  : 저널을 인자한다.
// PARAMETER : 인자하고자 하는 데이타
// RETURN    : 함수수행결과
// REMARK    : 파라미터로 전달되는 출력데이타는 hexaString이다. 또한 Comma(,)로 구분되어 
//             있으므로 이를 나누고 변형하여 출력데이타 폼에 맞추어야 한다.
// ----------------------------------------------------------------------------
long CNHMWICtrl::JprPrint(LPCTSTR szPrintData) 
{
	m_nJprExist = (ST_SENSOR1 /*| ST_SENSOR2*/);
	SetEventMatrix(DEV_JPR, "PrintForm");

	long nRet;

	// 저널 인자시 저널부의 데이타의 길이를 확인한다.
	// 만일 데이타가 NULL이면 인자시 Device(혹은 Fatal)Error가 발생한다.
	// 명세표의 경우도 마찬가지이다.
	// 데이타가 없는 경우 "20"(Space)을 삽입한다.
	// 20030913_1.0_1072 : Print NULL Data 보정처리
	if (strlen((LPCSTR)szPrintData) == 0)
		nRet = DevJprPrint((LPCTSTR)"20");
	else
		nRet = DevJprPrint(szPrintData);

	SysFreeString((BSTR)szPrintData);

	return nRet;
}
