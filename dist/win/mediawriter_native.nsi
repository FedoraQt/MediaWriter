!include "MUI2.nsh"
!include "nsDialogs.nsh"

!addplugindir "plugins"

ManifestDPIAware true
XPStyle on

# If you change the names "app.exe", "logo.ico", or "license.rtf" you should do a search and replace
# they show up in a few places.
# All the other settings can be tweaked by editing the !defines at the top of this script

# These three must be defined from command line
#!define VERSIONMAJOR
#!define VERSIONMINOR
#!define VERSIONBUILD

!define APPNAME           "Fedora Media Writer"
!define /date CURRENTYEAR "%Y"
!define COMPANYNAME       "Fedora Project"
!define COPYRIGHT         "${COMPANYNAME} ${CURRENTYEAR}"
!define DESCRIPTION       "Tool to write Fedora images to flash drives"
!define FULLVERSION       "${VERSIONMAJOR}.${VERSIONMINOR}.${VERSIONBUILD}.0"

# VC++ Redistributable configuration
!define VCREDIST_FILE     "vc_redist.x64.exe"
!define VCREDIST_URL      "https://aka.ms/vs/17/release/vc_redist.x64.exe"

Name    "${APPNAME}"
Caption "${APPNAME} ${FULLVERSION}"

# These will be displayed by the "Click here for support information" link in "Add/Remove Programs"
# It is possible to use "mailto:" links in here to open the email client

!define HELPURL   "https://github.com/FedoraQt/MediaWriter" # "Support Information" link
!define UPDATEURL "https://getfedora.org"                   # "Product Updates" link
!define ABOUTURL  "https://getfedora.org"                   # "Publisher" link

# This is the size (in kB) of all the files copied into "Program Files"
#!define INSTALLSIZE

VIProductVersion "${FULLVERSION}"
VIFileVersion    "${FULLVERSION}"

VIAddVersionKey "ProductName"     "${APPNAME}"
VIAddVersionKey "ProductVersion"  "${FULLVERSION}"
VIAddVersionKey "Comments"        "${DESCRIPTION}"
VIAddVersionKey "CompanyName"     "${COMPANYNAME}"
VIAddVersionKey "LegalCopyright"  "${COPYRIGHT}"
VIAddVersionKey "LegalTrademarks" "${COPYRIGHT}"
VIAddVersionKey "FileDescription" "${APPNAME} installer"
VIAddVersionKey "FileVersion"     "${FULLVERSION}"

;Set the name of the uninstall log
!define UninstLog "uninstall.log"
Var UninstLog

!ifdef INNER
    !echo "Inner invocation"                  ; just to see what's going on
    OutFile "tempinstaller.exe"               ; not really important where this is
    SetCompress off                           ; for speed
!else
    !echo "Outer invocation"

    ; Call makensis again against current file, defining INNER.  This writes an installer for us which, when
    ; it is invoked, will just write the uninstaller to some location, and then exit.

    !makensis '-DINNER "${__FILE__}"' = 0

    ; Run the temporary installer and then sign the unsigned binary that has been created
    !system "chmod +x tempinstaller.exe" = 0
    !system "tempinstaller.exe" = 2
    !if "${CERTPASS}" != ""
        !system 'osslsigncode sign -pkcs12 "${CERTPATH}/authenticode.pfx" -readpass "${CERTPASS}" -h sha256 -n "Fedora Media Writer" -i https://getfedora.org -t http://timestamp.comodoca.com/authenticode -in "/c/uninstall.unsigned.exe" -out "/c/uninstall.exe" ' = 0
    !else
        !system 'mv "/c/uninstall.unsigned.exe" "/c/uninstall.exe"' = 0
    !endif

    outFile "FMW-setup.exe"
    SetCompressor /SOLID lzma
!endif


RequestExecutionLevel admin ;Require admin rights on NT6+ (When UAC is turned on)

InstallDir "$PROGRAMFILES64\${APPNAME}"

# rtf or txt file - remember if it is txt, it must be in the DOS text format (\r\n)
LicenseData "../../build/app/release/LICENSE.GPL-2.txt"
# This will be in the installer/uninstaller's title bar
Name "${APPNAME}"
Icon "../../src/app/data/icons/mediawriter.ico"

!include LogicLib.nsh

!define MUI_ICON ../../src/app/data/icons/mediawriter.ico

