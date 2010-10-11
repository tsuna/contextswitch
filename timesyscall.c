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
