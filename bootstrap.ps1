param(
    [switch]$Install = $true,
    [switch]$CleanBuild = $false
)

Write-Host "Initializing git submodules..."
git submodule update --init --recursive

# Remove Visual Studio local workspace folder to clear per-user CMake/VS settings
$VsDir = Join-Path $PSScriptRoot ".vs"
if (Test-Path $VsDir) {
    Write-Host "Removing local Visual Studio workspace directory '.vs' to clear cached settings..."
    Remove-Item -LiteralPath $VsDir -Recurse -Force -ErrorAction SilentlyContinue
}

$BuildDir = Join-Path $PSScriptRoot "build"

function Remove-BuildDir {
    param($Reason)
    if (Test-Path $BuildDir) {
        Write-Host $Reason
        Write-Host "Removing build directory to ensure a clean CMake configure..."
        Remove-Item -LiteralPath $BuildDir -Recurse -Force -ErrorAction Stop
    }
}

function Normalize-PathForComparison {
    param($path)
    return ($path -replace "\\","/")
}

$NormalizedSource = Normalize-PathForComparison $PSScriptRoot

# Perform build cleanup before running vcpkg to avoid vcpkg creating caches pointing to the old path
if ($CleanBuild -and (Test-Path $BuildDir)) {
    Remove-BuildDir "CleanBuild requested."
}
else {
    if (Test-Path $BuildDir) {
        $caches = Get-ChildItem -Path $BuildDir -Filter CMakeCache.txt -Recurse -ErrorAction SilentlyContinue
        $mismatchFound = $false
        foreach ($cache in $caches) {
            try {
                $content = Get-Content -Raw -ErrorAction Stop $cache.FullName
            }
            catch {
                continue
            }
            $contentNormalized = $content -replace "\\","/"
            $hasCurrentPath = $contentNormalized -match [regex]::Escape($NormalizedSource)
            $hasAnyDriveAbsolute = $content -match '[A-Za-z]:[\\/]'

            if ($hasAnyDriveAbsolute -and -not $hasCurrentPath) {
                Write-Host "Found CMake cache at '$($cache.FullName)' that references a different absolute path than the current source path '$PSScriptRoot'."
                $mismatchFound = $true
                break
            }
        }
        if ($mismatchFound) {
            Remove-BuildDir "Detected CMake cache mismatch after repository move."
        }
        else {
            $CMakeCache = Join-Path $BuildDir "CMakeCache.txt"
            if (Test-Path $CMakeCache) {
                Write-Host "Detected existing CMakeCache.txt. Removing to avoid stale state..."
                Remove-Item -LiteralPath $CMakeCache -Force -ErrorAction SilentlyContinue
            }
            $CMakeFilesDir = Join-Path $BuildDir "CMakeFiles"
            if (Test-Path $CMakeFilesDir) {
                Write-Host "Removing existing CMakeFiles directory to avoid stale build state..."
                Remove-Item -LiteralPath $CMakeFilesDir -Recurse -Force -ErrorAction SilentlyContinue
            }
        }
    }
}

# Additionally, remove any CMakeCache.txt anywhere in the repository that references an absolute path
# different from the current source root (handles cases like third_party/vcpkg buildtrees)
Write-Host "Scanning repository for stale CMakeCache.txt files referencing other absolute source paths..."
$allCaches = Get-ChildItem -Path $PSScriptRoot -Filter CMakeCache.txt -Recurse -ErrorAction SilentlyContinue
foreach ($cache in $allCaches) {
    try {
        $content = Get-Content -Raw -ErrorAction Stop $cache.FullName
    }
    catch {
        continue
    }
    $contentNormalized = $content -replace "\\","/"
    $hasCurrentPath = $contentNormalized -match [regex]::Escape($NormalizedSource)
    $hasAnyDriveAbsolute = $content -match '[A-Za-z]:[\/]'
    if ($hasAnyDriveAbsolute -and -not $hasCurrentPath) {
        Write-Host "Removing stale cache: $($cache.FullName) (references other absolute path)."
        Remove-Item -LiteralPath $cache.FullName -Force -ErrorAction SilentlyContinue
    }
}

# Ensure we operate from repository root so vcpkg resolves files relative to PSScriptRoot
Push-Location $PSScriptRoot
try {
    # Export VCPKG_ROOT for the current process to avoid picking up stale environment values
    $env:VCPKG_ROOT = Join-Path $PSScriptRoot "third_party\vcpkg"

    $VCPKG = Join-Path $PSScriptRoot "third_party\vcpkg\vcpkg.exe"
    if (-not (Test-Path $VCPKG)) {
        Write-Host "Bootstrapping vcpkg..."
        & "third_party\vcpkg\bootstrap-vcpkg.bat"
    }

    if ($Install) {
        Write-Host "Installing vcpkg manifest packages..."
        # Explicitly pass --vcpkg-root to ensure vcpkg uses the repository third_party copy
        & $VCPKG install --triplet x64-windows --vcpkg-root "$PSScriptRoot\third_party\vcpkg"
    }

    Write-Host "Configuring CMake (vcpkg toolchain)..."
    cmake -S $PSScriptRoot -B "$PSScriptRoot\build" -G "Ninja" `
        -DCMAKE_TOOLCHAIN_FILE="$PSScriptRoot\third_party\vcpkg\scripts\buildsystems\vcpkg.cmake" `
        -DVCPKG_TARGET_TRIPLET="x64-windows" `
        -DVCPKG_INSTALLED_DIR="$PSScriptRoot\vcpkg_installed"

    Write-Host "Building..."
    cmake --build "$PSScriptRoot\build" --config Release -v
}
finally {
    Pop-Location
}
