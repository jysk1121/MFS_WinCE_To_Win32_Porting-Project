#include "stdafx.h"
#include "EagleDefine.h"



/** **********************************************************
*	@brief		°í°´ Á¤º¸
************************************************************/
void EAGLE_CLIENT_INFO::Clear(void)
{
	m_bIsTransaction = FALSE;

	m_strAccountNo.Empty();

	m_strPinBlock.Empty();

	m_bIsSetError = FALSE;

	m_nErrorDevice = 0;

	m_strErrorCode.Empty();
	m_strErrorMsg.Empty();

	m_bIsSetReversalError = FALSE;

	m_bIsPrintReceipt = FALSE;

	m_bIsBlockBin = FALSE;

	m_nSoundVolume = 0;

	m_nProcessCount = 0;

	m_bPowerOffFlag = FALSE;

	m_bEMV_CardExist = FALSE;

	m_bEMV_Transaction = FALSE;

	m_bExec2ndGAC = FALSE;

	m_strTag57Data.Empty();
	m_strTag5AData.Empty();

	m_bEMVDeclined = FALSE;

	m_bEMV_SoundNotice = FALSE;

	m_bRecvConfigInfo = FALSE;
}
