@echo off
"%~dp0native\build\galaxy-probe.exe" --preview-mario-logical-animation "D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs" "%~dp0native\build\sample-assets\mario-jump.sjis"
if errorlevel 1 pause
