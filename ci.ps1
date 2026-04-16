<#
.SYNOPSIS
    Unified CI script for laststanding. Builds, tests, and verifies across Windows, Linux, ARM32, AArch64, RISC-V, and WASI.
.PARAMETER Target
    Build target: windows, linux, arm, riscv, wasi, all (default: all)
.PARAMETER Action
    Action to perform: build, test, verify, all (default: all)
.PARAMETER Compiler
    Compiler for Linux and ARM targets: gcc, clang, all (default: all — runs both gcc and clang)
.PARAMETER OptLevel
    Optimization level: 0, 1, 2, 3, s, or z (default: z — optimize for size)
.PARAMETER ShowAll
    Show full compiler output, test assertions, and verification details.
    By default output is concise: one PASS/FAIL line per step (full output shown on failure).
.PARAMETER Help
    Show this help message.
.EXAMPLE
    .\ci.ps1 -Target windows -Action build
    .\ci.ps1 -Target linux -Action test -Compiler clang -OptLevel 2
    .\ci.ps1 -ShowAll              # verbose: show everything
    .\ci.ps1                       # concise: builds, tests, and verifies everything
#>

param(
    [ValidateSet('windows', 'linux', 'arm', 'riscv', 'wasi', 'all')]
    [string]$Target = 'all',

    [ValidateSet('build', 'test', 'verify', 'all')]
    [string]$Action = 'all',

    [ValidateSet('gcc', 'clang', 'all')]
    [string]$Compiler = 'all',

    [ValidateSet('0', '1', '2', '3', 's', 'z')]
    [string]$OptLevel = 'z',

    [switch]$ShowAll,

    [switch]$Help,

    # Internal: used by parallel execution to mark sub-process invocations
    [switch]$SubProcess
)

if ($Help) {
    Get-Help $MyInvocation.MyCommand.Path -Detailed
    return
}

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Continue'

# --- OS detection ---
# PowerShell Core (pwsh) provides $IsLinux, $IsWindows, $IsMacOS.
# Windows PowerShell 5.1 doesn't define them — treat as Windows.
$RunningOnLinux   = if (Test-Path variable:IsLinux)   { $IsLinux }   else { $false }
$RunningOnWindows = if (Test-Path variable:IsWindows) { $IsWindows } else { $true }

# --- Resolve repo root ---
$RepoRoot = if ($PSScriptRoot) { $PSScriptRoot } else { (git rev-parse --show-toplevel 2>$null) }
if (-not $RepoRoot -or -not (Test-Path $RepoRoot)) {
    Write-Host "FAIL: Cannot determine repository root." -ForegroundColor Red
    exit 1
}
Push-Location $RepoRoot

# --- WSL path translation (Windows only) ---
$WslRepoRoot = $null
if ($RunningOnWindows) {
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
}

# --- State tracking ---
$Results = [System.Collections.ArrayList]::new()
$script:BinarySizes = @{}  # Key: target name, Value: hashtable of { binaryName: sizeBytes }

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
    if ($SubProcess) {
        Write-Output "##CI_RESULT##$Target##$Action##$Status##"
    }
}

# --- Step summary helpers (concise mode) ---
function Get-BuildSummary {
    param([string]$TargetName, [string]$Output)

    $cc = ""
    if ($Output -match 'Using compiler:\s*(.+)') {
        $ccRaw = $Matches[1].Trim()
        if ($ccRaw -match '[/\\]') {
            $cc = [System.IO.Path]::GetFileNameWithoutExtension($ccRaw)
        } else {
            $cc = $ccRaw
        }
    }
    if (-not $cc) {
        if ($TargetName -match 'Linux') {
            $cc = if ($TargetName -match 'clang') { 'clang' } else { 'gcc' }
        } elseif ($TargetName -match 'AArch64') {
            $cc = if ($TargetName -match 'clang') { 'clang' } else { 'aarch64-linux-gnu-gcc' }
        } elseif ($TargetName -match 'ARM') {
            $cc = if ($TargetName -match 'clang') { 'clang' } else { 'arm-linux-gnueabihf-gcc' }
        }
    }

    $cmd = if ($TargetName -eq 'Windows') {
        "$cc -I. -O$OptLevel -lkernel32 -ffreestanding"
    } elseif ($TargetName -match 'Linux') {
        "$cc -I. -O$OptLevel -ffreestanding -nostdlib"
    } elseif ($TargetName -match 'AArch64') {
        "$cc -I. -O$OptLevel -ffreestanding -nostdlib"
    } elseif ($TargetName -match 'ARM') {
        "$cc -I. -O$OptLevel -ffreestanding -nostdlib"
    } else {
        $cc
    }
    if ($cmd.Length -gt 42) { $cmd = $cmd.Substring(0, 39) + "..." }

    # Count compiled files from output (build scripts echo basenames)
    $fileCount = 0
    foreach ($line in ($Output -split "`n")) {
        $t = $line.Trim()
        if ($t -and $t -match '^\w[\w_.-]*$' -and $t -notmatch '^(Using|Build|Setting|Error|FAILED|mkdir|call)') {
            $fileCount++
        }
    }
    if ($fileCount -eq 0) {
        $fileCount =
            @(Get-ChildItem -Path (Join-Path $RepoRoot 'tests') -Filter '*.c' -File -ErrorAction SilentlyContinue).Count +
            @(Get-ChildItem -Path (Join-Path $RepoRoot 'examples') -Filter '*.c' -File -ErrorAction SilentlyContinue).Count
    }

    $parts = @()
    if ($cmd) { $parts += $cmd }
    if ($fileCount -gt 0) { $parts += "$fileCount files" }
    return ($parts -join "   ")
}

function Get-TestSummary {
    param([string]$TargetName, [string]$Output)

    $okCount = ([regex]::Matches($Output, '\[OK\]')).Count
    $regressionCount = ([regex]::Matches($Output, '--- Running bin[\\/][^ ]+ ---')).Count
    $runCount = ([regex]::Matches($Output, '--- Running ')).Count
    $smokeCount = $runCount - $regressionCount

    $parts = @()
    if ($regressionCount -gt 0) { $parts += "$regressionCount regression binaries" }
    if ($smokeCount -gt 0) { $parts += "$smokeCount smoke checks" }
    if ($okCount -gt 0) { $parts += "$okCount assertions" }
    if ($TargetName -match 'ARM' -or $TargetName -match 'AArch64') { $parts += "(QEMU)" }

    return ($parts -join ", ")
}

