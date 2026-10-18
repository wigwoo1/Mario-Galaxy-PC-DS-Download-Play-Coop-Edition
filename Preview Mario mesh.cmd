@echo off
"%~dp0native\build\galaxy-probe.exe" --preview-materials "%~dp0native\build\sample-assets\mario.bdl"
if errorlevel 1 pause
