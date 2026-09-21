@echo off
rem �Ķ���� ó�� �� �����ν��� ���� %1��� %~1�� ����Ѵ�
set SCR_ROOT=%~1
set ATM_SITE=%2
set BACK_TYPE=%3

rem �Ķ���� ����ó��
if "%SCR_ROOT%" == "" goto ERROR_PARAM1_MISSING

if /i "%ATM_SITE%" == "US" goto BUILD
if /i "%ATM_SITE%" == "CA" goto BUILD
if /i "%ATM_SITE%" == "AU" goto BUILD
if /i "%ATM_SITE%" == "MX" goto BUILD
goto ERROR_PARAM2_MISSING

:BUILD
md "%SCR_ROOT%\.restructured_%ATM_SITE%"
md "%SCR_ROOT%\.restructured_%ATM_SITE%\backs"

rem if not exist "%SCR_ROOT%\init.dat" goto ERROR_IN_PROGRESS
rem fsutil hardlink create "%SCR_ROOT%\.restructured_%ATM_SITE%\init.dat" "%SCR_ROOT%\init.dat"
fsutil hardlink create "%SCR_ROOT%\.restructured_%ATM_SITE%\masterscreendesc.dat" "%SCR_ROOT%\masterscreendesc.dat"
fsutil hardlink create "%SCR_ROOT%\.restructured_%ATM_SITE%\aptext.dat" "%SCR_ROOT%\aptext.dat"
fsutil hardlink create "%SCR_ROOT%\.restructured_%ATM_SITE%\optext.dat" "%SCR_ROOT%\optext.dat"
fsutil hardlink create "%SCR_ROOT%\.restructured_%ATM_SITE%\multitext.dat" "%SCR_ROOT%\multitext.dat"
fsutil hardlink create "%SCR_ROOT%\.restructured_%ATM_SITE%\keymgrtext.dat" "%SCR_ROOT%\keymgrtext.dat"
fsutil hardlink create "%SCR_ROOT%\.restructured_%ATM_SITE%\multitextkeymgr.dat" "%SCR_ROOT%\multitextkeymgr.dat"
fsutil hardlink create "%SCR_ROOT%\.restructured_%ATM_SITE%\DigitalMint.dat" "%SCR_ROOT%\DigitalMint.dat"
fsutil hardlink create "%SCR_ROOT%\.restructured_%ATM_SITE%\JustCash.dat" "%SCR_ROOT%\JustCash.dat"
fsutil hardlink create "%SCR_ROOT%\.restructured_%ATM_SITE%\CashDepot.dat" "%SCR_ROOT%\CashDepot.dat"

:BUILD_1500
rem AU / MX�� 1500 ����
if /i "%ATM_SITE%" == "AU" goto END_OF_1500
if /i "%ATM_SITE%" == "MX" goto END_OF_1500

md "%SCR_ROOT%\.restructured_%ATM_SITE%\1500"
linkd "%SCR_ROOT%\.restructured_%ATM_SITE%\640_480" "%SCR_ROOT%\1500\%ATM_SITE%\640_480"
linkd "%SCR_ROOT%\.restructured_%ATM_SITE%\backs\640_480" "%SCR_ROOT%\backs\640_480"

fsutil hardlink create "%SCR_ROOT%\.restructured_%ATM_SITE%\1500\ap.dat" "%SCR_ROOT%\1500\%ATM_SITE%\ap.dat"
fsutil hardlink create "%SCR_ROOT%\.restructured_%ATM_SITE%\1500\keymgr.dat" "%SCR_ROOT%\1500\keymgr.dat"
fsutil hardlink create "%SCR_ROOT%\.restructured_%ATM_SITE%\1500\op.dat" "%SCR_ROOT%\1500\op.dat"
:END_OF_1500

:BUILD_1800SE
rem AU / MX�� 1800SE ����
if /i "%ATM_SITE%" == "AU" goto END_OF_1800SE
if /i "%ATM_SITE%" == "MX" goto END_OF_1800SE

md "%SCR_ROOT%\.restructured_%ATM_SITE%\1800se"
linkd "%SCR_ROOT%\.restructured_%ATM_SITE%\800_600" "%SCR_ROOT%\1800SE\%ATM_SITE%\800_600"
linkd "%SCR_ROOT%\.restructured_%ATM_SITE%\backs\800_600" "%SCR_ROOT%\backs\800_600"

