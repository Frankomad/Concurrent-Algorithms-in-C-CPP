/*
 Implementation of a program that uses Lamport's algorithm to synchronize
 N threads incrementing a shared variable A by 1 in a loop executed M times.
 The program takes parameters N and M as command-line arguments to control
 the number of threads and the iteration count. Lamport's algorithm ensures
 correct and safe manipulation of the shared variable A among threads in a
 concurrent environment.
*/

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdatomic.h>

// Global variables
int numberOfThreads, numberOfIterations, a;
atomic_int *entry;
atomic_int *number;

// Function to enter the critical section
void enterTheCriticalSection(atomic_int i)
{
    *(entry + i) = 1;
    int max_number = *(number + 0);

    // Find the maximum number among all threads
    for (int j = 1; j < numberOfThreads; j++)
    {
        if (max_number < *(number + j))
        {
            max_number = *(number + j);
        }
    }

    // Assign a unique number to the current thread
    *(number + i) = max_number + 1;
    *(entry + i) = 0;

    int j;
    for (j = 0; j < numberOfThreads; j++)
    {
        // Wait until other threads exit the entry section
        while (*(entry + j) != 0)
        {
            ;
        }

        // Wait until it's this thread's turn based on Lamport's algorithm
        while ((*(number + j) != 0) && ((*(number + j) < *(number + i)) || (*(number + j) == *(number + i) && j < i)))
        {
            ;
        }
    }
}

// Function to exit the critical section
void exitTheCriticalSection(atomic_int i)
{
    // Mark this thread's number as 0 to exit the critical section
    *(number + i) = 0;
}

// Thread function
void *dretva(void *thr_id)
{
    atomic_int id = *((int *)thr_id);

    for (int i = 0; i < numberOfIterations; i++)
    {
        // Enter the critical section
        enterTheCriticalSection(id);

        // Increment the shared variable A
        ++(a);

        // Exit the critical section
        exitTheCriticalSection(id);
    }
    return NULL;
}

int main()
{
    printf("Enter N -> ");
    scanf("%d", &numberOfThreads);
    printf("Enter M -> ");
    scanf("%d", &numberOfIterations);

    // Dynamically allocate memory for entry and number arrays
    entry = malloc(numberOfThreads * sizeof(int));
    number = malloc(numberOfThreads * sizeof(int));

    // Check for valid parameters
    if (numberOfThreads < 1 || numberOfIterations < 1)
    {
        printf("Wrong parameters");
        return -1;
    }

    // Initialize entry and number arrays to 0
    for (int i = 0; i < numberOfThreads; i++)
    {
        *(entry + i) = *(number + i) = 0;
    }
    a = 0;
    int i;
    pthread_t threads[numberOfThreads];
    int thr_id[numberOfThreads];
    for (i = 0; i < numberOfThreads; i++)
    {
        thr_id[i] = i;
        if (pthread_create(&threads[i], NULL, dretva, &thr_id[i]))
        {
            printf("Error while creating thread!\n");
            return -1;
        }
    }

    // Wait for all threads to finish
    for (i = 0; i < numberOfThreads; i++)
    {
        pthread_join(threads[i], NULL);
    }
    printf("A = %d\n", a);

    // Free dynamically allocated memory
    free(entry);
    free(number);

    return 0;
}
