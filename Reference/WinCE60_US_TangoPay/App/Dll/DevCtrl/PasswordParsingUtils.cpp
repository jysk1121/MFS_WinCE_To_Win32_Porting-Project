#include "stdafx.h"

#include ".\Common\CmnLib.h"
#include ".\Dev\LoginManager.h"

#define SEP			':'
#define PASS_SEP	'$'

#define FIELD_VERSION	0
#define FIELD_VERSION_1 1

// Version 1
#define V1_FIELD_IDENTITY	1
#define V1_FIELD_PASSWORD	2
#define V1_FIELD_SINCE		3

#define PASS_FIELD_SEP1 0
#define PASS_FIELD_ALGO 1
#define PASS_FIELD_SALT 2
#define PASS_FIELD_HASH 3

#define ALGO_MODE_SALTED	0
#define ALGO_MODE_UNSALTED	1

/**
 * The serialized password format is as specified in ABNF below and is similar to the UNIX password format.
 * 
 * base64-char		= %s(ALPHA / DIGIT / "+" / "/")
 * base64-terminal	= (2base64-char "==") / (3base64-char "=")
 * base64			= *(4base64-char) [base64-terminal]
 *
 * version		= 1*DIGIT	; the version of the password format
 * identity		= 1*DIGIT	; the identity whose password is represented
 * sep			= ":"		; a logical field separator
 * since		= 1*DIGIT	; represents a timespan since an occurance
 * 
 * pass-sep		= "$"				; a logical field separator for password components
 * sha512		= "1"				; indicates that the password hashing algorithm is SHA512
 * bcrypt		= "2"				; indicates that the password hashing algorithm is BCrypt
 * pbkdf2		= "3"				; indicates that the password hashing algorithm is PBKDF2/HMAC/SHA512 with 50,000 iterations
 * 
 * algo			= sha512 / bcrypt / pbkdf2	; indicates which password hashing algorithm was used
 * salt			= 16(ALPHA / DIGIT)			; the salt used in the hashing function
 * hash			= base64					; the base64-encoded hash of the password
 *
 * salted-password	= pass-sep algo pass-sep salt pass-sep hash
 * password			= pass-sep algo pass-sep hash
 * format			= version sep identity sep (salted-password / password) sep since
 * 
 * Here's an example:
 * 1:0:$1$NRF5ip1pQQEjiZa$keoSRfINRq6aA3qYn1Tx95DwpHYH7rihTRKJDOp3obvGx+2c8gXme38rj9TH39OnqGF+RfPEY9SBx+WGw5rB7Q==:18256
 * That format indicates:
 * 1	-> Version 1
 * 0	-> Master User
 * $1	-> SHA512
 * $x	-> Salt data
 * $x	-> Hash Data
 * 18256 -> Days since last change 
 */

int GetAlgoSaltMode(int algo)
{
	if (algo == 1) // bcrypt
	{
		// Bcrypt is salted, but the format returned by the library is self-contained,
		// so we shouldn't manage the salting.
		return ALGO_MODE_UNSALTED;
	}

	return ALGO_MODE_SALTED;
}

bool ParsePasswordRecord(char * data, PasswordRecord &record)
{
	int passwordFieldIdx = PASS_FIELD_SEP1;
	size_t dataLen = strlen(data);
	char *current = data;
	char temp[256] = {};
	int len = 0;

	memset(record.Salt, 0, sizeof(record.Salt));
	memset(record.Data, 0, sizeof(record.Data));

	for (unsigned int i = 0; i < dataLen; )
	{
		switch (passwordFieldIdx)
		{
		case PASS_FIELD_ALGO:
			len = strchr(current, PASS_SEP) - current;
			strncpy(temp, current, len);
			record.Algorithm = (Algorithm)atoi(temp);

			if (GetAlgoSaltMode(record.Algorithm) == ALGO_MODE_SALTED)
			{
				passwordFieldIdx = PASS_FIELD_SALT;
			}
			else
			{
				passwordFieldIdx = PASS_FIELD_HASH;
			}
			break;
		case PASS_FIELD_SALT:
			len = strchr(current, PASS_SEP) - current; // Get value of next separator
			strncpy(record.Salt, current, len);
			passwordFieldIdx = PASS_FIELD_HASH;
			break;
		case PASS_FIELD_HASH:
			len = strlen(current); // Get value of next separator
			strncpy(temp, current, len);
			record.DataLength = base64_decode(temp, len, (unsigned char*)record.Data, sizeof(record.Data));
			passwordFieldIdx = PASS_FIELD_HASH;
			break;
		case PASS_FIELD_SEP1:
			len = 0;
			passwordFieldIdx = PASS_FIELD_ALGO;
			break;
		}

		// Reset pointers
		current = current + len + 1;
		i += len + 1;
		memset(temp, 0, sizeof(temp));
	}

	return true;
}