function Get-VerifySummary {
    param([string]$TargetName, [string]$Output)

    $analyzeCount = ([regex]::Matches($Output, 'Analyzing:')).Count

    # Extract binary sizes and populate comparison table data
    $maxSize = [int64]0
    Collect-BinarySizes $TargetName $Output
    if ($script:BinarySizes.ContainsKey($TargetName)) {
        foreach ($sz in $script:BinarySizes[$TargetName].Values) {
            if ($sz -gt $maxSize) { $maxSize = $sz }
        }
    }

    # Dependency type
    $depType = ""
    if ($TargetName -eq 'Windows') {
        $dllMatches = [regex]::Matches($Output, '(?i)(\w+)\.dll')
        $dlls = @($dllMatches | ForEach-Object { $_.Groups[1].Value.ToUpper() } | Sort-Object -Unique)
        if ($dlls.Count -ge 1 -and @($dlls | Where-Object { $_ -ne 'KERNEL32' }).Count -eq 0) {
            $depType = "KERNEL32 only"
        } elseif ($dlls.Count -eq 0) {
            $depType = "no imports"
        }
    } else {
        if ($Output -match 'statically linked' -or $Output -match 'no dynamic section' -or $Output -match 'No dynamic dependencies') {
            $depType = "static"
        }
    }

    $parts = @()
    if ($analyzeCount -gt 0) { $parts += "$analyzeCount binaries" }
    if ($maxSize -gt 0) {
        $kb = [math]::Ceiling($maxSize / 1024)
        $parts += "max ${kb}KB"
    }
    if ($depType) { $parts += $depType }

    return ($parts -join ", ")
}

function Collect-BinarySizes {
    param([string]$TargetName, [string]$Output)

    $sizes = @{}
    $lines = $Output -split "`n"
    $currentBinary = ""

    foreach ($line in $lines) {
        if ($line -match 'Analyzing:\s*(.+)') {
            $path = $Matches[1].Trim()
            $currentBinary = [System.IO.Path]::GetFileNameWithoutExtension($path)
        }
        # Windows verify.bat format: "File: test.exe  16384 bytes"
        if ($line -match 'File:\s+(\S+)\s+(\d+)\s+bytes') {
            $fname = $Matches[1]
            $sz = [int64]$Matches[2]
            $name = [System.IO.Path]::GetFileNameWithoutExtension($fname)
            $sizes[$name] = $sz
        }
        # Linux/ARM Taskfile format: "Binary size: 13401 bytes (text: ...)"
        elseif ($line -match 'Binary size:\s+(\d+)\s+bytes') {
            $sz = [int64]$Matches[1]
            if ($currentBinary) { $sizes[$currentBinary] = $sz }
        }
    }
    if ($sizes.Count -gt 0) {
        $script:BinarySizes[$TargetName] = $sizes
    }
}

function Emit-BinarySizeMarkers {
    param([string]$TargetName)
    if (-not $SubProcess) { return }
    if (-not $script:BinarySizes.ContainsKey($TargetName)) { return }
    $pairs = @()
    foreach ($k in $script:BinarySizes[$TargetName].Keys) {
        $pairs += "${k}:$($script:BinarySizes[$TargetName][$k])"
    }
    Write-Output "##CI_SIZES##$TargetName##$($pairs -join ',')##"
}

function Get-StepSummary {
    param([string]$TargetName, [string]$ActionName, [string]$Output)
    switch ($ActionName) {
        'Build'  { return Get-BuildSummary $TargetName $Output }
        'Test'   { return Get-TestSummary  $TargetName $Output }
        'Verify' { return Get-VerifySummary $TargetName $Output }
    }
    return ""
}

function Write-BinarySizeTable {
    $targets = @($script:BinarySizes.Keys | Sort-Object @{
        Expression = {
            if ($script:BinarySizes[$_].ContainsKey('hello')) {
                $script:BinarySizes[$_]['hello']
            } else { [int]::MaxValue }
        }
    })
    if ($targets.Count -eq 0) { return }

    # Short display names for row labels
    $shortNames = @{
        'Windows'       = 'Win/clang'
        'Linux (gcc)'   = 'Lin/gcc'
        'Linux (clang)' = 'Lin/clang'
        'ARM (gcc)'     = 'ARM/gcc'
        'ARM (clang)'   = 'ARM/clang'
        'AArch64 (gcc)'   = 'A64/gcc'
        'AArch64 (clang)' = 'A64/clang'
    }

    # Show only representative binaries (smallest CLI + UI) to keep table readable
    $showBinaries = @('hello', 'ui_controls')
    $allBinaries = @()
    foreach ($t in $targets) {
        foreach ($name in $script:BinarySizes[$t].Keys) {
            if ($name -in $showBinaries -and $name -notin $allBinaries) { $allBinaries += $name }
        }
    }
    $allBinaries = @($allBinaries | Sort-Object)
    if ($allBinaries.Count -eq 0) { return }

    Write-Host ""
    Write-Host "=== Binary Sizes ===" -ForegroundColor Cyan

    # Build config labels for first column
    $configLabels = @()
    foreach ($t in $targets) {
        $configLabels += if ($shortNames.ContainsKey($t)) { $shortNames[$t] } else { $t }
    }
    $configWidth = [math]::Max("Configuration".Length, ($configLabels | ForEach-Object { $_.Length } | Measure-Object -Maximum).Maximum)

    # Compute column widths per binary (right-aligned numbers)
    $colWidths = @{}
    foreach ($bin in $allBinaries) {
        $w = $bin.Length
        foreach ($t in $targets) {
            if ($script:BinarySizes[$t].ContainsKey($bin)) {
                $len = ("{0:N0}" -f $script:BinarySizes[$t][$bin]).Length
                if ($len -gt $w) { $w = $len }
            }
        }
        $colWidths[$bin] = $w
    }

    # Header (binaries as columns)
    $header = "Configuration".PadRight($configWidth)
    $sep    = ("-" * "Configuration".Length).PadRight($configWidth)
    foreach ($bin in $allBinaries) {
        $w = $colWidths[$bin]
        $header += "  " + $bin.PadLeft($w)
        $sep    += "  " + ("-" * $bin.Length).PadLeft($w)
    }
    Write-Host $header
    Write-Host $sep

    # Rows (one per configuration)
    for ($i = 0; $i -lt $targets.Count; $i++) {
        $t = $targets[$i]
        $label = $configLabels[$i]
        $row = $label.PadRight($configWidth)
        foreach ($bin in $allBinaries) {
            $w = $colWidths[$bin]
            if ($script:BinarySizes[$t].ContainsKey($bin)) {
                $row += "  " + ("{0:N0}" -f $script:BinarySizes[$t][$bin]).PadLeft($w)
            } else {
                $row += "  " + "-".PadLeft($w)
            }
        }
        Write-Host $row
    }
}

