# Microsoft Developer Studio Project File - Name="iaxclient_dll" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=iaxclient_dll - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "iaxclient_dll.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "iaxclient_dll.mak" CFG="iaxclient_dll - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "iaxclient_dll - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "iaxclient_dll - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "iaxclient_dll - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseDLL"
# PROP Intermediate_Dir "ReleaseDLL"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_USRDLL" /D "_WINDLL" /YX /FD /c
# ADD CPP /nologo /Gz /MD /w /W0 /GX /I ".." /I "..\..\gsm\inc" /I "..\..\portaudio\include" /I "..\..\portaudio\src\common" /I "..\..\portaudio\pablio" /I "..\..\portmixer\px_common" /I "..\..\libspeex\include" /I "..\..\libiax2\src" /I "..\..\wince" /D "WIN32" /D "NDEBUG" /D "_USRDLL" /D "_WINDLL" /D "PA_NO_DS" /D "PA_NO_ASIO" /D SPEEX_PREPROCESS=1 /D "NEWJB" /D "LIBIAX" /D SPEEX_EC=1 /D inline=__inline /D strncasecmp=strnicmp /D vsnprintf=_vsnprintf /FD /c
# SUBTRACT CPP /X /Fr /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 wsock32.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"ReleaseDLL/iaxclient.dll"

!ELSEIF  "$(CFG)" == "iaxclient_dll - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugDLL"
# PROP Intermediate_Dir "DebugDLL"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_USRDLL" /D "_WINDLL" /YX /FD /GZ /c
# ADD CPP /nologo /Gz /MDd /w /W0 /Gm /GX /Zi /Od /I ".." /I "..\..\gsm\inc" /I "..\..\portaudio\include" /I "..\..\portaudio\src\common" /I "..\..\portaudio\pablio" /I "..\..\portmixer\px_common" /I "..\..\libspeex\include" /I "..\..\libiax2\src" /I "..\..\wince" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_WINDLL" /D "BUILDING_DLL" /D "PA_NO_DS" /D "PA_NO_ASIO" /D SPEEX_PREPROCESS=1 /D "NEWJB" /D "LIBIAX" /D SPEEX_EC=1 /D inline=__inline /D strncasecmp=strnicmp /D vsnprintf=_vsnprintf /FD /GZ /c
# SUBTRACT CPP /Fr /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 msvcrt.lib wsock32.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /map /debug /machine:I386 /out:"DebugDLL/iaxclient.dll"

!ENDIF 

# Begin Target

# Name "iaxclient_dll - Win32 Release"
# Name "iaxclient_dll - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\audio_encode.c
# End Source File
# Begin Source File

SOURCE=..\..\audio_file.c
# End Source File
# Begin Source File

SOURCE=..\..\audio_portaudio.c
# End Source File
# Begin Source File

SOURCE=..\..\codec_alaw.c
# End Source File
# Begin Source File

SOURCE=..\..\codec_gsm.c
# End Source File
# Begin Source File

SOURCE=..\..\codec_speex.c
# End Source File
# Begin Source File

SOURCE=..\..\codec_ulaw.c
# End Source File
# Begin Source File

SOURCE=..\..\libiax2\src\iax.c
# End Source File
# Begin Source File

SOURCE="..\..\libiax2\src\iax2-parser.c"
# End Source File
# Begin Source File

SOURCE=..\iaxclient_dll.c
# End Source File
# Begin Source File

SOURCE=..\..\iaxclient_lib.c
# End Source File
# Begin Source File

SOURCE=..\..\libiax2\src\jitterbuf.c
# End Source File
# Begin Source File

SOURCE=..\..\libiax2\src\md5.c
# End Source File
# Begin Source File

SOURCE=..\..\winfuncs.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\audio_encode.h
# End Source File
# Begin Source File

SOURCE=..\..\audio_file.h
# End Source File
# Begin Source File

SOURCE=..\..\audio_portaudio.h
# End Source File
# Begin Source File

SOURCE=..\..\codec_alaw.h
# End Source File
# Begin Source File

