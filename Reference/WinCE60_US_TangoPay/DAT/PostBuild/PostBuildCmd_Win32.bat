@ECHO OFF

if EXIST "%nextware%\..\form\ptrform\30\slipform-win32.wfm" (
	echo "slipform-win32.wfm already installed!"
) ELSE (
	copy "..\..\slipform-win32.wfm" "%nextware%\..\form\ptrform\30\slipform-win32.wfm"
)

SET ATM_SITE=%1
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
GOTO ERROR


REM //////////////////////////////////
REM           LABLE : SETVARIABLE
REM //////////////////////////////////
:SETVARIABLE
SET EXE_DIR=..\..\ExeRele_Win32_%ATM_SITE%\ATM
SET EXE_DIR_UPPER=..\..\ExeRele_Win32_%ATM_SITE%

SET WAV_DEF_DIR=..\..\DAT\WAVE\US
IF /i "%ATM_SITE%" == "CA" SET WAV_DEF_DIR=..\..\DAT\WAVE\CA
IF /i "%ATM_SITE%" == "MX" SET WAV_DEF_DIR=..\..\DAT\WAVE\MX

REM 호주 ADA 전체 재녹음 이후 WAVE 복사 방식 변경됨
REM 기존 : US WAVE 복사 후 일부 AU WAVE 덮어씀
REM 신규 : AU WAVE 단독 복사 (US WAVE 사용안함)
IF /i "%ATM_SITE%" == "AU" SET WAV_DEF_DIR=..\..\DAT\WAVE\AU

SET WAV_DIR=..\..\DAT\WAVE\%ATM_SITE%
SET ERR_DIR=..\..\DAT\ErrorCode
SET INI_DIR=..\..\DAT\INI\%ATM_SITE%
SET SCR_DIR=..\..\DAT\SCREEN
SET BACK_SCR_DIR=..\..\DAT\SCREEN\Backs
SET BACK_2800_SCR_DIR=..\..\DAT\SCREEN\Backs_2800
SET ADV_DIR=..\..\DAT\ADVERTISEMENT
SET CRT_DIR=..\..\DAT\POSTBUILD
SET EMV_DIR=..\..\DAT\EMVINI
SET TTS_DB_DIR=..\..\DAT\TTS_DB

SET EXTRA_DLL_DIR=..\..\DllRele\Win32


REM ADDITIONAL EMV DATA
IF /i "%ATM_SITE%" == "US" SET EMV_DIR_ADD=..\..\DAT\EMVINI\US
IF /i "%ATM_SITE%" == "CA" SET EMV_DIR_ADD=..\..\DAT\EMVINI\CA
IF /i "%ATM_SITE%" == "AU" SET EMV_DIR_ADD=..\..\DAT\EMVINI\AU
IF /i "%ATM_SITE%" == "MX" SET EMV_DIR_ADD=..\..\DAT\EMVINI\MX

REM SET SPR_DIR=..\..\DAT\RECEIPT
REM SET RAM_DIR=..\..\DAT\NVRAM

REM EMV_ON is enabled on HEADER
REM IF /i "%ATM_SITE%" == "CA" SET EMV_ON=OFF
REM IF /i "%ATM_SITE%" == "MX" SET EMV_ON=OFF
REM IF /i "%ATM_SITE%" == "US" SET EMV_ON=OFF
REM IF /i "%ATM_SITE%" == "AU" SET EMV_ON=OFF

REM OP_LOOK is set on HEADER
REM IF /i "%ATM_SITE%" == "US" SET OP_LOOK=OP
REM IF /i "%ATM_SITE%" == "CA" SET OP_LOOK=OP
REM IF /i "%ATM_SITE%" == "AU" SET OP_LOOK=OP
REM IF /i "%ATM_SITE%" == "MX" SET OP_LOOK=OP

SET SCR_OP_DIR=..\..\DAT\SCREEN\%OP_LOOK%

ECHO #################################
ECHO         POST BUILD START
ECHO #################################



REM ECHO ----- Deleting (*.lib, *.exp)..
REM DEL %EXE_DIR%\*.lib /F /Q
REM DEL %EXE_DIR%\*.exp /F /Q



REM ECHO ----- Copying WAVE Files...
REM XCOPY /s %WAV_DEF_DIR%\*	%EXE_DIR%\WAVE\ /y /q



