@echo off

xcopy makeHardLink.bat ..\..\ExeRele_Win32_US\ /q/y
xcopy delHardLink_ALL.bat ..\..\ExeRele_Win32_US\ /q/y
xcopy linkd.exe ..\..\ExeRele_Win32_US\ /q/y

