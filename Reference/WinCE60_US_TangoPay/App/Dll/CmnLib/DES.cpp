#include "stdafx.h"
#include ".\Common\CmnLib.h"

//#define NH_DEBUG		// Debug Message On/Off

#include ".\Common\NHDbgApi.h"
#include <Tlhelp32.h>	// [#2095] NH KSK 2011.10.28

// [#2515] US Justin 2017.11.09 Mastercard phase2 development : Need Encryption/Decryption using Private and Public Keys
#include <winsock2.h>		// Surpressing "warning C4005: 'xxxx' : macro redefinition"
#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>

#include "openssl_typ_int.h"
// End of [#2515]

#define	DBG_CALL		1
#define DBG_INFO		1

void DES_rand(BYTE* buffer, int length)
{
	int iters = 0;
	while (RAND_bytes(buffer, length) != 1)
	{
		NHERROR((L"Random number (iv) failed\r\n"));
		Delay_Msg(250);

		if (5 == iters++)
		{
			// Fall back to insecure number generation
			for (int i = 0; i < length; i++)	
			{
				buffer[i] = (BYTE)(rand() % 255 + 1);  // Compliant: we try to do CSPRNG, first
			}

			break;
		}
	}
}

int WINAPI base64_decode(char* input, int inputlen, unsigned char* dst, int dstlen)
{
	int outlen = 0;
	BIO *b64 = BIO_new(BIO_f_base64());
	BIO *inputBio = BIO_new(BIO_s_mem());

	BIO_write(inputBio, input, inputlen);
	inputBio = BIO_push(b64, inputBio);
	BIO_set_flags(inputBio, BIO_FLAGS_BASE64_NO_NL);
	outlen = BIO_read(inputBio, dst, dstlen);

	BIO_free_all(inputBio);

	return outlen;
}

int WINAPI base64_encode(char* input, int inputlen, char* dst, int dstlen)
{
	int outlen = 0;
	BIO *b64 = BIO_new(BIO_f_base64());
	BIO *outputBio = BIO_new(BIO_s_mem());

	BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

	BIO_push(b64, outputBio);
	BIO_write(b64, input, inputlen);
	BIO_flush(b64);

	outlen = BIO_read(outputBio, dst, dstlen);

	BIO_free_all(outputBio);

	return outlen;
}

// [#2292] US Justin 2014.09.19 Add TDL TLV Logic
int	WINAPI TLV_FillLength(BYTE *pTargetBuffer, int nLen)
{
	int nBufLoc = 0;
	if(nLen>65536) 		// Do not support bigger than 65,536 (256*256)
		return nBufLoc;

	if(nLen>127)
	{
		if(nLen>255)
		{
			pTargetBuffer[nBufLoc++] = 0X82;
			pTargetBuffer[nBufLoc++] = (int) (1.0 * nLen / 256); 
			pTargetBuffer[nBufLoc++] = nLen % 256;
		}
		else
		{
			pTargetBuffer[nBufLoc++] = 0X81;
			pTargetBuffer[nBufLoc++] = nLen;
		}
	}
	else
		pTargetBuffer[nBufLoc++] = nLen;

	return nBufLoc;
}

int	WINAPI TLV_FillValueString(BYTE *pTargetBuffer, CString strValue)
{
	// Fill Length
	int nLen = TLV_FillLength(pTargetBuffer, strValue.GetLength());

	// Fill Value
	for(int i=0; i<strValue.GetLength(); i++)
		pTargetBuffer[nLen++] = (char) strValue.GetAt(i);
	return nLen;
}


int WINAPI TLV_FillValueBuffer(BYTE *pTargetBuffer, BYTE *pBufValue, int nLenValue)
{
	// Fill Length
	int nLen = TLV_FillLength(pTargetBuffer, nLenValue);
	
	// Fill Value
	memcpy(&pTargetBuffer[nLen], pBufValue, nLenValue);

	return (nLen + nLenValue);
}

int WINAPI TLV_GetTagIDandType(BYTE *pTargetBuffer, int*nTagType, int *nBufLoc)
{
	int nTagID = 0;
	int nDataSize = 0;

	if(pTargetBuffer[0] == 0XFF)
		*nTagType = 1; // _TDL_TAGTYPE_CONSTRUCTIVE;
	else if(pTargetBuffer[0] == 0XDF)
		*nTagType = 0; //_TDL_TAGTYPE_PRIMITIVE;
	else
		return nTagID;

	// Check multiple length
	if(0X80 & pTargetBuffer[1])				// if the b8 is set, it has two byte Tag
	{
		nTagID = (pTargetBuffer[1] & 0X7F) * 128 + pTargetBuffer[2];	
		nDataSize = 3;
	}
	else
	{
		nTagID = pTargetBuffer[1];
		nDataSize = 2;
	}

	*nBufLoc += nDataSize;
	return nTagID;	
}

int WINAPI TLV_GetDataLength(BYTE *pTargetBuffer, int *nBufLoc)
{
	int	nDataLength = 0;
	int	nDataSize = 1;

	// Check multiple length
	if(0X80 & pTargetBuffer[0])				// if the b8 is set, this byte has Data byte Size
	{
		nDataSize = (0X7F & pTargetBuffer[0]);	// Data Size
		if(nDataSize==1)
			nDataLength = pTargetBuffer[1];
		else if(nDataSize == 2)
			nDataLength = pTargetBuffer[1]*256 + pTargetBuffer[2];
		nDataSize += 1;
	}
	else
		nDataLength = pTargetBuffer[0];

	*nBufLoc += nDataSize;
	return nDataLength;
}

int	WINAPI TLV_GetDataString(BYTE *pTargetBuffer, CString *strData, int nLen)
{
	// Change to Wide Characters 
	WCHAR* wchTmp = new TCHAR[nLen+1]();
	MultiToWide( wchTmp, (char*) pTargetBuffer, nLen);
	CString strTemp = CString(wchTmp);
	strData->Format( L"%s", strTemp.Left(nLen) );
	delete[] wchTmp;
	return nLen;
}