SOURCE=..\..\codec_gsm.h
# End Source File
# Begin Source File

SOURCE=..\..\codec_speex.h
# End Source File
# Begin Source File

SOURCE=..\..\codec_ulaw.h
# End Source File
# Begin Source File

SOURCE=..\..\libiax2\src\iax.h
# End Source File
# Begin Source File

SOURCE="..\..\libiax2\src\iax2-parser.h"
# End Source File
# Begin Source File

SOURCE=..\..\iaxclient.h
# End Source File
# Begin Source File

SOURCE=..\..\iaxclient_lib.h
# End Source File
# Begin Source File

SOURCE=..\..\libiax2\src\jitterbuf.h
# End Source File
# Begin Source File

SOURCE=..\..\libiax2\src\md5.h
# End Source File
# End Group
# Begin Group "DSP"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\spandsp\plc.c
# End Source File
# Begin Source File

SOURCE=..\..\spandsp\plc.h
# End Source File
# End Group
# Begin Group "GSM"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\gsm\src\add.c
# End Source File
# Begin Source File

SOURCE=..\..\gsm\src\code.c
# End Source File
# Begin Source File

SOURCE=..\..\gsm\inc\config.h
# End Source File
# Begin Source File

SOURCE=..\..\gsm\src\debug.c
# End Source File
# Begin Source File

SOURCE=..\..\gsm\src\decode.c
# End Source File
# Begin Source File

SOURCE=..\..\gsm\inc\gsm.h
# End Source File
# Begin Source File

SOURCE=..\..\gsm\src\gsm_create.c
# End Source File
# Begin Source File

SOURCE=..\..\gsm\src\gsm_decode.c
# End Source File
# Begin Source File

SOURCE=..\..\gsm\src\gsm_destroy.c
# End Source File
# Begin Source File

SOURCE=..\..\gsm\src\gsm_encode.c
# End Source File
# Begin Source File

SOURCE=..\..\gsm\src\gsm_explode.c
# End Source File
# Begin Source File

SOURCE=..\..\gsm\src\gsm_implode.c
# End Source File
# Begin Source File

SOURCE=..\..\gsm\src\gsm_option.c
# End Source File
# Begin Source File

SOURCE=..\..\gsm\src\gsm_print.c
# End Source File
# Begin Source File

SOURCE=..\..\gsm\src\k6opt.h
# End Source File
# Begin Source File

SOURCE=..\..\gsm\src\long_term.c
# End Source File
# Begin Source File

SOURCE=..\..\gsm\src\lpc.c

!IF  "$(CFG)" == "iaxclient_dll - Win32 Release"

# PROP Intermediate_Dir "ReleaseDLL\gsm"

!ELSEIF  "$(CFG)" == "iaxclient_dll - Win32 Debug"

# PROP Intermediate_Dir "DebugDLL\gsm"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\gsm\src\preprocess.c

!IF  "$(CFG)" == "iaxclient_dll - Win32 Release"

# PROP Intermediate_Dir "ReleaseDLL\gsm"

!ELSEIF  "$(CFG)" == "iaxclient_dll - Win32 Debug"

# PROP Intermediate_Dir "DebugDLL\gsm"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\gsm\inc\private.h
# End Source File
# Begin Source File

SOURCE=..\..\gsm\inc\proto.h
# End Source File
# Begin Source File

SOURCE=..\..\gsm\src\rpe.c
# End Source File
# Begin Source File

SOURCE=..\..\gsm\src\short_term.c
# End Source File
# Begin Source File

SOURCE=..\..\gsm\src\table.c
# End Source File
# Begin Source File

SOURCE=..\..\gsm\inc\unproto.h
# End Source File
# End Group
# Begin Group "PortAudio"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\portaudio\src\common\pa_allocation.c
# End Source File
# Begin Source File

SOURCE=..\..\portaudio\src\common\pa_allocation.h
# End Source File
# Begin Source File

SOURCE=..\..\portaudio\src\common\pa_converters.c
# End Source File
# Begin Source File

SOURCE=..\..\portaudio\src\common\pa_converters.h
# End Source File
# Begin Source File

