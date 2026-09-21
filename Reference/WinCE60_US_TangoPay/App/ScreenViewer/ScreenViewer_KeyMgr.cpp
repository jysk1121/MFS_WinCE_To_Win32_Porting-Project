#include "stdafx.h"
#include "ScreenViewer.h"
#include "ScreenViewerDlg.h"

//
// EVENT HANDLERS
//
void CScreenViewerDlg::OnBnClickedBtnLoadKeymgr()
{
	// TODO: Add your control notification handler code here
	UnloadScreen(SCR_KEYMGR);
	LoadScreen(SCR_KEYMGR);
	(GetDlgItem(IDC_BTN_LOAD_KEYMGR))->SetWindowText(_T("Reload"));
}

void CScreenViewerDlg::OnBnClickedButtonLanguageKeymgr()
{
	// TODO: Add your control notification handler code here
	if (m_nLanguage[SCR_KEYMGR] == 0)
		m_nLanguage[SCR_KEYMGR] = 2;
	else
		m_nLanguage[SCR_KEYMGR] = 0;

	m_btnLang_KeyMgr.SetWindowText(GET_LANG_STRING(m_nLanguage[SCR_KEYMGR]));
	ApplyScreenValues(SCR_KEYMGR);
}

void CScreenViewerDlg::OnBnClickedBtnPrevKeymgr()
{
	// TODO: Add your control notification handler code here
	ShowPrevScreen(SCR_KEYMGR);
}

void CScreenViewerDlg::OnBnClickedBtnNextKeymgr()
{
	// TODO: Add your control notification handler code here
	ShowNextScreen(SCR_KEYMGR);
}

void CScreenViewerDlg::OnBnClickedCheckKeymgr640480()
{
	// TODO: Add your control notification handler code here
	UpdateScreenVisibility(SCR_KEYMGR, RES_640_480);
}

void CScreenViewerDlg::OnBnClickedCheckKeymgr800600()
{
	// TODO: Add your control notification handler code here
	UpdateScreenVisibility(SCR_KEYMGR, RES_800_600);
}

void CScreenViewerDlg::OnBnClickedCheckKeymgr1024600()
{
	// TODO: Add your control notification handler code here
	UpdateScreenVisibility(SCR_KEYMGR, RES_1024_600);
}

void CScreenViewerDlg::OnBnClickedCheckKeymgr1024768()
{
	// TODO: Add your control notification handler code here
	UpdateScreenVisibility(SCR_KEYMGR, RES_1024_768);
}
