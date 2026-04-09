@echo off
setlocal enabledelayedexpansion
echo === Quick smoke test (tests only, no build) ===
set "FAIL=0"
for %%f in (bin\test.exe bin\test_strings.exe bin\test_fs.exe bin\test_utils.exe bin\test_img.exe bin\gfx_test.exe bin\ui_test.exe) do (
    if exist "%%f" (
        "%%f" >nul 2>&1
        if !ERRORLEVEL! neq 0 (
            echo FAIL: %%f
            set "FAIL=1"
        )
    )
)
if "!FAIL!"=="1" (
    echo === SOME TESTS FAILED ===
    exit /b 1
)
echo === All tests passed ===
exit /b 0
