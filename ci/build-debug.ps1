param(
    [string]$ConfigurePreset = "x64-vcpkg-tests",
    [string]$BuildPreset = "x64-debug-vcpkg-tests"
)

Write-Host "Configuring preset: $ConfigurePreset"
cmake --preset $ConfigurePreset
if ($LASTEXITCODE -ne 0) { throw "CMake configuration failed." }

Write-Host "Building preset: $BuildPreset"
cmake --build --preset $BuildPreset
if ($LASTEXITCODE -ne 0) { throw "Build failed." }

Write-Host "Done. To run tests: .\\ci\\run-tests.ps1 -BuildDir .build\\$ConfigurePreset"