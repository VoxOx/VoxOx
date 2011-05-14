# Wedit Makefile for project libgsm
SRCDIR=z:\utils\tools\iax\dll\gsm
CFLAGS=-Ic:\programme\lcc\include -Iz:\utils\tools\iax\dll\gsm\inc  -g2 
CC=c:\programme\lcc\bin\lcc.exe
LINKER=c:\programme\lcc\bin\lcclnk.exe
OBJS=\
	table.obj \
	short_term.obj \
	rpe.obj \
	preprocess.obj \
	lpc.obj \
	long_term.obj \
	gsm_print.obj \
	gsm_option.obj \
	gsm_implode.obj \
	gsm_explode.obj \
	gsm_encode.obj \
	gsm_destroy.obj \
	gsm_decode.obj \
	gsm_create.obj \
	decode.obj \
	debug.obj \
	code.obj \
	add.obj

LIBS=

libgsm.lib:	$(OBJS) Makefile
	c:\programme\lcc\bin\lcclib.exe /OUT:z:\utils\tools\iax\dll\gsm\lcc\libgsm.lib $(OBJS)

# Build TABLE.C
TABLE_C=\
	$(SRCDIR)\inc\private.h\
	$(SRCDIR)\inc\proto.h\
	$(SRCDIR)\inc\unproto.h\
	$(SRCDIR)\inc\gsm.h\

table.obj: $(TABLE_C) $(SRCDIR)\src\table.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\src\table.c

# Build SHORT_TERM.C
SHORT_TERM_C=\
	$(SRCDIR)\inc\private.h\
	$(SRCDIR)\inc\proto.h\
	$(SRCDIR)\inc\unproto.h\
	$(SRCDIR)\inc\gsm.h\
	$(SRCDIR)\inc\proto.h\

short_term.obj: $(SHORT_TERM_C) $(SRCDIR)\src\short_term.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\src\short_term.c

# Build RPE.C
RPE_C=\
	$(SRCDIR)\inc\private.h\
	$(SRCDIR)\inc\proto.h\
	$(SRCDIR)\inc\unproto.h\
	$(SRCDIR)\inc\gsm.h\
	$(SRCDIR)\inc\proto.h\

rpe.obj: $(RPE_C) $(SRCDIR)\src\rpe.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\src\rpe.c

# Build PREPROCESS.C
PREPROCESS_C=\
	$(SRCDIR)\inc\private.h\
	$(SRCDIR)\inc\proto.h\
	$(SRCDIR)\inc\unproto.h\
	$(SRCDIR)\inc\gsm.h\
	$(SRCDIR)\inc\proto.h\

preprocess.obj: $(PREPROCESS_C) $(SRCDIR)\src\preprocess.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\src\preprocess.c

# Build LPC.C
LPC_C=\
	$(SRCDIR)\inc\private.h\
	$(SRCDIR)\inc\proto.h\
	$(SRCDIR)\inc\unproto.h\
	$(SRCDIR)\inc\gsm.h\
	$(SRCDIR)\inc\proto.h\

lpc.obj: $(LPC_C) $(SRCDIR)\src\lpc.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\src\lpc.c

# Build LONG_TERM.C
LONG_TERM_C=\
	$(SRCDIR)\inc\private.h\
	$(SRCDIR)\inc\proto.h\
	$(SRCDIR)\inc\unproto.h\
	$(SRCDIR)\inc\gsm.h\
	$(SRCDIR)\inc\proto.h\

long_term.obj: $(LONG_TERM_C) $(SRCDIR)\src\long_term.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\src\long_term.c

# Build GSM_PRINT.C
GSM_PRINT_C=\
	$(SRCDIR)\inc\private.h\
	$(SRCDIR)\inc\proto.h\
	$(SRCDIR)\inc\unproto.h\
	$(SRCDIR)\inc\gsm.h\
	$(SRCDIR)\inc\proto.h\

gsm_print.obj: $(GSM_PRINT_C) $(SRCDIR)\src\gsm_print.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\src\gsm_print.c

# Build GSM_OPTION.C
GSM_OPTION_C=\
	$(SRCDIR)\inc\private.h\
	$(SRCDIR)\inc\proto.h\
	$(SRCDIR)\inc\unproto.h\
	$(SRCDIR)\inc\gsm.h\
	$(SRCDIR)\inc\proto.h\

gsm_option.obj: $(GSM_OPTION_C) $(SRCDIR)\src\gsm_option.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\src\gsm_option.c

