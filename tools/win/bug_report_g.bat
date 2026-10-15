@echo off
rem Monster Hunter G: like bug_report.bat, for G's own folder (%APPDATA%\mhgpc).
set MH1_APPNAME=mhgpc
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0bug_report.ps1" %*
pause
