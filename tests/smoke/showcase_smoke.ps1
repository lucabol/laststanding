Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$TestsDir = Split-Path $PSScriptRoot -Parent
$RepoRoot = Split-Path $TestsDir -Parent
$FixtureDir = Join-Path $TestsDir 'fixtures\showcase_smoke'
$BinDir = Join-Path $RepoRoot 'bin'
$TempDir = Join-Path ([System.IO.Path]::GetTempPath()) ("laststanding-showcase-smoke-" + [guid]::NewGuid().ToString('N'))

function Test-ByteEqual {
    param(
        [string]$ActualPath,
        [string]$ExpectedPath
    )

    $actual = [System.IO.File]::ReadAllBytes($ActualPath)
    $expected = [System.IO.File]::ReadAllBytes($ExpectedPath)
    if ($actual.Length -ne $expected.Length) { return $false }
    for ($i = 0; $i -lt $actual.Length; $i++) {
        if ($actual[$i] -ne $expected[$i]) { return $false }
    }
    return $true
}

function Show-Bytes {
    param([string]$Path)
    $bytes = [System.IO.File]::ReadAllBytes($Path)
    if ($bytes.Length -eq 0) {
        Write-Host "      [empty]"
        return
    }
    for ($i = 0; $i -lt $bytes.Length; $i += 16) {
        $count = [Math]::Min(16, $bytes.Length - $i)
        $chunk = New-Object byte[] $count
        [Array]::Copy($bytes, $i, $chunk, 0, $count)
        Write-Host ("      " + ([BitConverter]::ToString($chunk)))
    }
}

function Invoke-Program {
    param(
        [string]$OutPath,
        [string]$ExeName,
        [string[]]$Arguments = @(),
        [hashtable]$Environment = @{},
        [string]$WorkingDirectory = $FixtureDir
    )

    $psi = New-Object System.Diagnostics.ProcessStartInfo
    $psi.FileName = Join-Path $BinDir "$ExeName.exe"
    $psi.WorkingDirectory = $WorkingDirectory
    $psi.UseShellExecute = $false
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true
    $psi.Arguments = ($Arguments | ForEach-Object {
        if ($_ -match '\s|"') {
            '"' + $_.Replace('"', '\"') + '"'
        } else {
            $_
        }
    }) -join ' '

    foreach ($name in $Environment.Keys) {
        $psi.EnvironmentVariables[$name] = [string]$Environment[$name]
    }

    $process = [System.Diagnostics.Process]::Start($psi)
    try {
        $stream = [System.IO.File]::Create($OutPath)
        try {
            $process.StandardOutput.BaseStream.CopyTo($stream)
        }
        finally {
            $stream.Dispose()
        }
        $stderr = $process.StandardError.ReadToEnd()
        $process.WaitForExit()
        return @{
            ExitCode = $process.ExitCode
            Stderr = $stderr
        }
    }
    finally {
        $process.Dispose()
    }
}

function Invoke-Check {
    param(
        [string]$Name,
        [string]$ExpectedFile,
        [int]$ExpectedExit,
        [string]$ExeName,
        [string[]]$Arguments = @(),
        [hashtable]$Environment = @{}
    )

    $safeName = [regex]::Replace($Name, '[^A-Za-z0-9_.-]', '_')
    $out = Join-Path $TempDir "$safeName.out"
    $expectedPath = Join-Path $FixtureDir $ExpectedFile

    Write-Host "--- Running $Name ---"
    $result = Invoke-Program -OutPath $out -ExeName $ExeName -Arguments $Arguments -Environment $Environment
    $status = $result.ExitCode
    if ($status -ne $ExpectedExit) {
        if ($result.Stderr) {
            Write-Host $result.Stderr.TrimEnd() -ForegroundColor Yellow
        }
        throw "FAIL: $Name exited with $status (expected $ExpectedExit)"
    }

    if (-not (Test-ByteEqual $out $expectedPath)) {
        if ($result.Stderr) {
            Write-Host $result.Stderr.TrimEnd() -ForegroundColor Yellow
        }
        Write-Host "Expected bytes:" -ForegroundColor Yellow
        Show-Bytes $expectedPath
        Write-Host "Actual bytes:" -ForegroundColor Yellow
        Show-Bytes $out
        throw "FAIL: $Name output mismatch"
    }
}

function Invoke-EnvCheck {
    param(
        [string]$Name,
        [string]$ExpectedFile,
        [int]$ExpectedExit,
        [string]$EnvName,
        [string]$EnvValue,
        [string]$ExeName,
        [string[]]$Arguments = @()
    )

    Invoke-Check $Name $ExpectedFile $ExpectedExit $ExeName $Arguments @{ $EnvName = $EnvValue }
}

