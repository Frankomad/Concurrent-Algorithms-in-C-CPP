/*
 Problem of cannibals and missionaries
 In this simulation:

There is a river with a boat that can carry passengers between two banks.
The boat can hold up to 7 passengers and needs at least 3 passengers to start its journey.
The boat must adhere to the rule of never having more cannibals than missionaries on board.
Passengers, represented as missionaries and cannibals, arrive randomly on both sides of the river.
One missionary arrives every two seconds, and one cannibal arrives every second, with their arrival side being random.
After passengers cross the river, they continue their journey and are removed from the simulation.
The simulation uses threads or processes to represent each missionary, cannibal, and the boat.
The boat, as a thread or process, prints the names of the passengers it carries when it makes a journey.
Initially, the boat starts on the right bank, waits for one second to potentially accommodate more passengers,
 and then takes two seconds to cross the river.
The primary goal of the code is to correctly synchronize the threads or processes representing missionaries, cannibals,
 and the boat to ensure that they adhere to the specified rules and constraints while simulating the crossing of the river by passengers.
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
#include <pthread.h>
#include <list>
#include <ctime>
#include <cstdlib>
#include <string>

using namespace std;

pthread_mutex_t m;           // Mutex for thread synchronization
pthread_cond_t cam;          // Condition variable for boat
pthread_cond_t kan;          // Condition variable for cannibals
pthread_cond_t mis;          // Condition variable for missionaries
int brojOsobaUCamcu = 0;     // Number of people on the boat
int obalaCamca = 1;          // Current location of the boat: 0 (left) or 1 (right), -1 (traveling)
int brojKanibalaUCamcu = 0;  // Number of cannibals on the boat
int brojMisionaraUCamcu = 0; // Number of missionaries on the boat
list<string> lijevaObala;    // List of people on the left bank
list<string> desnaObala;     // List of people on the right bank
list<string> osobeUCamcu;    // List of people on the boat
list<string> lijevaObalaM;   // List of missionaries on the left bank
list<string> lijevaObalaK;   // List of cannibals on the left bank
list<string> desnaObalaM;    // List of missionaries on the right bank
list<string> desnaObalaK;    // List of cannibals on the right bank

// Function to print a list of strings
void print_list(const std::list<std::string> &my_list)
{
    for (auto it = my_list.begin(); it != my_list.end(); ++it)
    {
        std::cout << *it;
        if (std::next(it) != my_list.end())
        {
            std::cout << " ";
        }
    }
}

// Missionary thread function
void *misionar(void *id)
{
    // Lock the mutex for synchronization
    pthread_mutex_lock(&m);

    // Randomly determine the initial side of the missionary
    srand(time(nullptr));
    int obalaMisionara = rand() % 2; // 0 -> left bank || 1 -> right bank

    int m_id = *((int *)id);
    string misionar = "M" + to_string(m_id);

    // Place the missionary on the appropriate bank
    if (obalaMisionara == 1)
    {
        desnaObala.push_back(misionar);
        desnaObalaM.push_back(misionar);
    }
    else
    {
        lijevaObala.push_back(misionar);
        lijevaObalaM.push_back(misionar);
    }

    // Print the arrival message
    cout << misionar << ": dosao na " << (obalaMisionara == 1 ? "desnu obalu" : "lijevu obalu") << endl;
    cout << "C[" << (obalaCamca == 1 ? "D" : "L") << "]={";
    print_list(osobeUCamcu);
    cout << "} LO={";
    print_list(lijevaObala);
    cout << "} DO={";
    print_list(desnaObala);
    cout << "}" << endl
         << endl;

    // Wait until it's possible to board the boat
    while (brojOsobaUCamcu == 7 || obalaCamca != obalaMisionara || brojKanibalaUCamcu >= brojMisionaraUCamcu + 2)
    {
        pthread_cond_wait(&mis, &m);
    }

    // Increment counters and signal other threads
    brojMisionaraUCamcu++;
    brojOsobaUCamcu++;
    pthread_cond_broadcast(&mis);

    // Get the missionary to board the boat
    string ulazi;
    if (obalaCamca == 1)
    {
        ulazi = desnaObalaM.front();
        desnaObala.remove(ulazi);
        desnaObalaM.pop_front();
        osobeUCamcu.push_back(ulazi);
    }
    else if (obalaCamca == 0)
    {
        ulazi = lijevaObalaM.front();
        lijevaObala.remove(ulazi);
        lijevaObalaM.pop_front();
        osobeUCamcu.push_back(ulazi);
    }

    // Print the boarding message
    cout << ulazi << ": ušao u čamac" << endl;
    cout << "C[" << (obalaCamca == 1 ? "D" : "L") << "]={";
    print_list(osobeUCamcu);
    cout << "} LO={";
    print_list(lijevaObala);
    cout << "} DO={";
    print_list(desnaObala);
    cout << "}" << endl
         << endl;

    // Signal the boat if three passengers are on board
    if (brojOsobaUCamcu >= 3)
    {
        pthread_cond_signal(&cam);
    }

    // Unlock the mutex
    pthread_mutex_unlock(&m);

    return NULL;
}

// Cannibal thread function (similar to missionary)
void *kanibal(void *id)
{
    pthread_mutex_lock(&m);
    srand(time(nullptr));
    int obalaKanibala = rand() % 2; // 0 -> left bank || 1 -> right bank
    int k_id = *((int *)id);
    string kanibal = "K" + to_string(k_id);

    // Place the cannibal on the appropriate bank
    if (obalaKanibala == 1)
    {
        desnaObala.push_back(kanibal);
        desnaObalaK.push_back(kanibal);
    }
    else
    {
        lijevaObala.push_back(kanibal);
        lijevaObalaK.push_back(kanibal);
    }

    // Print the arrival message
    cout << kanibal << ": dosao na " << (obalaKanibala == 1 ? "desnu obalu" : "lijevu obalu") << endl;
    cout << "C[" << (obalaCamca == 1 ? "D" : "L") << "]={";
    print_list(osobeUCamcu);
    cout << "} LO={";
    print_list(lijevaObala);
    cout << "} DO={";
    print_list(desnaObala);
    cout << "}" << endl
         << endl;

    while (brojOsobaUCamcu == 7 || obalaCamca != obalaKanibala || (brojKanibalaUCamcu == brojMisionaraUCamcu && brojMisionaraUCamcu > 0))
    {
        pthread_cond_wait(&kan, &m);
    }

    // Increment counters and signal other threads
    brojKanibalaUCamcu++;
    brojOsobaUCamcu++;
    pthread_cond_broadcast(&kan);

    // Get the cannibals to board the boat
    string ulazi;
    if (obalaCamca == 1)
    {
        ulazi = desnaObalaK.front();
        desnaObala.remove(ulazi);
        desnaObalaK.pop_front();
        osobeUCamcu.push_back(ulazi);
    }
    else if (obalaCamca == 0)
    {
        ulazi = lijevaObalaK.front();
        lijevaObala.remove(ulazi);
        lijevaObalaK.pop_front();
        osobeUCamcu.push_back(ulazi);
    }

    // Print the boarding message
    cout << ulazi << ": ušao u čamac" << endl;
    cout << "C[" << (obalaCamca == 1 ? "D" : "L") << "]={";
    print_list(osobeUCamcu);
    cout << "} LO={";
    print_list(lijevaObala);
    cout << "} DO={";
    print_list(desnaObala);
    cout << "}" << endl
         << endl;

    // Signal the boat if three passengers are on board
    if (brojOsobaUCamcu >= 3)
    {
        pthread_cond_signal(&cam);
    }

    // Unlock the mutex
    pthread_mutex_unlock(&m);
    return NULL;
}

void *camac(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&m); // Lock the mutex for synchronization

        int zadnjaObala = obalaCamca; // Store the current boat location

        // Check if the boat is empty
        if (brojOsobaUCamcu == 0)
        {
            cout << "C: prazan na " << (obalaCamca == 1 ? "desnoj obali" : "lijevoj obali") << endl;
        }

        // Print the state of the simulation
        cout << "C[" << (obalaCamca == 1 ? "D" : "L") << "]={";
        print_list(osobeUCamcu);
        cout << "} LO={";
        print_list(lijevaObala);
        cout << "} DO={";
        print_list(desnaObala);
        cout << "}" << endl
             << endl;

        // Wait until there are at least 3 people to board the boat
        while (brojOsobaUCamcu < 3)
        {
            pthread_cond_wait(&cam, &m);
        }

        cout << "C: tri putnika ukrcana, polazim za jednu sekundu" << endl;

        // Print the state of the simulation before departure
        cout << "C[" << (obalaCamca == 1 ? "D" : "L") << "]={";
        print_list(osobeUCamcu);
        cout << "} LO={";
        print_list(lijevaObala);
        cout << "} DO={";
        print_list(desnaObala);
        cout << "}" << endl
             << endl;

        pthread_mutex_unlock(&m); // Unlock the mutex to allow others to board

        sleep(1); // Wait for one second before departure

        pthread_mutex_lock(&m); // Lock the mutex again for synchronization

        // Print the boat's departure message
        cout << "C: prevozim s " << (obalaCamca == 1 ? "desne" : "lijeve") << " na " << (obalaCamca == 1 ? "lijevu " : "desnu ") << "obalu: ";
        print_list(osobeUCamcu);
        cout << endl
             << endl;

        obalaCamca = -1; // Mark the boat as in transit

        pthread_mutex_unlock(&m); // Unlock the mutex to simulate the boat journey

        sleep(2); // Wait for two seconds to simulate the boat crossing

        pthread_mutex_lock(&m); // Lock the mutex again for synchronization

        // Print the boat's arrival message
        cout << "C: preveo s " << (zadnjaObala == 1 ? "desne" : "lijeve") << " na " << (zadnjaObala == 1 ? "lijevu " : "desnu ") << "obalu ";
        print_list(osobeUCamcu);
        cout << endl
             << endl;

        osobeUCamcu.clear(); // Clear the list of people on the boat

        obalaCamca = 1 - zadnjaObala; // Update the boat's location
        brojOsobaUCamcu = 0;          // Reset the passenger count on the boat
        brojKanibalaUCamcu = 0;       // Reset the count of cannibals on the boat
        brojMisionaraUCamcu = 0;      // Reset the count of missionaries on the boat

        // Signal waiting cannibals and missionaries that the boat has arrived
        pthread_cond_broadcast(&kan);
        pthread_cond_broadcast(&mis);

        pthread_mutex_unlock(&m); // Unlock the mutex to prepare for the next trip
    }

    return NULL;
}

void *kreiranje_m_i_k(void *arg)
{
    int kanibal_id = 0;  // Initialize kanibal ID counter
    int misionar_id = 0; // Initialize misionar ID counter

    pthread_t misionari; // Thread for misionari
    pthread_t kanibali;  // Thread for kanibali

    // Create 5 pairs of misionari and kanibali threads
    for (int i = 0; i < 5; i++)
    {
        pthread_create(&misionari, NULL, misionar, &misionar_id); // Create a misionar thread
        misionar_id++;

        pthread_create(&kanibali, NULL, kanibal, &kanibal_id); // Create a kanibal thread
        kanibal_id++;

        sleep(1); // Sleep for 1 second to space out thread creation

        pthread_create(&kanibali, NULL, kanibal, &kanibal_id); // Create another kanibal thread
        kanibal_id++;

        sleep(1); // Sleep for 1 second before creating the next pair
    }

    // Wait for all misionari and kanibali threads to complete
    for (int i = 0; i < 5; i++)
    {
        pthread_join(misionari, NULL);
        pthread_join(kanibali, NULL);
        pthread_join(kanibali, NULL);
    }

    return NULL;
}

int main()
{
    pthread_mutex_init(&m, NULL);  // Initialize the mutex for synchronization
    pthread_cond_init(&cam, NULL); // Initialize the condition variable for the boat
    pthread_cond_init(&kan, NULL); // Initialize the condition variable for kanibali
    pthread_cond_init(&mis, NULL); // Initialize the condition variable for misionari

    pthread_t c;                // Thread for the boat
    pthread_t misionar_kanibal; // Thread for creating misionari and kanibali

    pthread_create(&c, NULL, camac, NULL);                          // Create the boat thread
    pthread_create(&misionar_kanibal, NULL, kreiranje_m_i_k, NULL); // Create the thread to create misionari and kanibali

    pthread_join(c, NULL);                // Wait for the boat thread to complete
    pthread_join(misionar_kanibal, NULL); // Wait for the misionar_kanibal thread to complete

    return 0;
}
