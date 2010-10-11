CC = gcc
CFLAGS = -mtune=native -O3 -D_XOPEN_SOURCE=600 -D_GNU_SOURCE -std=c99 \
         -W -Wall -Werror
LDFLAGS = -lrt

all: bench

bench: timectxsw timectxswws timesyscall timetctxsw
	./cpubench.sh

.PHONY: all bench
