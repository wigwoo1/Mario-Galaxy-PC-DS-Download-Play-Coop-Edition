@echo off
"%~dp0native\build\galaxy-probe.exe" --preview-stateful-placement "D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs" /StageData/SoramameZone.arc /jmp/start/common/startinfo 0
if errorlevel 1 pause