// "12.3" => 1230 / "12." => 1200 / "12" => "1200" / ".01" => 1 / "0.12" => 12 / "12.3456" => 1234
int WINAPI Dollar2Cent(LPCTSTR lpvoid)
{
	char chTemp[1024] = { 0, };
	WideToMulti(chTemp, lpvoid, sizeof(chTemp));
	int nCentValue = (int) (atof(chTemp) * 100 + 0.0001);
	return nCentValue;
}
// End of [#2292]


CString WINAPI GetCenteredString(CString strIn, int nMaxCol)
{
	// 1. Max Col - str > 0
	int nLeftPadding = 0;
	if( (nLeftPadding = nMaxCol - strIn.GetLength()) > 0 )
	{
		nLeftPadding /= 2;
		CString strRet(_T(""));
		strRet.Format( _T("%*.*s%s"), nLeftPadding, nLeftPadding, _T(" "), strIn );
		return strRet;
	}
	// 2. Max Col - str <= 0
	else
	{
		return strIn.Left( nMaxCol );
	}
}

// [#2557] US Justin 2018.07.24
CString	WINAPI GetDistributedString(CString strIn1, CString strIn2, int nMaxCol)
{
	CString strReturn = L"";
	CString	strAllSpaces = L"                                                            ";

	int nTotalLen = strIn1.GetLength() + strIn2.GetLength();
	if( nTotalLen > nMaxCol )	strReturn = strIn1 + strIn2;
	else						strReturn.Format( L"%s%s%s", strIn1, strAllSpaces.Left(nMaxCol-nTotalLen) ,strIn2);	
	return strReturn;
}
// End of [#25557]

// [#2513] US Justin 
CString WINAPI GetJSONNodeValue(CString sRcv, CString sNodeName, BOOL bMultiple)
{
	CString strReceived = sRcv;
	CString strReturn = L"";
	CString strNodeName;
	strNodeName.Format(L"\"%s\"", sNodeName);

	BOOL bContinue;
	do
	{
		bContinue = FALSE;

		// Find Node Name
		int nLoc = strReceived.Find(strNodeName);
		if(nLoc<0)
			return strReturn;

		strReceived = strReceived.Mid(nLoc + strNodeName.GetLength() );
		
		// Get Value after colon
		nLoc = strReceived.Find(L":");
		if(nLoc<0)
			return strReturn;

		CString strTemp = strReceived.Mid(nLoc + 1 );
		strTemp.TrimLeft();
		if( strTemp.GetLength()<=0 )
			return strReturn;

		// Find Value
		CString strTempFirst = strTemp.Left(1);
		if( (strTempFirst==L"{") || (strTempFirst==L"[") )
		{
			CString strVal1;
			int nNumBracket = 0;
			int nNumCurlBrace = 0;
			for(int i=1; i<strTemp.GetLength(); i++)
			{
				strVal1 = strTemp.GetAt(i);

				if( strVal1 == L"{" )				nNumCurlBrace++;
				else if( strVal1 == L"[" )			nNumBracket++;
				else if( strVal1 == L"}" )
				{
					if( strTempFirst==L"{" )
					{
						if( (nNumBracket==0)&&(nNumCurlBrace==0) )
							break;
					}
					if(nNumCurlBrace>0)
						nNumCurlBrace--;
				}
				else if( strVal1 == L"]" )
				{
					if( strTempFirst==L"[" )
					{
						if( (nNumBracket==0)&&(nNumCurlBrace==0) )
							break;
					}
					if(nNumBracket>0)
						nNumBracket--;
				}
				strReturn += strVal1;
			}
		}
		else if( strTempFirst==L"\"" )	// [#J006]
		{
			CString strVal1 ;
			for(int i=1; i<strTemp.GetLength(); i++)
			{
				strVal1 = strTemp.GetAt(i);
				if(strVal1 == L"\"") 
				{
					BOOL bAddString = FALSE;
					CString strAdditional = L"\"";
					for(int j=i+1; j<strTemp.GetLength(); j++)
					{
						CString strCheck = strTemp.GetAt(j);
						if( ( strCheck == L"," ) || ( strCheck == L"}" ) )
							break;
						else if(strCheck == "\"")
						{
							bAddString = TRUE;
							break;
						}
						else
							strAdditional += strCheck;
					}
					if(bAddString==TRUE)
						strReturn += strAdditional;

					break;
				}
				strReturn += strVal1;
			}
		}
		else
		{
			CString strVal1;
			for(int i=0; i<strTemp.GetLength(); i++)
			{
				strVal1 = strTemp.GetAt(i);
				if( ( strVal1 == L"," ) || ( strVal1 == L"}" ) )
					break;
				strReturn += strVal1;
			}
		}
		strReturn.TrimLeft();
		strReturn.TrimRight();

		// Remove "
		int nLenRtn = strReturn.GetLength(); 
		if( nLenRtn > 0)
		{
			CString strBg = strReturn.Left(1);
			CString strEd = strReturn.Right(1);
			if( (strBg == L"\"") && (strEd == L"\"") )
				strReturn = strReturn.Mid(1, nLenRtn-2);
		}

		if( (strReturn.GetLength() <= 0) && ( bMultiple == TRUE) )
			bContinue = TRUE;

	} while (bContinue == TRUE );

	// [#J006] Replace JSON Escape character
	strReturn.Replace(L"\\\"", L"\"");
	strReturn.Replace(L"\\\\", L"\\");
	// End of [#J006]

	return strReturn;
}
// End of [#2513]

