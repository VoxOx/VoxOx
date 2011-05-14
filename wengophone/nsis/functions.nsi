!include WinMessages.nsh
!include i18n.nsi

; USAGE:
; push int		# message
; push string	# HWNDTITLE
; push string	# HWNDCLASS
; call SendMessage
Function SendMessage
	pop $0
	pop $1
	pop $2

	FindWindow $3 $0 $1
	IntCmp $3 0 errornosuchwindow
	IsWindow $3 0 errornotawindow

	; stands for WM_USER + xxx
	System::Int64Op 1024 + $2
	Pop $4
	System::Call 'user32::PostMessageA(i,i,i,i) i($3,$4,0,0)'

errornosuchwindow:
	;MessageBox MB_ICONSTOP|MB_OK "Error:  no such window!"
	Goto done
errornotawindow:
	;MessageBox MB_ICONSTOP|MB_OK "Error: this is not a window!!"
	Goto done
done:
FunctionEnd

; USAGE:
; push string	# HWNDTITLE
; push string	# HWNDCLASS
; call SendMessage
Function isRunning
	pop $0
	pop $1
	FindWindow $2 $0 $1

	StrCmp $2 0 continueInstall
		MessageBox MB_ICONSTOP|MB_OK "$(i18n_already_running_message)"
		Abort
continueInstall:
FunctionEnd


; Function that checks if the user is an Administrator
; Usage:
; Call isUserAdmin
; Pop $R0
; StrCmp $R0 "true" isAdmin
; 	; Not an admin
; 	StrCpy $INSTDIR "$DOCUMENTS\${PRODUCT_NAME}"
; 	SetShellVarContext current
; 	goto initDone
; isAdmin:
; 	; User is admin
; 	StrCpy $INSTDIR "$PROGRAMFILES\${PRODUCT_NAME}"
; 	SetShellVarContext all
; initDone:
Function isUserAdmin
	Push $R0
	Push $R1
	Push $R2

	ClearErrors
	UserInfo::GetName
	IfErrors Win9x
	Pop $R1
	UserInfo::GetAccountType
	Pop $R2

	StrCmp $R2 "Admin" 0 Continue
	StrCpy $R0 "true"
	Goto Done

	Continue:
		StrCmp $R2 "" Win9x
	StrCpy $R0 "false"
		Goto Done

	Win9x:
		StrCpy $R0 "true"

	Done:
	Pop $R2
	Pop $R1
	Exch $R0
FunctionEnd



Function StrStr
  Exch $R1 ; st=haystack,old$R1, $R1=needle
  Exch    ; st=old$R1,haystack
  Exch $R2 ; st=old$R1,old$R2, $R2=haystack
  Push $R3
  Push $R4
  Push $R5
  StrLen $R3 $R1
  StrCpy $R4 0
  ; $R1=needle
  ; $R2=haystack
  ; $R3=len(needle)
  ; $R4=cnt
  ; $R5=tmp
  loop:
    StrCpy $R5 $R2 $R3 $R4
    StrCmp $R5 $R1 done
    StrCmp $R5 "" done
    IntOp $R4 $R4 + 1
    Goto loop
  done:
  StrCpy $R1 $R2 "" $R4
  Pop $R5
  Pop $R4
  Pop $R3
  Pop $R2
  Exch $R1
FunctionEnd

Function IsSilent
  Push $0
  Push $CMDLINE
  Push "/S"
  Call StrStr
  Pop $0
  StrCpy $0 $0 3
  StrCmp $0 "/S" silent
  StrCmp $0 "/S " silent
    StrCpy $0 0
    Goto notsilent
  silent: StrCpy $0 1
  notsilent: Exch $0
FunctionEnd
