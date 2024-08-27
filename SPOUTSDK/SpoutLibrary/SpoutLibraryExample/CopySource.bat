@echo off
rem
rem Copies source files to SpoutLibraryExample "src" folder
rem
cls
echo.
echo Copy source files to the example 'src' folder
rem
echo.
echo [1] Sender                  Basic graphics sender
echo [2] Receiver                Basic graphics receiver
echo [3] Multiple receivers      Multiple receivers in one application
echo [4] Dynamic                 Dynamic load of SpoutLibrary
echo.
set /P input=Enter option to copy source files :
if "%input%" == "" goto End
rem
echo.
if %input% EQU 1 copy /y /v Source\Sender\*.* src
if %input% EQU 2 copy /y /v Source\Receiver\*.* src
if %input% EQU 3 copy /y /v Source\"Multiple receivers"\*.* src
if %input% EQU 4 copy /y /v Source\Dynamic\*.* src
rem
echo.
echo Rebuild "SpoutLibraryExample" with the new source files
echo Rescan can be used to refresh Intellisense
echo.
set /p input=Enter to end
set input=
rem
:End
rem







