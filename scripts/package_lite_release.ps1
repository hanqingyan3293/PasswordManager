param(
    [string]$SourceDir = "out/PasswordManager-portable",
    [string]$OutputDir = "out/PasswordManager-lite-portable",
    [string]$Version = "0.1.0"
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$sourcePath = Join-Path $repoRoot $SourceDir
$outputPath = Join-Path $repoRoot $OutputDir
$outRoot = Join-Path $repoRoot "out"
$zipFileName = "PasswordManager-$Version-win-x64-lite.zip"
$zipPath = Join-Path $outRoot $zipFileName
$zipHashPath = "$zipPath.sha256"

function Assert-UnderDirectory {
    param(
        [string]$Path,
        [string]$Parent
    )

    $fullPath = [System.IO.Path]::GetFullPath($Path)
    $fullParent = [System.IO.Path]::GetFullPath($Parent)
    if (-not $fullParent.EndsWith([System.IO.Path]::DirectorySeparatorChar)) {
        $fullParent += [System.IO.Path]::DirectorySeparatorChar
    }

    if (-not $fullPath.StartsWith($fullParent, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Refusing to write outside project out directory: $fullPath"
    }
}

function Remove-IfExists {
    param([string]$Path)

    if (Test-Path $Path) {
        Remove-Item -LiteralPath $Path -Force
        return $true
    }
    return $false
}

if (-not (Test-Path $sourcePath)) {
    throw "Source release directory was not found: $sourcePath"
}

Assert-UnderDirectory -Path $outputPath -Parent $outRoot
Assert-UnderDirectory -Path $zipPath -Parent $outRoot
Assert-UnderDirectory -Path $zipHashPath -Parent $outRoot

if (Test-Path $outputPath) {
    Remove-Item -LiteralPath $outputPath -Recurse -Force
}
if (Test-Path $zipPath) {
    Remove-Item -LiteralPath $zipPath -Force
}
if (Test-Path $zipHashPath) {
    Remove-Item -LiteralPath $zipHashPath -Force
}

Copy-Item -LiteralPath $sourcePath -Destination $outputPath -Recurse -Force

$removed = New-Object System.Collections.Generic.List[string]

$vcRedist = Join-Path $outputPath "vc_redist.x64.exe"
if (Remove-IfExists $vcRedist) {
    $removed.Add("vc_redist.x64.exe")
}

$translationsDir = Join-Path $outputPath "translations"
if (Test-Path $translationsDir) {
    Get-ChildItem -LiteralPath $translationsDir -File |
        Where-Object { $_.Name -ne "qt_zh_CN.qm" } |
        ForEach-Object {
            Remove-Item -LiteralPath $_.FullName -Force
            $removed.Add("translations\$($_.Name)")
        }
}

$sqlDriversDir = Join-Path $outputPath "sqldrivers"
if (Test-Path $sqlDriversDir) {
    Get-ChildItem -LiteralPath $sqlDriversDir -File |
        Where-Object { $_.Name -ne "qsqlite.dll" } |
        ForEach-Object {
            Remove-Item -LiteralPath $_.FullName -Force
            $removed.Add("sqldrivers\$($_.Name)")
        }
}

$notesPath = Join-Path $outputPath "LITE_PACKAGE_NOTES.txt"
@"
PasswordManager lite portable release

This package keeps all PasswordManager application features, but removes optional
deployment files to reduce download size.

Removed:
$($removed | ForEach-Object { "  - $_" } | Out-String)
Compatibility notes:
  - Requires Microsoft Visual C++ runtime already installed on the target system.
  - Keeps only SQLite SQL driver.
  - Keeps only Qt Simplified Chinese translation.
  - Keeps graphics runtime fallback files for compatibility.

If this package does not start on a clean Windows machine, use the full portable
release instead.
"@ | Set-Content -Path $notesPath -Encoding UTF8

Compress-Archive -LiteralPath $outputPath -DestinationPath $zipPath -Force
$zipHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $zipPath).Hash.ToLowerInvariant()
"$zipHash  $zipFileName" | Set-Content -Path $zipHashPath -Encoding ASCII

Write-Host "Lite release directory created: $outputPath"
Write-Host "Lite release archive created: $zipPath"
Write-Host "Lite release archive SHA256: $zipHashPath"
Write-Host "Removed optional files: $($removed.Count)"
