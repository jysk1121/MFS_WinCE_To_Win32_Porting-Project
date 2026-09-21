#include "stdafx.h"
#include ".\Common\CmnLib.h"

//#include ".\Common\NHDbgApi.h"	// [RWC6-676] SKKim 2024.05.22

/////////////////////////////////////////////////
//
// 명령재사용관련
//
char* WINAPI Strcpy(void* dBuffc, void* sBuffc)
{
	return strcpy((char*)dBuffc, (char*)sBuffc);
}

char* WINAPI Strcpy(void* dBuffc, LPCTSTR sBuffc)
{
	char szSource[2048] = {};
	WideToMulti(szSource, sBuffc, sizeof(szSource));

	return strcpy((char*)dBuffc, (char*)szSource);
}

char* WINAPI Strcpy(void* dBuffc, void* sBuffc, int slen)
{
	return strncpy((char*)dBuffc, (char*)sBuffc, slen);
}

char*	WINAPI Strcpy(void* dBuffc, LPCTSTR sBuffc, int slen)
{
	char szSource[2048] = {};
	WideToMulti(szSource, sBuffc, sizeof(szSource));

	return strncpy((char*)dBuffc, szSource, slen);
}

char*	WINAPI StrcpyRight(void* dBuffc, void* sBuffc, int slen, int dlen, char InitChar)
{
	char*	dBuff = (char*)dBuffc;
	char*	sBuff = (char*)sBuffc;

	memset(dBuff, InitChar, dlen);
	if (dlen >= slen)
		memcpy(&dBuff[dlen - slen], sBuff, slen);
	else
		memcpy(dBuff, &sBuff[slen - dlen], dlen);
	return dBuff;
}

char*	WINAPI StrcpyRight(void* dBuffc, LPCTSTR sBuffc, int slen, int dlen, char InitChar)
{
	char szSource[2048] = {};
	WideToMulti(szSource, sBuffc, sizeof(szSource));
	
	return StrcpyRight(dBuffc, szSource, slen, dlen, InitChar);
}

LPCTSTR WINAPI Strcpy(CString& dBuffc, void* sBuffc)
{
	dBuffc.Format(_T("%S"), sBuffc);
	return dBuffc;
}

LPCTSTR WINAPI Strcpy(CString& dBuffc, void* sBuffc, int slen)
{
	TCHAR	buf[256] = {};
	swprintf(buf, _T("%%%d.%ds"), slen);

	dBuffc.Format(buf, sBuffc);
	return dBuffc;
}

int WINAPI Strlen(void* sBuffc)
{
	return strlen((char*)sBuffc);
}

void WINAPI AppendFormatCenter(CString& dest, int width, LPCTSTR format, ...)
{
	CString temp;
	CString centeredFormatString;
	int tempWidth = 0;
	int leftSpaces = 0;
	int rightSpaces = 0;

	va_list args;
	va_start(args, format);
	temp.FormatV(format, args);
	va_end(args);

	tempWidth = temp.GetLength();

	if (tempWidth >= width)
	{
		// If it can't fit inside the defined width, just append the data
		dest.Append(temp);
		return;
	}

	leftSpaces = (width - tempWidth) / 2;
	rightSpaces = width - (leftSpaces + tempWidth);

	// Evaluates to "%-xs%s%ys", where x is leftSpaces and y is rightSpaces
	centeredFormatString.Format(L"%%-%ds%%s%%%ds", leftSpaces, rightSpaces);
	dest.AppendFormat(centeredFormatString, L"", temp, L"");
}

CString WINAPI CenterLines(CString data, int width, CString sep)
{
	CString curLine;
	CString result;
	int index = 0, start = 0;

	while ((index = data.Find(sep, start)) != -1)
	{
		curLine = data.Mid(start, index - start);
		AppendFormatCenter(result, width, L"%s", curLine);
		result.Append(sep);
		start = index + sep.GetLength();
	}

	if (start < data.GetLength())
	{
		curLine = data.Mid(start);
		AppendFormatCenter(result, width, L"%s", curLine);
		result.Append(sep);
	}

	return result;
}

////////////////////////////////////////
//
//	Asc type to Integer type
//  Ex) "123456" -> 123456
//
int WINAPI Asc2Int(void* lpvoid, int length)
{
    int		ivalue=0,bminus=FALSE;
    int		i;
	char*	asc = (char* )lpvoid;

	if (!length)
		return 0;

    for(i=0;i<length;i++)
    {
        if(asc[i]>='0' && asc[i]<='9')
        {
            ivalue = (ivalue*10) + (asc[i]-'0');
            continue;
        }
        if(asc[i]=='-') bminus = TRUE;
    }

    return (bminus) ? 0-ivalue : ivalue;
}

int WINAPI Asc2Int(LPCTSTR lpvoid, int length)
{
	char szTemp[128] = {};
	WideToMulti(szTemp, lpvoid, sizeof(szTemp));
		
	return (Asc2Int((void*)szTemp, strlen(szTemp)));
}

int WINAPI Asc2Int(LPCTSTR lpvoid)
{
	char szTemp[128] = {};
	WideToMulti(szTemp, lpvoid, sizeof(szTemp));

	return (Asc2Int((void*)szTemp, strlen(szTemp)));
}

// [#2317] US Justin 2015.01.05 Add Long integer Conversion.
long WINAPI Asc2Long(void* lpvoid, int length)
{
    long ivalue=0;
	bool bminus=FALSE;
    int		i;

	char*	asc = (char* )lpvoid;

	if (!length)
		return 0;

    for(i=0;i<length;i++)
    {
        if(asc[i]>='0' && asc[i]<='9')
        {
            ivalue = (ivalue*10) + (asc[i]-'0');
        	continue;
        }
        if(asc[i]=='-') 
			bminus = TRUE;
    }

    return (bminus) ? 0-ivalue : ivalue;
}

long WINAPI Asc2Long(LPCTSTR lpvoid, int length)
{
	char szTemp[128] = {};
	WideToMulti(szTemp, lpvoid, sizeof(szTemp));
		
	return (Asc2Long((void*)szTemp, strlen(szTemp)));
}

long WINAPI Asc2Long(LPCTSTR lpvoid)
{
	char szTemp[128] = {};
	WideToMulti(szTemp, lpvoid, sizeof(szTemp));

	return (Asc2Long((void*)szTemp, strlen(szTemp)));
}
// End of [#2317]

long long WINAPI Asc2LongLong(void* lpvoid, int length)
{
    long long ivalue=0;
	bool bminus=FALSE;
    int		i;

	char*	asc = (char* )lpvoid;

	if (!length)
		return 0;

    for(i=0;i<length;i++)
    {
        if(asc[i]>='0' && asc[i]<='9')
        {
            ivalue = (ivalue*10) + (asc[i]-'0');
        	continue;
        }
        if(asc[i]=='-') 
			bminus = TRUE;
    }

    return (bminus) ? 0-ivalue : ivalue;
}

long long WINAPI Asc2LongLong(LPCTSTR lpvoid, int length)
{
	char szTemp[128] = {};
	WideToMulti(szTemp, lpvoid, sizeof(szTemp));
		
	return (Asc2LongLong((void*)szTemp, strlen(szTemp)));
}

long long WINAPI Asc2LongLong(LPCTSTR lpvoid)
{
	char szTemp[128] = {};
	WideToMulti(szTemp, lpvoid, sizeof(szTemp));

	return (Asc2LongLong((void*)szTemp, strlen(szTemp)));
}

int	WINAPI Asc2IntHighOrder(LPCTSTR lpvoid, void* kBuffc, int inLen, int outLen)
{
	unsigned char*	kBuff = (unsigned char*)kBuffc;
	int				ivalue = Asc2Int(lpvoid, wcslen(lpvoid));

    unsigned char*	lpValue =(unsigned char *)&ivalue;
	int				k = 0;
	
	memset(kBuff, 0, outLen);

	for (int i = 0; i < outLen; i++)
	{
		k = outLen - i - 1;
		if (k < 4)
			kBuff[i] = lpValue[k];
		else
			kBuff[i] = 0x00;
	}

	return ivalue;
}

////////////////////////////////////////
//
//	Asc type to Integer type
//  Ex) "123456" -> 123456
//
double WINAPI Asc2Float(void* lpvoid, int length)
{
	double	ivalue=0;
	int		i;
	char*	asc = (char* )lpvoid;

	if (!length)
		return 0.;

	for(i=0;i<length;i++)
	{
		if(asc[i]>='0' && asc[i]<='9')
		{
			ivalue = (ivalue*10.) + (double)(asc[i]-'0');
		}
	}
	return ivalue;
}

double WINAPI Asc2Float(LPCTSTR lpvoid, int length)
{
	char szTemp[128] = {};
	WideToMulti(szTemp, lpvoid, sizeof(szTemp));
	
	return (Asc2Float((void*)szTemp, length));
}

