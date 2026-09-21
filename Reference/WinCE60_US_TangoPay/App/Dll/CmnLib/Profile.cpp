#include "stdafx.h"
#include "afxwin.h"

#include "stdio.h"
#include "stdlib.h"
#include ".\Common\DataCtrl.h"
#include ".\Common\CmnLib.h"
#include ".\Common\NHDbgApi.h"

//#define MAXINIVALUE_LEN 512
#define MAXINIVALUE_LEN 2048		// [RWC6-676] SKKim 2024.04.05

//	[#81] UK KGS 2008.04.07 AID List 
CINIFile::CINIFile(char* szFileName)
{
	fp = NULL;
	m_szFileName = szFileName;
}

BOOL CINIFile::GetNextSection(char* szBuf)
{
	int nIdx=-1;
	char ch;
	memset(szBuf,NULL,sizeof(szBuf));
	
	while(TRUE)
	{
		ch=fgetc(fp);
		if(ch==EOF)
			return FALSE;
		else if(ch=='[')
			nIdx=0;
		else if(ch==']')
		{
			if (nIdx >= 0)
				szBuf[nIdx]=NULL;
			break;
		}
		else if(nIdx>=0)
			szBuf[nIdx++]=ch;
	}
	return TRUE;
}

BOOL CINIFile::GetNextKey(char* szBuf)
{
	int nIdx=-1;
	char ch;
	memset(szBuf,NULL,sizeof(szBuf));

	while(TRUE)
	{
		ch=fgetc(fp);
		if(ch==EOF || ch=='[')
			return FALSE;
		else if(ch=='\n')
			nIdx=0;
		else if(ch == '=')
		{
			if (nIdx >= 0)
				szBuf[nIdx]=NULL;
			break;
		}
		else if(nIdx>=0)
			szBuf[nIdx++]=ch;
	}
	return TRUE;
}

BOOL CINIFile::GetValue(char* szBuf)
{
	int nIdx=0;
	char ch;
	memset(szBuf,NULL,sizeof(szBuf));

	while(TRUE)
	{
		ch=fgetc(fp);
		if(ch=='[')
			return FALSE;
		else if(ch==EOF || ch=='\n')
		{
			szBuf[nIdx]=NULL;
			break;
		}
		else if(nIdx>=0)
			szBuf[nIdx++]=ch;
	}
	return TRUE;
}

BOOL CINIFile::GetPrivateProfileString(char* lpAppName,char* lpKeyName, char* lpReturnedString, DWORD nSize)
{
	char szBuf[2048];
	BOOL ret = FALSE;

	if((fp = fopen(m_szFileName,"r")) == NULL)
		goto ErrorReturn;				// return FALSE
	
	while(GetNextSection(szBuf))
	{
		if(strcmp(szBuf,lpAppName)==0)
		{
			while(GetNextKey(szBuf))
			{
				if(strcmp(szBuf,lpKeyName)==0)
				{
					ret =  GetValue(lpReturnedString);
					goto ErrorReturn;	// return TRUE or FALSE
				}
			}
			goto ErrorReturn;			// return FALSE
		}
	}

ErrorReturn:
	if (fp != NULL)
		fclose(fp);
	return ret;
}
// end of [#81]

CString WINAPI GetPStr(char *FileName, char *Section, char *Key, char *Default)
{
NHDEBUG(1, (_T("***Profile.cpp***  GetPStr() \n")));
	char* ucaReadBuf = NULL;
	CString strResult("");

	if (GetIniString(FileName, Section, Key, &ucaReadBuf))
	{
		strResult = ucaReadBuf;
		NHDEBUG(1, (_T("***Profile.cpp***  GetPStr() ucaReadBuf(%s) strResult(%s) \n"), ucaReadBuf, strResult));
		delete [] ucaReadBuf;
	}
	else
	{
		strResult = Default;
		NHDEBUG(1, (_T("***Profile.cpp***  GetPStr() Default(%s) strResult(%s) \n"), Default, strResult));
	}


	return strResult;
}

