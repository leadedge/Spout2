@echo off
rem
rem Use MSbuild to build the example project
rem
msbuild.exe ofSpoutExample.sln -v:m -p:Configuration=RELEASE -p:Platform=x64
rem







