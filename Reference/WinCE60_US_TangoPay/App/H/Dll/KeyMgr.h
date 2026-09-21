#ifndef __NH_KEY_MGR_H__
#define __NH_KEY_MGR_H__

#ifndef _KEYMGR_OWNER_
#pragma comment(lib, "KeyMgr.lib")
#endif

#include "KeyMgrStateManager.h"
#include ".\Scr\ScrCtrl.h"

class AFX_CLASS_EXPORT CKeyMgr
{
public:
	CKeyMgr();
	~CKeyMgr();

	/*	METHOD */
public:

	/*	ATTRIBUTE */
public:
	//void	StartKeyMgr(CScrCtrl *pScrCtrl);
	CString	StartKeyMgr(CScrCtrl *pScrCtrl);
	int	SetLanguage(int nLangKind);

private:
	CKeyMgrStateManager	m_KeyStateMgr;
};

#endif //__NH_KEY_MGR_H__