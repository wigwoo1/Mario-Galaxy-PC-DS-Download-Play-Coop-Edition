@echo off
call "%~dp0native\run-csv-resource-checks.cmd"
exit /b %errorlevel%
