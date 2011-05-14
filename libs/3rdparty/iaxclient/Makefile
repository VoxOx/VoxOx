#
# iaxclient: a cross-platform IAX softphone library
#
# Copyrights:
# Copyright (C) 2003 HorizonLive.com, (c) 2004, Horizon Wimba, Inc.
#
# Contributors <see README for complete list>
# Steve Kann <stevek@stevek.com>
# Michael Van Donselaar <mvand@vandonselaar.org>
# Shawn Lawrence <shawn.lawrence@terracecomm.com>
#
#
# This program is free software, distributed under the terms of
# the GNU Lesser (Library) General Public License
#



CODEC_ILBC=0

SPEEX_PREPROCESS=1
SPAN_EC=0
SPEEX_EC=1
MEC2_EC=0

USE_NEWJB=1

USE_VIDEO=0

# For Linux, you can choose between portaudio with OSS, Alsa and Jack backend, 
# and native Alsa
# Choice is inclusive
USE_PA_OSS=1
USE_PA_ALSA=1
USE_PA_JACK=1
AUDIO_ALSA=0

CFLAGS= -I. -Igsm/inc \
	-Iportaudio/include -Iportaudio/src/common -Iportaudio/pablio -Iportmixer/px_common \
	-Ilibspeex/include --std=c99 -Wall -Wmissing-noreturn

#CFLAGS:= $(CFLAGS) -g -Wall -Wpointer-arith
CFLAGS:= $(CFLAGS) -g -O2

CFLAGS:= $(CFLAGS) -DLIBVER='"SVN $(shell svnversion .)"'

ifeq ($(SPEEX_PREPROCESS),1)
CFLAGS:= $(CFLAGS) -DSPEEX_PREPROCESS=1
endif

ifeq ($(USE_NEWJB),1)
CFLAGS:= $(CFLAGS) -DNEWJB
endif

# debug flags
# CFLAGS := $(CFLAGS) -DDEBUG_SUPPORT -DDEBUG_DEFAULT -DEXTREME_DEBUG -D_DEBUG 
# CFLAGS := $(CFLAGS) -DDEBUG_SUPPORT -DDEBUG_DEFAULT -D_DEBUG
# enable this for API level debug info for PortAudio
# CFLAGS:= $(CFLAGS) -DPA_LOG_API_CALLS

OBJS=\
	portaudio/src/common/pa_allocation.o \
	portaudio/src/common/pa_converters.o \
	portaudio/src/common/pa_cpuload.o \
	portaudio/src/common/pa_dither.o \
	portaudio/src/common/pa_front.o \
	portaudio/src/common/pa_process.o \
	portaudio/src/common/pa_skeleton.o \
	portaudio/src/common/pa_stream.o \
	portaudio/src/common/pa_trace.o \
	spandsp/plc.o \
	audio_encode.o \
	audio_portaudio.o \
	audio_file.o \
	codec_gsm.o \
	codec_ulaw.o \
	codec_alaw.o \
	codec_speex.o \
	iaxclient_lib.o 

OBJS_GSM=\
	gsm/src/add.o \
	gsm/src/code.o \
	gsm/src/debug.o \
	gsm/src/decode.o \
	gsm/src/gsm_create.o \
	gsm/src/gsm_decode.o \
	gsm/src/gsm_destroy.o \
	gsm/src/gsm_encode.o \
	gsm/src/gsm_explode.o \
	gsm/src/gsm_implode.o \
	gsm/src/gsm_option.o \
	gsm/src/gsm_print.o \
	gsm/src/long_term.o \
	gsm/src/lpc.o \
	gsm/src/preprocess.o \
	gsm/src/rpe.o \
	gsm/src/short_term.o \
	gsm/src/table.o

