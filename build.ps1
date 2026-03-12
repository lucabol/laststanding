<#
.SYNOPSIS
    Unified build script for laststanding. Wraps Windows-native and WSL-based Linux/ARM builds.
.PARAMETER Target
    Build target: windows, linux, arm, all (default: all)
.PARAMETER Action
    Action to perform: build, test, verify, all (default: all)
.PARAMETER Compiler
    Compiler for Linux/ARM targets: gcc, clang (default: gcc)
.PARAMETER OptLevel
    Optimization level 0-3 (default: 3)
.EXAMPLE
    .\build.ps1 -Target windows -Action build
    .\build.ps1 -Target linux -Action test -Compiler clang -OptLevel 2
    .\build.ps1   # builds, tests, and verifies everything
#>

param(
    [ValidateSet('windows', 'linux', 'arm', 'all')]
    [string]$Target = 'all',

    [ValidateSet('build', 'test', 'verify', 'all')]
    [string]$Action = 'all',

    [ValidateSet('gcc', 'clang')]
    [string]$Compiler = 'gcc',

    [ValidateRange(0, 3)]
    [int]$OptLevel = 3
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Continue'

# --- Resolve repo root ---
$RepoRoot = if ($PSScriptRoot) { $PSScriptRoot } else { (git rev-parse --show-toplevel 2>$null) }
if (-not $RepoRoot -or -not (Test-Path $RepoRoot)) {
    Write-Host "FAIL: Cannot determine repository root." -ForegroundColor Red
    exit 1
}
Push-Location $RepoRoot

# --- WSL path translation ---
function Get-WslPath {
    param([string]$WinPath)
    $p = $WinPath -replace '\\', '/'
    if ($p -match '^([A-Za-z]):(.*)') {
        $drive = $Matches[1].ToLower()
        $rest  = $Matches[2]
        return "/mnt/$drive$rest"
    }
    return $p
}

$WslRepoRoot = Get-WslPath $RepoRoot

# --- State tracking ---
$Results = [System.Collections.ArrayList]::new()

function Write-Header {
    param([string]$Text)
    Write-Host ""
    Write-Host "=== $Text ===" -ForegroundColor Cyan
}

function Add-Result {
    param([string]$Target, [string]$Action, [string]$Status)
    [void]$Results.Add([PSCustomObject]@{
        Target = $Target
        Action = $Action
        Status = $Status
    })
}

# --- WSL availability check (cached) ---
$script:WslAvailable = $null
function Test-WslAvailable {
    if ($null -ne $script:WslAvailable) { return $script:WslAvailable }
    try {
        $out = wsl echo ok 2>&1
        $script:WslAvailable = ($out -match 'ok')
    } catch {
        $script:WslAvailable = $false
    }
    return $script:WslAvailable
}

# --- ARM cross-compiler check (cached) ---
$script:ArmAvailable = $null
function Test-ArmCompiler {
    if ($null -ne $script:ArmAvailable) { return $script:ArmAvailable }
    if (-not (Test-WslAvailable)) { $script:ArmAvailable = $false; return $false }
    try {
        $out = wsl bash -c "command -v arm-linux-gnueabihf-gcc" 2>&1
        $script:ArmAvailable = ($LASTEXITCODE -eq 0)
    } catch {
        $script:ArmAvailable = $false
    }
    return $script:ArmAvailable
}

# --- QEMU emulator checks (cached) ---
$script:QemuArmAvailable = $null
function Test-QemuArm {
    if ($null -ne $script:QemuArmAvailable) { return $script:QemuArmAvailable }
    if (-not (Test-WslAvailable)) { $script:QemuArmAvailable = $false; return $false }
    try {
        $out = wsl bash -c "which qemu-arm" 2>&1
        $script:QemuArmAvailable = ($LASTEXITCODE -eq 0)
    } catch {
        $script:QemuArmAvailable = $false
    }
    return $script:QemuArmAvailable
}

$script:QemuAarch64Available = $null
function Test-QemuAarch64 {
    if ($null -ne $script:QemuAarch64Available) { return $script:QemuAarch64Available }
    if (-not (Test-WslAvailable)) { $script:QemuAarch64Available = $false; return $false }
    try {
        $out = wsl bash -c "which qemu-aarch64" 2>&1
        $script:QemuAarch64Available = ($LASTEXITCODE -eq 0)
    } catch {
        $script:QemuAarch64Available = $false
    }
    return $script:QemuAarch64Available
}

# --- Run a step, report result ---
function Invoke-Step {
    param(
        [string]$TargetName,
        [string]$ActionName,
        [scriptblock]$Block
    )
    Write-Header "$TargetName $ActionName"
    try {
        & $Block
        if ($LASTEXITCODE -and $LASTEXITCODE -ne 0) {
            Write-Host "FAIL" -ForegroundColor Red
            Add-Result $TargetName $ActionName "FAIL"
            return
        }
        Write-Host "PASS" -ForegroundColor Green
        Add-Result $TargetName $ActionName "PASS"
    } catch {
        Write-Host "FAIL: $_" -ForegroundColor Red
        Add-Result $TargetName $ActionName "FAIL"
    }
}

# ============================================================
#  Windows actions
# ============================================================
function Invoke-WindowsBuild {
    Invoke-Step "Windows" "Build" {
        cmd /c "call build.bat"
    }
}

function Invoke-WindowsTest {
    Invoke-Step "Windows" "Test" {
        cmd /c "call test_all.bat"
    }
}

function Invoke-WindowsVerify {
    Invoke-Step "Windows" "Verify" {
        cmd /c "call verify.bat"
    }
}

# --- CRLF fix for WSL builds ---
# Windows checkouts have CRLF; gcc and bash in WSL choke on \r in
# macro continuations, string literals, and GCC statement expressions.
# Do the conversion in pure PowerShell — avoids quoting issues with
# sed through PowerShell → WSL → bash, and avoids sed -i reliability
# problems on /mnt/c/ (NTFS-via-WSL) filesystems.
$script:CrlfFixed = $false
function Invoke-CrlfFix {
    if ($script:CrlfFixed) { return }
    Write-Host "  Stripping CRLF for WSL..." -ForegroundColor DarkGray
    $targets = Get-ChildItem -Path $RepoRoot -Include '*.c','*.h' -Recurse -File
    $taskfile = Join-Path $RepoRoot 'Taskfile'
    if (Test-Path $taskfile) { $targets += Get-Item $taskfile }
    foreach ($f in $targets) {
        $bytes = [System.IO.File]::ReadAllBytes($f.FullName)
        $hasCR = $false
        foreach ($b in $bytes) { if ($b -eq 0x0D) { $hasCR = $true; break } }
        if ($hasCR) {
            $clean = [System.Collections.Generic.List[byte]]::new($bytes.Length)
            foreach ($b in $bytes) { if ($b -ne 0x0D) { $clean.Add($b) } }
            [System.IO.File]::WriteAllBytes($f.FullName, $clean.ToArray())
        }
    }
    $script:CrlfFixed = $true
}

# ============================================================
#  Linux actions (via WSL)
# ============================================================
function Invoke-LinuxBuild {
    Invoke-Step "Linux" "Build" {
        Invoke-CrlfFix
        wsl bash -c "cd '$WslRepoRoot' && ./Taskfile build $Compiler $OptLevel"
    }
}

function Invoke-LinuxTest {
    Invoke-Step "Linux" "Test" {
        Invoke-CrlfFix
        wsl bash -c "cd '$WslRepoRoot' && ./Taskfile test $Compiler $OptLevel"
    }
}

function Invoke-LinuxVerify {
    Invoke-Step "Linux" "Verify" {
        Invoke-CrlfFix
        wsl bash -c "cd '$WslRepoRoot' && ./Taskfile verify"
    }
}

# ============================================================
#  ARM actions (via WSL + QEMU)
# ============================================================
function Invoke-ArmBuild {
    Invoke-Step "ARM" "Build" {
        Invoke-CrlfFix
        wsl bash -c "cd '$WslRepoRoot' && ./Taskfile build_arm"
    }
}

function Invoke-ArmTest {
    if (-not (Test-QemuArm)) {
        Write-Header "ARM Test"
        Write-Host "SKIP [install qemu-user in WSL: sudo apt-get install qemu-user]" -ForegroundColor Yellow
        Add-Result "ARM" "Test" "SKIP"
        return
    }
    Invoke-Step "ARM" "Test" {
        Invoke-CrlfFix
        wsl bash -c "cd '$WslRepoRoot' && ./Taskfile test_arm"
    }
}

function Invoke-Aarch64Test {
    if (-not (Test-QemuAarch64)) {
        Write-Header "AArch64 Test"
        Write-Host "SKIP [install qemu-user in WSL: sudo apt-get install qemu-user]" -ForegroundColor Yellow
        Add-Result "AArch64" "Test" "SKIP"
        return
    }
    Invoke-Step "AArch64" "Test" {
        Invoke-CrlfFix
        wsl bash -c "cd '$WslRepoRoot' && ./Taskfile test_aarch64"
    }
}

function Invoke-ArmVerify {
    Invoke-Step "ARM" "Verify" {
        Invoke-CrlfFix
        wsl bash -c "cd '$WslRepoRoot' && ./Taskfile verify"
    }
}

# ============================================================
#  Dispatch
# ============================================================
function Invoke-Target {
    param([string]$T, [string]$A)

    switch ($T) {
        'windows' {
            $actions = if ($A -eq 'all') { @('build', 'test', 'verify') } else { @($A) }
            foreach ($act in $actions) {
                switch ($act) {
                    'build'  { Invoke-WindowsBuild  }
                    'test'   { Invoke-WindowsTest   }
                    'verify' { Invoke-WindowsVerify }
                }
            }
        }
        'linux' {
            if (-not (Test-WslAvailable)) {
                Write-Host "SKIP: WSL not available -- skipping Linux target." -ForegroundColor Yellow
                $actions = if ($A -eq 'all') { @('build', 'test', 'verify') } else { @($A) }
                foreach ($act in $actions) { Add-Result "Linux" $act "SKIP" }
                return
            }
            $actions = if ($A -eq 'all') { @('build', 'test', 'verify') } else { @($A) }
            foreach ($act in $actions) {
                switch ($act) {
                    'build'  { Invoke-LinuxBuild  }
                    'test'   { Invoke-LinuxTest   }
                    'verify' { Invoke-LinuxVerify }
                }
            }
        }
        'arm' {
            if (-not (Test-WslAvailable)) {
                Write-Host "SKIP: WSL not available -- skipping ARM target." -ForegroundColor Yellow
                $actions = if ($A -eq 'all') { @('build', 'test', 'verify') } else { @($A) }
                foreach ($act in $actions) { Add-Result "ARM" $act "SKIP" }
                return
            }
            if (-not (Test-ArmCompiler)) {
                Write-Host "SKIP: arm-linux-gnueabihf-gcc not found in WSL -- skipping ARM target." -ForegroundColor Yellow
                $actions = if ($A -eq 'all') { @('build', 'test', 'verify') } else { @($A) }
                foreach ($act in $actions) { Add-Result "ARM" $act "SKIP" }
                return
            }
            $actions = if ($A -eq 'all') { @('build', 'test') } else { @($A) }
            foreach ($act in $actions) {
                switch ($act) {
                    'build'  { Invoke-ArmBuild  }
                    'test'   { Invoke-ArmTest   }
                    'verify' { Invoke-ArmVerify }
                }
            }
        }
    }
}

# --- Main execution ---
$targets = if ($Target -eq 'all') { @('windows', 'linux', 'arm') } else { @($Target) }
foreach ($t in $targets) {
    Invoke-Target $t $Action
}

# ============================================================
#  Summary
# ============================================================
Write-Host ""
Write-Host "=== Summary ===" -ForegroundColor Cyan
Write-Host ("{0,-10} {1,-10} {2}" -f "Target", "Action", "Result")
Write-Host ("{0,-10} {1,-10} {2}" -f "------", "------", "------")
foreach ($r in $Results) {
    $color = switch ($r.Status) {
        'PASS' { 'Green'  }
        'FAIL' { 'Red'    }
        'SKIP' { 'Yellow' }
    }
    Write-Host ("{0,-10} {1,-10} {2}" -f $r.Target, $r.Action, $r.Status) -ForegroundColor $color
}

Pop-Location

# Exit code: 1 if any FAIL
$failed = $Results | Where-Object { $_.Status -eq 'FAIL' }
if ($failed) { exit 1 } else { exit 0 }
