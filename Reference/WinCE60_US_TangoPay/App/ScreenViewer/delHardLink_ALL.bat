@echo off
rem 파라매터 처리 시 공백인식을 위해 %1대신 %~1을 사용한다
set SCR_ROOT=%~1

rd /s /q "%SCR_ROOT%\.restructured_US"
rd /s /q "%SCR_ROOT%\.restructured_CA"
rd /s /q "%SCR_ROOT%\.restructured_MX"
rd /s /q "%SCR_ROOT%\.restructured_AU"