// [#2095] NH KSK 2011.10.27
//==============================================================================
// Function Name : GetPID
// Description   : Process ID를 가져온다.
//
// Parameters    : CString   processname        : 가져올 Process의 실행파일명
// Return Value  : DWORD Process ID
// Initial Date  : 2010.10.10
//==============================================================================
DWORD GetPID(CString processname)
{
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 pe32 = { 0, };
	CString strExeFile;

	pe32.dwSize = sizeof(PROCESSENTRY32);

	if ( !Process32First(hProcessSnap, &pe32) )
	{
#ifdef UNDER_CE
		CloseToolhelp32Snapshot(hProcessSnap);
#else
		CloseHandle(hProcessSnap);
#endif
		return false;
	}
	do
	{
		strExeFile = pe32.szExeFile;
		if ( strExeFile.CompareNoCase(processname) == 0 )
		{
#ifdef UNDER_CE
		CloseToolhelp32Snapshot(hProcessSnap);
#else
		CloseHandle(hProcessSnap);
#endif
			return pe32.th32ProcessID;
		}
	}while( Process32Next(hProcessSnap, &pe32) );

#ifdef UNDER_CE
		CloseToolhelp32Snapshot(hProcessSnap);
#else
		CloseHandle(hProcessSnap);
#endif
	return false;
}

//==============================================================================
// Function Name : ConvEncryMode2Value
// Description   : 암호화/비암호화 모드를 구분하는 BOOL 값을 레지스트리에 
//				   쓰기 위하여 암호화된 int값으로 변환한다.
//
// Parameters    : BOOL   bEncryMode        : TRUE 암호화 모드 / FALSE 비암호화 모드
// Return Value  : 암호화된 int 값
// Initial Date  : 2010.10.10
//==============================================================================
DWORD WINAPI ConvEncryMode2Value(BOOL bEncryMode)
{
	DWORD dwSeed = GetPID(_T("WINATM.EXE"));

	if(bEncryMode)
		dwSeed = dwSeed^(dwSeed >> 1);
	else 
		dwSeed = dwSeed^(dwSeed << 1);
	return dwSeed;
}
// end of [#2095]

// [#2515] US Justin 2017.11.09 Mastercard phase2 development : Need Encryption/Decryption using Private and Public Keys
int RSA_OSSL_Public_Decrypt(int flen, const unsigned char *from, unsigned char *to, RSA *rsa, int padding)
{
	BIGNUM *f, *ret;
    int i, num = 0, r = -1;
    unsigned char *buf = NULL;
    BN_CTX *ctx = NULL;

    if (BN_num_bits(rsa->n) > OPENSSL_RSA_MAX_MODULUS_BITS) {
        RSAerr(RSA_F_RSA_OSSL_PUBLIC_DECRYPT, RSA_R_MODULUS_TOO_LARGE);
        return -1;
    }

    if (BN_ucmp(rsa->n, rsa->e) <= 0) {
        RSAerr(RSA_F_RSA_OSSL_PUBLIC_DECRYPT, RSA_R_BAD_E_VALUE);
        return -1;
    }

    /* for large moduli, enforce exponent limit */
    if (BN_num_bits(rsa->n) > OPENSSL_RSA_SMALL_MODULUS_BITS) {
        if (BN_num_bits(rsa->e) > OPENSSL_RSA_MAX_PUBEXP_BITS) {
            RSAerr(RSA_F_RSA_OSSL_PUBLIC_DECRYPT, RSA_R_BAD_E_VALUE);
            return -1;
        }
    }

    if ((ctx = BN_CTX_new()) == NULL)
        goto err;
    BN_CTX_start(ctx);
    f = BN_CTX_get(ctx);
    ret = BN_CTX_get(ctx);
    num = BN_num_bytes(rsa->n);
    buf = (unsigned char *)OPENSSL_malloc(num);
    if (ret == NULL || buf == NULL) {
        RSAerr(RSA_F_RSA_OSSL_PUBLIC_DECRYPT, ERR_R_MALLOC_FAILURE);
        goto err;
    }

    /*
     * This check was for equality but PGP does evil things and chops off the
     * top '0' bytes
     */
    if (flen > num) {
        RSAerr(RSA_F_RSA_OSSL_PUBLIC_DECRYPT, RSA_R_DATA_GREATER_THAN_MOD_LEN);
        goto err;
    }

    if (BN_bin2bn(from, flen, f) == NULL)
        goto err;

    if (BN_ucmp(f, rsa->n) >= 0) {
        RSAerr(RSA_F_RSA_OSSL_PUBLIC_DECRYPT,
               RSA_R_DATA_TOO_LARGE_FOR_MODULUS);
        goto err;
    }

    if (rsa->flags & RSA_FLAG_CACHE_PUBLIC)
        if (!BN_MONT_CTX_set_locked(&rsa->_method_mod_n, rsa->lock,
                                    rsa->n, ctx))
            goto err;

    if (!rsa->meth->bn_mod_exp(ret, f, rsa->e, rsa->n, ctx,
                               rsa->_method_mod_n))
        goto err;

    i = BN_bn2binpad(ret, buf, num);

    switch (padding) {
    case RSA_PKCS1_PADDING:
        r = RSA_padding_check_PKCS1_type_1(to, num, buf, i, num);
        break;
    case RSA_X931_PADDING:
        r = RSA_padding_check_X931(to, num, buf, i, num);
        break;
    case RSA_NO_PADDING:
        memcpy(to, buf, (r = i));
        break;
	case RSA_PKCS1_OAEP_PADDING:
		r = RSA_padding_check_PKCS1_OAEP(to, num, buf, i, num, NULL, 0);
		break;
    default:
        RSAerr(RSA_F_RSA_OSSL_PUBLIC_DECRYPT, RSA_R_UNKNOWN_PADDING_TYPE);
        goto err;
    }
    if (r < 0)
        RSAerr(RSA_F_RSA_OSSL_PUBLIC_DECRYPT, RSA_R_PADDING_CHECK_FAILED);

 err:
    BN_CTX_end(ctx);
    BN_CTX_free(ctx);
    OPENSSL_clear_free(buf, num);
    return r;
}

