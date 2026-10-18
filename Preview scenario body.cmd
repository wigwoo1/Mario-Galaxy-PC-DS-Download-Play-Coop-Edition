@echo off
cd /d "%~dp0"
native\build\galaxy-probe.exe --preview-scenario-body "D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs" EggStarGalaxy 1 SoramameZone
if errorlevel 1 pause
