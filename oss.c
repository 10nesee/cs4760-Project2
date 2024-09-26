#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#define SHMKEY 12345678 // unique key
#define CLOCK_SIZE sizeof(int)*2
#define NANO_INCREMENT 1000000 // 1 ms in nanoseconds
#define MAX_PROCESSES 20       // Max number of worker processes

// PCB structure to store data
struct PCB {
    int occupied;          // Whether this entry is occupied
    pid_t pid;             // PID of the worker process
    int startSeconds;      // Start time (seconds)
    int startNano;         // Start time (nanoseconds)
};

// Process table to store PCBs
struct PCB processTable[MAX_PROCESSES];

// Shared memory
int *clock = NULL;
int shmid;

// Signal handler to shut down all worker processes
void signal_handler(int sig) {
    if (sig == SIGALRM || sig == SIGINT) {
        printf("\nSignal received, shutting down all processes...\n");

        // Send kill signal to all children based on their PIDs in the process table
        for (int i = 0; i < MAX_PROCESSES; i++) {
            if (processTable[i].occupied) {
                kill(processTable[i].pid, SIGTERM);  // Send termination signal
                processTable[i].occupied = 0;
                printf("Terminated worker PID: %d\n", processTable[i].pid);
            }
        }

        // Free up shared memory
        if (clock != NULL) {
            shmdt(clock);  // Detach shared memory
        }

        shmctl(shmid, IPC_RMID, NULL);  // Mark shared memory for destruction
        printf("Shared memory cleaned up.\n");

        exit(1);
    }
}

// Function to print the process table
void printProcessTable() {
    printf("\nSimulated System Clock: %d seconds, %d nanoseconds\n", clock[0], clock[1]);
    printf("Process Table:\n");
    printf("Entry  Occupied  PID     StartSeconds  StartNano\n");
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processTable[i].occupied) {
            printf("%5d %9d %6d %13d %10d\n", i, processTable[i].occupied, processTable[i].pid, processTable[i].startSeconds, processTable[i].startNano);
        }
    }
    printf("\n");
}

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
    shmid = shmget(SHMKEY, 2 * sizeof(int), 0777 | IPC_CREAT);
    if (shmid == -1) {
        perror("Shared memory error");
        exit(1);
    }

    // Attach shared memory
    clock = (int *)shmat(shmid, NULL, 0);
    if (clock == (int *) -1) {
        perror("Attach shared memory error");
        exit(1);
    }

    // Initialize the clock
    clock[0] = 0;  // seconds
    clock[1] = 0;  // nanoseconds

    // Initialize the Process table
    for (int i = 0; i < MAX_PROCESSES; i++) {
        processTable[i].occupied = 0; // Mark all entries as empty
    }

    // Fork process
    int active_workers = 0;
    int launched_children = 0;
    int half_sec_counter = 0;

    // Set up signal handling for SIGALRM and SIGINT
    signal(SIGALRM, signal_handler);  
    signal(SIGINT, signal_handler);   

    // Set alarm to terminate after 60 seconds
    alarm(60);

    // Loop for children and clock control
    while (launched_children < proc || active_workers > 0) {

        // Clock increment
        clock[1] += NANO_INCREMENT;
        if (clock[1] >= 1000000000) {
            // Handle overflow
            clock[0]++;
            clock[1] -= 1000000000;
        }

        // Print the process table every half second
        if (half_sec_counter >= 500000000) {
            printProcessTable();
            half_sec_counter = 0;
        } else {
            half_sec_counter += NANO_INCREMENT;
            // Track half-second increments
        }

        // Check if workers have finished
        int status;
        pid_t pid = waitpid(-1, &status, WNOHANG);
        if (pid > 0) {

            // Find the terminated worker and free its entry
            for (int j = 0; j < MAX_PROCESSES; j++) {
                if (processTable[j].pid == pid) {
                    processTable[j].occupied = 0;  // Free the entry
                    printf("Parent: Worker with PID %d terminated, freeing process table entry %d.\n", pid, j);
                    active_workers--;
                    break;
                }
            }
        }

        // Launch a new worker if there is a slot
        if (active_workers < simul && launched_children < proc) {
            // Find an empty slot in the process table
            int pcbIndex = -1;
            for (int j = 0; j < MAX_PROCESSES; j++) {
                if (processTable[j].occupied == 0) {
                    pcbIndex = j;
                    break;
                }
            }

            if (pcbIndex == -1) {
                printf("No available slots in the process table. Cannot fork new worker.\n");
                break;
            }

            // Fork new worker
            pid_t pid = fork();
            if (pid < 0) {
                perror("Fork error");
                exit(1);
            }

            if (pid == 0) {

                // Child process exec the worker program with arguments
                char max_sec[10], max_nsec[10];
                sprintf(max_sec, "%d", timelimit); // Maximum seconds to run
                sprintf(max_nsec, "500000");       // Arbitrary nanoseconds for now

                execl("./worker", "worker", max_sec, max_nsec, (char *)NULL);
                perror("Exec error");
                exit(1);
            } else {
                // In parent process: update the process table with the new worker's info
                processTable[pcbIndex].occupied = 1;
                processTable[pcbIndex].pid = pid;
                processTable[pcbIndex].startSeconds = clock[0];
                processTable[pcbIndex].startNano = clock[1];
                printf("Parent: Forked worker %d with PID %d in process table entry %d.\n", launched_children + 1, pid, pcbIndex);
                active_workers++;
                launched_children++;
            }

            // Sleep before launching the next process
            usleep(interval * 1000); // Milliseconds to microseconds
        }

        usleep(1000);  // 1 millisecond
    }

    shmdt(clock);
    return 0;
}

