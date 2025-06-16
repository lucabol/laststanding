@echo off
REM Windows verification script for stdlib independence and bloat-free executables
REM This script works with Windows SDK tools and MinGW tools

echo === Verifying executables are stdlib-independent and bloat-free ===

REM Ensure bin directory exists and has executables
if not exist bin\ (
    echo Error: bin directory not found. Run build.bat first.
    exit /b 1
)

REM Build executables first (basic Windows build)
if not exist bin\test.exe (
    echo Building executables first...
    call build.bat
)

for %%f in (bin\*.exe) do (
    echo.
    echo Analyzing: %%f
    echo ----------------------------------------
    
    REM Check file type using PowerShell
    echo | set /p="✓ File type: "
    powershell -Command "Get-ItemProperty '%%f' | Select-Object Name, Length" 2>nul
    
    REM Check for dependencies and print all of them
    echo | set /p="✓ Dependencies: "
    where dumpbin >nul 2>&1
    if %errorlevel% equ 0 (
        REM Test if dumpbin actually works by running it
        dumpbin /dependents "%%f" >nul 2>&1
        if %errorlevel% equ 0 (
            REM dumpbin works, show all dependencies
            echo.
            echo   All dependencies for %%f:
            dumpbin /dependents "%%f" 2>nul
        ) else (
            echo SKIP ^(dumpbin available but failed to analyze file^)
        )
    ) else (
        REM Fallback to objdump if available (MinGW)
        where objdump >nul 2>&1
        if %errorlevel% equ 0 (
            echo.
            echo   All dependencies for %%f:
            objdump -p "%%f" 2>nul | findstr "DLL Name"
            if %errorlevel% neq 0 (
                echo   ^(No dynamic dependencies found^)
            )
        ) else (
            echo SKIP ^(no dependency analysis tools available^)
        )
    )
    
    REM Check for standard library symbols using strings or findstr
    echo | set /p="✓ No stdlib symbols: "
    where strings >nul 2>&1
    if %errorlevel% equ 0 (
        strings "%%f" | findstr /I "libc glibc stdlib printf malloc free msvcrt" >nul
        if %errorlevel% neq 0 (
            echo PASS
        ) else (
            echo FAIL ^(found stdlib references^)
            strings "%%f" | findstr /I "libc glibc stdlib printf malloc free msvcrt"
        )
    ) else (
        REM Fallback to simple binary search
        findstr /L "msvcrt libc stdlib" "%%f" >nul 2>&1
        if %errorlevel% neq 0 (
            echo PASS ^(basic check^)
        ) else (
            echo WARN ^(possible stdlib references^)
        )
    )
    
    REM Show file size
    echo | set /p="✓ Binary size: "
    for %%A in ("%%f") do echo %%~zA bytes
    
    REM Check for exports/symbols using dumpbin
    echo | set /p="✓ Symbol count: "
    where dumpbin >nul 2>&1
    if %errorlevel% equ 0 (
        for /f %%i in ('dumpbin /exports "%%f" 2^>nul ^| findstr /C:"ordinal hint" ^| find /c /v ""') do (
            if %%i gtr 0 (
                echo WARN ^(%%i exported symbols^)
            ) else (
                echo PASS ^(no exports^)
            )
        )
    ) else (
        where objdump >nul 2>&1
        if %errorlevel% equ 0 (
            for /f %%i in ('objdump -t "%%f" 2^>nul ^| find /c /v ""') do (
                if %%i gtr 5 (
                    echo WARN ^(%%i symbols^)
                ) else (
                    echo PASS ^(minimal symbols^)
                )
            )
        ) else (
            echo SKIP ^(no symbol analysis tools available^)
        )
    )
)

echo.
echo === Verification complete ===
echo.
echo Note: This script works best with Windows SDK tools ^(dumpbin^) or MinGW tools ^(objdump, strings^)
echo Install Visual Studio Build Tools or MinGW for complete analysis.