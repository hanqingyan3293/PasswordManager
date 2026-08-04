param(
    [string]$ReleaseDir = "out/PasswordManager-portable",
    [string]$ZipPath = "out/PasswordManager-0.1.0-win-x64-portable.zip",
    [string]$OutputPath = "out/release-size-baseline.md",
    [int]$TopFiles = 20
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$releasePath = Join-Path $repoRoot $ReleaseDir
$archivePath = Join-Path $repoRoot $ZipPath
$reportPath = Join-Path $repoRoot $OutputPath

function Format-Size {
    param([long]$Bytes)

    if ($Bytes -ge 1GB) {
        return ("{0:N2} GiB" -f ($Bytes / 1GB))
    }
    if ($Bytes -ge 1MB) {
        return ("{0:N2} MiB" -f ($Bytes / 1MB))
    }
    if ($Bytes -ge 1KB) {
        return ("{0:N2} KiB" -f ($Bytes / 1KB))
    }
    return ("{0} B" -f $Bytes)
}

if (-not (Test-Path $releasePath)) {
    throw "Release directory was not found: $releasePath"
}
if (-not (Test-Path $archivePath)) {
    throw "Release archive was not found: $archivePath"
}

$allFiles = Get-ChildItem -LiteralPath $releasePath -Recurse -File
$totalBytes = ($allFiles | Measure-Object -Property Length -Sum).Sum
$zipItem = Get-Item -LiteralPath $archivePath
$exePath = Join-Path $releasePath "PasswordManager.exe"
$exeItem = Get-Item -LiteralPath $exePath
$zipHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $archivePath).Hash.ToLowerInvariant()

$categoryRules = @(
    @{ Name = "Application"; Pattern = { param($file) $file.Name -eq "PasswordManager.exe" } },
    @{ Name = "Qt DLL"; Pattern = { param($file) $file.Name -like "Qt6*.dll" } },
    @{ Name = "Graphics Runtime"; Pattern = { param($file) $file.Name -in @("opengl32sw.dll", "d3dcompiler_47.dll", "dxcompiler.dll", "dxil.dll") } },
    @{ Name = "MSVC Redistributable"; Pattern = { param($file) $file.Name -eq "vc_redist.x64.exe" } },
    @{ Name = "7-Zip"; Pattern = { param($file) $file.FullName -like "*\tools\7zip\*" } },
    @{ Name = "Qt Plugins"; Pattern = { param($file) $file.FullName -match "\\(generic|iconengines|imageformats|networkinformation|platforms|sqldrivers|styles|tls)\\" } },
    @{ Name = "Qt Translations"; Pattern = { param($file) $file.FullName -match "\\translations\\" } },
    @{ Name = "Runtime Data"; Pattern = { param($file) $file.FullName -match "\\(data|config|logs|backup)\\" } }
)

$categoryTotals = [ordered]@{}
foreach ($rule in $categoryRules) {
    $categoryTotals[$rule.Name] = 0L
}
$categoryTotals["Other"] = 0L

foreach ($file in $allFiles) {
    $matched = $false
    foreach ($rule in $categoryRules) {
        if (& $rule.Pattern $file) {
            $categoryTotals[$rule.Name] = [long]$categoryTotals[$rule.Name] + [long]$file.Length
            $matched = $true
            break
        }
    }
    if (-not $matched) {
        $categoryTotals["Other"] = [long]$categoryTotals["Other"] + [long]$file.Length
    }
}

$smokeElapsed = Measure-Command {
    & $exePath --smoke-test | Out-Null
}

$qmlResidue = Get-ChildItem -LiteralPath $releasePath -File |
    Where-Object { $_.Name -like "Qt6Qml*.dll" -or $_.Name -like "Qt6Quick*.dll" -or $_.Name -eq "Qt6OpenGL.dll" -or $_.Name -eq "Qt6QuickWidgets.dll" }
$qmlDirectories = @("qml", "qmltooling") | Where-Object { Test-Path (Join-Path $releasePath $_) }

$lines = @()
$lines += "# Release Size Baseline"
$lines += ""
$lines += "GeneratedAt: $((Get-Date).ToString('yyyy-MM-dd HH:mm:ss zzz'))"
$lines += ""
$lines += "## Summary"
$lines += ""
$lines += "| Item | Value |"
$lines += "| --- | ---: |"
$lines += "| Release directory | ``$ReleaseDir`` |"
$lines += "| Release directory size | $(Format-Size $totalBytes) |"
$lines += "| Release archive size | $(Format-Size $zipItem.Length) |"
$lines += "| PasswordManager.exe size | $(Format-Size $exeItem.Length) |"
$lines += "| File count | $($allFiles.Count) |"
$lines += "| smoke-test elapsed | $([Math]::Round($smokeElapsed.TotalMilliseconds, 0)) ms |"
$lines += "| SHA256 | ``$zipHash`` |"
$lines += ""
$lines += "## Category Breakdown"
$lines += ""
$lines += "| Category | Size | Share |"
$lines += "| --- | ---: | ---: |"
foreach ($entry in $categoryTotals.GetEnumerator()) {
    $share = if ($totalBytes -gt 0) { "{0:P1}" -f ([double]$entry.Value / [double]$totalBytes) } else { "0.0%" }
    $lines += "| $($entry.Name) | $(Format-Size ([long]$entry.Value)) | $share |"
}
$lines += ""
$lines += "## Largest Files"
$lines += ""
$lines += "| File | Size |"
$lines += "| --- | ---: |"
$releasePrefix = [System.IO.Path]::GetFullPath($releasePath)
if (-not $releasePrefix.EndsWith([System.IO.Path]::DirectorySeparatorChar)) {
    $releasePrefix += [System.IO.Path]::DirectorySeparatorChar
}
$allFiles |
    Sort-Object Length -Descending |
    Select-Object -First $TopFiles |
    ForEach-Object {
        $relative = [System.IO.Path]::GetFullPath($_.FullName).Substring($releasePrefix.Length)
        $lines += "| ``$relative`` | $(Format-Size $_.Length) |"
    }
$lines += ""
$lines += "## Lightweight Route Checks"
$lines += ""
$lines += "- QML/Quick DLL residue: $($qmlResidue.Count)"
$lines += "- QML directories: $($qmlDirectories.Count)"
$lines += "- Qt Quick is not expected in the current lightweight Widgets release route."
$lines += ""
$lines += "## Notes"
$lines += ""
$lines += "- This baseline measures package composition only. It does not prove a file is safe to remove."
$lines += "- Potential removals must be tested on a clean Windows machine or VM."

New-Item -ItemType Directory -Force -Path (Split-Path -Parent $reportPath) | Out-Null
$lines | Set-Content -Path $reportPath -Encoding UTF8

Write-Host "Release baseline report created: $reportPath"
Write-Host "Release archive SHA256: $zipHash"
