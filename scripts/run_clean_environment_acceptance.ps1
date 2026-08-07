param(
    [string]$InputDir = "C:\PasswordManagerInput",
    [string]$ResultsDir = "C:\PasswordManagerResults",
    [string]$WorkingRoot = "",
    [switch]$SkipGuiLaunch,
    [switch]$CloseGuiAfterInspection,
    [switch]$SkipExplorerLaunch
)

$ErrorActionPreference = "Stop"

$mappedInputAvailable = Test-Path -LiteralPath $InputDir -PathType Container
if (-not $mappedInputAvailable) {
    $InputDir = $PSScriptRoot
    $ResultsDir = Join-Path $InputDir "results"
}
if (-not (Test-Path -LiteralPath $ResultsDir -PathType Container)) {
    New-Item -ItemType Directory -Force -Path $ResultsDir | Out-Null
}
$sessionName = "Session-" + (Get-Date -Format "yyyyMMdd-HHmmss")
$sessionResultsDir = Join-Path $ResultsDir $sessionName
if (Test-Path -LiteralPath $sessionResultsDir) {
    $sessionResultsDir = Join-Path $ResultsDir ($sessionName + "-" + [System.Guid]::NewGuid().ToString("N"))
}
New-Item -ItemType Directory -Force -Path $sessionResultsDir | Out-Null
if (-not $WorkingRoot) {
    $desktop = [Environment]::GetFolderPath("Desktop")
    $WorkingRoot = Join-Path $desktop ("PasswordManager-Clean-Test-" + (Get-Date -Format "yyyyMMdd-HHmmss"))
}
$workingFullPath = [System.IO.Path]::GetFullPath($WorkingRoot)
if (Test-Path -LiteralPath $workingFullPath) {
    throw "Working directory already exists; refusing to overwrite it: $workingFullPath"
}
New-Item -ItemType Directory -Force -Path $workingFullPath | Out-Null

$checks = New-Object System.Collections.Generic.List[object]
function Add-Check {
    param(
        [string]$Name,
        [bool]$Passed,
        [string]$Details
    )
    $checks.Add([pscustomobject]@{
        Name = $Name
        Passed = $Passed
        Details = $Details
    })
}

function Test-Manifest {
    param(
        [string]$Root,
        [string]$ManifestName
    )

    $manifestPath = Join-Path $Root $ManifestName
    if (-not (Test-Path -LiteralPath $manifestPath -PathType Leaf)) {
        return [pscustomobject]@{ Passed = $false; Details = "Missing $ManifestName" }
    }

    $rootPrefix = [System.IO.Path]::GetFullPath($Root)
    if (-not $rootPrefix.EndsWith([System.IO.Path]::DirectorySeparatorChar)) {
        $rootPrefix += [System.IO.Path]::DirectorySeparatorChar
    }
    $entryCount = 0
    foreach ($line in Get-Content -LiteralPath $manifestPath) {
        if ($line -notmatch "^([0-9a-fA-F]{64})  (.+)$") {
            continue
        }
        $entryCount++
        $expectedHash = $Matches[1]
        $relativePath = $Matches[2]
        $targetPath = [System.IO.Path]::GetFullPath((Join-Path $Root $relativePath))
        if (-not $targetPath.StartsWith($rootPrefix, [System.StringComparison]::OrdinalIgnoreCase)) {
            return [pscustomobject]@{ Passed = $false; Details = "Manifest path escapes root: $relativePath" }
        }
        if (-not (Test-Path -LiteralPath $targetPath -PathType Leaf)) {
            return [pscustomobject]@{ Passed = $false; Details = "Manifest file is missing: $relativePath" }
        }
        $actualHash = (Get-FileHash -LiteralPath $targetPath -Algorithm SHA256).Hash
        if ($actualHash -ne $expectedHash) {
            return [pscustomobject]@{ Passed = $false; Details = "Manifest hash mismatch: $relativePath" }
        }
    }
    if ($entryCount -eq 0) {
        return [pscustomobject]@{ Passed = $false; Details = "$ManifestName contains no file entries" }
    }
    return [pscustomobject]@{ Passed = $true; Details = "$entryCount entries passed" }
}