!insertmacro MUI_PAGE_LICENSE "../../build/app/release/LICENSE.GPL-2.txt"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_NOAUTOCLOSE
!define MUI_FINISHPAGE_RUN $INSTDIR\mediawriter.exe
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"       ;first language is the default language
!insertmacro MUI_LANGUAGE "Afrikaans"
!insertmacro MUI_LANGUAGE "Albanian"
!insertmacro MUI_LANGUAGE "Arabic"
!insertmacro MUI_LANGUAGE "Belarusian"
!insertmacro MUI_LANGUAGE "Bosnian"
!insertmacro MUI_LANGUAGE "Breton"
!insertmacro MUI_LANGUAGE "Bulgarian"
!insertmacro MUI_LANGUAGE "Catalan"
!insertmacro MUI_LANGUAGE "Croatian"
!insertmacro MUI_LANGUAGE "Czech"
!insertmacro MUI_LANGUAGE "Danish"
!insertmacro MUI_LANGUAGE "Dutch"
!insertmacro MUI_LANGUAGE "Esperanto"
!insertmacro MUI_LANGUAGE "Estonian"
!insertmacro MUI_LANGUAGE "Farsi"
!insertmacro MUI_LANGUAGE "Finnish"
!insertmacro MUI_LANGUAGE "French"
!insertmacro MUI_LANGUAGE "Galician"
!insertmacro MUI_LANGUAGE "German"
!insertmacro MUI_LANGUAGE "Greek"
!insertmacro MUI_LANGUAGE "Hebrew"
!insertmacro MUI_LANGUAGE "Hungarian"
!insertmacro MUI_LANGUAGE "Icelandic"
!insertmacro MUI_LANGUAGE "Indonesian"
!insertmacro MUI_LANGUAGE "Irish"
!insertmacro MUI_LANGUAGE "Italian"
!insertmacro MUI_LANGUAGE "Japanese"
!insertmacro MUI_LANGUAGE "Korean"
!insertmacro MUI_LANGUAGE "Kurdish"
!insertmacro MUI_LANGUAGE "Latvian"
!insertmacro MUI_LANGUAGE "Lithuanian"
!insertmacro MUI_LANGUAGE "Luxembourgish"
!insertmacro MUI_LANGUAGE "Macedonian"
!insertmacro MUI_LANGUAGE "Malay"
!insertmacro MUI_LANGUAGE "Mongolian"
!insertmacro MUI_LANGUAGE "Norwegian"
!insertmacro MUI_LANGUAGE "NorwegianNynorsk"
!insertmacro MUI_LANGUAGE "Polish"
!insertmacro MUI_LANGUAGE "Portuguese"
!insertmacro MUI_LANGUAGE "PortugueseBR"
!insertmacro MUI_LANGUAGE "Romanian"
!insertmacro MUI_LANGUAGE "Russian"
!insertmacro MUI_LANGUAGE "Serbian"
!insertmacro MUI_LANGUAGE "SerbianLatin"
!insertmacro MUI_LANGUAGE "SimpChinese"
!insertmacro MUI_LANGUAGE "Slovak"
!insertmacro MUI_LANGUAGE "Slovenian"
!insertmacro MUI_LANGUAGE "Spanish"
!insertmacro MUI_LANGUAGE "SpanishInternational"
!insertmacro MUI_LANGUAGE "Swedish"
!insertmacro MUI_LANGUAGE "Thai"
!insertmacro MUI_LANGUAGE "TradChinese"
!insertmacro MUI_LANGUAGE "Turkish"
!insertmacro MUI_LANGUAGE "Ukrainian"
!insertmacro MUI_LANGUAGE "Uzbek"

!include "languages.nsh"

!macro VerifyUserIsAdmin
UserInfo::GetAccountType
pop $0
${If} $0 != "admin" ;Require admin rights on NT4+
        messageBox mb_iconstop "$(AdmingRightsRequired)"
        setErrorLevel 740 ;ERROR_ELEVATION_REQUIRED
        quit
${EndIf}
!macroend

function .onInit
    !ifdef INNER

    ; If INNER is defined, then we aren't supposed to do anything except write out
    ; the installer.  This is better than processing a command line option as it means
    ; this entire code path is not present in the final (real) installer.

    WriteUninstaller "C:\uninstall.unsigned.exe"
    Quit  ; just bail out quickly when running the "inner" installer
    !endif

    setShellVarContext all
    !insertmacro VerifyUserIsAdmin
    
    ; Check if VC++ Redistributable 2015-2022 is already installed
    DetailPrint "Checking for Visual C++ 2015-2022 Redistributable..."
    
    ClearErrors
    ReadRegDWORD $0 HKLM "SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64" "Installed"
    StrCmp $0 "1" vcredist_already_installed
    
    ; Also check alternative registry location (WOW6432Node)
    ClearErrors
    ReadRegDWORD $0 HKLM "SOFTWARE\WOW6432Node\Microsoft\VisualStudio\14.0\VC\Runtimes\x64" "Installed"
    StrCmp $0 "1" vcredist_already_installed
    
    ; VC++ not installed - ask user if they want to download and install it
    MessageBox MB_YESNO|MB_ICONQUESTION "$(VCRedistNotInstalled)" IDNO skip_vcredist_init
    
    ; User chose to install - download the redistributable
    DetailPrint "$(VCRedistDownloading)"
    inetc::get /CAPTION "Downloading VC++ Redistributable" /POPUP "" "${VCREDIST_URL}" "$TEMP\vc_redist.exe" /END
    
    Pop $0
    StrCmp $0 "OK" vcredist_download_ok
        MessageBox MB_OK|MB_ICONEXCLAMATION "$(VCRedistDownloadFailed)"
        Goto vcredist_init_done
    
    vcredist_download_ok:
    ; Install VC++ Redistributable
    DetailPrint "$(VCRedistInstalling)"
    ExecWait '"$TEMP\vc_redist.exe" /install /norestart' $1

    Delete "$TEMP\vc_redist.exe"

    ${If} $1 == 0
        DetailPrint "VC++ Redistributable installation completed successfully"
    ${Else}
        DetailPrint "VC++ Redistributable installation returned code: $1"
    ${EndIf}
    Goto vcredist_init_done
    
    vcredist_already_installed:
    DetailPrint "Visual C++ Redistributable already installed. Skipping..."
    Goto vcredist_init_done
    
    skip_vcredist_init:
    MessageBox MB_OK|MB_ICONINFORMATION "$(VCRedistSkipped)"
    
    vcredist_init_done:
