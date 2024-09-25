#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SHMKEY 12345678


int main(int argc, char *argv[]) {

// Check for two arguments
if (argc != 3) {
        fprintf(stderr, "Usage: %s <max_seconds> <max_nanoseconds>\n", argv[0]);
        exit(1);
    }

// convert strings to intergers 
int max_seconds = atoi(argv[1]);
int max_nanoseconds = atoi(argv[2]);

// Access shared memory
int shmid = shmget(SHMKEY, sizeof(int) * 2, 0777);
    if (shmid == -1) {
        perror("Shared memory error");
        exit(1);
    }	
// Attach shared memory
int *clock = (int *)shmat(shmid, NULL, 0);
    if (clock == (void *) -1) {
        perror("Attach memory error");
        exit(1);
    }

// Calculate the termination time
int start_seconds = clock[0];  
int start_nanoseconds = clock[1];
int term_seconds = start_seconds + max_seconds;
int term_nanoseconds = start_nanoseconds + max_nanoseconds;


// Handle overflow in nanoseconds
if (term_nanoseconds >= 1000000000) {
    term_seconds++;
    term_nanoseconds -= 1000000000;
}

// Print status
printf("WORKER PID: %d PPID: %d SysClockS: %d SysClockNano: %d TermTimeS: %d TermTimeNano: %d --Just Starting\n",
           getpid(), getppid(), start_seconds, start_nanoseconds, term_seconds, term_nanoseconds);

// While loop
while (1) {
    // Check if the worker's time is up
    if (clock[0] > term_seconds || (clock[0] == term_seconds && clock[1] >= term_nanoseconds)) {
        printf("WORKER PID: %d SysClockS: %d SysClockNano: %d TermTimeS: %d TermTimeNano: %d --Terminating\n", getpid(), clock[0], clock[1], term_seconds, term_nanoseconds);
        break;

    // Periodically print status if a second has passed
        if (clock[0] > start_seconds) {
            printf("WORKER PID: %d SysClockS: %d SysClockNano: %d TermTimeS: %d TermTimeNano: %d --%d seconds have passed since starting\n",
                   getpid(), clock[0], clock[1], term_seconds, term_nanoseconds, clock[0] - start_seconds);
            start_seconds = clock[0]; // Update to avoid printing the same second again
        }
}
        // Avoid tight looping
        usleep(1000);  // Sleep for 1 millisecond

}



// Detach shared memory
shmdt(clock);

return 0;
}

