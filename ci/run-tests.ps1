param(
    [string]$BuildDir = ".build/x64-vcpkg"
)

# Run unit tests
Write-Host "Running UNIT tests"
ctest --test-dir $BuildDir -L UNIT --output-on-failure
if ($LASTEXITCODE -ne 0) { throw "Unit tests failed." }

# Run interaction tests
Write-Host "Running INTERACTION tests"
ctest --test-dir $BuildDir -L INTERACTION --output-on-failure
if ($LASTEXITCODE -ne 0) { throw "Interaction tests failed." }

Write-Host "All tests passed."