OBJS_LIBSPEEX=\
        libspeex/nb_celp.o \
        libspeex/sb_celp.o \
        libspeex/lpc.o \
        libspeex/ltp.o \
        libspeex/lsp.o \
        libspeex/quant_lsp.o \
        libspeex/lsp_tables_nb.o \
        libspeex/gain_table.o \
        libspeex/gain_table_lbr.o \
        libspeex/cb_search.o \
        libspeex/filters.o \
        libspeex/bits.o \
        libspeex/modes.o \
        libspeex/speex.o \
        libspeex/vq.o \
        libspeex/high_lsp_tables.o \
        libspeex/vbr.o \
        libspeex/hexc_table.o \
        libspeex/exc_5_256_table.o \
        libspeex/exc_5_64_table.o \
        libspeex/exc_8_128_table.o \
        libspeex/exc_10_32_table.o \
        libspeex/exc_10_16_table.o \
        libspeex/exc_20_32_table.o \
        libspeex/hexc_10_32_table.o \
        libspeex/misc.o \
        libspeex/speex_header.o \
        libspeex/speex_callbacks.o \
        libspeex/math_approx.o \
        libspeex/stereo.o \
        libspeex/preprocess.o \
        libspeex/smallft.o \
        libspeex/lbr_48k_tables.o \
        libspeex/jitter.o \
        libspeex/mdf.o

OBJS_ILBC=\
	iLBC/anaFilter.o\
	iLBC/iCBSearch.o\
	iLBC/packing.o\
	iLBC/constants.o\
	iLBC/gainquant.o\
	iLBC/iLBC_decode.o\
	iLBC/StateConstructW.o \
	iLBC/createCB.o\
	iLBC/getCBvec.o\
	iLBC/iLBC_encode.o\
	iLBC/StateSearchW.o\
	iLBC/doCPLC.o\
	iLBC/helpfun.o\
	iLBC/syntFilter.o\
	iLBC/enhancer.o\
	iLBC/hpInput.o\
	iLBC/LPCdecode.o\
	iLBC/filter.o\
	iLBC/hpOutput.o\
	iLBC/LPCencode.o\
	iLBC/FrameClassify.o\
	iLBC/iCBConstruct.o\
	iLBC/lsf.o

OBJS_VIDEO=\
	video.o\
	video_portvideo.o\
	PortVideoSDL/common/cameraTool.o\
	PortVideoSDL/common/RingBuffer.o\
	PortVideoSDL/common/FrameInverter.o\
	codec_theora.o


OBJS:= $(OBJS) $(OBJS_GSM) $(OBJS_LIBSPEEX) 

ifeq ($(CODEC_ILBC),1)
OBJS:= $(OBJS) $(OBJS_ILBC) codec_ilbc.o
CFLAGS:= $(CFLAGS) -DCODEC_ILBC
endif

OBJS_WIN32=\
	winfuncs.o \
	portmixer/px_win_wmme/px_win_wmme.o \
	portaudio/src/hostapi/wmme/pa_win_wmme.o \
	portaudio/src/os/win/pa_win_hostapis.o \
	portaudio/src/os/win/pa_win_util.o \
	portaudio/pablio/ringbuffer.o

OBJS_LINUX=\
	unixfuncs.o \
	portmixer/px_unix_oss/px_unix_oss.o \
	portaudio/src/os/unix/pa_unix_util.o \
	portaudio/src/os/unix/pa_unix_hostapis.o

OBJS_SOLARIS=\
	unixfuncs.o \
	portmixer/px_solaris/px_solaris.o \
	portaudio/pa_unix_oss/pa_unix_solaris.o \
	portaudio/pa_unix_oss/pa_unix.o 

OBJS_MACOSX=\
	unixfuncs.o \
	portmixer/px_mac_core/px_mac_core.o \
	portaudio/src/hostapi/coreaudio/pa_mac_core.o \
	portaudio/src/hostapi/coreaudio/pa_mac_core_utilities.o \
	portaudio/src/hostapi/coreaudio/pa_mac_core_blocking.o \
	portaudio/src/os/unix/pa_unix_util.o \
	portaudio/src/os/mac_osx/pa_mac_hostapis.o \
	portaudio/src/hostapi/coreaudio/ringbuffer.o

OBJS_IAX2=\
        libiax2/src/md5.o \
        libiax2/src/iax.o \
        libiax2/src/iax2-parser.o \
        libiax2/src/jitterbuf.o


LIB=libiaxclient.a

RANLIB= echo

all: $(LIB)

