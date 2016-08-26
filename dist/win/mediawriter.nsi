Name "Fedora Media Writer"
OutFile "FMW-setup.exe"

!include "MUI2.nsh"
XPStyle on

!include "MUI2.nsh"
XPStyle on

SetCompressor lzma

InstallDir "$PROGRAMFILES\Fedora Media Writer"
InstallDirRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Fedora Media Writer" ""

DirText "Select the directory to install Fedora Media Writer in:"

!define MUI_ICON mediawriter.ico

!insertmacro MUI_PAGE_WELCOME
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

Section ""

; Install files.
SetOverwrite on

SetOutPath $INSTDIR


File /r ..\..\build\app\release\*

; Create shortcut.
SetOutPath -
CreateDirectory "$SMPROGRAMS\Fedora Media Writer"
CreateShortCut "$SMPROGRAMS\Fedora Media Writer\Fedora Media Writer.lnk" "$INSTDIR\mediawriter.exe"
CreateShortCut "$SMPROGRAMS\Fedora Media Writer\Uninstall Fedora Media Writer.lnk" "$INSTDIR\uninst.exe" "" "$INSTDIR\uninst.exe" 0

; Create uninstaller.
WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Fedora Media Writer" "" "$INSTDIR"
WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\Fedora Media Writer" "DisplayName" "Fedora Media Writer (remove only)"
WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\Fedora Media Writer" "UninstallString" '"$INSTDIR\uninst.exe"'
WriteUninstaller "$INSTDIR\uninst.exe"

SectionEnd

UninstallText "This will uninstall Fedora Media Writer from your system."

Section Uninstall

; Delete shortcuts.
Delete "$SMPROGRAMS\Fedora Media Writer\Fedora Media Writer.lnk"
Delete "$SMPROGRAMS\Fedora Media Writer\Uninstall Fedora Media Writer.lnk"
RMDir "$SMPROGRAMS\Fedora Media Writer"
Delete "$DESKTOP\Fedora Media Writer.lnk"

; Delete registry keys.
Delete "$INSTDIR\uninst.exe"
DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Fedora Media Writer"
DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Fedora Media Writer"

; Delete everything in the installation directory.
RMDir /R "$INSTDIR"

SectionEnd

!macro SetUILanguage UN
Function ${UN}SetUILanguage
  Push $R0
  ; Call GetUserDefaultUILanguage (available on Windows Me, 2000 and later)
  ; $R0 = GetUserDefaultUILanguage()
  System::Call 'kernel32::GetUserDefaultUILanguage() i.r10'
  StrCpy $LANGUAGE $R0
  Pop $R0
FunctionEnd
!macroend
!insertmacro SetUILanguage ""
!insertmacro SetUILanguage "un."

Function .onInit
  Call SetUILanguage
FunctionEnd
 
Function un.onInit
  Call un.SetUILanguage
FunctionEnd
