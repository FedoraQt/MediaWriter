!include "MUI2.nsh"
XPStyle on

# If you change the names "app.exe", "logo.ico", or "license.rtf" you should do a search and replace - they
# show up in a few places.
# All the other settings can be tweaked by editing the !defines at the top of this script
!define APPNAME "Fedora Media Writer"
!define COMPANYNAME "Fedora Project"
!define DESCRIPTION "Tool to write Fedora images to flash drives"
# These three must be defined from command line
#!define VERSIONMAJOR
#!define VERSIONMINOR
#!define VERSIONBUILD
# These will be displayed by the "Click here for support information" link in "Add/Remove Programs"
# It is possible to use "mailto:" links in here to open the email client
!define HELPURL "https://github.com/FedoraQt/MediaWriter" # "Support Information" link
!define UPDATEURL "https://getfedora.org" # "Product Updates" link
!define ABOUTURL "https://getfedora.org" # "Publisher" link
# This is the size (in kB) of all the files copied into "Program Files"
#!define INSTALLSIZE



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
    !system "./tempinstaller.exe" = 2
    !if "${CERTPASS}" != ""
        !system 'osslsigncode sign -pkcs12 "${CERTPATH}/authenticode.pfx" -readpass "${CERTPASS}" -h sha256 -n "Fedora Media Writer" -i https://getfedora.org -t http://timestamp.comodoca.com/authenticode -in "/c/uninstall.unsigned.exe" -out "/c/uninstall.exe" ' = 0
    !else
        !system 'mv "/c/uninstall.unsigned.exe" "/c/uninstall.exe"' = 0
    !endif

    outFile "FMW-setup.exe"
    SetCompressor /SOLID lzma
!endif


RequestExecutionLevel admin ;Require admin rights on NT6+ (When UAC is turned on)

InstallDir "$PROGRAMFILES\${APPNAME}"

# rtf or txt file - remember if it is txt, it must be in the DOS text format (\r\n)
LicenseData "../../build/app/release/LICENSE.GPL-2.txt"
# This will be in the installer/uninstaller's title bar
Name "${APPNAME}"
Icon "../../app/data/icons/mediawriter.ico"

!include LogicLib.nsh

!define MUI_ICON ../../app/data/icons/mediawriter.ico

!insertmacro MUI_PAGE_LICENSE "../../build/app/release/LICENSE.GPL-2.txt"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_NOAUTOCLOSE
!define MUI_FINISHPAGE_RUN $INSTDIR\mediawriter.exe
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English" ;first language is the default language
!insertmacro MUI_LANGUAGE "French"
!insertmacro MUI_LANGUAGE "German"
!insertmacro MUI_LANGUAGE "Spanish"
!insertmacro MUI_LANGUAGE "SpanishInternational"
!insertmacro MUI_LANGUAGE "SimpChinese"
!insertmacro MUI_LANGUAGE "TradChinese"
!insertmacro MUI_LANGUAGE "Japanese"
!insertmacro MUI_LANGUAGE "Korean"
!insertmacro MUI_LANGUAGE "Italian"
!insertmacro MUI_LANGUAGE "Dutch"
!insertmacro MUI_LANGUAGE "Danish"
!insertmacro MUI_LANGUAGE "Swedish"
!insertmacro MUI_LANGUAGE "Norwegian"
!insertmacro MUI_LANGUAGE "NorwegianNynorsk"
!insertmacro MUI_LANGUAGE "Finnish"
!insertmacro MUI_LANGUAGE "Greek"
!insertmacro MUI_LANGUAGE "Russian"
!insertmacro MUI_LANGUAGE "Portuguese"
!insertmacro MUI_LANGUAGE "PortugueseBR"
!insertmacro MUI_LANGUAGE "Polish"
!insertmacro MUI_LANGUAGE "Ukrainian"
!insertmacro MUI_LANGUAGE "Czech"
!insertmacro MUI_LANGUAGE "Slovak"
!insertmacro MUI_LANGUAGE "Croatian"
!insertmacro MUI_LANGUAGE "Bulgarian"
!insertmacro MUI_LANGUAGE "Hungarian"
!insertmacro MUI_LANGUAGE "Thai"
!insertmacro MUI_LANGUAGE "Romanian"
!insertmacro MUI_LANGUAGE "Latvian"
!insertmacro MUI_LANGUAGE "Macedonian"
!insertmacro MUI_LANGUAGE "Estonian"
!insertmacro MUI_LANGUAGE "Turkish"
!insertmacro MUI_LANGUAGE "Lithuanian"
!insertmacro MUI_LANGUAGE "Slovenian"
!insertmacro MUI_LANGUAGE "Serbian"
!insertmacro MUI_LANGUAGE "SerbianLatin"
!insertmacro MUI_LANGUAGE "Arabic"
!insertmacro MUI_LANGUAGE "Farsi"
!insertmacro MUI_LANGUAGE "Hebrew"
!insertmacro MUI_LANGUAGE "Indonesian"
!insertmacro MUI_LANGUAGE "Mongolian"
!insertmacro MUI_LANGUAGE "Luxembourgish"
!insertmacro MUI_LANGUAGE "Albanian"
!insertmacro MUI_LANGUAGE "Breton"
!insertmacro MUI_LANGUAGE "Belarusian"
!insertmacro MUI_LANGUAGE "Icelandic"
!insertmacro MUI_LANGUAGE "Malay"
!insertmacro MUI_LANGUAGE "Bosnian"
!insertmacro MUI_LANGUAGE "Kurdish"
!insertmacro MUI_LANGUAGE "Irish"
!insertmacro MUI_LANGUAGE "Uzbek"
!insertmacro MUI_LANGUAGE "Galician"
!insertmacro MUI_LANGUAGE "Afrikaans"
!insertmacro MUI_LANGUAGE "Catalan"
!insertmacro MUI_LANGUAGE "Esperanto"

