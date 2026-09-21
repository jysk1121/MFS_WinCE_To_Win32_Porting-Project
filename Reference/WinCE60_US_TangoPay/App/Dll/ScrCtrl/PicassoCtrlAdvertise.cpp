#include "stdafx.h"
#include "PicassoCtrl.h"

//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------

//#define NH_DEBUG		// Debug Message On/Off
#include ".\Common\NHDbgApi.h"

#define DBG_CALL		1
#define DBG_INFO		1


//------------------------------------------------------------------
//	Implementation
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: AdvertiseInitialize()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoCtrl::AdvertiseInitialize()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	CString		strKey;
	int			nIndex;
	int			i;

	// InitValue
	for (i = 0; i < SUPPORT_ADVNOTICE; i++)
	{
		m_AdvNoticeWelcomeInfo.bEnable = FALSE;
		m_AdvNoticeWelcomeInfo.nCurIndex = 0;
		m_AdvNoticeWelcomeInfo.nMaxIndex = 0;
		m_AdvNoticeWelcomeInfo.pScreen[i] = NULL;
	}

	nIndex = 1;

	// Set Advertisement
	for (i = 0; i < SUPPORT_ADVERTISE; i++)
	{
		if (m_AdvertiseInfo.bEnableAdv[i] == TRUE && i < m_AdvertiseInfo.nEnableCount)
		{
			NHUIDBG(DBG_INFO, (_T("index [%d] is Enabled.\n"), i));

			// Get Advertisement Screen.
			if (m_AdvertiseInfo.pAdvScreen[i] == NULL)
			{
				strKey.Format(_T("%d"), 30011+i);
				//m_PicassoScreens.FindDataOfList(strKey, &m_AdvertiseInfo.pAdvScreen[i]);
				m_PicassoScreensCMap.Lookup(strKey, m_AdvertiseInfo.pAdvScreen[i]);
			}

			if (m_AdvertiseInfo.pAdvScreen[i] != NULL)
			{
				CPicassoRequest		Req;

				m_AdvertiseInfo.pAdvScreen[i]->Initialize();

				// SET FILE NAME
				Req.Set(REQ_SETVALUE, _T("APValue1"), m_AdvertiseInfo.strFileName[i]);
				m_AdvertiseInfo.pAdvScreen[i]->AddRequest(Req);

				m_AdvertiseInfo.pAdvScreen[i]->ProcessRequest();
			}
			
			m_AdvNoticeWelcomeInfo.pScreen[nIndex] = m_AdvertiseInfo.pAdvScreen[i];
			nIndex += 2;
		}
	}

	// Set Notice
	if (m_NoticeInfo.bEnable == TRUE)
	{
		if (m_NoticeInfo.pScreen == NULL)
		{
			strKey = _T("30101");
			//m_PicassoScreens.FindDataOfList(strKey, &m_NoticeInfo.pScreen);
			m_PicassoScreensCMap.Lookup(strKey, m_NoticeInfo.pScreen);
		}

		if (m_NoticeInfo.pScreen != NULL)
		{
			CPicassoRequest		Req;
			CString			strName, strData;

			m_NoticeInfo.pScreen->Initialize();

			// SET FILE NAME
			Req.Set(REQ_SETVALUE, _T("APValue1"), m_NoticeInfo.strMsg[0]);
			m_NoticeInfo.pScreen->AddRequest(Req);

			for (i = 1; i < SUPPORT_NOTICE; i++)
			{
				if (m_NoticeInfo.strMsg[i].GetLength() > 0)
				{
					if (i != 1)
						strData += _T("\n");

					strData += m_NoticeInfo.strMsg[i];
				}
			}

			Req.Set(REQ_SETVALUE, _T("APValue2"), strData);
			m_NoticeInfo.pScreen->AddRequest(Req);

			m_NoticeInfo.pScreen->ProcessRequest();

			m_AdvNoticeWelcomeInfo.pScreen[nIndex] = m_NoticeInfo.pScreen;
			nIndex += 2;
		}
	}

	if (nIndex != 1)
	{
		m_AdvNoticeWelcomeInfo.bEnable = TRUE;
		m_AdvNoticeWelcomeInfo.nCurIndex = 0;
		m_AdvNoticeWelcomeInfo.nMaxIndex = nIndex-2;

		NHUIDBG(DBG_INFO, (_T("bEnable(%d), CurIndex(%d), MaxIndex(%d)\n"), m_AdvNoticeWelcomeInfo.bEnable, 
																		 m_AdvNoticeWelcomeInfo.nCurIndex, 
																		 m_AdvNoticeWelcomeInfo.nMaxIndex));
	}

	NHUIDBG(DBG_INFO, (_T("ADV RefreshTime[%d]\n"), m_AdvertiseInfo.nRefreshTime));

	if (m_AdvertiseInfo.nRefreshTime < 5 || m_AdvertiseInfo.nRefreshTime > 30)
		m_AdvertiseInfo.nRefreshTime = 5;

	if (m_AdvNoticeWelcomeInfo.bEnable)
		::SetTimer(m_hDisplayWnd, UM_ADV_SHOW_TIME, 100, NULL);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: AdvertiseDeinitialize()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoCtrl::AdvertiseDeinitialize()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	int i;

	if (m_pCurrentScreen == NULL)
		return;

	if (m_AdvNoticeWelcomeInfo.bEnable == TRUE)
	{
		m_AdvNoticeWelcomeInfo.bEnable = FALSE;
		m_AdvNoticeWelcomeInfo.nCurIndex = 0;
		m_AdvNoticeWelcomeInfo.nMaxIndex = 0;
	}

	// Reset Advertisement Info
	if (m_AdvertiseInfo.nEnableCount > 0)
	{
		for (i = 0; i < SUPPORT_ADVERTISE; i++)
		{
			m_AdvertiseInfo.bEnableAdv[i] = FALSE;
			m_AdvertiseInfo.strFileName[i] = L"";
		}
		 m_AdvertiseInfo.nRefreshTime = 0;
	}

	// Reset Notice Info
	if (m_NoticeInfo.bEnable == TRUE)
	{
		m_NoticeInfo.bEnable = FALSE;

		for (i = 1; i < SUPPORT_NOTICE; i++)
			m_NoticeInfo.strMsg[i] = L"";
	}

	if (_ttoi(m_pCurrentScreen->GetID()) < 30000)
	{
		// Adv Release Memory
		for (int i = 0; i < SUPPORT_ADVERTISE; i++)
		{
			if (m_AdvertiseInfo.pAdvScreen[i] != NULL)
				m_AdvertiseInfo.pAdvScreen[i]->Deinitialize();
		}

		// Notice Release Memroy
		if (m_NoticeInfo.pScreen != NULL)
			m_NoticeInfo.pScreen->Deinitialize();
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: AdvertiseGetNextScreen()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
int  CPicassoCtrl::AdvertiseGetNextScreen()
{
	int nCurIndex;

	nCurIndex = m_AdvNoticeWelcomeInfo.nCurIndex + 1;
	if (nCurIndex > m_AdvNoticeWelcomeInfo.nMaxIndex)
		nCurIndex = 0;

	return nCurIndex;
}