// DES.cpp: implementation of the CDES class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DES.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDES::CDES()
{

}

CDES::~CDES()
{

}

void CDES::CONV_A_H(BYTE *a_data, BYTE *h_data)
{
	BYTE	Tmp;
	
	for (int i = 0; i<16; i++){
		Tmp = (BYTE)(toupper(a_data[i]));
		if (Tmp >= 'A')
			a_data[i] = (BYTE)(Tmp - 'A' + 10);
	}
	
	for (int j = 0; j<8; j++){
		h_data[j] = (BYTE)(((a_data[j*2] & 0x0f) << 4) | (a_data[2*j+1] & 0x0f));
	}
}

/*-------------------------------------------------------
	CONVERT HEX DATA TO ASCII DATA 8B -> 16B
	H_P ; POINTER FOR HEX DATA
	A_P ; POINTER FOR ASCII DATA
-------------------------------------------------------*/
void CDES::CONV_H_A(BYTE *h_data, BYTE *a_data, int size)
{
	for (int i = 0; i<size; i++){
		a_data[2*i]   = HEX_TBL[(h_data[i] >> 4) & 0x0f];
		a_data[2*i+1] = HEX_TBL[h_data[i] & 0x0f];
	}
}

/*-------------------------------------------------------
	64 BYTE -> 8 BYTE
-------------------------------------------------------*/
void CDES::DES_PACK(BYTE *s_buf, BYTE *d_buf)
{
	int  s_index = 0;
	int  d_index = 0;
	BYTE k;
	
	for (int i = 0; i<8; i++){
		k = 0;
		for (int j=0; j<8; j++){
			k = (BYTE)((k<<1) + s_buf[s_index]);
			s_index++;
		}
		d_buf[d_index] = k;
		d_index++;
	}
}

/*-------------------------------------------------------
	8 BYTE -> 64 BYTE
-------------------------------------------------------*/
void CDES::DES_UNPACK(BYTE *s_buf, BYTE *d_buf)
{
	int s_index = 0;
	int d_index = 0;
	BYTE	k;
	
	for (int i=0; i<8; i++){
		k = s_buf[s_index];
		s_index++;
		for (int j=0; j<8; j++){
			d_buf[d_index] = (BYTE)((k >> (7-j)) & 0x01);
			d_index++;
		}
	}
}

/*-------------------------------------------------------
	GENERATE KEY SCHEDULE
-------------------------------------------------------*/
void CDES::DES_SETKEY(BYTE d_flag, BYTE *k_p)
{
	BYTE	T1, T2;
	int t;
	int i = 0, j = 0, k = 0;
	
	DES_UNPACK(k_p, DES_KS);
	for (i=0; i< 56; i++)
		DES_BLOCK[i] = DES_KS[DES_PC1[i]-1];
	
	for (i=0; i<16; i++){
		for (j=0; j<DES_SHIFTS[i];j++){
			T1 = DES_BLOCK[0];
			T2 = DES_BLOCK[28];
			for (k=0; k<27; k++){
				DES_BLOCK[k] = DES_BLOCK[k + 1];
				DES_BLOCK[k + 28] = DES_BLOCK[k + 29];
			}
			DES_BLOCK[27] = T1;
			DES_BLOCK[55] = T2;
		}
		if (d_flag)
			j = 15-i;
		else j = i;
		t = j*48;
		for (k=0; k<48; k++)
			DES_KS[t + k] = DES_BLOCK[DES_PC2[k] - 1];
	}
}

/*-------------------------------------------------------
	CONVERT DATA
-------------------------------------------------------*/
void CDES::DES_CONVERT(BYTE *s_p, BYTE *d_p)
{
	BYTE LR[64];
	BYTE F[32];
	int T,ii,k, j;
	
	DES_UNPACK(s_p, DES_BLOCK);
	for (int j=0; j<64; j++)
		LR[j] = DES_BLOCK[DES_IPER[j] -1];
	for (int i = 0; i<16; i++){
		T = i*48;
		for (j=0; j<48; j++)
			DES_BLOCK[j] = uchar(LR[DES_E[j] + 31] ^ DES_KS[T + j]);
		ii = 0;
		for (j=0; j<8; j++){
			k = 6*j;
			T = DES_BLOCK[k];
			T = (T<<1) | DES_BLOCK[k+5];
			T = (T<<1) | DES_BLOCK[k+1];
			T = (T<<1) | DES_BLOCK[k+2];
			T = (T<<1) | DES_BLOCK[k+3];
			T = (T<<1) | DES_BLOCK[k+4];
			
			T = DES_S[ii+T];
			k = 4*j;
			
			F[k]     = (BYTE)((T>> 3) & 1);
			F[k + 1] = (BYTE)((T>> 2) & 1);
			F[k + 2] = (BYTE)((T>> 1) & 1);
			F[k + 3] = (BYTE)(T & 1);
			ii = ii + 64;
		}
		for (j=0; j<32;j++){
			T = LR[j+32];
			LR[j+32] = (BYTE)(LR[j] ^ F[DES_PER[j] -1]);
			LR[j] = (BYTE)T;
		}
	}
	for (j=0; j<64; j++)
		DES_BLOCK[j] = LR[DES_RFP[j] - 1];
	DES_PACK(DES_BLOCK, d_p);
}

/*-------------------------------------------------------
	DES ENCRYPT
-------------------------------------------------------*/
void CDES::ENCRYPT(BYTE *s_p, BYTE *d_p, BYTE *k_p)
{
	DES_SETKEY(FALSE, k_p);
	DES_CONVERT(s_p, d_p);
}

/*-------------------------------------------------------
	DES DECRYPT
-------------------------------------------------------*/
void CDES::DECRYPT(BYTE *s_p, BYTE *d_p, BYTE *k_p)
{
	DES_SETKEY(TRUE, k_p);
	DES_CONVERT(s_p, d_p);
}

int CDES::FIND_POS(BYTE *buff, BYTE value, int len)
{
	for (int i=0; i<len; i++){
		if (buff[i] == value)
			return i;
	}
	return 0xffff;
}
