/*

This C++ program demonstrates a multi-level interrupt processing system using signal handlers and a stack data structure. 
It handles three levels of interrupts (Level 1, Level 2, and Level 3) and prioritizes them based on their levels. 
The program keeps track of the interrupt levels, processes them, and prints relevant information along with the current stack contents.
*/
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stack>
#include <iostream>

using namespace std;

static int k_z[3] = {0};
static int t_p = 0;
stack<int> stog;

void printStack(stack<int> stog)
{
    if (stog.empty()) {
        cout << "-";
    } else {
        while (!stog.empty()) {
            cout << stog.top() << ", "
                 << "reg[" << stog.top() << "]"
                 << ";"
                 << " ";
            stog.pop();
        }
    }
    cout << endl;
}

/* funkcije za obradu signala, navedene ispod main-a */
void obradi_sigint(int sig); // simulira prekid najvise razine
void obradi_sigusr1(int sig); // simulira prekid srednje razine
void obradi_sigterm(int sig); // simulira prekid najnize razine

struct timespec t0; /* vrijeme pocetka programa */

/* postavlja trenutno vrijeme u t0 */

void postavi_pocetno_vrijeme()
{
    clock_gettime(CLOCK_REALTIME, &t0);
}

/* dohvaca vrijeme proteklo od pokretanja programa */
void vrijeme(void)
{
    struct timespec t;

    clock_gettime(CLOCK_REALTIME, &t);

    t.tv_sec -= t0.tv_sec;
    t.tv_nsec -= t0.tv_nsec;
    if (t.tv_nsec < 0) {
        t.tv_nsec += 1000000000;
        t.tv_sec--;
    }

    printf("%03ld.%03ld:\t", t.tv_sec, t.tv_nsec / 1000000);
}

/* ispis kao i printf uz dodatak trenutnog vremena na pocetku */
#define PRINTF(format, ...)       \
do {                              \
  vrijeme();                      \
  printf(format, ##__VA_ARGS__);  \
}                                 \
while(0)

int main()
{
    struct sigaction act;

    /* 1. maskiranje signala SIGINT*/
    act.sa_handler = obradi_sigint;
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGTERM);
    sigaddset(&act.sa_mask, SIGUSR1);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);

    /* 2. maskiranje signala SIGUSR1*/
    act.sa_handler = obradi_sigusr1;
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGTERM);
    sigaction(SIGUSR1, &act, NULL);

    /* 3. maskiranje signala SIGTERM*/
    act.sa_handler = obradi_sigterm;
    sigaction(SIGTERM, &act, NULL);

    postavi_pocetno_vrijeme();

    PRINTF("Program s PID=%ld krenuo s radom\n", (long)getpid());
    PRINTF("K_Z=000, T_P= 0, stog: -\n");

    cout << endl;

    while (1) {
        sleep(1);
    }

    return 0;
}

void obradi_sigint(int sig)
{
    if (k_z[2] == 1) {
        cout << endl;
        PRINTF("SKLOP: promijenio se T_P, prosljeduje prekid razine 3 procesoru\n");
        stog.push(t_p);
    } else {
        if (!stog.empty()) {
            cout << endl;
        }

        k_z[2] = 1;

        PRINTF("SKLOP: Dogodio se prekid razine 3 i prosljeduje se procesoru\n");
        PRINTF("K_Z = %d%d%d T_P=%d stog: ", k_z[0], k_z[1], k_z[2], t_p);
        printStack(stog);
        stog.push(t_p);
        cout << endl;
    }

    t_p = 3;
    k_z[2] = 0;

    sigset_t pending;

    PRINTF("pocela obrada razine 3\n");
    PRINTF("K_Z = %d%d%d T_P=%d stog:", k_z[0], k_z[1], k_z[2], t_p);
    printStack(stog);
    cout << endl;

    for (int i = 1; i <= 15; i++) {
        PRINTF("iteracija prekida sigint %d/15\n", i);
        sigpending(&pending);

        if (sigismember(&pending, 2) && k_z[2] == 0) {
            cout << endl;
            PRINTF("SKLOP: Dogodio se prekid razine 3 ali se on pamti i ne prosljeđuje procesoru\n");

            k_z[2] = 1;

            PRINTF("K_Z = %d%d%d T_P=%d stog: ", k_z[0], k_z[1], k_z[2], t_p);
            printStack(stog);
            cout << endl;
        }

        if (sigismember(&pending, 10) && k_z[1] == 0) {
            cout << endl;
            PRINTF("SKLOP: Dogodio se prekid razine 2 ali se on pamti i ne prosljeđuje procesoru\n");

            k_z[1] = 1;

            PRINTF("K_Z = %d%d%d T_P=%d stog: ", k_z[0], k_z[1], k_z[2], t_p);
            printStack(stog);
            cout << endl;
        }

        if (sigismember(&pending, 15) && k_z[0] == 0) {
            cout << endl;
            PRINTF("SKLOP: Dogodio se prekid razine 1 ali se on pamti i ne prosljeđuje procesoru\n");

            k_z[0] = 1;

            PRINTF("K_Z = %d%d%d T_P=%d stog: ", k_z[0], k_z[1], k_z[2], t_p);
            printStack(stog);
            cout << endl;
        }
        sleep(1);
    }

    cout << endl;
    PRINTF("Završila obrada prekida razine 3\n");

    t_p = stog.top();
    stog.pop();

    if (t_p == 2) {
        PRINTF("Nastavlja se obrada prekida razine 2\n");
        PRINTF("K_Z = %d%d%d T_P=%d stog: ", k_z[0], k_z[1], k_z[2], t_p);
        printStack(stog);
    }

    if (t_p == 1) {
        PRINTF("Nastavlja se obrada prekida razine 1\n");
        PRINTF("K_Z = %d%d%d T_P=%d stog: ", k_z[0], k_z[1], k_z[2], t_p);
        printStack(stog);
    }

    if (t_p == 0) {
        PRINTF("Nastavlja se izvodenje glavnog programa\n");
        PRINTF("K_Z = %d%d%d T_P=%d stog: -", k_z[0], k_z[1], k_z[2], t_p);
    }

    cout << endl;
}