# Build GSM_IMPLODE.C
GSM_IMPLODE_C=\
	$(SRCDIR)\inc\private.h\
	$(SRCDIR)\inc\proto.h\
	$(SRCDIR)\inc\unproto.h\
	$(SRCDIR)\inc\gsm.h\
	$(SRCDIR)\inc\proto.h\

gsm_implode.obj: $(GSM_IMPLODE_C) $(SRCDIR)\src\gsm_implode.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\src\gsm_implode.c

# Build GSM_EXPLODE.C
GSM_EXPLODE_C=\
	$(SRCDIR)\inc\private.h\
	$(SRCDIR)\inc\proto.h\
	$(SRCDIR)\inc\unproto.h\
	$(SRCDIR)\inc\gsm.h\
	$(SRCDIR)\inc\proto.h\

gsm_explode.obj: $(GSM_EXPLODE_C) $(SRCDIR)\src\gsm_explode.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\src\gsm_explode.c

# Build GSM_ENCODE.C
GSM_ENCODE_C=\
	$(SRCDIR)\inc\private.h\
	$(SRCDIR)\inc\proto.h\
	$(SRCDIR)\inc\unproto.h\
	$(SRCDIR)\inc\gsm.h\
	$(SRCDIR)\inc\proto.h\

gsm_encode.obj: $(GSM_ENCODE_C) $(SRCDIR)\src\gsm_encode.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\src\gsm_encode.c

# Build GSM_DESTROY.C
GSM_DESTROY_C=\
	$(SRCDIR)\inc\gsm.h\
	$(SRCDIR)\inc\config.h\
	$(SRCDIR)\inc\proto.h\

gsm_destroy.obj: $(GSM_DESTROY_C) $(SRCDIR)\src\gsm_destroy.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\src\gsm_destroy.c

# Build GSM_DECODE.C
GSM_DECODE_C=\
	$(SRCDIR)\inc\private.h\
	$(SRCDIR)\inc\proto.h\
	$(SRCDIR)\inc\unproto.h\
	$(SRCDIR)\inc\gsm.h\
	$(SRCDIR)\inc\proto.h\

gsm_decode.obj: $(GSM_DECODE_C) $(SRCDIR)\src\gsm_decode.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\src\gsm_decode.c

# Build GSM_CREATE.C
GSM_CREATE_C=\
	$(SRCDIR)\inc\config.h\
	$(SRCDIR)\inc\gsm.h\
	$(SRCDIR)\inc\private.h\
	$(SRCDIR)\inc\proto.h\
	$(SRCDIR)\inc\unproto.h\
	$(SRCDIR)\inc\proto.h\

gsm_create.obj: $(GSM_CREATE_C) $(SRCDIR)\src\gsm_create.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\src\gsm_create.c

# Build DECODE.C
DECODE_C=\
	$(SRCDIR)\inc\private.h\
	$(SRCDIR)\inc\proto.h\
	$(SRCDIR)\inc\unproto.h\
	$(SRCDIR)\inc\gsm.h\
	$(SRCDIR)\inc\proto.h\

decode.obj: $(DECODE_C) $(SRCDIR)\src\decode.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\src\decode.c

# Build DEBUG.C
DEBUG_C=\
	$(SRCDIR)\inc\private.h\
	$(SRCDIR)\inc\proto.h\
	$(SRCDIR)\inc\unproto.h\
	$(SRCDIR)\inc\proto.h\

debug.obj: $(DEBUG_C) $(SRCDIR)\src\debug.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\src\debug.c

# Build CODE.C
CODE_C=\
	$(SRCDIR)\inc\config.h\
	$(SRCDIR)\inc\proto.h\
	$(SRCDIR)\inc\private.h\
	$(SRCDIR)\inc\proto.h\
	$(SRCDIR)\inc\unproto.h\
	$(SRCDIR)\inc\gsm.h\
	$(SRCDIR)\inc\proto.h\

code.obj: $(CODE_C) $(SRCDIR)\src\code.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\src\code.c

# Build ADD.C
ADD_C=\
	$(SRCDIR)\inc\private.h\
	$(SRCDIR)\inc\proto.h\
	$(SRCDIR)\inc\unproto.h\
	$(SRCDIR)\inc\gsm.h\
	$(SRCDIR)\inc\proto.h\

add.obj: $(ADD_C) $(SRCDIR)\src\add.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\src\add.c

link:
	c:\programme\lcc\bin\lcclib.exe /OUT:z:\utils\tools\iax\dll\gsm\lcc\libgsm.lib $(OBJS)