REM ECHO ----- Copying TTS DB Files...
REM IF /i "%ATM_SITE%" == "US" XCOPY /s %TTS_DB_DIR%\*	%EXE_DIR%\TTS_DB\ /y /q



ECHO ----- Copying EXTRA DLLS
if EXIST "%EXE_DIR_UPPER%\ATM2\NVRAM.DAT" (
	echo "Backing up existing NVRAM.DAT"
	copy "%EXE_DIR_UPPER%\ATM2\NVRAM.DAT" "%EXE_DIR_UPPER%\ATM2\NVRAM.DAT.BAK" /y
)

XCOPY /s %EXTRA_DLL_DIR%\*	%EXE_DIR_UPPER%\ /y /q

if EXIST "%EXE_DIR_UPPER%\ATM2\NVRAM.DAT.BAK" (
	echo "Restoring NVRAM.DAT"
	copy "%EXE_DIR_UPPER%\ATM2\NVRAM.DAT.BAK" "%EXE_DIR_UPPER%\ATM2\NVRAM.DAT" /y
	echo "Deleting NVRAM.DAT backup"
	del "%EXE_DIR_UPPER%\ATM2\NVRAM.DAT.BAK"
)

ECHO ----- Copying ErrorCode Files...
REM XCOPY /s %ERR_DIR%\* 	           %EXE_DIR%\ERRORCODE\ /y /q
IF /i "%ATM_SITE%" == "AU" XCOPY /s %ERR_DIR%\ErrorCode_ENG.dat 	%EXE_DIR%\ERRORCODE\ /y /q
IF /i "%ATM_SITE%" == "US" XCOPY /s %ERR_DIR%\* 	           	%EXE_DIR%\ERRORCODE\ /y /q
IF /i "%ATM_SITE%" == "CA" XCOPY /s %ERR_DIR%\* 	           	%EXE_DIR%\ERRORCODE\ /y /q
IF /i "%ATM_SITE%" == "MX" XCOPY /s %ERR_DIR%\* 	           	%EXE_DIR%\ERRORCODE\ /y /q



ECHO ----- Copying EMV Files...
XCOPY  %EMV_DIR%\*.ini  %EXE_DIR%\EMVINI\ /y /q
XCOPY  %EMV_DIR_ADD%\*.ini  %EXE_DIR%\EMVINI\ /y /q


ECHO ----- Copying INI Files...
XCOPY  %INI_DIR%\*.*  %EXE_DIR%\ /y /q


ECHO ----- Copying Advertisement files..
XCOPY /s %ADV_DIR%\%ATM_SITE%\*    %EXE_DIR%\ADVERTISEMENT\ /y /q


ECHO - Copying Screen Image Files...
IF /i "%ATM_SITE%" == "US"	XCOPY /s %SCR_DIR%\1800SE\%ATM_SITE%\800_600\*    %EXE_DIR%\SCREEN\800_600\ /y /q
IF /i "%ATM_SITE%" == "CA"	XCOPY /s %SCR_DIR%\1800SE\%ATM_SITE%\800_600\*    %EXE_DIR%\SCREEN\800_600\ /y /q
IF /i "%ATM_SITE%" == "US"	XCOPY /s %SCR_DIR%\1500\%ATM_SITE%\640_480\*    %EXE_DIR%\SCREEN\640_480\ /y /q
IF /i "%ATM_SITE%" == "CA"	XCOPY /s %SCR_DIR%\1500\%ATM_SITE%\640_480\*    %EXE_DIR%\SCREEN\640_480\ /y /q

IF /i "%ATM_SITE%" == "US"	XCOPY /s %SCR_DIR%\2700\%ATM_SITE%\720_480\*    %EXE_DIR%\SCREEN\720_480\ /y /q
IF /i "%ATM_SITE%" == "CA"	XCOPY /s %SCR_DIR%\2700\%ATM_SITE%\720_480\*    %EXE_DIR%\SCREEN\720_480\ /y /q
IF /i "%ATM_SITE%" == "AU"	XCOPY /s %SCR_DIR%\2700\%ATM_SITE%\720_480\*    %EXE_DIR%\SCREEN\720_480\ /y /q
IF /i "%ATM_SITE%" == "MX"	XCOPY /s %SCR_DIR%\2700\%ATM_SITE%\720_480\*    %EXE_DIR%\SCREEN\720_480\ /y /q

