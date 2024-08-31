@echo off
rem
rem Use MSbuild to build all example projects
rem
copy /y /v Source\Receiver\ReceiverGraphics\ofApp.* src
copy /y /v Source\Receiver\ReceiverGraphics\*.bat
rmdir /s /q ofSpoutExample
msbuild.exe ofSpoutExample.sln -v:m -p:Configuration=RELEASE -p:Platform=x64
rem
copy /y /v Source\Receiver\ReceiverSync\ofApp.* src
copy /y /v Source\Receiver\ReceiverSync\*.bat
rmdir /s /q ofSpoutExample
msbuild.exe ofSpoutExample.sln -v:m -p:Configuration=RELEASE -p:Platform=x64
rem
copy /y /v Source\Receiver\ReceiverData\ofApp.* src
copy /y /v Source\Receiver\ReceiverData\*.bat
rmdir /s /q ofSpoutExample
msbuild.exe ofSpoutExample.sln -v:m -p:Configuration=RELEASE -p:Platform=x64
rem
copy /y /v Source\Receiver\ReceiverMultiple\ofApp.* src
copy /y /v Source\Receiver\ReceiverMultiple\*.bat
rmdir /s /q ofSpoutExample
msbuild.exe ofSpoutExample.sln -v:m -p:Configuration=RELEASE -p:Platform=x64
rem
copy /y /v Source\Sender\SenderSync\ofApp.* src
copy /y /v Source\Sender\SenderSync\*.bat
rmdir /s /q ofSpoutExample
msbuild.exe ofSpoutExample.sln -v:m -p:Configuration=RELEASE -p:Platform=x64
rem
copy /y /v Source\Sender\SenderData\ofApp.* src
copy /y /v Source\Sender\SenderData\*.bat
rmdir /s /q ofSpoutExample
msbuild.exe ofSpoutExample.sln -v:m -p:Configuration=RELEASE -p:Platform=x64
rem
copy /y /v Source\Sender\SenderVideo\ofApp.* src
copy /y /v Source\Sender\SenderVideo\*.bat
rmdir /s /q ofSpoutExample
msbuild.exe ofSpoutExample.sln -v:m -p:Configuration=RELEASE -p:Platform=x64
rem
copy /y /v Source\Sender\SenderWebcam\ofApp.* src
copy /y /v Source\Sender\SenderWebcam\*.bat
rmdir /s /q ofSpoutExample
msbuild.exe ofSpoutExample.sln -v:m -p:Configuration=RELEASE -p:Platform=x64
rem
copy /y /v Source\Sender\SenderUtilities\ofApp.* src
copy /y /v Source\Sender\SenderUtilities\*.bat
rmdir /s /q ofSpoutExample
msbuild.exe ofSpoutExample.sln -v:m -p:Configuration=RELEASE -p:Platform=x64
rem
copy /y /v Source\Sender\SenderGraphics\ofApp.* src
copy /y /v Source\Sender\SenderGraphics\*.bat
rmdir /s /q ofSpoutExample
msbuild.exe ofSpoutExample.sln -v:m -p:Configuration=RELEASE -p:Platform=x64
rem
echo.
echo =============================================
echo Build all examples complete
echo The Binaries folder contains executable files
echo =============================================
echo.
set /p input=Enter to end
set input=
rem