fsutil hardlink create "%SCR_ROOT%\.restructured_%ATM_SITE%\1800se\ap.dat" "%SCR_ROOT%\1800se\%ATM_SITE%\ap.dat"
fsutil hardlink create "%SCR_ROOT%\.restructured_%ATM_SITE%\1800se\keymgr.dat" "%SCR_ROOT%\1800se\keymgr.dat"
fsutil hardlink create "%SCR_ROOT%\.restructured_%ATM_SITE%\1800se\op.dat" "%SCR_ROOT%\1800se\op.dat"
fsutil hardlink create "%SCR_ROOT%\.restructured_%ATM_SITE%\1800se\CashDepotOp.dat" "%SCR_ROOT%\1800se\CashDepotOp.dat"
fsutil hardlink create "%SCR_ROOT%\.restructured_%ATM_SITE%\1800se\DigitalMintOp.dat" "%SCR_ROOT%\1800se\DigitalMintOp.dat"
fsutil hardlink create "%SCR_ROOT%\.restructured_%ATM_SITE%\1800se\JustCashOp.dat" "%SCR_ROOT%\1800se\JustCashOp.dat"
:END_OF_1800SE

:BUILD_2700
md "%SCR_ROOT%\.restructured_%ATM_SITE%\2700"
linkd "%SCR_ROOT%\.restructured_%ATM_SITE%\1024_600" "%SCR_ROOT%\2700\%ATM_SITE%\1024_600"
linkd "%SCR_ROOT%\.restructured_%ATM_SITE%\backs\1024_600" "%SCR_ROOT%\backs\1024_600"

fsutil hardlink create "%SCR_ROOT%\.restructured_%ATM_SITE%\2700\ap.dat" "%SCR_ROOT%\2700\%ATM_SITE%\ap.dat"
fsutil hardlink create "%SCR_ROOT%\.restructured_%ATM_SITE%\2700\keymgr.dat" "%SCR_ROOT%\2700\keymgr.dat"
fsutil hardlink create "%SCR_ROOT%\.restructured_%ATM_SITE%\2700\op.dat" "%SCR_ROOT%\2700\op.dat"
fsutil hardlink create "%SCR_ROOT%\.restructured_%ATM_SITE%\2700\CashDepotOp.dat" "%SCR_ROOT%\2700\CashDepotOp.dat"
fsutil hardlink create "%SCR_ROOT%\.restructured_%ATM_SITE%\2700\DigitalMintOp.dat" "%SCR_ROOT%\2700\DigitalMintOp.dat"
fsutil hardlink create "%SCR_ROOT%\.restructured_%ATM_SITE%\2700\JustCashOp.dat" "%SCR_ROOT%\2700\JustCashOp.dat"
:END_OF_2700

:BUILD_5200
rem 5200�� ���� �̹���(1024x600)�� ��� �̹���(1024x768)�� ������ �����
md "%SCR_ROOT%\.restructured_%ATM_SITE%\5200"
linkd "%SCR_ROOT%\.restructured_%ATM_SITE%\1024_600" "%SCR_ROOT%\2700\%ATM_SITE%\1024_600"
linkd "%SCR_ROOT%\.restructured_%ATM_SITE%\1024_768" "%SCR_ROOT%\5200\%ATM_SITE%\1024_768"
rem 5200�� ���� 1024x600 �̹��� + �߰� 1024x768 �̹����� ����ϹǷ�, linkd ��� ���� ������ ������
rem md "%SCR_ROOT%\.restructured_%ATM_SITE%\5200\1024_600"
rem xcopy "%SCR_ROOT%\2700\%ATM_SITE%\1024_600" "%SCR_ROOT%\.restructured_%ATM_SITE%\1024_600" /s /e /q 
rem xcopy "%SCR_ROOT%\5200\%ATM_SITE%\1024_768" "%SCR_ROOT%\.restructured_%ATM_SITE%\1024_600" /s /e /q 
linkd "%SCR_ROOT%\.restructured_%ATM_SITE%\backs\1024_768" "%SCR_ROOT%\backs\1024_768"

