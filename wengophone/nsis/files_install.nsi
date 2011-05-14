Delete "$INSTDIR\VoxOx.exe"
Delete "$INSTDIR\*.dll"
Delete "$INSTDIR\sounds\tones\default\*"
Delete "$INSTDIR\sounds\tones\terminator\*"
Delete "$INSTDIR\sounds\tones\*"
Delete "$INSTDIR\sounds\alerts\*"
Delete "$INSTDIR\sounds\ringtones\*"
Delete "$INSTDIR\sounds\*"
Delete "$INSTDIR\lang\*"
Delete "$INSTDIR\webpages\*"
Delete "$INSTDIR\imageformats\*"
Delete "$INSTDIR\css\*"
Delete "$INSTDIR\keypad\*"
Delete "$INSTDIR\chat\*"
Delete "$INSTDIR\html\*"
Delete "$INSTDIR\contactList\*"

Delete "$INSTDIR\vcredist_x86.exe"	#JRT - 2009.11.02

SetOutPath "$INSTDIR"
File "${BUILD_DIR}\VoxOx.exe"
CreateDirectory "$SMPROGRAMS\${PRODUCT_NAME}"
CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\${PRODUCT_NAME}.lnk" "$INSTDIR\VoxOx.exe"
CreateShortCut "$DESKTOP\${PRODUCT_NAME}.lnk" "$INSTDIR\VoxOx.exe"

File "${BUILD_DIR}\COPYING"
File "${BUILD_DIR}\AUTHORS"

#-----------------------------------
# MSVC redistributeables - JRT - 2009.11.02
#-----------------------------------
File "..\..\thirdparty\msvcr71.dll"
File "..\..\thirdparty\vcredist_x86.exe"


/** .dll files. */
File "${BUILD_DIR}\*.dll"
File "dll\dbghelp.dll"

/** .exe files. */
File "${BUILD_DIR}\*.exe"

/** .pdb files. */
!ifdef WITH_PDB
	File "${BUILD_DIR}\*.pdb"
!endif

/** Gaim plugins. */
;SetOutPath "$INSTDIR\plugins\"
;File "${BUILD_DIR}\plugins\*.dll"

/** Sounds. */
SetOutPath "$INSTDIR\sounds\"
File "${BUILD_DIR}\sounds\*"
SetOutPath "$INSTDIR\sounds\alerts\"
File "${BUILD_DIR}\sounds\alerts\*"
SetOutPath "$INSTDIR\sounds\ringtones\"
File "${BUILD_DIR}\sounds\ringtones\*"
SetOutPath "$INSTDIR\sounds\tones\default\"
File "${BUILD_DIR}\sounds\tones\default\*"
SetOutPath "$INSTDIR\sounds\tones\terminator\"
File "${BUILD_DIR}\sounds\tones\terminator\*"

/** Images. */
SetOutPath "$INSTDIR\pics\"
File /r "${BUILD_DIR}\pics\*.*"

/** Css. */
SetOutPath "$INSTDIR\css\"
File /r "${BUILD_DIR}\css\*"

/** Keypad Background. */
SetOutPath "$INSTDIR\keypad\"
File /r "${BUILD_DIR}\keypad\*"

/** Flags. */
SetOutPath "$INSTDIR\flags\"
File /r "${BUILD_DIR}\flags\*"

/** ContactList. */
SetOutPath "$INSTDIR\contactList\"
File /r "${BUILD_DIR}\contactList\*"

/** Chat. */
SetOutPath "$INSTDIR\chat\"
File /r "${BUILD_DIR}\chat\*"

/** HTML. */
SetOutPath "$INSTDIR\html\"
File /r "${BUILD_DIR}\html\*"

/** Dics. */
SetOutPath "$INSTDIR\dics\"
File /r "${BUILD_DIR}\dics\*"

/** Config. */
SetOutPath "$INSTDIR\config\"
File "${BUILD_DIR}\config\*"

/* Translations. */
SetOutPath "$INSTDIR\lang\"
File "${BUILD_DIR}\lang\*.qm"

/** Web pages. */
SetOutPath "$INSTDIR\webpages\"
File "${BUILD_DIR}\webpages\*"

/** Imageformats (GIF, MNG, JPEG) from Qt. */
SetOutPath "$INSTDIR\imageformats\"
File "${BUILD_DIR}\imageformats\*"

/** Phapi plugins */
SetOutPath "$INSTDIR\phapi-plugins\"
File "${BUILD_DIR}\phapi-plugins\*"

/**
 * So that we can execute wengophone.exe
 */
SetOutPath "$INSTDIR"
