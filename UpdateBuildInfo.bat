echo Updating Build Information...
pushd %~dp0
echo HandyRun Build Information >output/BuildInfo.txt
echo -------------------------- >>output/BuildInfo.txt
git config --get remote.origin.url >>output/BuildInfo.txt
git diff-index --quiet HEAD -- || set DIRTY= [with local changes]
git log --pretty=format:"commit: %%h (%%ad) %DIRTY%" --date=iso -n 1 >>output/BuildInfo.txt
echo. >>output/BuildInfo.txt
echo build: %date:~10,4%-%date:~4,2%-%date:~7,2% %time:~0,8% >>output/BuildInfo.txt
exit /b 0