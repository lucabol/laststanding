<#
.SYNOPSIS
    Generates the API function reference tables in README.md from /// doc-comments in l_os.h and l_gfx.h.
.DESCRIPTION
    Parses header files for lines matching '/// description' followed by a function declaration or
    inline definition. Groups functions by section comments ('// Section name') and outputs markdown
    tables between marker pairs in README.md.
.PARAMETER Check
    If set, exits with error code 1 when README.md is out of date instead of updating it.
#>
param(
    [switch]$Check
)

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$readmeFile = Join-Path $root 'README.md'

function Parse-Header {
    param(
        [string]$Path,
        [switch]$RequireWithDefs  # if set, only parse inside #ifdef L_WITHDEFS blocks
    )

    $lines = Get-Content $Path
    $functions = @()
    $currentGroup = ''
    $inWithDefs = -not $RequireWithDefs  # if not required, always "in"

    for ($i = 0; $i -lt $lines.Count; $i++) {
        $line = $lines[$i].Trim()

        if ($RequireWithDefs) {
            if ($line -eq '#ifdef L_WITHDEFS') { $inWithDefs = $true; continue }
            if ($line -eq '#endif // L_WITHDEFS') { $inWithDefs = $false; continue }
            if (-not $inWithDefs) { continue }
        }

        # Group header: // ── Section name ──  or  // Section name
        if ($line -match '^//\s+(?:──\s+)?(.+?)(?:\s+──.*)?$' -and $line -notmatch '^///') {
            $g = $Matches[1].Trim()
            # Skip noisy implementation comments
            if ($g -notmatch 'Suppress|Window class|Framebuffer ioctl|Simplified|Each character') {
                $currentGroup = $g
            }
            continue
        }

        # Doc-comment: /// description
        if ($line -match '^///\s+(.+)$') {
            $desc = $Matches[1]
            # Next non-empty line should be a function declaration or inline definition
            $j = $i + 1
            while ($j -lt $lines.Count -and $lines[$j].Trim() -eq '') { $j++ }
            if ($j -lt $lines.Count) {
                $declLine = $lines[$j].Trim()
                # Match: [noreturn] [static] [inline] type [*]name(params) [;|{]
                if ($declLine -match '(?:noreturn\s+)?(?:static\s+)?(?:inline\s+)?(\w+)\s+\*?\s*(l_\w+)\s*\(([^)]*)\)\s*[;{]') {
                    $name = $Matches[2]
                    $functions += [PSCustomObject]@{
                        Group    = $currentGroup
                        Name     = $name
                        Desc     = $desc
                        Platform = if ($currentGroup -match '(?i)unix') { 'Unix' } else { 'All' }
                    }
                }
                # Match #define macros: /// desc then #define NAME(...)
                elseif ($declLine -match '^#define\s+(L_\w+)\(([^)]*)\)') {
                    $name = $Matches[1]
                    $functions += [PSCustomObject]@{
                        Group    = $currentGroup
                        Name     = $name
                        Desc     = $desc
                        Platform = 'All'
                    }
                }
            }
        }
    }

    return $functions
}

function Build-MarkdownTable {
    param(
        [array]$Functions,
        [string[]]$Columns  # e.g. @('Function', 'Description', 'Platform')
    )

    $md = @()
    $md += ''

    if ($Columns.Count -eq 3) {
        $md += "| Function | Description | Platform |"
        $md += "|----------|-------------|----------|"
    } else {
        $md += "| Function | Description |"
        $md += "|----------|-------------|"
    }

    $lastGroup = ''
    foreach ($fn in $Functions) {
        if ($fn.Group -ne $lastGroup) {
            if ($Columns.Count -eq 3) {
                $md += "| **$($fn.Group)** | | |"
            } else {
                $md += "| **$($fn.Group)** | |"
            }
            $lastGroup = $fn.Group
        }
        if ($Columns.Count -eq 3) {
            $md += "| ``$($fn.Name)`` | $($fn.Desc) | $($fn.Platform) |"
        } else {
            $md += "| ``$($fn.Name)`` | $($fn.Desc) |"
        }
    }
    $md += ''

    return $md -join "`n"
}

function Update-ReadmeSection {
    param(
        [string]$Readme,
        [string]$BeginMarker,
        [string]$EndMarker,
        [string]$MdBlock
    )

    if ($Readme -notmatch [regex]::Escape($BeginMarker)) {
        Write-Error "Missing '$BeginMarker' marker in README.md."
        exit 1
    }
    if ($Readme -notmatch [regex]::Escape($EndMarker)) {
        Write-Error "Missing '$EndMarker' marker in README.md."
        exit 1
    }

    $pattern = "(?s)$([regex]::Escape($BeginMarker)).*?$([regex]::Escape($EndMarker))"
    $replacement = "$BeginMarker`n$MdBlock`n$EndMarker"
    return [regex]::Replace($Readme, $pattern, $replacement)
}

# --- Parse both headers ---

$osFunctions = Parse-Header -Path (Join-Path $root 'l_os.h') -RequireWithDefs
if ($osFunctions.Count -eq 0) {
    Write-Error "No documented functions found in l_os.h."
    exit 1
}

$gfxFunctions = Parse-Header -Path (Join-Path $root 'l_gfx.h')
if ($gfxFunctions.Count -eq 0) {
    Write-Error "No documented functions found in l_gfx.h."
    exit 1
}

# --- Build markdown ---

$osMd  = Build-MarkdownTable -Functions $osFunctions -Columns @('Function','Description','Platform')
$gfxMd = Build-MarkdownTable -Functions $gfxFunctions -Columns @('Function','Description')

# --- Update README.md ---

$readme = Get-Content $readmeFile -Raw
$readme = Update-ReadmeSection $readme '<!-- BEGIN FUNCTION REFERENCE -->' '<!-- END FUNCTION REFERENCE -->' $osMd
$readme = Update-ReadmeSection $readme '<!-- BEGIN GFX REFERENCE -->' '<!-- END GFX REFERENCE -->' $gfxMd

if ($Check) {
    $original = Get-Content $readmeFile -Raw
    if ($readme -ne $original) {
        Write-Host "README.md function reference is out of date. Run: .\gen-docs.ps1" -ForegroundColor Red
        exit 1
    }
    Write-Host "README.md function reference is up to date." -ForegroundColor Green
    exit 0
}

Set-Content $readmeFile $readme -NoNewline
Write-Host "Updated README.md with $($osFunctions.Count) functions from l_os.h and $($gfxFunctions.Count) functions from l_gfx.h" -ForegroundColor Green
