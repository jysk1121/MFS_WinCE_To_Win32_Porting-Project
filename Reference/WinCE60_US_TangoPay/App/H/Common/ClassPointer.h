/////////////////////////////////////////////////////////////////////////////
//	Class Pointer Header
/////////////////////////////////////////////////////////////////////////////
	CMainFrame*			m_pOwner;
//	CScrCtrl*			m_pScrCtrl;
#if (_WIN32_WCE < 0x600)
	CADACtrlMain*		m_pAdaCtrl;
#else
	CAdaCtrl*			m_pAdaCtrl;
#endif
	CDevCmn*			m_pDevCmn;
	CTranCmn*			m_pTranCmn;
	CNetWork*			m_pNetWork;
