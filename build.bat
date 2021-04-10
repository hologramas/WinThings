@echo off

IF NOT EXIST bin mkdir bin
pushd bin

echo ====
echo Building memnotify.
cl ..\memnotify.cc /Fememnotify.exe /Fdmemnotify.pdb /DDBUG /Zi /link /SUBSYSTEM:console -opt:ref
IF "%errorlevel%" NEQ "0" goto :exit

echo ====
echo Building squat.
cl ..\squat.cc /Fesquat.exe /Fdsquat.pdb /DDBUG /Zi /link /SUBSYSTEM:console -opt:ref
IF "%errorlevel%" NEQ "0" goto :exit

echo ====
echo Building usecpu.
cl ..\usecpu.cc /Feusecpu.exe /Fdusecpu.pdb /DDBUG /Zi /link /SUBSYSTEM:console -opt:ref
IF "%errorlevel%" NEQ "0" goto :exit

echo ====
echo Building disktype.
cl ..\disktype.cc /Fedisktype.exe /Fddisktype.pdb /DDBUG /Zi /link /SUBSYSTEM:console -opt:ref
IF "%errorlevel%" NEQ "0" goto :exit

:exit
popd