double WINAPI Asc2Float(LPCTSTR lpvoid)
{
	char szTemp[128] = {};
	WideToMulti(szTemp, lpvoid, sizeof(szTemp));
		
	return Asc2Float((void*)szTemp, strlen(szTemp));
}

////////////////////////////////////////
//	Ex) "00123456" -> "  123456"
CString WINAPI Asc2String(void* lpvoid, int inLen, int outLen)
{
	char	str[1024] = {};
	char	str2[1024] = {};
	CString	cstrstr("");

	memcpy(str,lpvoid,inLen);
	
	for(int i = 0; i < inLen; i++)
	{
		if (str[i] == '.')
		{
			if (i)	
				str[i - 1] = '0';
			break;
		}

		if (str[i] != '0')	
			break;

		str[i] = ' ';
	}

	StrcpyRight(str2, str, inLen, outLen, ' ');
	Strcpy(cstrstr, str2);
	return cstrstr;
}

/////////////////////////////////////////////////
//
// Warning - Target Buffer의 크기가 
//           Conversion한 Data의 크기 보다 작다면
//           Error가 발생할 것이다.
// Ex) 340 -> "340"
//
char* WINAPI Int2Asc(int Value, void* Target, int len, int mode, char InitChar)
{
// mode : decimal,hex, binary..
	char	tmp[1024] = {};
    char	*Dst  = (char* )Target;
	int		slen;

	_itoa(Value, tmp, mode);
	Lower2Upper(tmp, strlen(tmp));
	memset(Dst, InitChar, len);
	slen = strlen(tmp);
	if (len >= slen)
		memcpy(&Dst[len-slen], tmp, slen);
	else memcpy(Dst, &tmp[slen-len], len);
	return ((char*)Target);
}

CString WINAPI Int2Asc(int Value, int len, int mode, char InitChar)
{
	CString	cstrstr("");
    char buf[1024] = {};
	if (len)
		Int2Asc(Value, buf, len, mode, InitChar);
	else														// len=0이면 그대로
	{
		_itoa(Value, buf, mode);
		Lower2Upper(buf, strlen(buf));
	}

	cstrstr = CString(buf);

	return cstrstr;
}

// [#2022] NH KSK 2011.02.22
/////////////////////////////////////////////////
//
// Warning - Target Buffer의 크기가 
//           Conversion한 Data의 크기 보다 작다면
//           Error가 발생할 것이다.
// Ex) 340 -> "340", 0x80000000 -> "2147483648"
//
// char * WINAPI ULong2Asc(ULONG Value, void *Target, int len, int mode, char InitChar)
// {
// // mode : decimal,hex, binary..
// 	char	tmp[256];
// 	char    *Dst  = (char *)Target;
// 	int		slen;
// 
// 	memset(tmp, 0x00, sizeof(tmp));
// 	_ultoa(Value, tmp, mode);
// 
// 	Lower2Upper(tmp, strlen(tmp));
// 	memset(Dst, InitChar, len);
// 	slen = strlen(tmp);
// 	
// 	if (len >= slen)
// 		memcpy(&Dst[len-slen], tmp, slen);
// 	else 
// 		memcpy(Dst, &tmp[slen-len], len);
// 
// 	return ((char*)Target);
// }
// end of [#2022]

////////////////////////////////////////
int	WINAPI SplitString(CString cstrSrc, CString cstrSplit, CStringArray& acstrRet)
{
	CString cstrTemp = cstrSrc;
	CString cstrData("");
	int		nIndex=0, nCnt=0;
	int		nlen=0, nsplitlen=0; 

	acstrRet.RemoveAll();										// 20030703 : 초기화하지 않으면 원래배열의 뒤에 추가됨
	
	while (TRUE)												// 20031030 : 무조건처리로변경
	{
		nIndex = cstrTemp.Find(cstrSplit);
		if(nIndex == -1)
		{
			acstrRet.Add(cstrTemp);								// 20031030 : 무조건처리로변경
			nCnt++;												// 20031030 : 마지막처리추가
			break;
		}
		else
		{
			cstrData = cstrTemp.Left(nIndex);

			// [RWC6-676] start SKKim 2024.05.22 1KB이상 data 처리시 버그 수정
			#ifdef UNDER_CE
			nlen = WideCharToMultiByte(CP_ACP, 0, cstrTemp, -1, NULL, NULL, NULL, NULL);
			nsplitlen = WideCharToMultiByte(CP_ACP, 0, cstrSplit, -1, NULL, NULL, NULL, NULL);
			cstrTemp = cstrTemp.Right(nlen - (nIndex + nsplitlen));  
			#else
			cstrTemp = cstrTemp.Right(cstrTemp.GetLength() - (nIndex + cstrSplit.GetLength()));  
			#endif
			// [RWC6-676] end SKKim 2024.05.22

			acstrRet.Add(cstrData);
			nCnt++;
		}
	}
	return nCnt;
}

////////////////////////////////////////
// 한글절사
CString WINAPI GetString(void* inbuff,int inLen,int outLen)
{
	BYTE	str[1024] = {};
	CString	cstrstr("");
	BYTE	strtemp[1024] = {};

    BYTE    *instr  = (BYTE* )inbuff;
	int		i;

	if (inbuff == str)											// 재귀호출처리
	{
		memcpy(strtemp, inbuff, sizeof(strtemp));
		instr = strtemp;
	}

	if(inLen <= outLen)
	{
		memset(str,0x20,outLen);
		outLen = inLen;
	}

	for (i=0;i<outLen;i++)
	{
		if (instr[i] & 0x80)									// hangul
		{
			if (i+1 < outLen)
			{
				if (instr[i+1] & 0x80)							// hangul garbage clear
				{
					str[i] = instr[i];	i++;
					str[i] = instr[i];
				}
				else
				{
					str[i] = ' ';	i++;
					str[i] = instr[i];
				}
			}
			else
			{
				str[i] = ' ';									// hangul left clear
			}
		}
		else
		{
			if (instr[i] < ' ')									// spec. clear
				str[i] = ' ';
			else 
				str[i] = instr[i];
		}
	}

	Strcpy(cstrstr, str);
	return cstrstr;
}

//////////////////////////////////////////
//
//  Ex1) "000100" -> return index 3
//  Ex2) "    20" -> return index 4
//
int WINAPI Compress(void* sBuff, int slen)
{
	int i;
	BYTE	*bsBuff = (BYTE *)sBuff;

	for (i = 0; i < slen; i++)
	{
		if ((bsBuff[i] != 0x30) && (bsBuff[i] != 0x20) && (bsBuff[i] != 0x00))
			break;
	}

	// if all data are 0x30, 0x20, 0x00
	if (i == slen)
		return 0xff;
	return i;
}

int WINAPI MarkAmount(void* inbuff,int inLen, char Value)
{
    char    *instr  = (char* )inbuff;
	int i;
	for(i=inLen-1;i>=0;i--)
	{
		if(instr[i]==' ')
		{
			instr[i]=Value;
			break;
		}
	}
	return i;
}

int WINAPI EditString(void* inBufc, int inLen, char* fmt, void* outBufc, char MoveChar, char HiddenChar, char HidenMoveChar, int HiddenFlg)
{
	char*	inBuf = (char*)inBufc;
	char*	outBuf = (char*)outBufc;
	int	flen = strlen(fmt);
	int i,j=0;

	memset(outBuf,' ',flen);
	for(i=0;i<flen;i++)
	{
		if (j == inLen)
			break;

		if (fmt[i] == MoveChar)
			outBuf[i] = inBuf[j++];
		else 
		if ((HiddenFlg) &&
			(fmt[i] == HiddenChar))
			{
				outBuf[i] = HidenMoveChar;
				j++;
			}
		else
		if ((!HiddenFlg) &&
			(fmt[i] == HiddenChar))
			outBuf[i] = inBuf[j++];
		else outBuf[i] = fmt[i];	
	}
	return i;
}

int	WINAPI EditString(void* inBufc, int inLen, char* fmt, CString& outBufc, char MoveChar, char HiddenChar, char HidenMoveChar, int HiddenFlg)
{
    char buf[1024] = {};

	int i = EditString(inBufc, inLen, fmt, buf, MoveChar, HiddenChar, HidenMoveChar, HiddenFlg);
	Strcpy(outBufc, buf);
	return i;
}

int	WINAPI EditString(LPCTSTR inBufc, char* fmt, CString& outBufc, char MoveChar, char HiddenChar, char HidenMoveChar, int HiddenFlg)
{
    char buf[1024] = {};
	char szSource[2048] = {};

	WideToMulti(szSource, inBufc, sizeof(szSource));

	int i = EditString(szSource, strlen(szSource), fmt, buf, MoveChar, HiddenChar, HidenMoveChar, HiddenFlg);
	Strcpy(outBufc, buf);
	return i;
}