fsutil hardlink create "%SCR_ROOT%\.restructured_%ATM_SITE%\5200\ap.dat" "%SCR_ROOT%\5200\%ATM_SITE%\ap.dat"
fsutil hardlink create "%SCR_ROOT%\.restructured_%ATM_SITE%\5200\keymgr.dat" "%SCR_ROOT%\5200\keymgr.dat"
fsutil hardlink create "%SCR_ROOT%\.restructured_%ATM_SITE%\5200\op.dat" "%SCR_ROOT%\5200\op.dat"
fsutil hardlink create "%SCR_ROOT%\.restructured_%ATM_SITE%\5200\CashDepotOp.dat" "%SCR_ROOT%\5200\CashDepotOp.dat"
fsutil hardlink create "%SCR_ROOT%\.restructured_%ATM_SITE%\5200\DigitalMintOp.dat" "%SCR_ROOT%\5200\DigitalMintOp.dat"
fsutil hardlink create "%SCR_ROOT%\.restructured_%ATM_SITE%\5200\JustCashOp.dat" "%SCR_ROOT%\5200\JustCashOp.dat"
:END_OF_5200

:BUILD_2700T
md "%SCR_ROOT%\.restructured_%ATM_SITE%\2700"
linkd "%SCR_ROOT%\.restructured_%ATM_SITE%\720_480" "%SCR_ROOT%\2700\%ATM_SITE%\720_480"
:END_OF_2700T

:BUILD_2800
if exist "%SCR_ROOT%\2800" (
    if "%BACK_TYPE%" == "2800" md "%SCR_ROOT%\.restructured_%ATM_SITE%\2800"
    if "%BACK_TYPE%" == "2800" linkd "%SCR_ROOT%\.restructured_%ATM_SITE%\1024_600" "%SCR_ROOT%\2700\%ATM_SITE%\1024_600"
    if "%BACK_TYPE%" == "2800" linkd "%SCR_ROOT%\.restructured_%ATM_SITE%\1024_768" "%SCR_ROOT%\2800\%ATM_SITE%\1024_768"
    if "%BACK_TYPE%" == "2800" linkd "%SCR_ROOT%\.restructured_%ATM_SITE%\backs\1024_768" "%SCR_ROOT%\backs_2800\1024_768"

    if "%BACK_TYPE%" == "2800" fsutil hardlink create "%SCR_ROOT%\.restructured_%ATM_SITE%\2800\ap.dat" "%SCR_ROOT%\2800\%ATM_SITE%\ap.dat"
    if "%BACK_TYPE%" == "2800" fsutil hardlink create "%SCR_ROOT%\.restructured_%ATM_SITE%\2800\keymgr.dat" "%SCR_ROOT%\5200\keymgr.dat"
    if "%BACK_TYPE%" == "2800" fsutil hardlink create "%SCR_ROOT%\.restructured_%ATM_SITE%\2800\op.dat" "%SCR_ROOT%\5200\op.dat"
    if "%BACK_TYPE%" == "2800" fsutil hardlink create "%SCR_ROOT%\.restructured_%ATM_SITE%\2800\pai.dat" "%SCR_ROOT%\2800\pai.dat"
    if "%BACK_TYPE%" == "2800" fsutil hardlink create "%SCR_ROOT%\.restructured_%ATM_SITE%\2800\CashDepotOp.dat" "%SCR_ROOT%\2800\CashDepotOp.dat"
    if "%BACK_TYPE%" == "2800" fsutil hardlink create "%SCR_ROOT%\.restructured_%ATM_SITE%\2800\DigitalMintOp.dat" "%SCR_ROOT%\2800\DigitalMintOp.dat"
    if "%BACK_TYPE%" == "2800" fsutil hardlink create "%SCR_ROOT%\.restructured_%ATM_SITE%\2800\JustCashOp.dat" "%SCR_ROOT%\2800\JustCashOp.dat"

    if /i "%ATM_SITE%" == "AU" (
        fsutil hardlink create "%SCR_ROOT%\.restructured_%ATM_SITE%\2800\keymgr.dat" "%SCR_ROOT%\2800\keymgr.dat"
        fsutil hardlink create "%SCR_ROOT%\.restructured_%ATM_SITE%\2800\op.dat" "%SCR_ROOT%\2800\op.dat"
    )
) ELSE (
    @echo 2800 not found!
)
:END_OF_2800

rem DAT integration
fsutil hardlink create "%SCR_ROOT%\.restructured_%ATM_SITE%\op.dat" "%SCR_ROOT%\op.dat"

goto QUIT

:ERROR_PARAM1_MISSING
@echo SCR_ROOT is missing.
goto QUIT

:ERROR_PARAM2_MISSING
@echo ATM_SITE is missing or incorrect.
goto QUIT

:ERROR_IN_PROGRESS
@echo an error occured while building.
cmd /c delHardLink_ALL.bat
goto QUIT

:QUIT
@echo on
rem pause