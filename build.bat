@echo off
setlocal

REM Auto-detect a C compiler. Works from any cmd.exe window.

REM --- 1. If clang or cl is already on PATH, use it ---
where clang >nul 2>&1 && ( set "CC=clang" & goto :found )
where cl    >nul 2>&1 && ( set "CC=cl"    & goto :found )

REM --- 2. Try vswhere to locate Visual Studio ---
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist "%VSWHERE%" (
    for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath 2^>nul`) do (
        set "VSINSTALL=%%i"
    )
)

REM --- 3. If VS found, check for bundled clang first, then vcvarsall ---
if defined VSINSTALL (
    if exist "%VSINSTALL%\VC\Tools\Llvm\x64\bin\clang.exe" (
        set "CC=%VSINSTALL%\VC\Tools\Llvm\x64\bin\clang.exe"
        goto :found
    )
    set "VCVARSALL=%VSINSTALL%\VC\Auxiliary\Build\vcvarsall.bat"
    if exist "%VCVARSALL%" (
        echo Setting up MSVC environment via vcvarsall.bat ...
        call "%VCVARSALL%" x64 >nul 2>&1
        where clang >nul 2>&1 && ( set "CC=clang" & goto :found )
        where cl    >nul 2>&1 && ( set "CC=cl"    & goto :found )
    )
)

REM --- 4. Nothing found ---
echo Error: No C compiler found.
echo   Install one of: LLVM/clang, Visual Studio Build Tools
exit /b 1

:found
echo Using compiler: %CC%
if not exist bin mkdir bin

for %%f in (test\*.c) do (
    echo   %%~nf
    "%CC%" -I. "%%f" -O3 -lkernel32 -ffreestanding -o "bin\%%~nf.exe"
    if errorlevel 1 (
        echo FAILED: %%~nf
        exit /b 1
    )
)
echo Build complete.
