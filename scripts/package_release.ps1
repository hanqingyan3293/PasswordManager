param(
    [string]$BuildDir = "build-release",
    [string]$OutputDir = "out/PasswordManager-portable",
    [string]$QtPrefixPath = "C:\Qt\6.8.3\msvc2022_64",
    [string]$Version = "0.1.0",
    [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$buildPath = Join-Path $repoRoot $BuildDir
$outputPath = Join-Path $repoRoot $OutputDir
$outRoot = Join-Path $repoRoot "out"
$exePath = Join-Path $buildPath "PasswordManager.exe"
$windeployqt = Join-Path $QtPrefixPath "bin\windeployqt.exe"
$zipFileName = "PasswordManager-$Version-win-x64-portable.zip"
$zipPath = Join-Path $outRoot $zipFileName
$zipHashPath = "$zipPath.sha256"
$preserveRuntimeDir = Join-Path $outRoot (".package-preserve-" + [System.Guid]::NewGuid().ToString("N"))
$runtimeDirs = @("data", "config", "logs", "backup")

$vswhere = "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path $vswhere)) {
    throw "vswhere.exe was not found. Install Visual Studio 2022 with Desktop development with C++."
}

$vsInstall = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vsInstall) {
    throw "Visual Studio 2022 with MSVC C++ tools was not found."
}

$vcvars = Join-Path $vsInstall "VC\Auxiliary\Build\vcvars64.bat"
if (-not (Test-Path $vcvars)) {
    throw "vcvars64.bat was not found under Visual Studio installation."
}

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
        throw "Refusing to clean output outside project out directory: $fullPath"
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

