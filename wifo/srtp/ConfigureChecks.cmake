include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckFunctionExists)

# check for <stdint.h> or <machine/types.h>
check_include_files(stdint.h HAVE_STDINT_H)
check_include_files(machine/types.h HAVE_MACHINE_TYPES_H)
check_include_files(sys/inttypes.h HAVE_SYS_INT_TYPES_H)

# if we don't have uio.h, we'll need to define struct iovec
check_include_files(sys/uio.h HAVE_SYS_UIO_H)

# <unistd.h> is used by some test/ apps
check_include_files(unistd.h HAVE_UNISTD_H)

# check for stdlib.h - we use it for xalloc() and free()
check_include_files(stdlib.h HAVE_STDLIB_H)

# test apps should use inet_aton(), if it's available
check_function_exists(inet_aton HAVE_INET_ATON)

# check if we have syslog function
check_include_files(syslog.h HAVE_SYSLOG_H)

# use syslog?
set(USE_SYSLOG 0)
set(ERR_REPORTING_STDOUT 0)
set(ERR_REPORTING_FILE "\"srtp.log\"")

# set ENABLE_DEBUGGING to 1 to compile in dynamic debugging system,
# set it to 0 to not compile in dynamic debugging (for a slight
# performance improvement)

set(ENABLE_DEBUGGING 1)

# if we're going to use GDOI, define SRTP_GDOI to 1
set(SRTP_GDOI 0)

# if /dev/random is available, then DEV_RANDOM == 1
#
# /dev/random is a (true) random number generator which is
# implemented in many modern operating systems
if (EXISTS /dev/random)
  set(DEV_RANDOM 1)
endif (EXISTS /dev/random)

# whether to use ismacryp code
set(GENERIC_AESICM 0)

