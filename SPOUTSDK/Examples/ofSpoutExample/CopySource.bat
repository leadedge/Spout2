@echo off
rem
rem Copy source files to ofSpoutExample "src" folder
rem
cls
echo.
echo Copy source files to example 'src' folder
rem
echo.
echo [1] Sender Data
echo [2] Sender Graphics
echo [3] Sender Sync
echo [4] Sender Utilities
echo [5] Sender Video
echo [6] Sender Webcam
echo [7] Receiver Data
echo [8] Receiver Graphics
echo [9] Receiver Multiple
echo [0] Receiver sync
set /P input=:
if "%input%" == "" goto End
rem
if %input% EQU 1 copy /y /v Source\Sender\Data\*.* src
if %input% EQU 2 copy /y /v Source\Sender\Graphics\*.* src
if %input% EQU 3 copy /y /v Source\Sender\Sync\*.* src
if %input% EQU 4 copy /y /v Source\Sender\Utilities\*.* src
if %input% EQU 5 copy /y /v Source\Sender\Video\*.* src
if %input% EQU 6 copy /y /v Source\Sender\Webcam\*.* src
if %input% EQU 7 copy /y /v Source\Receiver\Data\*.* src
if %input% EQU 8 copy /y /v Source\Receiver\Graphics\*.* src
if %input% EQU 9 copy /y /v Source\Receiver\Multiple\*.* src
if %input% EQU 0 copy /y /v Source\Receiver\Sync\*.* src
rem
echo.
set /p input=Enter to quit
set input=
rem
:End
rem