function Write-BinarySizeJson {
    if ($script:BinarySizes.Count -eq 0) { return }
    $binDir = Join-Path $RepoRoot 'bin'
    if (-not (Test-Path $binDir)) { New-Item -ItemType Directory -Path $binDir -Force | Out-Null }
    $jsonPath = Join-Path $binDir '.ci_sizes.json'
    $showBinaries = @('hello', 'ui_controls')
    $entries = @()
    foreach ($target in $script:BinarySizes.Keys) {
        $entry = @{ target = $target }
        foreach ($bin in $script:BinarySizes[$target].Keys) {
            if ($bin -in $showBinaries) { $entry[$bin] = $script:BinarySizes[$target][$bin] }
        }
        $entries += $entry
    }
    $entries | ConvertTo-Json -Depth 3 | Set-Content -Path $jsonPath -Encoding UTF8
}

function Write-BinarySizeMarkdownSummary {
    if (-not $env:GITHUB_STEP_SUMMARY) { return }
    if ($script:BinarySizes.Count -eq 0) { return }

    $shortNames = @{
        'Windows'           = 'Win/clang'
        'Linux (gcc)'       = 'Lin/gcc'
        'Linux (clang)'     = 'Lin/clang'
        'ARM (gcc)'         = 'ARM/gcc'
        'ARM (clang)'       = 'ARM/clang'
        'AArch64 (gcc)'     = 'A64/gcc'
        'AArch64 (clang)'   = 'A64/clang'
        'RISC-V (gcc)'      = 'RV64/gcc'
        'WASI (clang)'      = 'WASI/clang'
    }

    $targets = @($script:BinarySizes.Keys | Sort-Object @{
        Expression = {
            if ($script:BinarySizes[$_].ContainsKey('hello')) {
                $script:BinarySizes[$_]['hello']
            } else { [int]::MaxValue }
        }
    })

    $showBinaries = @('hello', 'ui_controls')
    $allBinaries = @()
    foreach ($t in $targets) {
        foreach ($name in $script:BinarySizes[$t].Keys) {
            if ($name -in $showBinaries -and $name -notin $allBinaries) { $allBinaries += $name }
        }
    }
    $allBinaries = @($allBinaries | Sort-Object)
    if ($allBinaries.Count -eq 0) { return }

    $md = @()
    $md += '### 📏 Binary Sizes'
    $md += ''
    $headerCols = @('Configuration') + $allBinaries
    $md += '| ' + ($headerCols -join ' | ') + ' |'
    $md += '| ' + (($headerCols | ForEach-Object { '---' }) -join ' | ') + ' |'

    foreach ($t in $targets) {
        $label = if ($shortNames.ContainsKey($t)) { $shortNames[$t] } else { $t }
        $cols = @($label)
        foreach ($bin in $allBinaries) {
            if ($script:BinarySizes[$t].ContainsKey($bin)) {
                $cols += '{0:N0}' -f $script:BinarySizes[$t][$bin]
            } else {
                $cols += '-'
            }
        }
        $md += '| ' + ($cols -join ' | ') + ' |'
    }

    $md -join "`n" | Out-File -FilePath $env:GITHUB_STEP_SUMMARY -Append -Encoding UTF8
}

# --- WSL availability check (cached, Windows only) ---
$script:WslAvailable = $null
function Test-WslAvailable {
    if ($RunningOnLinux) { return $false }  # Not needed on Linux
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
    if ($RunningOnLinux) {
        try {
            bash -c "command -v arm-linux-gnueabihf-gcc" 2>&1 | Out-Null
            $script:ArmAvailable = ($LASTEXITCODE -eq 0)
        } catch { $script:ArmAvailable = $false }
    } else {
        if (-not (Test-WslAvailable)) { $script:ArmAvailable = $false; return $false }
        try {
            $out = wsl bash -c "command -v arm-linux-gnueabihf-gcc" 2>&1
            $script:ArmAvailable = ($LASTEXITCODE -eq 0)
        } catch { $script:ArmAvailable = $false }
    }
    return $script:ArmAvailable
}

$script:Aarch64Available = $null
function Test-Aarch64Compiler {
    if ($null -ne $script:Aarch64Available) { return $script:Aarch64Available }
    if ($RunningOnLinux) {
        try {
            bash -c "command -v aarch64-linux-gnu-gcc" 2>&1 | Out-Null
            $script:Aarch64Available = ($LASTEXITCODE -eq 0)
        } catch { $script:Aarch64Available = $false }
    } else {
        if (-not (Test-WslAvailable)) { $script:Aarch64Available = $false; return $false }
        try {
            $out = wsl bash -c "command -v aarch64-linux-gnu-gcc" 2>&1
            $script:Aarch64Available = ($LASTEXITCODE -eq 0)
        } catch { $script:Aarch64Available = $false }
    }
    return $script:Aarch64Available
}

$script:RiscvAvailable = $null
function Test-RiscvCompiler {
    if ($null -ne $script:RiscvAvailable) { return $script:RiscvAvailable }
    if ($RunningOnLinux) {
        try {
            bash -c "command -v riscv64-linux-gnu-gcc" 2>&1 | Out-Null
            $script:RiscvAvailable = ($LASTEXITCODE -eq 0)
        } catch { $script:RiscvAvailable = $false }
    } else {
        if (-not (Test-WslAvailable)) { $script:RiscvAvailable = $false; return $false }
        try {
            $out = wsl bash -c "command -v riscv64-linux-gnu-gcc" 2>&1
            $script:RiscvAvailable = ($LASTEXITCODE -eq 0)
        } catch { $script:RiscvAvailable = $false }
    }
    return $script:RiscvAvailable
}

