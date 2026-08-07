param(
    [Parameter(Mandatory = $true)]
    [string]$ArchivePath,
    [Parameter(Mandatory = $true)]
    [string]$ExpectedRootName,
    [Parameter(Mandatory = $true)]
    [string]$ExpectedExePath,
    [switch]$RequireVcRedist,
    [switch]$ForbidVcRedist
)

$ErrorActionPreference = "Stop"

if ($RequireVcRedist -eq $ForbidVcRedist) {
    throw "Specify exactly one of -RequireVcRedist or -ForbidVcRedist."
}

$archiveFullPath = [System.IO.Path]::GetFullPath($ArchivePath)
$expectedExeFullPath = [System.IO.Path]::GetFullPath($ExpectedExePath)
if (-not (Test-Path -LiteralPath $archiveFullPath -PathType Leaf)) {
    throw "Release archive was not found: $archiveFullPath"
}
if (-not (Test-Path -LiteralPath $expectedExeFullPath -PathType Leaf)) {
    throw "Expected build executable was not found: $expectedExeFullPath"
}

$archiveDir = Split-Path -Parent $archiveFullPath
$verifyRoot = Join-Path $archiveDir (".release-verify-" + [System.Guid]::NewGuid().ToString("N"))
$verifyFullPath = [System.IO.Path]::GetFullPath($verifyRoot)
$archiveDirPrefix = [System.IO.Path]::GetFullPath($archiveDir)
if (-not $archiveDirPrefix.EndsWith([System.IO.Path]::DirectorySeparatorChar)) {
    $archiveDirPrefix += [System.IO.Path]::DirectorySeparatorChar
}
if (-not $verifyFullPath.StartsWith($archiveDirPrefix, [System.StringComparison]::OrdinalIgnoreCase)) {
    throw "Unsafe archive verification directory: $verifyFullPath"
}

function Assert-Manifest {
    param(
        [string]$Root,
        [string]$ManifestName
    )

    $manifestPath = Join-Path $Root $ManifestName
    if (-not (Test-Path -LiteralPath $manifestPath -PathType Leaf)) {
        throw "Release manifest was not found: $manifestPath"
    }

    $rootPrefix = [System.IO.Path]::GetFullPath($Root)
    if (-not $rootPrefix.EndsWith([System.IO.Path]::DirectorySeparatorChar)) {
        $rootPrefix += [System.IO.Path]::DirectorySeparatorChar
    }

    $entries = 0
    foreach ($line in Get-Content -LiteralPath $manifestPath) {
        if ($line -notmatch "^([0-9a-fA-F]{64})  (.+)$") {
            continue
        }

        $expectedHash = $Matches[1]
        $relativePath = $Matches[2]
        $targetPath = [System.IO.Path]::GetFullPath((Join-Path $Root $relativePath))
        if (-not $targetPath.StartsWith($rootPrefix, [System.StringComparison]::OrdinalIgnoreCase)) {
            throw "Manifest path escapes the release root: $relativePath"
        }
        if (-not (Test-Path -LiteralPath $targetPath -PathType Leaf)) {
            throw "Manifest file is missing from the release archive: $relativePath"
        }

        $actualHash = (Get-FileHash -LiteralPath $targetPath -Algorithm SHA256).Hash
        if ($actualHash -ne $expectedHash) {
            throw "Manifest hash mismatch: $relativePath"
        }
        $entries++
    }

    if ($entries -eq 0) {
        throw "Manifest contains no file entries: $manifestPath"
    }
}

try {
    New-Item -ItemType Directory -Path $verifyFullPath | Out-Null
    Expand-Archive -LiteralPath $archiveFullPath -DestinationPath $verifyFullPath -Force

    $packageRoot = Join-Path $verifyFullPath $ExpectedRootName
    if (-not (Test-Path -LiteralPath $packageRoot -PathType Container)) {
        throw "Release archive has an unexpected root directory. Expected: $ExpectedRootName"
    }

    $runtimeFiles = foreach ($dirName in @("data", "config", "logs", "backup")) {
        $runtimeDir = Join-Path $packageRoot $dirName
        if (Test-Path -LiteralPath $runtimeDir) {
            Get-ChildItem -LiteralPath $runtimeDir -Recurse -File
        }
    }
    if (@($runtimeFiles).Count -ne 0) {
        throw "Release archive contains local database, configuration, log, or backup files."
    }

    $packagedExe = Join-Path $packageRoot "PasswordManager.exe"
    $expectedExeHash = (Get-FileHash -LiteralPath $expectedExeFullPath -Algorithm SHA256).Hash
    $packagedExeHash = (Get-FileHash -LiteralPath $packagedExe -Algorithm SHA256).Hash
    if ($expectedExeHash -ne $packagedExeHash) {
        throw "Packaged PasswordManager.exe does not match the expected Release build output."
    }

    $vcRedistPath = Join-Path $packageRoot "vc_redist.x64.exe"
    if ($RequireVcRedist -and -not (Test-Path -LiteralPath $vcRedistPath -PathType Leaf)) {
        throw "Full release archive is missing vc_redist.x64.exe."
    }
    if ($ForbidVcRedist -and (Test-Path -LiteralPath $vcRedistPath)) {
        throw "Lite release archive contains vc_redist.x64.exe."
    }

    Assert-Manifest -Root $packageRoot -ManifestName "VC_RUNTIME_MANIFEST.txt"
    Assert-Manifest -Root $packageRoot -ManifestName "RELEASE_MANIFEST.txt"

    Write-Host "Release archive verification passed: $archiveFullPath"
    Write-Host "Runtime data files in archive: 0"
    Write-Host "PasswordManager.exe SHA256: $($packagedExeHash.ToLowerInvariant())"
}
finally {
    if (Test-Path -LiteralPath $verifyFullPath) {
        Remove-Item -LiteralPath $verifyFullPath -Recurse -Force -ErrorAction SilentlyContinue
    }
}
