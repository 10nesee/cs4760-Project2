#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/types.h>

#define SHMKEY 12345678 // unique key
#define CLOCK_SIZE sizeof(int)*2 //



int main(int argc, char *argv[]) {
    // Command-line arguments and their defaults
    int proc = 1;  // Default number of processes
    int simul = 2; // Default number of simultaneous workers
    int timelimit = 3; // Default time limit for children
    int interval = 50;  // Default interval in milliseconds

    // Parse arguments using getopt
    int opt;
    while ((opt = getopt(argc, argv, "hn:s:t:i:")) != -1) {
        switch (opt) {
            case 'h':
                printf("Usage: %s [-h] [-n proc] [-s simul] [-t timelimit] [-i interval]\n", argv[0]);
                exit(0);
            case 'n':
                proc = atoi(optarg);
                break;
            case 's':
                simul = atoi(optarg);
                break;
            case 't':
                timelimit = atoi(optarg);
                break;
            case 'i':
                interval = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s [-h] [-n proc] [-s simul] [-t timelimit] [-i interval]\n", argv[0]);
                exit(1);
        }
    }

    // Print the parsed values 
    printf("Number of processes: %d\n", proc);
    printf("Number of simultaneous workers: %d\n", simul);
    printf("Time limit for children: %d\n", timelimit);
    printf("Interval to launch children (ms): %d\n", interval);




// Create shared memory
int shmid = shmget(SHMKEY, 2 * sizeof(int), 0777 | IPC_CREAT);
if (shmid == -1) {
perror("Shared memory error");
exit(1);
}

// Attach shared memory
int *clock = (int *)shmat(shmid, NULL, 0);
if (clock == (int *) -1) {
perror("Attach shared memory error");
exit(1);
}

// Clock for second and nanosecond
clock[0] = 0;
clock[1] = 0;
printf("second: %d nanosecond: %d\n", clock[0], clock[1]);

shmdt(clock);
return 0;
}

