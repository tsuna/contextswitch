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

#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

static inline long long unsigned time_ns(struct timespec* const ts) {
  if (clock_gettime(CLOCK_REALTIME, ts)) {
    exit(1);
  }
  return ((long long unsigned) ts->tv_sec) * 1000000000LLU
    + (long long unsigned) ts->tv_nsec;
}

int main(void) {
  const int iterations = 10000000;
  struct timespec ts;
  const long long unsigned start_ns = time_ns(&ts);
  for (int i = 0; i < iterations; i++) {
    if (syscall(SYS_gettid) <= 1) {
      exit(2);
    }
  }
  const long long unsigned delta = time_ns(&ts) - start_ns;
  printf("%i system calls in %lluns (%.1fns/syscall)\n",
         iterations, delta, (delta / (float) iterations));
  return 0;
}
