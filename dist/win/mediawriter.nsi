!include "MUI2.nsh"
ManifestDPIAware true
XPStyle on

# If you change the names "app.exe", "logo.ico", or "license.rtf" you should do a search and replace
# they show up in a few places.
# All the other settings can be tweaked by editing the !defines at the top of this script

# These three must be defined from command line
#!define VERSIONMAJOR
#!define VERSIONMINOR
#!define VERSIONBUILD

!define APPNAME           "Bazzite Media Writer"
!define /date CURRENTYEAR "%Y"
!define COMPANYNAME       "Universal Blue"
!define COPYRIGHT         "${COMPANYNAME} ${CURRENTYEAR}"
!define DESCRIPTION       "Tool to write Bazzite images to flash drives"
!define FULLVERSION       "${VERSIONMAJOR}.${VERSIONMINOR}.${VERSIONBUILD}.0"
!define SHORTVERSION      "${VERSIONMAJOR}.${VERSIONMINOR}.${VERSIONBUILD}"

Name    "${APPNAME}"
Caption "${APPNAME} ${SHORTVERSION}"

# These will be displayed by the "Click here for support information" link in "Add/Remove Programs"
# It is possible to use "mailto:" links in here to open the email client

!define HELPURL   "https://github.com/ublue-os/bazzite" # "Support Information" link
!define UPDATEURL "https://universal-blue.org"           # "Product Updates" link
!define ABOUTURL  "https://universal-blue.org"           # "Publisher" link

# This is the size (in kB) of all the files copied into "Program Files"
#!define INSTALLSIZE

VIProductVersion "${FULLVERSION}"
VIFileVersion    "${FULLVERSION}"

VIAddVersionKey  "ProductName"     "${APPNAME}"
VIAddVersionKey  "ProductVersion"  "${FULLVERSION}"
VIAddVersionKey  "Comments"        "${DESCRIPTION}"
VIAddVersionKey  "CompanyName"     "${COMPANYNAME}"
VIAddVersionKey  "LegalCopyright"  "${COPYRIGHT}"
VIAddVersionKey  "LegalTrademarks" "${COPYRIGHT}"
VIAddVersionKey  "FileDescription" "${APPNAME} installer"
VIAddVersionKey  "FileVersion"     "${FULLVERSION}"

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
    !system "./tempinstaller.exe" = 512
    !if "${CERTPASS}" != ""
        !system 'osslsigncode sign -pkcs12 "${CERTPATH}/authenticode.pfx" -readpass "${CERTPASS}" -h sha256 -n "Fedora Media Writer" -i https://getfedora.org -t http://timestamp.comodoca.com/authenticode -in "../../build/wineprefix/drive_c/uninstall.unsigned.exe" -out "../../build/wineprefix/drive_c/uninstall.exe" ' = 0
    !else
        !system 'mv "../../build/wineprefix/drive_c/uninstall.unsigned.exe" "../../build/wineprefix/drive_c/uninstall.exe"' = 0
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
!define      MUI_FINISHPAGE_NOAUTOCLOSE
!define      MUI_FINISHPAGE_RUN $INSTDIR\mediawriter.exe
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