function Write-CombinedFile {
    param(
        [string]$OutPath,
        [string[]]$Parts
    )

    $stream = [System.IO.File]::Create($OutPath)
    try {
        foreach ($part in $Parts) {
            $bytes = [System.IO.File]::ReadAllBytes((Join-Path $FixtureDir $part))
            $stream.Write($bytes, 0, $bytes.Length)
        }
    }
    finally {
        $stream.Dispose()
    }
}

function Invoke-ShellRedirCheck {
    param(
        [string]$Name,
        [int]$ExpectedExit,
        [string]$ActualFile,
        [string[]]$InputFiles,
        [string[]]$ExpectedFiles,
        [string]$InputText
    )

    $safeName = [regex]::Replace($Name, '[^A-Za-z0-9_.-]', '_')
    $workDir = Join-Path $TempDir "$safeName.work"
    $stdoutPath = Join-Path $workDir 'shell.stdout'
    $stderrPath = Join-Path $workDir 'shell.stderr'
    $expectedPath = Join-Path $TempDir "$safeName.expected"
    $scriptPath = Join-Path $workDir 'script.in'

    New-Item -ItemType Directory -Path $workDir | Out-Null
    foreach ($file in $InputFiles) {
        Copy-Item (Join-Path $FixtureDir $file) -Destination $workDir
    }
    Write-CombinedFile -OutPath $expectedPath -Parts $ExpectedFiles
    [System.IO.File]::WriteAllBytes($scriptPath, ([System.Text.UTF8Encoding]::new($false)).GetBytes($InputText))

    $pathValue = if ($env:PATH) { "$BinDir;$env:PATH" } else { $BinDir }

    Write-Host "--- Running $Name ---"
    $psi = New-Object System.Diagnostics.ProcessStartInfo
    $psi.FileName = if ($env:ComSpec) { $env:ComSpec } else { 'cmd.exe' }
    $psi.WorkingDirectory = $workDir
    $psi.UseShellExecute = $false
    $psi.EnvironmentVariables['PATH'] = $pathValue
    $psi.Arguments = '/d /c sh.exe < script.in > shell.stdout 2> shell.stderr'

    $process = [System.Diagnostics.Process]::Start($psi)
    try {
        $process.WaitForExit()
        $status = $process.ExitCode
    }
    finally {
        $process.Dispose()
    }

    $stderr = if (Test-Path $stderrPath) { Get-Content -Raw $stderrPath } else { '' }

    if ($status -ne $ExpectedExit) {
        if ($stderr) {
            Write-Host $stderr.TrimEnd() -ForegroundColor Yellow
        }
        if (Test-Path $stdoutPath) {
            Write-Host (Get-Content -Raw $stdoutPath) -ForegroundColor Yellow
        }
        throw "FAIL: $Name exited with $status (expected $ExpectedExit)"
    }

    $actualPath = Join-Path $workDir $ActualFile
    if (-not (Test-Path $actualPath)) {
        if ($stderr) {
            Write-Host $stderr.TrimEnd() -ForegroundColor Yellow
        }
        throw "FAIL: $Name did not create $ActualFile"
    }

    if (-not (Test-ByteEqual $actualPath $expectedPath)) {
        if ($stderr) {
            Write-Host $stderr.TrimEnd() -ForegroundColor Yellow
        }
        if (Test-Path $stdoutPath) {
            Write-Host (Get-Content -Raw $stdoutPath) -ForegroundColor Yellow
        }
        Write-Host "Expected bytes:" -ForegroundColor Yellow
        Show-Bytes $expectedPath
        Write-Host "Actual bytes:" -ForegroundColor Yellow
        Show-Bytes $actualPath
        throw "FAIL: $Name output mismatch"
    }
}

