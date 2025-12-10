; Inno Setup script for Xeno CPU utility 1.4.1
; Usage: Publish the project into the `publish` folder, place the WebView2 bootstrapper
; inside the publish folder (optional), then open this script in Inno Setup and compile.

[Setup]
AppName=Xeno CPU utility
AppVersion=1.4.1
DefaultDirName={pf}\Xeno CPU utility
; Require elevation to install into Program Files and write to HKLM (Add/Remove Programs)
PrivilegesRequired=admin
DefaultGroupName=Xeno CPU utility
DisableProgramGroupPage=no
OutputBaseFilename=XenoCPUUtility_1.4.1_Installer
Compression=lzma
SolidCompression=yes
OutputDir=Output
; Optional installer appearance customization:
; To show a custom installer icon and wizard images, place an .ico/.bmp files alongside this script and set the paths below.
; Example (uncomment and update paths if you add these files):
SetupIconFile=installer.ico
; WizardImageFile=installer_wide.bmp
; WizardSmallImageFile=installer_small.bmp
; When `LicenseFile` is present, the installer shows a License page during setup.
LicenseFile=LICENSE.rtf

; --- FILES ---
; Copy all published files from the publish folder into the application folder
[Files]
Source: "..\publish\*"; DestDir: "{app}"; Flags: recursesubdirs createallsubdirs
; Explicitly include the www folder and its contents
Source: "..\publish\www\*"; DestDir: "{app}\www"; Flags: recursesubdirs createallsubdirs

; Includes the WebView2 bootstrapper and single-file executable.


[Run]
; Run the WebView2 bootstrapper silently if it exists in the installed app folder
Filename: "{app}\MicrosoftEdgeWebView2RuntimeInstallerX64.exe"; Parameters: "/silent /install"; StatusMsg: "Installing WebView2 runtime..."; Flags: runhidden waituntilterminated; Check: FileExists(ExpandConstant('{app}\MicrosoftEdgeWebView2RuntimeInstallerX64.exe'))

; Launch the application at the end of installation (postinstall)
Filename: "{app}\XenoCPUUtility.exe"; Description: "Xeno CPU utility 1.3.0"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: filesandordirs; Name: "{localappdata}\CPUUtilityHybrid"

[Tasks]
; Optional desktop shortcut
Name: "desktopicon"; Description: "Create a &desktop icon"; GroupDescription: "Additional icons:"; Flags: unchecked

[Icons]
Name: "{group}\Xeno CPU utility"; Filename: "{app}\XenoCPUUtility.exe"
Name: "{group}\Uninstall Xeno CPU utility"; Filename: "{uninstallexe}"
Name: "{commondesktop}\Xeno CPU utility"; Filename: "{app}\XenoCPUUtility.exe"; Tasks: desktopicon

[Setup]
; Uninstall display icon (shows in Add/Remove Programs)
UninstallDisplayIcon={app}\XenoCPUUtility.exe
