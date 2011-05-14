# Microsoft Developer Studio Project File - Name="iaxclient_lib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=iaxclient_lib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "iax1lib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "iax1lib.mak" CFG="iaxclient_lib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "iaxclient_lib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "iaxclient_lib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "iaxclient_lib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /w /W0 /GX /I ".\libiax\src" /I ".\gsm\inc" /I ".\portaudio\pa_common" /I ".\portaudio\pablio" /D "WIN32" /D SPEEX_PREPROCESS=1 /D SPEEX_EC=1 /D "NDEBUG" /D "_LIB" /FR /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Release\iaxclient1.lib"

!ELSEIF  "$(CFG)" == "iaxclient_lib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /w /W0 /Gm /GX /ZI /Od /I ".\libiax\src" /I ".\gsm\inc" /I ".\portaudio\pa_common" /I ".\portaudio\pablio" /I ".\libspeex" /D "WIN32" /D SPEEX_PREPROCESS=1 /D SPEEX_EC=1 /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug\iaxclient1.lib"

!ENDIF 

# Begin Target

# Name "iaxclient_lib - Win32 Release"
# Name "iaxclient_lib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\gsm\src\add.c
# End Source File
# Begin Source File

SOURCE=.\audio_encode.c
# End Source File
# Begin Source File

SOURCE=.\audio_file.c
# End Source File
# Begin Source File

SOURCE=.\audio_portaudio.c
# End Source File
# Begin Source File

SOURCE=.\audio_win32.c
# End Source File
# Begin Source File

SOURCE=.\gsm\src\code.c
# End Source File
# Begin Source File

SOURCE=.\sox\compand.c
# End Source File
# Begin Source File

SOURCE=.\gsm\src\debug.c
# End Source File
# Begin Source File

SOURCE=.\gsm\src\decode.c
# End Source File
# Begin Source File

SOURCE=.\gsm\src\gsm_create.c
# End Source File
# Begin Source File

SOURCE=.\gsm\src\gsm_decode.c
# End Source File
# Begin Source File

SOURCE=.\gsm\src\gsm_destroy.c
# End Source File
# Begin Source File

SOURCE=.\gsm\src\gsm_encode.c
# End Source File
# Begin Source File

SOURCE=.\gsm\src\gsm_explode.c
# End Source File
# Begin Source File

SOURCE=.\gsm\src\gsm_implode.c
# End Source File
# Begin Source File

SOURCE=.\gsm\src\gsm_option.c
# End Source File
# Begin Source File

SOURCE=.\gsm\src\gsm_print.c
# End Source File
# Begin Source File

SOURCE=.\libiax\src\iax.c
# End Source File
# Begin Source File

SOURCE=.\iaxclient_lib.c
# End Source File
# Begin Source File

SOURCE=.\gsm\src\long_term.c
# End Source File
# Begin Source File

SOURCE=.\gsm\src\lpc.c
# End Source File
# Begin Source File

SOURCE=.\libiax\src\md5.c
# End Source File
# Begin Source File

SOURCE=.\libspeex\misc.c
# End Source File
# Begin Source File

SOURCE=.\portaudio\pa_common\pa_lib.c
# End Source File
# Begin Source File

SOURCE=.\portaudio\pa_win_wmme\pa_win_wmme.c
# End Source File
# Begin Source File

SOURCE=.\portaudio\pablio\pablio.c
# End Source File
# Begin Source File

SOURCE=.\gsm\src\preprocess.c

!IF  "$(CFG)" == "iaxclient_lib - Win32 Release"

# PROP Intermediate_Dir "Release\gsm"

!ELSEIF  "$(CFG)" == "iaxclient_lib - Win32 Debug"

# PROP Intermediate_Dir "Debug\gsm"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\libspeex\preprocess.c
# End Source File
# Begin Source File

SOURCE=.\portaudio\pablio\ringbuffer.c
# End Source File
# Begin Source File

SOURCE=.\gsm\src\rpe.c
# End Source File
# Begin Source File

SOURCE=.\gsm\src\short_term.c
# End Source File
# Begin Source File

SOURCE=.\libspeex\smallft.c
# End Source File
# Begin Source File

SOURCE=.\sox\soxcompat.c
# End Source File
# Begin Source File

SOURCE=.\gsm\src\table.c
# End Source File
# Begin Source File

SOURCE=.\winfuncs.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\libiax\src\answer.h
# End Source File
# Begin Source File

SOURCE=.\audio_encode.h
# End Source File
# Begin Source File

SOURCE=.\audio_portaudio.h
# End Source File
# Begin Source File

SOURCE=.\audio_win32.h
# End Source File
# Begin Source File

SOURCE=.\libiax\src\frame.h
# End Source File
# Begin Source File

SOURCE=".\libiax\src\iax-client.h"
# End Source File
# Begin Source File

SOURCE=.\libiax\src\iax.h
# End Source File
# Begin Source File

SOURCE=.\iaxclient_lib.h
# End Source File
# Begin Source File

SOURCE=.\gsm\src\k6opt.h
# End Source File
# Begin Source File

SOURCE=.\libiax\src\md5.h
# End Source File
# Begin Source File

SOURCE=.\libspeex\misc.h
# End Source File
# Begin Source File

SOURCE=.\libspeex\smallft.h
# End Source File
# Begin Source File

SOURCE=.\sox\sox.h
# End Source File
# Begin Source File

SOURCE=.\libspeex\speex_preprocess.h
# End Source File
# Begin Source File

SOURCE=.\libiax\src\winpoop.h
# End Source File
# End Group
# End Target
# End Project