function Invoke-ShellPipeCheck {
    param(
        [string]$Name,
        [int]$ExpectedExit,
        [string]$EnvName,
        [string]$EnvValue,
        [string]$InputText,
        [string]$ExpectedStdout
    )

    $safeName = [regex]::Replace($Name, '[^A-Za-z0-9_.-]', '_')
    $workDir = Join-Path $TempDir "$safeName.work"
    $stdoutPath = Join-Path $workDir 'shell.stdout'
    $stderrPath = Join-Path $workDir 'shell.stderr'
    $expectedPath = Join-Path $TempDir "$safeName.expected"
    $scriptPath = Join-Path $workDir 'script.in'

    New-Item -ItemType Directory -Path $workDir | Out-Null
    [System.IO.File]::WriteAllBytes($expectedPath, ([System.Text.UTF8Encoding]::new($false)).GetBytes($ExpectedStdout))
    [System.IO.File]::WriteAllBytes($scriptPath, ([System.Text.UTF8Encoding]::new($false)).GetBytes($InputText))

    $pathValue = if ($env:PATH) { "$BinDir;$env:PATH" } else { $BinDir }

    Write-Host "--- Running $Name ---"
    $psi = New-Object System.Diagnostics.ProcessStartInfo
    $psi.FileName = if ($env:ComSpec) { $env:ComSpec } else { 'cmd.exe' }
    $psi.WorkingDirectory = $workDir
    $psi.UseShellExecute = $false
    $psi.EnvironmentVariables['PATH'] = $pathValue
    $psi.EnvironmentVariables[$EnvName] = $EnvValue
    $psi.Arguments = '/d /c sh.exe < script.in > shell.stdout 2> shell.stderr'

    $process = [System.Diagnostics.Process]::Start($psi)
    try {
        if (-not $process.WaitForExit(15000)) {
            $hungPid = $process.Id
            $process.Kill()
            $process.WaitForExit()
            throw "FAIL: $Name timed out (possible pipe hang, pid $hungPid)"
        }
        $status = $process.ExitCode
    }
    finally {
        $process.Dispose()
    }

    $stderr = if (Test-Path $stderrPath) { Get-Content -Raw $stderrPath } else { '' }
    if ($status -ne $ExpectedExit) {
        if ($stderr) {
            Write-Host $stderr.TrimEnd() -ForegroundColor Yellow
        }
        if (Test-Path $stdoutPath) {
            Write-Host (Get-Content -Raw $stdoutPath) -ForegroundColor Yellow
        }
        throw "FAIL: $Name exited with $status (expected $ExpectedExit)"
    }

    if (-not (Test-ByteEqual $stdoutPath $expectedPath)) {
        if ($stderr) {
            Write-Host $stderr.TrimEnd() -ForegroundColor Yellow
        }
        Write-Host "Expected bytes:" -ForegroundColor Yellow
        Show-Bytes $expectedPath
        Write-Host "Actual bytes:" -ForegroundColor Yellow
        Show-Bytes $stdoutPath
        throw "FAIL: $Name output mismatch"
    }
}

New-Item -ItemType Directory -Path $TempDir | Out-Null
Push-Location $FixtureDir

try {
    Write-Host "=== Running showcase smoke tests ==="

    Invoke-Check 'base64' 'base64.expected' 0 'base64' @('sample.txt')
    Invoke-Check 'base64 -d' 'sample.txt' 0 'base64' @('-d', 'sample.b64')
    Invoke-Check 'checksum' 'checksum.expected' 0 'checksum' @('sample.txt')
    Invoke-Check 'countlines' 'countlines.expected' 3 'countlines' @('sample.txt')
    Invoke-Check 'grep' 'grep.expected' 0 'grep' @('beta', 'sample.txt')
    Invoke-Check 'hexdump' 'hexdump.expected' 0 'hexdump' @('sample.txt')
    Invoke-Check 'ls' 'ls.expected' 0 'ls' @('lsdir')
    Invoke-EnvCheck 'printenv' 'printenv.expected' 0 'LASTSTANDING_SMOKE' 'showcase-value' 'printenv' @('LASTSTANDING_SMOKE')
    Invoke-Check 'sort -n' 'sort_numeric.expected' 0 'sort' @('-n', 'sort_numeric.txt')
    Invoke-Check 'sort -u' 'sort_unique.expected' 0 'sort' @('-u', 'sort_unique.txt')
    Invoke-Check 'upper' 'upper.expected' 0 'upper' @('sample.txt')
    Invoke-Check 'wc' 'wc.expected' 0 'wc' @('sample.txt')
    Invoke-Check 'led' 'led.expected' 0 'led'
    Invoke-Check 'sh --help' 'sh.expected' 0 'sh' @('--help')
    Invoke-ShellRedirCheck 'sh redirection' 0 'sh-redir.out' @('sample.txt', 'sort_numeric.txt') @('sort_numeric.expected', 'upper.expected') "sort -n < sort_numeric.txt > sh-redir.out`nupper sample.txt >> sh-redir.out`nexit 0`n"
    $pipeLeaf = 'sh_pipe.work'
    Invoke-ShellPipeCheck 'sh pipe' 0 'LASTSTANDING_SMOKE' 'showcase-value' "printenv LASTSTANDING_SMOKE | sort`nexit 0`n" "$pipeLeaf`$ LASTSTANDING_SMOKE=showcase-value`n$pipeLeaf`$ "

    Write-Host "=== Showcase smoke tests passed ===" -ForegroundColor Green
    exit 0
}
catch {
    Write-Host $_.Exception.Message -ForegroundColor Red
    exit 1
}
finally {
    Pop-Location
    if (Test-Path $TempDir) {
        Remove-Item -Path $TempDir -Recurse -Force
    }
}
