@echo off
setlocal
set "resultDir=%~dp0native\build\texture-pointer-test"
if not exist "%resultDir%" mkdir "%resultDir%"
if not exist "%resultDir%" goto folderFailed
> "%resultDir%\RESULT.txt" echo RUNNING - Texture pointer checks started.
echo Running texture pointer checks. Please wait...
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0native\test-texture-pointers.ps1" > "%resultDir%\checks.log" 2>&1
set "checkResult=%errorlevel%"
echo.
if not "%checkResult%"=="0" goto failed
> "%resultDir%\RESULT.txt" echo PASS - Texture pointer checks passed.
goto report
:failed
> "%resultDir%\RESULT.txt" echo FAIL - Texture pointer checks failed. Exit code: %checkResult%
:report
>> "%resultDir%\RESULT.txt" echo These checks do not mean the game is playable.
>> "%resultDir%\RESULT.txt" echo Details are saved in checks.log in this folder.
type "%resultDir%\RESULT.txt"
if not "%checkResult%"=="0" type "%resultDir%\checks.log"
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
echo This window will stay open until you press a key.
pause
:done
exit /b %checkResult%