int WINAPI GetPInt(char *FileName, char *Section, char *Key, int nDefault)
{
NHDEBUG(1, (_T("***Profile.cpp***  GetPInt() \n")));
	char*	ucaReadBuf = NULL;
	int		nResult = 0;

	if (GetIniString(FileName, Section, Key, &ucaReadBuf))
	{
		nResult = Asc2Int((CString)ucaReadBuf);
		NHDEBUG(1, (_T("***Profile.cpp***  GetPInt() ucaReadBuf(%s) nResult(%d) \n"), ucaReadBuf, nResult));
		delete [] ucaReadBuf;
	}
	else
	{
		nResult = nDefault;
		NHDEBUG(1, (_T("***Profile.cpp***  GetPInt() nDefault(%s) nResult(%d) \n"), nDefault, nResult));
	}

	return nResult;
}

/*****************************************************************************
	일치하는 INI파일내에 (해당 섹션안에) 키의 값을 읽어 온다.
*****************************************************************************/
int WINAPI GetIniString(char *FileName, char *Section, char *Key, char **Value)
{
	LINK first, current;
	int ret = -1;
	int valuesize = 0;
	*Value = NULL;

    first = NULL;
	
	first = read_list_file(FileName,first);
	if (first == NULL) return -1;
   
	if(Section[0] != NULL)
	{
		current = find_section (Section, first);
		if(current == NULL) goto cleanup;
		current = current->next;
		current = find_key( 1, Key, current);
		if(current == NULL) goto cleanup;
	}
	else 
	{
		current = find_key( 0, Key, first);
		if(current == NULL) goto cleanup;
	}

	valuesize = strlen(current->value) + 1;
	*Value = new char[valuesize]();
	strncpy_s(*Value, valuesize, current->value, valuesize);
	ret = 1;

cleanup:
	free_memory_list(first);
	return ret;
}

