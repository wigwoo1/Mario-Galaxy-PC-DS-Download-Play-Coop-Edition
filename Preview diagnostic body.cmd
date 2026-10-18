@echo off
cd /d "%~dp0"
native\build\galaxy-probe.exe --preview-zone-body "D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs" /StageData/SoramameZone.arc
if errorlevel 1 pause
