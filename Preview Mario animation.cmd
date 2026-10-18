@echo off
cd /d "%~dp0"
native\build\galaxy-probe.exe --preview-animation native\build\sample-assets\mario.bdl native\build\sample-assets\run.bck
if errorlevel 1 pause
