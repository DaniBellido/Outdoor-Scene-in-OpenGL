@echo off
setlocal enableDelayedExpansion

if exist game\stdafx.h goto start
if exist 3dgp\3dgp.vcxproj goto start
echo Run this script from the project root folder (where *.SLN file is stored)
echo Script aborted.
pause
goto eof

:start
echo This script will clean your solution by removing all non-essential files.
choice /M "Continue"
if errorlevel 2 goto eof

if exist *.suo del /A:H *.suo 
if exist *.sdf del *.sdf 
if exist *.db del *.db
if exist *.opendb del *.opendb
if exist dump.txt del dump.txt
if exist Debug\*.* rmdir /S /Q Debug
if exist Release\*.* rmdir /S /Q Release
if exist game\Debug\*.* rmdir /S /Q game\Debug
if exist game\Release\*.* rmdir /S /Q game\Release
if exist 3dgp\Debug\*.* rmdir /S /Q 3dgp\Debug
if exist 3dgp\Release\*.* rmdir /S /Q 3dgp\Release
if exist ipch\*.* rmdir /S /Q ipch
if exist .vs\*.* rmdir /S /Q .vs
echo.
echo All non-essential files have been removed.
echo.

:eof

