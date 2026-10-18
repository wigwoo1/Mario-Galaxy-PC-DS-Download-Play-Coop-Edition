@echo off
setlocal
if not exist "%~dp0build\csv-check-results" mkdir "%~dp0build\csv-check-results"
set "resultDir=%~dp0build\csv-check-results"
> "%resultDir%\RESULT.txt" echo RUNNING - CSV resource checks started %date% %time%
> "%resultDir%\stage.log" echo Stage checks have not run yet in this attempt.
echo Checking real-disc planet and stage CSV resources...
echo.
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0test-csv-resource.ps1" > "%resultDir%\planet.log" 2>&1
if errorlevel 1 goto failed
type "%resultDir%\planet.log"
echo.
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0test-csv-resource.ps1" -StageTables > "%resultDir%\stage.log" 2>&1
if errorlevel 1 goto failed
type "%resultDir%\stage.log"
echo.
echo BOTH CSV CHECKS PASSED.
> "%resultDir%\RESULT.txt" echo PASS - Both CSV resource checks passed. Finished %date% %time%
>> "%resultDir%\RESULT.txt" echo These checks verify resource parsing only. The game is not yet playable.
set "checkResult=0"
goto finish
:failed
echo.
echo CSV CHECK FAILED. Read the messages above.
type "%resultDir%\planet.log"
if exist "%resultDir%\stage.log" type "%resultDir%\stage.log"
> "%resultDir%\RESULT.txt" echo FAIL - A CSV resource check failed. Finished %date% %time%
>> "%resultDir%\RESULT.txt" echo See planet.log and stage.log in this folder for details.
set "checkResult=1"
:finish
echo.
echo Saved result: "%resultDir%\RESULT.txt"
echo You can reopen that text file after closing this window.
echo.
pause
exit /b %checkResult%
