/**
 * Global variables, already defined.
 *
 * If you add a global variable, declare it here as commented.
 */
;------------------------------------------------------------------------------
; NOTE: To clarify, all these are defined on the command line using /D
; Here's a sample:
; C:/Program Files/NSIS/makensis.exe /DPRODUCT_NAME=VoxOx;/DPRODUCT_VERSION=2.0.5;/DBUILD_DIR=C:/Dev/Voxox.2.0.5/build/relwithdebinfo;/DQTDIR=C:\Qt\2009.03\qt;/DBINARY_NAME=VoxOx;/DPRODUCT_VERSION_1=2;/DPRODUCT_VERSION_2=0;/DPRODUCT_VERSION_3=5;/DPRODUCT_VERSION_4=0 /DINSTALLER_NAME=VoxOx-2.0.5.1659-setup-relwithdebinfo.exe C:/Dev/Voxox.2.0.5/wengophone/nsis/installer_2005.nsi
;------------------------------------------------------------------------------
;!define PRODUCT_NAME "WengoPhone"
;!define PRODUCT_VERSION "x.y.z"
;!define PRODUCT_VERSION_1 "x"
;!define PRODUCT_VERSION_2 "y"
;!define PRODUCT_VERSION_3 "z"
;!define PRODUCT_VERSION_4 "0"
;!define BUILD_DIR "..\..\release-symbols\"
;!define INSTALLER_NAME "WengoPhone-setup-0.13.exe"
;!define QTDIR "C:\Qt\4.1.1\"
;!define DEBUG
;!define WITH_PDB
;!define BINARY_NAME "qtwengophone"


!define PRODUCT_PUBLISHER 	"VoxOx"
!define PRODUCT_WEB_SITE 	"http://www.voxox.com"
!define PRODUCT_REGKEY 		"Software\VoxOx"
!define PRODUCT_UNINSTALL_KEY 	"Software\Microsoft\Windows\CurrentVersion\Uninstall\VoxOx"


;------------------------------------------------------------------------------
; Version information for the installer executable
;------------------------------------------------------------------------------
VIAddVersionKey ProductName 	"VoxOx"
VIAddVersionKey ProductVersion 	"${PRODUCT_VERSION}"
VIAddVersionKey Comments 	"VoxOx, Take Control"
VIAddVersionKey CompanyName 	"${PRODUCT_PUBLISHER}"
VIAddVersionKey LegalCopyright 	"Copyright (C) 2004-2009 VoxOx"
VIAddVersionKey LegalTrademarks "VoxOx 2.0"
VIAddVersionKey FileDescription "VoxOx"
VIAddVersionKey FileVersion 	"${PRODUCT_VERSION}"
VIAddVersionKey InternalName 	"VoxOx"
VIAddVersionKey OriginalFilename "${INSTALLER_NAME}"

VIProductVersion "${PRODUCT_VERSION_1}.${PRODUCT_VERSION_2}.${PRODUCT_VERSION_3}.${PRODUCT_VERSION_4}"


!macro LANG_STRING NAME VALUE
	LangString "${NAME}" "${LANG_${LANG}}" "${VALUE}"
!macroend

!macro LANG_UNSTRING NAME VALUE
	!insertmacro LANG_STRING "un.${NAME}" "${VALUE}"
!macroend

Var HaveInstallDirRegKey 	;JRT - 2009.11.04

;------------------------------------------------------------------------------
; Start attributes
;------------------------------------------------------------------------------

RequestExecutionLevel admin

SetCompressor 	lzma
XPStyle 	on

Name 		"VoxOx ${PRODUCT_VERSION}"
OutFile 	"${INSTALLER_NAME}"	;Installer name (e.g. WengoPhone-setup-0.13.exe).

; Gets installation folder from registry, if available
;InstallDirRegKey HKCU "${PRODUCT_REGKEY}" ""
InstallDirRegKey HKLM "${PRODUCT_REGKEY}" ""

ShowInstDetails   show
ShowUnInstDetails show

;------------------------------------------------------------------------------
; End attributes
;------------------------------------------------------------------------------


;------------------------------------------------------------------------------
; Modern UI 1.67 compatible
;------------------------------------------------------------------------------
!include "MUI.nsh"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON   "..\src\presentation\qt\win32\application.ico"
!define MUI_UNICON "..\src\presentation\qt\win32\application.ico"

; Language Selection Dialog Settings
; Remember the installer language
;!define MUI_LANGDLL_REGISTRY_ROOT 	"HKCU"
!define MUI_LANGDLL_REGISTRY_ROOT 	"HKLM"
!define MUI_LANGDLL_REGISTRY_KEY 	"${PRODUCT_REGKEY}"
!define MUI_LANGDLL_REGISTRY_VALUENAME "Installer Language"

