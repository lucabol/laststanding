<#
.SYNOPSIS
    Generates the API function reference table in README.md from /// doc-comments in l_os.h.
.DESCRIPTION
    Parses l_os.h for lines matching '/// description' followed by a function declaration.
    Groups functions by section comments ('// Section name') and outputs a markdown table
    between <!-- BEGIN FUNCTION REFERENCE --> and <!-- END FUNCTION REFERENCE --> markers in README.md.
.PARAMETER Check
    If set, exits with error code 1 when README.md is out of date instead of updating it.
#>
param(
    [switch]$Check
)

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $MyInvocation.MyCommand.Path

$headerFile = Join-Path $root 'l_os.h'
$readmeFile = Join-Path $root 'README.md'

# Parse l_os.h for doc-comments and function declarations
$lines = Get-Content $headerFile
$functions = @()
$currentGroup = ''
$inWithDefs = $false

for ($i = 0; $i -lt $lines.Count; $i++) {
    $line = $lines[$i].Trim()

    if ($line -eq '#ifdef L_WITHDEFS') { $inWithDefs = $true; continue }
    if ($line -eq '#endif // L_WITHDEFS') { $inWithDefs = $false; continue }
    if (-not $inWithDefs) { continue }

    # Group header: // Section name
    if ($line -match '^//\s+(.+)$' -and $line -notmatch '^///') {
        $currentGroup = $Matches[1]
        continue
    }

    # Doc-comment followed by function declaration
    if ($line -match '^///\s+(.+)$') {
        $desc = $Matches[1]
        # Next non-empty line should be the function declaration
        $j = $i + 1
        while ($j -lt $lines.Count -and $lines[$j].Trim() -eq '') { $j++ }
        if ($j -lt $lines.Count) {
            $declLine = $lines[$j].Trim()
            # Extract function name and signature from declaration
            if ($declLine -match '(?:noreturn\s+)?(?:static\s+)?(?:inline\s+)?([\w]+)\s+\*?\s*(l_\w+)\s*\(([^)]*)\)\s*;') {
                $retType = $Matches[1]
                $name    = $Matches[2]
                $params  = $Matches[3]
                $functions += [PSCustomObject]@{
                    Group     = $currentGroup
                    Name      = $name
                    Signature = "$retType $name($params)"
                    Desc      = $desc
                    Platform  = if ($currentGroup -match '(?i)unix') { 'Unix' } else { 'All' }
                }
            }
        }
    }
}

if ($functions.Count -eq 0) {
    Write-Error "No documented functions found in $headerFile. Check /// comment format."
    exit 1
}

# Build markdown
$md = @()
$md += ''
$md += "| Function | Description | Platform |"
$md += "|----------|-------------|----------|"

$lastGroup = ''
foreach ($fn in $functions) {
    if ($fn.Group -ne $lastGroup) {
        $md += "| **$($fn.Group)** | | |"
        $lastGroup = $fn.Group
    }
    $sig = $fn.Signature -replace '\|', '\|'
    $md += "| ``$($fn.Name)`` | $($fn.Desc) | $($fn.Platform) |"
}
$md += ''

$mdBlock = $md -join "`n"

# Read README.md and replace between markers
$readme = Get-Content $readmeFile -Raw
$beginMarker = '<!-- BEGIN FUNCTION REFERENCE -->'
$endMarker   = '<!-- END FUNCTION REFERENCE -->'

if ($readme -notmatch [regex]::Escape($beginMarker)) {
    Write-Error "Missing '$beginMarker' marker in $readmeFile. Add it where you want the table."
    exit 1
}
if ($readme -notmatch [regex]::Escape($endMarker)) {
    Write-Error "Missing '$endMarker' marker in $readmeFile. Add it after the begin marker."
    exit 1
}

$pattern = "(?s)$([regex]::Escape($beginMarker)).*?$([regex]::Escape($endMarker))"
$replacement = "$beginMarker`n$mdBlock`n$endMarker"
$newReadme = [regex]::Replace($readme, $pattern, $replacement)

if ($Check) {
    if ($newReadme -ne $readme) {
        Write-Host "README.md function reference is out of date. Run: .\gen-docs.ps1" -ForegroundColor Red
        exit 1
    }
    Write-Host "README.md function reference is up to date." -ForegroundColor Green
    exit 0
}

Set-Content $readmeFile $newReadme -NoNewline
$count = $functions.Count
Write-Host "Updated README.md with $count functions from l_os.h" -ForegroundColor Green
