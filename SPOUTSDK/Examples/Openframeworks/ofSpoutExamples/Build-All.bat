@echo off
rem
rem Use MSbuild to build all example projects
rem Note: build the Openframeworks library first
rem
msbuild.exe ofSpoutExamples.sln -v:m /p:Configuration=Release /p:Platform=x64























