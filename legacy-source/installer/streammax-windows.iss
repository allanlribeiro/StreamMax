; StreamMax Windows - instalador por usuario (sem privilegios administrativos)
; Preview W0: janela e navegacao em validacao; player de video ainda pendente.
#define MyAppName "StreamMax"
#define MyAppVersion "0.1.0"
#define MyAppPublisher "StreamMax"

[Setup]
AppId={{87B91E8D-F99C-40AD-BD38-A1BC6B65C4B1}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName=StreamMax Windows experimental {#MyAppVersion}
AppPublisher={#MyAppPublisher}
UninstallDisplayName=StreamMax (Windows experimental)
DefaultDirName={localappdata}\Programs\StreamMax
DefaultGroupName=StreamMax
PrivilegesRequired=lowest
AllowNoIcons=yes
UsePreviousAppDir=yes
OutputDir=..\build\windows\installer
OutputBaseFilename=StreamMax-Setup-Windows-x64
Compression=lzma
SolidCompression=yes
WizardStyle=modern
SetupIconFile=..\build\windows\streammax.ico
SetupLogging=yes
CloseApplications=yes
RestartApplications=no
MinVersion=10.0
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
VersionInfoVersion=0.1.0.0
DisableProgramGroupPage=no
ShowLanguageDialog=no

[Languages]
Name: "brazilianportuguese"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"

[Tasks]
Name: "desktopicon"; Description: "Criar atalho na area de trabalho"; GroupDescription: "Atalhos:"; Flags: unchecked

[Files]
Source: "..\build\windows\package\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\StreamMax"; Filename: "{app}\StreamMax.exe"; WorkingDir: "{app}"
Name: "{autodesktop}\StreamMax"; Filename: "{app}\StreamMax.exe"; WorkingDir: "{app}"; Tasks: desktopicon

[Run]
Filename: "{app}\StreamMax.exe"; Description: "Abrir StreamMax"; WorkingDir: "{app}"; Flags: nowait postinstall skipifsilent
