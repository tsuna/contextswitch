# Copyright (C) 2010  Benoit Sigoure
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

CC = gcc
CFLAGS = -pthread -march=native -O3 -mno-avx -D_XOPEN_SOURCE=600 -D_GNU_SOURCE -std=c99 \
         -W -Wall -Werror
LDFLAGS = -lrt -lpthread

TARGETS = timectxsw timectxswws timesyscall timetctxsw timetctxsw2

all: bench

bench: $(TARGETS)
	./cpubench.sh

clean:
	rm -f $(TARGETS)

.PHONY: all bench
