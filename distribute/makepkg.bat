@echo off
echo "Make package for IE Tab"
set /p ver=Please input version number:
del *.xpi *.zip
7z.exe a -mx9 -tzip -xr!.svn ietab-tmp.zip ..\src\extension\*
rename ietab-tmp.zip ietab-%ver%.xpi
echo "Finished!"
pause
