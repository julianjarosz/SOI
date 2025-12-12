#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include "semaphore.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
// 2 producentow, 2 konsumentow. Kons A pobiera liczby parzyste, Kons B nieparzyste.
// warunki: bufor.size() > 3 przed pobraniem przez konsumenta i parzystosc/liczba nieparzysta
int const threadsCounts = 4;
int const bufferSize = 9;

class Buffer
{
private:
    Semaphore mutex;       
    Semaphore empty;       
    Semaphore semA;       
    Semaphore semB;

    bool waitingA;
    bool waitingB;

    std::vector<int> values;

    void printBuffer()
    {
        std::cout << "   [BUFOR: ";
        for (auto v : values) std::cout << v << " ";
        std::cout << "] Size: " << values.size() << "\n";
    }

    void signalNext()
    {
        if(values.size() > 3){
            int val = values.front();
            
            if (val % 2 == 0 && waitingA)
            {
                waitingA = false;
                semA.v();
            }
            else if (val % 2 != 0 && waitingB)
            {
                waitingB = false;
                semB.v();
            }
        }
    }

public:
    Buffer() 
        : mutex(1), empty(bufferSize), semA(0), semB(0), 
          waitingA(false), waitingB(false)
    {
    }

    void put(int value)
    {
        empty.p(); 
        mutex.p(); 
        values.push_back(value);
        std::cout << "PRODUCENT: Dodano " << value << ".";
        printBuffer();
        signalNext();
        mutex.v(); 
    }

    int getA()
    {
        mutex.p(); 
        
        while (values.size() <= 3 || values.front() % 2 != 0)
        {
            waitingA = true;
            mutex.v();
            semA.p();         // czeka na sygnał od signalNext()
            mutex.p();
        }
        
        int v = values.front();
        values.erase(values.begin());
        std::cout << "KONSUMENT A: ZABRAL " << v << ".";
        printBuffer();
        
        empty.v();
        signalNext();
        mutex.v();
        return v;
    }

    int getB()
    {
        mutex.p();
        
        while (values.size() <= 3 || values.front() % 2 == 0)
        {
           
            waitingB = true;
            mutex.v();
            semB.p();
            mutex.p();
        }
        
        int v = values.front();
        values.erase(values.begin());
        std::cout << "KONSUMENT B: ZABRAL " << v << ".";
        printBuffer();
        
        empty.v();
        signalNext();
        mutex.v();
        return v;
    }
};
Buffer buffer;


void* threadProd(void* arg)
{
    while(true) {
        #ifdef _WIN32
        Sleep(800);  
        #else
        usleep(8000); 
        #endif
        buffer.put(rand() % 100 + 1);
    }
    return NULL;
}

void* threadConsA(void* arg)
{
     while(true)
    {
        buffer.getA();
        #ifdef _WIN32
        Sleep(1500);
        #else
        usleep(800000);
        #endif
    }
    return NULL;
}

void* threadConsB(void* arg)
{
    while(true)
    {
        buffer.getB();
        #ifdef _WIN32
        Sleep(1500);
        #else
        usleep(800000);
        #endif
    }
    return NULL;
}

int main()
{
    srand(time(NULL)); 

#ifdef _WIN32
    HANDLE tid[threadsCounts];
    DWORD id;

    tid[0] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadProd, 0, 0, &id);
    tid[1] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadProd, 0, 0, &id);
    tid[2] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadConsA, 0, 0, &id);
    tid[3] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadConsB, 0, 0, &id);

    for (int i = 0; i < threadsCounts; i++)
        WaitForSingleObject(tid[i], INFINITE);
#else
    pthread_t tid[threadsCounts];

    pthread_create(&(tid[0]), NULL, threadProd, NULL);
    pthread_create(&(tid[1]), NULL, threadProd, NULL);
    pthread_create(&(tid[2]), NULL, threadConsA, NULL);
    pthread_create(&(tid[3]), NULL, threadConsB, NULL);

    for (int i = 0; i < threadsCounts; i++)
        pthread_join(tid[i], (void**)NULL);
#endif
    return 0;
}