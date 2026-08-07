param(
    [Parameter(Mandatory = $true)]
    [string]$PackageDir,
    [string]$ReportPath = ""
)

$ErrorActionPreference = "Stop"

$packagePath = [System.IO.Path]::GetFullPath($PackageDir)
if (-not (Test-Path -LiteralPath $packagePath -PathType Container)) {
    throw "Package directory was not found: $packagePath"
}

$vswhere = "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path -LiteralPath $vswhere)) {
    throw "vswhere.exe was not found. Install Visual Studio 2022 with Desktop development with C++."
}

$vsInstall = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vsInstall) {
    throw "Visual Studio 2022 with MSVC C++ tools was not found."
}

$dumpbin = Get-ChildItem -LiteralPath (Join-Path $vsInstall "VC\Tools\MSVC") -Recurse -Filter "dumpbin.exe" |
    Where-Object { $_.FullName -like "*\Hostx64\x64\dumpbin.exe" } |
    Sort-Object FullName -Descending |
    Select-Object -First 1
if (-not $dumpbin) {
    throw "The x64 dumpbin.exe tool was not found."
}

$redistDir = Get-ChildItem -LiteralPath (Join-Path $vsInstall "VC\Redist\MSVC") -Recurse -Directory -Filter "Microsoft.VC143.CRT" |
    Where-Object { $_.FullName -like "*\x64\Microsoft.VC143.CRT" } |
    Sort-Object FullName -Descending |
    Select-Object -First 1
if (-not $redistDir) {
    throw "The x64 Microsoft.VC143.CRT redistributable directory was not found."
}

function Get-RuntimeDependencies {
    param([string]$Root)

    $dependencies = New-Object System.Collections.Generic.HashSet[string]([System.StringComparer]::OrdinalIgnoreCase)
    $peFiles = Get-ChildItem -LiteralPath $Root -Recurse -File |
        Where-Object { $_.Extension -in @(".exe", ".dll") -and $_.Name -ne "vc_redist.x64.exe" }

    foreach ($file in $peFiles) {
        $output = & $dumpbin.FullName /dependents $file.FullName 2>$null
        if ($LASTEXITCODE -ne 0) {
            throw "Failed to inspect PE dependencies: $($file.FullName)"
        }

        foreach ($line in $output) {
            if ($line -match "^\s+((?:msvcp|vcruntime|concrt|vccorlib)[A-Za-z0-9_]*\.dll)\s*$") {
                [void]$dependencies.Add($Matches[1])
            }
        }
    }

    return @($dependencies | Sort-Object)
}

for ($pass = 0; $pass -lt 10; $pass++) {
    $dependencies = Get-RuntimeDependencies -Root $packagePath
    $copiedThisPass = $false

    foreach ($dependency in $dependencies) {
        $source = Join-Path $redistDir.FullName $dependency
        if (-not (Test-Path -LiteralPath $source -PathType Leaf)) {
            throw "Required VC++ Runtime DLL was not found in the official x64 redist directory: $dependency"
        }

        $headers = & $dumpbin.FullName /headers $source 2>$null
        if ($LASTEXITCODE -ne 0 -or -not ($headers -match "8664 machine \(x64\)")) {
            throw "VC++ Runtime DLL is not x64: $source"
        }

        $target = Join-Path $packagePath $dependency
        $sourceHash = (Get-FileHash -LiteralPath $source -Algorithm SHA256).Hash
        $targetHash = if (Test-Path -LiteralPath $target) {
            (Get-FileHash -LiteralPath $target -Algorithm SHA256).Hash
        } else {
            ""
        }

        if ($sourceHash -ne $targetHash) {
            Copy-Item -LiteralPath $source -Destination $target -Force
            $copiedThisPass = $true
        }
    }

    if (-not $copiedThisPass) {
        break
    }

    if ($pass -eq 9) {
        throw "VC++ Runtime dependency closure did not stabilize."
    }
}

$finalDependencies = Get-RuntimeDependencies -Root $packagePath
foreach ($dependency in $finalDependencies) {
    $target = Join-Path $packagePath $dependency
    if (-not (Test-Path -LiteralPath $target -PathType Leaf)) {
        throw "VC++ Runtime dependency is not deployed beside the application: $dependency"
    }
}

if (-not $ReportPath) {
    $ReportPath = Join-Path $packagePath "VC_RUNTIME_MANIFEST.txt"
}
$reportFullPath = [System.IO.Path]::GetFullPath($ReportPath)
$reportLines = @(
    "PasswordManager app-local VC++ Runtime manifest",
    "Architecture: x64",
    "MSVC redist version: $($redistDir.Parent.Parent.Name)",
    "GeneratedAt: $((Get-Date).ToUniversalTime().ToString('yyyy-MM-ddTHH:mm:ssZ'))",
    "",
    "Files:"
)
$reportLines += $finalDependencies | ForEach-Object {
    $target = Join-Path $packagePath $_
    $hash = (Get-FileHash -LiteralPath $target -Algorithm SHA256).Hash.ToLowerInvariant()
    "$hash  $_"
}
$reportLines | Set-Content -LiteralPath $reportFullPath -Encoding UTF8

Write-Host "VC++ Runtime dependency scan passed."
Write-Host "App-local VC++ Runtime files: $($finalDependencies.Count)"
Write-Host "VC++ Runtime manifest: $reportFullPath"
