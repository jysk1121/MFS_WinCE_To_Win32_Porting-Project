// DES.h: interface for the CDES class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DES_H__2C5550EC_A8CF_4470_BC9F_6BF0C5A512D0__INCLUDED_)
#define AFX_DES_H__2C5550EC_A8CF_4470_BC9F_6BF0C5A512D0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DesApi.h"

class CDES  
{
public:
	int FIND_POS(BYTE* buff, BYTE  value, int len);
	void DECRYPT(BYTE* s_p, BYTE* d_p, BYTE* k_p);
	void ENCRYPT(BYTE* s_p, BYTE* d_p, BYTE* k_p);
	void DES_CONVERT(BYTE* s_p, BYTE* d_p);
	void DES_SETKEY(BYTE d_flag, BYTE* k_p);
	void DES_UNPACK(BYTE* s_buf, BYTE* d_buf);
	void DES_PACK(BYTE* s_buf, BYTE* d_buf);
	void CONV_H_A(BYTE* h_data, BYTE* a_data, int size);
	void CONV_A_H(BYTE* a_data, BYTE* h_data);
	
	CDES();
	virtual ~CDES();
};

#endif // !defined(AFX_DES_H__2C5550EC_A8CF_4470_BC9F_6BF0C5A512D0__INCLUDED_)
