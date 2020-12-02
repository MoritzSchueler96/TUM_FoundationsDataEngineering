# Repo accompanying the lecture Foundations in Data Engineering at TUM

fork projects and then can use it normally
add projects as submodules to this github project

## perf

### installation

apt install linux-tools-\$(uname -r) linux-tools-generic

### to enable perf

sudo nano /etc/sysctl.conf -> add kernel.perf_event_paranoid = -1
sudo nano /proc/sys/kernel/perf_event_paranoid -> default 3, set to -1 for perf
sudo nano /proc/sys/kernel/kptr_restrict -> default 1, set to 0 for perf

get content of tips.txt file: https://github.com/torvalds/linux/blob/master/tools/perf/Documentation/tips.txt
sudo mkdir /usr/share/doc/perf-tip/
sudo touch /usr/share/doc/perf-tip/tips.txt
sudo nano /usr/share/doc/perf-tip/tips.txt

### usage

branch misses are important for performance!
perf stat <program> -> to get performance measures
perf record <program> -> to record normally
perf record -e cycles:pp <program> -> to record with high precision
pref report -> show performance -> Enter -> Annotate main -> get assembler code mixed with c code to see where most time is spent

## CPP

### compile

g++ -O0 -g <program.cpp> -> unoptimized, good for debugging
g++ -O3 -g <program.cpp> -> optimized
g++ -O3 -g -march=native <programm.cpp> -> compile for local machine, if time is spent outside of main for no reason
g++ -O3 -g -march=native -pthread <programm.cpp> -> enable threads

TODO: setup cmake

folder names
cmake-build-debug
cmake-build-release
