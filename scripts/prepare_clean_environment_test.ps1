param(
    [string]$LiteZipPath = "out/PasswordManager-0.1.0-win-x64-lite.zip",
    [string]$OutputDir = "out/clean-environment-test",
    [switch]$LaunchSandbox
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$outRoot = Join-Path $repoRoot "out"
$liteZipFullPath = [System.IO.Path]::GetFullPath((Join-Path $repoRoot $LiteZipPath))
$outputFullPath = [System.IO.Path]::GetFullPath((Join-Path $repoRoot $OutputDir))
$outPrefix = [System.IO.Path]::GetFullPath($outRoot)
if (-not $outPrefix.EndsWith([System.IO.Path]::DirectorySeparatorChar)) {
    $outPrefix += [System.IO.Path]::DirectorySeparatorChar
}
if (-not $outputFullPath.StartsWith($outPrefix, [System.StringComparison]::OrdinalIgnoreCase)) {
    throw "Clean-environment test output must stay under the project out directory: $outputFullPath"
}
if (-not (Test-Path -LiteralPath $liteZipFullPath -PathType Leaf)) {
    throw "Lite release archive was not found: $liteZipFullPath"
}

$sevenZip = Join-Path $repoRoot "tools\7zip\7z.exe"
if (-not (Test-Path -LiteralPath $sevenZip -PathType Leaf)) {
    throw "Bundled 7-Zip was not found: $sevenZip"
}
& $sevenZip t $liteZipFullPath | Out-Null
if ($LASTEXITCODE -ne 0) {
    throw "Lite release archive integrity check failed."
}

$sidecarPath = "$liteZipFullPath.sha256"
if (-not (Test-Path -LiteralPath $sidecarPath -PathType Leaf)) {
    throw "Lite release SHA256 sidecar was not found: $sidecarPath"
}
$archiveHash = (Get-FileHash -LiteralPath $liteZipFullPath -Algorithm SHA256).Hash.ToLowerInvariant()
$sidecarText = Get-Content -LiteralPath $sidecarPath -Raw
if ($sidecarText -notmatch [regex]::Escape($archiveHash)) {
    throw "Lite release SHA256 does not match its sidecar file."
}

if (-not (Test-Path -LiteralPath $outputFullPath)) {
    New-Item -ItemType Directory -Force -Path $outputFullPath | Out-Null
}
$inputDir = Join-Path $outputFullPath "input"
$resultsDir = Join-Path $outputFullPath "results"
if (Test-Path -LiteralPath $inputDir) {
    Remove-Item -LiteralPath $inputDir -Recurse -Force
}
New-Item -ItemType Directory -Force -Path $inputDir, $resultsDir | Out-Null

$portableZipName = "PasswordManager-lite.zip"
Copy-Item -LiteralPath $liteZipFullPath -Destination (Join-Path $inputDir $portableZipName) -Force
Copy-Item -LiteralPath $sidecarPath -Destination (Join-Path $inputDir "$portableZipName.sha256") -Force
Copy-Item -LiteralPath (Join-Path $PSScriptRoot "run_clean_environment_acceptance.ps1") -Destination $inputDir -Force
Copy-Item -LiteralPath (Join-Path $repoRoot "docs\123_Phase59_Manual_Checklist.md") -Destination (Join-Path $inputDir "MANUAL_CHECKLIST.md") -Force

$launcherPath = Join-Path $inputDir "RUN_ON_CLEAN_MACHINE.cmd"
@"
@echo off
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0run_clean_environment_acceptance.ps1"
echo.
echo Acceptance script finished. Press any key to close.
pause >nul
"@ | Set-Content -LiteralPath $launcherPath -Encoding ASCII

$inputXml = [System.Security.SecurityElement]::Escape($inputDir)
$resultsXml = [System.Security.SecurityElement]::Escape($resultsDir)
$wsbPath = Join-Path $outputFullPath "PasswordManager-Clean-Test.wsb"
@"
<Configuration>
  <VGpu>Disable</VGpu>
  <Networking>Disable</Networking>
  <ClipboardRedirection>Disable</ClipboardRedirection>
  <MemoryInMB>4096</MemoryInMB>
  <MappedFolders>
    <MappedFolder>
      <HostFolder>$inputXml</HostFolder>
      <SandboxFolder>C:\PasswordManagerInput</SandboxFolder>
      <ReadOnly>true</ReadOnly>
    </MappedFolder>
    <MappedFolder>
      <HostFolder>$resultsXml</HostFolder>
      <SandboxFolder>C:\PasswordManagerResults</SandboxFolder>
      <ReadOnly>false</ReadOnly>
    </MappedFolder>
  </MappedFolders>
  <LogonCommand>
    <Command>powershell.exe -NoProfile -ExecutionPolicy Bypass -File C:\PasswordManagerInput\run_clean_environment_acceptance.ps1</Command>
  </LogonCommand>
</Configuration>
"@ | Set-Content -LiteralPath $wsbPath -Encoding UTF8

$bundleInfoPath = Join-Path $outputFullPath "BUNDLE_INFO.txt"
$sandboxFeature = Get-WindowsOptionalFeature -Online -FeatureName "Containers-DisposableClientVM" -ErrorAction SilentlyContinue
$sandboxFeatureState = if ($sandboxFeature) { [string]$sandboxFeature.State } else { "Unavailable" }
@"
PasswordManager clean-environment acceptance bundle

GeneratedAt: $((Get-Date).ToUniversalTime().ToString('yyyy-MM-ddTHH:mm:ssZ'))
Lite archive SHA256: $archiveHash
Windows Sandbox feature state: $sandboxFeatureState

Windows Sandbox:
  Double-click PasswordManager-Clean-Test.wsb after Windows Sandbox is enabled.

Other clean Windows 10/11 x64 machine:
  Copy the complete input directory to the target machine.
  Double-click RUN_ON_CLEAN_MACHINE.cmd.

Results:
  Each run creates a timestamped session under the results directory.
  A normal clean machine creates timestamped sessions under input\results.
"@ | Set-Content -LiteralPath $bundleInfoPath -Encoding UTF8

Write-Host "Clean-environment test bundle created: $outputFullPath"
Write-Host "Lite archive SHA256: $archiveHash"
Write-Host "Windows Sandbox configuration: $wsbPath"
Write-Host "Windows Sandbox feature state: $sandboxFeatureState"

if ($LaunchSandbox) {
    $feature = Get-WindowsOptionalFeature -Online -FeatureName "Containers-DisposableClientVM" -ErrorAction SilentlyContinue
    if (-not $feature -or $feature.State -ne "Enabled") {
        throw "Windows Sandbox is not enabled. The test bundle was created, but no system setting was changed."
    }

    $sandboxCommand = Get-Command "WindowsSandbox.exe" -ErrorAction SilentlyContinue
    if (-not $sandboxCommand) {
        throw "WindowsSandbox.exe was not found even though the optional feature is enabled."
    }
    Start-Process -FilePath $wsbPath
}
