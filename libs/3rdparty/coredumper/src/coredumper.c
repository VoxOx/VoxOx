/* Copyright (c) 2005, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ---
 * Author: Markus Gutschke
 *
 * Code to extract a core dump snapshot of the current process.
 */
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "google/coredumper.h"
#include "elfcore.h"
#include "thread_lister.h"

#ifndef DUMPER
/* If the target platform lacks the necessary support for generating core dumps
 * on the fly, or if nobody has ported the code, then return an error.
 */
typedef void *Frame;
#define FRAME(f) void *f = &&label; label:


int InternalGetCoreDump(void *frame, int num_threads, pid_t *thread_pids) {
  errno = EINVAL;
  return -1;
}
#endif

/* Returns a file handle that can be read to obtain a snapshot of the
 * current state of this process. If a core file could not be
 * generated for any reason, -1 is returned.
 *
 * This function momentarily suspends all threads, while creating a
 * COW copy of the process's address space.
 *
 * This function is neither reentrant nor async signal safe. Callers
 * should wrap a mutex around the invocation of this function, if necessary.
 *
 * The current implementation tries very hard to do behave reasonably when
 * called from a signal handler, but no guarantees are made that this will
 * always work.
 */
int GetCoreDump() {
  FRAME(frame);
  return GetAllProcessThreads(&frame, InternalGetCoreDump);
}


/* Re-runs fn until it doesn't cause EINTR.
 */
#define NO_INTR(fn)   do {} while ((fn) < 0 && errno == EINTR)


/* Internal helper method used by WriteCoreDump().
 */
static int WriteCoreDumpFunction(void *frame, const char *file_name,
                                 size_t max_length) {
  int rc = 0;
  int coreFd;

  if (!max_length)
    return 0;
  coreFd = GetAllProcessThreads(frame, InternalGetCoreDump);
  if (coreFd >= 0) {
    int writeFd;
    NO_INTR(writeFd = open(file_name, O_WRONLY|O_CREAT|O_TRUNC, 0666));
    if (writeFd >= 0) {
      char buffer[16384];
      ssize_t len;
      while (max_length > 0 &&
             ((len = read(coreFd, buffer,
                          sizeof(buffer) < max_length
                          ? sizeof(buffer) : max_length)) > 0 ||
              (len < 0 && errno == EINTR))) {
        char *ptr = buffer;
        while (len > 0) {
          int i;
          NO_INTR(i = write(writeFd, ptr, len));
          if (i <= 0) {
            rc = -1;
            break;
          }
          ptr        += i;
          len        -= i;
          max_length -= i;
        }
      }
      NO_INTR(close(writeFd));
    } else {
      rc = -1;
    }
    NO_INTR(close(coreFd));
  } else {
    rc = -1;
  }
  return rc;
}

/* Writes the core file to disk. This is a convenience method wrapping
 * GetCoreDump(). If a core file could not be generated for any reason,
 * -1 is returned. On success, zero is returned.
 */
int WriteCoreDump(const char *file_name) {
  FRAME(frame);
  return WriteCoreDumpFunction(&frame, file_name, SIZE_MAX);
}

/* Callers might need to restrict the maximum size of the core file. This
 * convenience method provides the necessary support to emulate "ulimit -c".
 */
int WriteCoreDumpLimited(const char *file_name, size_t max_length) {
  FRAME(frame);
  return WriteCoreDumpFunction(&frame, file_name, max_length);
}
