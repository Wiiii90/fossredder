[Setup]
AppName=FossRedder
AppVersion={#Version}
DefaultDirName={autopf}\FossRedder
DefaultGroupName=FossRedder
OutputBaseFilename=fossredder-{#Version}
OutputDir={#OutputDir}
Compression=lzma
SolidCompression=yes
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64

[Files]
Source: "{#Staging}\*"; DestDir: "{app}"; Flags: recursesubdirs createallsubdirs ignoreversion

[Icons]
Name: "{group}\FOSSRedder"; Filename: "{app}\bin\fossredder.exe"

[Run]
Filename: "{app}\bin\fossredder.exe"; Description: "Launch FossRedder"; Flags: nowait postinstall skipifsilent