# glean the platform
# We will define this as either Linux, Win32, SOLARIS, or MacOSX
ifneq (,$(findstring Linux,$(shell uname)))
OSTYPE=LINUX
else
ifneq (,$(findstring Darwin,$(shell uname)))
OSTYPE=MACOSX
else
# CYGWIN reports CYGWIN_NT-5.0 under Win2K
ifneq (,$(findstring WIN,$(shell uname)))
OSTYPE=WIN32
else
ifneq (,$(findstring MINGW,$(shell uname)))
OSTYPE=WIN32
else
ifneq (,$(findstring SunOS,$(shell uname)))
OSTYPE=SOLARIS
else
$(warning OSTYPE cannot be detected, assuming Linux)
OSTYPE=LINUX
endif
endif
endif
endif
endif

# Setup platform-specific stuff based on gleaning
ifeq ($(OSTYPE),WIN32)
OBJS:= $(OBJS) $(OBJS_WIN32) 
CFLAGS:= -mno-cygwin -DPA_NO_DS -DPA_NO_ASIO $(CFLAGS)
DYNLDFLAGS= -mno-cygwin -lwinmm -lwsock32 -lcomdlg32 -luser32 -lgdi32 -lole32 -lcomctl32 -lctl3d32 \
   -lstdc++ -lshell32 -loleaut32 -ladvapi32 -luuid -Wl,--output-def,iaxclient.def,--add-stdcall-alias,--enable-stdcall-fixup,--out-implib,libiaxclient.a
DYNCFLAGS=-DBUILDING_DLL
DYNLIB=iaxclient.dll
endif

ifeq ($(OSTYPE),LINUX)
CFLAGS:= $(CFLAGS) -Iportaudio/src/os/unix
DYNCFLAGS=-fPIC
DYNLIB=libiaxclient.so
ifeq ($(USE_PA_OSS),1)
OBJS_LINUX:= $(OBJS_LINUX) portaudio/src/hostapi/oss/pa_unix_oss.o
CFLAGS:= $(CFLAGS) -DPA_USE_OSS
endif
ifeq ($(USE_PA_ALSA),1)
OBJS_LINUX:= $(OBJS_LINUX) portaudio/src/hostapi/alsa/pa_linux_alsa.o
CFLAGS:= $(CFLAGS) -DPA_USE_ALSA
endif
ifeq ($(USE_PA_JACK),1)
OBJS_LINUX:= $(OBJS_LINUX) portaudio/src/hostapi/jack/pa_jack.o
CFLAGS:= $(CFLAGS) -DPA_USE_JACK
endif
OBJS_LINUX:= $(OBJS_LINUX) portaudio/pablio/ringbuffer.o
OBJS:= $(OBJS) $(OBJS_LINUX) 
endif # linux

ifeq ($(OSTYPE),SOLARIS)
OBJS:= $(OBJS) $(OBJS_SOLARIS) 
# only SPARC solaris machines need the alignment stuff
ifeq (sparc,$(findstring sparc,$(shell uname -a)))
CFLAGS:= $(CFLAGS) -DALIGN32 
endif
DYNCFLAGS=-fPIC
DYNLIB=libiaxclient.so
CC=gcc
endif

ifeq ($(OSTYPE),MACOSX)
OBJS:= $(OBJS) $(OBJS_MACOSX) 
RANLIB= ranlib
# comment these lines in to build against version  10.2.7.
#CC=MACOSX_DEPLOYMENT_TARGET=10.2 NEXT_ROOT=/Developer/SDKs/MacOSX10.2.7.sdk cc
CFLAGS:= $(CFLAGS) -DPA_USE_COREAUDIO=1 -Iportaudio/src/hostapi/coreaudio

#architecture
CC=NEXT_ROOT=/Developer/SDKs/MacOSX10.4u.sdk/ gcc -arch i386 -arch ppc


#CFLAGS:= $(CFLAGS) -isystem /Developer/SDKs/MacOSX10.2.7.sdk/usr/include
# Build Universal Binaries on MacOSX
CC=NEXT_ROOT=/Developer/SDKs/MacOSX10.4u.sdk/ MACOSX_DEPLOYMENT_TARGET=10.3 gcc -arch i386 -arch ppc
DYNLIB=libiaxclient.dylib
endif

