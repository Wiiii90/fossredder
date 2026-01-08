param(
    [string]$BuildDir = ".build/x64-vcpkg-tests",
    [ValidateSet('Debug','Release')][string]$Config = "Debug",
    [string]$OutDir = "coverage",
    [string]$OpenCppCoverageExe = "C:\Program Files\OpenCppCoverage\OpenCppCoverage.exe"
)

if (!(Test-Path $OpenCppCoverageExe)) { throw "OpenCppCoverage not found at: $OpenCppCoverageExe" }
if (!(Test-Path $OutDir)) { New-Item -ItemType Directory -Path $OutDir | Out-Null }

$unitExe = Join-Path -Path $BuildDir -ChildPath "bin\$Config\core_unit_tests.exe"
$integExe = Join-Path -Path $BuildDir -ChildPath "bin\$Config\core_interaction_tests.exe"

if (!(Test-Path $unitExe)) { throw "Unit test exe not found: $unitExe" }

Write-Host "Running OpenCppCoverage for unit tests"
& $OpenCppCoverageExe --sources "core/src" --export_type cobertura --output "$OutDir\coverage-unit.xml" -- $unitExe
if ($LASTEXITCODE -ne 0) { throw "OpenCppCoverage failed for unit tests." }

if (Test-Path $integExe) {
    Write-Host "Running OpenCppCoverage for interaction tests"
    & $OpenCppCoverageExe --sources "core/src" --export_type cobertura --output "$OutDir\coverage-interaction.xml" -- $integExe
    if ($LASTEXITCODE -ne 0) { throw "OpenCppCoverage failed for interaction tests." }
}

Write-Host "Coverage outputs: $OutDir"