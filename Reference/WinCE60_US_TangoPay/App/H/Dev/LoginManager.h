#pragma once

#include ".\Dev\DevDefine.h"
#include ".\Dev\Devcmn.h"

#define MIN_PASSWORD_LENGTH 6
#define MAX_PASSWORD_SERIALIZATION_SIZE 512
// 64 bytes is 512 bits for SHA512
#define HASHED_PASSWORD_SIZE 64
// Salt is 16 chars. (FYI this is a C string)
#define SALT_SIZE 16

#define NO_VERIFICATION_PASSWORD_ACCESS private
#if !(US_VERSION || CA_VERSION || MX_VERSION)
#  undef  NO_VERIFICATION_PASSWORD_ACCESS
#  define NO_VERIFICATION_PASSWORD_ACCESS public
#  define PUBLIC_PASSWORD_CHANGE_NO_VERIF
#elif (APP_CUSTOM_PAI)
#  undef  NO_VERIFICATION_PASSWORD_ACCESS
#  define NO_VERIFICATION_PASSWORD_ACCESS public
#  define PUBLIC_PASSWORD_CHANGE_NO_VERIF
#endif

/**
 * Represents the various identities with which a users can be authenticated with.
 */
enum Identity
{
	Master		= 0,
	Operator	= 1,
	Service		= 2,
	IDENTITY_MAX
};

/**
 * Represents the algorithm used to store the password
 */
enum Algorithm
{
	SHA2_512 = 0,
	BCrypt = 1,
	PBKDF2_HMAC_PKCS5_50000 = 2,
};

// The most secure algorithm available. Change for other territories due to password hash calculation time
#if (US_VERSION || CA_VERSION || MX_VERSION)
#  define LATEST_ALGO PBKDF2_HMAC_PKCS5_50000
#else
#  define LATEST_ALGO SHA2_512
#endif

/**
 * Represents a login result
 */
typedef struct _login_result 
{
	/**
	 * True if the authentication was successful
	 */
	bool Success;

	/**
	 * The name of the logged-in identity
	 */
	Identity Identity;

	/**
	 * An optional message to be presented to the user. Only present
	 * if Success == false.
	 */
	CString Message;

	/**
	 * Gets the login mode for the auth result
	 */
	int GetLoginMode()
	{
		if (!Success)				return -1;
		if (Identity == Master)		return LOGIN_MASTER;
		if (Identity == Operator)	return LOGIN_OPERATOR;
		if (Identity == Service)	return LOGIN_SERVICE;

		return -1;
	};

	/**
	 * Gets the username of the identity
	 */
	CString GetUsername()
	{
		if (!Success)				return L"";
		if (Identity == Master)		return L"MASTER";
		if (Identity == Operator)	return L"OPERATOR";
		if (Identity == Service)	return L"SERVICE";

		return L"";
	};

} LoginResult, *LPLoginResult;

/**
 * A struct for the deserialized data of a password
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
 * sha512		= "0"				; indicates that the password hashing algorithm is SHA512
 * bcrypt		= "1"				; indicates that the password hashing algorithm is BCrypt
 * pbkdf2		= "2"				; indicates that the password hashing algorithm is PBKDF2/HMAC/SHA512 with 50,000 iterations
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
typedef struct _password_record
{
	Algorithm	Algorithm;
	Identity	Identity;

	char				Salt[SALT_SIZE + 1];
	unsigned char		Data[HASHED_PASSWORD_SIZE];
	int					DataLength;
} PasswordRecord;

/**
 * A utility function for parsing the serialized password record format
 */
bool ParseRecord(char * data, PasswordRecord &record);
bool SerializeRecord(char * data, int datasize, const PasswordRecord &record);

class AFX_EXT_CLASS CLoginManager
{
public:
	CLoginManager(void);
	~CLoginManager(void);

	/**
	 * Changes the user's password if the old password matches the current password.
	 * @param identity[in] the identity of the user whose password is being changed
	 * @param oldPassword[in] the user's old password. This can also be the password of a supervisor user
	 * @param newPassword[in] the user's new password
	 * @returns true if the operation was successful
	 */
	bool ChangePassword(Identity identity, CString oldPassword, CString newPassword);

