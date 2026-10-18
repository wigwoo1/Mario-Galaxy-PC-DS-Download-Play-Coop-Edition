@echo off
"%~dp0native\build\galaxy-probe.exe" --preview-stateful-animation "D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs" /StageData/SoramameZone.arc /jmp/start/common/startinfo 0 /ObjectData/MarioAnime.arc /wait.bck
if errorlevel 1 pause