!macro VerifyUserIsAdmin
UserInfo::GetAccountType
pop $0
${If} $0 != "admin" ;Require admin rights on NT4+
        messageBox mb_iconstop "Administrator rights required!"
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
        # Files for the install directory - to build the installer, these should be in the same directory as the install script (this file)
        SetOutPath $INSTDIR
        SetOverwrite on

        !ifndef INNER
            SetOutPath $INSTDIR

            # Files added here should be removed by the uninstaller (see section "uninstall")
            File /r "..\..\build\app\release\*.*"
            File "..\..\app\data\icons\mediawriter.ico"

            ; this packages the signed uninstaller
            File c:\uninstall.exe
        !endif

        # Start Menu
        createShortCut "$SMPROGRAMS\${APPNAME}.lnk" "$INSTDIR\mediawriter.exe" "" "$INSTDIR\mediawriter.ico"

        # Registry information for add/remove programs
        WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayName" "${APPNAME}"
        WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "UninstallString" "$\"$INSTDIR\uninstall.exe$\""
        WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "QuietUninstallString" "$\"$INSTDIR\uninstall.exe$\" /S"
        WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "InstallLocation" "$\"$INSTDIR$\""
        WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayIcon" "$\"$INSTDIR\mediawriter.ico$\""
        WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "Publisher" "${COMPANYNAME}"
        WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "HelpLink" "${HELPURL}"
        WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "URLUpdateInfo" "${UPDATEURL}"
        WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "URLInfoAbout" "${ABOUTURL}"
        WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayVersion" "${VERSIONMAJOR}.${VERSIONMINOR}.${VERSIONBUILD}"
        WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "VersionMajor" ${VERSIONMAJOR}
        WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "VersionMinor" ${VERSIONMINOR}
        # There is no option for modifying or repairing the install
        WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "NoModify" 1
        WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "NoRepair" 1
        # Set the INSTALLSIZE constant (!defined at the top of this script) so Add/Remove Programs can accurately report the size
        WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "EstimatedSize" ${INSTALLSIZE}
sectionEnd

# Uninstaller
!ifdef INNER
    function un.onInit
        SetShellVarContext all

        #Verify the uninstaller - last chance to back out
        MessageBox MB_OKCANCEL "Permanently remove ${APPNAME}?" IDOK next
                Abort
        next:
        !insertmacro VerifyUserIsAdmin
    functionEnd

    section "uninstall"
        # Remove Start Menu launcher
        delete "$SMPROGRAMS\${APPNAME}.lnk"

        # Remove files
        delete $INSTDIR\*

        # Try to remove the install directory - this will only happen if it is empty
        rmDir /r $INSTDIR

        # Remove uninstaller information from the registry
        DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"
    sectionEnd
!endif