/*****************************************************************************
	일치하는 INI파일내에 (해당 섹션안에) 키의 값을 기록한다.
*****************************************************************************/
int WINAPI SetIniString(char *FileName, char *Section, char *Key, char *Value)
{
	LINK first, section, current;
	char *temp;
	DWORD dwFileWriten = 0;

    first = NULL;
	first = read_list_file(FileName,first);

	if (first == NULL) return -1;

	if(Section[0] != NULL)		// Use Section name
	{
		current = find_section (Section, first);
		if(current == NULL)		// No Section
		{
			int tempSize = strlen(Section) + 3;
			temp = new char[tempSize]();
			if(temp == NULL)	// Code sonar 지적사항 수정 2008.04.16 PSC
				goto Error_return;
			sprintf_s(temp, tempSize, "[%s]",Section);
			add_to_list(temp, "", first);
			delete [] temp;
			current = find_section (Section, first);
			if(current == NULL)	// Code sonar 지적사항 수정 2008.04.16 PSC
				goto Error_return;
		}

		section = current;				//point to insert key
		current = current->next;		//current is section in here so increase
		current = find_key( 1, Key, current );
		if(current == NULL)		// No Key
		{
			current = add_to_section( Key, Value, section );	
			if(current == NULL)
				goto Error_return;
		}
		else	//Key is and need change value
		{
			// Ensure the string is empty before copying new value in.
			// [#RWC6-59] US William 2019.10.03 LibertyX
			memset(current->value, 0, sizeof(current->value));
			//int nLen = strnlen_s(Value, MAXINIVALUE_LEN);	// test code
			if (strncpy_s(current->value, sizeof(current->value), Value, strnlen_s(Value, MAXINIVALUE_LEN)) != 0)
			{
				memset(current->value, 0, sizeof(current->value));
				return -1;
			}
		}

	}
	else	// Don't use section name
	{
		current = find_key( 0, Key, first);
		if(current == NULL)
		{
			first = add_to_list(Key,Value,first);
			if(first == NULL)	// Code sonar 지적사항 수정 2008.04.16 PSC
				return -1;
		}
		else 
		{
			// Ensure the string is empty before copying new value in.
			// [#RWC6-59] US William 2019.10.03 LibertyX
			memset(current->value, 0, sizeof(current->value));
			if (strncpy_s(current->value, sizeof(current->value), Value, strnlen_s(Value, MAXINIVALUE_LEN)) != 0)
			{
				memset(current->value, 0, sizeof(current->value));
				return -1;
			}
		}
	}		

    write_list_file(FileName,first);
	free_memory_list(first);
	return 1;

Error_return:
	free_memory_list(first);
	return -1;
}
// [#151] KSK 2008.04.21 File Version Read 함수 추가
CString WINAPI GetFileReadVersion(CString strLocalFileName, BOOL bUseLowVersion)
{
	DWORD	dwhandle = 0;
	DWORD	dwlength = 0;
	int		nVersion[4] = {0, 0, 0, 0};
	TCHAR	TLocalFileName[256];
	CString	strResult = _T("");

	wsprintf(TLocalFileName, _T("%s"), strLocalFileName);

	dwlength = ::GetFileVersionInfoSize(TLocalFileName, &dwhandle);

	if (dwlength > 0)
	{
		BYTE *pVerInfo = (BYTE*)new BYTE[dwlength];
		if(::GetFileVersionInfo(TLocalFileName, NULL, dwlength, pVerInfo))
		{
			UINT vLength = 0;
			LPVOID VersionPtr = NULL;
			
			if(VerQueryValue(pVerInfo, _T("\\"), &VersionPtr, &vLength))
			{
				VS_FIXEDFILEINFO vi = *(VS_FIXEDFILEINFO*) VersionPtr;
				
				// Major1
				nVersion[0] = vi.dwFileVersionMS >> 16;
				// Major2
				nVersion[1] = (vi.dwFileVersionMS & 0xFF);
				// Minor1
				nVersion[2] = vi.dwFileVersionLS >> 16;
				// Minor2
				nVersion[3] = (vi.dwFileVersionLS & 0xFF);
			}
		}
		delete [] pVerInfo;
		
		if (bUseLowVersion == TRUE)
			strResult.Format(_T("V%02d.%02d.%02d.%02d"), nVersion[0], nVersion[1], nVersion[2], nVersion[3]);
		else
			strResult.Format(_T("V%02d.%02d.%02d"), nVersion[0], nVersion[1], nVersion[2]);
		return strResult;
	}
	return _T("N/A");
}
// end of [#151]

/*****************************************************************************
	오른쪽 부터 공백을 0 치환
*****************************************************************************/
void rtrim(char	*str)
{
	int	i=0;
	for	(i=strlen(str)-1; i>=0;	i--) {
		if(str[i] == ' ')
			str[i] = 0;
		else
			break;
	}
}

/*****************************************************************************
	in_data에서 fCnt번째 cG의 위치를 리턴 
*****************************************************************************/
int strpos(unsigned char *in_data,unsigned char cG, unsigned int fCnt)
{
	unsigned int iFor,iCnt;

	iFor = 0;
	iCnt = 0;

	for (iFor = 0 ; iFor <= strlen((char *)in_data); iFor ++) {
		if ((unsigned char)in_data[iFor] == cG) {
			iCnt ++;
			if (iCnt == fCnt) {
				return iFor + 1;
			}
		}
	}
	return -1;
}

/*****************************************************************************
	리턴값 : 읽은 자료가 하나도 없으면 -1, 한바이트라도 있으면 0 보다 크거나 같다
*****************************************************************************/
int read_line(FILE *f ,char * out_data, int max_cnt)
{
    int cnt, ret;
	int ch;

    ret = -1;
	cnt = 0;

	//while(!feof(f) && (cnt < max_cnt))
	while(!feof(f) && (cnt < max_cnt-1))		// [CODESONAR] fix Buffer Overrun (ID: 178)
	{	
        ret = cnt;		
		// fgetc() 은 이상하게도 0x0D 는 읽지 않는다. ..
		ch = fgetc( f );
		if (ch == 13) break;
		else if (ch == 10) break;
		else if (ch == EOF) break;  // US William 2020.04.23 Fix EOF into strings when no line break at end of file.
        else 
			out_data[cnt++] = (char)ch;
        ret = cnt;		
    }

    /* Add null to end string */
	out_data[cnt] = '\0';					/* occur Buffer Overrun (ID: 178) */
	return ret;
}



