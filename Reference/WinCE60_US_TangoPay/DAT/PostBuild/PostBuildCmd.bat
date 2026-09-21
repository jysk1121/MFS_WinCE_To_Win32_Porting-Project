@ECHO OFF

SET ATM_SITE=%1
REM SET ATM_DEALER=%1
SET CE_VER=%2
SET EMV_ON=ON
SET OP_LOOK=OP

REM //////////////////////////////////
REM ATM_SITE 국가
REM ATM_DEALER 고객(호주만 국가 <> 고객, 그외는 국가 == 고객)
REM V06.00.04부터 호주도 ATM_DEALER 사용안하도록 통합됨
REM //////////////////////////////////


REM //////////////////////////////////
REM 호주의 경우에는 이 bat파일에서 고객을 REM처리해주어야 함 (사용 안함)
REM //////////////////////////////////
REM IF /i "%ATM_SITE%" == "AU" SET ATM_DEALER=AU_A
REM IF /i "%ATM_SITE%" == "AU" SET ATM_DEALER=AU_C

IF /i "%ATM_SITE%" == "US" GOTO SETVARIABLE
IF /i "%ATM_SITE%" == "CA" GOTO SETVARIABLE
IF /i "%ATM_SITE%" == "AU" GOTO SETVARIABLE
IF /i "%ATM_SITE%" == "MX" GOTO SETVARIABLE
goto ERROR

REM //////////////////////////////////
REM           LABLE : SETVARIABLE
REM //////////////////////////////////
:SETVARIABLE
SET EXE_DIR=..\..\ExeRele_%CE_VER%_%ATM_SITE%
SET CRT_DIR=..\..\DAT\POSTBUILD
SET BUILDER_DIR=%CRT_DIR%\APBuilder

SET SRC_DIR=..\..
SET OUTPUT_DIR=..\..\UpdateFiles
SET MASTERFILES_DIR=..\..\DAT\MasterFiles

SET SEVERITY=%SEVERITY%
SET UPDATE_TYPE=%RELEASE_TYPE%

IF "%CE_VER%" == "70" (
    del %EXE_DIR%\ /F /Q
    mkdir %EXE_DIR%\
    copy ..\..\ExeRele_60_%ATM_SITE% %EXE_DIR%\
    copy ..\..\ExeRele_60_%ATM_SITE% %EXE_DIR%\
)

ECHO ----- Deleting (*.lib, *.exp)..
DEL %EXE_DIR%\*.lib /F /Q
DEL %EXE_DIR%\*.exp /F /Q

IF "%CE_VER%" == "60" (
    REM Execute Certification 
    %CRT_DIR%\signtool.exe sign /f %CRT_DIR%\MX5300CE2ndCert.pfx %EXE_DIR%\*.EXE
    %CRT_DIR%\signtool.exe sign /f %CRT_DIR%\MX5300CE2ndCert.pfx %EXE_DIR%\*.DLL
    %CRT_DIR%\signtool.exe sign /f %CRT_DIR%\MX5300CE2ndCert.pfx %EXE_DIR%\*.OCX
) ELSE (
    REM WinCE 70 signatures
    powershell.exe .\sign_wince7.ps1 %EXE_DIR%
)

REM powershell.exe -ExecutionPolicy Unrestricted  %BUILDER_DIR%\build_all.ps1 %ATM_SITE%

ECHO ----- Building Master Zips...
REM ----- CWD: "(src_dir)\App\WinAtm"
py.exe %BUILDER_DIR%\build.py build -t %ATM_SITE% -o %OUTPUT_DIR% -s %SRC_DIR% -f %MASTERFILES_DIR% -V %CE_VER% -m 1500SE
py.exe %BUILDER_DIR%\build.py build -t %ATM_SITE% -o %OUTPUT_DIR% -s %SRC_DIR% -f %MASTERFILES_DIR% -V %CE_VER% -m 1800SE -m 2700SE -m 2700T -m 2800SE -m 2800T -m 5200SE

REM This is a flaw in the current design on py-hsm. The libhsm.dll must be in the CWD, so we'll relocate them breifly.
copy %BUILDER_DIR%\libhsm.dll .
copy %BUILDER_DIR%\fxpkcs11.dll .

