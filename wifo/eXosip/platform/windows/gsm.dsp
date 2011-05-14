# Microsoft Developer Studio Project File - Name="gsm" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=gsm - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "gsm.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "gsm.mak" CFG="gsm - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "gsm - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "gsm - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "gsm - Win32 Release"

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
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "gsm - Win32 Debug"

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
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "_MT" /D "__STDC__" /YX /FD /GZ /c
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "gsm - Win32 Release"
# Name "gsm - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\phapi\gsm\add.c
# End Source File
# Begin Source File

SOURCE=..\..\phapi\gsm\code.c
# End Source File
# Begin Source File

SOURCE=..\..\phapi\gsm\debug.c
# End Source File
# Begin Source File

SOURCE=..\..\phapi\gsm\decode.c
# End Source File
# Begin Source File

SOURCE=..\..\phapi\gsm\gsm_create.c
# End Source File
# Begin Source File

SOURCE=..\..\phapi\gsm\gsm_decode.c
# End Source File
# Begin Source File

SOURCE=..\..\phapi\gsm\gsm_destroy.c
# End Source File
# Begin Source File

SOURCE=..\..\phapi\gsm\gsm_encode.c
# End Source File
# Begin Source File

SOURCE=..\..\phapi\gsm\gsm_explode.c
# End Source File
# Begin Source File

SOURCE=..\..\phapi\gsm\gsm_implode.c
# End Source File
# Begin Source File

SOURCE=..\..\phapi\gsm\gsm_option.c
# End Source File
# Begin Source File

SOURCE=..\..\phapi\gsm\gsm_print.c
# End Source File
# Begin Source File

SOURCE=..\..\phapi\gsm\long_term.c
# End Source File
# Begin Source File

SOURCE=..\..\phapi\gsm\lpc.c
# End Source File
# Begin Source File

SOURCE=..\..\phapi\gsm\preprocess.c
# End Source File
# Begin Source File

SOURCE=..\..\phapi\gsm\rpe.c
# End Source File
# Begin Source File

SOURCE=..\..\phapi\gsm\short_term.c
# End Source File
# Begin Source File

SOURCE=..\..\phapi\gsm\table.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\phapi\gsm\config.h
# End Source File
# Begin Source File

SOURCE=..\..\phapi\gsm\gsm.h
# End Source File
# Begin Source File

SOURCE=..\..\phapi\gsm\private.h
# End Source File
# Begin Source File

SOURCE=..\..\phapi\gsm\proto.h
# End Source File
# Begin Source File

SOURCE=..\..\phapi\gsm\unproto.h
# End Source File
# End Group
# End Target
# End Project
