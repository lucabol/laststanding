@echo off
setlocal enabledelayedexpansion

REM Build first so examples and test_net are current.
call build.bat
if !ERRORLEVEL! neq 0 exit /b !ERRORLEVEL!

echo === Running on-demand socket/runtime checks ===
if exist bin\test_net.exe (
    echo --- Running bin\test_net.exe ---
    bin\test_net.exe
    if !ERRORLEVEL! neq 0 (
        echo FAIL: bin\test_net.exe exited with error
        exit /b !ERRORLEVEL!
    )
    echo.
)

powershell -NoProfile -ExecutionPolicy Bypass -File tests\smoke\showcase_smoke.ps1
if !ERRORLEVEL! neq 0 exit /b !ERRORLEVEL!
exit /b 0