$zipPath = Join-Path $InputDir "PasswordManager-lite.zip"
$sidecarPath = "$zipPath.sha256"
if (-not (Test-Path -LiteralPath $zipPath -PathType Leaf)) {
    throw "Lite release archive was not found: $zipPath"
}

$os = Get-CimInstance Win32_OperatingSystem
$computer = Get-CimInstance Win32_ComputerSystem
Add-Check "64-bit Windows" ([Environment]::Is64BitOperatingSystem) ("{0}; {1}" -f $os.Caption, $os.Version)

$archiveHash = (Get-FileHash -LiteralPath $zipPath -Algorithm SHA256).Hash.ToLowerInvariant()
$sidecarMatches = (Test-Path -LiteralPath $sidecarPath -PathType Leaf) -and ((Get-Content -LiteralPath $sidecarPath -Raw) -match [regex]::Escape($archiveHash))
Add-Check "Lite ZIP SHA256" $sidecarMatches $archiveHash

$extractRoot = Join-Path $workingFullPath "package"
Expand-Archive -LiteralPath $zipPath -DestinationPath $extractRoot -Force
$packageRoot = Join-Path $extractRoot "PasswordManager-lite-portable"
Add-Check "ZIP root directory" (Test-Path -LiteralPath $packageRoot -PathType Container) $packageRoot
if (-not (Test-Path -LiteralPath $packageRoot -PathType Container)) {
    throw "Unexpected release archive layout."
}

$runtimeFiles = foreach ($dirName in @("data", "config", "logs", "backup")) {
    $runtimeDir = Join-Path $packageRoot $dirName
    if (Test-Path -LiteralPath $runtimeDir) {
        Get-ChildItem -LiteralPath $runtimeDir -Recurse -File
    }
}
Add-Check "ZIP contains no runtime data" (@($runtimeFiles).Count -eq 0) ("File count: {0}" -f @($runtimeFiles).Count)

$requiredRuntime = @("MSVCP140.dll", "MSVCP140_1.dll", "MSVCP140_2.dll", "VCRUNTIME140.dll", "VCRUNTIME140_1.dll")
$missingRuntime = @($requiredRuntime | Where-Object { -not (Test-Path -LiteralPath (Join-Path $packageRoot $_) -PathType Leaf) })
$runtimeDetails = if ($missingRuntime.Count) { "Missing: " + ($missingRuntime -join ", ") } else { "All 5 DLLs are present" }
Add-Check "App-local VC++ Runtime" ($missingRuntime.Count -eq 0) $runtimeDetails

$runtimeManifest = Test-Manifest -Root $packageRoot -ManifestName "VC_RUNTIME_MANIFEST.txt"
Add-Check "VC++ Runtime manifest" $runtimeManifest.Passed $runtimeManifest.Details
$releaseManifest = Test-Manifest -Root $packageRoot -ManifestName "RELEASE_MANIFEST.txt"
Add-Check "Release manifest" $releaseManifest.Passed $releaseManifest.Details

$systemRuntime = @($requiredRuntime | Where-Object { Test-Path -LiteralPath (Join-Path $env:WINDIR "System32\$_") })
$developerTools = @("cl.exe", "cmake.exe", "qmake.exe") | Where-Object { Get-Command $_ -ErrorAction SilentlyContinue }

$sevenZip = Join-Path $packageRoot "tools\7zip\7z.exe"
$sevenZipOutput = & $sevenZip i 2>&1
$sevenZipPassed = $LASTEXITCODE -eq 0 -and ($sevenZipOutput.Count -gt 0)
Add-Check "Bundled 7-Zip probe" $sevenZipPassed ("Exit code: {0}" -f $LASTEXITCODE)

