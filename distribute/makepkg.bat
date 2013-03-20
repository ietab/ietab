@echo off
echo Make package for IE Tab

rem set /p ver=Please input version number:
set /p ver=<version.txt


del ietab-%ver%.xpi
del ietab-tmp.zip

del ..\src\extension\plugins\npietab.dll
copy ..\src\plugin\Release\npIETab.dll ..\src\extension\plugins

7za.exe a -mx9 -tzip -xr!.svn ietab-tmp.zip ..\src\extension\* ..\src\plugin\license.txt

rename ietab-tmp.zip ietab-%ver%.xpi

echo Finished!

pause