void obradi_sigusr1(int sig)
{
    if (k_z[1] == 1) {
        cout << endl;
        PRINTF("SKLOP: promijenio se T_P, prosljeduje prekid razine 2 procesoru\n");
        stog.push(t_p);
    } else {
        if (!stog.empty()) {
            cout << endl;
        }

        k_z[1] = 1;

        PRINTF("SKLOP: Dogodio se prekid razine 2 i prosljeduje se procesoru\n");
        PRINTF("K_Z = %d%d%d T_P=%d stog: ", k_z[0], k_z[1], k_z[2], t_p);
        printStack(stog);
        cout << endl;
        stog.push(t_p);
    }

    sigset_t pending;
    k_z[1] = 0;
    t_p = 2;

    PRINTF("pocela obrada razine 2\n");
    PRINTF("K_Z = %d%d%d T_P=%d stog: ", k_z[0], k_z[1], k_z[2], t_p);
    printStack(stog);
    cout << endl;

    for (int i = 1; i <= 15; i++) {
        PRINTF("iteracija prekida sigusr1 %d/15\n", i);
        sleep(1);
        sigpending(&pending);
        if (sigismember(&pending, 10) && k_z[1] == 0) {
            cout << endl;
            PRINTF("SKLOP: Dogodio se prekid razine 2 ali se on pamti i ne prosljeđuje procesoru\n");

            k_z[1] = 1;

            PRINTF("K_Z = %d%d%d T_P=%d stog: ", k_z[0], k_z[1], k_z[2], t_p);
            printStack(stog);
            cout << endl;
        }

        if (sigismember(&pending, 15) && k_z[0] == 0) {
            cout << endl;
            PRINTF("SKLOP: Dogodio se prekid razine 1 ali se on pamti i ne prosljeđuje procesoru\n");

            k_z[0] = 1;

            PRINTF("K_Z = %d%d%d T_P=%d stog: ", k_z[0], k_z[1], k_z[2], t_p);
            printStack(stog);
            cout << endl;
        }
    }

    cout << endl;
    PRINTF("Završila obrada prekida razine 2\n");

    t_p = stog.top();
    stog.pop();

    if (t_p == 1) {
        PRINTF("Nastavlja se obrada prekida razine 1\n");
        PRINTF("K_Z = %d%d%d T_P=%d stog: ", k_z[0], k_z[1], k_z[2], t_p);
        printStack(stog);
    }

    if (t_p == 0) {
        PRINTF("Nastavlja se izvodenje glavnog programa\n");
        PRINTF("K_Z = %d%d%d T_P=%d stog: -", k_z[0], k_z[1], k_z[2], t_p);
    }

    cout << endl;
}

void obradi_sigterm(int sig)
{

    sigset_t pending;

    if (k_z[0] == 1) {
        cout << endl;
        PRINTF("SKLOP: promijenio se T_P, prosljeduje prekid razine 1 procesoru\n");
        stog.push(t_p);
        t_p = 1;
        k_z[0] = 0;
    } else {
        k_z[0] = 1;
        PRINTF("SKLOP: Dogodio se prekid razine 1 i prosljeduje se procesoru\n");
        PRINTF("K_Z = %d%d%d T_P=%d stog: ", k_z[0], k_z[1], k_z[2], t_p);
        printStack(stog);
        stog.push(t_p);
        cout << endl;
        t_p = 1;
        k_z[0] = 0;
    }

    PRINTF("Pocela obrada razine 1\n");
    PRINTF("K_Z = %d%d%d T_P=%d stog: ", k_z[0], k_z[1], k_z[2], t_p);
    printStack(stog);
    cout << endl;

    for (int i = 1; i <= 15; i++) {
        PRINTF("iteracija prekida sigterm %d/15\n", i);
        sigpending(&pending);
        if (sigismember(&pending, 15) && k_z[0] == 0) {
            cout << endl;
            PRINTF("SKLOP: Dogodio se prekid razine 1 ali se on pamti i ne prosljeđuje procesoru\n");

            k_z[0] = 1;

            PRINTF("K_Z = %d%d%d T_P=%d stog: ", k_z[0], k_z[1], k_z[2], t_p);
            printStack(stog);
            cout << endl;
        }

        sleep(1);
    }

    cout << endl;
    PRINTF("Zavrsila obrada prekida razine 1\n");

    t_p = stog.top();
    stog.pop();

    if (t_p == 0) {
        PRINTF("Nastavlja se izvodenje glavnog programa\n");
        PRINTF("K_Z = %d%d%d T_P=%d stog: -\n", k_z[0], k_z[1], k_z[2], t_p);
    }

    cout << endl;
}
