#include "stdafx.h"

#include <openssl/evp.h>
#include <openssl/rand.h>
#include "Common/bcrypt.h"

#include ".\Common\NHDbgApi.h"
#include ".\Dev\LoginManager.h"

//#define TEST_MODE
#ifdef TEST_MODE
# define TEST_FORMAT "1:0:$0$asdfasdflkjhlkjh$36tYmHGISsWpgv6FGXrTH9/BU2n+SqJuzAnpKu/pOCGv4fAXCmcF1c+AFsNDjgHxcZZDgwMl1LQnX4qfD4wwYQ==:0"
#endif

#define LOCKOUT_DURATION_MILLIS (30 * 60) // 30 Minutes
#define ALLOWED_INVALID_ATTEMPTS 6

#define DEFAULT_UNINIT_PASSWORD L"XXXXXX"
#define DEFAULT_MASTER_PASSWORD "555555"

// Login Message
#define LOGGED_IN			L"Success"
#define INVALID_PASSWORD	L"Invalid Password"
#define LOCKOUT_ENABLED		L"Lockout Enabled"

// The password value which will live in the existing fields to denote that the passwords have been upgraded
#define PW_DEPRECATION L"__dep_new"

// This macro will send the data in a CString to the great beyond. It is
// imporant that p is marked volatile, so that the compiler won't optimize
// it out of existance.
#define SECURE_CLEAR_CSTRING(s) { volatile TCHAR* p = s.GetBuffer(); \
	SecureZeroMemory((void*)p, s.GetLength() * sizeof(*p)); \
	s.ReleaseBuffer(); }

CLoginManager::CLoginManager(void)
{

}

CLoginManager::~CLoginManager(void)
{

}

/**
 * Changes the user's password if the old password matches the current password.
 * @param identity[in] the identity of the user whose password is being changed
 * @param oldPassword[in] the user's old password. This can also be the password of a supervisor user
 * @param newPassword[in] the user's new password
 * @returns true if the operation was successful
 */
bool CLoginManager::ChangePassword(Identity identity, CString oldPassword, CString newPassword)
{
	bool result = true;

	NHDBG((L"Change Password Start\r\n"));

	// Validate complexity requirements
	if (!IsPasswordValid(newPassword))
	{
		NHWARN((L"New password is invalid\r\n"));
		return false;
	}

	int passwordLen = oldPassword.GetLength() + 1;
	char * multiPassword = new char[passwordLen]();
	WideToMulti(multiPassword, oldPassword, passwordLen);

	// Check old password
	bool verified = VerifyPassword(identity, multiPassword, true);
	memset(multiPassword, 0, passwordLen);
	delete [] multiPassword;
	if (!verified)
	{
		NHERROR((L"Old password is not correct\r\n"));
		return false;
	}

	passwordLen = newPassword.GetLength() + 1;
	char * multiNewPassword = new char[passwordLen]();
	WideToMulti(multiNewPassword, newPassword, passwordLen);

	bool reset = UpdatePassword(identity, multiNewPassword);
	memset(multiNewPassword, 0, passwordLen);
	delete [] multiNewPassword;
	if (!reset)
	{
		NHERROR((L"Error while changing password\r\n"));
		return false;
	}

	NHINFO((L"Password changed successfully!\r\n"));

	// Clear buffers
	SECURE_CLEAR_CSTRING(newPassword);
	SECURE_CLEAR_CSTRING(oldPassword);

	return true;
}

/** 
 * Authenticates the identity and password
 * @param identity[in] the identity to authenticate
 * @param password[in] the password for the identity
 * @returns a LoginResult
 */
LoginResult CLoginManager::Login(Identity identity, CString password)
{
	NHDBG((L"Login Start\r\n"));

	LoginResult result;
	result.Identity = identity;

	int passwordLen = password.GetLength() + 1;
	char * multiPassword = new char[passwordLen]();
	WideToMulti(multiPassword, password, passwordLen);

	// Test Lockout - this could be a duplicate, but this method is idempotent
	if (IsLockoutActivated())
	{
		result.Success = false;
		result.Message = LOCKOUT_ENABLED;
		goto cleanup;
	}

	if (!VerifyPassword(identity, multiPassword))
	{
		result.Success = false;
		result.Message = INVALID_PASSWORD;
		goto cleanup;
	}

	result.Success = true;
	result.Message = LOGGED_IN;

cleanup:
	memset(multiPassword, 0, passwordLen);
	delete [] multiPassword;

	NHDBG((L"Login End\r\n"));

	SECURE_CLEAR_CSTRING(password);

	return result;
}