/*****************************************************************************
	파일에서 리스트를 추출한다.
*****************************************************************************/
LINK read_list_file(char *filename,LINK first)
{
	FILE *in;
	int i = 0;
	char buffer[2048];
	char name[2048];
	char value[2048];

	if(first != NULL) first = NULL;

	if((in=fopen(filename,"r"))==NULL)
	{
		return NULL;
	}
	while(!feof(in))
	{	
		memset(name,0,sizeof(name));
		memset(value,0,sizeof(value));
		memset(buffer,0,sizeof(buffer));
        
		i = read_line( in , buffer,  sizeof(buffer));
        if (i <= 0 ) break;

		if(buffer[0] =='[' && buffer[strlen(buffer)-1] ==']')
		{
			strncpy_s(name, sizeof(name), buffer, sizeof(buffer));
			first = add_to_list(name,value,first);
		}
		else
		{
			i = strpos((unsigned char *)buffer, '=', 1 );	//seek '='
			if( i < 0 )
			{
				continue;
			}
			else
			{
				char* valuepos = strchr(buffer, '=') + 1;
				int valuesize = strnlen_s(valuepos, buffer - valuepos);
				
				strncpy_s(value, sizeof(value), valuepos, valuesize);
				strncpy_s(name, sizeof(name), buffer, i - 1);
				rtrim(value);
				first = add_to_list(name,value,first);
			}
		}//end else
	
	}//end whilE
	fclose(in);
	return first;
}		

/*****************************************************************************
	일치하는 이름의 값을 교체한다.
*****************************************************************************/
int change_list(char *name, char *value, LINK first)
{	
	LINK cur_list;
	int ret = 0;
	cur_list = first;
	while(cur_list != NULL)
	{	
		if(cur_list->value[0] == '[')
			continue;
		else
		{
			if(strncmp(cur_list->name, name, strlen(name)+1) == 0 )
			{
				memset(cur_list->value, 0x00, sizeof(cur_list->value));
				strncpy(cur_list->value, value, strlen(value));
				ret = 1;
				break;
			}
			else ret = -1;
		}
		cur_list = cur_list->next; 
	}
	return ret;
}


/*****************************************************************************
	리스트 맨 마지막에 데이터를 삽입한다.
*****************************************************************************/
LINK add_to_list(char *name, char *value, LINK first )//queue put
{
	LINK current = first;
	LINK new_list = new CONFFILE();

	strncpy(new_list->name   ,name   ,strlen(name   ));
	strncpy(new_list->value  ,value  ,strlen(value  ));

	new_list->next = NULL;
	if(current == NULL)
	{
		first = new_list;
		first->next = NULL;
	}
	else
	{
		while(current->next != NULL)
		{
			current = current->next;
		}	
		current->next = new_list;
	}
	return first;
}

/*****************************************************************************/
void delete_list(char *name, LINK first)
{
	LINK tmp_list;
	LINK cur_list;
//	FILE	*in;
	cur_list = first;
	tmp_list = cur_list->next;	
	
	while(tmp_list != NULL)
	{
	    if(tmp_list->name[0] == '[')
			continue;
		else
		{
			if(strncmp(tmp_list->name,name,strlen(name)+1) == 0 )
			{
				cur_list->next = tmp_list->next ;
				delete tmp_list;
			}
		}
		cur_list = cur_list->next;
		tmp_list = cur_list->next;
	}
}

