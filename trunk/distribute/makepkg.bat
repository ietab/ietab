@echo off
echo Make package for IE Tab

set /p ver=Please input version number:

del ietab-%ver%.xpi
del ietab-tmp.zip

7z.exe a -mx9 -tzip -xr!.svn ietab-tmp.zip ..\src\extension\* ..\src\plugin\license.txt

rename ietab-tmp.zip ietab-%ver%.xpi

echo Finished!

pause
