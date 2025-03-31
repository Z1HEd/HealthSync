@echo off
xcopy /Y "%1" "G:\Programs\steam\steamapps\common\4D Miner Demo\mods\HealthSync\"
xcopy /Y "%2info.json5" "G:\Programs\steam\steamapps\common\4D Miner Demo\mods\HealthSync\"
REM xcopy /E /I /Y /D "%2Res" "G:\Programs\steam\steamapps\common\4D Miner Demo\mods\HealthSync\assets\"