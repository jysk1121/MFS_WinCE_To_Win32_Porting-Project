@ECHO OFF

REM //////////////////////////////////
REM           LABLE : SETVARIABLE
REM //////////////////////////////////
:SETVARIABLE
SET EXE_SW_DIR=..\..\SoftwarePackage
REM SET EXE_DIR=..\..\SoftwarePackage\Install
SET EXE_DIR=..\..\SoftwarePackage\Update
SET BIN_EXE_DIR=..\..\EAGLECE_ATM_EXE
SET SCR_DIR=..\..\EagleCE_ATM\EagleCE_ATM\SCREEN
SET DEVICE_DLL_DIR=..\..\DLL_CE
SET CONFIG_DIR=..\..\EagleCE_ATM\EagleCE_ATM\CONFIG
SET WAVE_DIR=..\..\EagleCE_ATM\EagleCE_ATM\Wave
SET ADV_DIR=..\..\EagleCE_ATM\EagleCE_ATM\AD
SET PRT_LOGO_DIR=..\..\EagleCE_ATM\EagleCE_ATM\Receipt
SET CRT_DIR=..\..\Buildfiles

ECHO #################################
ECHO         POST BUILD START
ECHO #################################

ECHO - Deleting Files..
RMDIR %EXE_SW_DIR% /S /Q

ECHO - Deleting (*.lib, *.exp *pdb)..
DEL %BIN_EXE_DIR%\*.lib
DEL %BIN_EXE_DIR%\*.exp
DEL %BIN_EXE_DIR%\*.pdb


ECHO - Copying Advertisement files..
XCOPY /s %ADV_DIR%\*    %EXE_DIR%\AD\ /y /q

REM MF200은 추후 양산시 지원
REM RMDIR %EXE_DIR%\AD\1024_768 /S /Q


ECHO - Copying Screen Files...
XCOPY /s %SCR_DIR%\*    %EXE_DIR%\Screen\ /y /q

REM MF200은 추후 양산시 지원
REM RMDIR %EXE_DIR%\Screen\1024_768 /S /Q


ECHO - Deleting Unnessary Screen Files...
DEL %EXE_DIR%\Screen\1280_800\*.dat
DEL %EXE_DIR%\Screen\1280_800\Supervisor\*.dat
DEL %EXE_DIR%\Screen\1024_768\*.dat
DEL %EXE_DIR%\Screen\1024_768\Supervisor\*.dat

ECHO - Animation Image Screen Files...
RMDIR %EXE_DIR%\Screen\1024_768\Client_MF200_Image /S /Q

REM RMDIR %EXE_DIR%\Screen\1024_768\Client /S /Q
REM move %EXE_DIR%\Screen\1024_768\Client_MF200_Image %EXE_DIR%\Screen\1024_768\Client

REM ECHO - Machine Image Screen Files...
REM RMDIR %EXE_DIR%\Screen\1024_768\Client_Animation /S /Q

ECHO - Copying Config Files...
XCOPY /s %CONFIG_DIR%\*    %EXE_DIR%\Config\ /y /q

ECHO - Delete default config Files...
DEL %EXE_DIR%\Config\Default\*.CFG

ECHO - Copying Logo Image Files...
XCOPY /s %PRT_LOGO_DIR%\*    %EXE_DIR%\Receipt\ /y /q

ECHO - Delete error code csv file
DEL %EXE_DIR%\Config\*.csv

ECHO - Copying Wave Files...
XCOPY /s %WAVE_DIR%\*    %EXE_DIR%\Wave\ /y /q

ECHO - Copying EagleCE Binary Files...
XCOPY %BIN_EXE_DIR%\*.*								 %EXE_DIR% /y /q
REM XCOPY %BIN_EXE_DIR%\*.*						 %EXE_UPDATE_DIR% /y /q

ECHO - Copying SoftwareUpdate Binary Files...
copy %BIN_EXE_DIR%\SoftwareUpdate.exe		%EXE_SW_DIR%\SoftwareUpdate.exe

ECHO - Delete SoftwareUpdate Binary Files...
DEL %EXE_DIR%\SoftwareUpdate.exe

REM DLL_CE holds every prebuilt ARM DLL, TLS ones (OpenSSL/curl) included.
ECHO - Copying Device Dll Binary Files...
XCOPY %DEVICE_DLL_DIR%\*.*						 %EXE_DIR% /y /q

ECHO #################################
ECHO          EXECUTE CERTIFICATION
ECHO #################################
%CRT_DIR%\signtool.exe sign /f %CRT_DIR%\MFS_CA_CERT.pfx %EXE_DIR%\*.EXE
%CRT_DIR%\signtool.exe sign /f %CRT_DIR%\MFS_CA_CERT.pfx %EXE_DIR%\*.DLL

%CRT_DIR%\signtool.exe sign /f %CRT_DIR%\MFS_CA_CERT.pfx %EXE_SW_DIR%\*.EXE
REM %CRT_DIR%\signtool.exe sign /f %CRT_DIR%\MFS_CA_CERT.pfx %EXE_SW_DIR%\*.DLL



ECHO #################################
ECHO          POST BUILD END
ECHO #################################
GOTO QUIT

:QUIT
@ECHO ON