;------------------------------------------------------------------------------
;Begin MUI page settings
;------------------------------------------------------------------------------

;------ WelCome Page ----------------------------------------------------------
!insertmacro MUI_PAGE_WELCOME

;------ License Page ----------------------------------------------------------
!insertmacro MUI_PAGE_LICENSE $(license)

;------ Directory Page --------------------------------------------------------
;!insertmacro MUI_PAGE_DIRECTORY
PageEx directory
	PageCallbacks dirCreate "" ""
PageExEnd

;------ InstFiles Page --------------------------------------------------------
!insertmacro MUI_PAGE_INSTFILES

;------ Finish Page -----------------------------------------------------------
!define MUI_FINISHPAGE_RUN "$INSTDIR\VoxOx.exe"
!insertmacro MUI_PAGE_FINISH

;------ Uninstaller Pages -----------------------------------------------------
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

; Language files, first language is the default language
!insertmacro MUI_LANGUAGE "English"

; Loads license translation
LicenseLangString license ${LANG_ENGLISH} "..\COPYING"
LicenseData $(license)

; Reserve files
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

;------------------------------------------------------------------------------
;End Installer Page Settings
;------------------------------------------------------------------------------




!define WND_CLASS "QWidget"
!define WND_TITLE "VoxOx"
!define TO_MS 2000
!define SYNC_TERM 0x00100001

!macro TerminateApp
	Push $0
	Push $1
	Push $2
	DetailPrint "Stopping ${WND_TITLE} Application"
	FindWindow $0 '${WND_CLASS}' '${WND_TITLE}'
	IntCmp $0 0 done
	System::Call 'user32.dll::GetWindowThreadProcessId(i r0, *i .r1) i .r2'
	System::Call 'kernel32.dll::OpenProcess(i ${SYNC_TERM}, i 0, i r1) i .r2'
	SendMessage $0 ${WM_CLOSE} 0 0 /TIMEOUT=${TO_MS}
	System::Call 'kernel32.dll::WaitForSingleObject(i r2, i ${TO_MS}) i .r1'
	IntCmp $1 0 close
	System::Call 'kernel32.dll::TerminateProcess(i r2, i 0) i .r1'
	close:
		System::Call 'kernel32.dll::CloseHandle(i r2) i .r1'
	done:
		Pop $2
		Pop $1
		Pop $0
!macroend

!include "functions.nsi"

;------------------------------------------------------------------------------
; Some initialization code
;------------------------------------------------------------------------------
Function .onInit
	!insertmacro TerminateApp
	System::Call 'kernel32::CreateMutexA(i 0, i 0, t "VoxOxInstaller") ?e'
	Pop $R0
	StrCmp $R0 0 +8
		IfSilent 0 exitMessage
		Abort

	exitMessage:
	 MessageBox MB_OK "The VoxOx installer is already running."
	 Abort

	/** For future use */
	/** TODO: just if an instance is detected */
	/** tells qtwengophone.exe to exit */
	;push 685
	;push "WengoPhone"
	;push "QWidget"
	;call SendMessage

	!insertmacro MUI_LANGDLL_DISPLAY

	Call DetermineInstallDir
FunctionEnd

;------------------------------------------------------------------------------
; If in silent mode, launch Voxox upon successul install.  This is mostly for upgrades.
;------------------------------------------------------------------------------
Function .onInstSuccess
	IfSilent 0 done
	Exec '"$INSTDIR\VoxOx.exe"'

  done:
FunctionEnd


;------------------------------------------------------------------------------
; Determine proper Install Dir based on Registry keys, admin level of user
;------------------------------------------------------------------------------
Function DetermineInstallDir
	StrCpy $HaveInstallDirRegKey "no"	;Set initial value

;	MessageBox MB_OK "DetermineInstallDir: InstallDir RegKey = $INSTDIR ${PRODUCT_REGKEY}"

	;Because we changed theRegistry key nest from HKCU to HKLM, we need to check BOTH for valid entry.
	ReadRegStr $0 HKLM "${PRODUCT_REGKEY}" ""
	StrCmp $0 "" checkHKCU haveRegKey

  checkHKCU:
	ReadRegStr $0 HKCU "${PRODUCT_REGKEY}" ""
;	MessageBox MB_OK "installDir RegKey = $0"

	StrCmp $0 "" continueCheck haveRegKey

  haveRegKey:
	; If already installed, we MUST use the same InstallDir.
	StrCpy $INSTDIR $0
	StrCpy $HaveInstallDirRegKey "yes"
	goto done

  continueCheck:
	;If silent mode, will just use default or InstallDirRegKey for $INSTDIR
