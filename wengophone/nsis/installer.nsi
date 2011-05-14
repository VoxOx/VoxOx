/**
 * Global variables, already defined.
 *
 * If you add a global variable, declare it here as commented.
 */
;!define PRODUCT_NAME "WengoPhone"
;!define PRODUCT_VERSION "x.y.z"
;!define PRODUCT_VERSION_1 "x"
;!define PRODUCT_VERSION_2 "y"
;!define PRODUCT_VERSION_3 "z"
;!define PRODUCT_VERSION_4 "0"
;!define DEBUG
;!define BUILD_DIR "..\..\release-symbols\"
;!define INSTALLER_NAME "WengoPhone-setup-0.13.exe"
;!define QTDIR "C:\Qt\4.1.1\"
;!define WITH_PDB
;!define BINARY_NAME "qtwengophone"


!define PRODUCT_PUBLISHER "Wengo"
!define PRODUCT_WEB_SITE "http://www.wengo.com"
!define PRODUCT_REGKEY "Software\${PRODUCT_NAME}"
!define PRODUCT_UNINSTALL_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"

; Version information for the installer executable
VIAddVersionKey ProductName "${PRODUCT_NAME}"
VIAddVersionKey ProductVersion "${PRODUCT_VERSION}"
VIAddVersionKey Comments "${PRODUCT_NAME}, a voice over Internet phone"
VIAddVersionKey CompanyName "${PRODUCT_PUBLISHER}"
VIAddVersionKey LegalCopyright "Copyright (C) 2004-2007 Wengo"
VIAddVersionKey LegalTrademarks "Wengo"
VIAddVersionKey FileDescription "${PRODUCT_NAME}"
VIAddVersionKey FileVersion "${PRODUCT_VERSION}"
VIAddVersionKey InternalName "${PRODUCT_NAME}"
VIAddVersionKey OriginalFilename "${INSTALLER_NAME}"
VIProductVersion "${PRODUCT_VERSION_1}.${PRODUCT_VERSION_2}.${PRODUCT_VERSION_3}.${PRODUCT_VERSION_4}"

SetCompressor lzma
XPStyle on

!macro LANG_STRING NAME VALUE
	LangString "${NAME}" "${LANG_${LANG}}" "${VALUE}"
!macroend

!macro LANG_UNSTRING NAME VALUE
	!insertmacro LANG_STRING "un.${NAME}" "${VALUE}"
!macroend


; Modern UI 1.67 compatible
!include "MUI.nsh"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "..\src\presentation\qt\win32\application.ico"
!define MUI_UNICON "..\src\presentation\qt\win32\application.ico"

; Language Selection Dialog Settings
; Remember the installer language
!define MUI_LANGDLL_REGISTRY_ROOT "HKCU"
!define MUI_LANGDLL_REGISTRY_KEY "${PRODUCT_REGKEY}"
!define MUI_LANGDLL_REGISTRY_VALUENAME "Installer Language"

; Pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE $(license)
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_RUN "$INSTDIR\${BINARY_NAME}.exe"
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

; Language files, first language is the default language
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "French"

; Loads license translation
LicenseLangString license ${LANG_ENGLISH} "..\COPYING"
LicenseLangString license ${LANG_FRENCH} "..\COPYING"
LicenseData $(license)

; Reserve files
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

; MUI end

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
/** Installer name (e.g. WengoPhone-setup-0.13.exe). */
OutFile "${INSTALLER_NAME}"

!define WND_CLASS "QWidget"
!define WND_TITLE "WengoPhone"
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
Function .onInit

	!insertmacro TerminateApp

	/** For future use */
	/** TODO: just if an instance is detected */
	/** tells qtwengophone.exe to exit */
	;push 685
	;push "WengoPhone"
	;push "QWidget"
	;call SendMessage

	!insertmacro MUI_LANGDLL_DISPLAY

	Call IsSilent
	Pop $0
	StrCmp $0 1 0 +2
		goto initDone

	Call isUserAdmin
	Pop $R0
	StrCmp $R0 "true" isAdmin
		; Not an admin
		StrCpy $INSTDIR "$DOCUMENTS\${PRODUCT_NAME}"
		goto initDone
	isAdmin:
		; User is admin
		StrCpy $INSTDIR "$PROGRAMFILES\${PRODUCT_NAME}"
	initDone:
FunctionEnd

; Gets installation folder from registry if available
InstallDirRegKey HKCU "${PRODUCT_REGKEY}" ""

ShowInstDetails show
ShowUnInstDetails show

Section BaseSection
	!include "files_install.nsi"
SectionEnd

Section -AdditionalIcons
	WriteIniStr "$INSTDIR\${PRODUCT_PUBLISHER}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
	CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\${PRODUCT_NAME} Website.lnk" "$INSTDIR\${PRODUCT_PUBLISHER}.url"
	CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\${PRODUCT_NAME} Uninstall.lnk" "$INSTDIR\uninstall.exe"
SectionEnd

Section -Post
	; Stores installation folder
	WriteRegStr HKCU "${PRODUCT_REGKEY}" "" $INSTDIR

	; Creates uninstaller
	WriteUninstaller "$INSTDIR\uninstall.exe"

	WriteRegStr HKLM "${PRODUCT_UNINSTALL_KEY}" "DisplayName" "$(^Name)"
	WriteRegStr HKLM "${PRODUCT_UNINSTALL_KEY}" "UninstallString" "$INSTDIR\uninstall.exe"
	WriteRegStr HKLM "${PRODUCT_UNINSTALL_KEY}" "DisplayIcon" "$INSTDIR\${BINARY_NAME}.exe"
	WriteRegStr HKLM "${PRODUCT_UNINSTALL_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
	WriteRegStr HKLM "${PRODUCT_UNINSTALL_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
	WriteRegStr HKLM "${PRODUCT_UNINSTALL_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
SectionEnd

Function un.onInit
	!insertmacro MUI_UNGETLANGUAGE
FunctionEnd

Section Uninstall
	!include "files_uninstall.nsi"
SectionEnd
