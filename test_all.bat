@echo off
setlocal enabledelayedexpansion

REM Skip rebuild if all binaries are up-to-date
set "NEED_BUILD=0"
if not exist bin\test.exe (
    set "NEED_BUILD=1"
) else (
    for %%f in (test\*.c) do (
        if not exist "bin\%%~nf.exe" set "NEED_BUILD=1"
    )
)

if "!NEED_BUILD!"=="1" (
    call build.bat
    if !ERRORLEVEL! neq 0 exit /b !ERRORLEVEL!
) else (
    echo Skipping build: all binaries up-to-date.
)

for %%f in (bin\test*.exe) do (
    if exist "%%f" (
        echo --- Running %%f ---
        "%%f"
        if !ERRORLEVEL! neq 0 (
            echo FAIL: %%f exited with error
            exit /b !ERRORLEVEL!
        )
        echo.
    )
)

powershell -NoProfile -ExecutionPolicy Bypass -File test\showcase_smoke.ps1
if !ERRORLEVEL! neq 0 exit /b !ERRORLEVEL!
exit /b 0
