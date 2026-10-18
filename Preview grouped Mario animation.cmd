@echo off
"%~dp0native\build\galaxy-probe.exe" --preview-mario-group "D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs"
if errorlevel 1 pause
