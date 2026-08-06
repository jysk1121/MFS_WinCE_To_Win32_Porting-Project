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

rem delete lib file in executable folder
rd /s /q .\EagleCE_ATM_EXE
rd /s /q .\SoftwarePackage
rd /s /q .\MainBD_TP_EXE

del /s /q .\DLL_SRC\DLL_CE\*.*


rem project option file.
del /S *.vcb
del /S *.opt
del /S /A *.suo


rem Delete ActiveX temporary file.
del /S *.map
del /S *.oca
del /S *.trg
del /S *.tlb