CString	WINAPI EditString(void* inBufc, int inLen, char* fmt, char MoveChar, char HiddenChar, char HidenMoveChar, int HiddenFlg)
{
	CString	cstrstr("");

	EditString(inBufc, inLen, fmt, cstrstr, MoveChar, HiddenChar, HidenMoveChar, HiddenFlg);
	return cstrstr;
}

CString	WINAPI EditString(LPCTSTR inBufc, char* fmt, char MoveChar, char HiddenChar, char HidenMoveChar, int HiddenFlg)
{
	CString	cstrstr("");

	char szSource[2048] = {};
	WideToMulti(szSource, inBufc, sizeof(szSource));

	EditString(szSource, strlen(szSource), fmt, cstrstr, MoveChar, HiddenChar, HidenMoveChar, HiddenFlg);
	return cstrstr;
}

// [#2022] NH KSK 2011.02.22
//int WINAPI Sprintf(void* dst,int len,char* fmt,...)
// {
//     char buf[1024];
//     memset(buf, 0x00, sizeof(buf));
//     int ret = vsprintf((LPSTR)buf, (LPSTR)fmt, (va_list)(&fmt+1));
// 
// 	if(ret>len)
// 	{
// 		memcpy(dst,buf,len);
// 	}
// 	else
// 	{
// 		memcpy(dst,buf,ret);
// 	}
// 
// 	return ret;
// }
// end of [#2022]

// KSK 2012.03.01 Code Snonar 지적사항 대책
//CString WINAPI GetSprintf(char* fmt,...)
//{
//	CString	cstrstr;
//	char buf[1024] = { 0, };
//
//	vsprintf((LPSTR)buf, (LPSTR)fmt, (va_list)(&fmt+1));
//	Strcpy(cstrstr, buf);
//
//	return cstrstr;
//}

int WINAPI FindChar(void* inbuf,int len, char chr)
{
    char    *instr  = (char* )inbuf;

	for(int i=0;i<len;i++)	
	{
		if((instr[i]==chr) ||
		   (instr[i]==0x00))
			return i;
	}
	return len;
}

int WINAPI FindNotNum(void* inbuf,int len)
{
    char    *instr  = (char* )inbuf;

	for(int i=0;i<len;i++)	
	{
		if((instr[i]<'0') ||
		   (instr[i]>'9') ||
		   (instr[i]==0x00))
			return i;
	}
	return len;
}

int WINAPI IsChar(void* inbuf,int len, char chr)
{
    char    *instr  = (char* )inbuf;

	for(int i=0;i<len;i++)	if(instr[i]!=chr)	return FALSE;
	return TRUE;
}

int WINAPI IsSpace(void* inbuf,int len)
{
    char    *instr  = (char* )inbuf;

	for(int i=0;i<len;i++)	if(instr[i]!=' ')	return FALSE;
	return TRUE;
}

int WINAPI IsZero(void* inbuf,int len)
{
    char    *instr  = (char* )inbuf;

	for(int i=0;i<len;i++)	if(instr[i]!='0')	return FALSE;
	return TRUE;
}

int WINAPI IsZero(CString strString)
{
	int i;
	char ch;

	for (i = 0; i < strString.GetLength(); i++)
	{
		ch = (char)strString.GetAt(i);
		if (ch != '0')
			return FALSE;
	}

	return TRUE;	
}

int WINAPI IsNum(void* inbuf,int len)
{
    char    *instr  = (char* )inbuf;

	// KSK 2010.02.05 보완 처리 (Len이 0보다 작은 경우 FALSE 처리)
	if (len <= 0)	return FALSE;

	for(int i=0;i<len;i++)
	{
		if((instr[i]<'0') || (instr[i]>'9'))	return FALSE;
	}

	return TRUE;
}

int WINAPI IsNum(CString strInbuf)
{
	if (strInbuf.GetLength() <= 0)		return FALSE;

	for(int i=0; i<strInbuf.GetLength(); i++)
	{
		if((strInbuf.GetAt(i)<'0') || (strInbuf.GetAt(i)>'9'))	return FALSE;
	}

	return TRUE;
}

int WINAPI IsUnPack(void* inbuf,int len)
{
    char    *instr  = (char* )inbuf;

	for(int i=0;i<len;i++)	if((instr[i]<0x30) || (instr[i]>0x3f))	return FALSE;
	return TRUE;
}

int WINAPI IsNull(void *inbuf,int len)
{
    char    *instr  = (char *)inbuf;

	for(int i=0;i<len;i++)	if(instr[i]!=NULL)	return FALSE;
	return TRUE;
}

// '000103' => '   103'
int WINAPI Zero2Space(void* sBuff, int slen)
{
	int ret = Compress(sBuff, slen);

	if (ret != 0xff)
		memset(sBuff, ' ', ret);
	else memset(sBuff, ' ', slen);
	return ret;
}

// 'abc' => 'ABC'
int WINAPI Lower2Upper(void* sBuff, int slen)
{
	BYTE	*bsBuff = (BYTE *)sBuff;

	for (int i=0;i<slen;i++) if((bsBuff[i]>='a') && (bsBuff[i]<='z')) bsBuff[i] &= 0xdf;
	return TRUE;
}

// 'ABC' => 'abc'
int WINAPI Upper2Lower(void* sBuff, int slen)
{
	BYTE	*bsBuff = (BYTE *)sBuff;

	for (int i=0;i<slen;i++) if((bsBuff[i]>='A') && (bsBuff[i]<='Z')) bsBuff[i] |= 0x20;
	return TRUE;
}

int WINAPI LTrim(void *inbuf, int len, int ch)
{
	int	i,j;
	BYTE*	binbuf = (BYTE*)inbuf;
	char	*tbuff;

	tbuff = new char [len + 1]();

	j = 0;

	for(i=0; i < len; i++)
	{
		if (binbuf[i] != ch)	tbuff[j++] = binbuf[i];
	}

	memcpy(binbuf, tbuff, j);
	binbuf[j] = 0x00;

	delete [] tbuff;				// 2008.03.05 JSW

	return j;
}

//////////////////////////////////////////
//	A Byte type to a Ascii string
//  Ex) 0x1f -> "1f"
void WINAPI Byte2Asc(BYTE byteptr, void* strptrc, int mode)
{
	char*	strptr = (char*)strptrc;

	if ((mode == 8) || (mode == 10))
		Int2Asc((int)byteptr, strptr, 3, mode, '0');
	else if (mode == 16)
		Int2Asc((int)byteptr, strptr, 2, mode, '0');
	else Int2Asc((int)byteptr, strptr, 8, mode, '0');
}

CString WINAPI Byte2Asc(BYTE byteptr, int mode)
{
	CString	cstrstr("");
	char buf[1024] = {};
    Byte2Asc(byteptr, buf, mode);

	Strcpy(cstrstr, buf);
	return cstrstr;
}

void WINAPI Byte2Ascn(BYTE *byteptr, void* strptrc, int blen, int mode)
{
	char*	strptr = (char*)strptrc;

	for (int i = 0; i < blen ; i++)
	{
		Byte2Asc(*byteptr, strptr, mode);
		byteptr++;
		
		if ((mode == 8) || (mode == 10))
			strptr += 3;
		else if (mode == 16)
			strptr += 2;
		else strptr += 8;
	}
}

CString WINAPI Byte2Ascn(BYTE *byteptr, int blen, int mode)
{
	CString	cstrstr("");
	char buf[STRBUFFSIZE] = {};
    Byte2Ascn(byteptr, buf, blen, mode);

	Strcpy(cstrstr, buf);
	return cstrstr;
}

// 12 34 56 78 => 78 56 34 12 => int로 변경함
int WINAPI ByteHighOrder2Int(BYTE* Src, int inLen)
{
	int m_int = 0;

	for (int i = 0; i < inLen; i++)
	{
		m_int = m_int << 8;
		m_int |= Src[i];
	}

	return m_int;
}

void WINAPI Asc2Byte(void* ascptrc, BYTE *byteptr)
{
	char*	ascptr = (char*)ascptrc;
	BYTE	tmp[2] ;

	if (*(ascptr) > '9')
	{
		if (*(ascptr) < 'a')
			tmp[0] = ((((BYTE)(*(ascptr))) - 0x37) << 4);
		else 
			tmp[0] = ((((BYTE)(*(ascptr))) - 0x57) << 4);
	}
	else 
		tmp[0] = ((((BYTE)(*(ascptr))) - '0') << 4);
	ascptr++;

	if (*(ascptr) > '9')
	{
		if (*(ascptr) < 'a')
			tmp[1] = (((BYTE)(*(ascptr))) - 0x37);
		else 
			tmp[1] = (((BYTE)(*(ascptr))) - 0x57);
	}
	else 
		tmp[1] = (((BYTE)(*(ascptr))) - '0');
	//ascptr++;			// [CS#5] NH AIREAT 2008.3.14

	*byteptr = tmp[0] | tmp[1]; 
}

