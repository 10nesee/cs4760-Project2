#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/types.h>

#define SHMKEY 12345678 // unique key
#define CLOCK_SIZE sizeof(int)*2 //


int main(int argc, char *arggv[]) {

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

// shmdt(clock);
return 0;
}

