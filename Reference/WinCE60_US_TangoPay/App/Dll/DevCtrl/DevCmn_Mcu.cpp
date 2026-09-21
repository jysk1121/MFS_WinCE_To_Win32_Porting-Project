/////////////////////////////////////////////////////////////////////////////
//	DevMcu.cpp : implementation file
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include ".\Dev\DevCmn.h"

//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------

//#define NH_DEBUG		// Debug Message On/Off
#include ".\Common\NHDbgApi.h"

#define	DBG_CALL		1
#define DBG_INFO		1

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnMCU_CardEnDisable()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Card En/Disable
-------------------------------------------------------------------*/
int CDevCmn::fnMCU_CardEnDisable(int Action, int WaitFlag)
{
	NHTRACE((L"[CDevCmn::fnMCU_CardEnDisable] Action(%d)\n", Action));

	if (!fnAPL_GetAvailDevice(DEV_MCU))
		return T_OK;

	if (Action == ENABLE)
	{
		fnAPL_CheckDeviceAction(DEV_MCU);
		fnMCU_ClearErrorCode();					// 2005.09.11
		fnAPL_DeviceEnDisable(DEV_MCU, ENABLE, FALSE);	// [#397] [NH] psc 2008.08.01 EMV, MS 공통화

		if (WaitFlag)
			fnAPL_CheckDeviceAction(DEV_MCU);

		// [#2205] US KSK 2013.06.28
		if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH2600")
			SetHaloLedControl(SKIP_HALOLED_COLOR, SKIP_HALOLED_MODE, FLICKER_MCULED);
		else if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX5400")		// [#GLDV-2890]
			SetGuideLight(0x08, CDU_VB_FLICKER | EPP_VB_FLICKER);
		else
			fnAPL_DeviceSetFlicker(DEV_MCU, FLICKER_ON);
		// end of [#2205]
	}
	else
	{
		fnAPL_CheckDeviceAction(DEV_MCU);
		fnAPL_DeviceEnDisable(DEV_MCU, DISABLE, FALSE);	// [#397] [NH] psc 2008.08.01 EMV, MS 공통화

		if (WaitFlag)
			fnAPL_CheckDeviceAction(DEV_MCU);

		/* IC CHIP POWER OFF & Check Device Action */
		if(fnMCU_IsEmvChipPowerOn())
		{
			fnMCU_ICChipPower(IC_POWER_OFF);
			fnAPL_CheckDeviceAction(DEV_MCU);
		}

		// [#2205] US KSK 2013.06.28
		if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH2600")
			SetHaloLedControl(SKIP_HALOLED_COLOR, SKIP_HALOLED_MODE, OFF_MCULED);
		else if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX5400")		// [#GLDV-2890]
			SetGuideLight(0x00, CDU_VB_FLICKER);
		else
			fnAPL_DeviceSetFlicker(DEV_MCU, FLICKER_OFF);
		// end of [#2205]
	}

	return T_OK;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnMCU_ClearErrorCode()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Clear Error Code
-------------------------------------------------------------------*/
int	CDevCmn::fnMCU_ClearErrorCode()
{
	NHTRACE((L"[CDevCmn::fnMCU_ClearErrorCode]\n"));

	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_MCU))
		nReturn = m_pMwi->McuClearErrorCode();

	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fstrMCU_GetErrorCode()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Get Error Code