$unicodeName = "$([char]0x4e2d)$([char]0x6587)-Unicode"
$testDataDir = Join-Path $workingFullPath ("AcceptanceData-" + $unicodeName)
New-Item -ItemType Directory -Force -Path $testDataDir | Out-Null
$payloadPath = Join-Path $testDataDir ("payload-" + $unicodeName + ".txt")
Add-Check "Unicode path round-trip" ((Split-Path -Leaf $testDataDir).Contains($unicodeName)) "Contains U+4E2D and U+6587"
@"
PasswordManager Phase59 clean-environment acceptance payload.
Passwords used only for disposable acceptance archives:
  password-1234.7z = 1234
  password-5678.zip = 5678
"@ | Set-Content -LiteralPath $payloadPath -Encoding UTF8

$noPasswordArchive = Join-Path $testDataDir "no-password.7z"
$passwordArchive = Join-Path $testDataDir "password-1234.7z"
$zipPasswordArchive = Join-Path $testDataDir "password-5678.zip"
& $sevenZip a -y $noPasswordArchive $payloadPath | Out-Null
$createNoPasswordExit = $LASTEXITCODE
& $sevenZip a -y -p1234 -mhe=on $passwordArchive $payloadPath | Out-Null
$createPasswordExit = $LASTEXITCODE
& $sevenZip a -y -p5678 $zipPasswordArchive $payloadPath | Out-Null
$createZipPasswordExit = $LASTEXITCODE
Add-Check "Create acceptance archives" ($createNoPasswordExit -eq 0 -and $createPasswordExit -eq 0 -and $createZipPasswordExit -eq 0) "No-password 7z, password 7z, password zip"

& $sevenZip t $noPasswordArchive | Out-Null
$noPasswordExit = $LASTEXITCODE
& $sevenZip t -p1234 $passwordArchive | Out-Null
$correctPasswordExit = $LASTEXITCODE
$previousErrorActionPreference = $ErrorActionPreference
try {
    $ErrorActionPreference = "SilentlyContinue"
    & $sevenZip t -p0000 $passwordArchive 2>$null | Out-Null
    $wrongPasswordExit = $LASTEXITCODE
}
finally {
    $ErrorActionPreference = $previousErrorActionPreference
}
Add-Check "7-Zip password scenarios" ($noPasswordExit -eq 0 -and $correctPasswordExit -eq 0 -and $wrongPasswordExit -ne 0) ("No password={0}; correct password={1}; wrong password={2}" -f $noPasswordExit, $correctPasswordExit, $wrongPasswordExit)

$appPath = Join-Path $packageRoot "PasswordManager.exe"
$smokeProcess = Start-Process -FilePath $appPath -ArgumentList "--smoke-test" -Wait -PassThru
Add-Check "PasswordManager smoke-test" ($smokeProcess.ExitCode -eq 0) ("Exit code: {0}" -f $smokeProcess.ExitCode)

