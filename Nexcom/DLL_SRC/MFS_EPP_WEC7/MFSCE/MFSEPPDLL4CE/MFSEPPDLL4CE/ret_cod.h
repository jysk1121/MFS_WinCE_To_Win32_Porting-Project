#ifndef RET_COD_H /* Multiple Inclusions Prevention Mechanism.*/
#define RET_COD_H

/***************************************************************************
*
*  Project title: 
*
*  Functional descrip.:    General used return codes.
*
****************************************************************************/

#ifdef OK
#undef OK
#endif

/*
***  General return codes *************************************************
*/
typedef enum
{
   OK                                       = 0x00,     // The command executed correctly
   MACERR                                   = 0x01,     // MAC error
   ILLKEY                                   = 0x02,     // Specified key can't be used
   FORMERR                                  = 0x03,     // Parameter error in command
   NO_KEY                                   = 0x04,     // Specified key does not exist
   MACLNG                                   = 0x05,     // MAC length illegal
   REDLNG                                   = 0x06,     // Redundancy field less than 4
   NOMATCH                                  = 0x07,     // Password error
   NOT_ACKED                                = 0x08,     // Routed command was not ACK'ed
   ACCERR                                   = 0x09,     // MPA can't be accessed ( missing rights )*/
   UNEXPECTED                               = 0x0a,     // Command or option received in an unexpected at this time
   NO_CFS_REG                               = 0x0b,     // CFS variable does not exist
   NO_MPA                                   = 0x0b,     // MPA variable does not exist
   DECERR                                   = 0x0d,     // Decryption did not fit the redundancy
   BLKERR                                   = 0x0e,     // Block error in decryption data
// RAM_ERR_NOT_IN_CERTGROUP is equal to CERT_RAM_ERR in the EPP CVS
   RAM_ERR_NOT_IN_CERTGROUP                 = 0x0f,     // MPA checksum error NOT in the certificate group, (substitutes error code CERT_RAM_ERR)
   CERT_RAM_ERR                             = 0x0f,     // MPA checksum error NOT in the certificate group, (substitutes error code CERT_RAM_ERR)
   UNKNOWN                                  = 0x10,     // The command is unknown
   MODE_ERR                                 = 0x11,     // Keyload command illegal in this statea
   CHKERR                                   = 0x12,     // Selftest failed
   PINLNG                                   = 0x13,     // PIN length less than 4
   SIZE_ERR                                 = 0x16,     // MPA variable has different size
   VERIFICATION_FAILED                      = 0x17,     // Verification failed
   NO_RETRIES_LEFT                          = 0x18,     // All no of retries are used, the process is terminated!
   MPA_VAR_SIZE_MAX_TOO_SMALL               = 0x19,     // The preprocessor constant MPA_VAR_SIZE_MAX is too small!
   ENC_MPA_VAR_SIZE_MAX_TOO_SMALL           = 0x1a,     // The preprocessor constant ENC_MPA_VAR_SIZE_MAX_TOO_SMALL is too small!
   MPA_NOT_PRESERVED                        = 0x1b,     // The MPA register is not preserved in the previsous version
   BACKPLATE_TAMPER_DETECT                  = 0x1c,     // Backplate tamper has been detected and commands are disabled.
   PCI_ERR                                  = 0x1d,     // Non conformity to PCI3 requirements
   CURSOR_ERR                               = 0x1e,     // Image position width or height is invalid
// *** Remark: Double defined
   ILL_MPA                                  = 0x1f,     // MPA is illegal for this use
   PROG_ERR                                 = 0x1f,     // Program checksum error
   RAM_ERR                                  = 0x20,     // MPA checksum error
   DISP_ERR                                 = 0x21,     // Display hardware error
   NO_CARD                                  = 0x22,     // Magstribe not present
   UNINITIALIZED                            = 0x23,     // MPA variable uninitialized
   ADDRERR                                  = 0x24,     // Address error in download parameter
   SCRAMBLE                                 = 0x25,     // Scramble value is NOT ok
   KEYERR                                   = 0x26,     // The key specified is not initialized
   NOTDONE                                  = 0x27,     // Scramble check is rejected (search)
   PINERR                                   = 0x28,     // PIN value is wrong
   KCVERR                                   = 0x29,     // The KCV is NOT correct
   DUKPTERR                                 = 0x2a,     // The DUKPT system is not initialized, or
   REJECTED                                 = 0x2b,     // Command could not be executed
   T2_OK                                    = 0x2c,     // Track 2   has been read
   T3_OK                                    = 0x2d,     // Track 1/3 has been read
   T2_T3_OK                                 = 0x2e,     // Track 2 and track 1/3 have been read
   NO_APP                                   = 0x2f,     // No application program loaded
   NOT_FOUND                                = 0x30,     // Search result negative
   BAT_LOW                                  = 0x31,     // Battery low status
/* Return codes from chipcard interface*/
   TIMEOUT                                  = 0x32,     // Nothing received from card within time limit
   STATE_ERR                                = 0x33,     // Command not allowed in this state
   PARITY_ERR                               = 0x34,     // Parity error from card
   NO_ATR                                   = 0x35,     // No Answer To Reset received
   NO_CC                                    = 0x36,     // Card was not inserted
   ACTIVE                                   = 0x37,     // Only used internally in Physical Layer
   VCC_ERR                                  = 0x38,     // Vcc was disconnected due to Vcc timeout
   VPP_ERR                                  = 0x39,     // Vpp was disconnected due to Vpp timeout
   REM_ERR                                  = 0x3a,     // Card was removed and reinserted
   KEY_PARITY_ERROR                         = 0x3a,     // 58d Used by OPT/TSS
   POWERFAIL                                = 0x3b,     // Card deactivated due to powerfail
/* Return codes from chipcard Data Link Layer*/
   ILL_ATR                                  = 0x3c,     // Card could not be treated
   CC_FORM_ERR                              = 0x3d,     // Error in procedure byte from card
   SIMPLE                                   = 0x3f,     // Only used internally in Data Link Layer
   SLAVE                                    = 0x40,     // Only used internally in Data Link Layer
   SW1                                      = 0x41,     // Only used internally in Data Link Layer
   MORE                                     = 0x42,     // Only used internally in Data Link Layer
   PROTOCOL_ERROR                           = 0x43,     // Error in the communication protocol
   NORESPONSE                               = 0x44,     // No response has been received
/* General return codes, continued*/
   PAR_ERR                                  = 0x46,     // Parameter format error
   SW_VERS_ERR                              = 0x47,     // Software version not legal
   HEX_ENTRY_LNG                            = 0x48,     // No of present HEX_ENTRY-digits too small
// *** Remark: Double defined
   KEY_LENGTH                               = 0x49,     // Key length is not valid
   CONFIG_ERR                               = 0x49,     // Configuration error
// *** Remark: Double defined
   INVALID_KEY                              = 0x4a,     // Key not released, not activated or expired
   TDES_ERR                                 = 0x4a,     // 3DES key decrypted by duplicated key
   CARD_ERR                                 = 0x4b,     // Card authentication error
   PIN_REJECT                               = 0x4c,     // PIN error, no more trials
   BLZ_NO_MATCH_ERR                         = 0x4d,     // ZKA register (CARD_BLZ) do not match EF_ID[1..3]
   PIN_NO_REUSE_ERR                         = 0x4e,     // PIN_CONFIG specify reuse of PIN NOT allowed.
/* DISP_LOCKED overlaps with KEY_REG_NUM which is used by OPT TSS part */
   KEY_REG_NUM                              = 0x4f,     // 79d Used by OPT/TSS
   DISP_LOCKED                              = 0x4f,     // The display is locked by Amount Entry/Verification
   AMOUNT_ERROR                             = 0x50,     // The amount is not entered/or verified correctly
   CANCELLED                                = 0x51,     // The amount entry is CANCELLED
   OVERFLOW                                 = 0x52,     // The amount entry is overflown
   COMMAND_DISABLED                         = 0x53,     // The amount entry is CANCELLED
/* REMOVE_BMP... & CANT_REMOVE... is used the Remove_8583_data() function.*/
   REMOVE_BMP_NO_OUT_OF_RANGE               = 0x54,     // The no of the bmp to be removed is not in range
   CANT_REMOVE_BMP_NOT_PRESENT              = 0x55,     // The bmp is not present, can't be removed
/* EXPONENT_S..., MODULUS_..., ENC_DEC... and EXPONENT_L... are used in the RSA crypto functions */
   ENC_DEC_DATA_LENGTH_ERR                  = 0x56,     // The clartext lengt is wrong
   EXPONENT_LENGTH_ERR                      = 0x57,     // The exponent lenght is wrong
   EXPONENT_SPEC_ERR                        = 0x58,     // The exponent specification
   MODULUS_LENGTH_ERR                       = 0x59,     // The modulus length
   HASH_ERR                                 = 0x5a,     // Hash check failed
   SERNO_ERR                                = 0x5b,     // Serial number check failed
   INVALID_RSA_KEY_TYPE                     = 0x5c,     // The RSA key structure type is invalid
   INVALID_PUBLIC_EXPONENT                  = 0x5d,     // The public exponent value is invalid
   PUBLIC_EXPONENT_NOT_INVERTIBLE           = 0x5e,     // The public exponent is not invertible modulo n
   RSA_IN_PROGRESS                          = 0x5f,     // RSA calculation in progress
   RSA_STATE_ERR                            = 0x60,     // RSA function was called in an illegal state
// *** Remark: Double defined
// CERTIFICATE_DATE_ERROR has previous been defined in the EPP CVS
   ARITHMETIC_ERR                           = 0x61,     // Fatal error from low level arithmetic functions
   CERTIFICATE_DATE_ERROR                   = 0x61,     // Certificate date (year or month) expired or certificate not active yet
// Generic RSA Key Management:
   CERT_GENERATION_ERR                      = 0x62,     // An error during Certificate.
   KEY_VS_REG_LENGTH_ERR                    = 0x63,     // The key and the register lengths does not match.
// Return codes used by GK/PACMAC:
   ERROR_BMP53_NOT_PRESENT                  = 0x64,
   ERROR_BMP53_PIN_ENC_ALGO_ID              = 0x65,
// Return codes used by UKPT module (duplets of above!)
   KEYS_SKIPPED                             = 0x64,
   NO_MORE_KEYS                             = 0x65,
// -------------------
   ERROR_BMP53_PIN_BLOCK_FORMAT             = 0x66,
   ERROR_BMP53_PIN_KEY_INDEX                = 0x67,
   ERROR_BMP53_MAC_GEN_MODE                 = 0x68,
   ERROR_BMP53_RFU                          = 0x69,
   ERROR_BMP53_FORMAT_CODE                  = 0x6a,
   ERROR_BMP57_NOT_PRESENT                  = 0x6b,
   ERROR_BMP57_LENGTH_INVALID               = 0x6c,
   ERROR_KID_INVALID                        = 0x6d,
   ERROR_KV_INVALID                         = 0x6e,
   ERROR_BMP64_NOT_PRESENT                  = 0x6f,
   ERROR_BMP34_NOT_PRESENT                  = 0x70,
// PKCS#1 and RKL error codes/RKL:
   MESSAGE_DIGEST_MISMATCH                  = 0x71,     // pkcs msg#2 bad msg digest
   SERIALNO_MISMATCH                        = 0x72,     // An mismatch of SerialNumber received.
   MESS_TOO_LONG                            = 0x73,     // message too long
   INTENDED_ENC_MESS_TOO_SHORT              = 0x74,     // intended encoded message length too short
   MODULUS_TOO_SHORT                        = 0x75,     // modulus too short
   INVALID_SIGNATURE                        = 0x76,     // invalid signature
   INTEGER_TOO_LARGE                        = 0x77,     // integer too large
   MESS_REPRES_OUT_OF_RANGE                 = 0x78,     // message representative out of range
   CIPHERTXT_MESS_REPRES_OUT_OF_RANGE       = 0x79,     // ciphertext representative out of range
// OAEP error codes
   OAEP_DECODING_ERROR                      = 0x7a,     // mismatch in data length of OAEP decoding
   OAEP_HASH_ERROR                          = 0x7b,     // mismatch in calculated pHash and given value
   OAEP_MESSLEN_OUTOF_RANGE                 = 0x7c,     // input len too short or too long
   OAEP_PADDING_ERROR                       = 0x7d,     // missing PAD separator or invalid PAD characters
   PKCS_DECODING_ERROR                      = 0x7e,     // PKCS1-decoding error.
   OAEP_ENCODING_ERROR                      = 0x7f,     // mismatch in data length of OAEP encoding
   PARSING_ERROR                            = 0x80,     // Error parsing data block
   CARD_IN_MOUTH                            = 0x82,     // Motorised card reader, card located inside the card reader
   CARD_STATUS_UNKNOWN                      = 0x83,     // (Motorised)card reader, where card status is unknown
   CVV_ERROR                                = 0x85,     // Card Verification Value error
   SC_ERROR                                 = 0x86,     // Service Code error
   PINTYPE_ERROR                            = 0x87,     // PIN type error
// Return codes used by Key History Logging
   KEY_IN_HISTORY_LOG                       = 0x88,     // Key exists in history log allready
   HISTORY_LOG_FULL                         = 0x89,     // Key history log full
   HISTORY_LOG_ALMOST_FULL                  = 0x8a,     // Key history logis almost full
// Return codes used by Removal Detector
   REV_ACTIVE                               = 0x8c,
   REV_INACTIVE                             = 0x8d,
   KEYPAD_STUCK_KEY_DETCT                   = 0x8e,
   FDK_VERIFICATION_FAILED                  = 0x8f,
   REG_RESET_ERR                            = 0x90, 

// Return codes for CFS
   ILLEGAL_NAME                             = 0xb0,
   OFFSET_LENGTH_ERR                        = 0xb1,
   UNALLOCATED_SECTORS_EXIST                = 0xb2,
   TOO_MUCH_ALLOCATED                       = 0xb3,
   CHKSUM_ERROR                             = 0xb5,
   NOT_ENOUGH_SPACE                         = 0xb6,
   REGISTER_EXISTS                          = 0xb7,
   REGISTER_DOES_NOT_EXIST                  = 0xb8,
   BIT_ERROR_NOT_CORRECTABLE                = 0xb9,
   CFS_FILESYSTEM_ERROR                     = 0xba,
   TYPE_VS_ACCESS_RIGHT_MISMATCH            = 0xbb,
   REG_HEADER_CRC_ERROR                     = 0xbc,
   REG_DATA_CRC_ERROR                       = 0xbd,
   TYPE_VS_SIZE_MISMATCH                    = 0xbe,
   TYPE_VS_WRITE_MISMATCH                   = 0xbf,
   WRITE_VS_WRITE_MISMATCH                  = 0xc0,
   WRITE_VS_PARENT_USE_MISMATCH             = 0xc1,
   LEN_VS_PARENT_LEN_MISMATCH               = 0xc2,
   TYPE_VS_USAGE_MISMATCH                   = 0xc3,
   USE_VS_SIBLING_USE_MISMATCH              = 0xc4,
   PARENT_ID_MISMATCH                       = 0xc5,
   UNKNOWN_DATA_MODE                        = 0xc6,
   INVALID_ROLE                             = 0xc7,     // 199d Role vs command mis-match
   ACCESS_RIGHT_NOT_FULFILLED               = 0xc8,
   USE_CONDITION_NOT_FULFILLED              = 0xc9,
   MULTIPLE_USE_NOT_ALLOWED                 = 0xca,
   KDK_DELETE_ERROR                         = 0xcb,
   EXP_DES_NOT_ALLOWED                      = 0xcc,
   ECB_IMPORT_NOT_ALLOWED                   = 0xcd,
   ERR_MUTUAL_UNIQUENESS                    = 0xce,
   DECOMMISSION_ERROR                       = 0xcf,
   WRONG_KEY_TYPE                           = 0xd0,
   REGISTER_LOCKED                          = 0xd1,
   WEAK_KEY                                 = 0xd2,
   REG_FNT_SECTOR_CRC_BROKEN                = 0xd3,
   TR31_WRITE_NOT_ALLOWED                   = 0xd4,
   USAGE_VS_USAGE_MISMATCH                  = 0xd5,
   UNSUPPORTED_USAGE                        = 0xd6,
   ILLEGAL_USAGE                            = 0xd7,
   ILLEGAL_TYPE                             = 0xd8,
   ILLEGAL_ACCESS_RIGHT                     = 0xd9,
   ILLEGAL_WRITING_METHOD                   = 0xda,
   PARENT_REQUIRED                          = 0xdb,
   PARENT_NOT_REQUIRED                      = 0xdc,
   EXISTING_REG_NOT_COMPATIBLE              = 0xde,
   TYPE_VS_PARENT_TYPE_MISMATCH             = 0xdf,
   UNDER_CONSTRUCTION                       = 0xe0,
   MAN_LOAD_NON_KDK                         = 0xe1,
   NOT_ALLOWED_DIRECT_INJECTION             = 0xe2,
   ERR_CMD_NOT_ENABLED                      = 0xe3,
   ERR_CMD_NOT_ALLOWED                      = 0xe4,
   ERR_INVALID_CIPHER_MODE                  = 0xe5,
   REGISTER_NOT_EMPTY                       = 0xe6,     // 230d
   ERR_NONCE_REG_TOO_BIG                    = 0xe7,
   DES_NOT_ALLOWED                          = 0xe8,
   DEFAULT_REG                              = 0xe9,
   SESSION_COUNTER_DIFF                     = 0xea,
   MESSAGE_COUNTER_DIFF                     = 0xeb,
   MASTERKEY_CIRCULARLOAD_NOT_ALLOWED       = 0xec,
   CFS_VERSION_INCOMPATIBILITY              = 0xed,
   DATA_DECRYPT_NOT_ALLOWED                 = 0xee,
   SAME_KEY_ALREADY_LOADED									= 0xef,
   

// Generic RSA Key Manager return codes (begin):
   MPA_READ_ERROR                           = 0x130,    // Cannot read from the MPA
   INVALID_DATA                             = 0x131,    // The data passed with the command was invalid for the command.
   TOO_LITTLE_DATA                          = 0x132,    // Not enough data was sent.
   TOO_MUCH_DATA                            = 0x133,    // Too much data was sent.
   INTERNAL_EPP_FIRM_ERROR                  = 0x134,    // The only recourse is to decommission the EPP and retry.
   INVALID_VERSION_NO                       = 0x135,    // Invalid version number in ASN.1 structure.
   INVALID_DIGEST_ALGOID                    = 0x136,    // Invalid digest algorithm identifier.
   INVALID_ISSUER_SERIAL_NO                 = 0x137,    // Invalid issuer and serial number structure.
   INVALID_PARAMETER                        = 0x138,    // Invalid input parameter to function
   OBJECT_NOT_FOUND                         = 0x139,    // ASN.1 object missing
   INVALID_BER_STRUCTURE                    = 0x13a,    // bad formatted ASN.1 structure
   INVALID_CERT_VERSION                     = 0x13b,    // Unknown certificate version (!= 3)
   UNSUPPORTED_STRING_TYPE                  = 0x13c,    // as it says
   INVALID_CN_DATA                          = 0x13d,    // Bad formatted CommonName element
   UNKNOWN_SUBJECT_CN                       = 0x13e,    // Host or EPP serial no cannot be found in
   UNKNOWN_CERT_KEY                         = 0x13f,    // not E, V, or C keytypes
   MPA_WRITE_ERROR                          = 0x140,    // Cannot write to the MPA
   INVALID_ATTRIBUTE                        = 0x141,    // Subject id not recognized
   INVALID_CERTID                           = 0x142,    // Unknown certificate identifier
   BAD_VALIDITY_FORMAT                      = 0x143,    // bad structure of the valifity element or data format not in UTC
   BAD_PUBLIC_KEY_FORMAT                    = 0x144,    // Structure of public key element incorrect
   BAD_PUBLIC_KEY_TYPE                      = 0x145,    // modulus is not as expected an integer element
   BAD_PUBLIC_EXPONENT                      = 0x146,    // exponent is not as expected an integer element
   UNSUPPORTED_ENCRYPTIONALGO               = 0x147,
   UNSUPPORTED_KEYUSAGE                     = 0x148,    // the keyusage specified is not supported
   PUBLICKEY_LEN_MISMATCH                   = 0x149,    // Mismatch when comparing internal key modulul len  with received modulus
   PUBLIC_MOD_MISMATCH                      = 0x14a,    // Mismatch when comparing internal key modululs value with received modulus
   PUBLIC_EXP_MISMATCH                      = 0x14b,    // Mismatch when comparing internal key exponent value with received modulus
   INVALID_NONCE                            = 0x14c,    // mismatch between own nonce - received or bad length of field
   INVALID_SIGN_ALGOID                      = 0x14d,    // Signature Algorithm Value or Length error.
   EMPTY_CERTIFICATE                        = 0x14e,    // The requested certificate is empty
   KEY_STORAGE_ERROR                        = 0x14f,    // PIN-reg. MPA-read error! (should not be possible! / implies an NVM failure).
// Generic RSA Key Manager return codes (end):
   

// Error-codes returned by VerifyRecoveredPublicKeyCert():
   ERR_LENGTH_DIFF                          = 0x209,
   ERR_RECOVERED_DATA_TRAILER               = 0x20A,
   ERR_RECOVERED_DATA_HEADER                = 0x20B,
   ERR_CERTIFICATE_FORMAT                   = 0x20C,
   ERR_HASH_ALGORITHM_INDICATOR             = 0x20D,
   ERR_RECOVERED_VS_CALCULATED_HASH         = 0x20E,
   ERR_RECOVERED_PAN                        = 0x20F,
   ERR_RECOVERED_ALGORITHM_INDICATOR        = 0x210,
// Error-codes returned by VerifySelfSignedAssData():
   ERR_RID                                  = 0x211,
   ERR_PK_LENGTH                            = 0x212,
   ERR_EUP_PUBLIC_KEY_INDEX                 = 0x213,
   ERR_SELF_CERTIF_EXTENDED_DATA            = 0x214,
   ERR_ILLEGAL_PAN_SIZE                     = 0x215,
   ERR_ILLEGAL_ID_NUMBER                    = 0x216,
   ERR_CERTIFICATE_NOT_YET_ACTIVE           = 0x218,
   ERR_ILLEGAL_SIGNED_DATA_FORMAT           = 0x219,
   ERR_FORMAT_UNKNOWN                       = 0x220,
   ERR_ICC_CERTIFICATE_FORMAT               = 0x221,
   ERR_DDA_CERTIFICATE_FORMAT               = 0x222,
   ERR_CRYPTOGRAMS_NOT_EQUAL                = 0x223,
   ERR_INVALID_CURRENT_YEAR                 = 0x224,
   ERR_DYNAMIC_DATA_LENGTH                  = 0x225,
   ERR_ILLEGAL_PAN                          = 0x226,
   ERR_VERIFYING_PAN                        = 0x227,
   ERR_CDA_RECOVERED_VS_CALCULATED_HASH     = 0x228,
// End EMV/GPA error codes
// Error-codes returned by the ACP channel encryption
   ENC_ACP_LENGTH_ERROR                     = 0x230,    // Length error in the encrypted ACP data (not mod 8)
   ENC_ACP_DEC_ERROR                        = 0x231,    // Decryption error in the ACP data
   ENC_ACP_KEY_ERROR                        = 0x232,    // Problem/missing ACP decryption key
   ENC_ACP_PAD_ERROR                        = 0x233,    // Padding error in decrypted the ACP data
   ENC_ACP_CRC_ERROR                        = 0x234,    // CRC in the decrypte ACP data
// End encrypted ACP return codes
// Error-codes returned from external device (Magtek)
   ERR_EXT_DEVICE_OK                        = 0x240,    // Command send to EXTernal device failed on the device
   ERR_EXT_DEVICE_CMD_FAIL                  = 0x241,    // Command send to EXTernal device failed on the device
   ERR_EXT_DEVICE_CMD_BAD                   = 0x242,    // Bad parameters or Bad command send to EXTernal device
   ERR_EXT_DEVICE_SOF                       = 0x246,    // Expected response header not received
   ERR_EXT_DEVICE_TIMEOUT                   = 0x247,    // Expected response not received
   ERR_EXT_DEVICE_KEYERR                    = 0x248,    // EXTernal device key match error. Key on device do not match key in MPA
   ERR_EXT_DEVICE_UNEXPT                    = 0x249,    // Unexpected response from EXTernal device.
   ERR_EXT_DEVICE_ILL_LEN                   = 0x24A,    // Unexpected length field (greater than buffer)
// End


// General return codes continued
   PAR1_NOT_ACCEPTED                        = 0x0600,
   PAR2_NOT_ACCEPTED                        = 0x0601,
   PAR3_NOT_ACCEPTED                        = 0x0602,
   PAR4_NOT_ACCEPTED                        = 0x0603,
   PAR5_NOT_ACCEPTED                        = 0x0604,
   // Reserved for PARX_NOT_ACCEPTED
   SIGNING_DATA_FAILED                      = 0x060A,
   ENCRYPTION_ERROR                         = 0x060B,
   DECRYPTION_ERROR                         = 0x060C,
   ERROR_TRANSLATION_DUMMY                  = 0x060D,   // Used when a command only returns OK to indicate that error translation has been implemented
   INVALID_X509_DATA                        = 0x060E,
   KEY_MISMATCH                             = 0x060F,
   UNKNOWN_CERT_TYPE                        = 0x0610,
   KBH_ERROR                                = 0x0611,   // The TR31 KBH data block is not accepted
   ASCII_CONVERSION_ERROR                   = 0x0612,   // Conversion to/from ascii failed
   BMP60_ERROR                              = 0x0613,   // Error in data to or from BMP60.
   BMP61_NOT_PRESENT                        = 0x0614,   // ISO8583-BMP61 was expected but not present.
   BMP61_PRESENT                            = 0x0615,   // ISO8583-BMP61 was NOT expected but present.
   KEY_LENGTH_SPEC_ERROR                    = 0x0616,   // Key-length specifier invalid
   KEY_GENERATION_ERROR                     = 0x0617,   // Invalid Key-Version or Key-Generation number.
   BMP61_LENGTH_ERROR                       = 0x0618,   // ISO8583-BMP61 unexpected length.
   DATE_FORMAT_ERROR                        = 0x0619,   // Invalid format or range of DATE.
   DATE_FORMAT_ERROR_GROUP_FFFF00           = 0x061a,   // Invalid format or range of DATE for K_INIT_T or K_PERS_T.

   
/*** IMPORTANT: These return codes are for the command interpreter only ***/
   CMD_INT_UNKNOWN                          = 0x0620,
   CMD_INT_TO_FEW_PARAMETERS                = 0x0621,
   CMD_INT_TOO_MANY_PARAMETERS              = 0x0622,
   CMD_INT_PARAMETER_TOO_SHORT              = 0x0623,
   CMD_INT_PARAMETER_TOO_LONG               = 0x0624,
   CMD_INT_WRONG_MPA_NAME                   = 0x0625,
   CMD_INT_REJECTED                         = 0x0626,
  /*********** NB **************************
   * Error codes in the range 0xff01 - 0xff50 are used as
   * internal error codes in the EMV GPA code,
   * the intern error codes are only used as debug guide and will
   * never be returned as a general return code
   * please if possible avoid using these codes
   *****************************************/

