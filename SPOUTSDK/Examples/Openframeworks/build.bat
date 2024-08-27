@echo off
rem
rem Use MSbuild to build the example project
rem
rem Remove ofSpoutExample directory to use the new source files
rmdir /s /q ofSpoutExample
rem build the project using the new files
msbuild.exe ofSpoutExample.sln -v:m -p:Configuration=RELEASE -p:Platform=x64























