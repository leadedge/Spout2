@ECHO OFF
rem
rem Register Spout settings with user selection of values
rem
mode con cols=95 lines=18
cls
rem
title Spout Settings
rem
Pushd "%~dp0"
rem add double backslashes to %~dp0 for .reg file requirement
rem https://www.tenforums.com/software-apps/176293-bat-output.html#post2180423
set dp=%~dp0
if "!dp:~-1!"=="\" set dp=!dp:~,-1!
set dp=%dp:\=\\%
rem
rem
rem Retrieve settings from the registry
rem
set auto=1
set cpu=0
set framecount=1
set buffering=1
set buffers=2
set maxsenders=0x40
rem Version
rem The number must be hex e.g. 2007015 = 0x1E9FE7 
set version=0x1E9FE7
rem
rem Write default settings if the Spout registry key does not exist
rem 
reg query "HKCU\SOFTWARE\Leading Edge\Spout" >nul 2>nul
if %errorlevel% neq 0 (
	rem write defaults
	REG ADD "HKCU\Software\Leading Edge\Spout" /t REG_DWORD /d 1 /v Auto /f >nul 2>nul
	REG ADD "HKCU\Software\Leading Edge\Spout" /t REG_DWORD /d 0 /v CPU /f >nul 2>nul
	REG ADD "HKCU\Software\Leading Edge\Spout" /t REG_DWORD /d 1 /v Framecount /f >nul 2>nul
	REG ADD "HKCU\Software\Leading Edge\Spout" /t REG_DWORD /d 1 /v Buffering /f >nul 2>nul
	REG ADD "HKCU\Software\Leading Edge\Spout" /t REG_DWORD /d 2 /v Buffers /f >nul 2>nul
	REG ADD "HKCU\Software\Leading Edge\Spout" /t REG_DWORD /d 0x40 /v MaxSenders /f >nul 2>nul
	REG ADD "HKCU\Software\Leading Edge\Spout" /t REG_DWORD /d 0x1E9FE7 /v Version /f >nul 2>nul
)
rem
rem Get existing values from the registry into variables
rem All are hex values
rem
setlocal enabledelayedexpansion
FOR /F "skip=2 tokens=2,*" %%A IN ('reg.exe query "HKCU\SOFTWARE\Leading Edge\Spout" /v Auto') DO set "auto=%%B"
FOR /F "skip=2 tokens=2,*" %%A IN ('reg.exe query "HKCU\SOFTWARE\Leading Edge\Spout" /v CPU') DO set "cpu=%%B"
FOR /F "skip=2 tokens=2,*" %%A IN ('reg.exe query "HKCU\SOFTWARE\Leading Edge\Spout" /v Framecount') DO set "framecount=%%B"
FOR /F "skip=2 tokens=2,*" %%A IN ('reg.exe query "HKCU\SOFTWARE\Leading Edge\Spout" /v Buffering') DO set "buffering=%%B"
FOR /F "skip=2 tokens=2,*" %%A IN ('reg.exe query "HKCU\SOFTWARE\Leading Edge\Spout" /v Buffers') DO set "buffers=%%B"
FOR /F "skip=2 tokens=2,*" %%A IN ('reg.exe query "HKCU\SOFTWARE\Leading Edge\Spout" /v Maxsenders') DO set "maxsenders=%%B"
FOR /F "skip=2 tokens=2,*" %%A IN ('reg.exe query "HKCU\SOFTWARE\Leading Edge\Spout" /v Version') DO set "version=%%B"
rem
ECHO.
ECHO                    -------------------- SPOUT SETTINGS --------------------
ECHO.
echo 1) Update          Review and change Spout settings
echo 2) Default         Reset all settings to default
echo 3) Remove          Delete Spout registry settings if no longer required
rem
echo.
SET INPUT=
SET /P INPUT=Select option (Enter to quit) :
IF "%INPUT%" equ "" GOTO End
rem
IF "%INPUT%" equ "3" (
	cls
	echo.
	echo                     ------------------- SPOUT REMOVAL ---------------------	
	echo.
	rem REG DELETE with prompt for removal
	REG DELETE "HKCU\Software\Leading Edge\Spout"
	rem Unregister SpoutPanel without prompt
	rem It is registered again when next opened
	REG DELETE "HKCU\Software\Leading Edge\SpoutPanel" /f >nul 2>nul
	rem Remove SpoutCam settings
	rem They can be restored by "SpoutCamSettings"
	rem SpoutCam is a DirectSHow filter and must be un-registered separately
	REG DELETE "HKCU\Software\Leading Edge\SpoutCam" /f >nul 2>nul
	echo.
	goto End
)
rem
rem Set defaults option
rem
IF "%INPUT%" equ "2" (
echo defaults
    rem Set hex as retrieved from the registry
	set auto=0x1
	set cpu=0x0
	set framecount=0x1
	set buffering=0x1
	set buffers=0x2
	set maxsenders=0x40
)
rem
rem New SpoutSettings path
set fpath=%dp%\\SpoutSettings.exe
rem
cls
ECHO.
ECHO                     -------------------- SPOUT SETTINGS --------------------
ECHO.
echo Current settings 
echo.
if "%auto%" == "0x0" echo Auto share  (Texture share by compatibility) 0 (disabled)
if "%auto%" == "0x1" echo Auto share  (Texture share by compatibility) 1 (enabled)
if "%cpu%" == "0x0" echo CPU share   (Force CPU share)                0 (disabled)
if "%cpu%" == "0x1" echo CPU share   (Force CPU share)                1 (enabled)
if "%framecount%" == "0x0" echo Frame count (Sender frame counter)           0 (disabled)
if "%framecount%" == "0x1" echo Frame count (Sender frame counter)           1 (enabled)
if "%buffering%" == "0x0" echo Buffering   (OpenGL pixel buffering)         0 (disabled)
if "%buffering%" == "0x1" echo Buffering   (OpenGL pixel buffering)         1 (enabled)
rem
rem Hex to decimal for display
rem
SET /A X = %buffers% + 0
echo Number of buffers                            %X% (2-4)
SET /A X = %maxsenders% + 0
echo Maximum number of senders                    %X% (10-255)
rem
echo.
SET INPUT=
SET /P INPUT=Do you want to change any settings? 
IF "%INPUT%" == "" GOTO End
IF "%INPUT%" == "n" GOTO End
IF "%INPUT%" == "N" GOTO End
rem
rem Hex to decimal for display
rem
SET /A X = %auto% + 0
SET /P auto=Auto share   %X% (0 - disable, 1 enable,  enter to keep): 
SET /A X = %cpu% + 0
SET /P cpu=Cpu share    %X% (1 - enable,  0 disable, enter to keep): 
SET /A X = %framecount% + 0
SET /P framecount=Frame count  %X% (0 - disable, 1 enable,  enter to keep): 
SET /A X = %buffering% + 0
SET /P buffering=Buffering    %X% (0 - disable, 1 enable,  enter to keep): 
SET /A X = %buffers% + 0
SET /P buffers=Number of buffers            %X%  (2-4,    none to keep): 
IF "%buffers%" == "" goto MAXSENDERS
IF %buffers% LSS 2 SET /A buffers=2
IF %buffers% GTR 4 SET /A buffers=4
:MAXSENDERS
SET /A X = %maxsenders% + 0
SET /P maxsenders=Maximum number of senders    %X% (10-255, none to keep): 
IF %maxsenders% LSS 10 SET /A maxsenders=10
IF %maxsenders% GTR 255 SET /A maxsenders=255
call :ConvertDecToHex %maxsenders% maxsenders
rem
rem Prompt for registry write
rem
:Reg
echo.
SET INPUT=
SET /P INPUT=Write settings to the registry? 
IF "%INPUT%" == "" GOTO End
IF "%INPUT%" == "n" GOTO End
IF "%INPUT%" == "N" GOTO End
rem
rem Create the registry file
rem
echo Windows Registry Editor Version 5.00 > spoutsettings.reg
echo. >> spoutsettings.reg
echo [HKEY_CURRENT_USER\SOFTWARE\Leading Edge\Spout] >> spoutsettings.reg
echo "Auto"=dword:%auto% >> spoutsettings.reg
echo "Cpu"=dword:%cpu% >> spoutsettings.reg
echo "Framecount"=dword:%framecount% >> spoutsettings.reg
echo "Buffering"=dword:%buffering% >> spoutsettings.reg
echo "Buffers"=dword:%buffers% >> spoutsettings.reg
echo "Maxsenders"=dword:%maxsenders% >> spoutsettings.reg
echo "Version"=dword:%version% >> spoutsettings.reg
echo "SpoutSettings"="%fpath%" >> spoutsettings.reg
rem
rem Run the registry file with user warning
rem
"%~dp0\spoutsettings.reg"
rem
rem delete the reg file
rem
del /q "%~dp0\spoutsettings.reg"
rem
:End
echo.
echo.
rem Clear environment variables
set fpath=
set spath=
set sname=
set auto=
set cpu=
set framecount=
set buffering=
set buffers=
set maxsenders=
set version=
rem
echo Done
SET /P INPUT=
Goto:eof
rem
rem https://gist.github.com/ijprest/1207832
rem
rem A function to convert Decimal to Hexadecimal
rem you need to pass the Decimal as first parameter
rem and return it in the second
rem This function needs setlocal EnableDelayedExpansion to be set at the start if this batch file
rem
:ConvertDecToHex
set LOOKUP=0123456789abcdef
set HEXSTR=
set PREFIX=
if "%1" EQU "" (
  set "%2" = "0"
  Goto:eof
)
set /a A=%1 || exit /b 1
if !A! LSS 0 set /a A=0xfffffff + !A! + 1 & set PREFIX=f
:loop
set /a B=!A! %% 16 & set /a A=!A! / 16
set HEXSTR=!LOOKUP:~%B%,1!%HEXSTR%
if %A% GTR 0 Goto :loop
set "%2=%PREFIX%%HEXSTR%"
Goto:eof
rem
rem End of :ConvertDecToHex function
rem
:eof
set INPUT=
set LOOKUP=
set HEXSTR=
set PREFIX=
set X=
rem