int ParseVersion1Record(char * data, PasswordRecord &record)
{
	unsigned int i = 0;
	int mainFieldIdx = V1_FIELD_IDENTITY;
	size_t dataLen = strlen(data);
	char *current = data;
	char temp[256] = {};
	int len = 0;
	
	for (; i < dataLen; )
	{
		switch (mainFieldIdx)
		{
		case V1_FIELD_IDENTITY:
			len = strchr(current, SEP) - current; // Get value of next separator
			strncpy(temp, current, len);
			record.Identity = (Identity) atoi(temp);
			mainFieldIdx = V1_FIELD_PASSWORD;
			break;
		case V1_FIELD_PASSWORD:
			len = strchr(current, SEP) - current;
			strncpy(temp, current, len);
			ParsePasswordRecord(temp, record);
			mainFieldIdx = V1_FIELD_SINCE;
			break;
		case V1_FIELD_SINCE: // Value ignored
			len = strlen(current);
			break;
		}

		// Reset pointers
		current = current + len + 1;
		i += len + 1;
		memset(temp, 0, sizeof(temp));
	}

	return i;
}

/**
 * A utility function for parsing the serialized password record format
 */
bool ParseRecord(char * data, PasswordRecord &record)
{
	int mainFieldIdx = FIELD_VERSION;
	size_t dataLen = strlen(data);
	char *current = data;
	char temp[256] = {};
	int len = 0;

	int version = 0;
	
	for (unsigned int i = 0; i < dataLen;)
	{
		switch (mainFieldIdx)
		{
		case FIELD_VERSION:
			len = strchr(current, SEP) - current;
			strncpy(temp, current, len);
			version = atoi(temp);
			mainFieldIdx = FIELD_VERSION_1;
			break;
		case FIELD_VERSION_1:
			len = strlen(current);
			strncpy(temp, current, len);
			ParseVersion1Record(temp, record);
			break;
		}

		// Reset pointers
		current = current + len + 1;
		i += len + 1;
		memset(temp, 0, sizeof(temp));
	}

	return true;
}

bool SerializeRecord(char * data, int datasize, const PasswordRecord &record)
{
	// NB: This function will only ever write the latest version.
	char temp[MAX_PASSWORD_SERIALIZATION_SIZE] = {};

	// 1. Version
	strncat_s(data, datasize, "1", 1);

	// 2. Identity
	strncat_s(data, datasize, ":", 1);
	_itoa(record.Identity, (char *)temp, 10);
	strncat_s(data, datasize, (char *)temp, strnlen_s(temp, 2));
	memset(temp, 0, sizeof(temp));

	// 3. Password (algo, salt, hash)
	strncat_s(data, datasize, ":$", 2);
	_itoa(record.Algorithm, (char *)temp, 10);
	strncat_s(data, datasize, (char *)temp, strnlen_s(temp, 2));
	memset(temp, 0, sizeof(temp));

	if (GetAlgoSaltMode(record.Algorithm) == ALGO_MODE_SALTED)
	{
		strncat_s(data, datasize, "$", 1);
		strncat_s(data, datasize, (char *)record.Salt, sizeof(record.Salt));
	}
	
	strncat_s(data, datasize, "$", 1);
	int encodedLen = base64_encode((char *)record.Data, record.DataLength, (char *)temp, MAX_PASSWORD_SERIALIZATION_SIZE);
	strncat_s(data, datasize, (char *)temp, encodedLen);

	// 4. Since
	strncat_s(data, datasize, ":", 1);
	strncat_s(data, datasize, "0", 1);

	return true;
}