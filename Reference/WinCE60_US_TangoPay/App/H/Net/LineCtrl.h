// LineCtrl.h: interface for the CLineCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LINECTRL_H__583BD1E5_F01B_4DA0_A103_F2FAEFE4CEA4__INCLUDED_)
#define AFX_LINECTRL_H__583BD1E5_F01B_4DA0_A103_F2FAEFE4CEA4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef enum TypeOfLine
{
	LINE_DIALUP=0,
	LINE_TCP,
	LINE_TLS,
	MAX_LINETYPE
};

class CLineCtrl  
{
public:
	CLineCtrl();
	virtual ~CLineCtrl();

	static void			Register(TypeOfLine LineType, CLineCtrl *pLine);
	static CLineCtrl*	Instance(int LineType);

private:
	static CLineCtrl *m_pNetLines[MAX_LINETYPE];

public:
	// 각 LineCtrl 선언 참조 -- 파라미터 의미가 약간 다름.
	virtual int	LineOpen(LPCTSTR Dest_Info, LPCTSTR dest_port= L"", LPCTSTR option=L"", LPCTSTR mode=L"0")=0;
	virtual int	LineClose()=0;
	virtual int	LineSendData(BYTE *send_data, int len)=0;
	// 각 LineCtrl 선언 참조 -- 파라미터 의미가 약간 다름.
	virtual int LineAccept(BYTE opt=0)=0;
//	virtual int LineCallState()=0;
	virtual int LineCallState(int nMode=0)=0;	// [#2075] NH KSK 2011.06.27
	virtual int GetLineType()=0;				// [#RWC6-67] US William 2019.10.22 MoniView TLS

protected:
	static char		m_RecvBuf[2048];
};

#endif // !defined(AFX_LINECTRL_H__583BD1E5_F01B_4DA0_A103_F2FAEFE4CEA4__INCLUDED_)