////////////////////////////////////////////////////////////////////////////
//	2 Digit Ascii type String to BYTE type string
//	Ex) 2 Digit Asc type "a23376f7a5" -> BYTE 0xa2,0x33,0x76,0xf7,0xa5 conversion
void WINAPI Asc2Byten(void* ascptrc, BYTE *byteptr, int blen)
{
	char*	ascptr = (char*)ascptrc;

	for (int i = 0; i < blen ; i++)
	{
		Asc2Byte(ascptr, byteptr);
		ascptr += 2;
		byteptr++;
	}
}

BYTE WINAPI MakeBcc(void* asc, int length)
{
	BYTE	*basc = (BYTE *)asc;
	int i = 1;

	BYTE cc = basc[0];
	while (i < length)
		cc ^= basc[i++];
	return (cc);
}

int WINAPI MakePack(void* Src, void* Dest, int Slen, char OperMode, int LowNibbleFlg)
{
	BYTE	*bSrc = (BYTE *)Src;
	BYTE	*bDest = (BYTE *)Dest;
	BYTE	Value = 0;
	int		i=0, j=0;//, jSave = -1;

	bDest[0] = 0;	// [CS#28] NH AIREAT 2008.3.17  2008.3.14버전 버그수정.

	for(i = 0; i < Slen; i++)
	{
		if ((bSrc[i] >= 'a') && (bSrc[i] <= 'f'))
			Value = ((bSrc[i] - 0x57) & 0x0f);
		else
		if ((bSrc[i] >= 'A') && (bSrc[i] <= 'F'))
			Value = ((bSrc[i] - 0x37) & 0x0f);
		else
			Value = (bSrc[i] & 0x0f);

// [CS#28] NH AIREAT 2008.3.14
// 버그가 있어 조건문과 jSave추가 했으나, 성능향상을 위해 삭제하고 초기에 한번에 0으로 셋팅하도록 수정.
// 		if(j != jSave && OperMode != '|' && OperMode != '^')
// 		{
// 			bDest[j] = 0;
// 			jSave = j;
// 		}
// end of [CS#28]

		if (!LowNibbleFlg)
		{
			if (!(i & 1))
			{
				if (OperMode == '|')
					bDest[j] |= (Value << 4);
				else 
				if (OperMode == '^')
					bDest[j] ^= (Value << 4);
				else
					bDest[j] = (bDest[j] & 0x0f) | (Value << 4);
			}
			else
			{
				if (OperMode == '|')
					bDest[j] |= (Value);
				else 
				if (OperMode == '^')
					bDest[j] ^= (Value);
				else
					bDest[j] = (bDest[j] & 0xf0) | (Value);
				j++;
			}
		}
		else
		{
			if (i & 1)
			{
				if (OperMode == '|')
					bDest[j] |= (Value << 4);
				else 
				if (OperMode == '^')
					bDest[j] ^= (Value << 4);
				else
					bDest[j] = (bDest[j] & 0x0f) | (Value << 4);
			}
			else
			{
				if (OperMode == '|')
					bDest[j] |= (Value);
				else 
				if (OperMode == '^')
					bDest[j] ^= (Value);
				else
					bDest[j] = (bDest[j] & 0xf0) | (Value);
				j++;
			}
		}
	}
	return j;
}

// [#2001] NH KSK 2010.11.15 함수 삭제
// int WINAPI MakeUnPack(void* Src, void* Dest, int Slen, int AscFlg)
// {
// 	BYTE	*bSrc = (BYTE *)Src;
// 	BYTE	*bDest = (BYTE *)Dest;
// 	int		i=0, j=0;
// 
// 	for(i = 0; i < Slen; i++)
// 	{
// 		if (!AscFlg)											// 0x30 - 0x3f
// 		{
// 			bDest[j++] = ((bSrc[i] & 0xf0) >> 4) | '0';
// 			bDest[j++] = (bSrc[i] & 0x0f) | '0';
// 		}
// 		else													// 0x30 - 0x39, 0x41 - 0x46
// 		{	
// 			bDest[j] = ((bSrc[i] & 0xf0) >> 4) | '0';
// 			if (bDest[j] >= 0x3a)
// 				bDest[j] += 0x07;
// 			j++;
// 
// 			bDest[j] = (bSrc[i] & 0x0f) | '0';
// 			if (bDest[j] >= 0x3a)
// 				bDest[j] += 0x07;
// 			j++;
// 		}
// 	}
// 	return j;
// }
// 
// int WINAPI MakeUnPack(void* Src, CString& Dest, int Slen, int AscFlg)
// {
//     char str[STRBUFFSIZE];
// 
// 	memset(str, 0, sizeof(str));
// 	int i = MakeUnPack(Src, str, Slen, AscFlg);
// 	Strcpy(Dest, str);
// 	return i;
// }
// end of [#2001]

CString WINAPI MakeUnPack(void* Src, int Slen, int AscFlg)
{
	// [#2001] NH KSK 2010.11.15 512byte 초과 시 mac 생성이 잘못되는 bug fix
//	CString	cstrstr("");

//	MakeUnPack(Src, cstrstr, Slen, AscFlg);
//	return cstrstr;

	CString strUnPackData = L"";
	CString strTemp = L"";
	BYTE chUnPack = 0;

	BYTE	*bSrc = (BYTE *)Src;
	int		i=0;

	for(i = 0; i < Slen; i++)
	{
		if (!AscFlg)											// 0x30 - 0x3f
		{
			chUnPack = ((bSrc[i] & 0xf0) >> 4) | '0';
			strTemp.Format(L"%c", chUnPack);
			strUnPackData += strTemp;

			chUnPack = (bSrc[i] & 0x0f) | '0';
			strTemp.Format(L"%c", chUnPack);
			strUnPackData += strTemp;
		}
		else													// 0x30 - 0x39, 0x41 - 0x46
		{	
			chUnPack = ((bSrc[i] & 0xf0) >> 4) | '0';
			if (chUnPack >= 0x3a)	chUnPack += 0x07;
			strTemp.Format(L"%c", chUnPack);
			strUnPackData += strTemp;

			chUnPack = (bSrc[i] & 0x0f) | '0';
			if (chUnPack >= 0x3a)	chUnPack += 0x07;
			strTemp.Format(L"%c", chUnPack);
			strUnPackData += strTemp;
		}
	}

	return strUnPackData;
	// end of [#2001]
}
/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: MakePack()
 RETURN TYPE  : int : 결과(Pack되어진) 데이터 Length
 PARAMETER    : strSrc : 원본 데이터
				strDest : 결과 데이터
				OperMode : 두개를 합칠때 Operation ('^','|')
				LowNibbleFlg : TRUE : "123456" -> 0x01 0x23 0x34 0x45 0x60
							   FALSE: "123456" -> 0x12 0x34 0x56
 DESCRIPTION  : strSrc를 UnPack(0x34 -> "34") 한다.
 HISTORY LIST : 2009.03.26, AIREAT : 작성.
-------------------------------------------------------------------*/
int WINAPI MakePack(CString &strSrc, CString &strDest, char OperMode, int LowNibbleFlg)
{
	int	i=0, j=0;
	int	nLenSrc, nLenDest;
	char chSrc, chDest, chNewValue;
	LPTSTR	lpDest = NULL;

	// Calc Src string Length.
	nLenSrc = strSrc.GetLength();
	nLenDest = nLenSrc;

	// Get Dest string Buffer
	strDest.Empty();
	lpDest = strDest.GetBuffer(nLenDest);

	lpDest[0] = (TCHAR)0;
	chNewValue = 0;

	for (i = 0, j = 0; i < nLenSrc; i++)
	{
		chSrc = (char)strSrc[i];

		if (chSrc >= 'a' && chSrc <= 'f')
			chDest = ((chSrc - 0x57) & 0x0F);
		else
		if (chSrc >= 'A' && chSrc <= 'F')
			chDest = ((chSrc - 0x37) & 0x0F);
		else
			chDest = (chSrc & 0x0F);

		// 01 23 45 67 (zero base index)
		if (!LowNibbleFlg)
		{
			if (!(i&1))
			{
				if (OperMode == '|')
					chNewValue |= (chDest << 4);
				else
				if (OperMode == '^')
					chNewValue ^= (chDest << 4);
				else
					chNewValue = (chNewValue & 0x0F) | (chDest << 4);
			}
			else
			{
				if (OperMode == '|')
					chNewValue |= (chDest);
				else
				if (OperMode == '^')
					chNewValue ^= (chDest);
				else
					chNewValue = (chNewValue & 0xF0) | (chDest);
				
				lpDest[j++] = chNewValue;
				chNewValue = 0;
			}
		}
		// 00 12 34 56 (zero base index)
		else
		{
			if (i & 1)
			{
				if (OperMode == '|')
					chNewValue |= (chDest << 4);
				else
				if (OperMode == '^')
					chNewValue ^= (chDest << 4);
				else
					chNewValue = (chNewValue & 0x0F) | (chDest << 4);
			}
			else
			{
				if (OperMode == '|')
					chNewValue |= (chDest);
				else
				if (OperMode == '^')
					chNewValue ^= (chDest);
				else
					chNewValue = (chNewValue & 0xF0) | (chDest);
				
				lpDest[j++] = chNewValue;
				chNewValue = 0;
			}
		}
	}

	// Release Dest string Buffer
	strDest.ReleaseBuffer(j);

	return j;
}

