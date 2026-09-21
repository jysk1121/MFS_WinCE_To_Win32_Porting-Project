//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:  

pwindbas.h

Abstract:

Private portion of windbase.h

Notes: 


--*/
   
#ifndef _PRIV_WINDBASE__
#define _PRIV_WINDBASE__

#ifdef __cplusplus
extern "C" {
#endif

// @CESYSGEN IF FILESYS_FSMAIN
  

//------------------------------------------------------------------------------
// BIT-BASED REPLICATION
//------------------------------------------------------------------------------

/*
@func BOOL | CeGetReplChangeMask | Gets current replication change mask
@parm LPDWORD | lpmask | gets set to current replication mask
@comm Obtains the current replication change bit mask.  Currently only 2
		bits are supported.  Returns TRUE for success.
*/ 
BOOL CeGetReplChangeMask (LPDWORD lpmask);

/*
@func BOOL | CeSetReplChangeMask | Sets current replication change mask
@parm DWORD | mask | The new replication change mask
@comm Sets a new replication change bit mask.  Currently only 2 bits are supported.
		Returns TRUE for success.
*/ 
BOOL CeSetReplChangeMask (DWORD mask);

/*
@func BOOL | CeGetReplChangeBits | Gets the change bits for a particular oid
@parm CEOID | oid | oid to get change bits for
@parm LPDWORD | lpbits | Gets set to current bits for that oid
@parm DWORD | dwFlags | If set to REPL_CHANGE_WILLCLEAR, then the next call to
	CeClearReplChangeBits will actually clear these bits unless there has been
	a subsequent change to the item
@comm Obtains the current change bits for an oid.  If the REPL_CHANGE_WILLCLEAR flag
	is set, then the CeClearReplChangeBits api can be used next to actually clear
	some of the bits.  Currently only 2 bits are supported.
*/ 
BOOL CeGetReplChangeBitsEx (PCEGUID pguid, CEOID oid, LPDWORD lpbits, DWORD dwFlags);
#define CeGetReplChangeBits(oid,lpbits,dwFlags)	CeGetReplChangeBitsEx(NULL,oid,lpbits,dwFlags)

/*
@func BOOL | CeSetReplChangeBits | Sets selected change bits for an oid
@parm CEOID | oid | oid to get change bits for
@parm DWORD | mask | Bits to set
@comm This clears the bits which are set in the mask.  Currently only 2 bits are supported.
*/
BOOL CeSetReplChangeBitsEx (PCEGUID pguid, CEOID oid, DWORD mask);
#define CeSetReplChangeBits(oid,mask) CeSetReplChangeBitsEx(NULL,oid,mask)

/*
@func BOOL | CeClearReplChangeBits | Clears selected change bits for an oid
@parm CEOID | oid | oid to get change bits for
@parm DWORD | mask | Bits to clear
@comm If the oid hasn't changed since the last call to CeGetReplChangeBits with the
	REPL_CHANGE_WILLCLEAR flag, it clears the bits which are set in the mask.  Currently
	only 2 bits are supported.
*/ 
BOOL CeClearReplChangeBitsEx (PCEGUID pguid, CEOID oid, DWORD mask);
#define CeClearReplChangeBits(oid,mask) CeClearReplChangeBitsEx(NULL,oid,mask)

/*
@func BOOL | CeGetReplOtherBits | Retrives the replication private bits for an oid
@parm CEOID | oid | oid to get bits for
@parm LPDWORD | lpbits | Gets current bits
@comm Gets the current private bits for an oid.  Currently only 1 bit is supported.
*/ 
BOOL CeGetReplOtherBitsEx (PCEGUID pguid, CEOID oid, LPDWORD lpbits);
#define CeGetReplOtherBits(oid,lpbits) CeGetReplOtherBitsEx(NULL,oid,lpbits)

/*
@func BOOL | CeSetReplOtherBits | Sets the replication private bits for an oid
@parm CEOID | oid | oid to set bits for
@parm DWORD | bits | New bits
@comm Sets the current private bits for an oid.  Currently only 1 bit is supported.
*/ 
BOOL CeSetReplOtherBitsEx (PCEGUID pguid, CEOID oid, DWORD bits);
#define CeSetReplOtherBits(oid,bits) CeSetReplOtherBitsEx(NULL,oid,bits)

#define OID_REPL				2
#define MAKEREPLOID(bits28) (((OID_REPL)<<28)|((bits28)&0x0fffffff))
#define ISREPLOID(dwOid) ((dwOid>>28)==OID_REPL)
#define GETREPLOIDBITS(dwOid) ((dwOid)&0x0fffffff)


//------------------------------------------------------------------------------
// COUNT-BASED REPLICATION
//------------------------------------------------------------------------------

typedef DWORD HREPL;
#define INVALID_HREPL INVALID_HANDLE_VALUE

// Sync flags control settings for the sync session
#define REPL_SYNC_ALLOWOVERWRITE    0x00000001  // Overwrite unsync'd data
// DB-only and file-only sync is not yet supported but their flags are defined for future use
#define REPL_SYNC_DB_TYPES          0x00000100  // Sync only database objects (databases, records)
#define REPL_SYNC_FILE_TYPES        0x00000200  // Sync only file system objects (files, directories)
#define REPL_SYNC_ALL_TYPES         (REPL_SYNC_DB_TYPES | REPL_SYNC_FILE_TYPES)
#define REPL_VALIDSYNCFLAGS         (REPL_SYNC_ALLOWOVERWRITE | REPL_SYNC_ALL_TYPES)

// Flags used in the ReplChangeInfo struct
#define REPLSTATUS_ERROR     0  // Item status could not be retrieved
#define REPLSTATUS_CONFLICT  1  // Item cannot be sync'd by this client yet
#define REPLSTATUS_UNCHANGED 2  // Item has been previously sync'd or written by client
#define REPLSTATUS_DELETED   3  // Item was deleted by another client or application
#define REPLSTATUS_MODIFIED  4  // Item has been modified by another client or application
                                // (MODIFIED is generic: Unknown whether it's a create/change/recreate)
// The following flags are currently unused, but may be used in the future to
// supply more specific information
//#define REPLSTATUS_CREATED   5  // Item was newly created
//#define REPLDISP_CHANGED     6  // Item was changed
//#define REPLDISP_RECREATED   7  // Item was deleted and then recreated

// Change information about a single object
#define REPLCHANGEINFO_VERSION  1
typedef struct _ReplChangeInfo {
    WORD  wVersion;     // Version of this struct, set to 1
    WORD  wStatus;      // Disposition: Deleted/Modified
    WORD  wObjType;     // Object type: OBJTYPE_*
    WORD  wReserved;    // DWORD alignment
    CEOID oidChanged;   // OID of the changed item
    CEOID oidParent;    // For record objects, oid of the DB owning it
                        // For files and directories, oid of the directory it is in
} REPLCHANGEINFO;

// This macro provides repl clients with the part of the OID that should be used
// to track objects: it is always unique within a volume, but if an object is
// deleted, a new object may be given its search ID (but a different OID).
// Changes are tracked per search ID, not per OID.
#define GetOidSearchID(ceoid)   (((DWORD)(ceoid)) & 0x00FFFFFF)


BOOL ReplOpenSync(CEGUID* pguid, LPBYTE lpSyncState, DWORD dwSize,
                  DWORD dwSyncFlags, HREPL* phRepl);
BOOL ReplCheckpoint(HREPL hRepl, HANDLE hChangeEvent);
BOOL ReplCloseSync(HREPL hRepl);
BOOL ReplGetSyncState(HREPL hRepl, LPBYTE lpSyncState, DWORD* lpdwSize,
                      BOOL fFailIfUnchanged);
BOOL ReplChangeSyncSettings(HREPL hRepl, DWORD dwSyncFlags, DWORD* lpdwOldFlags);
BOOL ReplFindNextChange(HREPL hRepl, REPLCHANGEINFO* lpReplInfo, BOOL fRestart);
BOOL ReplGetOidStatus(HREPL hRepl, CEOID oid, REPLCHANGEINFO* lpReplInfo);



void DumpFileSystemHeap(void);

/*
@doc INTERNAL
@func BOOL | GetSystemMemoryDivision | Gets information on object store and system memory
@parm LPDWORD | lpdwStorePages | receives number of pages dedicated to the store
@parm LPDWORD | lpdwRamPages | receives number of pages dedicated to system memory
@parm LPDWORD | lpdwPageSize | receives the number of bytes in a page
@comm Gets information on object store and system memory
*/ 
BOOL GetSystemMemoryDivision (LPDWORD lpdwStorePages, LPDWORD lpdwRamPages, LPDWORD lpdwPageSize);

#define SYSMEM_CHANGED	0
#define SYSMEM_MUSTREBOOT 1
#define SYSMEM_REBOOTPENDING 2
#define SYSMEM_FAILED 3

/*
@doc INTERNAL
@func DWORD | SetSystemMemoryDivision | Sets number of pages for the object store
@parm DWORD | dwStorePages | Number of pages to allocate for the store
@comm Returs SYSMEM_CHANGED on success.  Returns SYSMEM_MUSTREBOOT if a reboot is neccessary to
	get the change to take effect.  Returns SYSMEM_REBOOTPENDING if a previous call returned
	SYSMEM_MUSTREBOOT, in which case the boundary cannot be adjusted further until the reboot
	happens.  Returns SYSMEM_FAILED if the dwStorePages is out of range (you cannot shrink
	system memory or the object store memory beyond a certain point, 256K each plus space for
	existing files and records.  If SYSMEM_FAILED is returned, you can get further information
	by calling GetLastError.
*/ 
DWORD SetSystemMemoryDivision (DWORD dwStorePages);

/*
  @func BOOL | CeRegisterFileSystemNotification | For internal use of the shell
  @parm HWND | hWnd | Window handle to which notifications will be posted
  @comm Posts a DB_* message to hWnd on any change in the file system.  Subsequent calls
  	override earlier calls.  Returns FALSE for failure.
  */
BOOL CeRegisterFileSystemNotification (HWND hWnd);
  
  /*
  @func BOOL | CeRegisterReplNotification | For internal use of replication
  @parm HWND | hWnd | Window handle for notifications
  @comm Posts a message of the form DB_CEOID_* when something changes in the file system or
   the system database.  Pass in the NULL hWnd to cancel notification.  Only one window
   may be registered at a time.  Subsequent calls override earlier calls.  Returns FALSE
   for failure.
  */ 
BOOL CeRegisterReplNotification (CENOTIFYREQUEST *pRequest);

#define AFS_FLAG_HIDDEN     1   // Hidden file system
#define AFS_FLAG_BOOTABLE   2   // May contain system registry
#define AFS_FLAG_ROOTFS     4   // Mount as root of file system, "\"
#define AFS_FLAG_HIDEROM    8   // Hide ROM when mounting FS root; use with FLAG_ROOTFS -- depricated
#define AFS_FLAG_MOUNTROM   16  // Mount the new filesystem as an additional ROM filesystem
#define AFS_FLAG_SYSTEM     32  // Mount as a system volume, which untrusted processes cannot access
#define AFS_FLAG_PERMANENT  64  // Mount as a permanent volume that cannot be unmounted

// Functions
BOOL RegisterAFSEx (int index, HANDLE h, DWORD dw, DWORD version, DWORD flags);
BOOL DeregisterAFS (int index);
int RegisterAFSName(LPCWSTR pName);
BOOL DeregisterAFSName(int index);

// @CESYSGEN ENDIF

// @CESYSGEN IF FILESYS_FSDBASE

/*
@func VOID | CeChangeDatabaseLCID | Changes the LCID for all database sorts
@parm DWORD | lcid | The lcid wanted
@comm Changes what the LCID used for all database sorts is
*/
VOID CeChangeDatabaseLCID (PCEGUID pguid, DWORD LCID);

BOOL DBCanonicalize(LCID Locale, LPCWSTR lpSrc, DWORD dwSrcLen, LPWSTR lpDst);

// @CESYSGEN ENDIF

#ifdef __cplusplus
}
#endif

    
#endif //_PRIV_WINDBASE_H_