$appProcess = $null
if (-not $SkipGuiLaunch) {
    $appStartParameters = @{
        FilePath = $appPath
        WorkingDirectory = $packageRoot
        PassThru = $true
    }
    if ($CloseGuiAfterInspection) {
        $appStartParameters.WindowStyle = "Minimized"
    }
    $appProcess = Start-Process @appStartParameters
    $runtimeModules = @()
    for ($attempt = 0; $attempt -lt 20; $attempt++) {
        Start-Sleep -Milliseconds 500
        if ($appProcess.HasExited) {
            break
        }
        try {
            $appProcess.Refresh()
            $runtimeModules = @($appProcess.Modules | Where-Object { $requiredRuntime -contains $_.ModuleName })
            if ($runtimeModules.Count -eq $requiredRuntime.Count) {
                break
            }
        }
        catch {
            $runtimeModules = @()
        }
    }
    if ($appProcess.HasExited) {
        Add-Check "GUI launch" $false ("Process exited early with code {0}" -f $appProcess.ExitCode)
        Add-Check "Loaded VC++ Runtime source" $false "GUI process exited before module inspection"
    } else {
        Add-Check "GUI launch" $true "Process is running"
        try {
            $packagePrefix = [System.IO.Path]::GetFullPath($packageRoot)
            if (-not $packagePrefix.EndsWith([System.IO.Path]::DirectorySeparatorChar)) {
                $packagePrefix += [System.IO.Path]::DirectorySeparatorChar
            }
            $nonLocalModules = @($runtimeModules | Where-Object {
                -not ([System.IO.Path]::GetFullPath($_.FileName).StartsWith($packagePrefix, [System.StringComparison]::OrdinalIgnoreCase))
            })
            $runtimeSourcePassed = $runtimeModules.Count -eq $requiredRuntime.Count -and $nonLocalModules.Count -eq 0
            $runtimeSourceDetails = "Loaded {0}/{1} required DLLs from the application directory" -f $runtimeModules.Count, $requiredRuntime.Count
            Add-Check "Loaded VC++ Runtime source" $runtimeSourcePassed $runtimeSourceDetails
        }
        catch {
            Add-Check "Loaded VC++ Runtime source" $false $_.Exception.Message
        }
    }
}

$manualChecklistSource = Join-Path $InputDir "MANUAL_CHECKLIST.md"
$manualChecklistTarget = Join-Path $sessionResultsDir "MANUAL_CHECKLIST.md"
if (Test-Path -LiteralPath $manualChecklistSource) {
    Copy-Item -LiteralPath $manualChecklistSource -Destination $manualChecklistTarget -Force
}

$reportPath = Join-Path $sessionResultsDir "Phase59-Automatic-Result.md"
$passedCount = @($checks | Where-Object Passed).Count
$failedCount = @($checks | Where-Object { -not $_.Passed }).Count
$report = @(
    "# Phase59 Automatic Acceptance Result",
    "",
    "- Time: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss zzz')",
    "- System: $($os.Caption) $($os.Version)",
    "- Architecture: $($os.OSArchitecture)",
    "- Memory: $([math]::Round($computer.TotalPhysicalMemory / 1GB, 2)) GiB",
    "- VC++ DLLs already present in System32: $($systemRuntime.Count)/$($requiredRuntime.Count)",
    "- Developer tools detected: $(if ($developerTools) { $developerTools -join ', ' } else { 'None' })",
    "- Working directory: $workingFullPath",
    "- Lite ZIP SHA256: $archiveHash",
    "- Passed: $passedCount",
    "- Failed: $failedCount",
    "",
    "| Check | Result | Details |",
    "| --- | --- | --- |"
)
$report += $checks | ForEach-Object {
    $details = ([string]$_.Details).Replace("|", "\|").Replace("`r", " ").Replace("`n", " ")
    "| $($_.Name) | $(if ($_.Passed) { 'PASS' } else { 'FAIL' }) | $details |"
}
$report | Set-Content -LiteralPath $reportPath -Encoding UTF8

Write-Host ""
Write-Host "Phase59 automatic acceptance finished."
Write-Host "Passed: $passedCount"
Write-Host "Failed: $failedCount"
Write-Host "Report: $reportPath"
Write-Host "Test archives: $testDataDir"

if (-not $SkipGuiLaunch -and -not $SkipExplorerLaunch -and $appProcess -and -not $appProcess.HasExited) {
    Start-Process -FilePath "explorer.exe" -ArgumentList $testDataDir
}
if ($CloseGuiAfterInspection -and $appProcess -and -not $appProcess.HasExited) {
    Stop-Process -Id $appProcess.Id -Force
    $appProcess.WaitForExit()
}

if ($failedCount -ne 0) {
    exit 1
}
