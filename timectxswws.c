// Copyright (C) 2010  Benoit Sigoure
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <linux/futex.h>

static inline long long unsigned time_ns(struct timespec* const ts) {
  if (clock_gettime(CLOCK_REALTIME, ts)) {
    exit(1);
  }
  return ((long long unsigned) ts->tv_sec) * 1000000000LLU
    + (long long unsigned) ts->tv_nsec;
}

static inline int get_iterations(int ws_pages) {
  int iterations = 1000;
  while (iterations * ws_pages * 4096L < 4294967296L) {  // 4GB
    iterations += 1000;
  }
  return iterations;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s <size of working set in 4K pages>\n", *argv);
    return 1;
  }
  const long ws_pages = strtol(argv[1], NULL, 10);
  if (ws_pages < 0) {
    fprintf(stderr, "Invalid usage: working set size must be positive\n");
    return 1;
  }
  const int iterations = get_iterations(ws_pages);
  struct timespec ts;

  long long unsigned memset_time = 0;
  if (ws_pages) {
    void* buf = malloc(ws_pages * 4096);
    memset_time = time_ns(&ts);
    for (int i = 0; i < iterations; i++) {
      memset(buf, i, ws_pages * 4096);
    }
    memset_time = time_ns(&ts) - memset_time;
    printf("%i memset on %4liK in %10lluns (%.1fns/page)\n",
           iterations, ws_pages * 4, memset_time,
           (memset_time / ((float) ws_pages * iterations)));
    free(buf);
  }

  const int shm_id = shmget(IPC_PRIVATE, (ws_pages + 1) * 4096,
                            IPC_CREAT | 0666);
  const pid_t other = fork();
  int* futex = shmat(shm_id, NULL, 0);
  void* ws = ((char *) futex) + 4096;
  *futex = 0xA;
  if (other == 0) {
    for (int i = 0; i < iterations; i++) {
      sched_yield();
      while (syscall(SYS_futex, futex, FUTEX_WAIT, 0xA, NULL, NULL, 42)) {
        // retry
        sched_yield();
      }
      *futex = 0xB;
      if (ws_pages) {
        memset(ws, i, ws_pages * 4096);
      }
      while (!syscall(SYS_futex, futex, FUTEX_WAKE, 1, NULL, NULL, 42)) {
        // retry
        sched_yield();
      }
    }
    return 0;
  }

  const long long unsigned start_ns = time_ns(&ts);
  for (int i = 0; i < iterations; i++) {
    *futex = 0xA;
    if (ws_pages) {
      memset(ws, i, ws_pages * 4096);
    }
    while (!syscall(SYS_futex, futex, FUTEX_WAKE, 1, NULL, NULL, 42)) {
      // retry
      sched_yield();
    }
    sched_yield();
    while (syscall(SYS_futex, futex, FUTEX_WAIT, 0xB, NULL, NULL, 42)) {
      // retry
      sched_yield();
    }
  }
  const long long unsigned delta = time_ns(&ts) - start_ns - memset_time * 2;

  const int nswitches = iterations * 4;
  printf("%i process context switches (wss:%4liK) in %12lluns (%.1fns/ctxsw)\n",
         nswitches, ws_pages * 4, delta, (delta / (float) nswitches));
  wait(futex);
  return 0;
}
