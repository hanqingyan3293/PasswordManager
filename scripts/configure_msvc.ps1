param(
    [string]$QtPrefixPath = "",
    [string]$BuildDir = "build",
    [string]$BuildType = ""
)

$ErrorActionPreference = "Stop"

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

$cmake = Get-Command "cmake" -ErrorAction SilentlyContinue
$cmakePath = if ($cmake) { $cmake.Source } else { "C:\Program Files\CMake\bin\cmake.exe" }
if (-not (Test-Path $cmakePath)) {
    throw "cmake.exe was not found. Install CMake or add it to PATH."
}

$cmakeArgs = @(
    "-S", ".",
    "-B", $BuildDir,
    "-G", "Ninja"
)

if ($QtPrefixPath) {
    $cmakeArgs += "-DCMAKE_PREFIX_PATH=$QtPrefixPath"
}

if ($BuildType) {
    $cmakeArgs += "-DCMAKE_BUILD_TYPE=$BuildType"
}

$quotedArgs = ($cmakeArgs | ForEach-Object { "`"$_`"" }) -join " "
$configureCommand = "call `"$vcvars`" && `"$cmakePath`" $quotedArgs"
cmd.exe /c $configureCommand
