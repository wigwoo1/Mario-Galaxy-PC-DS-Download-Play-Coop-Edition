@echo off
"%~dp0native\build\galaxy-probe.exe" --preview-texture "%~dp0native\build\sample-assets\titlelogo.tpl"
if errorlevel 1 pause
