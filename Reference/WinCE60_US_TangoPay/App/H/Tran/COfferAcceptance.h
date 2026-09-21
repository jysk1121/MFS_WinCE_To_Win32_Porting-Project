#ifndef __COFFERACCEPTANCE_H__
#define __COFFERACCEPTANCE_H__

// [#2540] NH Justin 2018.03.13 Surcharge and DCC Acceptance Report
#define OFFER_SURCHARGE_ACCEPT			0
#define OFFER_SURCHARGE_DECLINE			1
#define OFFER_DCC_ACCEPT				2
#define OFFER_DCC_DECLINE				3

#define OFFER_ACCEPTANCE_RESULT_FILE		_T("OfferAcceptance.dat")
// End of [#2540]

class AFX_EXT_CLASS COfferAcceptance
{
public:
	COfferAcceptance();
	virtual ~COfferAcceptance();

	CString m_strOAStartDateTime;
	CString m_strSurchargeAccept, m_strSurchargeDecline;
	CString m_strDCCAccept, m_strDCCDecline;

	BOOL	ResetAcceptance();
	BOOL	ReadAcceptanceHistory();
	BOOL	SaveAcceptanceResult(BOOL bReset=FALSE);
	void	IncreaseAcceptanceResult(int nType);
	CString GetAcceptanceData();	
};

#endif //__COFFERACCEPTANCE_H__