functionEnd

section "install"
        ; Uninstall previous version when installing to same directory
        ExecWait '"$INSTDIR\uninstall.exe" /S _?=$INSTDIR'

        # Files for the install directory - to build the installer, these should be in the same directory as the install script (this file)
        SetOutPath $INSTDIR
        SetOverwrite on

        !ifndef INNER
            SetOutPath $INSTDIR

            # Files added here should be removed by the uninstaller (see section "uninstall")
            File /r "..\..\build\app\release\*.*"
            File "..\..\src\app\data\icons\mediawriter.ico"

            ; this packages the signed uninstaller
            File c:\uninstall.exe
        !endif

        # Start Menu
        createShortCut "$SMPROGRAMS\${APPNAME}.lnk" "$INSTDIR\mediawriter.exe" "" "$INSTDIR\mediawriter.ico"

        # Registry information for add/remove programs
        WriteRegStr HKLM   "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayName"          "${APPNAME}"
        WriteRegStr HKLM   "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "UninstallString"      "$\"$INSTDIR\uninstall.exe$\""
        WriteRegStr HKLM   "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "QuietUninstallString" "$\"$INSTDIR\uninstall.exe$\" /S"
        WriteRegStr HKLM   "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "InstallLocation"      "$\"$INSTDIR$\""
        WriteRegStr HKLM   "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayIcon"          "$\"$INSTDIR\mediawriter.ico$\""
        WriteRegStr HKLM   "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "Publisher"            "${COMPANYNAME}"
        WriteRegStr HKLM   "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "HelpLink"             "${HELPURL}"
        WriteRegStr HKLM   "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "URLUpdateInfo"        "${UPDATEURL}"
        WriteRegStr HKLM   "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "URLInfoAbout"         "${ABOUTURL}"
        WriteRegStr HKLM   "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayVersion"       "${VERSIONMAJOR}.${VERSIONMINOR}.${VERSIONBUILD}"
        WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "VersionMajor"         ${VERSIONMAJOR}
        WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "VersionMinor"         ${VERSIONMINOR}
        # There is no option for modifying or repairing the install
        WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "NoModify"             1
        WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "NoRepair"             1
        # Set the INSTALLSIZE constant (!defined at the top of this script) so Add/Remove Programs can accurately report the size
        WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "EstimatedSize"        ${INSTALLSIZE}
sectionEnd

# Uninstaller
!ifdef INNER
    function un.onInit
        SetShellVarContext all

        #Verify the uninstaller - last chance to back out
        MessageBox MB_OKCANCEL "$(UninstallProgram)" IDOK next
                Abort
        next:
        !insertmacro VerifyUserIsAdmin
    functionEnd

    section "uninstall"
        # Remove Start Menu launcher
        delete "$SMPROGRAMS\${APPNAME}.lnk"

        ;Can't uninstall if uninstall log is missing!
        IfFileExists "$INSTDIR\${UninstLog}" +3
            MessageBox MB_OK|MB_ICONSTOP "$(UninstallProgram)"
            Abort

        Push $R0
        Push $R1
        Push $R2
        SetFileAttributes "$INSTDIR\${UninstLog}" NORMAL
        FileOpen $UninstLog "$INSTDIR\${UninstLog}" r
        StrCpy $R1 -1

        GetLineCount:
            ClearErrors
            FileRead $UninstLog $R0
            IntOp $R1 $R1 + 1
            StrCpy $R0 $R0 -1
            Push $R0
            IfErrors 0 GetLineCount

        Pop $R0

        LoopRead:
            StrCmp $R1 0 LoopDone
            Pop $R0

            IfFileExists "$INSTDIR\$R0\*.*" 0 +3
                RMDir /r "$INSTDIR\$R0"  #is dir
            Goto +3
            IfFileExists "$INSTDIR\$R0" 0 +2
                Delete "$INSTDIR\$R0" #is file
            IntOp $R1 $R1 - 1
            Goto LoopRead
        LoopDone:
        FileClose $UninstLog
        Delete "$INSTDIR\mediawriter.ico"
        Delete "$INSTDIR\${UninstLog}"
        Delete "$INSTDIR\uninstall.exe"
        Pop $R2
        Pop $R1
        Pop $R0

        # Try to remove the install directory - this will only happen if it is empty
        rmDir $INSTDIR

        # Remove uninstaller information from the registry
        DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"
    sectionEnd
!endif
