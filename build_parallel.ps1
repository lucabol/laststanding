# Parallel compilation helper for build.bat
# Usage: powershell -NoProfile -ExecutionPolicy Bypass -File build_parallel.ps1 -CC <compiler> -SrcDirs tests,examples -OutDir bin
param(
    [Parameter(Mandatory)][string]$CC,
    [string[]]$SrcDirs = @('tests', 'examples'),
    [string]$OutDir = 'bin'
)

$maxJobs = if ($env:NUMBER_OF_PROCESSORS) { [int]$env:NUMBER_OF_PROCESSORS } else { 4 }
$files = foreach ($dir in $SrcDirs) {
    if (Test-Path $dir) {
        Get-ChildItem -Path (Join-Path $dir '*.c') -File
    }
}
$files = $files | Sort-Object FullName
$failed = [System.Collections.ArrayList]::new()
$procs = [System.Collections.ArrayList]::new()

foreach ($f in $files) {
    $name = $f.BaseName
    Write-Host "  $name"

    # Detect if source uses sockets and needs ws2_32
    $extraLibs = ""
    $content = Get-Content $f.FullName -Raw
    if ($content -match 'L_WITHSOCKETS') {
        $extraLibs = "-lws2_32"
    }

    $psi = [System.Diagnostics.ProcessStartInfo]::new()
    $psi.FileName = $CC
    $psi.Arguments = "-I. `"$($f.FullName)`" -Oz -lkernel32 $extraLibs -ffreestanding -o `"$OutDir\$name.exe`""
    $psi.UseShellExecute = $false
    $psi.CreateNoWindow = $true
    $proc = [System.Diagnostics.Process]::Start($psi)
    [void]$procs.Add(@{ Name = $name; Proc = $proc })

    # Throttle to $maxJobs concurrent processes
    while (($procs | Where-Object { -not $_.Proc.HasExited }).Count -ge $maxJobs) {
        Start-Sleep -Milliseconds 50
    }
}

# Wait for remaining
foreach ($p in $procs) {
    $p.Proc.WaitForExit()
    if ($p.Proc.ExitCode -ne 0) { [void]$failed.Add($p.Name) }
}

if ($failed.Count -gt 0) {
    foreach ($n in $failed) { Write-Host "FAILED: $n" }
    exit 1
}