// [#2524] US Justin 2018.01.03, Add return format ====> 0:HexDecimal str(9d2afa..), 1:BASE64, 2:Binary
CString WINAPI SHA_256_Encrypt(unsigned char *data, int len, int nFormat, unsigned char *sEncrypted)
{
	unsigned char digest[SHA256_DIGEST_LENGTH];
	SHA256_CTX ctx;
	SHA256_Init(&ctx);
	SHA256_Update(&ctx, data, len);
	SHA256_Final(digest, &ctx);

	CString strHash = L"";

	if (nFormat == 0) // HEXDECIMAL STRING
	{
		char *mdString = new char[SHA256_DIGEST_LENGTH * 2 + 1];

		for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
			sprintf(&mdString[i * 2], "%02x", (unsigned int)digest[i]);

		strHash.Format(L"%S", mdString);

		delete[] mdString;
	}
	else if (nFormat == 1) // BASE64
	{
		BYTE *pBase64Data = new BYTE[SHA256_DIGEST_LENGTH * 2];
		int nDataSize = base64_encode((char *)digest, SHA256_DIGEST_LENGTH, (char *)pBase64Data, SHA256_DIGEST_LENGTH * 2);

		strHash.Format(L"%S", pBase64Data);
		strHash = strHash.Left(nDataSize);

		delete[] pBase64Data;
	}
	else if (nFormat == 2) // BINARY
	{
		if (sEncrypted != NULL)
			memcpy(sEncrypted, digest, SHA256_DIGEST_LENGTH);
	}

	return strHash;
}
// End of [#2524]

RSA* CreateRSA(unsigned char * key, bool bIsPublicKey)
{
	RSA *rsa = NULL;
	BIO *keybio;
	keybio = BIO_new_mem_buf(key, -1);
	
	if (keybio == NULL)
		return NULL;

	if (bIsPublicKey)	rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa, NULL, NULL);
	else				rsa = PEM_read_bio_RSAPrivateKey(keybio, &rsa, NULL, NULL);
	BIO_free( keybio );
	
	if(!rsa)	NHDEBUG(DBG_INFO, (L"CreateRSA (isPublicKey=[%d]) Failed\n", bIsPublicKey ) );
	else		NHDEBUG(DBG_INFO, (L"CreateRSA (isPublicKey=[%d]) OK return\n", bIsPublicKey) );
	return rsa;
}

int Encrypt_With_AsymKey(unsigned char * data, int data_len, unsigned char * key, unsigned char *encrypted, bool bIsPublicKey)
{
	int nRtn;
	RSA * rsa = CreateRSA(key, bIsPublicKey);
	if(!rsa)
		return -1;

	if(bIsPublicKey) 	
		nRtn = RSA_public_encrypt(data_len, data, encrypted, rsa, RSA_PKCS1_OAEP_PADDING);
	else				
		nRtn = RSA_private_encrypt(data_len, data, encrypted, rsa, RSA_PKCS1_PADDING);				// PIN4 Does not use this case => NEED WORK AROUND

	NHDEBUG(DBG_INFO, (L"Encrypt_With_AsymKey (datalen=[%d], isPublicKey=[%d]) : return = [%d]\n", data_len, bIsPublicKey, nRtn) );
	RSA_free(rsa);
	return nRtn;
}

int Decrypt_With_AsymKey(unsigned char * enc_data, int data_len, unsigned char * key, unsigned char *decrypted, bool bIsPublicKey)
{
	int nRtn;
	RSA * rsa = CreateRSA(key, bIsPublicKey);
	if(!rsa)
		return -1;

	if(bIsPublicKey) 	
		nRtn = RSA_OSSL_Public_Decrypt(data_len, enc_data, decrypted, rsa, RSA_PKCS1_OAEP_PADDING);	// OPENSSL does not support RSA_PKCS1_OAEP_PADDING for this... NEED TO WORK AROUND
		// nRtn = RSA_public_decrypt(data_len, enc_data, decrypted, rsa, RSA_PKCS1_PADDING);		
	else				
		nRtn = RSA_private_decrypt(data_len, enc_data, decrypted, rsa, RSA_PKCS1_OAEP_PADDING);		// PIN4 Does not use this case

	NHDEBUG(DBG_INFO, (L"Decrypt_With_AsymKey (datalen=[%d], isPublicKey = [%d]) : return = [%d]\n", data_len, bIsPublicKey, nRtn) );
	RSA_free(rsa);
	return nRtn;
}

int AES_Encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key, unsigned char *iv, unsigned char *ciphertext)
{
	EVP_CIPHER_CTX *ctx = NULL;
	int len = 0, ciphertext_len = 0;

	bool bSuccess = true;

	// Create and initialise the context
	if (!(ctx = EVP_CIPHER_CTX_new()))
		bSuccess = false;

	// Initialise the encryption operation.
	if (bSuccess)
	{
		if( 1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) )
			bSuccess = false;
	}

	// Provide the message to be encrypted, and obtain the encrypted output. EVP_EncryptUpdate can be called multiple times if necessary
	if (bSuccess)
	{
		if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
			bSuccess = false;
		else
			ciphertext_len = len;
	}

	// Finalise the encryption. Normally ciphertext bytes may be written at this stage, but this does not occur in GCM mode
	if (bSuccess)
	{
		if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
			bSuccess = false;
		else
			ciphertext_len += len;
	}

	// Clean up
	EVP_CIPHER_CTX_free(ctx);

	if(bSuccess)
	{
		NHDEBUG(DBG_INFO, (L"AES_Encrypt SUCCESS, datalen=[%d], encrypted_len=[%d]\n", plaintext_len, ciphertext_len));
		return ciphertext_len;
	}
	else
	{
		NHDEBUG(DBG_INFO, (L"AES_Encrypt FAILURE, datalen=[%d]\n", plaintext_len));
		return -1;
	}
}

