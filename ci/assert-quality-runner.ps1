param(
    [switch]$RequireClangTidy,
    [switch]$RequireCoverageTools
)

function Resolve-VcpkgRoot {
    if (-not [string]::IsNullOrWhiteSpace($env:VCPKG_ROOT) -and (Test-Path $env:VCPKG_ROOT)) {
        return $env:VCPKG_ROOT
    }

    $userRoot = [Environment]::GetEnvironmentVariable('VCPKG_ROOT', 'User')
    if (-not [string]::IsNullOrWhiteSpace($userRoot) -and (Test-Path $userRoot)) {
        return $userRoot
    }

    $machineRoot = [Environment]::GetEnvironmentVariable('VCPKG_ROOT', 'Machine')
    if (-not [string]::IsNullOrWhiteSpace($machineRoot) -and (Test-Path $machineRoot)) {
        return $machineRoot
    }

    if (Test-Path 'P:\vcpkg') {
        return 'P:\vcpkg'
    }

    return $null
}

$minimumCmakeVersion = [Version]'4.0.1'
$cmakeCommand = Get-Command cmake -ErrorAction SilentlyContinue
if (-not $cmakeCommand) {
    throw "cmake was not found on the runner. Install CMake 4.0.1 or newer on the self-hosted Windows runner."
}

$cmakeVersionLine = (& $cmakeCommand.Path --version | Select-Object -First 1)
if ($cmakeVersionLine -notmatch 'cmake version ([0-9]+\.[0-9]+\.[0-9]+)') {
    throw "Could not determine CMake version. Output: $cmakeVersionLine"
}

$cmakeVersion = [Version]$Matches[1]
if ($cmakeVersion -lt $minimumCmakeVersion) {
    throw "CMake $cmakeVersion is too old. Minimum required is $minimumCmakeVersion."
}

if ([string]::IsNullOrWhiteSpace($env:VCPKG_ROOT) -or !(Test-Path $env:VCPKG_ROOT)) {
    $env:VCPKG_ROOT = Resolve-VcpkgRoot
}

if ([string]::IsNullOrWhiteSpace($env:VCPKG_ROOT)) {
    throw "VCPKG_ROOT is not set on the self-hosted runner. Install vcpkg once on the runner and set the variable permanently."
}

$toolchainFile = Join-Path $env:VCPKG_ROOT 'scripts\buildsystems\vcpkg.cmake'
if (!(Test-Path $toolchainFile)) {
    throw "The vcpkg toolchain was not found at '$toolchainFile'. Check VCPKG_ROOT on the self-hosted runner."
}

if ($RequireClangTidy -and -not (Get-Command clang-tidy -ErrorAction SilentlyContinue)) {
    throw "clang-tidy was not found on the self-hosted runner. Install LLVM/clang-tidy on the runner."
}

if ($RequireCoverageTools) {
    foreach ($tool in @('clang-cl.exe', 'llvm-profdata.exe', 'llvm-cov.exe')) {
        if (-not (Get-Command $tool -ErrorAction SilentlyContinue)) {
            throw "$tool was not found on the self-hosted runner. Install the LLVM toolset on the runner."
        }
    }
}

Write-Host "Runner prerequisites ok."
Write-Host "Using cmake: $cmakeVersion"
Write-Host "Using VCPKG_ROOT: $env:VCPKG_ROOT"