/**
 * Authenticates the password against all identities.
 * @param password[in] the password to check
 * @returns a LoginResult
 */
LoginResult CLoginManager::Login(CString password)
{
	LoginResult result;

	// Test lockout
	if (IsLockoutActivated())
	{
		result.Success = false;
		result.Message = LOCKOUT_ENABLED;
		goto end;
	}

	// Test Master
	result = Login(Master, password);
	if (result.Success)
	{
		goto end;
	}

	// Operator
	result = Login(Operator, password);
	if (result.Success)
	{
		goto end;
	}

	// Service
	result = Login(Service, password);
	if (result.Success)
	{
		goto end;
	}

	result.Success = false;
	IncrementInvalidLoginAttempts();
end:

	if (result.Success)
	{
		ClearInvalidLoginAttempts();
	}

	SECURE_CLEAR_CSTRING(password);
	return result;
}

/**
 * Verifies that the passwords have been initialized properly. A negative result
 * could indicate an "NVRAM-broken" error.
 * @returns true is the passwords are initialized
 */
bool CLoginManager::ArePasswordsInInitialState()
{
	 return MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MASTERPW, DEFAULT_UNINIT_PASSWORD) == DEFAULT_UNINIT_PASSWORD;
}

/**
 * Initializes all passwords, requiring a master password reset sequence.
 */
void CLoginManager::InitializePasswords()
{
	MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MASTERPW, DEFAULT_UNINIT_PASSWORD);
}

/**
 * Resets the master password but only under a few conditions
 * @param dev[in] a pointer to the device common control class
 * @returns true if the operation is successful.
 */
bool CLoginManager::ResetMasterPassword(CDevCmn* dev)
{
	// Exit if the hotkey wasn't input or the door is closed
	if (dev->fnDOR_GetDoorStatus() == DOOR_CLOSED)
	{
		return false;
	}

	// Verify the new password DB is in initial state
	if (!ArePasswordsInInitialState())
	{
		return false;
	}

	dev->m_JNLMgr.Save(OPERATOR_ACTION, L"Reset Master Password");
	MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MASTERPW, MASTERPW_DEFAULT);

	return true;
}

/**
 * Upgrades the old password storage to the new password storage, if neccessary.
 */
bool CLoginManager::UpgradeOldPasswordsOrNoop()
{
	CString masterPw = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MASTERPW);
	if (masterPw != PW_DEPRECATION) // Perform upgrade since master pw is not cleared
	{
		if (!UpdatePassword(Master, MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MASTERPW)))
		{
			NHERROR((L"Error while updating the master password\r\n"));
			return false;
		}

		if (!UpdatePassword(Operator, MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_OPERATORPW)))
		{
			NHERROR((L"Error while updating the operator password\r\n"));
			return false;
		}

		if (!UpdatePassword(Service, MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SERVICEPW)))
		{
			NHERROR((L"Error while updating the service password\r\n"));
			return false;
		}

		// Set deprecations on old values
		MemSetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MASTERPW, PW_DEPRECATION);
		MemSetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_OPERATORPW, PW_DEPRECATION);
		MemSetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SERVICEPW, PW_DEPRECATION);
	}

	SECURE_CLEAR_CSTRING(masterPw);
	
	return true;
}

/**
 * Gets the password length
 * @returns the password length
 */
int CLoginManager::GetPasswordLength()
{
	int len = MemGetInt(_VAR_OPT2_NO, _MEM_VAR_OPT2_PWLENGTH);
	if (len < MIN_PASSWORD_LENGTH)
	{
		return MIN_PASSWORD_LENGTH;
	}

	return len;
}

/**
 * Updates the system password length
 * @param length [in] the new password length for the system
 */
void CLoginManager::SetPasswordLength(int length)
{
	if (length < MIN_PASSWORD_LENGTH)
	{
		length = MIN_PASSWORD_LENGTH;
	}

	MemSetInt(_VAR_OPT2_NO, _MEM_VAR_OPT2_PWLENGTH, length);
}

