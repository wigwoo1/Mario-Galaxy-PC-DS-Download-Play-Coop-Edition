@echo off
setlocal

echo Running ArchiveHolder cleanup checks...
echo.
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0test-archive-holder-cleanup.ps1" -Implementation petari
if errorlevel 1 goto failed

echo.
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0test-archive-holder-cleanup.ps1" -Implementation native
if errorlevel 1 goto failed

echo.
echo Checking archive reference-count cleanup...
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0test-file-loader-unmount.ps1"
if errorlevel 1 goto failed
echo.
echo ALL THREE CHECKS PASSED.
echo Log: "%~dp0build\archive-holder-cleanup-test\build.log"
goto finish

:failed
echo.
echo CHECK FAILED. Read the error messages above and inspect the build log.
echo Log: "%~dp0build\archive-holder-cleanup-test\build.log"

:finish
echo.
pause
endlocal