SOURCE=..\..\portaudio\src\common\pa_cpuload.c
# End Source File
# Begin Source File

SOURCE=..\..\portaudio\src\common\pa_cpuload.h
# End Source File
# Begin Source File

SOURCE=..\..\portaudio\src\common\pa_dither.c
# End Source File
# Begin Source File

SOURCE=..\..\portaudio\src\common\pa_dither.h
# End Source File
# Begin Source File

SOURCE=..\..\portaudio\src\common\pa_endianness.h
# End Source File
# Begin Source File

SOURCE=..\..\portaudio\src\common\pa_front.c
# End Source File
# Begin Source File

SOURCE=..\..\portaudio\src\common\pa_hostapi.h
# End Source File
# Begin Source File

SOURCE=..\..\portaudio\src\common\pa_process.c
# End Source File
# Begin Source File

SOURCE=..\..\portaudio\src\common\pa_process.h
# End Source File
# Begin Source File

SOURCE=..\..\portaudio\src\common\pa_skeleton.c
# End Source File
# Begin Source File

SOURCE=..\..\portaudio\src\common\pa_stream.c
# End Source File
# Begin Source File

SOURCE=..\..\portaudio\src\common\pa_stream.h
# End Source File
# Begin Source File

SOURCE=..\..\portaudio\src\common\pa_trace.c
# End Source File
# Begin Source File

SOURCE=..\..\portaudio\src\common\pa_trace.h
# End Source File
# Begin Source File

SOURCE=..\..\portaudio\src\common\pa_types.h
# End Source File
# Begin Source File

SOURCE=..\..\portaudio\src\common\pa_util.h
# End Source File
# Begin Source File

SOURCE=..\..\portaudio\src\os\win\pa_win_hostapis.c
# End Source File
# Begin Source File

SOURCE=..\..\portaudio\src\os\win\pa_win_util.c
# End Source File
# Begin Source File

SOURCE=..\..\portaudio\src\hostapi\wmme\pa_win_wmme.c
# End Source File
# Begin Source File

SOURCE=..\..\portaudio\pablio\ringbuffer.c
# End Source File
# Begin Source File

SOURCE=..\..\portaudio\pablio\ringbuffer.h
# End Source File
# End Group
# Begin Group "PortMixer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\portmixer\px_win_wmme\px_win_wmme.cpp
# End Source File
# End Group
# Begin Group "Speex"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\libspeex\arch.h
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\bits.c
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\cb_search.c
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\cb_search.h
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\cb_search_sse.h
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\exc_10_16_table.c
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\exc_10_32_table.c
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\exc_20_32_table.c
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\exc_5_256_table.c
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\exc_5_64_table.c
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\exc_8_128_table.c
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\filters.c
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\filters.h
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\filters_arm4.h
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\filters_sse.h
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\fixed_debug.h
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\fixed_generic.h
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\gain_table.c
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\gain_table_lbr.c
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\hexc_10_32_table.c
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\hexc_table.c
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\high_lsp_tables.c
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\jitter.c
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\lbr_48k_tables.c
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\lpc.c
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\lpc.h
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\lsp.c
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\lsp.h
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\lsp_tables_nb.c
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\ltp.c
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\ltp.h
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\ltp_sse.h
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\math_approx.c
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\math_approx.h
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\mdf.c
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\medfilter.c
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\medfilter.h
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\misc.c
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\misc.h
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\modes.c
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\modes.h
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\nb_celp.c
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\nb_celp.h
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\preprocess.c
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\quant_lsp.c
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\quant_lsp.h
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\sb_celp.c
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\sb_celp.h
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\smallft.c
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\smallft.h
# End Source File
# Begin Source File

SOURCE=..\..\sox\sox.h
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\speex.c
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\speex_callbacks.c
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\speex_header.c
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\stack_alloc.h
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\stereo.c
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\vbr.c
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\vbr.h
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\vq.c
# End Source File
# Begin Source File

SOURCE=..\..\libspeex\vq.h
# End Source File
# End Group
# End Target
# End Project