/**
 * Determines whether the password is compliant with the password rules
 * @params password [in] the password to check
 * @returns true if the password meets the requirements
 */
bool CLoginManager::IsPasswordValid(CString password)
{
	bool result = false;
	if (password.GetLength() != GetPasswordLength())
	{
		goto end;
	}

	// TODO: evaluate password complexity
	
	result = true;

end:
	SECURE_CLEAR_CSTRING(password);
	return result;
}

/**
 * Determines if the lockout period is active after a number of successive invalid auth attempts
 * @returns true if the lockout period is active.
 */
bool CLoginManager::IsLockoutActivated()
{
	unsigned long long now = GetUnixTimestamp();
	unsigned long long lockoutExpiration = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_LOCKOUTEXPIRATION);
	return now <= lockoutExpiration;
}

//
// Private functions
//

bool CLoginManager::VerifyPassword(Identity identity, char * password, bool checkSupervisorPasswords)
{
	PasswordRecord record;

	// Loop through all parent identities to check supervisor passwords
	while (identity >= Master)
	{
		if (!GetPasswordRecord(identity, record))
		{
			NHERROR((L"Could not find identity\r\n"));

			return false;
		}

		if (!CheckPasswordMatchesRecord(record, password))
		{
			// We should continue the search if the function should check supervisor passwords
			if (checkSupervisorPasswords)
			{
				identity = (Identity)((int)identity - 1);
				continue;
			}
			else
			{
				return false;
			}
		}

		// Update the password, if the algorithm is updated
		if (record.Algorithm != LATEST_ALGO)
		{
			NHINFO((L"Password does not use the latest algo. Updating\r\n"));
			UpdatePassword(identity, password);
		}

		return true;
	}

	// No hits...
	return false;
}

bool CLoginManager::UpdatePassword(Identity identity, CString password)
{
	int len = password.GetLength() + 1;
	char * multiPassword = new char[len]();
	WideToMulti(multiPassword, password, len);

	bool result = UpdatePassword(identity, multiPassword);

	memset(multiPassword, 0, len);
	delete [] multiPassword;

	SECURE_CLEAR_CSTRING(password);

	return result;
}

bool CLoginManager::UpdatePassword(Identity identity, char * password)
{
	bool result = true;
	char * salt = new char[SALT_SIZE + 1]();
	unsigned char * hashBuffer = new unsigned char[HASHED_PASSWORD_SIZE]();
	GetRandomSalt((char *)salt, SALT_SIZE);

	Algorithm algo;
	int hashLen = CreateDefaultHash((const char *)salt, password, hashBuffer, algo);
	if (hashLen <= 0)
	{
		NHERROR((L"Could not create password hash\r\n"));
		result = false;
		goto cleanup;
	}

	PasswordRecord record;
	record.Identity = identity;
	record.Algorithm = algo;
	memset(record.Salt, 0, sizeof(record.Salt));
	memcpy_s(record.Salt, sizeof(record.Salt), salt, SALT_SIZE);
	memset(record.Data, 0, sizeof(record.Data));
	memcpy_s(record.Data, sizeof(record.Data), hashBuffer, hashLen);
	record.DataLength = hashLen;

	if (!SavePasswordRecord(record))
	{
		NHERROR((L"Could not save password\r\n"));
		result = false;
		goto cleanup;
	}

cleanup:
	memset(salt, 0, SALT_SIZE + 1);
	delete [] salt;

	memset(hashBuffer, 0, HASHED_PASSWORD_SIZE);
	delete [] hashBuffer;

	return result;
}

bool CLoginManager::GetPasswordRecord(Identity identity, PasswordRecord &record)
{
	char data[MAX_PASSWORD_SERIALIZATION_SIZE] = {};
	CString encodedData;

#ifndef TEST_MODE
	switch (identity)
	{
	case Master:
		encodedData = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MASTERPW, L"");
		break;
	case Operator:
		encodedData = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_OPERATORPW, L"");
		break;
	case Service:
		encodedData = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SERVICEPW, L"");
		break;
	}

	encodedData.Trim();
	// Limit to more than 32 chars because bad NVRAM parsing could result in a false-positive
	if (encodedData.IsEmpty() || encodedData.GetLength() <= 32)
	{
		NHINFO((L"Password data is blank\r\n"));
		return false;
	}

	WideToMulti(data, encodedData, sizeof(data));
	NHDBG((L"Password data: %S\r\n", data));

	SECURE_CLEAR_CSTRING(encodedData);
