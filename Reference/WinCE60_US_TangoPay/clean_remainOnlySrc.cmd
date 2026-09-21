rem Delete C++ temporary file.
del /S *.ncb
del /S *.obj
del /S *.ilk
del /S *.pdb
del /S *.plg
del /S *.aps
del /S *.bsc
del /S *.tmp
del /S *.trc
del /S *.pch
del /S *.idb
del /S *.exp
del /S *.sbr
del /S *.res
del /S *.tlh
del /S *.tli
del /S *.vcl
del /S *.htm
del /S *.user
del /S *.ti
del /S *.cod

rem Exclude library file and project option file.
rem del /S *.lib
rmdir /S /Q .\LibRele_US
rmdir /S /Q .\ExeRele_US
rmdir /S /Q .\LibRele_50_US
rmdir /S /Q .\ExeRele_50_US
rmdir /S /Q .\LibRele_60_US
rmdir /S /Q .\ExeRele_60_US
rmdir /S /Q .\LibRele_UK
rmdir /S /Q .\ExeRele_UK
rmdir /S /Q .\LibRele_50_UK
rmdir /S /Q .\ExeRele_50_UK
rmdir /S /Q .\LibRele_60_UK
rmdir /S /Q .\ExeRele_60_UK
rmdir /S /Q .\LibRele_NZ
rmdir /S /Q .\ExeRele_NZ
rmdir /S /Q .\LibRele_50_NZ
rmdir /S /Q .\ExeRele_50_NZ
rmdir /S /Q .\LibRele_60_NZ
rmdir /S /Q .\ExeRele_60_NZ
rmdir /S /Q .\LibRele_MX
rmdir /S /Q .\ExeRele_MX
rmdir /S /Q .\LibRele_50_MX
rmdir /S /Q .\ExeRele_50_MX
rmdir /S /Q .\LibRele_60_MX
rmdir /S /Q .\ExeRele_60_MX
rmdir /S /Q .\LibRele_AU
rmdir /S /Q .\ExeRele_AU
rmdir /S /Q .\LibRele_50_AU_A
rmdir /S /Q .\ExeRele_50_AU_A
rmdir /S /Q .\LibRele_50_AU_C
rmdir /S /Q .\ExeRele_50_AU_C
rmdir /S /Q .\LibRele_60_AU
rmdir /S /Q .\ExeRele_60_AU
rmdir /S /Q .\LibRele_60_AU_A
rmdir /S /Q .\ExeRele_60_AU_A
rmdir /S /Q .\LibRele_60_AU_C
rmdir /S /Q .\ExeRele_60_AU_C
rmdir /S /Q .\LibRele_AU_ATMPlus
rmdir /S /Q .\ExeRele_AU_ATMPlus
rmdir /S /Q .\LibRele_50_AU_A_ATMPlus
rmdir /S /Q .\ExeRele_50_AU_A_ATMPlus
rmdir /S /Q .\LibRele_60_AU_A_ATMPlus
rmdir /S /Q .\ExeRele_60_AU_A_ATMPlus
rmdir /S /Q .\LibRele_CA
rmdir /S /Q .\ExeRele_CA
rmdir /S /Q .\LibRele_50_CA
rmdir /S /Q .\ExeRele_50_CA
rmdir /S /Q .\LibRele_60_CA
rmdir /S /Q .\ExeRele_60_CA
rmdir /S /Q .\ExeRele
rmdir /S /Q .\LibRele
rmdir /S /Q .\ExeRele_50
rmdir /S /Q .\LibRele_50
rmdir /S /Q .\ExeRele_60
rmdir /S /Q .\LibRele_60
rmdir /S /Q .\debug
rmdir /S /Q .\release
del /S *.vcb
rem del /S *.opt

rem Delete ActiveX temporary file.
del /S *.map
del /S *.oca
del /S *.trg
del /S *.tlb

rem Delete ATM Executable file.
rem del /S .\App\*.exe
