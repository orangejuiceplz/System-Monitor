# System Monitor 🖥️

```
█████████╗   ███████████████████████████╗   ███╗    ███╗   ███╗██████╗███╗   ████████████╗██████╗██████╗ 
██╔════╚██╗ ██╔██╔════╚══██╔══██╔════████╗ ████║    ████╗ ██████╔═══██████╗  ████╚══██╔══██╔═══████╔══██╗
███████╗╚████╔╝███████╗  ██║  █████╗ ██╔████╔█████████╔████╔████║   ████╔██╗ ████║  ██║  ██║   ████████╔╝
╚════██║ ╚██╔╝ ╚════██║  ██║  ██╔══╝ ██║╚██╔╝██╚════██║╚██╔╝████║   ████║╚██╗████║  ██║  ██║   ████╔══██╗
███████║  ██║  ███████║  ██║  █████████║ ╚═╝ ██║    ██║ ╚═╝ ██╚██████╔██║ ╚██████║  ██║  ╚██████╔██║  ██║
╚══════╝  ╚═╝  ╚══════╝  ╚═╝  ╚══════╚═╝     ╚═╝    ╚═╝     ╚═╝╚═════╝╚═╝  ╚═══╚═╝  ╚═╝   ╚═════╝╚═╝  ╚═╝
                                                                                                         
```
## introduction

welcome to system-monitor, a worse version of btop (not forked) written in C++

## key features as of now

- **RTM - Real Time Monitoring**: keep track of CPU usage, memory consumption, disk space, and more.
- **Simple Interface**: easy-to-read output, perfect for quick checks.
- **Customizable Alerts**: set thresholds for alerts to stay informed about potential issues.

## getting Started

### prerequisites

make sure you have the following installed on your system

- ncurses
- g++ (over gcc)
- CMake

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

now you can use the worse version of btop, for whatever reason

## contributions

i guess you can contribute? a bug fix, new feature, or just a suggestion, i can look at it and potentially add it. just make an issue and refer to the (not present contributions.md)

## licensed

System Monitor is licensed under the MIT License. See the  [LICENSE](https://github.com/orangejuiceplz/System-Monitor/blob/main/LICENSE) for more information.

## if used in another project

this is never going to happen, but credit me please! 
