$ErrorActionPreference = "Stop"

function Find-CommandPath {
    param(
        [Parameter(Mandatory = $true)][string]$Name,
        [string[]]$FallbackPaths = @()
    )

    $command = Get-Command $Name -ErrorAction SilentlyContinue
    if ($command) {
        return $command.Source
    }

    foreach ($path in $FallbackPaths) {
        if (Test-Path $path) {
            return $path
        }
    }

    return $null
}

function Write-ToolStatus {
    param(
        [Parameter(Mandatory = $true)][string]$Name,
        [string]$Path
    )

    if ($Path) {
        Write-Host ("{0}: {1}" -f $Name, $Path)
    } else {
        Write-Host ("{0}: <missing>" -f $Name)
    }
}

$vswhereDefault = "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
$vswhere = Find-CommandPath "vswhere"
if (-not $vswhere -and (Test-Path $vswhereDefault)) {
    $vswhere = $vswhereDefault
}

$vsInstall = $null
if ($vswhere) {
    $vsInstall = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
}

Write-ToolStatus "git" (Find-CommandPath "git")
Write-ToolStatus "cmake" (Find-CommandPath "cmake" @("C:\Program Files\CMake\bin\cmake.exe"))
Write-ToolStatus "ninja" (Find-CommandPath "ninja")
Write-ToolStatus "qmake" (Find-CommandPath "qmake")
Write-ToolStatus "qt-cmake" (Find-CommandPath "qt-cmake")
Write-ToolStatus "vswhere" $vswhere

if ($vsInstall) {
    Write-Host ("Visual Studio with MSVC: {0}" -f $vsInstall)
} else {
    Write-Host "Visual Studio with MSVC: <missing>"
}

$candidateQtRoots = @(
    "C:\Qt",
    "D:\Qt"
)

$qtCandidates = foreach ($root in $candidateQtRoots) {
    if (Test-Path $root) {
        Get-ChildItem -Path $root -Directory -Recurse -ErrorAction SilentlyContinue |
            Where-Object { Test-Path (Join-Path $_.FullName "lib\cmake\Qt6\Qt6Config.cmake") } |
            Select-Object -ExpandProperty FullName
    }
}

if ($qtCandidates) {
    Write-Host "Qt candidates:"
    $qtCandidates | ForEach-Object { Write-Host ("  {0}" -f $_) }
} else {
    Write-Host "Qt candidates: <missing>"
}

$sevenZip = Join-Path (Resolve-Path ".") "tools\7zip\7z.exe"
if (Test-Path $sevenZip) {
    Write-Host ("Bundled 7-Zip: {0}" -f $sevenZip)
} else {
    Write-Host "Bundled 7-Zip: <missing>"
}
