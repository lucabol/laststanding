@echo off
setlocal enabledelayedexpansion

call build.bat
if !ERRORLEVEL! neq 0 exit /b !ERRORLEVEL!

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
