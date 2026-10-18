@echo off
setlocal
set "resultDir=%~dp0native\build\visible-check-results"
if not exist "%resultDir%" mkdir "%resultDir%"
if not exist "%resultDir%" goto folderFailed
> "%resultDir%\RESULT.txt" echo RUNNING - Native camera and animation checks. Started %date% %time%
> "%resultDir%\animation.log" echo Animation checks have not run in this attempt.
echo Running camera checks. Please wait...
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0native\test-dotcam-reader.ps1" > "%resultDir%\camera.log" 2>&1
set "cameraResult=%errorlevel%"
echo Running animation and camera lifetime checks. Please wait...
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0native\test-original-xanime-core.ps1" -UseExistingGeneratedSources > "%resultDir%\animation.log" 2>&1
set "animationResult=%errorlevel%"
set "checkResult=0"
> "%resultDir%\RESULT.txt" echo Native camera and animation checks - finished %date% %time%
if "%cameraResult%"=="0" (echo Camera checks: PASS>> "%resultDir%\RESULT.txt") else (echo Camera checks: FAIL - exit code %cameraResult%>> "%resultDir%\RESULT.txt")
if "%animationResult%"=="0" (echo Animation and camera lifetime checks: PASS>> "%resultDir%\RESULT.txt") else (echo Animation and camera lifetime checks: FAIL - exit code %animationResult%>> "%resultDir%\RESULT.txt")
if not "%cameraResult%"=="0" set "checkResult=1"
if not "%animationResult%"=="0" set "checkResult=1"
if "%checkResult%"=="0" (echo Overall: PASS>> "%resultDir%\RESULT.txt") else (echo Overall: FAIL>> "%resultDir%\RESULT.txt")
>> "%resultDir%\RESULT.txt" echo These checks do not verify that the game is playable.
>> "%resultDir%\RESULT.txt" echo Details: camera.log and animation.log in this folder.
echo.
type "%resultDir%\RESULT.txt"
echo.
echo Saved result: "%resultDir%\RESULT.txt"
goto finish
:folderFailed
echo FAIL - Could not create the results folder:
echo "%resultDir%"
set "checkResult=1"
:finish
echo.
if /I "%~1"=="--no-pause" goto done
echo This window stays open until you press a key.
pause
:done
exit /b %checkResult%