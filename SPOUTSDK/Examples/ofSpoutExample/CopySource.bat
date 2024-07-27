@echo off
rem
rem Copy source files to ofSpoutExample "src" folder
rem
cls
echo.
echo Copy source files to the example 'src' folder
rem
echo.
echo [1] Sender Graphics         Basic graphics sender
echo [2] Sender Sync             Synchronise Sender and Receiver
echo [3] Sender Data             Exchange data
echo [4] Sender Video            Basic video sender
echo [5] Sender Webcam           Webcam sender
echo [6] Sender Utilities        Spout utilities example
echo [7] Receiver Graphics       Basic graphics receiver
echo [8] Receiver sync           Synchronise Sender and Receiver
echo [9] Receiver Data           Exchange data
echo [0] Receiver Multiple       Multiple receivers in one application
echo.
set /P input=Enter option to copy source files :
if "%input%" == "" goto End
rem
echo.
if %input% EQU 1 copy /y /v Source\Sender\Graphics\*.* src
if %input% EQU 2 copy /y /v Source\Sender\Sync\*.* src
if %input% EQU 3 copy /y /v Source\Sender\Data\*.* src
if %input% EQU 4 copy /y /v Source\Sender\Video\*.* src
if %input% EQU 5 copy /y /v Source\Sender\Webcam\*.* src
if %input% EQU 6 copy /y /v Source\Sender\Utilities\*.* src
if %input% EQU 7 copy /y /v Source\Receiver\Graphics\*.* src
if %input% EQU 8 copy /y /v Source\Receiver\Sync\*.* src
if %input% EQU 9 copy /y /v Source\Receiver\Data\*.* src
if %input% EQU 0 copy /y /v Source\Receiver\Multiple\*.* src
rem
echo.
echo Rebuild "ofSpoutExample" with the new source files
echo Rescan can be used to refresh Intellisense
echo.
set /p input=Enter to end
set input=
rem
:End
rem