$runtimeBackupReady = $false
Push-Location $repoRoot
try {
    if (-not (Test-Path $windeployqt)) {
        throw "windeployqt.exe was not found: $windeployqt"
    }

    if (-not $SkipBuild) {
        & (Join-Path $PSScriptRoot "configure_msvc.ps1") -BuildDir $BuildDir -QtPrefixPath $QtPrefixPath -BuildType "Release"
        & (Join-Path $PSScriptRoot "build_msvc.ps1") -BuildDir $BuildDir
    }

    if (-not (Test-Path $exePath)) {
        throw "Build output was not found: $exePath"
    }

    Assert-UnderDirectory -Path $outputPath -Parent $outRoot
    if (Test-Path $outputPath) {
        New-Item -ItemType Directory -Force -Path $preserveRuntimeDir | Out-Null
        foreach ($dirName in $runtimeDirs) {
            $runtimeSource = Join-Path $outputPath $dirName
            if (Test-Path $runtimeSource) {
                Copy-Item -LiteralPath $runtimeSource -Destination (Join-Path $preserveRuntimeDir $dirName) -Recurse -Force
            }
        }
        $runtimeBackupReady = $true
    }

    if (Test-Path $outputPath) {
        Remove-Item -LiteralPath $outputPath -Recurse -Force
    }

    Assert-UnderDirectory -Path $zipPath -Parent $outRoot
    if (Test-Path $zipPath) {
        Remove-Item -LiteralPath $zipPath -Force
    }

    Assert-UnderDirectory -Path $zipHashPath -Parent $outRoot
    if (Test-Path $zipHashPath) {
        Remove-Item -LiteralPath $zipHashPath -Force
    }

    New-Item -ItemType Directory -Force -Path $outputPath | Out-Null
    Copy-Item -LiteralPath $exePath -Destination $outputPath -Force

    $deployedExe = Join-Path $outputPath "PasswordManager.exe"
    $deployCommand = "call `"$vcvars`" && `"$windeployqt`" --release --compiler-runtime `"$deployedExe`""
    cmd.exe /c $deployCommand
    if ($LASTEXITCODE -ne 0) {
        throw "windeployqt failed with exit code $LASTEXITCODE."
    }

    $release7zDir = Join-Path $outputPath "tools\7zip"
    New-Item -ItemType Directory -Force -Path $release7zDir | Out-Null

    $sevenZipFiles = @("7z.exe", "7zG.exe", "7zFM.exe", "7z.dll", "License.txt", "VERSION.txt", "SHA256SUMS.txt")
    foreach ($fileName in $sevenZipFiles) {
        $source = Join-Path $repoRoot "tools\7zip\$fileName"
        if (-not (Test-Path $source)) {
            throw "Bundled 7-Zip file was not found: $source"
        }
        Copy-Item -LiteralPath $source -Destination $release7zDir -Force
    }
    $release7zLangDir = Join-Path $release7zDir "Lang"
    New-Item -ItemType Directory -Force -Path $release7zLangDir | Out-Null
    Copy-Item -LiteralPath (Join-Path $repoRoot "tools\7zip\Lang\zh-cn.txt") -Destination $release7zLangDir -Force

    foreach ($dirName in $runtimeDirs) {
        New-Item -ItemType Directory -Force -Path (Join-Path $outputPath $dirName) | Out-Null
    }

    $runtimeFiles = foreach ($dirName in $runtimeDirs) {
        $runtimeDir = Join-Path $outputPath $dirName
        Get-ChildItem -LiteralPath $runtimeDir -Recurse -File -ErrorAction SilentlyContinue
    }
    if (@($runtimeFiles).Count -ne 0) {
        throw "Release staging contains runtime data files. Packaging was stopped."
    }

    & (Join-Path $PSScriptRoot "deploy_vc_runtime.ps1") -PackageDir $outputPath

    $readmePath = Join-Path $outputPath "README_RELEASE.txt"
    @"
PasswordManager portable release

Version:
  $Version

Run:
  PasswordManager.exe

Runtime directories:
  data    - local SQLite database
  config  - local configuration files
  logs    - local logs
  backup  - reserved backups

Bundled dependency:
  tools\7zip\7z.exe
  tools\7zip\7zG.exe
  tools\7zip\7zFM.exe
  tools\7zip\Lang\zh-cn.txt

This application is designed to use the bundled 7-Zip under its own program
directory. Do not delete tools\7zip.
"@ | Set-Content -Path $readmePath -Encoding UTF8

    $acceptancePath = Join-Path $outputPath "ACCEPTANCE_CHECKLIST.txt"
    @"
PasswordManager manual acceptance checklist

[ ] Start PasswordManager.exe from this directory.
[ ] Settings page shows bundled 7-Zip as available.
[ ] Add at least one plaintext password.
[ ] Scan a folder that contains zip/7z/rar archives.
[ ] Test one correct password and one wrong password.
[ ] Run smart matching against the test archives.
[ ] Confirm task queue states update.
[ ] Confirm successful archive/password history is recorded.
[ ] Extract one known-good archive to a selected output folder.
[ ] Confirm logs stay under this directory's logs folder.

Command checks:
  PasswordManager.exe --smoke-test
  tools\7zip\7z.exe i
"@ | Set-Content -Path $acceptancePath -Encoding UTF8

    $sourceUserManual = Join-Path $repoRoot "docs\46_User_Manual.md"
    if (Test-Path $sourceUserManual) {
        Copy-Item -LiteralPath $sourceUserManual -Destination (Join-Path $outputPath "USER_MANUAL.md") -Force
    }

    $sourceLicense = Join-Path $repoRoot "LICENSE"
    if (Test-Path $sourceLicense) {
        Copy-Item -LiteralPath $sourceLicense -Destination (Join-Path $outputPath "LICENSE") -Force
    }

    $manifestPath = Join-Path $outputPath "RELEASE_MANIFEST.txt"
    $outputPrefix = [System.IO.Path]::GetFullPath($outputPath)
    if (-not $outputPrefix.EndsWith([System.IO.Path]::DirectorySeparatorChar)) {
        $outputPrefix += [System.IO.Path]::DirectorySeparatorChar
    }

    $manifestLines = @(
        "PasswordManager release manifest",
        "Version: $Version",
        "Package: $zipFileName",
        "GeneratedAt: $((Get-Date).ToUniversalTime().ToString('yyyy-MM-ddTHH:mm:ssZ'))",
        "",
        "Files:"
    )

    $manifestLines += Get-ChildItem -LiteralPath $outputPath -Recurse -File |
        Where-Object { $_.FullName -ne $manifestPath } |
        Sort-Object FullName |
        ForEach-Object {
            $fullName = [System.IO.Path]::GetFullPath($_.FullName)
            $relative = $fullName.Substring($outputPrefix.Length)
            $hash = (Get-FileHash -Algorithm SHA256 -LiteralPath $_.FullName).Hash.ToLowerInvariant()
            "$hash  $relative"
        }

    $manifestLines | Set-Content -Path $manifestPath -Encoding UTF8

    Compress-Archive -LiteralPath $outputPath -DestinationPath $zipPath -Force
    $zipHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $zipPath).Hash.ToLowerInvariant()
    "$zipHash  $zipFileName" | Set-Content -Path $zipHashPath -Encoding ASCII

    & (Join-Path $PSScriptRoot "verify_release_archive.ps1") `
        -ArchivePath $zipPath `
        -ExpectedRootName (Split-Path -Leaf $outputPath) `
        -ExpectedExePath $exePath `
        -RequireVcRedist

    Write-Host "Release directory created: $outputPath"
    Write-Host "Release archive created: $zipPath"
    Write-Host "Release archive SHA256: $zipHashPath"
}
finally {
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