/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: MakeUnPack()
 RETURN TYPE  : int : 결과(UnPack되어진) 데이터 Length
 PARAMETER    : strSrc : 원본 데이터
				strDest : 결과 데이터
				AscFlag : 원본 데이터가 ASCII 형 데이터이다.
 DESCRIPTION  : strSrc를 UnPack(0x34 -> "34") 한다.
 HISTORY LIST : 2009.03.26, AIREAT : 작성.
-------------------------------------------------------------------*/
int WINAPI MakeUnPack(CString &strSrc, CString &strDest, int AscFlg)
{
	int	i=0, j=0;
	int	nLenSrc, nLenDest;
	char chSrc, chDest;
	LPTSTR	lpDest = NULL;
	
	// Calc Src string Length.
	nLenSrc = strSrc.GetLength();
	nLenDest = nLenSrc * 2;

	// Get Dest string Buffer
	strDest.Empty();
	lpDest = strDest.GetBuffer(nLenDest);	

	lpDest[0] = (TCHAR)0;

	for (i = 0, j = 0; i < nLenSrc; i++)
	{
		chSrc = (char)strSrc[i];

		if (!AscFlg)
		{
			lpDest[j++] = (TCHAR) (((chSrc & 0xF0) >> 4) | '0');
			lpDest[j++] = (TCHAR) ((chSrc & 0x0F) | '0');
		}
		else
		{
			// 0x3a -> "3A"
			chDest =  (((chSrc & 0xF0) >> 4) | '0');
			if (chDest >= 0x3A) 
				chDest += 0x07;
			lpDest[j++] = (TCHAR) chDest;

			chDest =  ((chSrc & 0x0F) | '0');
			if (chDest >= 0x3A) 
				chDest += 0x07;
			lpDest[j++] = (TCHAR) chDest;
		}
	}

	// Release Dest string Buffer
	strDest.ReleaseBuffer(j);

	return j;
}

// [#508] NH AIREAT 2008.03.03
/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: IsValidDate()
 RETURN TYPE  : nonzero : successful
				zero : fail
 PARAMETER    : nYear : year, nMonth : month, nDay : day
				nHour : hour, nMin : min, nSec : sec
 DESCRIPTION  : date time의 값이 유효한지 검증한다.