LangString UninstallProgram ${LANG_ENGLISH}                  "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_AFRIKAANS}                "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_ALBANIAN}                 "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_ARABIC}                   "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_BELARUSIAN}               "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_BOSNIAN}                  "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_BRETON}                   "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_BULGARIAN}                "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_CATALAN}                  "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_CROATIAN}                 "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_CZECH}                    "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_DANISH}                   "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_DUTCH}                    "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_ESPERANTO}                "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_ESTONIAN}                 "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_FARSI}                    "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_FINNISH}                  "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_FRENCH}                   "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_GALICIAN}                 "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_GERMAN}                   "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_GREEK}                    "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_HEBREW}                   "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_HUNGARIAN}                "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_ICELANDIC}                "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_INDONESIAN}               "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_IRISH}                    "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_ITALIAN}                  "Vuoi disinstallaree ${APPNAME}?"
LangString UninstallProgram ${LANG_JAPANESE}                 "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_KOREAN}                   "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_KURDISH}                  "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_LATVIAN}                  "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_LITHUANIAN}               "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_LUXEMBOURGISH}            "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_MACEDONIAN}               "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_MALAY}                    "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_MONGOLIAN}                "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_NORWEGIAN}                "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_NORWEGIANNYNORSK}         "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_POLISh}                   "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_PORTUGUESE}               "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_PORTUGUESEBR}             "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_ROMANIAN}                 "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_RUSSIAN}                  "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_SERBIAN}                  "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_SERBIANLATIN}             "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_SIMPCHINESE}              "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_SLOVAK}                   "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_SLOVENIAN}                "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_SPANISH}                  "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_SPANISHINTERNATIONAL}     "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_SWEDISH}                  "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_THAI}                     "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_TRADCHINESE}              "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_TURKISH}                  "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_UKRAINIAN}                "Do you want to uninstall ${APPNAME}?"
LangString UninstallProgram ${LANG_UZBEK}                    "Do you want to uninstall ${APPNAME}?"

