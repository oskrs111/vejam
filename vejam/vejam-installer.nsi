;NSIS Modern User Interface
;Header Bitmap Example Script
;Written by Joost Verburg

;--------------------------------
;Include Modern UI

  !include "MUI2.nsh"

;--------------------------------
;General

  ;Name and file
  Name "VEJAM Installer"
  OutFile "Setup.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\VEJAM"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\VEJAM" ""

  ;Request application privileges for Windows Vista
  RequestExecutionLevel admin
  
  !include LogicLib.nsh
 
# Just three pages - license agreement, install location, and installation
;page license
;page directory
;Page instfiles
 
!macro VerifyUserIsAdmin
UserInfo::GetAccountType
pop $0
${If} $0 != "admin" ;Require admin rights on NT4+
        messageBox mb_iconstop "Administrator rights required!"
        setErrorLevel 740 ;ERROR_ELEVATION_REQUIRED
        quit
${EndIf}
!macroend
  

;--------------------------------
;Interface Configuration
  !define MUI_ICON "img\vejam_p.ico"
  !define MUI_HEADERIMAGE
  !define MUI_HEADERIMAGE_BITMAP "img\vejam.bmp"
  !define MUI_WELCOMEFINISHPAGE_BITMAP "img\vejam_v.bmp"
  !define MUI_ABORTWARNING
  !define MUI_FINISHPAGE_RUN_TEXT "Iniciar Vejam.exe"
  !define MUI_FINISHPAGE_RUN "$INSTDIR\Vejam.exe"

;--------------------------------
;Pages
  
  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "doc\license.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "Componentes" SecMain

  SetOutPath "$INSTDIR"
  
  ;ADD YOUR OWN FILES HERE...
  File /r bin\*.*
  
  CreateShortCut "$SMSTARTUP\Vejam.lnk" "$INSTDIR\vejam.exe" "" "$INSTDIR\vejam_p.ico"
  
  ;Store installation folder
  WriteRegStr HKCU "Software\VEJAM" "" $INSTDIR
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VEJAM" "DisplayName" "VEJAM"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VEJAM" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VEJAM" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VEJAM" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  
  ;Make the directory "$INSTDIR" read write accessible by all users
  AccessControl::GrantOnFile \
  "$INSTDIR" "(BU)" "GenericRead + GenericWrite"

SectionEnd

; Optional section (can be disabled by the user)
Section "Menu inicio" SecShortcuts

  CreateDirectory "$SMPROGRAMS\VEJAM"
  CreateShortCut "$SMPROGRAMS\VEJAM\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0  
  CreateShortCut "$SMPROGRAMS\VEJAM\Vejam.lnk" "$INSTDIR\vejam.exe" "" "$INSTDIR\vejam.exe" 0    
SectionEnd

;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_SecMain ${LANG_ENGLISH} "Instala los componentes necesarios para ejecutar VEJAM."
  LangString DESC_SecShortcuts ${LANG_ENGLISH} "Añade los accesos directos al menu inicio."
  
  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecMain} $(DESC_SecMain)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecShortcuts} $(DESC_SecShortcuts)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END
 
;--------------------------------
;Uninstaller Section

Section "Uninstall"

  ;ADD YOUR OWN FILES HERE...   
  ;Delete "$INSTDIR\accessible\*.*"
  ;Delete "$INSTDIR\bearer\*.*"
  ;Delete "$INSTDIR\imageformats\*.*"
  ;Delete "$INSTDIR\mediaservice\*.*"
  ;Delete "$INSTDIR\platforms\*.*"    
  ;RMDir "$INSTDIR\accessible"
  ;RMDir "$INSTDIR\bearer"
  ;RMDir "$INSTDIR\imageformats"
  ;RMDir "$INSTDIR\mediaservice"
  ;RMDir "$INSTDIR\platforms"  
  ;Delete "$INSTDIR\*.*"
  
  RMDir /r /REBOOTOK $INSTDIR
  
  Delete "$SMSTARTUP\Vejam.lnk"
 
  DeleteRegKey /ifempty HKCU "Software\VEJAM"

SectionEnd
