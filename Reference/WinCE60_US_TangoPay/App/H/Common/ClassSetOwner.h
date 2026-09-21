/////////////////////////////////////////////////////////////////////////////
//	Class Set Owner Header
/////////////////////////////////////////////////////////////////////////////
	m_pOwner		= pOwner;

	if (m_pOwner)
	{
		m_pDevCmn		= m_pOwner->m_pDevCmn;
		m_pTranCmn		= m_pOwner->m_pTranCmn;
//		m_pScrCtrl		= m_pOwner->m_pScrCtrl;
		m_pAdaCtrl		= m_pOwner->m_pAdaCtrl;

		if (m_pOwner->m_pDevCmn)
			m_pNetWork		= m_pOwner->m_pDevCmn->m_pNetWork;
	}
