# bootstrap.ps1 - convenience to get vcpkg
param()
$dest = Join-Path $PSScriptRoot '..\third_party\vcpkg'
if(Test-Path $dest) { Write-Host 'vcpkg already present' ; exit 0 }
Write-Host 'Downloading vcpkg...'
Invoke-WebRequest -Uri 'https://github.com/microsoft/vcpkg/archive/refs/heads/master.zip' -OutFile "$PSScriptRoot\vcpkg.zip"
Expand-Archive -LiteralPath "$PSScriptRoot\vcpkg.zip" -DestinationPath "$PSScriptRoot\..\third_party"
Move-Item -Path "$PSScriptRoot\..\third_party\vcpkg-master" -Destination $dest
Write-Host 'Bootstrap vcpkg manually by running bootstrap-vcpkg.bat inside the vcpkg folder'