-------------------------------------------------------------------*/
CString CDevCmn::fstrMCU_GetErrorCode()
{
	NHTRACE((L"[CDevCmn::fstrMCU_GetErrorCode]\n"));

	CString strReturn("");

	if (fnAPL_GetDefineDevice(DEV_MCU))
		strReturn = m_pMwi->McuGetErrorCode();

	return strReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnMCU_GetDeviceStatus()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Get Device Status
-------------------------------------------------------------------*/
int CDevCmn::fnMCU_GetDeviceStatus()
{
	// [Win32-fixes] US ryan.payton 2022.10.10 Need a proper way to check MCU status for Win32
#ifndef UNDER_CE
	return 0;
#endif

	NHTRACE((L"[CDevCmn::fnMCU_GetDeviceStatus]\n"));

	static int nSaveReturn = FALSE;
	int nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_MCU))
		nReturn = m_pMwi->McuGetDeviceStatus();

	if (nReturn != nSaveReturn)
	{
		nSaveReturn = nReturn;
		m_strNVLog.Format(L"1:MCUST(%d)", nReturn);
		NVDump('O', 'E', "73", L"", m_strNVLog);
	}

	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnMCU_GetMaterialInfo()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Get Media Status
-------------------------------------------------------------------*/
int	CDevCmn::fnMCU_GetMaterialInfo()
{
	NHTRACE((L"[CDevCmn::fnMCU_GetMaterialInfo]\n"));

	int	nReturn = 0;
	
	if(!fnMCU_IsEmvEnable())					// MS mode 일때는 무조건 0(return 값 참조하지 않기위해) 을 return
		return nReturn;

	if (fnAPL_GetDefineDevice(DEV_MCU))			// IC mode
		nReturn = m_pMwi->McuGetMaterialInfo();	// return value: ST_NOT_DETECT(0), ST_SENSOR1(1), ST_SENSOR2(2)

	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fstrMCU_GetCardData()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Get Card Data
-------------------------------------------------------------------*/
CString CDevCmn::fstrMCU_GetCardData()
{
	NHTRACE((L"[CDevCmn::fstrMCU_GetCardData]\n"));

	CString strReturn("");
	
	if (fnAPL_GetDefineDevice(DEV_MCU))
	{
		strReturn = m_pMwi->McuGetCardData();
	}

	return strReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fstrMCU_GetICData()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Get IC Data
-------------------------------------------------------------------*/
CString	CDevCmn::fstrMCU_GetICData()
{
	NHTRACE((L"[CDevCmn::fstrMCU_GetICData]\n"));

	CString	strReturn("");

	strReturn = m_pMwi->McuGetICData();
	
	return strReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnMCU_Initialize()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Initialize
-------------------------------------------------------------------*/
int	CDevCmn::fnMCU_Initialize()
{
	NHTRACE((L"[CDevCmn::fnMCU_Initialize]\n"));

	int		nReturn = FALSE;

	DeviceOperationFlag = TRUE;
	if (fnAPL_GetDefineDevice(DEV_MCU))
		nReturn = m_pMwi->McuInitialize();

	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnMCU_Deinitialize()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Deinitialize
-------------------------------------------------------------------*/
int	CDevCmn::fnMCU_Deinitialize()
{
	NHTRACE((L"[CDevCmn::fnMCU_Deinitialize]\n"));

	int		nReturn = FALSE;

	DeviceOperationFlag = TRUE;
	if (fnAPL_GetDefineDevice(DEV_MCU))
		nReturn = m_pMwi->McuDeinitialize();

	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnMCU_EntryEnable()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Entry Enable(Wait Time)
-------------------------------------------------------------------*/
int	CDevCmn::fnMCU_EntryEnable(int nWaitSec)
{
	NHTRACE((L"[CDevCmn::fnMCU_EntryEnable]\n"));

	int		nReturn = FALSE;

	DeviceOperationFlag = TRUE;
	if (fnAPL_GetDefineDevice(DEV_MCU))
		nReturn = m_pMwi->McuEntryEnable(nWaitSec);

	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnMCU_EntryDisable()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Entry Disable
-------------------------------------------------------------------*/
int	CDevCmn::fnMCU_EntryDisable()
{
	NHTRACE((L"[CDevCmn::fnMCU_EntryDisable]\n"));

	int		nReturn = FALSE;

	DeviceOperationFlag = TRUE;
	if (fnAPL_GetDefineDevice(DEV_MCU))
		nReturn = m_pMwi->McuEntryDisable();

	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnMCU_ICChipInitialize()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Initialize IC Chip:Move Point
-------------------------------------------------------------------*/
int CDevCmn::fnMCU_ICChipInitialize()
{
	NHTRACE((L"[CDevCmn::fnMCU_ICChipInitialize]\n"));

	int		nReturn = FALSE;

	DeviceOperationFlag = TRUE;
	nReturn = m_pMwi->McuICChipInitialize();

	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnMCU_ICEntryEnable()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : ICEntry Enable(Wait Time)
-------------------------------------------------------------------*/
int	CDevCmn::fnMCU_ICEntryEnable(int nWaitSec)
{
	NHTRACE((L"[CDevCmn::fnMCU_ICEntryEnable]\n"));

	int	nReturn = FALSE;

	nIsEmvChipPowerOn = 1;								// Chip PowerOff를 여러번 하지 않기 위해 flag 셋팅
	DeviceOperationFlag = TRUE;
	nReturn = m_pMwi->McuICEntryEnable(nWaitSec);

	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnMCU_ICEntryDisable()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : ICEntry Disable
-------------------------------------------------------------------*/
int	CDevCmn::fnMCU_ICEntryDisable()
{
	NHTRACE((L"[CDevCmn::fnMCU_ICEntryDisable]\n"));

	int	nReturn = FALSE;

	DeviceOperationFlag = TRUE;
	
	nReturn = m_pMwi->McuICEntryDisable();

	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnMCU_ICChipPower()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : IC Chip Power
-------------------------------------------------------------------*/
int	CDevCmn::fnMCU_ICChipPower(int nActType)
{
	NHTRACE((L"[CDevCmn::fnMCU_ICChipPower] nActType(%d), nIsEmvChipPowerOn(%d)\n", nActType, nIsEmvChipPowerOn));

	int	nReturn = FALSE;

	nIsEmvChipPowerOn = 0;

	DeviceOperationFlag = TRUE;
	nReturn = m_pMwi->McuICChipPower(nActType);

	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnMCU_IsEmvTransaction()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : EMV(IC), MS 거래 판단.
-------------------------------------------------------------------*/
BOOL CDevCmn::fnMCU_IsEmvTransaction()
{
	NHTRACE((L"[CDevCmn::fnMCU_IsEmvTransaction]\n"));

	if(fnMCU_IsEmvEnable())
		return ((nKindOfMedia & MEDIA_IC) == MEDIA_IC);

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnMCU_IsEmvEnable()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : EMV(IC)거래가 가능한지 판단.
-------------------------------------------------------------------*/
BOOL CDevCmn::fnMCU_IsEmvEnable()
{
	NHTRACE((L"[CDevCmn::fnMCU_IsEmvEnable]\n"));

	return ((BOOL)MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_EMV_ENABLE));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnMCU_IsEmvAvailable()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : EMV(IC)거래가 가능한 기기인지 판단.
-------------------------------------------------------------------*/
BOOL CDevCmn::fnMCU_IsEmvAvailable()
{
	NHTRACE((L"[CDevCmn::fnMCU_IsEmvAvailable]\n"));

	return ((BOOL)MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_EMV_AVAILABLE));
}

// [#2082] CA KSK 2011.07.02
/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnMCU_IsEmvMSFirst()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
//BOOL CDevCmn::fnMCU_IsEmvMSFirst()
//{
//	NHTRACE((L"[CDevCmn::fnMCU_IsEmvMSFirst]\n"));
//
//	if(fnMCU_IsEmvEnable())
//	{
//		if(MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_EMV_TRANSACTIONOPTION) == 0)
//			return TRUE;
//	}
//
//	return FALSE;
//}
// end of [#2082]

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnMCU_IsEmvChipPowerOn()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CDevCmn::fnMCU_IsEmvChipPowerOn()
{
	NHTRACE((L"[CDevCmn::fnMCU_IsEmvChipPowerOn]\n"));

	if(nIsEmvChipPowerOn == 1)
		return TRUE;

	return FALSE;
}