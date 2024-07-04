> [!WARNING]
> there are active issues, for example if you input like anything cpu numbers rise drastically and is VERY not representative (e.g 500%, what??) i'll fix this within the next 100 commits or something.
> also, utilization and memory usage is not exactly working well for nvidia. resource usage overall is very weird anyway.
> i will be transforming this into an actual application soon

# System-Monitor 🖥️

```
█████████╗   ███████████████████████████╗   ███╗    ███╗   ███╗██████╗███╗   ████████████╗██████╗██████╗ 
██╔════╚██╗ ██╔██╔════╚══██╔══██╔════████╗ ████║    ████╗ ██████╔═══██████╗  ████╚══██╔══██╔═══████╔══██╗
███████╗╚████╔╝███████╗  ██║  █████╗ ██╔████╔█████████╔████╔████║   ████╔██╗ ████║  ██║  ██║   ████████╔╝
╚════██║ ╚██╔╝ ╚════██║  ██║  ██╔══╝ ██║╚██╔╝██╚════██║╚██╔╝████║   ████║╚██╗████║  ██║  ██║   ████╔══██╗
███████║  ██║  ███████║  ██║  █████████║ ╚═╝ ██║    ██║ ╚═╝ ██╚██████╔██║ ╚██████║  ██║  ╚██████╔██║  ██║
╚══════╝  ╚═╝  ╚══════╝  ╚═╝  ╚══════╚═╝     ╚═╝    ╚═╝     ╚═╝╚═════╝╚═╝  ╚═══╚═╝  ╚═╝   ╚═════╝╚═╝  ╚═╝
                                                                                                         
```
## introduction

welcome to system-monitor, a worse version of top and btop (not forked) written in C++

## key features as of now

- **RTM - Real Time Monitoring**: keep track of CPU usage, memory consumption, disk space, and more.
- **Simple Interface**: easy-to-read output straight from the console, perfect for quick checks.
- **Customizable Alerts**: set thresholds for alerts to stay informed about potential issues.

## getting Started

### prerequisites

make sure you have the following installed on your system

- ncurses
- g++ (over gcc)
- CMake
- nvml and cuda (for nvidia gpu's)
- dl
- procps

### installation 

install the requirements, then clone the repo. from there, build from source

bash 
```
git clone https://github.com/orangejuiceplz/System-Monitor.git
cd System-Monitor
mkdir build
cd build
cmake ..
make
```
### execute

after building from source, simply run:

bash
```
./system_monitor
```

now you can use the worse version of top and btop, for whatever reason

## contributions

i guess you can contribute? a bug fix, new feature, or just suggestions, i can look at it and potentially add it. please refer to [CONTRIBUTIONS.md](https://github.com/orangejuiceplz/System-Monitor/blob/main/CONTRIBUTIONS.md)

## licensed

System Monitor is licensed under the MIT License. See the  [LICENSE](https://github.com/orangejuiceplz/System-Monitor/blob/main/LICENSE) for more information.

## if used in another project

this is never going to happen, but credit me please! 
