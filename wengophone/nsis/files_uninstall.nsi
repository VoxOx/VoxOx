KillProcDLL::KillProc "voxox.exe"

Delete "$INSTDIR\styles\*"
RMDir "$INSTDIR\styles\"

Delete "$INSTDIR\icons\*"
RMDir "$INSTDIR\icons\"

Delete "$INSTDIR\lang\*"
RMDir "$INSTDIR\lang\"

RMDir /r "$INSTDIR\sounds\"
RMDir "$INSTDIR\sounds\"

Delete "$INSTDIR\sounds\*"
RMDir "$INSTDIR\sounds\"

Delete "$INSTDIR\emoticons\*"
RMDir "$INSTDIR\emoticons\"

Delete "$INSTDIR\plugins\*"
RMDir "$INSTDIR\plugins\"

Delete "$INSTDIR\config\*"
RMDir "$INSTDIR\config\"

RMDir /r "$INSTDIR\contactList\"
RMDir "$INSTDIR\contactList\" 	

RMDir /r "$INSTDIR\dics\"
RMDir "$INSTDIR\dics\" 	

RMDir /r "$INSTDIR\html\"
RMDir "$INSTDIR\html\" 	

Delete "$INSTDIR\css\*"
RMDir "$INSTDIR\css\"

Delete "$INSTDIR\flags\*"
RMDir "$INSTDIR\flags\"

Delete "$INSTDIR\keypad\*"
RMDir "$INSTDIR\keypad\"

RMDir /r "$INSTDIR\pics\"

/* Delete css dir if it was left empty */
Delete "$INSTDIR\css\README.txt"
RMDir "$INSTDIR\css\"

/* Delete distributed themes but not custom ones */
RMDir /r "$INSTDIR\chat\"
RMDir "$INSTDIR\chat\"

Delete "$INSTDIR\webpages\windows\*"
RMDir "$INSTDIR\webpages\windows\"

Delete "$INSTDIR\webpages\*"
RMDir "$INSTDIR\webpages\"

Delete "$INSTDIR\imageformats\*"
RMDir "$INSTDIR\imageformats\"

Delete "$INSTDIR\phapi-plugins\*"
RMDir "$INSTDIR\phapi-plugins\"

Delete "$INSTDIR\*"
RMDir "$INSTDIR"

Delete "$SMPROGRAMS\VoxOx\*"
Delete "$DESKTOP\VoxOx.lnk"
RMDir  "$SMPROGRAMS\VoxOx\"

/**
 * Removes the configuration file + the contacts.
 * Only if the user asks for it.
 */
/*Delete "${WENGODIR}\contacts\*"
RMDir "${WENGODIR}\contacts\"
Delete "${WENGODIR}\*"
RMDir "${WENGODIR}"*/



