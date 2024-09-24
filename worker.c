#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SHMKEY 12345678


int main(int argc, char *argv[]) {


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

// Print the clock values
printf("Seconds: %d\nNanoseconds: %d\n", clock[0], clock[1]);

// Detach shared memory
shmdt(clock);

return 0;
}