#else
	strcpy(data, TEST_FORMAT);
#endif

	bool result = DeserializePasswordRecord(data, record);
	memset(data, 0, sizeof(data));
	return result;
}

bool CLoginManager::SavePasswordRecord(const PasswordRecord &record)
{
	char data[MAX_PASSWORD_SERIALIZATION_SIZE] = {};

	if (!SerializePasswordRecord(record, data, MAX_PASSWORD_SERIALIZATION_SIZE))
	{
		NHERROR((L"Record serialization failed\r\n"));
		return false;
	}

#ifndef TEST_MODE

	CString encodedData(data);

	switch (record.Identity)
	{
	case Master:
		MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MASTERPW, encodedData);
		break;
	case Operator:
		MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_OPERATORPW, encodedData);
		break;
	case Service:
		MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SERVICEPW, encodedData);
		break;
	}

	SECURE_CLEAR_CSTRING(encodedData);
#endif

	NHINFO((L"Password saved successfully: %S\r\n", data));
	memset(data, 0, sizeof(data));
	return true;
}

bool CLoginManager::DeserializePasswordRecord(char * data, PasswordRecord &record)
{
	if (data == NULL)
	{
		return false;
	}

	return ParseRecord(data, record);
}

bool CLoginManager::SerializePasswordRecord(const PasswordRecord &record, char * buffer, int buffersize)
{
	return SerializeRecord(buffer, buffersize, record);
}

bool CLoginManager::CheckPasswordMatchesRecord(const PasswordRecord &record, char * password)
{
	unsigned char hashedOutputBuffer[HASHED_PASSWORD_SIZE] = {};
	unsigned int outputSize = 0;

	switch (record.Algorithm)
	{
	case SHA2_512:
		outputSize = CreateSHA512Hash(record.Salt, password, hashedOutputBuffer);
		break;
	case BCrypt:
		return VerifyBcryptHash(password, (char *)record.Data);
	case PBKDF2_HMAC_PKCS5_50000:
		outputSize = CreatePBKDF2Hash(record.Salt, password, hashedOutputBuffer);
		break;
	}

	if (outputSize == 0)
	{
		NHERROR((L"Problem hashing password\r\n"));
		return false;
	}

	// Compare hashes - NB: do not short-circuit this loop because a password compare must
	// not change it's timing based on the result. That would implmement a timing attack.
	bool result = true;
	for (int i = 0; i < HASHED_PASSWORD_SIZE; i++)
	{
		if (record.Data[i] != hashedOutputBuffer[i])
		{
			result = false;
		}
	}

	memset(hashedOutputBuffer, 0, sizeof(hashedOutputBuffer));
	return result;
}

//
// Lockout/Login Throttling
//

unsigned long long CLoginManager::GetUnixTimestamp()
{
	const __int64 UNIX_TIME_START = 116444736000000000; //January 1, 1970 (start of Unix epoch) in "ticks" // A Windows tick is 100 nanoseconds. Windows epoch 1601-01-01T00:00:00Z, 11644473600 seconds before Unix epoch 1970-01-01T00:00:00Z.
	const __int64 TICKS_PER_SECOND = 10000000; //a tick is 100ns

	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);

	ULARGE_INTEGER li;
	li.LowPart  = ft.dwLowDateTime;
	li.HighPart = ft.dwHighDateTime;

    return (li.QuadPart - UNIX_TIME_START) / TICKS_PER_SECOND;	//Convert ticks since 1/1/1970 into seconds
}

int CLoginManager::GetInvalidLoginAttempts()
{
	return MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_INVALIDLOGONATTEMPTS);
}

void CLoginManager::IncrementInvalidLoginAttempts()
{
	int attempts = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_INVALIDLOGONATTEMPTS) + 1;
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_INVALIDLOGONATTEMPTS, attempts);

	if (attempts >= ALLOWED_INVALID_ATTEMPTS)
	{
		ClearInvalidLoginAttempts();
		InitiateLockout();
	}
}

void CLoginManager::InitiateLockout()
{
	NHERROR((L"Authentication lockout begin.\r\n"));
		
	unsigned long long now = GetUnixTimestamp();
	unsigned long long lockoutExpiration = now + LOCKOUT_DURATION_MILLIS;

	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_LOCKOUTEXPIRATION, lockoutExpiration);
}

