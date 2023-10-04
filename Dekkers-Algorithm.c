/*
This C program implements Dekker's algorithm, a classic mutual exclusion algorithm that ensures only one process enters a critical section at a time. 
It uses shared memory and forked processes to demonstrate this concept.
Processes take turns entering the critical section based on a set of flags and variables. 
After execution, the program prints the final value of a shared integer variable 'a' and cleans up the shared memory segment.
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

int id;            /* identifikacijski broj segmenta */
int *a;            // a[1] - shared integer variable
int *rightOfEntry; // rightOfEntry[1] - shared integer variable
int *flag;         // flag[2] - shared integer array with two elements

// Function to enter the critical section for a process with index 'i'
void enterTheCriticalSection(int i)
{
    int j = 1 - i;
    *(flag + i) = 1;

    // Wait until the other process is not in its critical section
    while (*(flag + j) != 0)
    {
        if (*rightOfEntry == j)
        {
            *(flag + i) == 0;
            while (*rightOfEntry == j)
            {
                // Wait until it's your turn
            }
            *(flag + i) = 1;
        }
    }
}

// Function to exit the critical section for a process with index 'i'
void exitTheCriticalSection(int i)
{
    *(flag + i) = 0;
    *rightOfEntry = 1 - i;
}

// Function to simulate a process
void proces(int id, int m)
{
    for (int j = 0; j < m; j++)
    {
        enterTheCriticalSection(id);
        ++(*a); // Increment the shared integer 'a' within the critical section
        exitTheCriticalSection(id);
    }
}

int main()
{
    int i;
    int m;
    printf("Upisi M -> ");
    scanf("%d", &m);

    // Create a shared memory segment
    id = shmget(IPC_PRIVATE, sizeof(int) * (1 + 1 + 2), 0600);

    if (id == -1)
        exit(1); // (error - no shared memory)

    // Attach shared memory to variables
    a = (int *)shmat(id, NULL, 0);
    rightOfEntry = (int *)(a + 1);
    flag = (int *)(rightOfEntry + 1);

    // Initialize shared variables
    *a = 0;
    *rightOfEntry = 0;
    *flag = 0;
    *(flag + 1) = 0;

    for (i = 0; i < 2; i++)
    {
        if (fork() == 0)
        {
            // Child process executes the 'proces' function
            proces(i, m);
            exit(0);
        }
    }

    for (i = 0; i < 2; i++)
    {
        (void)wait(NULL);
    }
    printf("a= %d\n", *a);

    (void)wait(NULL);

    // Detach and remove the shared memory segment
    (void)shmdt((char *)a);
    (void)shmctl(id, IPC_RMID, NULL);
    return 0;
}