$script:WasiAvailable = $null
function Test-WasiToolchain {
    if ($null -ne $script:WasiAvailable) { return $script:WasiAvailable }
    $check = 'command -v clang >/dev/null 2>&1 && command -v wasm-ld >/dev/null 2>&1 && command -v wasmtime >/dev/null 2>&1 && command -v wasm-validate >/dev/null 2>&1 && command -v wasm-objdump >/dev/null 2>&1 && test -d /usr/include/wasm32-wasi && test -f "$(clang -print-resource-dir)/lib/wasi/libclang_rt.builtins-wasm32.a"'
    if ($RunningOnLinux) {
        try {
            bash -c $check 2>&1 | Out-Null
            $script:WasiAvailable = ($LASTEXITCODE -eq 0)
        } catch { $script:WasiAvailable = $false }
    } else {
        if (-not (Test-WslAvailable)) { $script:WasiAvailable = $false; return $false }
        try {
            $out = wsl bash -c $check 2>&1
            $script:WasiAvailable = ($LASTEXITCODE -eq 0)
        } catch { $script:WasiAvailable = $false }
    }
    return $script:WasiAvailable
}

# --- QEMU emulator checks (cached) ---
$script:QemuArmAvailable = $null
function Test-QemuArm {
    if ($null -ne $script:QemuArmAvailable) { return $script:QemuArmAvailable }
    if ($RunningOnLinux) {
        try {
            bash -c "which qemu-arm" 2>&1 | Out-Null
            $script:QemuArmAvailable = ($LASTEXITCODE -eq 0)
        } catch { $script:QemuArmAvailable = $false }
    } else {
        if (-not (Test-WslAvailable)) { $script:QemuArmAvailable = $false; return $false }
        try {
            $out = wsl bash -c "which qemu-arm" 2>&1
            $script:QemuArmAvailable = ($LASTEXITCODE -eq 0)
        } catch { $script:QemuArmAvailable = $false }
    }
    return $script:QemuArmAvailable
}

$script:QemuAarch64Available = $null
function Test-QemuAarch64 {
    if ($null -ne $script:QemuAarch64Available) { return $script:QemuAarch64Available }
    if ($RunningOnLinux) {
        try {
            bash -c "which qemu-aarch64" 2>&1 | Out-Null
            $script:QemuAarch64Available = ($LASTEXITCODE -eq 0)
        } catch { $script:QemuAarch64Available = $false }
    } else {
        if (-not (Test-WslAvailable)) { $script:QemuAarch64Available = $false; return $false }
        try {
            $out = wsl bash -c "which qemu-aarch64" 2>&1
            $script:QemuAarch64Available = ($LASTEXITCODE -eq 0)
        } catch { $script:QemuAarch64Available = $false }
    }
    return $script:QemuAarch64Available
}

$script:QemuRiscvAvailable = $null
function Test-QemuRiscv {
    if ($null -ne $script:QemuRiscvAvailable) { return $script:QemuRiscvAvailable }
    if ($RunningOnLinux) {
        try {
            bash -c "which qemu-riscv64" 2>&1 | Out-Null
            $script:QemuRiscvAvailable = ($LASTEXITCODE -eq 0)
        } catch { $script:QemuRiscvAvailable = $false }
    } else {
        if (-not (Test-WslAvailable)) { $script:QemuRiscvAvailable = $false; return $false }
        try {
            $out = wsl bash -c "which qemu-riscv64" 2>&1
            $script:QemuRiscvAvailable = ($LASTEXITCODE -eq 0)
        } catch { $script:QemuRiscvAvailable = $false }
    }
    return $script:QemuRiscvAvailable
}