int WINAPI AES_Encrypt_128_ECB(unsigned char *plaintext, int plaintext_len, unsigned char *key, unsigned char *iv, unsigned char *ciphertext)
{
	EVP_CIPHER_CTX *ctx = NULL;
	int len = 0, ciphertext_len = 0;

	bool bSuccess = true;

	// Create and initialise the context
	if (!(ctx = EVP_CIPHER_CTX_new()))
		bSuccess = false;

	// Initialise the encryption operation.
	if (bSuccess)
	{
		if( 1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, key, iv) )
			bSuccess = false;
	}

	// Provide the message to be encrypted, and obtain the encrypted output. EVP_EncryptUpdate can be called multiple times if necessary
	if (bSuccess)
	{
		if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
			bSuccess = false;
		else
			ciphertext_len = len;
	}

	// Finalise the encryption. Normally ciphertext bytes may be written at this stage, but this does not occur in GCM mode
	if (bSuccess)
	{
		if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
			bSuccess = false;
		else
			ciphertext_len += len;
	}

	// Clean up
	EVP_CIPHER_CTX_free(ctx);

	if(bSuccess)
	{
		NHDEBUG(DBG_INFO, (L"AES_Encrypt SUCCESS, datalen=[%d], encrypted_len=[%d]\n", plaintext_len, ciphertext_len));
		return ciphertext_len;
	}
	else
	{
		NHDEBUG(DBG_INFO, (L"AES_Encrypt FAILURE, datalen=[%d]\n", plaintext_len));
		return -1;
	}
}