-------------------------------------------------------------------*/
int	WINAPI IsValidDateTime(int nMonth, int nDay, int nYear, int nHour, int nMin, int nSec)
{
	/*                              Month    01  02  03  04  05  06  07  08  09  10  11  12*/
	static const int g_DayOfMonthinYear[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	static const int g_DayOfMonthinLeap[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	int nDayOfMonth = 0;

	// check normal condition each other
	if ((1970 <= nYear  && nYear  <= 2999) &&		// Year  (1970 - 2999)
		(   1 <= nMonth && nMonth <= 12  ) &&		// Month (1 - 12)
		(   1 <= nDay   && nDay   <= 31  ) &&		// Day (1 - 31)
		(   0 <= nHour  && nHour  <= 23  ) &&		// Hour (0 - 23)
		(   0 <= nMin   && nMin   <= 59  ) &&		// Minute (0 - 59)
		(   0 <= nSec   && nSec   <= 59  ))			// Second (0 - 59)
	{
		// check Leap & day of Month
		// 1. 해당 연도를 400으로 나눈 나머지가 0이면 윤년.
		// 2. 400으로 나눈 나머지가 0이 아니고, 100 으로 나눈 나머지가 0이면 윤년 아님.
		// 3. 400이나 100으로 나눈 나머지가 모두 0이 아니지만, 4로 나눈 나머지가 0이면 윤년.
		// 4. 그외에는 모두 윤년이 아님.
		if (((nYear % 4) == 0 && (nYear % 100) != 0) || (nYear % 400) == 0)
			nDayOfMonth = g_DayOfMonthinLeap[nMonth-1];
		else 
			nDayOfMonth = g_DayOfMonthinYear[nMonth-1];

		// check day of month
		if (nDay <= nDayOfMonth)
			return 1;
	}

	return 0;
}
// end of[#508]

void WINAPI GetDate(void* gDate)
{
	char	*cgDate = (char* )gDate;
	SYSTEMTIME	ti;

	GetLocalTime(&ti);
	Int2Asc(ti.wYear,	&cgDate[0], 4, 10, '0');
	Int2Asc(ti.wMonth,	&cgDate[4], 2, 10, '0');
	Int2Asc(ti.wDay,	&cgDate[6], 2, 10, '0');
}

CString WINAPI GetDate()
{
	CString	cstrstr("");
	char buf[64] = {};
    GetDate(buf);

	Strcpy(cstrstr, buf);
	return cstrstr;
}

void WINAPI GetTime(void* gTime)
{
	char	*cgTime = (char* )gTime;
	SYSTEMTIME	ti;

	GetLocalTime(&ti);
	Int2Asc(ti.wHour,	&cgTime[0], 2, 10, '0');
	Int2Asc(ti.wMinute,	&cgTime[2], 2, 10, '0');
	Int2Asc(ti.wSecond,	&cgTime[4], 2, 10, '0');
}

CString WINAPI GetTime()
{
	CString	cstrstr("");
	char buf[64] = {};
    GetTime(buf);

	Strcpy(cstrstr, buf);
	return cstrstr;
}

void WINAPI GetDateTime(void* gDate, void* gTime)
{
	GetDate(gDate);
	GetTime(gTime);
}

void WINAPI PutDate(void* pDate)
{
	char	*cpDate = (char* )pDate;
	SYSTEMTIME	ti;

	GetLocalTime(&ti);
	ti.wYear   = Asc2Int(&cpDate[0],4);
	ti.wMonth  = Asc2Int(&cpDate[4],2);
	ti.wDay    = Asc2Int(&cpDate[6],2);
	SetLocalTime(&ti);
}

void WINAPI PutDate(LPCTSTR pDate)
{
	char szSource[32] = {};
	WideToMulti(szSource, pDate, sizeof(szSource));

	PutDate((void*)szSource);
}

void WINAPI PutTime(void* pTime)
{
	char	*cpTime = (char* )pTime;
	SYSTEMTIME	ti;

	GetLocalTime(&ti);
	ti.wHour   = Asc2Int(&cpTime[0],2);
	ti.wMinute = Asc2Int(&cpTime[2],2);
	ti.wSecond = Asc2Int(&cpTime[4],2);
	SetLocalTime(&ti);
}

void WINAPI PutTime(LPCTSTR pTime)
{
	char szSource[32] = {};
	WideToMulti(szSource, pTime, sizeof(szSource));
	
	PutTime((void*)szSource);
}

void WINAPI PutDateTime(void* pDate, void* pTime)
{
	PutDate(pDate);
	PutTime(pTime);
}

void WINAPI PutDateTime(LPCTSTR pDate, LPCTSTR pTime)
{
	char szSource[32] = {};
	WideToMulti(szSource, pDate, sizeof(szSource));

	char szSource2[32] = {};
	WideToMulti(szSource2, pTime, sizeof(szSource2));

	PutDateTime((void*)szSource, (void*)szSource2);
}

CString WINAPI MakeMoneySpace(int nTotalLength, CString strAmount)
{
	CString strRetVal = _T("");
	int	nRetVal = 0;

	// "00150" -> "150"
	if (strAmount.GetLength())
	{
		nRetVal = Asc2Int(strAmount);
		strAmount = Int2Asc(nRetVal);
	}
	else return strRetVal;

	for(int i=0; i<nTotalLength; i++)
	{
		if((strRetVal.GetLength()+strAmount.GetLength()) == nTotalLength)
		{
			strRetVal = strRetVal + strAmount;
			break;
		}
		else strRetVal += " ";
	}

	return strRetVal;
}

// [#27] NH PSC 함수 수정. 기능은 동일, 자리수가 8자리 이상일때 처리하지 못하는 버그 수정.
CString WINAPI MakeMoneyCent(CString stramount, int opt)
{
	CString strRet = L"";
	
	CString strAmount(stramount);
	bool fMinus = false;
	
	// 공백 제거
	strAmount.Replace(L" ",L"");
	// 공백을 '0'으로 치환
	//strAmount.Replace(L" ",L"0");

	// 길이가 0 일때, "0.00"을 리턴
	if(strAmount.GetLength() < 1)
	{
		strRet = L"0.00";
		return strRet;
	}

	// 마이너스 값인지 체크
	if(strAmount.GetLength() > 0)
	{
		if(strAmount.GetAt(0) == L'-')
		{
			strAmount = strAmount.Right(strAmount.GetLength()-1);
			fMinus = true;
		}
	}

	// 앞에 '0'을 제거한다.
	while(strAmount.GetLength() > 0)
	{
		if(strAmount.GetAt(0) == L'0')
			strAmount = strAmount.Right(strAmount.GetLength()-1);
		else
			break;
	}

	// 3자리 이하일때 '0'을 채워서 3자리로 고정
	if(strAmount.GetLength() < 3)
	{
		CString strTemp(strAmount);
		strAmount.Format(L"%03s", strTemp);
	}
	
	// 센트 구분자('.')을 삽입.
	strRet = L"." + strAmount.Right(2);

	strAmount = strAmount.Left(strAmount.GetLength()-2);
	// 세자리마다 쉼표 삽입
	while(strAmount.GetLength() > 0)
	{
		if(strAmount.GetLength() > 3)
		{
			// 3자리 이상일때 3자리씩 짤라서 붙인다
			strRet = L"," + strAmount.Right(3) + strRet;
			strAmount = strAmount.Left(strAmount.GetLength()-3);
		}
		else
		{
			// 3자리 이하가 남았을때 나머지를 붙이고 종료
			strRet = strAmount + strRet;
			break;
		}
	}
	
	// 마이너스 이면 '-' 기호를 붙인다.
	if(fMinus && opt)
		strRet = L"-" + strRet;
	
	return strRet;
}

// [RWC6-699] Start SKKim 2024.04.25 Cent가 없는 Amount Value값 처리
CString WINAPI MakeMoneyAmount(CString strSrcAmount, int opt)
{
	CString strRet = L"";

	CString strAmount(strSrcAmount);
	bool fMinus = false;

	// 공백 제거
	strAmount.Replace(L" ",L"");
	// 공백을 '0'으로 치환
	//strAmount.Replace(L" ",L"0");

	// 길이가 0 일때, "0.00"을 리턴
	if(strAmount.GetLength() < 1)
	{
		strRet = L"0.00";
		return strRet;
	}

	// 마이너스 값인지 체크
	if(strAmount.GetLength() > 0)
	{
		if(strAmount.GetAt(0) == L'-')
		{
			strAmount = strAmount.Right(strAmount.GetLength()-1);
			fMinus = true;
		}
	}

	// 금액이 0보다 큰경우에만 앞에 0을 제거한다.
	if (Asc2Int(strSrcAmount) > 0)
	{
		// 앞에 '0'을 제거한다.
		while(strAmount.GetLength() > 0)
		{
			if(strAmount.GetAt(0) == L'0')
				strAmount = strAmount.Right(strAmount.GetLength()-1);
			else
				break;
		}
	}

	// 세자리마다 쉼표 삽입
	while(strAmount.GetLength() > 0)
	{
		if(strAmount.GetLength() > 3)
		{
			// 3자리 이상일때 3자리씩 짤라서 붙인다
			strRet = L"," + strAmount.Right(3) + strRet;
			strAmount = strAmount.Left(strAmount.GetLength()-3);
		}
		else
		{
			// 3자리 이하가 남았을때 나머지를 붙이고 종료
			strRet = strAmount + strRet;
			break;
		}
	}

	// 마이너스 이면 '-' 기호를 붙인다.
	if(fMinus && opt)
		strRet = L"-" + strRet;

	return strRet;
}
// [RWC6-699] End SKKim 2024.04.25

// [#2] NH JSW 2008.03.03 지정된 날짜 포맷으로 변환
/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: GetCmnLocalDate()
 RETURN TYPE  : 국가별 DATE 스트링.
 PARAMETER    : strMonth : Year
				strDay : Month
				strYear : Day
				chDelimiter : string을 만들때 삽입할 구분자.
 DESCRIPTION  : 국가 별 DATE string을 만든다.
-------------------------------------------------------------------*/
CString WINAPI GetCmnLocalDate(CString strMonth, CString strDay, CString strYear, TCHAR chDelimiter)
{
	CString strTmp = L"";
	if( strMonth.IsEmpty() && strDay.IsEmpty() &&  strYear.IsEmpty() )	return strTmp;	// 2008.04.18 JSW NULL일 경우 처리
	if( strYear.GetLength() == 2 )	strYear.Format(L"20%s", strYear);

#if DATE_YYYYMMDD
	strTmp.Format(L"%4.4s/%2.2s/%2.2s", strYear, strMonth, strDay);
#elif DATE_MMDDYYYY
	strTmp.Format(L"%2.2s/%2.2s/%4.4s", strMonth, strDay, strYear);
#elif DATE_DDMMYYYY
	strTmp.Format(L"%2.2s/%2.2s/%4.4s", strDay, strMonth, strYear);
#else
	strTmp.Format(L"%2.2s/%2.2s/%4.4s", strMonth, strDay, strYear);
#endif

	// [#508] NH AIREAT 2008.03.03
	if (chDelimiter == 0)
		strTmp.Remove('/');
	else if (chDelimiter != '/')
		strTmp.Replace('/', chDelimiter);
	// end of [#508]

	return strTmp;
}

/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: GetCmnLocalDate()
 RETURN TYPE  : 국가별 DATE 스트링.
 PARAMETER    : nMonth : Year
				nDay : Month
				nYear : Day
				chDelimiter : string을 만들때 삽입할 구분자.
 DESCRIPTION  : 국가 별 DATE string을 만든다.
-------------------------------------------------------------------*/
CString WINAPI GetCmnLocalDate(int nMonth, int nDay, int nYear, TCHAR chDelimiter)
{
	CString strTmp = L"";
	if( nMonth == 0 && nDay == 0 && nYear == 0 )	return strTmp;	// 2008.04.18 JSW NULL일 경우 처리
	if( nYear <= 99 )	nYear += 2000;

#if DATE_YYYYMMDD
	strTmp.Format(L"%4.4d/%2.2d/%2.2d", nYear, nMonth, nDay);
#elif DATE_MMDDYYYY
	strTmp.Format(L"%2.2d/%2.2d/%4.4d", nMonth, nDay, nYear);
#elif DATE_DDMMYYYY
	strTmp.Format(L"%2.2d/%2.2d/%4.4d", nDay, nMonth, nYear);
#else
	strTmp.Format(L"%2.2d/%2.2d/%4.4d", nMonth, nDay, nYear);
#endif

	// [#508] NH AIREAT 2008.03.03
	if (chDelimiter == 0)
		strTmp.Remove('/');
	else if (chDelimiter != '/')
		strTmp.Replace('/', chDelimiter);
	// end of [#508]

	return strTmp;
}
// end of [#2]

// [#508] NH AIREAT 2008.03.03
/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: GetCmnLocalDate()
 RETURN TYPE  : nonzero : successful
				zero : fail
 PARAMETER    : strDate : Date를 가지고 있는 문자열.
						"MM/DD/YYYY", "DD/MM/YYYY", "YYYY/MM/DD"
				pnYear : 문자열에서 추출한 Year
				pnMonth : 문자열에서 추출한 Month
				pnDay : 문자열에서 추출한 Day
				nHour : hour, nMin : min, nSec : sec
 DESCRIPTION  : 국가 별 DATE string에서 Year, Month, Day를 구한다.
				일반변수를 이용하여 값을 가져간다. 포인터는 안되...
-------------------------------------------------------------------*/
int WINAPI GetCmnLocalDate(CString strDate, int *pnMonth, int *pnDay, int *pnYear)
{
	// only check length
	if (strDate.GetLength() != 10)		return 0;

#if DATE_YYYYMMDD
	*pnYear = Asc2Int(strDate.Left(4));
	*pnMonth = Asc2Int(strDate.Mid(5,2));
	*pnDay = Asc2Int(strDate.Right(2));
#elif DATE_MMDDYYYY
	*pnYear = Asc2Int(strDate.Right(4));
	*pnMonth = Asc2Int(strDate.Left(2));
	*pnDay = Asc2Int(strDate.Mid(3,2));
#elif DATE_DDMMYYYY
	*pnYear = Asc2Int(strDate.Right(4));
	*pnMonth = Asc2Int(strDate.Mid(3,2));
	*pnDay = Asc2Int(strDate.Left(2));
#else
	*pnYear = Asc2Int(strDate.Right(4));
	*pnMonth = Asc2Int(strDate.Left(2));
	*pnDay = Asc2Int(strDate.Mid(3,2));
#endif

	return 1;
}
// end of [#508]

// [#4] NH PSC 2008.03.10
const unsigned int crc_table[256] = 
{
		0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241, 
		0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440, 
		0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40, 
		0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841, 
		0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40, 
		0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41, 
		0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641, 
		0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040, 
		0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240, 
		0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441, 
		0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41, 
		0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840, 
		0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41, 
		0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40, 
		0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640, 
		0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041, 
		0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240, 
		0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441, 
		0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41, 
		0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840, 
		0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41, 
		0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40, 
		0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640, 
		0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041, 
		0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241, 
		0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440, 
		0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40, 
		0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841, 
		0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40, 
		0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41, 
		0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641, 
		0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};

unsigned int WINAPI crc16 (unsigned char *string, int length ) 
{ 
    unsigned int crc; 
    crc = 0x0000;    /* initial CRC value 0x0000 */ 
    while (length--) 
    { 
        crc = ( crc >> 8 ) ^ crc_table[(crc ^ (int) *string++) & 0xff];   
		/* This performs the modulo-2 division by using the look up table. */ 
    } 
    return(crc); /* return the calculated CRC */ 
}
// end of [#4]

unsigned char WINAPI lrc(unsigned char *string, int length)
{
	unsigned char lrc = 0x00; /* initial CRC value 0x00 */

	if (length <=0)	return 0;	// KSK 2012.08.16 Invalid 처리 추가

	while (length--)
	{
		lrc = lrc ^ *string++;
	}
	return lrc;
}

CString WINAPI GetCurrencySymbol(int nLength, int nBracket, int nCustomerType)
{
	CString	strSymbol;

#if (MX_VERSION)	// [#2115] MX KSK 2012.02.05
	if ( (nCustomerType == MX_INTERNATIONAL_TYPE) ||
		((MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MULTI_CURRENCY_ENABLE) == ENABLE) && (nCustomerType == 0)) )
	{
		strSymbol.Format(L"%s", MULTI_CURRENCY_SYMBOL);
	}
	else
	{
		strSymbol.Format(L"%s", CURRENCY_SYMBOL);
	}
#else
	strSymbol.Format(L"%c", CURRENCY_SYMBOL);
#endif				// end of [#2115]

	if (nBracket == 1)
	{
		strSymbol = L"(" + strSymbol + L")";
	}

	if ( nLength > strSymbol.GetLength() )
	{
		strSymbol = strSymbol + CString(' ', nLength - strSymbol.GetLength());
	}

	return strSymbol;
}

void WINAPI CheckSystemMemory(CString	DebugLocation)
{
#ifdef MEM_LEAK_CHECK
	CString	szTemp = _T("");
	static	DWORD	saveupm, saveuvm;
	{
		MEMORYSTATUS memStatus;	
		memStatus.dwLength = sizeof(MEMORYSTATUS);
		GlobalMemoryStatus(&memStatus);
		DWORD upm, uvm;

		upm = memStatus.dwTotalPhys - memStatus.dwAvailPhys;
		uvm = memStatus.dwTotalVirtual - memStatus.dwAvailVirtual;
		if ((saveupm != upm) || (saveuvm != uvm))
		{
			RETAILMSG(1, (_T("[%s] -- PU:%8d (%d), VU:%08d (%d) <----- Changed!!!\n"),
							DebugLocation, upm, upm - saveupm, uvm, uvm - saveuvm));
			saveupm = upm;
			saveuvm = uvm;
		}
		else
		{
			RETAILMSG(1, (_T("[%s] -- PU:%8d, VU:%08d \n"),
							DebugLocation, upm, uvm));
		}
	}
#endif
}


void WINAPI AddPrintData(CString &strPrintData, CString strData)
{
	strPrintData += strData;
	strPrintData += FIELD_DELIMITER;
}

void WINAPI AddPrintData(CString &strPrintData, LPCTSTR lpszFormat, ...)
{
	CString temp;

	va_list args;
	va_start(args, lpszFormat);
	temp.FormatV(lpszFormat, args);
	va_end(args);

	strPrintData.Append(temp);
	strPrintData += FIELD_DELIMITER;
}

// [#2012] NH KJW 2010.01.10 LRC 생성 함수 추가/ Endian을 변경 함수 추가
BYTE WINAPI lrc8 (PBYTE pBuf, int nLen ) 
{ 
	BYTE byt = 0x00;

	for( int i = 0 ; i < nLen ; i++ )
		byt = byt ^ pBuf[i];

	return byt;
}

UINT16 WINAPI SwapEndian(UINT16 val)
{
	val = (val<<8) | (val>>8);
	return val;
}

UINT32 WINAPI SwapEndian24(UINT32 val)
{
	val = (0x00ffffff) & (((val<<16) & 0x00ff0000) | (val & 0x0000ff00) | ((val>>16) & 0x000000ff));
	return val;
}

UINT32 WINAPI SwapEndian(UINT32 val)
{
	val = (val<<24) | ((val<<8) & 0x00ff0000) | ((val>>8) & 0x0000ff00) | (val>>24);
	return val;
}
// end of [#2012]

// [#2571] NH Justin 2018.08.03 Fix Machine Type Setting Problem - 1024*600 Type ATMs
BOOL WINAPI IsUniqueATMType(CString strATMType)
{
	if(	strATMType == L"NH2700T" || strATMType == L"NH1500SE" || strATMType == L"MX2800T")	// [#GLDV-2505] Support MX2800T
		return TRUE;
	
	return FALSE;
}
// End of [#2571]

// [#2373] US Justin 2015.09.22 Support Barcode Printing
int g_I2of5_Pattern[] = {
	0,0,1,1,0,			// 0
	1,0,0,0,1,			// 1
	0,1,0,0,1,			// 2
	1,1,0,0,0,			// 3
	0,0,1,0,1,			// 4
	1,0,1,0,0,			// 5
	0,1,1,0,0,			// 6
	0,0,0,1,1,			// 7
	1,0,0,1,0,			// 8
	0,1,0,1,0			// 9
};

//////////////////////////////////////////////////////////////////////
// One Sripe
//////////////////////////////////////////////////////////////////////

COneStripe::COneStripe()
{
	m_fLocX = m_fWidth = 0;
}

COneStripe::~COneStripe()
{

}

void COneStripe::GetStripeInfo(float*fLoc, float*fWidth)
{
	*fLoc = m_fLocX;
	*fWidth = m_fWidth;
}

void COneStripe::AssignLocAndWidth(float fLocX, float fWidth)
{
	m_fLocX = fLocX;
	m_fWidth = fWidth;
}

//////////////////////////////////////////////////////////////////////
// CBarStripe() Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBarStripe::CBarStripe()
{
	m_pStripes = NULL;
	InitializeData();
}

CBarStripe::~CBarStripe()
{
	InitializeData();
}

void CBarStripe::InitializeData()
{
	m_szBarStripe.cx = 0;
	m_szBarStripe.cy = 0;
	m_nStripeNumber = 0;
	if( m_pStripes != NULL)
		delete [] m_pStripes;
	m_pStripes = NULL;
}

CSize CBarStripe::GenerateStripes(int nWidth, int nHeight, LPCTSTR sData)
{
	int i;
	float fRatio = (float) 2.3;
	InitializeData();

	CString strData = CString(sData);
	int nDataLen = strData.GetLength();
	if( (nWidth<=0)||(nHeight<=0)||(nDataLen<=0) )
		return m_szBarStripe;

	// Check Data Len => Odd : Add '0' in front : I 2 of 5 can handle only even digit, Odd digit => prefix "0"
	if( ((int)(0.5*nDataLen+0.1)*2) != nDataLen )
	{
		strData = "0" + strData;
		nDataLen = strData.GetLength();
	}
	
	// Interleave 2 of 5 can handle only Numbers......
	for(i=0; i<nDataLen; i++)
	{
		int nOneValue = Asc2Int(strData.Mid(i, 1) );
		if( (nOneValue>9)||(nOneValue<0) )
			return m_szBarStripe;
	}

	// Width (Narrow, Wide, Total)
	float fWidN = (float) ( 1.0*nWidth / ( nDataLen*(2*fRatio+3)+6+fRatio ) );
	float fWidW   = (float) ( fRatio * fWidN );
	m_szBarStripe.cx = nWidth;
	m_szBarStripe.cy = nHeight;

	// Number of bar and space
	m_nStripeNumber = (int)(0.5*nDataLen+0.6) * 5 + 4;
	m_pStripes = new COneStripe[m_nStripeNumber];
	
	int nNumN = 0;	int nNumW = 0;
	// Start Character  -   Narrow bar / Narrow space / Narrow bar / Narrow space
	m_pStripes[0].AssignLocAndWidth( (float)(fWidN*nNumN+fWidW*nNumW), fWidN);	nNumN += 2;	
	m_pStripes[1].AssignLocAndWidth( (float)(fWidN*nNumN+fWidW*nNumW), fWidN);	nNumN += 2;

	// Data (Interleave 2 of 5)
	int nHalfData = (int) ( nDataLen*0.5 + 0.1);
	for(i=0; i<nHalfData; i++)
	{
		int nPattern;
		float fStrWid;
		for(int k=0; k<5; k++)
		{
			// BAR
			nPattern = g_I2of5_Pattern[ Asc2Int(strData.Mid(i*2,1))  * 5 + k];	
			if(nPattern)	fStrWid = fWidW;
			else			fStrWid = fWidN;	
			m_pStripes[2 + i*5 + k].AssignLocAndWidth( (float)(fWidN*nNumN+fWidW*nNumW), fStrWid);		
			if(nPattern)	nNumW++;
			else			nNumN++;

			// SPACE
			nPattern = g_I2of5_Pattern[ Asc2Int(strData.Mid(i*2+1,1)) * 5 + k];	
			if(nPattern)	nNumW++;
			else			nNumN++;
		}
	}

	// End Character  -   Wide bar / Narrow space / Narrow bar
	m_pStripes[m_nStripeNumber-2].AssignLocAndWidth( (float)(fWidN*nNumN+fWidW*nNumW), fWidW);		nNumW++;  nNumN++;
	m_pStripes[m_nStripeNumber-1].AssignLocAndWidth( (float)(fWidN*nNumN+fWidW*nNumW), fWidN);
	return m_szBarStripe;
}


int CBarStripe::GetNumberOfStripe()
{
	return m_nStripeNumber;
}

BOOL CBarStripe::GetStripeInfo(int nOrder, int*nLocX, int*nWidth, int*nHeight)
{
	if( (nOrder>=m_nStripeNumber)||(m_pStripes==NULL) )
		return FALSE;
	
	float fLocX, fWidth;
	m_pStripes[nOrder].GetStripeInfo(&fLocX, &fWidth);
	*nLocX = (int)(fLocX+0.5);		// round up
	*nWidth = (int)(fWidth+0.5);	// round up
	*nHeight = m_szBarStripe.cy;

	return TRUE;
}

BOOL WINAPI CreateNumberBarcode(LPCTSTR strData, int nImgWidth, int nImgHeight, LPCTSTR strFileName)
{
	if( (nImgWidth<=0)||(nImgHeight<=0))
		return FALSE;
	
	HDC hMemDC = NULL;
	int nWidth = nImgWidth;
	int nHeight = nImgHeight; 

	hMemDC = ::CreateCompatibleDC(NULL);

	BITMAPINFO bi; 
	ZeroMemory(&bi, sizeof(BITMAPINFO));
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = nWidth;
	bi.bmiHeader.biHeight = nHeight;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 1;

	BYTE *lpBitmapBits = NULL; 
	HBITMAP bitmap = ::CreateDIBSection(hMemDC, &bi, DIB_RGB_COLORS, (LPVOID*)&lpBitmapBits, NULL, 0);
	HGDIOBJ oldbmp = ::SelectObject(hMemDC, bitmap); 
	HBRUSH hBrushWhite = ::CreateSolidBrush(RGB(255,255,255));
	HBRUSH hBrushBlack = ::CreateSolidBrush(RGB(0,  0,  0));

	if(lpBitmapBits == NULL)
	{
		::SelectObject(hMemDC, oldbmp);
		::DeleteObject(hBrushWhite);
		::DeleteObject(hBrushBlack);
		::DeleteObject(bitmap);
		::DeleteObject(hMemDC);
		return FALSE;
	}

	//////////////////////////////////////////////////
	// Draw Barcode
	// Generate Barcode Data
	CBarStripe	barStripe;
	CSize szBmpsize = barStripe.GenerateStripes(nWidth, nHeight, strData);

	// Draw Background
	RECT rectFill;
	rectFill.left = 0; rectFill.top = 0; rectFill.right = szBmpsize.cx; rectFill.bottom = szBmpsize.cy;
	::FillRect(hMemDC, &rectFill, hBrushWhite);

	// Draw Black Bars
	int nLocX, nBarWidth, nBarHeight;
	int nNumStripe = barStripe.GetNumberOfStripe();
	for(int i=0; i<nNumStripe; i++)
	{
		if( barStripe.GetStripeInfo(i, &nLocX, &nBarWidth, &nBarHeight ) )
		{
			rectFill.left = nLocX; rectFill.top = 0; rectFill.right = nLocX+nBarWidth; rectFill.bottom = nBarHeight;
			::FillRect(hMemDC, &rectFill, hBrushBlack);
		}
	}

	BITMAPFILEHEADER bh;
	ZeroMemory(&bh, sizeof(BITMAPFILEHEADER));
	bh.bfType = 0x4d42;			//BM - BMP
	bh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	int nNewWidth = nWidth;
	if( nWidth % 64 != 0 )
	{
		nNewWidth = (int) (1.0*nWidth/64) + 1;
		nNewWidth *= 64;
	}
	bh.bfSize = bh.bfOffBits + ((nNewWidth*nHeight) / 8);

	BOOL bSaved = FALSE;
	CFile file;
	if(file.Open(strFileName, CFile::modeCreate | CFile::modeWrite))
	{ 
		/*
		file.Write(&bh, sizeof(BITMAPFILEHEADER));
		file.Write(&(bi.bmiHeader), sizeof(BITMAPINFOHEADER));
		file.Write(lpBitmapBits, bh.bfSize);
		file.Close();
		bSaved = TRUE;
		*/

		// Write Bitmap File
		TRY
		{
			file.Write(&bh, sizeof(BITMAPFILEHEADER));
			file.Write(&(bi.bmiHeader), sizeof(BITMAPINFOHEADER));
			file.Write(lpBitmapBits, bh.bfSize);
			bSaved = TRUE;
		}
		CATCH(CFileException, e)
			bSaved = FALSE;
		END_CATCH
		file.Close();

		// Check Bitmap file Size
		if(bSaved)
		{
			CFile cf;
			if( cf.Open( strFileName, CFile::modeRead) == FALSE )
				bSaved = FALSE;
			else
			{
				int nFileLen = (int)cf.GetLength();
				cf.Close();

				if( nFileLen != ( sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bh.bfSize) )
					bSaved = FALSE;
			}
		}
	}
	::SelectObject(hMemDC, oldbmp);
	::DeleteObject(hBrushWhite);
	::DeleteObject(hBrushBlack);
	::DeleteObject(bitmap);
	::DeleteObject(hMemDC);

	return bSaved;
}

void WINAPI SaveScreenToFile(LPCTSTR szFileName)
{
	HDC hScrDC = ::GetDC(NULL);
	HDC hMemDC = NULL;

	BYTE *lpBitmapBits = NULL; 

	int nWidth = GetSystemMetrics(SM_CXSCREEN);
	int nHeight = GetSystemMetrics(SM_CYSCREEN); 

	hMemDC = ::CreateCompatibleDC(hScrDC); 

	BITMAPINFO bi; 
	ZeroMemory(&bi, sizeof(BITMAPINFO));
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = nWidth;
	bi.bmiHeader.biHeight = nHeight;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 24;

	HBITMAP bitmap = ::CreateDIBSection(hMemDC, &bi, DIB_RGB_COLORS, (LPVOID*)&lpBitmapBits, NULL, 0);
	HGDIOBJ oldbmp = ::SelectObject(hMemDC, bitmap); 

	::BitBlt(hMemDC, 0, 0, nWidth, nHeight, hScrDC, 0, 0, SRCCOPY);

	BITMAPFILEHEADER bh;
	ZeroMemory(&bh, sizeof(BITMAPFILEHEADER));
	bh.bfType = 0x4d42; //bitmap 
	bh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bh.bfSize = bh.bfOffBits + ((nWidth*nHeight)*3);

	CFile file;
	if(file.Open(szFileName, CFile::modeCreate | CFile::modeWrite))
	{ 
		file.Write(&bh, sizeof(BITMAPFILEHEADER));
		file.Write(&(bi.bmiHeader), sizeof(BITMAPINFOHEADER));
		file.Write(lpBitmapBits, 3 * nWidth * nHeight);
		file.Close();
	}

	::SelectObject(hMemDC, oldbmp);
	::DeleteObject(bitmap);
	::DeleteObject(hMemDC);
	::ReleaseDC(NULL, hScrDC);
}