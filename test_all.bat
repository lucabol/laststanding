call build.bat

@echo off

for %%f in (bin\*.exe) do (
    if exist "%%f" (
        echo --- Running %%f ---
        "%%f"
        echo.
    )
)
