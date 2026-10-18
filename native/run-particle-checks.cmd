@echo off
setlocal
if not exist "%~dp0build\jpa-archive-test" mkdir "%~dp0build\jpa-archive-test"
set "resultDir=%~dp0build\jpa-archive-test"
> "%resultDir%\RESULT.txt" echo RUNNING - Particle resource checks started.
echo Checking real-disc particle data and PC callback allocation sizes...
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0test-jpa-archive.ps1" > "%resultDir%\checks.log" 2>&1
if errorlevel 1 goto failed
type "%resultDir%\checks.log"
> "%resultDir%\RESULT.txt" echo PASS - Particle resource and callback allocation checks passed.
echo PARTICLE CHECKS PASSED.
set "checkResult=0"
goto finish
:failed
type "%resultDir%\checks.log"
> "%resultDir%\RESULT.txt" echo FAIL - Particle checks failed. Read checks.log in this folder.
echo PARTICLE CHECKS FAILED.
set "checkResult=1"
:finish
>> "%resultDir%\RESULT.txt" echo These are resource checks. The game is not yet playable.
echo Saved result: "%resultDir%\RESULT.txt"
pause
exit /b %checkResult%