int AES_Decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key, unsigned char *iv, unsigned char *plaintext)
{
	EVP_CIPHER_CTX *ctx = NULL;
	int len = 0, plaintext_len = 0;
	int ret = 0;
	bool bSuccess = true;

	// Create and initialise the context 
	if (!(ctx = EVP_CIPHER_CTX_new()))
		bSuccess = false;

	// Initialise the decryption operation.
	if (bSuccess)
	{
		if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) )
			bSuccess = false;
	}

	// Provide the message to be decrypted, and obtain the plaintext output. EVP_DecryptUpdate can be called multiple times if necessary
	if (bSuccess)
	{
		if (!EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
			bSuccess = false;
		else
			plaintext_len = len;
	}

	// Finalise the decryption. Further plaintext bytes may be written at this stage.
	if (bSuccess)
	{
		if (1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
			bSuccess = false;
		else
			plaintext_len += len;
	}

	// Clean up
	EVP_CIPHER_CTX_free(ctx);

	if (bSuccess)
	{
		NHDEBUG(DBG_INFO, (L"AES_Decrypt SUCCESS, datalen=[%d], decrypted_len=[%d]\n", ciphertext_len, plaintext_len));
		return plaintext_len;
	}
	else
	{
		NHDEBUG(DBG_INFO, (L"AES_Encrypt FAILURE, datalen=[%d]\n", ciphertext_len));
		return -1;
	}
}

int WINAPI AES_Decrypt_128_ECB(unsigned char *ciphertext, int ciphertext_len, unsigned char *key, unsigned char *iv, unsigned char *plaintext)
{
	EVP_CIPHER_CTX *ctx = NULL;
	int len = 0, plaintext_len = 0;
	int ret = 0;
	bool bSuccess = true;

	// Create and initialise the context 
	if (!(ctx = EVP_CIPHER_CTX_new()))
		bSuccess = false;

	// Initialise the decryption operation.
	if (bSuccess)
	{
		if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, key, iv) )
			bSuccess = false;
	}

	// Provide the message to be decrypted, and obtain the plaintext output. EVP_DecryptUpdate can be called multiple times if necessary
	if (bSuccess)
	{
		if (!EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
			bSuccess = false;
		else
			plaintext_len = len;
	}

	// Finalise the decryption. Further plaintext bytes may be written at this stage.
	if (bSuccess)
	{
		if (1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
			bSuccess = false;
		else
			plaintext_len += len;
	}

	// Clean up
	EVP_CIPHER_CTX_free(ctx);

	if (bSuccess)
	{
		NHDEBUG(DBG_INFO, (L"AES_Decrypt SUCCESS, datalen=[%d], decrypted_len=[%d]\n", ciphertext_len, plaintext_len));
		return plaintext_len;
	}
	else
	{
		NHDEBUG(DBG_INFO, (L"AES_Encrypt FAILURE, datalen=[%d]\n", ciphertext_len));
		return -1;
	}
}

CString	WINAPI Encrypt_And_Base64Encode(LPCTSTR sOrgData, unsigned char * key, int nKeyType, unsigned char* ivIN)			// [#2535]
{
	CString strData = CString(sOrgData);
	NHDEBUG(DBG_INFO, (L"Encrypt_And_Base64Encode: data len=[%d], nKeyType = [%d]\n", strData.GetLength(), nKeyType) );

	CString strReturn = L"";
	if(key==NULL)
	{
		NHDEBUG(DBG_INFO, (L"ERROR : Key is NULL ==> return original data [%s]\n", strData) );
		return strData;
	}

	// Assign Value
	char *pBuffer = new char[strData.GetLength()+1]();
	WideToMulti( pBuffer, strData, strData.GetLength());

	// Encrypting
	int nConvertedLen = 0;
	unsigned char  encrypted[4098] = {};
	
	if( nKeyType==0 || nKeyType==1 )	// nKeyType = 0:Private, 1:Public
	{
		bool bIsPublicKey;
		if( nKeyType==1 )				bIsPublicKey = true;
		else							bIsPublicKey = false;
		nConvertedLen = Encrypt_With_AsymKey((unsigned char*)pBuffer, strData.GetLength(), key, encrypted, bIsPublicKey);
	}
	else								// nKeyType = 2:Symmetric, 3:Symmetric w/Separate IV
	{
		unsigned char enc_wo_iv[4096] = {};
		unsigned char iv[16] = {};	
		
		if( nKeyType==2 )
		{
			DES_rand(iv, sizeof(iv));
		}
		else if( nKeyType==3 )
		{
			memcpy(iv, ivIN, 16);
		}

		// Encrypt with key and IV
		nConvertedLen = AES_Encrypt((unsigned char*)pBuffer, strData.GetLength(), key, iv, enc_wo_iv);

		if(nConvertedLen>0)
		{
			// [#2535] US Justin 2018.03.05 GivePay Online
			/*
			memcpy(encrypted, iv, 16);							// Fill 16 byte IV
			memcpy(encrypted+16, enc_wo_iv, nConvertedLen);		// Fill encrypted data
			nConvertedLen += 16;
			*/
			if(nKeyType==2)		// Combine IV and encrypted data
			{
				memcpy(encrypted, iv, 16);							// Fill 16 byte IV
				memcpy(encrypted+16, enc_wo_iv, nConvertedLen);		// Fill encrypted data (IV is attached in front)
				nConvertedLen += 16;
			}
			else if (nKeyType==3)
			{
				memcpy(encrypted, enc_wo_iv, nConvertedLen);		// encrypted data only
			}
			// End of [#2535]
		}
	}
	delete[] pBuffer;

	// Base64 Encoding
	if (nConvertedLen > 0)
	{
		BYTE* pBase64Data = new BYTE[nConvertedLen*2];
		int nDataSize = base64_encode( (char*) encrypted, nConvertedLen, (char*) pBase64Data, nConvertedLen*2);
		strReturn.Format(L"%S", pBase64Data);
		strReturn = strReturn.Left(nDataSize);
		delete [] pBase64Data;
	}
	else					
		strReturn = strData;		

	NHDEBUG(DBG_INFO, (L"Encrypt_And_Base64Encode: return [%s]\n", strReturn) );
	return strReturn;
}

CString	WINAPI Encrypt_And_Base64Encode_AES_128_ECB(LPCTSTR sOrgData, unsigned char * key, int nKeyType, unsigned char* ivIN)			// [#2535]
{
	CString strData = CString(sOrgData);
	NHDEBUG(DBG_INFO, (L"Encrypt_And_Base64Encode: data len=[%d], nKeyType = [%d]\n", strData.GetLength(), nKeyType) );

	CString strReturn = L"";
	if(key==NULL)
	{
		NHDEBUG(DBG_INFO, (L"ERROR : Key is NULL ==> return original data [%s]\n", strData) );
		return strData;
	}

	// Assign Value
	char *pBuffer = new char[strData.GetLength()+1]();
	WideToMulti( pBuffer, strData, strData.GetLength());

	// Encrypting
	int nConvertedLen = 0;
	unsigned char  encrypted[4098] = {};
	
	if( nKeyType==0 || nKeyType==1 )	// nKeyType = 0:Private, 1:Public
	{
		bool bIsPublicKey;
		if( nKeyType==1 )				bIsPublicKey = true;
		else							bIsPublicKey = false;
		nConvertedLen = Encrypt_With_AsymKey((unsigned char*)pBuffer, strData.GetLength(), key, encrypted, bIsPublicKey);
	}
	else								// nKeyType = 2:Symmetric, 3:Symmetric w/Separate IV
	{
		unsigned char enc_wo_iv[4096] = {};
		unsigned char iv[16] = {};	
		
		if( nKeyType==2 )
		{
			DES_rand(iv, sizeof(iv));
		}
		else if( nKeyType==3 )
		{
			memcpy(iv, ivIN, 16);
		}

		// Encrypt with key and IV
		nConvertedLen = AES_Encrypt_128_ECB((unsigned char*)pBuffer, strData.GetLength(), key, iv, enc_wo_iv);

		if(nConvertedLen>0)
		{
			// [#2535] US Justin 2018.03.05 GivePay Online
			/*
			memcpy(encrypted, iv, 16);							// Fill 16 byte IV
			memcpy(encrypted+16, enc_wo_iv, nConvertedLen);		// Fill encrypted data
			nConvertedLen += 16;
			*/
			if(nKeyType==2)		// Combine IV and encrypted data
			{
				memcpy(encrypted, iv, 16);							// Fill 16 byte IV
				memcpy(encrypted+16, enc_wo_iv, nConvertedLen);		// Fill encrypted data (IV is attached in front)
				nConvertedLen += 16;
			}
			else if (nKeyType==3)
			{
				memcpy(encrypted, enc_wo_iv, nConvertedLen);		// encrypted data only
			}
			// End of [#2535]
		}
	}
	delete[] pBuffer;

	// Base64 Encoding
	if (nConvertedLen > 0)
	{
		BYTE* pBase64Data = new BYTE[nConvertedLen*2];
		int nDataSize = base64_encode( (char*) encrypted, nConvertedLen, (char*) pBase64Data, nConvertedLen*2);
		strReturn.Format(L"%S", pBase64Data);
		strReturn = strReturn.Left(nDataSize);
		delete [] pBase64Data;
	}
	else					
		strReturn = strData;		

	NHDEBUG(DBG_INFO, (L"Encrypt_And_Base64Encode: return [%s]\n", strReturn) );
	return strReturn;
}

int WINAPI Base64Decode_And_Decrypt(LPCTSTR sOrgData, unsigned char * key, int nKeyType, unsigned char*decrypted, unsigned char* ivIN)		// [#2535]
{
	CString strData = CString(sOrgData);
	NHDEBUG(DBG_INFO, (L"Base64Decode_And_Decrypt: data len=[%d], keytype = [%d]\n", strData.GetLength(), nKeyType) );

	if(key==NULL)
	{
		NHDEBUG(DBG_INFO, (L"ERROR : Key is NULL \n") );
		return -1;
	}

	// Assign Value
	int inputlen = strData.GetLength() + 1;
	char* pBase64Data     = new char[inputlen]();
	WideToMulti( (char*)pBase64Data, strData, strData.GetLength() );

	// Base 64 Decoding
	char* pEncryptedData  = new char[strData.GetLength()]();
	int nBinaryDataSize = base64_decode( (char*)pBase64Data, inputlen, (unsigned char*) pEncryptedData, strData.GetLength() );
	delete [] pBase64Data;

	// Decrypting
	int nConvertedLen;	
	if( nKeyType==0 || nKeyType==1 )	// nKeyType = 0:Private, 1:Public
	{
		bool bIsPublicKey;
		if( nKeyType==1 )				bIsPublicKey = true;
		else							bIsPublicKey = false;
		nConvertedLen = Decrypt_With_AsymKey((unsigned char*)pEncryptedData, nBinaryDataSize, key, decrypted, bIsPublicKey);
	}
	else								// nKeyType = 2:Symmetric, 3:Symmetric w/Separating IV
	{
		unsigned char iv[EVP_MAX_IV_LENGTH] = {};		
		int nSzIV = 0;

		if(nKeyType==2)
		{
			// Assign IV : The First 16 bytes is IV
			if(nBinaryDataSize>16)
			{
				memcpy_s(iv, sizeof(iv), pEncryptedData, 16);
				nBinaryDataSize -= 16;
				nSzIV = 16;			// Exclude IV(First 16 bytes) when decrypt.
			}		
		}
		else if(nKeyType==3)
		{
			// Assign IV
			memcpy(iv, ivIN, 16);
			nSzIV = 0;
		}

		nConvertedLen = AES_Decrypt((unsigned char*)(pEncryptedData + nSzIV), nBinaryDataSize, key, iv, decrypted);
		// End of [#2535]
	}
	delete [] pEncryptedData;

	NHDEBUG(DBG_INFO, (L"Base64Decode_And_Decrypt: return length=[%d]\n", nConvertedLen ));
	return nConvertedLen;
}

int WINAPI Base64Decode_And_Decrypt_AES_128_ECB(LPCTSTR sOrgData, unsigned char * key, int nKeyType, unsigned char*decrypted, unsigned char* ivIN)		// [#2535]
{
	CString strData = CString(sOrgData);
	NHDEBUG(DBG_INFO, (L"Base64Decode_And_Decrypt: data len=[%d], keytype = [%d]\n", strData.GetLength(), nKeyType) );

	if(key==NULL)
	{
		NHDEBUG(DBG_INFO, (L"ERROR : Key is NULL \n") );
		return -1;
	}

	// Assign Value
	int inputlen = strData.GetLength() + 1;
	char* pBase64Data     = new char[inputlen]();
	WideToMulti( (char*)pBase64Data, strData, strData.GetLength() );

	// Base 64 Decoding
	char* pEncryptedData  = new char[strData.GetLength()]();
	int nBinaryDataSize = base64_decode( (char*)pBase64Data, inputlen, (unsigned char*) pEncryptedData, strData.GetLength() );
	delete [] pBase64Data;

	// Decrypting
	int nConvertedLen;	
	if( nKeyType==0 || nKeyType==1 )	// nKeyType = 0:Private, 1:Public
	{
		bool bIsPublicKey;
		if( nKeyType==1 )				bIsPublicKey = true;
		else							bIsPublicKey = false;
		nConvertedLen = Decrypt_With_AsymKey((unsigned char*)pEncryptedData, nBinaryDataSize, key, decrypted, bIsPublicKey);
	}
	else								// nKeyType = 2:Symmetric, 3:Symmetric w/Separating IV
	{
		unsigned char iv[16] = {};		
		int nSzIV = 0;

		// [#2535] US Justin 2018.03.05 GivePay Online
		/*		
		// The First 16 bytes is IV...=> Assign IV / remove IV when decrypt.
		if(nBinaryDataSize>16)
		{
			memcpy(iv, pEncryptedData, 16);
			nBinaryDataSize -= 16;
			nSzIV = 16;
		}
		*/

		if(nKeyType==2)
		{
			// Assign IV : The First 16 bytes is IV
			if(nBinaryDataSize>16)
			{
				memcpy(iv, pEncryptedData, 16); // Compliant: the length of pEncryptedData == nBinaryDataSize
				nBinaryDataSize -= 16;
				nSzIV = 16;			// Exclude IV(First 16 bytes) when decrypt.
			}		
		}
		else if(nKeyType==3)
		{
			// Assign IV
			memcpy(iv, ivIN, 16);
			nSzIV = 0;
		}

		nConvertedLen = AES_Decrypt_128_ECB((unsigned char*)(pEncryptedData + nSzIV), nBinaryDataSize, key, iv, decrypted);
		// End of [#2535]
	}
	delete [] pEncryptedData;

	NHDEBUG(DBG_INFO, (L"Base64Decode_And_Decrypt: return length=[%d]\n", nConvertedLen ));
	return nConvertedLen;
}

CString WINAPI Cal_Signature_And_Base64Encode(LPCTSTR sOrgData, unsigned char *prvkey, long key_len, int nKeyFormat)
{
	NHDEBUG(DBG_INFO, (L"Cal_Signature_And_Base64Encode: sOrgData=[%s]\n", sOrgData) );
	CString strReturn = L"";
	CString strData = CString(sOrgData);

	if(prvkey==NULL)
	{
		NHDEBUG(DBG_INFO, (L"ERROR : Private Key is NULL ==> return original data [%s]\n", strData) );
		return strData;
	}

	// Create RSA
	RSA * rsa = NULL;
	EVP_PKEY *pkey = NULL;
	if(nKeyFormat==0)		// PEM Format
	{
		rsa = CreateRSA(prvkey, 0);
		if(rsa == NULL)
		{
			NHDEBUG(DBG_INFO, (L"ERROR : RSA Creation Error using PEM formatted Key ==> return original data [%s]\n", strData) );
			return strData;
		}
	}
	else					// DER Format
	{
		const unsigned char *prv_key = prvkey;
		pkey = d2i_PrivateKey(EVP_PKEY_RSA, NULL, &prv_key, key_len);
		if (pkey == NULL)
		{
			NHDEBUG(DBG_INFO, (L"ERROR : Read RSA Private Key Fail (d2i_PrivateKey) return original data [%s]\n", strData) );
			return strData;
		}

		rsa = EVP_PKEY_get1_RSA(pkey);
		if (rsa == NULL)
		{
			NHDEBUG(DBG_INFO, (L"ERROR : Get RSA from EVP_Key Fail (EVP_PKEY_get1_RSA) return original data [%s]\n", strData) );
			EVP_PKEY_free(pkey);
			return strData;
		}
	}

	// Convert the wide string
	char* pMsgBody = new char[ strData.GetLength()+1 ]();
	WideToMulti( pMsgBody, strData, strData.GetLength());

	// SHA 256 of Message
	unsigned char pDigest[SHA256_DIGEST_LENGTH] = {};
	CString strTemp = SHA_256_Encrypt((unsigned char *)pMsgBody, strData.GetLength(), 2, pDigest);
	delete [] pMsgBody;

	// Calculate Signature of the SHA256
	unsigned int siglen = 0;
	unsigned char signature[4098] = {};
	int result = RSA_sign(NID_sha256, pDigest, SHA256_DIGEST_LENGTH, signature, &siglen, rsa);

	// Clear Buffer
	if(pkey!=NULL)			
		EVP_PKEY_free(pkey);
	RSA_free(rsa);

	// Base64 Encoding
	if (result > 0)
	{
		BYTE* pBase64Data = new BYTE[siglen*2]();
		int nDataSize = base64_encode( (char*) signature, siglen, (char*) pBase64Data, siglen*2);
		strReturn = CString (pBase64Data);
		strReturn = strReturn.Left(nDataSize);
		delete [] pBase64Data;
	}
	else					
	{
		NHDEBUG(DBG_INFO, (L"Signing Failure return Original Data\n") );
		strReturn = strData;		
	}
	NHDEBUG(DBG_INFO, (L"Cal_Signature_And_Base64Encode: Successful. Length = [%d]\n", strReturn.GetLength()) );
	return strReturn;
}
// End of [#2515]

// [#2535] US Justin 2018.03.05 GivePay Online
int WINAPI GetKeyFromSharedKey(unsigned char *out, char *shared, int sharedLen, unsigned char *salt, int saltLen, int iterations, int length, int padding)
{
	int resultLength = 0;
	unsigned char *result = new unsigned char[length + padding]();
	if (PKCS5_PBKDF2_HMAC_SHA1(shared, sharedLen, salt, saltLen, iterations, length+padding, result) != 0)
	{
		for (int i=0; i < length; i++)
			out[i] = result[i + padding];
		resultLength = length;
	}
	delete [] result;

	NHDEBUG(DBG_INFO, (L"GetKeyFromSharedKey: return Length = [%d]\n",resultLength) );
	return resultLength;
}
// End of [#2535]


// [RWC6-676] Start SKKim 2024.04.08
static float u[98], c, cd, cm;
static unsigned short i97, j97;
static unsigned short test = 0;

void rmarin(unsigned short ij, unsigned short kl)
{
	unsigned short i, j, k, l, ii, jj, m;
	float s, t;

	if (ij<0 || ij>31328 || kl<0 || kl>30081)
	{
		return;
	}

	i = (ij/177)%177 + 2;
	j = ij%177 + 2;
	k = (kl/169)%178 + 1;
	l = kl%169;

	for (ii=1; ii<=97; ii++)
	{
		s = 0.0;
		t = 0.5;
		for (jj=1; jj<=24; jj++)
		{
			m = (((i*j)%179)*k) % 179;
			i = j;
			j = k;
			k = m;
			l = (53*l + 1) % 169;
			if ((l*m)%64 >= 32) s += t;
			t *= 0.5;
		}
		u[ii] = s;
	}

	c = 362436.0 / 16777216.0;
	cd = 7654321.0 / 16777216.0;
	cm = 16777213.0 / 16777216.0;

	i97 = 97;
	j97 = 33;

	test = 1;
}

void ranmar(float rvec[], unsigned short len)
{
	unsigned short ivec;
	float uni;

	if (test == (unsigned short)0) {
		return;
	}
	for (ivec=1; ivec<=len; ivec++)
	{
		uni = u[i97] - u[j97];
		if (uni < 0.0) uni += 1.0;
		u[i97] = uni;
		i97--;
		if (i97==0) i97 = 97;
		j97--;
		if (j97==0) j97 = 97;
		c -= cd;
		if (c<0.0) c += cm;
		uni -= c;
		if (uni<0.0) uni += 1.0;
		rvec[ivec] = uni;
	}
}

void generate_srand(unsigned short seed)
{
	unsigned short iFirst, iSecond, i;
	unsigned long ulTmp;
	unsigned short ucDateTime[14+1];

	typedef union {
		unsigned int	x;
		unsigned char	h[4];
	}dWORD;

	dWORD	TimeTick;	

	TimeTick.x = ::GetTickCount();
	ucDateTime[2] = (TimeTick.h[0]);
	ucDateTime[3] = (BYTE)(TimeTick.h[1]);
	ucDateTime[4] = (BYTE)(TimeTick.h[2]);
	ucDateTime[5] = (BYTE)(TimeTick.h[3]);
	ucDateTime[6] = (BYTE)(~TimeTick.h[2]);
	ucDateTime[7] = (BYTE)(~TimeTick.h[3]);

	seed ^= TimeTick.h[1];
	while (seed>=200)seed>>=1;
	Sleep(seed*10);

	TimeTick.x = ::GetTickCount();
	ucDateTime[8] = (TimeTick.h[3]);
	ucDateTime[9] = (BYTE)(TimeTick.h[2]);
	ucDateTime[10] = (BYTE)(TimeTick.h[2]);
	ucDateTime[11] = (BYTE)(TimeTick.h[1]);
	ucDateTime[12] = (BYTE)(~TimeTick.h[2]);
	ucDateTime[13] = (BYTE)(~TimeTick.h[1]);


	for(i = 0, ulTmp = 0L; i < 6; i++)
	{
		ulTmp *= (unsigned long)10;
		ulTmp += (unsigned long)(ucDateTime[2 + i]);
	}
	iFirst = (unsigned short)(ulTmp % 31320) + seed;

	for(i = 0, ulTmp = 0L; i < 6; i++)
	{
		ulTmp *= (unsigned long)10;
		ulTmp += (unsigned long)(ucDateTime[8 + i]);
	}
	iSecond = (unsigned short)(ulTmp % 30080);

	rmarin(iFirst, iSecond);
}

BOOL	bExec_SRand = FALSE;

unsigned short WINAPI generate_rand(void)
{
	float faTemp[2] = {0, };	// CodeSonar 지적

	if (bExec_SRand == FALSE)
	{
		bExec_SRand = TRUE;

		unsigned short seed;
		seed = (unsigned short)GetTickCount();
		generate_srand(seed);
	}

	ranmar(&faTemp[0], 1);	

	return (unsigned short)(4096.0 * 4096.0 * faTemp[1]);
}