# --- Run a step, report result ---
function Invoke-Step {
    param(
        [string]$TargetName,
        [string]$ActionName,
        [scriptblock]$Block
    )

    # Safety-net patterns: if any of these appear in test output, treat as FAIL
    # regardless of exit code (catches cases where shell loops swallow failures)
    $FailurePatterns = @('Segmentation fault', 'core dumped', 'FAILED', 'SOME TESTS FAILED', 'SOME ARM TESTS FAILED', 'SOME AARCH64 TESTS FAILED', 'SOME RISC-V TESTS FAILED', 'SOME WASI TESTS FAILED')

    if ($ShowAll) {
        # Verbose: capture + stream, then check for failure indicators
        Write-Header "$TargetName $ActionName"
        try {
            $output = & $Block 2>&1
            $output | ForEach-Object { Write-Host $_ }
            $exitFail = ($LASTEXITCODE -and $LASTEXITCODE -ne 0)
            $outputStr = ($output | Out-String)
            $contentFail = $false
            foreach ($pat in $FailurePatterns) {
                if ($outputStr -match [regex]::Escape($pat)) { $contentFail = $true; break }
            }
            if ($exitFail -or $contentFail) {
                Write-Host "FAIL" -ForegroundColor Red
                Add-Result $TargetName $ActionName "FAIL"
                return
            }
            Write-Host "PASS" -ForegroundColor Green
            Add-Result $TargetName $ActionName "PASS"
            if ($ActionName -eq 'Verify') {
                Collect-BinarySizes $TargetName $outputStr
                Emit-BinarySizeMarkers $TargetName
            }
        } catch {
            Write-Host "FAIL: $_" -ForegroundColor Red
            Add-Result $TargetName $ActionName "FAIL"
        }
    } else {
        # Concise: capture output, show one-liner with stats, dump output only on failure
        $label = switch ($ActionName) {
            'Build'  { "Building $TargetName" }
            'Test'   { "Testing $TargetName" }
            'Verify' { "Verifying $TargetName" }
            default  { "$TargetName $ActionName" }
        }
        $paddedLabel = "$label...".PadRight(28)
        Write-Host -NoNewline "  $paddedLabel "
        try {
            $output = & $Block 2>&1
            $exitFail = ($LASTEXITCODE -and $LASTEXITCODE -ne 0)
            $outputStr = ($output | Out-String)
            $contentFail = $false
            foreach ($pat in $FailurePatterns) {
                if ($outputStr -match [regex]::Escape($pat)) { $contentFail = $true; break }
            }
            if ($exitFail -or $contentFail) {
                Write-Host "FAIL" -ForegroundColor Red
                $output | ForEach-Object { Write-Host "    $_" }
                Add-Result $TargetName $ActionName "FAIL"
                return
            }
            $summary = Get-StepSummary $TargetName $ActionName $outputStr
            if ($summary) {
                Write-Host -NoNewline "$summary  "
            }
            Write-Host "PASS" -ForegroundColor Green
            Add-Result $TargetName $ActionName "PASS"
            if ($ActionName -eq 'Verify') {
                Emit-BinarySizeMarkers $TargetName
            }
        } catch {
            Write-Host "FAIL: $_" -ForegroundColor Red
            Add-Result $TargetName $ActionName "FAIL"
        }
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

# --- CRLF fix for WSL builds (Windows only) ---
# Windows checkouts have CRLF; gcc and bash in WSL choke on \r in
# macro continuations, string literals, and GCC statement expressions.
# Do the conversion in pure PowerShell — avoids quoting issues with
# sed through PowerShell → WSL → bash, and avoids sed -i reliability
# problems on /mnt/c/ (NTFS-via-WSL) filesystems.
# On Linux, files already have LF — skip entirely.
$script:CrlfFixed = $false
function Invoke-CrlfFix {
    if ($RunningOnLinux) { return }  # Not needed on native Linux
    if ($SubProcess) { return }      # Parent already did CRLF fix
    if ($script:CrlfFixed) { return }
    if ($ShowAll) { Write-Host "  Stripping CRLF for WSL..." -ForegroundColor DarkGray }
    $targets = Get-ChildItem -Path $RepoRoot -Include '*.c','*.h' -Recurse -File
    $taskfile = Join-Path $RepoRoot 'Taskfile'
    $showcaseSmokeScript = Join-Path $RepoRoot 'tests\smoke\showcase_smoke.sh'
    $showcaseSmokeDir = Join-Path $RepoRoot 'tests\fixtures\showcase_smoke'
    if (Test-Path $taskfile) { $targets += Get-Item $taskfile }
    if (Test-Path $showcaseSmokeScript) { $targets += Get-Item $showcaseSmokeScript }
    if (Test-Path $showcaseSmokeDir) { $targets += Get-ChildItem -Path $showcaseSmokeDir -Recurse -File }
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
#  Linux actions — native on Linux, via WSL on Windows
# ============================================================
function Invoke-LinuxBuildWith {
    param([string]$CC, [string]$Label)
    Invoke-Step $Label "Build" {
        if ($RunningOnLinux) {
            bash -c "cd '$RepoRoot' && git submodule update --init --quiet 2>/dev/null; ./Taskfile build $CC $OptLevel 2>&1"
        } else {
            Invoke-CrlfFix
            wsl bash -c "cd '$WslRepoRoot' && git submodule update --init --quiet 2>/dev/null; ./Taskfile build $CC $OptLevel 2>&1"
        }
    }
}

function Invoke-LinuxTestWith {
    param([string]$CC, [string]$Label)
    Invoke-Step $Label "Test" {
        if ($RunningOnLinux) {
            bash -c "cd '$RepoRoot' && ./Taskfile test $CC $OptLevel 2>&1"
        } else {
            Invoke-CrlfFix
            wsl bash -c "cd '$WslRepoRoot' && ./Taskfile test $CC $OptLevel 2>&1"
        }
    }
}

function Invoke-LinuxVerifyWith {
    param([string]$CC, [string]$Label)
    Invoke-Step $Label "Verify" {
        if ($RunningOnLinux) {
            bash -c "cd '$RepoRoot' && ./Taskfile verify $CC $OptLevel 2>&1"
        } else {
            Invoke-CrlfFix
            wsl bash -c "cd '$WslRepoRoot' && ./Taskfile verify $CC $OptLevel 2>&1"
        }
    }
}

# ============================================================
#  ARM actions — native on Linux, via WSL + QEMU on Windows
# ============================================================
function Invoke-ArmBuildWith {
    param([string]$CC, [string]$Label)
    Invoke-Step $Label "Build" {
        if ($RunningOnLinux) {
            bash -c "cd '$RepoRoot' && git submodule update --init --quiet 2>/dev/null; ./Taskfile build_arm $CC $OptLevel 2>&1"
        } else {
            Invoke-CrlfFix
            wsl bash -c "cd '$WslRepoRoot' && git submodule update --init --quiet 2>/dev/null; ./Taskfile build_arm $CC $OptLevel 2>&1"
        }
    }
}

# ============================================================
#  AArch64 actions — native on Linux, via WSL + QEMU on Windows
# ============================================================
function Invoke-Aarch64BuildWith {
    param([string]$CC, [string]$Label)
    Invoke-Step $Label "Build" {
        if ($RunningOnLinux) {
            bash -c "cd '$RepoRoot' && git submodule update --init --quiet 2>/dev/null; ./Taskfile build_aarch64 $CC $OptLevel 2>&1"
        } else {
            Invoke-CrlfFix
            wsl bash -c "cd '$WslRepoRoot' && git submodule update --init --quiet 2>/dev/null; ./Taskfile build_aarch64 $CC $OptLevel 2>&1"
        }
    }
}

function Invoke-Aarch64TestWith {
    param([string]$CC, [string]$Label)
    if (-not (Test-QemuAarch64)) {
        $hint = if ($RunningOnLinux) { "sudo apt-get install qemu-user" } else { "sudo apt-get install qemu-user (in WSL)" }
        Write-Host "SKIP [install qemu-user: $hint]" -ForegroundColor Yellow
        Add-Result $Label "Test" "SKIP"
        return
    }
    Invoke-Step $Label "Test" {
        if ($RunningOnLinux) {
            bash -c "cd '$RepoRoot' && ./Taskfile test_aarch64 $CC $OptLevel 2>&1"
        } else {
            Invoke-CrlfFix
            wsl bash -c "cd '$WslRepoRoot' && ./Taskfile test_aarch64 $CC $OptLevel 2>&1"
        }
    }
}

function Invoke-Aarch64VerifyWith {
    param([string]$CC, [string]$Label)
    Invoke-Step $Label "Verify" {
        if ($RunningOnLinux) {
            bash -c "cd '$RepoRoot' && ./Taskfile verify_aarch64 $CC $OptLevel 2>&1"
        } else {
            Invoke-CrlfFix
            wsl bash -c "cd '$WslRepoRoot' && ./Taskfile verify_aarch64 $CC $OptLevel 2>&1"
        }
    }
}

# ============================================================
#  RISC-V actions — native on Linux, via WSL + QEMU on Windows
# ============================================================
function Invoke-RiscvBuildWith {
    param([string]$CC, [string]$Label)
    Invoke-Step $Label "Build" {
        if ($RunningOnLinux) {
            bash -c "cd '$RepoRoot' && ./Taskfile build_riscv $CC $OptLevel 2>&1"
        } else {
            Invoke-CrlfFix
            wsl bash -c "cd '$WslRepoRoot' && ./Taskfile build_riscv $CC $OptLevel 2>&1"
        }
    }
}

function Invoke-RiscvTestWith {
    param([string]$CC, [string]$Label)
    if (-not (Test-QemuRiscv)) {
        $hint = if ($RunningOnLinux) { "sudo apt-get install qemu-user" } else { "sudo apt-get install qemu-user (in WSL)" }
        Write-Host "SKIP [install qemu-user: $hint]" -ForegroundColor Yellow
        Add-Result $Label "Test" "SKIP"
        return
    }
    Invoke-Step $Label "Test" {
        if ($RunningOnLinux) {
            bash -c "cd '$RepoRoot' && ./Taskfile test_riscv $CC $OptLevel 2>&1"
        } else {
            Invoke-CrlfFix
            wsl bash -c "cd '$WslRepoRoot' && ./Taskfile test_riscv $CC $OptLevel 2>&1"
        }
    }
}

function Invoke-RiscvVerifyWith {
    param([string]$CC, [string]$Label)
    Invoke-Step $Label "Verify" {
        if ($RunningOnLinux) {
            bash -c "cd '$RepoRoot' && ./Taskfile verify_riscv $CC $OptLevel 2>&1"
        } else {
            Invoke-CrlfFix
            wsl bash -c "cd '$WslRepoRoot' && ./Taskfile verify_riscv $CC $OptLevel 2>&1"
        }
    }
}

# ============================================================
#  WASI actions — native on Linux, via WSL on Windows
# ============================================================
function Invoke-WasiBuild {
    Invoke-Step "WASI" "Build" {
        if ($RunningOnLinux) {
            bash -c "cd '$RepoRoot' && ./Taskfile build_wasi clang $OptLevel 2>&1"
        } else {
            Invoke-CrlfFix
            wsl bash -c "cd '$WslRepoRoot' && ./Taskfile build_wasi clang $OptLevel 2>&1"
        }
    }
}

function Invoke-WasiTest {
    Invoke-Step "WASI" "Test" {
        if ($RunningOnLinux) {
            bash -c "cd '$RepoRoot' && ./Taskfile test_wasi clang $OptLevel 2>&1"
        } else {
            Invoke-CrlfFix
            wsl bash -c "cd '$WslRepoRoot' && ./Taskfile test_wasi clang $OptLevel 2>&1"
        }
    }
}

function Invoke-WasiVerify {
    Invoke-Step "WASI" "Verify" {
        if ($RunningOnLinux) {
            bash -c "cd '$RepoRoot' && ./Taskfile verify_wasi clang $OptLevel 2>&1"
        } else {
            Invoke-CrlfFix
            wsl bash -c "cd '$WslRepoRoot' && ./Taskfile verify_wasi clang $OptLevel 2>&1"
        }
    }
}

function Invoke-ArmTestWith {
    param([string]$CC, [string]$Label)
    if (-not (Test-QemuArm)) {
        $hint = if ($RunningOnLinux) { "sudo apt-get install qemu-user" } else { "sudo apt-get install qemu-user (in WSL)" }
        Write-Host "SKIP [install qemu-user: $hint]" -ForegroundColor Yellow
        Add-Result $Label "Test" "SKIP"
        return
    }
    Invoke-Step $Label "Test" {
        if ($RunningOnLinux) {
            bash -c "cd '$RepoRoot' && ./Taskfile test_arm $CC $OptLevel 2>&1"
        } else {
            Invoke-CrlfFix
            wsl bash -c "cd '$WslRepoRoot' && ./Taskfile test_arm $CC $OptLevel 2>&1"
        }
    }
}

function Invoke-ArmVerifyWith {
    param([string]$CC, [string]$Label)
    Invoke-Step $Label "Verify" {
        if ($RunningOnLinux) {
            bash -c "cd '$RepoRoot' && ./Taskfile verify_arm $CC $OptLevel 2>&1"
        } else {
            Invoke-CrlfFix
            wsl bash -c "cd '$WslRepoRoot' && ./Taskfile verify_arm $CC $OptLevel 2>&1"
        }
    }
}

# ============================================================
#  Dispatch
# ============================================================
function Get-LinuxCompilers {
    if ($Compiler -eq 'all') { return @('gcc', 'clang') }
    return @($Compiler)
}

function Get-ArmCompilers {
    if ($Compiler -eq 'all') { return @('gcc', 'clang') }
    # When a specific compiler is requested, map to the ARM variant
    return @($Compiler)
}

function Invoke-Target {
    param([string]$T, [string]$A)

    switch ($T) {
        'windows' {
            if ($RunningOnLinux) {
                Write-Host "SKIP: Windows target not available on Linux." -ForegroundColor Yellow
                $actions = if ($A -eq 'all') { @('build', 'test', 'verify') } else { @($A) }
                foreach ($act in $actions) { Add-Result "Windows" $act "SKIP" }
                return
            }
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
            if ($RunningOnWindows -and -not (Test-WslAvailable)) {
                Write-Host "SKIP: WSL not available -- skipping Linux target." -ForegroundColor Yellow
                foreach ($cc in (Get-LinuxCompilers)) {
                    $label = "Linux ($cc)"
                    $actions = if ($A -eq 'all') { @('build', 'test', 'verify') } else { @($A) }
                    foreach ($act in $actions) { Add-Result $label $act "SKIP" }
                }
                return
            }
            foreach ($cc in (Get-LinuxCompilers)) {
                $label = "Linux ($cc)"
                $actions = if ($A -eq 'all') { @('build', 'test', 'verify') } else { @($A) }
                foreach ($act in $actions) {
                    switch ($act) {
                        'build'  { Invoke-LinuxBuildWith  $cc $label }
                        'test'   { Invoke-LinuxTestWith   $cc $label }
                        'verify' { Invoke-LinuxVerifyWith $cc $label }
                    }
                }
            }
        }
        'arm' {
            if ($RunningOnWindows -and -not (Test-WslAvailable)) {
                Write-Host "SKIP: WSL not available -- skipping ARM target." -ForegroundColor Yellow
                foreach ($cc in (Get-ArmCompilers)) {
                    $labels = @("ARM ($cc)", "AArch64 ($cc)")
                    $actions = if ($A -eq 'all') { @('build', 'test', 'verify') } else { @($A) }
                    foreach ($label in $labels) {
                        foreach ($act in $actions) { Add-Result $label $act "SKIP" }
                    }
                }
                return
            }
            if (-not (Test-ArmCompiler)) {
                $hint = if ($RunningOnLinux) { "sudo apt-get install gcc-arm-linux-gnueabihf" } else { "install in WSL" }
                Write-Host "SKIP: arm-linux-gnueabihf-gcc not found -- skipping ARM32 target. ($hint)" -ForegroundColor Yellow
                foreach ($cc in (Get-ArmCompilers)) {
                    $label = "ARM ($cc)"
                    $actions = if ($A -eq 'all') { @('build', 'test', 'verify') } else { @($A) }
                    foreach ($act in $actions) { Add-Result $label $act "SKIP" }
                }
            } else {
                foreach ($cc in (Get-ArmCompilers)) {
                    $armCC = if ($cc -eq 'gcc') { 'arm-linux-gnueabihf-gcc' } else { 'clang' }
                    $label = "ARM ($cc)"
                    $actions = if ($A -eq 'all') { @('build', 'test', 'verify') } else { @($A) }
                    foreach ($act in $actions) {
                        switch ($act) {
                            'build'  { Invoke-ArmBuildWith  $armCC $label }
                            'test'   { Invoke-ArmTestWith   $armCC $label }
                            'verify' { Invoke-ArmVerifyWith $armCC $label }
                        }
                    }
                }
            }

            if (-not (Test-Aarch64Compiler)) {
                $hint = if ($RunningOnLinux) { "sudo apt-get install gcc-aarch64-linux-gnu" } else { "install in WSL" }
                Write-Host "SKIP: aarch64-linux-gnu-gcc not found -- skipping AArch64 target. ($hint)" -ForegroundColor Yellow
                foreach ($cc in (Get-ArmCompilers)) {
                    $label = "AArch64 ($cc)"
                    $actions = if ($A -eq 'all') { @('build', 'test', 'verify') } else { @($A) }
                    foreach ($act in $actions) { Add-Result $label $act "SKIP" }
                }
            } else {
                foreach ($cc in (Get-ArmCompilers)) {
                    $aarch64CC = if ($cc -eq 'gcc') { 'aarch64-linux-gnu-gcc' } else { 'clang' }
                    $label = "AArch64 ($cc)"
                    $actions = if ($A -eq 'all') { @('build', 'test', 'verify') } else { @($A) }
                    foreach ($act in $actions) {
                        switch ($act) {
                            'build'  { Invoke-Aarch64BuildWith  $aarch64CC $label }
                            'test'   { Invoke-Aarch64TestWith   $aarch64CC $label }
                            'verify' { Invoke-Aarch64VerifyWith $aarch64CC $label }
                        }
                    }
                }
            }
        }
        'riscv' {
            if ($RunningOnWindows -and -not (Test-WslAvailable)) {
                Write-Host "SKIP: WSL not available -- skipping RISC-V target." -ForegroundColor Yellow
                foreach ($cc in (Get-ArmCompilers)) {
                    $label = "RISC-V ($cc)"
                    $actions = if ($A -eq 'all') { @('build', 'test', 'verify') } else { @($A) }
                    foreach ($act in $actions) { Add-Result $label $act "SKIP" }
                }
                return
            }
            if (-not (Test-RiscvCompiler)) {
                $hint = if ($RunningOnLinux) { "sudo apt-get install gcc-riscv64-linux-gnu" } else { "install in WSL" }
                Write-Host "SKIP: riscv64-linux-gnu-gcc not found -- skipping RISC-V target. ($hint)" -ForegroundColor Yellow
                foreach ($cc in (Get-ArmCompilers)) {
                    $label = "RISC-V ($cc)"
                    $actions = if ($A -eq 'all') { @('build', 'test', 'verify') } else { @($A) }
                    foreach ($act in $actions) { Add-Result $label $act "SKIP" }
                }
            } else {
                foreach ($cc in (Get-ArmCompilers)) {
                    $riscvCC = if ($cc -eq 'gcc') { 'riscv64-linux-gnu-gcc' } else { 'clang' }
                    $label = "RISC-V ($cc)"
                    $actions = if ($A -eq 'all') { @('build', 'test', 'verify') } else { @($A) }
                    foreach ($act in $actions) {
                        switch ($act) {
                            'build'  { Invoke-RiscvBuildWith  $riscvCC $label }
                            'test'   { Invoke-RiscvTestWith   $riscvCC $label }
                            'verify' { Invoke-RiscvVerifyWith $riscvCC $label }
                        }
                    }
                }
            }
        }
        'wasi' {
            if ($RunningOnWindows -and -not (Test-WslAvailable)) {
                Write-Host "SKIP: WSL not available -- skipping WASI target." -ForegroundColor Yellow
                $actions = if ($A -eq 'all') { @('build', 'test', 'verify') } else { @($A) }
                foreach ($act in $actions) { Add-Result 'WASI' $act 'SKIP' }
                return
            }
            if (-not (Test-WasiToolchain)) {
                $hint = if ($RunningOnLinux) { "install clang, lld, libclang-rt-dev-wasm32, wasi-libc, wabt, and wasmtime" } else { "install clang, lld, libclang-rt-dev-wasm32, wasi-libc, wabt, and wasmtime in WSL" }
                Write-Host "SKIP: WASI toolchain/runtime not found -- skipping WASI target. ($hint)" -ForegroundColor Yellow
                $actions = if ($A -eq 'all') { @('build', 'test', 'verify') } else { @($A) }
                foreach ($act in $actions) { Add-Result 'WASI' $act 'SKIP' }
            } else {
                $actions = if ($A -eq 'all') { @('build', 'test', 'verify') } else { @($A) }
                foreach ($act in $actions) {
                    switch ($act) {
                        'build'  { Invoke-WasiBuild  }
                        'test'   { Invoke-WasiTest   }
                        'verify' { Invoke-WasiVerify }
                    }
                }
            }
        }
    }
}

# --- Main execution ---
$targets = if ($Target -eq 'all') { @('windows', 'linux', 'arm', 'riscv', 'wasi') } else { @($Target) }

$windowsTargets = @($targets | Where-Object { $_ -eq 'windows' })
$wslTargets = @($targets | Where-Object { $_ -ne 'windows' })

# Windows targets run sequentially (native, fast)
foreach ($t in $windowsTargets) {
    Invoke-Target $t $Action
}

# WSL targets: run in parallel when multiple targets on Windows
if ($wslTargets.Count -gt 1 -and $RunningOnWindows -and -not $SubProcess) {
    # CRLF fix once before any WSL work
    Invoke-CrlfFix

    $wslProcs = @()
    foreach ($t in $wslTargets) {
        $logFile = Join-Path $RepoRoot "bin\.ci_${t}.log"
        $showAllStr = if ($ShowAll) { ' -ShowAll' } else { '' }

        # Build sub-process command; use -EncodedCommand to avoid quoting issues
        $psCmd = @"
`$ErrorActionPreference = 'Continue'
Start-Transcript -Path '$logFile' -Force | Out-Null
& '$PSCommandPath' -Target '$t' -Action '$Action' -Compiler '$Compiler' -OptLevel '$OptLevel' -SubProcess$showAllStr
`$rc = `$LASTEXITCODE
Stop-Transcript | Out-Null
exit `$rc
"@
        $bytes = [System.Text.Encoding]::Unicode.GetBytes($psCmd)
        $encoded = [Convert]::ToBase64String($bytes)

        $psi = [System.Diagnostics.ProcessStartInfo]::new()
        $psi.FileName = 'powershell'
        $psi.Arguments = "-NoProfile -ExecutionPolicy Bypass -EncodedCommand $encoded"
        $psi.UseShellExecute = $false
        $psi.CreateNoWindow = $true
        $psi.WorkingDirectory = $RepoRoot

        $proc = [System.Diagnostics.Process]::Start($psi)
        $wslProcs += @{ Target = $t; Proc = $proc; LogFile = $logFile }
    }

    # Wait for all WSL sub-processes and collect results
    foreach ($wp in $wslProcs) {
        $wp.Proc.WaitForExit()
        $exitCode = $wp.Proc.ExitCode

        if (Test-Path $wp.LogFile) {
            $rawLines = @(Get-Content $wp.LogFile)

            # Strip transcript header/footer (delimited by **** lines)
            $starIndices = @()
            for ($i = 0; $i -lt $rawLines.Count; $i++) {
                if ($rawLines[$i] -match '^\*{4,}') { $starIndices += $i }
            }
            if ($starIndices.Count -ge 4) {
                $bodyStart = $starIndices[1] + 1
                $bodyEnd = $starIndices[$starIndices.Count - 2] - 1
                $bodyLines = @($rawLines[$bodyStart..$bodyEnd])
            } else {
                $bodyLines = $rawLines
            }

            # Display output, parse structured markers, reconstruct one-liners
            $lineIdx = 0
            while ($lineIdx -lt $bodyLines.Count) {
                $line = $bodyLines[$lineIdx]
                if ($line -match '^##CI_RESULT##(.+?)##(.+?)##(.+?)##$') {
                    Add-Result $Matches[1] $Matches[2] $Matches[3]
                    $lineIdx++
                } elseif ($line -match '^##CI_SIZES##(.+?)##(.+?)##$') {
                    $sizesTarget = $Matches[1]
                    $sizesPairs = $Matches[2] -split ','
                    $sizes = @{}
                    foreach ($pair in $sizesPairs) {
                        $parts = $pair -split ':'
                        if ($parts.Count -eq 2) {
                            $sizes[$parts[0]] = [int64]$parts[1]
                        }
                    }
                    if ($sizes.Count -gt 0) {
                        $script:BinarySizes[$sizesTarget] = $sizes
                    }
                    $lineIdx++
                } elseif ($line -match '^\s+(Building|Testing|Verifying)\s') {
                    # Accumulate concise step lines into a single one-liner
                    $oneLiner = $line.TrimEnd()
                    $lineIdx++
                    while ($lineIdx -lt $bodyLines.Count) {
                        $nextLine = $bodyLines[$lineIdx]
                        if ($nextLine -match '^##CI_') { break }
                        if ($nextLine -match '^\s+(Building|Testing|Verifying)\s') { break }
                        $trimmed = $nextLine.Trim()
                        if ($trimmed -ne '') {
                            $oneLiner += "  $trimmed"
                        }
                        $lineIdx++
                        if ($trimmed -match '\b(PASS|FAIL)\b') { break }
                    }
                    Write-Host $oneLiner
                } else {
                    Write-Host $line
                    $lineIdx++
                }
            }

            Remove-Item $wp.LogFile -ErrorAction SilentlyContinue
        }

        # If sub-process failed and no results were parsed, add a generic FAIL
        if ($exitCode -ne 0) {
            $targetResults = @($Results | Where-Object { $_.Target -match [regex]::Escape($wp.Target) })
            if ($targetResults.Count -eq 0) {
                Add-Result $wp.Target "All" "FAIL"
            }
        }
    }
} else {
    # Sequential: single WSL target, running on Linux, or sub-process mode
    foreach ($t in $wslTargets) {
        Invoke-Target $t $Action
    }
}

# ============================================================
#  Binary size comparison table
# ============================================================
if (-not $SubProcess) {
    Write-BinarySizeTable
    Write-BinarySizeJson
    Write-BinarySizeMarkdownSummary
}

# ============================================================
#  Documentation regeneration
# ============================================================
if (-not $SubProcess) {
    Write-Host ""
    Write-Host "  Regenerating docs..." -NoNewline
    try {
        & (Join-Path $PSScriptRoot 'gen-docs.ps1')
        $Results += [PSCustomObject]@{ Target = 'Docs'; Action = 'Generate'; Status = 'PASS' }
    } catch {
        Write-Host "  FAILED: $_" -ForegroundColor Red
        $Results += [PSCustomObject]@{ Target = 'Docs'; Action = 'Generate'; Status = 'FAIL' }
    }
}

# ============================================================
#  Summary
# ============================================================
if (-not $SubProcess) {
    Write-Host ""
    Write-Host "=== Summary ===" -ForegroundColor Cyan
    Write-Host ("{0,-16} {1,-10} {2}" -f "Target", "Action", "Result")
    Write-Host ("{0,-16} {1,-10} {2}" -f "------", "------", "------")
    foreach ($r in $Results) {
        $color = switch ($r.Status) {
            'PASS' { 'Green'  }
            'FAIL' { 'Red'    }
            'SKIP' { 'Yellow' }
        }
        Write-Host ("{0,-16} {1,-10} {2}" -f $r.Target, $r.Action, $r.Status) -ForegroundColor $color
    }
}

Pop-Location

# Exit code: 1 if any FAIL
$failed = $Results | Where-Object { $_.Status -eq 'FAIL' }
if ($failed) { exit 1 } else { exit 0 }
