@echo off
rem
rem Copy source files to ofSpoutExample "src" folder
rem
setlocal
:Start
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
echo [8] Receiver Sync           Synchronise Sender and Receiver
echo [9] Receiver Data           Exchange data
echo [0] Receiver Multiple       Multiple receivers in one application
echo.
echo Rebuild "ofSpoutExample" with the new source files
echo Rescan can be used to refresh Intellisense
echo The executable file is copied to the Binaries folder
echo.
set input=
set /P input=Enter option (none to quit) :
if "%input%" equ "" goto End
echo [%input%]
rem
echo.
if %input% EQU 1 (
	copy /y /v Source\Sender\SenderGraphics\ofApp.* src
	copy /y /v Source\Sender\SenderGraphics\*.bat
)
if %input% EQU 2 (
	copy /y /v Source\Sender\SenderSync\ofApp.* src
	copy /y /v Source\Sender\SenderSync\*.bat
)
if %input% EQU 3 (
	copy /y /v Source\Sender\SenderData\ofApp.* src
	copy /y /v Source\Sender\SenderData\*.bat
)
if %input% EQU 4 (
	copy /y /v Source\Sender\SenderVideo\ofApp.* src
	copy /y /v Source\Sender\SenderVideo\*.bat
)
if %input% EQU 5 (
	copy /y /v Source\Sender\SenderWebcam\ofApp.* src
	copy /y /v Source\Sender\SenderWebcam\*.bat
)
if %input% EQU 6 (
	copy /y /v Source\Sender\SenderUtilities\ofApp.* src
	copy /y /v Source\Sender\SenderUtilities\*.bat
)
if %input% EQU 7 (
	copy /y /v Source\Receiver\ReceiverGraphics\ofApp.* src
	copy /y /v Source\Receiver\ReceiverGraphics\*.bat
)
if %input% EQU 8 (
	copy /y /v Source\Receiver\ReceiverSync\ofApp.* src
	copy /y /v Source\Receiver\ReceiverSync\*.bat
)
if %input% EQU 9 (
	copy /y /v Source\Receiver\ReceiverData\ofApp.* src
	copy /y /v Source\Receiver\ReceiverData\*.bat
)
if %input% EQU 0 (
	copy /y /v Source\Receiver\ReceiverMultiple\ofApp.* src
	copy /y /v Source\Receiver\ReceiverMultiple\*.bat
)
rem
rem Allow for repeats without re-starting CopySource
rem
goto Start
rem
:End
set input=
endlocal
rem