ifeq ($(USE_VIDEO),1)
	OBJS:= $(OBJS) $(OBJS_VIDEO) 
	CFLAGS:= $(CFLAGS) -DIAXC_VIDEO
	LIBS:= $(LIBS) -lavcodec -ltheora
  ifeq ($(OSTYPE),LINUX)
	OBJS:= $(OBJS) PortVideoSDL/linux/linuxfwCamera.o PortVideoSDL/linux/v4linuxCamera.o
  endif
  ifeq ($(OSTYPE),WIN32)
	CFLAGS:= $(CFLAGS) -IPortVideoSDL/win32/dslib/include -Ic:/DX90SDK/include-gcc -Ic:/DX90SDK/Samples/C++/DirectShow/BaseClasses
	OBJS:= $(OBJS) PortVideoSDL/win32/dslibCamera.cpp
  endif
  ifeq ($(OSTYPE),MACOSX)
        CFLAGS:= $(CFLAGS) -I../../theora/include -I/sw/include
        OBJS:= $(OBJS) PortVideoSDL/macosx/macvdCamera.o PortVideoSDL/macosx/vdigGrab.o
  endif
endif


ifeq ($(AUDIO_ALSA),1)
OBJS:= $(OBJS) audio_alsa.o
CFLAGS:= $(CFLAGS) -DAUDIO_ALSA
endif


OBJS:= $(OBJS) $(OBJS_IAX2)
#CFLAGS:= $(CFLAGS) -Ilibiax2/src -DIAXC_IAX2 -DLIBIAX -DDEBUG_SUPPORT 
CFLAGS:= $(CFLAGS) -Ilibiax2/src -DLIBIAX 

ifeq ($(SPAN_EC),1)
OBJS:= $(OBJS) ec/echo.o
CFLAGS:= $(CFLAGS) -DSPAN_EC=1
endif

ifeq ($(SPEEX_EC),1)
CFLAGS:= $(CFLAGS) -DSPEEX_EC=1
endif

ifeq ($(MEC2_EC),1)
CFLAGS:= $(CFLAGS) -DUSE_MEC2=1
endif

CFLAGS:= $(CFLAGS) -D$(OSTYPE)


# Treat some included libraries specially:
# compile gsm w/o warnings, because it was written to less strict warning systems.
gsm/%.o: gsm/%.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) -w $< -o $@

# pa_linux_alsa and pa_jack don't compile with c99
portaudio/src/hostapi/alsa/pa_linux_alsa.o: portaudio/src/hostapi/alsa/pa_linux_alsa.c
	$(CC) -c $(CFLAGS) --std=gnu89 $< -o $@
portaudio/src/hostapi/jack/pa_jack.o: portaudio/src/hostapi/jack/pa_jack.c
	$(CC) -c $(CFLAGS) --std=gnu89 $< -o $@


$(LIB): $(OBJS)
	rm -f $(LIB)
	ar cru $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

#	libtool -static -o $(LIB) $(OBJS) portaudio/lib/libportaudio.a
#	ar cru $(LIB) $(OBJS) 


MSLIB=iaxclient.lib
MGLIBDIR=c:/mingw/lib
GCCLIBDIR=$(MGLIBDIR)/gcc-lib/mingw32/3.2/

GCCLIB=$(GCCLIBDIR)/libgcc.a 

CXXFLAGS=$(CFLAGS)

$(MSLIB): $(OBJS)
#	rm -f $(MSLIB)
	cp $(GCCLIB) $(MSLIB)
	ar rs $(MSLIB) $(OBJS) 


#shared: CFLAGS := $(CFLAGS) $(DYNCFLAGS)
#shared: clean libiaxclient.a
#shared: libiaxclient.a
#ifeq ($(OSTYPE),MACOSX)
#	$(CC) -single_module -dylib -dynamiclib -o $(DYNLIB) $(OBJS) $(DYNLDFLAGS) -framework CoreAudio -framework AudioToolbox -framework System -framework Carbon -framework AudioUnit portaudio/lib/libportaudio.a
#else
#	$(CC) -shared -o $(DYNLIB) $(OBJS) $(DYNLDFLAGS)
#endif



plat:
	echo Platform is $(OSTYPE)
	echo OBJS are $(OBJS)
	echo OBJS_LINUX are $(OBJS_LINUX)

clean:
	rm -f $(OBJS) $(LIB)
