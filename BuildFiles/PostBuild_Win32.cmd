@ECHO OFF
REM ============================================================================
REM  PostBuild_Win32.cmd  -  Win32 (desktop simulator) deployment only
REM
REM  This script is NOT used by the WinCE build. WinCE keeps using BuildScript.bat,
REM  which deploys to ..\..\EAGLECE_ATM_EXE / ..\..\SoftwarePackage and code-signs
REM  the binaries. Nothing here touches those folders.
REM
REM  Everything is copied next to the built EagleCE_ATM.exe because the app
REM  resolves its data with CUtil::GetAppPath(), which returns the directory of
REM  the running module. That makes both "run the exe directly" and "F5 debug
REM  from Visual Studio" work identically.
REM
REM  Usage (invoked from the vcproj post-build step):
REM      PostBuild_Win32.cmd "<full path to $(OutDir)>"
REM ============================================================================

SETLOCAL

IF "%~1"=="" (
    ECHO [PostBuild_Win32] ERROR: output directory argument missing.
    EXIT /B 1
)

SET "OUT_DIR=%~1"
IF "%OUT_DIR:~-1%"=="\" SET "OUT_DIR=%OUT_DIR:~0,-1%"

REM This script lives in Nexcom\BuildFiles, so the repository root is one level up.
SET "ROOT=%~dp0.."
SET "ATM_DIR=%ROOT%\EagleCE_ATM\EagleCE_ATM"
SET "HOST_DIR=%ROOT%\EagleCE_Host\EagleCE_Host"
SET "DEVICE_DIR=%ROOT%\EagleCE_Device\EagleCE_Device"
SET "EJL_DIR=%ROOT%\DLL_SRC\MFS_EJL_WEC7\MFS_EJL_WEC7"
REM Prebuilt Win32 runtime DLLs (the ARM equivalents live in DLL_CE).
SET "DLL_WIN32_DIR=%ROOT%\DLL_Win32"

ECHO ###################################################
ECHO  POST BUILD (Win32 simulator)  -^> %OUT_DIR%
ECHO ###################################################

IF NOT EXIST "%OUT_DIR%" MKDIR "%OUT_DIR%"

REM ---------------------------------------------------------------- runtime DLLs
ECHO - Copying EagleCE_Host.dll ...
XCOPY "%HOST_DIR%\Win32\%~2\EagleCE_Host.dll" "%OUT_DIR%\" /y /q >NUL
IF ERRORLEVEL 1 ECHO   [warn] EagleCE_Host.dll not found - build EagleCE_Host first.

ECHO - Copying third-party DLLs (OpenSSL / curl / CxImage) ...
XCOPY "%DLL_WIN32_DIR%\*.dll" "%OUT_DIR%\" /y /q >NUL
IF ERRORLEVEL 1 ECHO   [warn] third-party DLLs not found in %DLL_WIN32_DIR%.

REM The electronic journal DLL is built from DLL_SRC for Win32 (WinCE ships a
REM prebuilt ARM binary in DLL_CE instead). DEV_EJL loads it by plain name, so
REM it has to sit next to the executable.
ECHO - Copying MFS_EJL_WEC7.dll ...
XCOPY "%EJL_DIR%\Win32\%~2\MFS_EJL_WEC7.dll" "%OUT_DIR%\" /y /q >NUL
IF ERRORLEVEL 1 ECHO   [warn] MFS_EJL_WEC7.dll not found - build MFS_EJL_WEC7 first.

REM ---------------------------------------------------------------- screen data
ECHO - Copying Screen files ...
XCOPY "%ATM_DIR%\Screen\*" "%OUT_DIR%\Screen\" /s /y /q >NUL

ECHO - Copying Config files ...
XCOPY "%ATM_DIR%\Config\*" "%OUT_DIR%\Config\" /s /y /q >NUL

ECHO - Copying Wave files ...
XCOPY "%ATM_DIR%\Wave\*" "%OUT_DIR%\Wave\" /s /y /q >NUL

ECHO - Copying Advertisement files ...
XCOPY "%ATM_DIR%\AD\*" "%OUT_DIR%\AD\" /s /y /q >NUL

ECHO - Copying Receipt logo ...
XCOPY "%ATM_DIR%\Receipt\*" "%OUT_DIR%\Receipt\" /s /y /q >NUL

REM ---------------------------------------------------------------- certificates
REM Read via GetAppPath() + "\xxx.pem", so they belong directly beside the exe.
ECHO - Copying certificates ...
XCOPY "%ATM_DIR%\Certificate\*.pem" "%OUT_DIR%\" /y /q >NUL

REM ---------------------------------------------------------------- runtime dirs
ECHO - Creating runtime folders ...
IF NOT EXIST "%OUT_DIR%\Trace"        MKDIR "%OUT_DIR%\Trace"
IF NOT EXIST "%OUT_DIR%\Temp"         MKDIR "%OUT_DIR%\Temp"
IF NOT EXIST "%OUT_DIR%\CaptureImage" MKDIR "%OUT_DIR%\CaptureImage"

REM ---------------------------------------------------------------- simulator ini
REM Never overwrite an existing DeviceSim.ini: it holds the tester's own settings.
IF NOT EXIST "%OUT_DIR%\DeviceSim.ini" (
    ECHO - Creating DeviceSim.ini from template ...
    COPY "%DEVICE_DIR%\DeviceSim.ini.template" "%OUT_DIR%\DeviceSim.ini" /y >NUL
) ELSE (
    ECHO - DeviceSim.ini already present - kept as is.
)

REM No code signing on Win32: the desktop simulator does not need signed binaries.

ECHO ###################################################
ECHO  POST BUILD (Win32) DONE
ECHO ###################################################

ENDLOCAL
EXIT /B 0
