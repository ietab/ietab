@echo off
echo Make package for IE Tab

rem set /p ver=Please input version number:
set /p ver=<version.txt


del ietab-%ver%.xpi
del ietab-tmp.zip

rem del ..\src\extension\plugins\npietab.dll
copy /y ..\src\plugin\Release\npIETab.dll ..\src\extension\plugins\npIETab.dll

7za.exe a -mx9 -tzip -xr!.svn ietab-tmp.zip ..\src\extension\* ..\src\plugin\license.txt

rename ietab-tmp.zip ietab-%ver%.xpi

echo Finished!

pause
