param(
    [string]$BuildDir = "build-release",
    [string]$OutputDir = "out/PasswordManager-lite-portable",
    [string]$QtPrefixPath = "C:\Qt\6.8.3\msvc2022_64",
    [string]$Version = "0.1.0",
    [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$outputPath = Join-Path $repoRoot $OutputDir
$outRoot = Join-Path $repoRoot "out"
$buildExePath = Join-Path $repoRoot (Join-Path $BuildDir "PasswordManager.exe")
$fullZipName = "PasswordManager-$Version-win-x64-portable.zip"
$fullZipPath = Join-Path $outRoot $fullZipName
$zipFileName = "PasswordManager-$Version-win-x64-lite.zip"
$zipPath = Join-Path $outRoot $zipFileName
$zipHashPath = "$zipPath.sha256"
$stagingRoot = Join-Path $outRoot (".lite-staging-" + [System.Guid]::NewGuid().ToString("N"))
$preserveRuntimeDir = Join-Path $outRoot (".lite-preserve-" + [System.Guid]::NewGuid().ToString("N"))
$runtimeDirs = @("data", "config", "logs", "backup")

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

function Assert-NoRuntimeFiles {
    param([string]$Root)

    $runtimeFiles = foreach ($dirName in $runtimeDirs) {
        $runtimeDir = Join-Path $Root $dirName
        if (Test-Path -LiteralPath $runtimeDir) {
            Get-ChildItem -LiteralPath $runtimeDir -Recurse -File
        }
    }
    if (@($runtimeFiles).Count -ne 0) {
        $paths = @($runtimeFiles | ForEach-Object { $_.FullName }) -join [Environment]::NewLine
        throw "Release staging contains runtime data files:`n$paths"
    }
}

function Restore-RuntimeDirectories {
    param(
        [string]$BackupRoot,
        [string]$TargetRoot
    )

    if (-not (Test-Path -LiteralPath $BackupRoot)) {
        return
    }
    if (-not (Test-Path -LiteralPath $TargetRoot)) {
        New-Item -ItemType Directory -Force -Path $TargetRoot | Out-Null
    }

    foreach ($dirName in $runtimeDirs) {
        $runtimeBackup = Join-Path $BackupRoot $dirName
        if (-not (Test-Path -LiteralPath $runtimeBackup)) {
            continue
        }

        $runtimeTarget = Join-Path $TargetRoot $dirName
        if (Test-Path -LiteralPath $runtimeTarget) {
            Remove-Item -LiteralPath $runtimeTarget -Recurse -Force
        }
        Copy-Item -LiteralPath $runtimeBackup -Destination $runtimeTarget -Recurse -Force
    }
}

function Write-ReleaseManifest {
    param(
        [string]$Root,
        [string]$PackageName
    )

    $manifestPath = Join-Path $Root "RELEASE_MANIFEST.txt"
    if (Test-Path -LiteralPath $manifestPath) {
        Remove-Item -LiteralPath $manifestPath -Force
    }

    $rootPrefix = [System.IO.Path]::GetFullPath($Root)
    if (-not $rootPrefix.EndsWith([System.IO.Path]::DirectorySeparatorChar)) {
        $rootPrefix += [System.IO.Path]::DirectorySeparatorChar
    }

    $lines = @(
        "PasswordManager release manifest",
        "Version: $Version",
        "Package: $PackageName",
        "GeneratedAt: $((Get-Date).ToUniversalTime().ToString('yyyy-MM-ddTHH:mm:ssZ'))",
        "",
        "Files:"
    )
    $lines += Get-ChildItem -LiteralPath $Root -Recurse -File |
        Sort-Object FullName |
        ForEach-Object {
            $relative = $_.FullName.Substring($rootPrefix.Length)
            $hash = (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
            "$hash  $relative"
        }
    $lines | Set-Content -LiteralPath $manifestPath -Encoding UTF8
}

Assert-UnderDirectory -Path $outputPath -Parent $outRoot
Assert-UnderDirectory -Path $zipPath -Parent $outRoot
Assert-UnderDirectory -Path $zipHashPath -Parent $outRoot
Assert-UnderDirectory -Path $stagingRoot -Parent $outRoot
Assert-UnderDirectory -Path $preserveRuntimeDir -Parent $outRoot

$runtimeBackupReady = $false
Push-Location $repoRoot
try {
    $packageReleaseArgs = @{
        BuildDir = $BuildDir
        QtPrefixPath = $QtPrefixPath
        Version = $Version
    }
    if ($SkipBuild) {
        $packageReleaseArgs.SkipBuild = $true
    }
    & (Join-Path $PSScriptRoot "package_release.ps1") @packageReleaseArgs

    if (-not (Test-Path -LiteralPath $fullZipPath -PathType Leaf)) {
        throw "Fresh full release archive was not found: $fullZipPath"
    }
    if (-not (Test-Path -LiteralPath $buildExePath -PathType Leaf)) {
        throw "Release build output was not found: $buildExePath"
    }

    New-Item -ItemType Directory -Force -Path $stagingRoot | Out-Null
    Expand-Archive -LiteralPath $fullZipPath -DestinationPath $stagingRoot -Force
    $fullPackageDir = Join-Path $stagingRoot "PasswordManager-portable"
    if (-not (Test-Path -LiteralPath $fullPackageDir -PathType Container)) {
        throw "The full release archive has an unexpected directory layout."
    }

    Assert-NoRuntimeFiles -Root $fullPackageDir

    $fullExePath = Join-Path $fullPackageDir "PasswordManager.exe"
    $buildExeHash = (Get-FileHash -LiteralPath $buildExePath -Algorithm SHA256).Hash
    $fullExeHash = (Get-FileHash -LiteralPath $fullExePath -Algorithm SHA256).Hash
    if ($buildExeHash -ne $fullExeHash) {
        throw "Full package PasswordManager.exe does not match the current Release build output."
    }

    if (Test-Path -LiteralPath $outputPath) {
        New-Item -ItemType Directory -Force -Path $preserveRuntimeDir | Out-Null
        foreach ($dirName in $runtimeDirs) {
            $source = Join-Path $outputPath $dirName
            if (Test-Path -LiteralPath $source) {
                Copy-Item -LiteralPath $source -Destination (Join-Path $preserveRuntimeDir $dirName) -Recurse -Force
            }
        }
        $runtimeBackupReady = $true
        Remove-Item -LiteralPath $outputPath -Recurse -Force
    }

    Copy-Item -LiteralPath $fullPackageDir -Destination $outputPath -Recurse -Force

    $vcRedist = Join-Path $outputPath "vc_redist.x64.exe"
    if (Test-Path -LiteralPath $vcRedist) {
        Remove-Item -LiteralPath $vcRedist -Force
    }

    $translationsDir = Join-Path $outputPath "translations"
    if (Test-Path -LiteralPath $translationsDir) {
        Get-ChildItem -LiteralPath $translationsDir -File |
            Where-Object { $_.Name -ne "qt_zh_CN.qm" } |
            Remove-Item -Force
    }

    $sqlDriversDir = Join-Path $outputPath "sqldrivers"
    if (Test-Path -LiteralPath $sqlDriversDir) {
        Get-ChildItem -LiteralPath $sqlDriversDir -File |
            Where-Object { $_.Name -ne "qsqlite.dll" } |
            Remove-Item -Force
    }

    & (Join-Path $PSScriptRoot "deploy_vc_runtime.ps1") -PackageDir $outputPath

    $notesPath = Join-Path $outputPath "LITE_PACKAGE_NOTES.txt"
    @"
PasswordManager lite portable release

This package keeps all PasswordManager application features and includes the
required app-local Microsoft Visual C++ Runtime DLLs for Windows 10/11 x64.

Removed:
  - Microsoft Visual C++ Runtime installer (app-local runtime DLLs are included).
  - Qt translations other than Simplified Chinese.
  - SQL drivers other than SQLite.

Kept:
  - Graphics runtime fallback files.
  - Bundled 7-Zip command-line and GUI tools.

The release archive is generated from a clean staging directory. Local database,
configuration, log, and backup files are never included.
"@ | Set-Content -LiteralPath $notesPath -Encoding UTF8

    foreach ($dirName in $runtimeDirs) {
        $runtimeDir = Join-Path $outputPath $dirName
        if (-not (Test-Path -LiteralPath $runtimeDir)) {
            New-Item -ItemType Directory -Force -Path $runtimeDir | Out-Null
        }
    }

    Assert-NoRuntimeFiles -Root $outputPath

    $liteExeHash = (Get-FileHash -LiteralPath (Join-Path $outputPath "PasswordManager.exe") -Algorithm SHA256).Hash
    if ($buildExeHash -ne $liteExeHash) {
        throw "Lite package PasswordManager.exe does not match the current Release build output."
    }

    Write-ReleaseManifest -Root $outputPath -PackageName $zipFileName

    if (Test-Path -LiteralPath $zipPath) {
        Remove-Item -LiteralPath $zipPath -Force
    }
    if (Test-Path -LiteralPath $zipHashPath) {
        Remove-Item -LiteralPath $zipHashPath -Force
    }

    Compress-Archive -LiteralPath $outputPath -DestinationPath $zipPath -Force
    $zipHash = (Get-FileHash -LiteralPath $zipPath -Algorithm SHA256).Hash.ToLowerInvariant()
    "$zipHash  $zipFileName" | Set-Content -LiteralPath $zipHashPath -Encoding ASCII

    & (Join-Path $PSScriptRoot "verify_release_archive.ps1") `
        -ArchivePath $zipPath `
        -ExpectedRootName (Split-Path -Leaf $outputPath) `
        -ExpectedExePath $buildExePath `
        -ForbidVcRedist

    Write-Host "Lite release directory created: $outputPath"
    Write-Host "Lite release archive created: $zipPath"
    Write-Host "Lite release archive SHA256: $zipHashPath"
    Write-Host "Release EXE SHA256: $($buildExeHash.ToLowerInvariant())"
}
finally {
    if (Test-Path -LiteralPath $stagingRoot) {
        Remove-Item -LiteralPath $stagingRoot -Recurse -Force -ErrorAction SilentlyContinue
    }
    $runtimeRestored = $false
    try {
        if ($runtimeBackupReady) {
            Restore-RuntimeDirectories -BackupRoot $preserveRuntimeDir -TargetRoot $outputPath
        }
        $runtimeRestored = $true
    }
    finally {
        if ($runtimeRestored -and (Test-Path -LiteralPath $preserveRuntimeDir)) {
            Remove-Item -LiteralPath $preserveRuntimeDir -Recurse -Force -ErrorAction SilentlyContinue
        }
        Pop-Location
    }
}