void CLoginManager::ClearInvalidLoginAttempts()
{
	NHINFO((L"Invalid logins cleared\r\n"));
	MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_INVALIDLOGONATTEMPTS, 0);
}

//
// Algorithms
//

unsigned int CLoginManager::CreateDefaultHash(const char * salt, char * password, unsigned char * hashBuffer, Algorithm &algo)
{
	algo = LATEST_ALGO;

	switch (LATEST_ALGO)
	{
	case SHA2_512:
		return CreateSHA512Hash(salt, password, hashBuffer);
	case BCrypt:
		return CreateBcryptHash(password, hashBuffer);
	case PBKDF2_HMAC_PKCS5_50000:
		return CreatePBKDF2Hash(salt, password, hashBuffer);
	}

	return 0;
}

unsigned int CLoginManager::CreateSHA512Hash(const char * salt, char * password, unsigned char * hashBuffer)
{
	unsigned int len = 0;
	EVP_MD_CTX *mdctx = EVP_MD_CTX_create();
	const EVP_MD *md = EVP_sha512();
	EVP_DigestInit_ex(mdctx,md, NULL);

	EVP_DigestUpdate(mdctx, password, strlen(password));
	EVP_DigestUpdate(mdctx, salt, SALT_SIZE);
	EVP_DigestFinal_ex(mdctx, hashBuffer, &len);

	EVP_MD_CTX_destroy(mdctx);

	return len;
}

unsigned int CLoginManager::CreatePBKDF2Hash(const char * salt, char * password, unsigned char * hashBuffer)
{
	int outlen = 0;

	if (PKCS5_PBKDF2_HMAC(password, strlen(password), (const unsigned char *)salt, SALT_SIZE, 50000, EVP_sha512(), HASHED_PASSWORD_SIZE - 1, hashBuffer) <= 0)
	{
		NHERROR((L"The PBKDF2 call failed\r\n"));
		return 0;
	}

	return HASHED_PASSWORD_SIZE; // the length of the digest
}

unsigned int CLoginManager::CreateBcryptHash(const char * password, unsigned char * hashBuffer)
{
	int outlen = 0;
	char salt[BCRYPT_HASHSIZE] = {};

	int ret = bcrypt_gensalt(12, salt);
	if (ret != 0)
	{
		NHERROR((L"Error generating bcrypt salt!!!\r\n"));
		return 0;
	}

	ret = bcrypt_hashpw(password, salt, (char *)hashBuffer);
	if (ret != 0)
	{
		NHERROR((L"Error generating bcrypt hash!!!\r\n"));
		return 0;
	}

	return BCRYPT_HASHSIZE;
}

bool CLoginManager::VerifyBcryptHash(const char * password, const char hash[HASHED_PASSWORD_SIZE])
{
	return bcrypt_checkpw(password, hash) == 0;
}

void CLoginManager::GetRandomSalt(char * output, int length)
{
	int generated = 0;
	char * rand = new char[length]();

	// NB: WW - This function is deprecated after OpenSSL version 1.1.1; however, since the
	// amount of entropy in the system is so low, I've included it to bolster our randomness.
	// I'm not confident that RAND_poll() adds enough entropy in WinCE due to the reliance on
	// Windows libraries for the entropy and the unavailability of the libraries in WinCE.
	//
	// In future versions of OpenSSL, RAND_screen() will simply make a call to RAND_poll().
	// RAND_poll()	 - https://github.com/openssl/openssl/blob/OpenSSL_1_0_2q/crypto/rand/rand_win.c#L201
	// RAND_screen() - https://github.com/openssl/openssl/blob/OpenSSL_1_0_2q/crypto/rand/rand_win.c#L617
	RAND_screen();

	while (generated < length)
	{
		if (RAND_bytes((unsigned char *)rand, length) <= 0)
		{
			continue;
		}

		for (int i = 0; i < length; i++)
		{
			char data = (char) rand[i];

			if (data >= '0' && data <= '9' || data >= 'a' && data <= 'z' || data >= 'A' && data <= 'Z')
			{
				output[generated++] = data;
			}

			if (generated >= length)
			{
				break;
			}
		}
	}

	memset(rand, 0, length);
	delete [] rand;
}