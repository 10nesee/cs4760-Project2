Project2

Project Overview
This project models a process control system in which several worker processes (worker) are forked by a parent process (oss). Using shared memory, the parent keeps track of running processes by managing process control blocks (PCBs) and maintaining a simulated system clock. Using signal handling, the system automatically shuts down after 60 seconds.

How to compile
1. Go into terminal
2. Run command: make

How to run
1. Run command: ./oss -n 5 -s 3 -t 7 -i 200

Git Repository
The project can be accessed at: https://github.com/10nesee/cs4760-Project2.git
