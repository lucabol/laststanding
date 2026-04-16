@echo off
setlocal enabledelayedexpansion

REM Skip rebuild if all binaries are up-to-date
set "NEED_BUILD=0"
if not exist bin\test.exe (
    set "NEED_BUILD=1"
) else (
    for %%d in (tests examples) do (
        for %%f in (%%d\*.c) do (
            if not exist "bin\%%~nf.exe" set "NEED_BUILD=1"
        )
    )
)

if "!NEED_BUILD!"=="1" (
    call build.bat
    if !ERRORLEVEL! neq 0 exit /b !ERRORLEVEL!
) else (
    echo Skipping build: all binaries up-to-date.
)

echo === Running default regression tests ===
for %%f in (bin\test.exe bin\test_strings.exe bin\test_fs.exe bin\test_utils.exe bin\test_img.exe bin\test_tls.exe bin\test_clipboard.exe bin\gfx_test.exe bin\test_term_gfx.exe bin\ui_test.exe bin\test_svg.exe) do (
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
exit /b 0