ECHO ----- Signing Files...
REM use "start /b" to run "py.exe" in parallel
start /b py.exe %BUILDER_DIR%\build.py sign -o %OUTPUT_DIR%\%CE_VER%\1500SE\manifest.xml   -src %SRC_DIR% -t %ATM_SITE% -m 1500SE -S %SEVERITY% -T %UPDATE_TYPE%       -d nha_hsm -c MyLabel -v 1.1.0 -f %BUILDER_DIR%\certs -k %BUILDER_DIR%\mxkey.pem %OUTPUT_DIR%\%CE_VER%\1500SE
start /b py.exe %BUILDER_DIR%\build.py sign -o %OUTPUT_DIR%\%CE_VER%\1800SE\manifest.xml   -src %SRC_DIR% -t %ATM_SITE% -m 1800SE -S %SEVERITY% -T %UPDATE_TYPE%       -d nha_hsm -c MyLabel -v 1.1.0 -f %BUILDER_DIR%\certs -k %BUILDER_DIR%\mxkey.pem %OUTPUT_DIR%\%CE_VER%\1800SE
start /b py.exe %BUILDER_DIR%\build.py sign -o %OUTPUT_DIR%\%CE_VER%\2700SE\manifest.xml   -src %SRC_DIR% -t %ATM_SITE% -m 2700SE -S %SEVERITY% -T %UPDATE_TYPE%       -d nha_hsm -c MyLabel -v 1.1.0 -f %BUILDER_DIR%\certs -k %BUILDER_DIR%\mxkey.pem %OUTPUT_DIR%\%CE_VER%\2700SE
start /b py.exe %BUILDER_DIR%\build.py sign -o %OUTPUT_DIR%\%CE_VER%\2700T\manifest.xml    -src %SRC_DIR% -t %ATM_SITE% -m 2700T  -S %SEVERITY% -T %UPDATE_TYPE%       -d nha_hsm -c MyLabel -v 1.1.0 -f %BUILDER_DIR%\certs -k %BUILDER_DIR%\mxkey.pem %OUTPUT_DIR%\%CE_VER%\2700T
start /b py.exe %BUILDER_DIR%\build.py sign -o %OUTPUT_DIR%\%CE_VER%\2800SE\manifest.xml   -src %SRC_DIR% -t %ATM_SITE% -m 2800SE -S %SEVERITY% -T %UPDATE_TYPE%       -d nha_hsm -c MyLabel -v 1.1.0 -f %BUILDER_DIR%\certs -k %BUILDER_DIR%\mxkey.pem %OUTPUT_DIR%\%CE_VER%\2800SE
start /b py.exe %BUILDER_DIR%\build.py sign -o %OUTPUT_DIR%\%CE_VER%\2800T\manifest.xml    -src %SRC_DIR% -t %ATM_SITE% -m 2800T  -S %SEVERITY% -T %UPDATE_TYPE%       -d nha_hsm -c MyLabel -v 1.1.0 -f %BUILDER_DIR%\certs -k %BUILDER_DIR%\mxkey.pem %OUTPUT_DIR%\%CE_VER%\2800T
start /b py.exe %BUILDER_DIR%\build.py sign -o %OUTPUT_DIR%\%CE_VER%\5200SE\manifest.xml   -src %SRC_DIR% -t %ATM_SITE% -m 5200SE -S %SEVERITY% -T %UPDATE_TYPE%       -d nha_hsm -c MyLabel -v 1.1.0 -f %BUILDER_DIR%\certs -k %BUILDER_DIR%\mxkey.pem %OUTPUT_DIR%\%CE_VER%\5200SE
start /b /wait py.exe %BUILDER_DIR%\build.py sign -o %OUTPUT_DIR%\%CE_VER%\Combined\manifest.xml -src %SRC_DIR% -t %ATM_SITE% -m none   -S %SEVERITY% -T %UPDATE_TYPE% -d nha_hsm -c MyLabel -v 1.1.0 -f %BUILDER_DIR%\certs -k %BUILDER_DIR%\mxkey.pem %OUTPUT_DIR%\%CE_VER%\Combined

goto END

:ERROR
ECHO ERROR!!!

:END

del libhsm.dll
del fxpkcs11.dll

ECHO #################################
ECHO          POST BUILD END
ECHO #################################

@ECHO ON