/*****************************************************************************
	일치하는 이름의 값을 읽어온다..
*****************************************************************************/
char *get_list_value(char *name, LINK first)
{
	LINK cur_list;	
	cur_list = first;
	while(cur_list != NULL)
	{
		if(cur_list->name[0] == '[')
			continue;
		else
		{
			if(strncmp(cur_list->name,name,strlen(name)+1) == 0)
			return cur_list->value;
		}
		cur_list = cur_list->next;
	}
	return NULL;
}	

/*****************************************************************************
	리스트를 파일로 저장한다.
*****************************************************************************/
void write_list_file(char *filename,LINK first)
{
	FILE *in;
	LINK cur_list;
	cur_list = first;
	in = fopen(filename,"w");
	if(in == NULL)
		return;
	while(cur_list != NULL)  
	{	
		if(cur_list->name[0] == '[')
		{
			fprintf(in,"%s\n",cur_list->name);
		}
		else
		{
			fprintf(in,"%s=%s\n",cur_list->name,cur_list->value);
		}
		cur_list = cur_list->next;
	}

	fclose(in);
}

/*****************************************************************************
	first부터 마지막까지 clear 시킨다.
*****************************************************************************/
void free_memory_list(LINK first)
{
	LINK cur_list,next;
	cur_list = first;
	while(cur_list != NULL)
	{
		next = cur_list->next;
		memset(cur_list->name, 0x00, sizeof(cur_list->name));
		memset(cur_list->value,0x00, sizeof(cur_list->value));
		delete cur_list;
		cur_list = next;
	} 
}
	

/*****************************************************************************/
int ReadConf(char *filename, char *name, char *value,int size )
{	
	LINK first;
	char *temp;
	int len;

    first = NULL;
	
	memset(value, 0, size);
	
	first = read_list_file(filename,first);
	
	if (first == NULL) return -1;
    
    temp = get_list_value(name,first);
	if (temp == NULL) return -1;

	len = strlen(temp);
	if (len >= size) len = size -1;

	//strncpy(value,get_list_value(name,first),len);
	strncpy(value,temp,len);			// Code sonar 지적사항 수정. 2008.04.16 PSC
	free_memory_list(first);

	return 1;
}

/*****************************************************************************/
int WriteConf(char *filename, char *name, char *value)
{
	LINK first;

	int ret = 0;

	first = NULL;

	first = read_list_file(filename,first);
	ret = change_list(name,value,first);

	if(ret <= 0){
		// 절대 NULL 을 패러메터로 던지지 마라 .. 하루 깨졌다.. 흑흑 ...
		// first = add_to_list(name,value,NULL,first);
		first = add_to_list(name,value,first);
	}

    write_list_file(filename,first);
	free_memory_list(first);

	return ret;

}

/*****************************************************************************
	일치하는 섹션을 찾는다.
*****************************************************************************/
LINK find_section (char *Section, LINK first)
{
	LINK cur_list;
	cur_list = first;

	while(cur_list != NULL)
	{	
		if(cur_list->name[0] == '[')
		{
			if(strncmp(&cur_list->name[1], Section, strlen(Section)) == 0 )
				return cur_list;
		}
		cur_list = cur_list->next; 
	}
	return NULL;
}

/*****************************************************************************
	일치하는 키를 찾는다. 모드에 따라 섹션이 무시된다.
*****************************************************************************/
LINK find_key (char mode, char *Key, LINK first )
{
	LINK cur_list;	
	cur_list = first;

	while(cur_list != NULL)
	{
		if(cur_list->name[0] == '[')
		{
			if(mode)
				return NULL;
		}
		else
		{
			if(strncmp(cur_list->name, Key, strlen(Key)) == 0)
			return cur_list;
		}
		cur_list = cur_list->next; 
	}
	return NULL;
}

/*****************************************************************************
	section에 바로 다음에 리스트와 값을 삽입한다.
*****************************************************************************/
LINK add_to_section(char *name, char *value, LINK first )//queue put
{
	LINK new_list = new CONFFILE();

	strncpy(new_list->name   ,name   ,strlen(name   ));
	strncpy(new_list->value  ,value  ,strlen(value  ));

	new_list->next = first->next;
	first->next = new_list;

	return first;
}

