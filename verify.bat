@echo off
setlocal enabledelayedexpansion
REM Verify executables are stdlib-independent and bloat-free.
REM Works from any cmd.exe window -- auto-detects analysis tools.

echo === Verifying executables are stdlib-independent and bloat-free ===

if not exist bin\ (
    echo Error: bin directory not found. Run build.bat first.
    exit /b 1
)

if not exist bin\test.exe (
    echo Building executables first...
    call build.bat
)

REM --- Detect analysis tools ---
set "HAS_STRINGS="
where strings >nul 2>&1 && set "HAS_STRINGS=1"

where dumpbin >nul 2>&1 && ( set "DEP_TOOL=dumpbin" & goto :tools_ready )
where objdump >nul 2>&1 && ( set "DEP_TOOL=objdump" & goto :tools_ready )

REM Try Visual Studio for dumpbin (same pattern as build.bat)
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist "%VSWHERE%" (
    for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath 2^>nul`) do set "VSINSTALL=%%i"
)
if defined VSINSTALL (
    set "VCVARSALL=!VSINSTALL!\VC\Auxiliary\Build\vcvarsall.bat"
    if exist "!VCVARSALL!" (
        call "!VCVARSALL!" x64 >nul 2>&1
        where dumpbin >nul 2>&1 && ( set "DEP_TOOL=dumpbin" & goto :tools_ready )
    )
)

set "DEP_TOOL=none"

:tools_ready
echo Dependency tool: %DEP_TOOL%

for %%f in (bin\*.exe) do (
    echo.
    echo Analyzing: %%f
    echo ----------------------------------------

    for %%A in ("%%f") do echo   File: %%~nxA  %%~zA bytes

    REM --- Dependencies ---
    if "!DEP_TOOL!"=="dumpbin" (
        echo   Dependencies:
        dumpbin /dependents "%%f" 2>nul | findstr "\.dll"
        if !errorlevel! neq 0 echo     [none]
    )
    if "!DEP_TOOL!"=="objdump" (
        echo   Dependencies:
        objdump -p "%%f" 2>nul | findstr "DLL Name"
        if !errorlevel! neq 0 echo     [none]
    )
    if "!DEP_TOOL!"=="none" (
        echo   Dependencies: SKIP [no tool]
    )

    REM --- Stdlib symbols (DLL import check + string patterns) ---
    set "STDLIB_FAIL="

    REM Primary check: look for CRT DLL imports via dependency tool
    if "!DEP_TOOL!"=="dumpbin" (
        dumpbin /dependents "%%f" 2>nul | findstr /I "msvcrt ucrtbase vcruntime api-ms-win-crt" >nul 2>&1
        if !errorlevel! equ 0 (
            set "STDLIB_FAIL=1"
            echo   Stdlib symbols: FAIL [CRT DLL import detected]
            dumpbin /dependents "%%f" 2>nul | findstr /I "msvcrt ucrtbase vcruntime api-ms-win-crt"
        )
    )
    if "!DEP_TOOL!"=="objdump" (
        objdump -p "%%f" 2>nul | findstr /I "msvcrt ucrtbase vcruntime api-ms-win-crt" >nul 2>&1
        if !errorlevel! equ 0 (
            set "STDLIB_FAIL=1"
            echo   Stdlib symbols: FAIL [CRT DLL import detected]
            objdump -p "%%f" 2>nul | findstr /I "msvcrt ucrtbase vcruntime api-ms-win-crt"
        )
    )

    REM Secondary check: string patterns (only if primary didn't already FAIL)
    if not defined STDLIB_FAIL (
        if defined HAS_STRINGS (
            strings "%%f" | findstr /I "msvcrt ucrtbase vcruntime __libc glibc" >nul 2>&1
            if !errorlevel! neq 0 (
                echo   Stdlib symbols: PASS
            ) else (
                echo   Stdlib symbols: FAIL [CRT string found]
                strings "%%f" | findstr /I "msvcrt ucrtbase vcruntime __libc glibc"
            )
        ) else (
            findstr /L "msvcrt ucrtbase vcruntime __libc glibc" "%%f" >nul 2>&1
            if !errorlevel! neq 0 (
                echo   Stdlib symbols: PASS [basic check]
            ) else (
                echo   Stdlib symbols: WARN [possible CRT refs]
            )
        )
    )

    REM --- Symbol count ---
    if "!DEP_TOOL!"=="dumpbin" (
        for /f %%i in ('dumpbin /exports "%%f" 2^>nul ^| findstr /C:"ordinal hint" ^| find /c /v ""') do (
            if %%i gtr 0 (
                echo   Symbol count: WARN [%%i exports]
            ) else (
                echo   Symbol count: PASS [no exports]
            )
        )
    )
    if "!DEP_TOOL!"=="objdump" (
        for /f %%i in ('objdump -t "%%f" 2^>nul ^| find /c /v ""') do (
            if %%i gtr 5 (
                echo   Symbol count: WARN [%%i symbols]
            ) else (
                echo   Symbol count: PASS [minimal]
            )
        )
    )
    if "!DEP_TOOL!"=="none" (
        echo   Symbol count: SKIP [no tool]
    )
)

echo.
echo === Verification complete ===
exit /b 0