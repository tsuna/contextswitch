#!/bin/bash
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

fgrep 'model name' /proc/cpuinfo | sort -u | xargs
pgrep vmware >/dev/null && echo Running under VMware
ncpus=`sort -u /sys/devices/system/cpu/cpu*/topology/physical_package_id | wc -l`
corepercpu=`sort -u /sys/devices/system/cpu/cpu*/topology/core_id | wc -l`
threadpercore=`sed \
"s/2/10/g;\
s/3/11/g;\
s/4/100/g;\
s/5/101/g;\
s/6/110/g;\
s/7/111/g;\
s/8/1000/g;\
s/9/1001/g;\
s/a/1010/g;\
s/b/1011/g;\
s/c/1100/g;\
s/d/1101/g;\
s/e/1110/g;\
s/f/1111/g;\
s/[^1]//g" /sys/devices/system/cpu/cpu*/topology/thread_siblings \
  | while read nthreads; do echo ${#nthreads}; done | sort -u`
total=$((ncpus * corepercpu * threadpercore))
echo "$ncpus physical CPUs, $corepercpu cores/CPU,\
 $threadpercore hardware threads/core = $total hw threads total"

runbench() {
  $* ./timesyscall
  $* ./timectxsw
  $* ./timetctxsw
  $* ./timetctxsw2
}

echo '-- No CPU affinity --'
runbench

echo '-- With CPU affinity --'
lastcpu=`awk </dev/null -v ncpus=$ncpus 'BEGIN {
  n = 1;
  for(i = 1; i < ncpus; i++) n *= 2;
  printf("0x%x\n", n);
}'`
runbench taskset $lastcpu

echo '-- With CPU affinity to CPU 0 --'
runbench taskset `sed 's/,//g;s/^/0x/' /sys/bus/node/devices/node0/cpumap`
