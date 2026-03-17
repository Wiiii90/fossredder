param(
    [string]$ConfigurePreset = "tests",
    [string]$BuildPreset = "debug-tests",
    [switch]$RunTests
)

if ([string]::IsNullOrWhiteSpace($env:VCPKG_ROOT)) {
    throw "VCPKG_ROOT ist nicht gesetzt. Installiere vcpkg einmalig an einem lokalen Pfad und setze danach die Benutzer-Umgebungsvariable VCPKG_ROOT dauerhaft auf dieses Verzeichnis."
}

$toolchainFile = Join-Path $env:VCPKG_ROOT "scripts\buildsystems\vcpkg.cmake"
if (!(Test-Path $toolchainFile)) {
    throw "Die vcpkg-Toolchain wurde unter '$toolchainFile' nicht gefunden. Prüfe VCPKG_ROOT und führe gegebenenfalls bootstrap-vcpkg erneut aus."
}

Write-Host "Using VCPKG_ROOT: $env:VCPKG_ROOT"
Write-Host "Configuring preset: $ConfigurePreset"
cmake --preset $ConfigurePreset
if ($LASTEXITCODE -ne 0) { throw "CMake configuration failed." }

Write-Host "Building preset: $BuildPreset"
cmake --build --preset $BuildPreset
if ($LASTEXITCODE -ne 0) { throw "Build failed." }

if ($RunTests) {
    & "$PSScriptRoot\run-tests.ps1" -BuildDir ".build\$ConfigurePreset" -Config Debug
    if ($LASTEXITCODE -ne 0) { throw "Debug test run failed." }
} else {
    Write-Host "Done. To run tests: .\\ci\\run-tests.ps1 -BuildDir .build\\$ConfigurePreset -Config Debug"
}
