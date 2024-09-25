#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>


#define SHMKEY 12345678 // unique key
#define CLOCK_SIZE sizeof(int)*2 //



int main(int argc, char *argv[]) {

    // Command-line arguments
    int proc = 1;  
    int simul = 2; 
    int timelimit = 3; 
    int interval = 50; 

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

// Fork process
int active_workers = 0;

for (int i = 0; i < proc; i++) {
        if (active_workers >= simul) {
            // Wait for worker before fork
            pid_t pid = waitpid(-1, NULL, 0);
            if (pid > 0) {
                active_workers--;
            }
        }

        // Fork new worker
        pid_t pid = fork();
        if (pid < 0) {
            perror("Fork error");
            exit(1);
        }

        if (pid == 0) {
            // Child process exec the worker
            char max_sec[10], max_nsec[10];
            sprintf(max_sec, "%d", timelimit); 
            sprintf(max_nsec, "500000");       

            execl("./worker", "worker", max_sec, max_nsec, (char *)NULL);
            perror("Exec error");
            exit(1);
        } else {
            // Parent process
            printf("Parent: Forked worker %d with PID %d\n", i + 1, pid);
            active_workers++;
        }

        // Sleep before next process
        usleep(interval * 1000); // Convert milliseconds to microseconds
    }

	// Wait for worker processes to finish
    	while (active_workers > 0) {
        pid_t pid = waitpid(-1, NULL, 0);
        if (pid > 0) {
            active_workers--;
        }
    }

shmdt(clock);
return 0;
}