LangString AdmingRightsRequired ${LANG_ENGLISH}              "Administrator rights required!"
LangString AdmingRightsRequired ${LANG_AFRIKAANS}            "Admin rights required!"
LangString AdmingRightsRequired ${LANG_ALBANIAN}             "Admin rights required!"
LangString AdmingRightsRequired ${LANG_ARABIC}               "Admin rights required!"
LangString AdmingRightsRequired ${LANG_BELARUSIAN}           "Admin rights required!"
LangString AdmingRightsRequired ${LANG_BOSNIAN}              "Admin rights required!"
LangString AdmingRightsRequired ${LANG_BRETON}               "Admin rights required!"
LangString AdmingRightsRequired ${LANG_BULGARIAN}            "Admin rights required!"
LangString AdmingRightsRequired ${LANG_CATALAN}              "Admin rights required!"
LangString AdmingRightsRequired ${LANG_CROATIAN}             "Admin rights required!"
LangString AdmingRightsRequired ${LANG_CZECH}                "Admin rights required!"
LangString AdmingRightsRequired ${LANG_DANISH}               "Admin rights required!"
LangString AdmingRightsRequired ${LANG_DUTCH}                "Admin rights required!"
LangString AdmingRightsRequired ${LANG_ESPERANTO}            "Admin rights required!"
LangString AdmingRightsRequired ${LANG_ESTONIAN}             "Admin rights required!"
LangString AdmingRightsRequired ${LANG_FARSI}                "Admin rights required!"
LangString AdmingRightsRequired ${LANG_FINNISH}              "Admin rights required!"
LangString AdmingRightsRequired ${LANG_FRENCH}               "Admin rights required!"
LangString AdmingRightsRequired ${LANG_GALICIAN}             "Admin rights required!"
LangString AdmingRightsRequired ${LANG_GERMAN}               "Admin rights required!"
LangString AdmingRightsRequired ${LANG_GREEK}                "Admin rights required!"
LangString AdmingRightsRequired ${LANG_HEBREW}               "Admin rights required!"
LangString AdmingRightsRequired ${LANG_HUNGARIAN}            "Admin rights required!"
LangString AdmingRightsRequired ${LANG_ICELANDIC}            "Admin rights required!"
LangString AdmingRightsRequired ${LANG_INDONESIAN}           "Admin rights required!"
LangString AdmingRightsRequired ${LANG_IRISH}                "Admin rights required!"
LangString AdmingRightsRequired ${LANG_ITALIAN}              "Sono necessari i diritti di amministratore!"
LangString AdmingRightsRequired ${LANG_JAPANESE}             "Admin rights required!"
LangString AdmingRightsRequired ${LANG_KOREAN}               "Admin rights required!"
LangString AdmingRightsRequired ${LANG_KURDISH}              "Admin rights required!"
LangString AdmingRightsRequired ${LANG_LATVIAN}              "Admin rights required!"
LangString AdmingRightsRequired ${LANG_LITHUANIAN}           "Admin rights required!"
LangString AdmingRightsRequired ${LANG_LUXEMBOURGISH}        "Admin rights required!"
LangString AdmingRightsRequired ${LANG_MACEDONIAN}           "Admin rights required!"
LangString AdmingRightsRequired ${LANG_MALAY}                "Admin rights required!"
LangString AdmingRightsRequired ${LANG_MONGOLIAN}            "Admin rights required!"
LangString AdmingRightsRequired ${LANG_NORWEGIAN}            "Admin rights required!"
LangString AdmingRightsRequired ${LANG_NORWEGIANNYNORSK}     "Admin rights required!"
LangString AdmingRightsRequired ${LANG_POLISh}               "Admin rights required!"
LangString AdmingRightsRequired ${LANG_PORTUGUESE}           "Admin rights required!"
LangString AdmingRightsRequired ${LANG_PORTUGUESEBR}         "Admin rights required!"
LangString AdmingRightsRequired ${LANG_ROMANIAN}             "Admin rights required!"
LangString AdmingRightsRequired ${LANG_RUSSIAN}              "Admin rights required!"
LangString AdmingRightsRequired ${LANG_SERBIAN}              "Admin rights required!"
LangString AdmingRightsRequired ${LANG_SERBIANLATIN}         "Admin rights required!"
LangString AdmingRightsRequired ${LANG_SIMPCHINESE}          "Admin rights required!"
LangString AdmingRightsRequired ${LANG_SLOVAK}               "Admin rights required!"
LangString AdmingRightsRequired ${LANG_SLOVENIAN}            "Admin rights required!"
LangString AdmingRightsRequired ${LANG_SPANISH}              "Admin rights required!"
LangString AdmingRightsRequired ${LANG_SPANISHINTERNATIONAL} "Admin rights required!"
LangString AdmingRightsRequired ${LANG_SWEDISH}              "Admin rights required!"
LangString AdmingRightsRequired ${LANG_THAI}                 "Admin rights required!"
LangString AdmingRightsRequired ${LANG_TRADCHINESE}          "Admin rights required!"
LangString AdmingRightsRequired ${LANG_TURKISH}              "Admin rights required!"
LangString AdmingRightsRequired ${LANG_UKRAINIAN}            "Admin rights required!"
LangString AdmingRightsRequired ${LANG_UZBEK}                "Admin rights required!"

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
            File /r "../../build/app/release/*"
            File "../../src/app/data/icons/mediawriter.ico"

            ; this packages the signed uninstaller
            File ../../build/wineprefix/drive_c/uninstall.exe
        !endif

        # Start Menu
        createShortCut "$SMPROGRAMS\${APPNAME}.lnk" "$INSTDIR\mediawriter.exe" "" "$INSTDIR\mediawriter.ico"

        # Registry information for add/remove programs
        WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"   "DisplayName"          "${APPNAME}"
        WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"   "UninstallString"      "$\"$INSTDIR\uninstall.exe$\""
        WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"   "QuietUninstallString" "$\"$INSTDIR\uninstall.exe$\" /S"
        WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"   "InstallLocation"      "$\"$INSTDIR$\""
        WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"   "DisplayIcon"          "$\"$INSTDIR\mediawriter.ico$\""
        WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"   "Publisher"            "${COMPANYNAME}"
        WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"   "HelpLink"             "${HELPURL}"
        WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"   "URLUpdateInfo"        "${UPDATEURL}"
        WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"   "URLInfoAbout"         "${ABOUTURL}"
        WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"   "DisplayVersion"       "${VERSIONMAJOR}.${VERSIONMINOR}.${VERSIONBUILD}"
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
        SetFileAttributes   "$INSTDIR\${UninstLog}" NORMAL
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
