param(
    [string]$BuildDir = ".build/tests",
    [ValidateSet('Debug','Release')][string]$Config = "Release",
    [string[]]$Labels = @('UNIT', 'INTERACTION', 'PERSISTENCE_UNIT', 'DEBUG_UNIT')
)

foreach ($label in $Labels) {
    Write-Host "Running $label tests"
    ctest --test-dir $BuildDir -C $Config -L $label --output-on-failure
    if ($LASTEXITCODE -ne 0) { throw "Tests failed for label '$label'." }
}

Write-Host "All tests passed."