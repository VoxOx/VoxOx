#ifndef CONFIG_H
#define CONFIG_H

#define inline
#define asm(tok) __asm { tok }

/* if we're on a big endian machine, we need to define this */

#ifdef __BIG_ENDIAN__
#define WORDS_BIGENDIAN      1
#else
#define WORDS_BIGENDIAN      0
#endif

/* check for <stdint.h> or <machine/types.h>              */
#cmakedefine HAVE_STDINT_H 1
#cmakedefine HAVE_MACHINE_TYPES_H 1
#cmakedefine HAVE_SYS_INT_TYPES_H 1

/* check for microsoft integer definitions (e.g., cygwin) */
#if defined(__CYGWIN__)
#define HAVE_MS_TYPES 1
#endif

/* if we don't have uio.h, we'll need to define struct iovec */
#cmakedefine HAVE_SYS_UIO_H 1

/* <unistd.h> is used by some test/ apps                  */
#cmakedefine HAVE_UNISTD_H 1

/* check for stdlib.h - we use it for xalloc() and free() */
#cmakedefine HAVE_STDLIB_H 1

/* test apps should use inet_aton(), if it's available */
#cmakedefine HAVE_INET_ATON 1

/* check if we have syslog functions                      */
#cmakedefine HAVE_SYSLOG_H 1

/* check to see if the user has requested the use of syslog */
#cmakedefine USE_SYSLOG 1

#cmakedefine ERR_REPORTING_STDOUT 1

#cmakedefine ERR_REPORTING_SYSLOG (HAVE_SYSLOG_H & USE_SYSLOG)

/* define ERR_REPORTING_FILE to have messages sent to file */
#cmakedefine ERR_REPORTING_FILE ${ERR_REPORTING_FILE}

/*
 * set ENABLE_DEBUGGING to 1 to compile in dynamic debugging system,
 * set it to 0 to not compile in dynamic debugging (for a slight
 * performance improvement)
 */
#cmakedefine ENABLE_DEBUGGING 1

/* if we're going to use GDOI, define SRTP_GDOI to 1      */
#cmakedefine SRTP_GDOI 1

/*
 * CPU_type is defined as 1 if the host processor is of that type.
 * Note that more than one type can be defined at once; this is so
 * that special instructions and other optimizations can be handled
 * independently.
 *
 * CPU_RISC     RISC machines (assume slow byte access)
 * CPU_CISC     CISC machines (e.g. Intel)
 * CPU_ALTIVEC  Motorola's SIMD instruction set
 *
 */
#ifdef __BIG_ENDIAN__ /* PPC processor */
#define CPU_RISC     1
#define CPU_CISC     0
#define CPU_ALTIVEC  1
#else
#define CPU_RISC     0
#define CPU_CISC     1
#define CPU_ALTIVEC  0
#endif

/*
 * if /dev/random is available, then DEV_RANDOM == 1
 *
 * /dev/random is a (true) random number generator which is
 * implemented in many modern operating systems
 */
#cmakedefine DEV_RANDOM 1

/* whether to use ismacryp code */
#cmakedefine GENERIC_AESICM 1

#endif /* CONFIG_H */




