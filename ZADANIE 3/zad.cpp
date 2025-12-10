#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include "monitor.h" 

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <pthread.h>
#endif

#define BUFFER_SIZE 9
#define MIN_TO_CONSUME 3 

class Buffer
{
private:
    Semaphore mutex;       
    Semaphore empty;      
    Semaphore semEven;   
    Semaphore semOdd;    

    int A[BUFFER_SIZE];
    int head;  
    int tail;  
    int count; 

    void printState(std::string who, std::string msg)
    {
        std::cout << " [" << who << "] -> " << msg << "\n" 
                  << "stan bufora: { ";
        if (count == 0) {
            std::cout << "pusto";
        } else {
            int tempHead = head; 
            for (int i = 0; i < count; ++i) {
                std::cout << A[tempHead];
                if (i < count - 1) std::cout << ", ";
                tempHead = (tempHead + 1) % BUFFER_SIZE;
            }
        }
        std::cout << " } (Liczba elem: " << count << ")\n\n";
    }

    void signalNextConsumer(std::string triggerSource)
    {
        if (count > MIN_TO_CONSUME)
        {
            int val = A[head];
            if (val % 2 == 0)
            {
                std::cout << "Otwieram semafor dla Konsumenta A.\n";
                semEven.v();
            }
            else
            {
                std::cout << "Otwieram semafor dla Konsumenta B.\n";
                semOdd.v();
            }
        }
        else
        {
            std::cout << "Za malo elementow (" << count << ").\n";
        }
    }

public:
    Buffer() 
        : mutex(1),           
          empty(9), 
          semEven(0), 
          semOdd(0)   
    {
        head = 0; tail = 0; 
        count = 0;
        count = 0;
    }

    void put(int item, std::string prodName)
    {
        empty.p();   
        mutex.p();   

        A[tail] = item;
        tail = (tail + 1) % BUFFER_SIZE;
        count++;

        printState(prodName, "WSTAWIL " + std::to_string(item));

        if (count == MIN_TO_CONSUME + 1)
        {
             signalNextConsumer(prodName);
        }

        mutex.v();    
    }

    void get(bool wantEven, std::string name)
    {
        std::string typ = wantEven ? "PARZYSTE" : "NIEPARZYSTE";
        std::cout << " [" << name << "] PROBUJE POBRAC LICZBE " << typ << ".\n";
        if (wantEven)
            semEven.p();
        else
            semOdd.p();

        mutex.p(); 

        int item = A[head];
        head = (head + 1) % BUFFER_SIZE;
        count--;
        
        printState(name, "POBRAL LICZBE: " + std::to_string(item));

        empty.v(); 

        signalNextConsumer(name);

        mutex.v(); 
    }
};

Buffer buffer;

void* watekProducent(void* arg)
{
    long id = (long)arg; 
    std::string name = (id == 1) ? "ProdA" : "ProdB";

    while(true)
    {
        int val = (rand() % 100) + 1;
        
        #ifdef _WIN32
        Sleep(800 + (id * 200)); 
        #else
        usleep(500000); 
        #endif
        
        buffer.put(val, name);
    }
    return NULL;
}

void* watekKonsumentA(void* arg) // Parzysty
{
    std::string name = "Kons(P)";
    while(true)
    {
        buffer.get(true, name);
        #ifdef _WIN32
        Sleep(1000);
        #else
        usleep(600000);
        #endif
    }
    return NULL;
}

void* watekKonsumentB(void* arg) // Nieparzysty
{
    std::string name = "Kons(NP)";
    while(true)
    {
        buffer.get(false, name);
        #ifdef _WIN32
        Sleep(500);
        #else
        usleep(700000);
        #endif
        
    }
    return NULL;
}

int main()
{
    srand(time(NULL));
    printf("--- START SYMULACJI ---\n");

#ifdef _WIN32
    HANDLE tid[4];
    DWORD id;
    tid[0] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)watekProducent, (void*)1, 0, &id); 
    tid[1] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)watekProducent, (void*)2, 0, &id); 
    tid[2] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)watekKonsumentA, 0, 0, &id);
    tid[3] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)watekKonsumentB, 0, 0, &id);
    WaitForSingleObject(tid[0], INFINITE); 
#else
    pthread_t p1, p2, kA, kB;
    pthread_create(&p1, NULL, watekProducent, (void*)1); 
    pthread_create(&p2, NULL, watekProducent, (void*)2); 
    pthread_create(&kA, NULL, watekKonsumentA, NULL);
    pthread_create(&kB, NULL, watekKonsumentB, NULL);
    pthread_join(p1, NULL);
#endif

    return 0;
}
