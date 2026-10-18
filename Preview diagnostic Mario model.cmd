@echo off
cd /d "%~dp0"
echo Mario model on diagnostic sphere physics - not original Mario gameplay.
echo WASD moves, Space jumps, Escape closes.
"native\build\galaxy-probe.exe" --preview-scenario-mario "D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs" EggStarGalaxy 1 SoramameZone
pause