	/** 
	 * Authenticates the identity and password
	 * @param identity[in] the identity to authenticate
	 * @param password[in] the password for the identity
	 * @returns a LoginResult
	 */
	LoginResult Login(Identity identity, CString password);

	/**
	 * Authenticates the password against all identities.
	 * @param password[in] the password to check
	 * @returns a LoginResult
	 */
	LoginResult Login(CString password);

	/**
	 * Determines if the ATM is currently using the old password format, i.e. the password
	 * database has not been upgraded.
	 * @returns true if the old passwords are in use
	 */
	bool AreOldPasswordsInUse();

	/**
	 * Verifies that the passwords have been initialized properly. A negative result
	 * could indicate an "NVRAM-broken" error.
	 * @returns true is the passwords are initialized
	 */
	bool ArePasswordsInInitialState();

	/**
	 * Initializes all passwords, requiring a master password reset sequence.
	 */
	void InitializePasswords();

	/**
	 * Resets the master password but only under a few conditions
	 * @param dev[in] a pointer to the device common control class
	 * @returns true if the operation is successful.
	 */
	bool ResetMasterPassword(CDevCmn* dev);

	/**
	 * Gets the password length
	 * @returns the password length
	 */
	static int GetPasswordLength();

	/**
	 * Updates the system password length
	 * @param length [in] the new password length for the system
	 */
	static void SetPasswordLength(int length);

	/**
	 * Determines whether the password is compliant with the password rules
	 * @params password [in] the password to check
	 * @returns true if the password meets the requirements
	 */
	static bool IsPasswordValid(CString password);

	/**
	 * Determines if the lockout period is active after a number of successive invalid auth attempts
	 * @returns true if the lockout period is active.
	 */
	static bool IsLockoutActivated();

	/**
	 * Upgrades the old password storage to the new password storage, if neccessary.
	 */
	bool UpgradeOldPasswordsOrNoop();

private:
	/**
	 * Verifies that the input password matches the hashed password of the identity.
	 * @param checkSupervisorPasswords[in] if true, the function will check the passwords of the parents roles, too. Therefore, Master can authenticate as all users, and operator can authenticate as service.
	 * @returns true if successful
	 */
	bool VerifyPassword(Identity identity, char * password, bool checkSupervisorPasswords = false);
	// Updates the password for the identity

	// NB: This method will change access protection levels based on the build type:
	// US/CA/MX (Non-PAI): Private
	// All others (+ PAI): Public
NO_VERIFICATION_PASSWORD_ACCESS:
	bool UpdatePassword(Identity identity, CString password);
	bool UpdatePassword(Identity identity, char * password);

private:

	static bool GetPasswordRecord(Identity identity, PasswordRecord &record);
	static bool SavePasswordRecord(const PasswordRecord &record);
	static bool DeserializePasswordRecord(char * data, PasswordRecord &record);
	static bool SerializePasswordRecord(const PasswordRecord &record, char * buffer, int buffersize);
	static bool CheckPasswordMatchesRecord(const PasswordRecord &record, char * password);

	static unsigned long long GetUnixTimestamp();
	static int GetInvalidLoginAttempts();
	static void IncrementInvalidLoginAttempts();
	static void InitiateLockout();
	static void ClearInvalidLoginAttempts();

	static unsigned int CreateDefaultHash(const char * salt, char * password, unsigned char * hashBuffer, Algorithm &algo);
	static unsigned int CreateSHA512Hash(const char * salt, char * password, unsigned char * hashBuffer);
	static unsigned int CreatePBKDF2Hash(const char * salt, char * password, unsigned char * hashBuffer);
	static unsigned int CreateBcryptHash(const char * password, unsigned char * hashBuffer);
	static bool VerifyBcryptHash(const char * password, const char hash[HASHED_PASSWORD_SIZE]);

	static void GetRandomSalt(char * output, int length);
};
