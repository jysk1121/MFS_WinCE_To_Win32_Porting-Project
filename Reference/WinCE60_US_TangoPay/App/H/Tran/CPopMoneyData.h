#ifndef __CPOPMONEYDATA_H__
#define __CPOPMONEYDATA_H__

// [#2442] US Justin Popmoney Day Total
#if(APP_POPMONEY)	// (APP_CUSTOM_PAI)	[#2471] US Justin 2017.02.01 Enable Popmoney to All customers.
#define POPMONEY_DISPENSERESULT_FILE		_T("PopMoneyDispenseResult.dat")

class AFX_EXT_CLASS CPopMoneyData
{
public:
	CPopMoneyData();
	virtual ~CPopMoneyData();

	// Day Total Data
	CString m_strPMDayTotalDateTime, m_strPMDayTotalDispCount, m_strPMDayTotalDispAmt;

	void	ResetTotal();
	BOOL	ReadDispenseHistory();
	BOOL	SaveDispenseResult(BOOL bReset=FALSE);
	void	RecodeDispenseResult(long nAmount /* Cent */);
	CString GetDaytotalJournalData();	
};
#endif
// End of [#2442]

#endif //__CPOPMONEYDATA_H__