XCOPY /s %SCR_DIR%\2700\%ATM_SITE%\1024_600\*    %EXE_DIR%\SCREEN\1024_600\ /y /q
XCOPY /s %SCR_DIR%\5200\%ATM_SITE%\1024_768\*    %EXE_DIR%\SCREEN\1024_768\ /y /q
XCOPY /s %SCR_DIR%\2800\%ATM_SITE%\1024_768\*    %EXE_DIR%\SCREEN\1024_768\ /y /q

IF /i "%ATM_SITE%" == "US"	XCOPY /s %BACK_SCR_DIR%\*    %EXE_DIR%\SCREEN\Backs\ /y /q
IF /i "%ATM_SITE%" == "CA"	XCOPY /s %BACK_SCR_DIR%\*    %EXE_DIR%\SCREEN\Backs\ /y /q
IF /i "%ATM_SITE%" == "AU"	XCOPY /s %BACK_SCR_DIR%\1024_600\*    %EXE_DIR%\SCREEN\Backs\1024_600\ /y /q
IF /i "%ATM_SITE%" == "AU"	XCOPY /s %BACK_2800_SCR_DIR%\1024_768\*    %EXE_DIR%\SCREEN\Backs\1024_768\ /y /q
IF /i "%ATM_SITE%" == "MX"	XCOPY /s %BACK_SCR_DIR%\1024_600\*    %EXE_DIR%\SCREEN\Backs\1024_600\ /y /q
IF /i "%ATM_SITE%" == "MX"	XCOPY /s %BACK_SCR_DIR%\1024_768\*    %EXE_DIR%\SCREEN\Backs\1024_768\ /y /q

ECHO - Copying Screen Dat Files...
IF /i "%ATM_SITE%" == "US"	XCOPY %SCR_DIR%\1800SE\%ATM_SITE%\*.dat	%EXE_DIR%\SCREEN\1800SE\ /y /q
IF /i "%ATM_SITE%" == "US"	XCOPY %SCR_DIR%\1800SE\*.dat		%EXE_DIR%\SCREEN\1800SE\ /y /q
IF /i "%ATM_SITE%" == "CA"	XCOPY %SCR_DIR%\1800SE\%ATM_SITE%\*.dat	%EXE_DIR%\SCREEN\1800SE\ /y /q
IF /i "%ATM_SITE%" == "CA"	XCOPY %SCR_DIR%\1800SE\*.dat		%EXE_DIR%\SCREEN\1800SE\ /y /q

IF /i "%ATM_SITE%" == "US"	XCOPY %SCR_DIR%\1500\%ATM_SITE%\*.dat	%EXE_DIR%\SCREEN\1500\ /y /q
IF /i "%ATM_SITE%" == "US"	XCOPY %SCR_DIR%\1500\*.dat		%EXE_DIR%\SCREEN\1500\ /y /q
IF /i "%ATM_SITE%" == "CA"	XCOPY %SCR_DIR%\1500\%ATM_SITE%\*.dat	%EXE_DIR%\SCREEN\1500\ /y /q
IF /i "%ATM_SITE%" == "CA"	XCOPY %SCR_DIR%\1500\*.dat		%EXE_DIR%\SCREEN\1500\ /y /q

XCOPY %SCR_DIR%\2700\%ATM_SITE%\*.dat		%EXE_DIR%\SCREEN\2700\ /y /q
XCOPY %SCR_DIR%\2700\*.dat					%EXE_DIR%\SCREEN\2700\ /y /q
XCOPY %SCR_DIR%\5200\%ATM_SITE%\*.dat		%EXE_DIR%\SCREEN\5200\ /y /q
XCOPY %SCR_DIR%\5200\*.dat					%EXE_DIR%\SCREEN\5200\ /y /q
XCOPY %SCR_DIR%\2800\%ATM_SITE%\*.dat		%EXE_DIR%\SCREEN\2800\ /y /q
XCOPY %SCR_DIR%\2800\*.dat					%EXE_DIR%\SCREEN\2800\ /y /q
XCOPY %SCR_DIR%\*.dat        		        %EXE_DIR%\SCREEN\ /y /q

:MAKE_SCR_CALL_END
ECHO #################################
ECHO          POST BUILD END
ECHO #################################
GOTO QUIT

REM //////////////////////////////////
REM           LABLE : ERROR
REM //////////////////////////////////
:ERROR
ECHO ERRROR : SITE 정의가 누락 되었습니다.

:QUIT
@ECHO ON