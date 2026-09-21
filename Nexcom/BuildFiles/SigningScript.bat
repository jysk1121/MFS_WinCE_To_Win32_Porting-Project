@ECHO OFF

SET CRT_DIR=.
SET EXE_DIR=..\SoftwarePackage\Install
SET SWUPDATE_EXE_DIR=..\SoftwarePackage

REM Execute Certification 
%CRT_DIR%\signtool.exe sign /f %CRT_DIR%\MFS_CA_CERT.pfx %EXE_DIR%\*.EXE
%CRT_DIR%\signtool.exe sign /f %CRT_DIR%\MFS_CA_CERT.pfx %EXE_DIR%\*.DLL

%CRT_DIR%\signtool.exe sign /f %CRT_DIR%\MFS_CA_CERT.pfx %SWUPDATE_EXE_DIR%\*.EXE
%CRT_DIR%\signtool.exe sign /f %CRT_DIR%\MFS_CA_CERT.pfx %SWUPDATE_EXE_DIR%\*.DLL

;%CRT_DIR%\signtool.exe sign /f MFS_CA_CERT.pfx %EXE_DIR%\*.EXE
;%CRT_DIR%\signtool.exe sign /f MFS_CA_CERT.pfx %EXE_DIR%\*.DLL

;%CRT_DIR%\signtool.exe sign /f MFS_CA_CERT.pfx %SWUPDATE_EXE_DIR%\*.EXE
;%CRT_DIR%\signtool.exe sign /f MFS_CA_CERT.pfx %SWUPDATE_EXE_DIR%\*.DLL

ECHO #################################
ECHO          POST BUILD END
ECHO #################################
GOTO QUIT

:QUIT
@ECHO ON