;	Call IsSilent
;	Pop $0
;	StrCmp $0 1 0 +2
;		goto done
	IfSilent done checkIt
	
  checkIt:
	Call isUserAdmin
	Pop $R0
	StrCmp $R0 "true" isAdmin
		; Not an admin
		StrCpy $INSTDIR "$DOCUMENTS\VoxOx"
		goto done
	isAdmin:
		; User is admin
		StrCpy $INSTDIR "$PROGRAMFILES\VoxOx"

  done:
;	MessageBox MB_OK "DetermineInstallDir: InstDir = $INSTDIR"

FunctionEnd


;------------------------------------------------------------------------------
; Skip display of Directory page if we already have a RegKey for InstallDir
;------------------------------------------------------------------------------
Function dirCreate
	StrCmp $HaveInstallDirRegKey "yes" skipIt done
  skipIt:
	MessageBox MB_OK "Previous version of Voxox is already installed in $INSTDIR.  This installation will be done in that directory."
	Abort
  done:
FunctionEnd


Section BaseSection
	!include "files_install.nsi"

	;JRT - 2009.11.02 - If this Reg Key exist, then vc redist is already installed and we don't need to do it again.	
	;JRT - 2010.02.22 - Because VC-Redist files use same key, we have issues with manifests if users do not maintain Windows Updates
	;		    Therefore, we will ALWAYS install this.
;	ReadRegStr $0 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{7299052b-02a4-4627-81f2-1818da5d550d}" "InstallSource"
;	MessageBox MB_OK "vcredistKey = $0"

;	StrCmp $0 "" 0 vcredistInstalled
	ExecWait '$INSTDIR\vcredist_x86.exe /q:a /c:"VCREDI~3.EXE /q:a /c:""msiexec /i vcredist.msi /qn"" "'

  vcredistInstalled:

SectionEnd

Section -AdditionalIcons
	WriteIniStr "$INSTDIR\${PRODUCT_PUBLISHER}.url"  "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"

	CreateShortCut "$SMPROGRAMS\VoxOx\Website.lnk" 	 "$INSTDIR\${PRODUCT_PUBLISHER}.url" "" "$INSTDIR\${PRODUCT_PUBLISHER}.url" 0
	CreateShortCut "$SMPROGRAMS\VoxOx\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0 

SectionEnd

; After installation, store some values in Registry.
Section -Post
	; Stores installation folder
;	WriteRegStr HKCU "${PRODUCT_REGKEY}" "" $INSTDIR
	WriteRegStr HKLM "${PRODUCT_REGKEY}" "" $INSTDIR	;JRT - 2009.11.03

	; Creates uninstaller
	WriteUninstaller "$INSTDIR\uninstall.exe"

	WriteRegStr HKLM "${PRODUCT_UNINSTALL_KEY}" "DisplayName" 	"$(^Name)"
	WriteRegStr HKLM "${PRODUCT_UNINSTALL_KEY}" "UninstallString" 	"$INSTDIR\uninstall.exe"
	WriteRegStr HKLM "${PRODUCT_UNINSTALL_KEY}" "DisplayIcon" 	"$INSTDIR\VoxOx.exe"
	WriteRegStr HKLM "${PRODUCT_UNINSTALL_KEY}" "DisplayVersion" 	"${PRODUCT_VERSION}"
	WriteRegStr HKLM "${PRODUCT_UNINSTALL_KEY}" "URLInfoAbout" 	"${PRODUCT_WEB_SITE}"
	WriteRegStr HKLM "${PRODUCT_UNINSTALL_KEY}" "Publisher" 	"${PRODUCT_PUBLISHER}"
SectionEnd

;--------------------------------------------------------
; End Installer related functions/sections
;--------------------------------------------------------


;--------------------------------------------------------
; Begin Uninstaller related functions
;--------------------------------------------------------
Function un.onInit
	!insertmacro MUI_UNGETLANGUAGE
FunctionEnd

Section Uninstall
	DeleteRegKey HKLM "${PRODUCT_UNINSTALL_KEY}"
	DeleteRegKey HKCU "${PRODUCT_REGKEY}"
	DeleteRegKey HKLM "${PRODUCT_REGKEY}"	;JRT - 2009.11.03 - delete both due to change.
	
	Delete "$SMPROGRAMS\VoxOx\Website.lnk"
	Delete "$SMPROGRAMS\VoxOx\Uninstall.lnk"
	Delete "$DESKTOP\VoxOx.lnk"
	RMDir  "$SMPROGRAMS\VoxOx"
	Delete "$SMPROGRAMS\VoxOx\*.lnk"
	RMDir  "$SMPROGRAMS\VoxOx\*.lnk"

	!include "files_uninstall.nsi"
SectionEnd

;--------------------------------------------------------
; End Uninstaller related functions
;--------------------------------------------------------
