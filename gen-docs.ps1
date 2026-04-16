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
        $raw = $lines[$i]
        $line = $raw.Trim()

        if ($RequireWithDefs) {
            if ($line -eq '#ifdef L_WITHDEFS') { $inWithDefs = $true; continue }
            if ($line -eq '#endif // L_WITHDEFS') { $inWithDefs = $false; continue }
            if (-not $inWithDefs) { continue }
        }

        # Group header: // ── Section name ──  or  // Section name
        # Must not be a doc-comment (///) and must start at column 0 (not indented)
        if ($line -match '^//\s+(?:──\s+)?(.+?)(?:\s+──.*)?$' -and $line -notmatch '^///') {
            $g = $Matches[1].Trim()
            if ($raw -match '^//') {  # not indented — genuine section header
                # Skip pure separator lines (all dashes/equals) and noisy comments
                if ($g -notmatch '^[-=─]+$' -and
                    $g -notmatch 'Suppress|Window class|Framebuffer ioctl|Simplified|Each character') {
                    $currentGroup = $g
                }
            }
            continue
        }

        # Doc-comment: /// description
        # Doc-comment block: collect consecutive /// lines, then match function
        if ($line -match '^///\s+(.+)$') {
            $desc = $Matches[1]
            # Skip any further consecutive /// lines (multi-line doc-comment)
            while (($i + 1) -lt $lines.Count -and $lines[$i + 1].Trim() -match '^///') { $i++ }
            # Next non-empty line should be a function declaration or inline definition
            $j = $i + 1
            while ($j -lt $lines.Count -and $lines[$j].Trim() -eq '') { $j++ }
            if ($j -lt $lines.Count) {
                $declLine = $lines[$j].Trim()
                # Join continuation lines (multi-line signatures)
                while ($declLine -notmatch '[;{]\s*$' -and ($j + 1) -lt $lines.Count) {
                    $j++
                    $nextLine = $lines[$j].Trim()
                    if ($nextLine -match '^//') { break }  # stop at comments
                    $declLine += ' ' + $nextLine
                }
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

# --- Parse all headers ---

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

$uiFunctions = Parse-Header -Path (Join-Path $root 'l_ui.h')
if ($uiFunctions.Count -eq 0) {
    Write-Error "No documented functions found in l_ui.h."
    exit 1
}

# Parse l_svg.h if it exists; otherwise set to empty (not required yet)
$svgPath = Join-Path $root 'l_svg.h'
if (Test-Path $svgPath) {
    $svgFunctions = Parse-Header -Path $svgPath
} else {
    $svgFunctions = @()
}

# --- Build platform compatibility matrix ---

function Build-CompatMatrix {
    param([string]$HeaderPath, [array]$Functions)

    $lines = Get-Content $HeaderPath

    # Collect all @stub annotations: function name → stub kind
    # Scan the WASI implementation block, tracking preprocessor nesting
    $wasiStubs = @{}
    $inWasi = $false
    $wasiDepth = 0
    $currentFunc = ''
    for ($i = 0; $i -lt $lines.Count; $i++) {
        $line = $lines[$i].Trim()
        if ($line -eq '#elif defined(__wasi__)') { $inWasi = $true; $wasiDepth = 1; continue }
        if (-not $inWasi) { continue }

        # Track preprocessor nesting depth
        if ($line -match '^#if') { $wasiDepth++ }
        if ($line -match '^#endif') {
            $wasiDepth--
            if ($wasiDepth -le 0) { $inWasi = $false; continue }
        }
        # #elif at depth 1 means we left the WASI block into the next platform
        if ($wasiDepth -eq 1 -and $line -match '^#(elif|else)\b') { $inWasi = $false; continue }

        # Track which function we're inside
        if ($line -match '(?:static\s+)?(?:noreturn\s+)?(?:inline\s+)?(?:\w+[\s*]+)(l_\w+)\s*\(') {
            $currentFunc = $Matches[1]
        }
        if ($currentFunc -and $line -match '@stub\s+(design|todo):') {
            $wasiStubs[$currentFunc] = $Matches[1]
        }
    }

    # Collect Unix-only declarations (inside #if defined(__unix__) in L_WITHDEFS block)
    $unixOnly = @{}
    $inWithDefs = $false
    $inUnixBlock = $false
    for ($i = 0; $i -lt $lines.Count; $i++) {
        $line = $lines[$i].Trim()
        if ($line -eq '#ifdef L_WITHDEFS') { $inWithDefs = $true; continue }
        if ($line -eq '#endif // L_WITHDEFS') { $inWithDefs = $false; continue }
        if (-not $inWithDefs) { continue }

        if ($line -match '^#if\s+defined\(__unix__\)' -or $line -match '^#ifdef\s+__unix__') {
            $inUnixBlock = $true; continue
        }
        if ($inUnixBlock -and $line -match '^#endif') { $inUnixBlock = $false; continue }

        if ($inUnixBlock -and $line -match '(l_\w+)\s*\(') {
            $unixOnly[$Matches[1]] = $true
        }
    }

    # Build the table
    $check = [char]0x2705   # ✅
    $cross = [char]0x274C   # ❌
    $warn  = [char]0x26A0   # ⚠️ (without variation selector — safe in PS)
    $dash  = [char]0x2014   # —

    $md = @()
    $md += ''
    $md += '| Function | Linux | Windows | WASI |'
    $md += '|----------|-------|---------|------|'

    $lastGroup = ''
    foreach ($fn in $Functions) {
        if ($fn.Group -ne $lastGroup) {
            $md += "| **$($fn.Group)** | | | |"
            $lastGroup = $fn.Group
        }

        $name = $fn.Name
        # Skip macros (L_*) — they're always available everywhere
        if ($name -cmatch '^L_') {
            $md += "| ``$name`` | All | All | All |"
            continue
        }

        $linux = $check
        $windows = $check
        $wasi = $check

        # Unix-only functions
        if ($unixOnly.ContainsKey($name)) {
            $windows = $dash
        }

        # WASI stubs
        if ($wasiStubs.ContainsKey($name)) {
            if ($wasiStubs[$name] -eq 'design') {
                $wasi = $cross
            } else {
                $wasi = $warn
            }
        }

        $md += ('| ``' + $name + '`` | ' + $linux + ' | ' + $windows + ' | ' + $wasi + ' |')
    }
    $md += ''
    return $md -join "`n"
}

# --- Build test coverage matrix ---

function Build-CoverageMatrix {
    param([array]$Functions, [string]$TestDir)

    $testFiles = Get-ChildItem -Path $TestDir -Filter '*.c' -File
    # Pre-read all test file contents
    $testContents = @{}
    foreach ($tf in $testFiles) {
        $testContents[$tf.Name] = Get-Content $tf.FullName -Raw
    }

    $md = @()
    $md += ''
    $md += '| Function | Tested | Test File |'
    $md += '|----------|--------|-----------|'

    $testedCount = 0
    $totalCount = 0
    $lastGroup = ''

    foreach ($fn in $Functions) {
        if ($fn.Group -ne $lastGroup) {
            $md += "| **$($fn.Group)** | | |"
            $lastGroup = $fn.Group
        }

        $name = $fn.Name
        $totalCount++
        $found = @()
        foreach ($tf in $testFiles) {
            if ($testContents[$tf.Name] -match [regex]::Escape($name)) {
                $found += $tf.Name
            }
        }
        if ($found.Count -gt 0) {
            $testedCount++
            $files = $found -join ', '
            $md += "| ``$name`` | $([char]0x2705) | $files |"
        } else {
            $md += ('| ``' + $name + '`` | ' + [char]0x2014 + ' | |')
        }
    }
    $md += ''
    $pct = [math]::Round($testedCount * 100 / $totalCount)
    $md += ('**Coverage: ' + $testedCount + ' / ' + $totalCount + ' functions referenced in tests** (' + $pct + '%)')
    $md += ''
    return $md -join "`n"
}

$osHeader = Join-Path $root 'l_os.h'
$testDir  = Join-Path $root 'tests'

$compatMd  = Build-CompatMatrix -HeaderPath $osHeader -Functions $osFunctions
$coverMd   = Build-CoverageMatrix -Functions $osFunctions -TestDir $testDir

# --- Build markdown ---

$osMd  = Build-MarkdownTable -Functions $osFunctions -Columns @('Function','Description','Platform')
$gfxMd = Build-MarkdownTable -Functions $gfxFunctions -Columns @('Function','Description')
$uiMd  = Build-MarkdownTable -Functions $uiFunctions  -Columns @('Function','Description')
# Only build SVG markdown if functions were found
if ($svgFunctions.Count -gt 0) {
    $svgMd = Build-MarkdownTable -Functions $svgFunctions -Columns @('Function','Description')
}

# --- Update README.md ---

$readme = [System.IO.File]::ReadAllText($readmeFile, [System.Text.UTF8Encoding]::new($false))
$readme = Update-ReadmeSection $readme '<!-- BEGIN FUNCTION REFERENCE -->' '<!-- END FUNCTION REFERENCE -->' $osMd
$readme = Update-ReadmeSection $readme '<!-- BEGIN GFX REFERENCE -->' '<!-- END GFX REFERENCE -->' $gfxMd
if ($svgFunctions.Count -gt 0) {
    $readme = Update-ReadmeSection $readme '<!-- BEGIN SVG REFERENCE -->' '<!-- END SVG REFERENCE -->' $svgMd
}
$readme = Update-ReadmeSection $readme '<!-- BEGIN UI REFERENCE -->' '<!-- END UI REFERENCE -->' $uiMd

# Update compat and coverage matrices only if markers exist
if ($readme -match '<!-- BEGIN COMPAT MATRIX -->') {
    $readme = Update-ReadmeSection $readme '<!-- BEGIN COMPAT MATRIX -->' '<!-- END COMPAT MATRIX -->' $compatMd
}
if ($readme -match '<!-- BEGIN COVERAGE MATRIX -->') {
    $readme = Update-ReadmeSection $readme '<!-- BEGIN COVERAGE MATRIX -->' '<!-- END COVERAGE MATRIX -->' $coverMd
}

if ($Check) {
    $original = [System.IO.File]::ReadAllText($readmeFile, [System.Text.UTF8Encoding]::new($false))
    if ($readme -ne $original) {
        Write-Host "README.md function reference is out of date. Run: .\gen-docs.ps1" -ForegroundColor Red
        exit 1
    }
    Write-Host "README.md function reference is up to date." -ForegroundColor Green
    exit 0
}

[System.IO.File]::WriteAllText($readmeFile, $readme, [System.Text.UTF8Encoding]::new($false))
$extra = @()
if ($readme -match '<!-- BEGIN COMPAT MATRIX -->') { $extra += 'compat matrix' }
if ($readme -match '<!-- BEGIN COVERAGE MATRIX -->') { $extra += 'coverage matrix' }
$suffix = if ($extra.Count -gt 0) { " + $($extra -join ', ')" } else { '' }
$svgNote = if ($svgFunctions.Count -gt 0) { ", $($svgFunctions.Count) functions from l_svg.h" } else { "" }
Write-Host "Updated README.md with $($osFunctions.Count) functions from l_os.h, $($gfxFunctions.Count) functions from l_gfx.h, and $($uiFunctions.Count) functions from l_ui.h$svgNote$suffix" -ForegroundColor Green
